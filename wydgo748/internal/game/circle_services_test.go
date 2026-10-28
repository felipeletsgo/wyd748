package game

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func circleServiceFixture(t *testing.T, name string) (*World, *Player, *craftStore, []byte) {
	t.Helper()
	npcs, geners := officialNPCDataset(t)
	if canonical := map[string]string{"Jeffi": "Alchemy_Jeffi", "Exploit_Leader": "ExploitLeader"}[name]; canonical != "" {
		name = canonical
	}
	w, p, st := handlerTestWorld(t)
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.items, w.skills = catalog.Items, catalog.Skills
	var spawn *model.NPCGener
	for i := range geners {
		if geners[i].Enabled && npcs[name] != nil && npcs[geners[i].Leader] == npcs[name] {
			if spawn != nil {
				t.Fatalf("duplicate enabled %s generator", name)
			}
			spawn = &geners[i]
		}
	}
	if spawn == nil || npcs[name] == nil {
		t.Fatalf("missing imported service/spawn: %s", name)
	}
	origin := spawn.Segments[0]
	p.X, p.Y = origin.X-1, origin.Y
	w.updatePlayerSpatial(p)
	m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: npcs[name]}
	w.registerMobSpatial(m)
	p.show(m.ID)
	p.Char.Score.Level = 400
	p.Char.Gold = 10_000_000
	w.recalcPlayer(p.Char)
	packet := useNPCPacket(m.ID)
	// Both Jeffi's direct click and Shaman's confirmed client dialog send 0.
	binary.LittleEndian.PutUint32(packet[16:20], 0)
	return w, p, st, packet
}

func circleTestItem(index uint16, amount byte) model.Item {
	item := model.Item{Index: index, UID: "11111111111141118111111111111111"}
	if amount > 1 {
		item.Eff = [6]byte{effectAmount, amount}
	}
	return item
}

func countCircleItems(inv [64]model.Item, index uint16) uint32 {
	var count uint32
	for _, item := range inv {
		if item.Index == index {
			count += itemStackAmount(item)
		}
	}
	return count
}

func TestOfficialJeffiCircleCreation(t *testing.T) {
	for _, source := range []uint16{447, 692} {
		for variant := 0; variant < 3; variant++ {
			w, p, st, packet := circleServiceFixture(t, "Jeffi")
			w.rng = fixedRNG{value: variant}
			before := circleTestItem(source, 1)
			before.Eff = [6]byte{7, 3, 8, 4, 9, 5}
			p.Char.Equip[13] = before
			w.onUseNPC(p.Session, packet)
			want := before
			want.Index = source + 1 + uint16(variant)
			cost := uint32(1_000_000)
			if source == 692 {
				cost = 5_000_000
			}
			if p.Char.Equip[13] != want || p.Char.Gold != 10_000_000-cost || st.saves != 1 {
				t.Fatalf("source %d variant %d: item=%+v gold=%d saves=%d", source, variant, p.Char.Equip[13], p.Char.Gold, st.saves)
			}
		}
	}
}

func TestOfficialJeffiConvertsAllGroupsWithOneFee(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Jeffi")
	w.rng = fixedRNG{value: 2} // variant 2, family 3324
	p.Char.Inv[0] = circleTestItem(419, 255)
	p.Char.Inv[1] = circleTestItem(419, 255)
	p.Char.Inv[2] = circleTestItem(420, 25)
	for slot := 3; slot < 6; slot++ {
		p.Char.Inv[slot] = circleTestItem(3327, 4)
	}
	existing := circleTestItem(412, 119)
	p.Char.Inv[6] = existing
	// Native services only inspect the first 60 slots, even though the current
	// inventory wire representation has 64 entries.
	hidden := circleTestItem(419, 20)
	p.Char.Inv[60] = hidden
	w.onUseNPC(p.Session, packet)
	for index, want := range map[uint16]uint32{419: 20, 412: 170, 420: 5, 413: 2, 3327: 0, 3326: 4} {
		if got := countCircleItems(p.Char.Inv, index); got != want {
			t.Fatalf("item %d quantity %d, want %d", index, got, want)
		}
	}
	if p.Char.Inv[60] != hidden || p.Char.Inv[6].UID != existing.UID || itemStackAmount(p.Char.Inv[6]) != 120 {
		t.Fatal("conversion changed hidden inventory or lost the merged stack identity")
	}
	for _, item := range p.Char.Inv[:60] {
		if item.Index != 0 && (item.UID == "" || itemStackAmount(item) > 120) {
			t.Fatalf("invalid materialized output: %+v", item)
		}
	}
	if p.Char.Gold != 9_000_000 || st.saves != 1 {
		t.Fatalf("expected one fee/save: gold=%d saves=%d", p.Char.Gold, st.saves)
	}
}

