package game

import (
	"bytes"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func silverDragonFixture(t *testing.T) (*World, *Player, *craftStore, []byte) {
	t.Helper()
	w, p, store := handlerTestWorld(t)
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.items, w.skills = catalog.Items, catalog.Skills
	def := channel2RegressionDefinitions(t)["Dragao_Vermelho"]
	if def == nil || def.Name != "Dragao_Vermelho" || def.InteractionName() != "SilverDragon" ||
		def.Score.Merchant != 62 || def.MakeScore(0).Merchant&15 != 14 {
		t.Fatal("Silver Dragon identity, authoritative service or client selector changed")
	}
	geners, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	count := 0
	for _, gener := range geners {
		if !gener.Enabled || gener.Leader != "Dragao_Vermelho" {
			continue
		}
		count++
		origin := gener.Segments[0]
		if origin.X != 2490 || origin.Y != 1688 {
			t.Fatal("Silver Dragon moved from the official Azran generator")
		}
		p.X, p.Y = origin.X-1, origin.Y
		w.updatePlayerSpatial(p)
		m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: def}
		w.registerMobSpatial(m)
		p.show(m.ID)
	}
	if count != 1 {
		t.Fatalf("expected one Silver Dragon generator, got %d", count)
	}
	p.Char.Gold = 2_000_000
	w.recalcPlayer(p.Char)
	return w, p, store, useNPCPacket(1100)
}

func silverDragonIngredients(p *Player) {
	slot := 0
	for i, count := range []int{10, 2, 4, 1, 1} {
		for n := 0; n < count; n++ {
			item := circleTestItem(uint16(1721+i), 1)
			item.UID = fmt.Sprintf("%032x", slot+1)
			p.Char.Inv[slot] = item
			slot++
		}
	}
}

func TestSilverDragonCreatesNativeReward(t *testing.T) {
	for refinement := 0; refinement < 4; refinement++ {
		t.Run(fmt.Sprint(refinement), func(t *testing.T) {
			w, p, store, packet := silverDragonFixture(t)
			w.rng = fixedRNG{value: refinement}
			silverDragonIngredients(p)
			// A full inventory must succeed by reusing a consumed slot. Extras,
			// unrelated items and the four hidden slots must remain untouched.
			for slot := 18; slot < len(p.Char.Inv); slot++ {
				p.Char.Inv[slot] = circleTestItem(1721, 1)
			}
			p.Char.Inv[19] = circleTestItem(485, 1)
			before := p.Char.Inv
			w.onUseNPC(p.Session, packet)
			reward := p.Char.Inv[0]
			if reward.Index != 1726 || reward.Eff != [6]byte{43, byte(refinement)} || reward.UID == "" ||
				reward.UID == before[0].UID || p.Char.Gold != 0 || store.saves != 1 || p.Session.QueuedPacketsForTest() == 0 {
				t.Fatalf("incorrect committed reward: %+v gold=%d saves=%d", reward, p.Char.Gold, store.saves)
			}
			update, ok := p.Session.DequeuePacketForTest()
			wantUpdate := wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold)
			if !ok || !wire.Decrypt(update) || wire.ParseHeader(update).Type != wire.ParseHeader(wantUpdate).Type ||
				!bytes.Equal(update[12:], wantUpdate[12:]) {
				t.Fatal("client inventory/balance packet does not match committed state")
			}
			for slot := 1; slot < len(before); slot++ {
				want := model.Item{}
				if slot >= 18 {
					want = before[slot]
				}
				if p.Char.Inv[slot] != want {
					t.Fatalf("unexpected material consumption in slot %d", slot)
				}
			}
			// Missing materials must reject a repeated click even with more gold.
			p.Char.Gold = 2_000_000
			after := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if store.saves != 1 || !reflect.DeepEqual(after, accountStateSnapshot(p.Account)) {
				t.Fatal("replay spent gold or created another reward without ingredients")
			}
		})
	}
}

