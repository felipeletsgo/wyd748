package game

import (
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type mysticLandRNG struct {
	t      *testing.T
	values []int
	limits []int
	next   int
}

func (r *mysticLandRNG) Intn(limit int) int {
	r.t.Helper()
	if r.next >= len(r.values) || r.limits[r.next] != limit || r.values[r.next] >= limit {
		r.t.Fatalf("unexpected random draw %d with limit %d", r.next, limit)
	}
	value := r.values[r.next]
	r.next++
	return value
}

func TestMysticLandRewardNativeBandsAndAdditionEndpoints(t *testing.T) {
	for _, band := range []struct{ level, first uint32 }{{0, 551}, {49, 551}, {50, 555}, {79, 555}, {80, 559}, {399, 559}} {
		for variant := 0; variant < 4; variant++ {
			for choice, expected := range [][3]byte{{4, 20, 60}, {2, 5, 25}, {26, 5, 15}, {5, 20, 70}, {60, 2, 8}, {7, 5, 20}, {8, 5, 20}, {9, 5, 20}} {
				rng := &mysticLandRNG{t: t, values: []int{variant, choice, 0, choice, int(expected[2] - expected[1])},
					limits: []int{4, 8, int(expected[2]-expected[1]) + 1, 8, int(expected[2]-expected[1]) + 1}}
				w := &World{rng: rng}
				item := w.rollMysticLandAmulet(band.level)
				if item.Index != uint16(band.first)+uint16(variant) || item.Eff != [6]byte{43, 0, expected[0], expected[1], expected[0], expected[2]} || rng.next != 5 {
					t.Fatalf("level %d variant %d choice %d: %+v", band.level, variant, choice, item)
				}
			}
		}
	}
}

func TestMysticLandStartsOnlyAfterConfirmationAndPersists(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Knights_Leader")
	w.onUseNPC(p.Session, packet)
	if p.Char.MysticLandStarted || st.saves != 0 {
		t.Fatal("unconfirmed click started the quest")
	}
	binary.LittleEndian.PutUint32(packet[16:20], 1)
	w.onUseNPC(p.Session, packet)
	w.onUseNPC(p.Session, packet)
	if !p.Char.MysticLandStarted || p.Char.MysticLandEligible || p.Char.MysticLandRewardClaimed || st.saves != 1 {
		t.Fatal("start was not a single durable transition")
	}
	encoded, err := json.Marshal(p.Account)
	if err != nil {
		t.Fatal(err)
	}
	var restored model.Account
	if err := json.Unmarshal(encoded, &restored); err != nil || !restored.Chars[p.CharSlot].MysticLandStarted {
		t.Fatal("start did not survive JSON persistence")
	}
}

func TestMysticLandKillChanceAndEligibility(t *testing.T) {
	for _, face := range []uint16{239, 241, 283} {
		for roll := 0; roll < 20; roll++ {
			w, p, mob, _ := killRewardTestWorld(t, nil)
			p.Char.MysticLandStarted = true
			mob.Def.Equip.Rosto.Index = face
			w.rng = fixedRNG{value: roll}
			got := w.advanceMysticLand(p, mob)
			if want := face != 283 && roll == 0; got != want || p.Char.MysticLandEligible != want {
				t.Fatalf("face %d roll %d: eligible=%v", face, roll, got)
			}
		}
	}
	for _, state := range []string{"not-started", "already-eligible", "claimed", "service-npc"} {
		w, p, mob, _ := killRewardTestWorld(t, nil)
		mob.Def.Equip.Rosto.Index = 239
		p.Char.MysticLandStarted = state != "not-started"
		p.Char.MysticLandEligible = state == "already-eligible"
		p.Char.MysticLandRewardClaimed = state == "claimed"
		if state == "service-npc" {
			mob.Def.Tipo = model.TipoNPC
		}
		w.rng = &mysticLandRNG{t: t} // Ineligible kills must not draw.
		if w.advanceMysticLand(p, mob) {
			t.Fatalf("invalid progress: %s", state)
		}
	}
}

func TestMysticLandRewardRequiresEligibilityAndIsOneTime(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Exploit_Leader")
	w.rng = fixedRNG{}
	w.onUseNPC(p.Session, packet)
	p.Char.MysticLandStarted = true
	w.onUseNPC(p.Session, packet)
	if st.saves != 0 || p.Char.MysticLandRewardClaimed {
		t.Fatal("unearned reward was granted")
	}
	p.Char.MysticLandEligible = true
	beforeGold, beforeEXP := p.Char.Gold, p.Char.Exp
	w.onUseNPC(p.Session, packet) // Native reward branch accepts ClickOk=0.
	item := p.Char.Inv[0]
	if !p.Char.MysticLandRewardClaimed || item.Index != 559 || item.UID == "" || st.saves != 1 || p.Char.Gold != beforeGold || p.Char.Exp != beforeEXP {
		t.Fatalf("invalid amulet reward: %+v saves=%d", item, st.saves)
	}
	encoded, err := json.Marshal(p.Account)
	if err != nil {
		t.Fatal(err)
	}
	var restored model.Account
	if err := json.Unmarshal(encoded, &restored); err != nil {
		t.Fatal(err)
	}
	*p.Account = restored
	p.Char = &p.Account.Chars[p.CharSlot]
	w.onUseNPC(p.Session, packet)
	if st.saves != 1 || p.Char.Inv[0] != item || p.Char.Inv[1].Index != 0 || !p.Char.MysticLandEligible {
		t.Fatal("relogin allowed a second reward or changed the original identity")
	}
}

func TestMysticLandRewardAdmissionAndSaveFailures(t *testing.T) {
	for _, failure := range []string{"full", "catalog", "save", "dead", "poisoned", "range"} {
		t.Run(failure, func(t *testing.T) {
			w, p, st, packet := circleServiceFixture(t, "Exploit_Leader")
			p.Char.MysticLandStarted, p.Char.MysticLandEligible = true, true
			w.rng = fixedRNG{}
			switch failure {
			case "full":
				for slot := 0; slot < model.PlayerCarrySlots; slot++ {
					p.Char.Inv[slot] = circleTestItem(500, 1)
				}
			case "catalog":
				delete(w.items, 559)
			case "save":
				st.err = errors.New("save unavailable")
			case "dead":
				setPlayerCurHP(p.Char, 0)
			case "poisoned":
				p.PersistencePoisoned = true
			case "range":
				p.X += 50
			}
			before := cloneCharacterState(p.Char)
			w.onUseNPC(p.Session, packet)
			if !reflect.DeepEqual(before, cloneCharacterState(p.Char)) {
				t.Fatal("rejected claim changed quest state or inventory")
			}
			if failure != "save" && st.saves != 0 {
				t.Fatal("invalid claim reached persistence")
			}
		})
	}
	w, p, st, packet := circleServiceFixture(t, "Knights_Leader")
	st.err = errors.New("start unavailable")
	binary.LittleEndian.PutUint32(packet[16:20], 1)
	w.onUseNPC(p.Session, packet)
	if p.Char.MysticLandStarted {
		t.Fatal("failed start was not restored")
	}
}

func TestMysticLandNPCWaitsForDurableCompletion(t *testing.T) {
	for _, name := range []string{"Knights_Leader", "Exploit_Leader"} {
		for _, outcome := range []string{"success", "rejected", "unknown"} {
			t.Run(name+"/"+outcome, func(t *testing.T) {
				w, p, fallback, packet := circleServiceFixture(t, name)
				if name == "Knights_Leader" {
					binary.LittleEndian.PutUint32(packet[16:20], 1)
				} else {
					p.Char.MysticLandStarted, p.Char.MysticLandEligible = true, true
				}
				claimCriticalTestPlayers(t, w, p)
				gate, release := criticalGate(t)
				backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
				if outcome == "rejected" {
					backend.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					backend.failure = errors.New("acknowledgement lost")
				}
				queue := newCriticalTestQueue(t, backend, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				before := accountStateSnapshot(p.Account)
				w.onUseNPC(p.Session, packet)
				write := awaitCriticalWrite(t, backend.entered)
				if reflect.DeepEqual(before, write.accounts[0]) || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || p.Session.QueuedPacketsForTest() != 0 || !w.criticalTradeReserved(p) {
					t.Fatal("quest or item exposed before durable acknowledgement")
				}
				w.onUseNPC(p.Session, packet)
				if len(backend.entered) != 0 {
					t.Fatal("duplicate claim admitted during persistence")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if fallback.saves != 0 {
					t.Fatal("critical transaction fell back to a second save")
				}
				if outcome == "success" {
					if !reflect.DeepEqual(write.accounts[0], accountStateSnapshot(p.Account)) || w.criticalTradeReserved(p) || p.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("acknowledged proposal was not installed and published")
					}
				} else if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
					t.Fatal("failed proposal changed live state")
				}
				if outcome == "unknown" && (!p.PersistencePoisoned || !w.criticalTradeReserved(p)) {
					t.Fatal("uncertain outcome was not fenced")
				}
			})
		}
	}
}

func TestMysticLandProgressCommitsWithTheKill(t *testing.T) {
	for _, failed := range []bool{false, true} {
		t.Run(fmt.Sprint(failed), func(t *testing.T) {
			var failure error
			if failed {
				failure = errors.New("kill save unavailable")
			}
			w, p, mob, st := killRewardTestWorld(t, failure)
			p.Char.MysticLandStarted = true
			mob.Def.Equip.Rosto.Index = 241
			w.killMobState(p, mob, 100, 100)
			if p.Char.MysticLandEligible == failed || mob.Dead == failed || st.saves != 1 || st.queuedAtSave != 0 {
				t.Fatal("kill and quest progress did not share the persistence boundary")
			}
			ready := false
			for packet, ok := p.Session.DequeuePacketForTest(); ok; packet, ok = p.Session.DequeuePacketForTest() {
				if !wire.Decrypt(packet) {
					t.Fatal("invalid response encryption")
				}
				if wire.ParseHeader(packet).Type == wire.OpMessageIndexed && int16(binary.LittleEndian.Uint16(packet[14:16])) == mysticLandReadyMessage-1000 {
					ready = true
				}
			}
			if ready == failed {
				t.Fatal("quest-ready message did not follow the durable outcome")
			}
		})
	}
}

func TestMysticLandProgressWaitsForKillAcknowledgement(t *testing.T) {
	w, p, mob, fallback := killRewardTestWorld(t, nil)
	p.Char.MysticLandStarted = true
	mob.Def.Equip.Rosto.Index = 239
	claimCriticalTestPlayers(t, w, p)
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	w.killMobState(p, mob, 100, 100)
	write := awaitCriticalWrite(t, backend.entered)
	if !write.accounts[0].Chars[p.CharSlot].MysticLandEligible || p.Char.MysticLandEligible || p.Session.QueuedPacketsForTest() != 0 || !w.criticalTradeReserved(p) {
		t.Fatal("kill progress was exposed before acknowledgement")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !p.Char.MysticLandEligible || w.criticalTradeReserved(p) || fallback.saves != 0 {
		t.Fatal("durable kill progress was not installed")
	}
}

func TestMysticLandImportedNPCQuestLifecycle(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Knights_Leader")
	w.rng = fixedRNG{}
	binary.LittleEndian.PutUint32(packet[16:20], 1)
	w.onUseNPC(p.Session, packet)
	npcs, geners := officialNPCDataset(t)
	monster := &Mob{ID: 1400, X: p.X + 1, Y: p.Y, GenerIndex: -1, Def: npcs["Basilisk"]}
	if monster.Def == nil || monster.Def.Equip.Rosto.Index != 241 {
		t.Fatal("actual Basilisk template does not match the native quest face")
	}
	w.mobs = append(w.mobs, monster)
	w.registerMobSpatial(monster)
	w.killMobState(p, monster, 100, 100)
	if !p.Char.MysticLandStarted || !p.Char.MysticLandEligible || p.Char.MysticLandRewardClaimed || !monster.Dead {
		t.Fatal("imported start NPC and monster did not unlock the reward")
	}
	for _, spawn := range geners {
		if !spawn.Enabled || spawn.Leader != "ExploitLeader" {
			continue
		}
		origin := spawn.Segments[0]
		reward := &Mob{ID: 1101, X: origin.X, Y: origin.Y, Def: npcs["ExploitLeader"]}
		w.registerMobSpatial(reward)
		p.X, p.Y = origin.X-1, origin.Y
		w.updatePlayerSpatial(p)
		p.show(reward.ID)
		w.onUseNPC(p.Session, useNPCPacket(reward.ID))
		w.onUseNPC(p.Session, useNPCPacket(reward.ID))
		var amulets int
		for _, item := range p.Char.Inv {
			if item.Index >= 551 && item.Index <= 562 {
				amulets++
			}
		}
		if !p.Char.MysticLandRewardClaimed || amulets != 1 || st.saves != 3 {
			t.Fatalf("quest lifecycle: claimed=%v amulets=%d saves=%d", p.Char.MysticLandRewardClaimed, amulets, st.saves)
		}
		return
	}
	t.Fatal("missing enabled reward NPC generator")
}

func TestMysticLandPartyKillCreditsOnlyKiller(t *testing.T) {
	w, killer, member := experiencePartyKillWorld(t)
	w.rng = fixedRNG{}
	killer.Char.MysticLandStarted, member.Char.MysticLandStarted = true, true
	mob := &Mob{ID: 1400, X: killer.X + 1, Y: killer.Y, GenerIndex: -1,
		Def: &model.NPCDef{Tipo: model.TipoMonstro, ExpReward: 100,
			Score: &model.Score{Version: model.ScoreVersion, Level: 1}}}
	mob.Def.Equip.Rosto.Index = 239
	plan := w.planMobKill(killer, mob, 100, 100)
	if plan == nil || len(plan.shares) != 2 || !killer.Char.MysticLandEligible || member.Char.MysticLandEligible {
		t.Fatal("party EXP sharing incorrectly shared Mystic Land eligibility")
	}
}

func TestMysticLandRejectsConflictingGenericQuestBindings(t *testing.T) {
	for _, merchant := range []uint32{mysticLandStartMerchant, mysticLandRewardMerchant} {
		def := model.NPCDef{Name: "NativeQuestNPC", Score: &model.Score{Merchant: merchant}}
		file := model.QuestFile{Version: model.QuestFileVersion,
			Quests: []model.QuestDef{{ID: 1, NPC: def.Name}}}
		if _, err := indexQuests(file, []model.NPCDef{def}); err == nil {
			t.Fatalf("merchant %d accepted an unreachable generic quest", merchant)
		}
	}
}