func TestOfficialJeffiIngredientAdmission(t *testing.T) {
	for _, tc := range []struct {
		name  string
		items map[int]model.Item
		want  uint16
		count uint32
	}{
		{"scraps alone", map[int]model.Item{0: circleTestItem(419, 10)}, 412, 1},
		{"powder alone", map[int]model.Item{0: circleTestItem(420, 10)}, 413, 1},
		{"three stone slots", map[int]model.Item{0: circleTestItem(3327, 1), 1: circleTestItem(3327, 1), 2: circleTestItem(3327, 1)}, 3390, 1},
		{"one stone stack is insufficient", map[int]model.Item{0: circleTestItem(3327, 120)}, 0, 0},
		{"two stone slots are insufficient", map[int]model.Item{0: circleTestItem(3327, 2), 1: circleTestItem(3327, 2)}, 0, 0},
		{"hidden ingredients are insufficient", map[int]model.Item{60: circleTestItem(419, 120)}, 0, 0},
		{"subthreshold scraps", map[int]model.Item{0: circleTestItem(419, 9)}, 0, 0},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st, packet := circleServiceFixture(t, "Jeffi")
			w.rng = fixedRNG{}
			for slot, item := range tc.items {
				p.Char.Inv[slot] = item
			}
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if tc.want == 0 {
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || st.saves != 0 {
					t.Fatal("rejected conversion mutated or persisted the account")
				}
			} else if countCircleItems(p.Char.Inv, tc.want) != tc.count || p.Char.Gold != 9_000_000 || st.saves != 1 {
				t.Fatalf("conversion not committed exactly once: inv=%+v gold=%d saves=%d", p.Char.Inv, p.Char.Gold, st.saves)
			}
		})
	}
}

func TestOfficialCircleServicesRejectWithoutMutation(t *testing.T) {
	for _, name := range []string{"Jeffi", "Shaman"} {
		for _, reason := range []string{"insufficient gold", "save failure", "dead", "out of range", "used ordinary circle"} {
			if name == "Jeffi" && reason == "used ordinary circle" || name == "Shaman" && reason == "insufficient gold" {
				continue
			}
			t.Run(name+"/"+reason, func(t *testing.T) {
				w, p, st, packet := circleServiceFixture(t, name)
				index := uint16(447)
				if name == "Shaman" {
					index = 448
				}
				p.Char.Equip[13] = circleTestItem(index, 1)
				switch reason {
				case "insufficient gold":
					p.Char.Gold = 999_999
				case "save failure":
					st.err = errors.New("injected save failure")
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "out of range":
					p.X += 100
					w.updatePlayerSpatial(p)
				case "used ordinary circle":
					p.Char.DivineCircleUsed = true
				}
				before := accountStateSnapshot(p.Account)
				w.onUseNPC(p.Session, packet)
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
					t.Fatal("rejected operation changed character state")
				}
				wantSaves := 0
				if reason == "save failure" {
					wantSaves = 1
				}
				if st.saves != wantSaves {
					t.Fatalf("save count=%d want=%d", st.saves, wantSaves)
				}
			})
		}
	}
}

func TestOfficialJeffiInsufficientOutputSpaceIsAtomic(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Jeffi")
	for slot := 0; slot < 60; slot++ {
		p.Char.Inv[slot] = circleTestItem(500, 1)
	}
	p.Char.Inv[0] = circleTestItem(419, 11) // Two outputs require two free slots.
	before := accountStateSnapshot(p.Account)
	w.onUseNPC(p.Session, packet)
	if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || st.saves != 0 {
		t.Fatal("insufficient space consumed ingredients, charged gold or used hidden slots")
	}
}

func TestOfficialShamanResetsSelectedTreeAndShortcuts(t *testing.T) {
	for _, first := range []uint16{448, 693} {
		for variant := 0; variant < 3; variant++ {
			w, p, st, packet := circleServiceFixture(t, "Shaman")
			p.Char.Score.Mastery = [4]uint32{300, 130, 40, 0}
			p.Char.LearnedSkill = 0xffffffff
			p.Char.SecondaryLearnedSkill = 0x0fff
			p.Char.Equip[13] = circleTestItem(first+uint16(variant), 1)
			for i := range p.Char.ShortSkill {
				p.Char.ShortSkill[i] = byte(i)
			}
			w.recalcPlayer(p.Char)
			p.Char.Gold = 0 // Shaman does not charge a second circle fee.
			w.onUseNPC(p.Session, packet)
			wantMastery := [4]uint32{200, 80, 0, 0}
			if first == 693 {
				wantMastery[1] = 30
			}
			wantPool := uint32(800) - wantMastery[0] - wantMastery[1]
			if p.Char.Score.Mastery != wantMastery || p.Char.Score.MasterPts != wantPool ||
				p.Char.LearnedSkill != [3]uint32{0xffffff00, 0xffff00ff, 0xff00ffff}[variant] ||
				p.Char.SecondaryLearnedSkill != [3]uint32{0x0ff0, 0x0f0f, 0x00ff}[variant] ||
				p.Char.Equip[13] != (model.Item{}) || p.Char.Gold != 0 || st.saves != 1 {
				t.Fatalf("circle %d variant %d did not apply native reset: %+v", first, variant, p.Char)
			}
			if p.Char.DivineCircleUsed != (first == 448) || p.Char.FullDivineCircleUsed != (first == 693) {
				t.Fatal("incorrect persistent circle usage flags")
			}
			if !bytes.Equal(p.Char.ShortSkill[:], bytes.Repeat([]byte{0xff}, 20)) {
				t.Fatal("shortcuts were not all cleared")
			}
			found := false
			for response, ok := p.Session.DequeuePacketForTest(); ok; response, ok = p.Session.DequeuePacketForTest() {
				if !wire.Decrypt(response) {
					t.Fatal("invalid response encryption")
				}
				if wire.ParseHeader(response).Type == wire.OpSetShortSkill {
					found = len(response) == 32 && bytes.Equal(response[12:], p.Char.ShortSkill[:])
				}
			}
			if !found {
				t.Fatal("missing existing 32-byte shortcut update")
			}
			encoded, err := json.Marshal(p.Account)
			if err != nil {
				t.Fatal(err)
			}
			var restored model.Account
			if err := json.Unmarshal(encoded, &restored); err != nil || restored.Chars[0].DivineCircleUsed != p.Char.DivineCircleUsed || restored.Chars[0].FullDivineCircleUsed != p.Char.FullDivineCircleUsed {
				t.Fatal("usage flags were not preserved across persistence")
			}
		}
	}
}