func TestSilverDragonRejectsWithoutEconomicMutation(t *testing.T) {
	for _, reason := range []string{"snowball", "twig", "straw", "hat", "carrot", "hidden", "stack", "gold", "catalog", "dead", "range", "visibility", "save"} {
		t.Run(reason, func(t *testing.T) {
			w, p, store, packet := silverDragonFixture(t)
			silverDragonIngredients(p)
			switch reason {
			case "snowball", "twig", "straw", "hat", "carrot":
				slot := map[string]int{"snowball": 0, "twig": 10, "straw": 12, "hat": 16, "carrot": 17}[reason]
				p.Char.Inv[slot] = model.Item{}
			case "hidden":
				p.Char.Inv[60], p.Char.Inv[17] = p.Char.Inv[17], model.Item{}
			case "stack":
				for slot := 1; slot < 10; slot++ {
					p.Char.Inv[slot] = model.Item{}
				}
				p.Char.Inv[0] = circleTestItem(1721, 10)
			case "gold":
				p.Char.Gold--
			case "catalog":
				delete(w.items, 1726)
			case "dead":
				p.Char.Score.CurHP, p.Char.RuntimeScore.CurHP = 0, 0
			case "range":
				p.X += 50
			case "visibility":
				p.Visible = nil
			case "save":
				store.err = errors.New("storage unavailable")
			}
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || reason != "save" && store.saves != 0 {
				t.Fatal("rejected or failed exchange changed the account")
			}
		})
	}
}

func TestSilverDragonWaitsForDurableCompletion(t *testing.T) {
	for _, outcome := range []string{"success", "rejected", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, fallback, packet := silverDragonFixture(t)
			silverDragonIngredients(p)
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
			if write.accounts[0].Chars[0].Inv[0].Index != 1726 || write.accounts[0].Chars[0].Gold != 0 ||
				!reflect.DeepEqual(before, accountStateSnapshot(p.Account)) ||
				p.Session.QueuedPacketsForTest() != 0 || !w.criticalTradeReserved(p) {
				t.Fatal("exchange exposed before durable acknowledgement")
			}
			w.onUseNPC(p.Session, packet)
			if len(backend.entered) != 0 {
				t.Fatal("duplicate click admitted while saving")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if fallback.saves != 0 {
				t.Fatal("exchange bypassed critical persistence")
			}
			if outcome == "success" {
				if !reflect.DeepEqual(write.accounts[0], accountStateSnapshot(p.Account)) ||
					w.criticalTradeReserved(p) || p.Session.QueuedPacketsForTest() == 0 {
					t.Fatal("committed exchange was not installed and published")
				}
			} else if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
				t.Fatal("failed exchange lost materials or gold")
			}
			if outcome == "unknown" && (!p.PersistencePoisoned || !w.criticalTradeReserved(p)) {
				t.Fatal("unknown result was not fenced against replay")
			}
		})
	}
}

func TestSilverDragonServiceIdentityAndQuestGuard(t *testing.T) {
	def := channel2RegressionDefinitions(t)["Dragao_Vermelho"]
	for _, name := range []string{"Dragao_Vermelho", "SilverDragon", "UnverifiedDragon"} {
		for _, merchant := range []uint32{62, 4} {
			candidate, score := *def, *def.Score
			candidate.Name, candidate.DisplayName = name, name
			candidate.ServiceIdentity = ""
			score.Merchant, candidate.Score = merchant, &score
			_, enabled := officialNPCServiceKind(&candidate)
			want := merchant == 62 && name != "UnverifiedDragon"
			if enabled != want {
				t.Fatalf("service identity %q merchant %d: enabled=%v", name, merchant, enabled)
			}
		}
	}
	if _, reserved := reservedNPCKind(def); !reserved {
		t.Fatal("imported service is not reserved")
	}
	quests := model.QuestFile{Version: model.QuestFileVersion, Quests: []model.QuestDef{simpleQuest(1, def.Name)}}
	if _, err := indexQuests(quests, []model.NPCDef{*def}); err == nil {
		t.Fatal("configured quest can shadow the Grewpain service")
	}
}