func TestOfficialShamanAllowsRepeatedFullCircle(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Shaman")
	p.Char.DivineCircleUsed, p.Char.FullDivineCircleUsed = true, true
	p.Char.Score.Mastery = [4]uint32{200, 200, 200, 0}
	p.Char.Equip[13] = circleTestItem(693, 1)
	w.onUseNPC(p.Session, packet)
	if p.Char.Score.Mastery != [4]uint32{100, 100, 100, 0} || st.saves != 1 || p.Char.Equip[13].Index != 0 {
		t.Fatal("an existing full-circle flag incorrectly blocked another full circle")
	}
}

func TestOfficialCircleServicesWaitForDurableAcknowledgement(t *testing.T) {
	for _, name := range []string{"Jeffi", "Shaman"} {
		for _, failed := range []bool{false, true} {
			w, p, fallback, packet := circleServiceFixture(t, name)
			p.Char.Equip[13] = circleTestItem(447, 1)
			if name == "Shaman" {
				p.Char.Equip[13].Index = 448
				p.Char.Score.Mastery = [4]uint32{200, 150, 50, 0}
				p.Char.LearnedSkill = 0xffffff
				w.recalcPlayer(p.Char)
			}
			w.rng = fixedRNG{}
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("account admission failed")
			}
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if failed {
				backend.failure = criticalCertifiedRejection(t)
			}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			write := awaitCriticalWrite(t, backend.entered)
			if reflect.DeepEqual(write.accounts[0], before) || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || !w.criticalTradeReserved(p) || p.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 {
				t.Fatalf("%s exposed an uncommitted operation", name)
			}
			w.onUseNPC(p.Session, packet) // A duplicate cannot charge or consume twice.
			select {
			case <-backend.entered:
				t.Fatal("duplicate admitted while account was reserved")
			default:
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.criticalTradeReserved(p) || p.Session.QueuedPacketsForTest() == 0 {
				t.Fatal("completion did not settle reservation and publication")
			}
			if failed {
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
					t.Fatal("rejected save lost gold, circle, flags or skill state")
				}
			} else if !reflect.DeepEqual(write.accounts[0], accountStateSnapshot(p.Account)) {
				t.Fatal("published state differs from the durable proposal")
			}
		}
	}
}

func TestOfficialCircleServicesFenceUnknownCommitOutcome(t *testing.T) {
	for _, name := range []string{"Jeffi", "Shaman"} {
		w, p, fallback, packet := circleServiceFixture(t, name)
		p.Char.Equip[13] = circleTestItem(447, 1)
		if name == "Shaman" {
			p.Char.Equip[13].Index = 448
		}
		if !w.claimAccountSession(p.Session, p.Account.Name) {
			t.Fatal("account admission failed")
		}
		backend := &criticalTestStore{entered: make(chan criticalWrite, 4), failure: errors.New("commit acknowledgement lost")}
		queue := newCriticalTestQueue(t, backend, 1, 2)
		w.criticalWrites = newCriticalPersistenceOwner(queue)
		before := accountStateSnapshot(p.Account)
		w.onUseNPC(p.Session, packet)
		awaitCriticalWrite(t, backend.entered)
		awaitReadyCriticalResults(t, queue, 1)
		w.processCriticalCompletions(1)
		if !p.PersistencePoisoned || !w.criticalTradeReserved(p) || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || fallback.saves != 0 {
			t.Fatal("unknown commit was published, retried or treated as a certified rollback")
		}
		w.onUseNPC(p.Session, packet)
		if len(backend.entered) != 0 {
			t.Fatal("fenced operation was submitted again")
		}
	}
}
