package game

import (
	"encoding/binary"
	"errors"
	"reflect"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func officialCrystalIngredients(p *Player) {
	for i := 0; i < 7; i++ {
		p.Char.Inv[i] = circleTestItem(uint16(421+i), 3)
	}
	p.Char.Inv[7] = circleTestItem(421, 1) // All visible crystal slots are consumed.
	p.Char.Inv[60] = circleTestItem(421, 10)
}

func officialCrystalWeapon(t *testing.T, w *World, magic bool) model.Item {
	t.Helper()
	for index := uint16(1); index < 3000; index++ {
		def := w.items[index]
		override := false
		for _, effect := range def.StaticEffects {
			switch effect.Name {
			case "EF_SANC", "EF_AMOUNT", "EF_INCUBATE", "EF_WDAY", "EF_HOUR", "EF_MIN":
				override = true
			}
		}
		if index > 0 && index < 3000 && (def.Pos == 64 || def.Pos == 192) &&
			(def.Unique == 44 || def.Unique == 47) == magic &&
			!override && staticAbility(def, "EF_MOBTYPE") != 1 && staticAbility(def, "EF_MOBTYPE") != 3 {
			item := circleTestItem(index, 1)
			item.Eff = [6]byte{43, 9, 2, 99, 60, 88}
			item.ActivatedUnix = 1234
			return item
		}
	}
	t.Fatal("no eligible weapon in active catalog")
	return model.Item{}
}

// Dormant crystal-service aliases retain unit coverage without requiring a
// non-reference world spawn. Gold_Dragon supplies the real integration fixture.
func officialCrystalServiceFixture(t *testing.T, name string) (*World, *Player, *craftStore, []byte) {
	t.Helper()
	w, p, st, packet := circleServiceFixture(t, "Gold_Dragon")
	if name != "Gold_Dragon" {
		npcs, _ := officialNPCDataset(t)
		if npcs[name] == nil {
			t.Fatalf("missing crystal-service template: %s", name)
		}
		w.mobsByID[1100].Def = npcs[name]
	}
	return w, p, st, packet
}

func TestOfficialNPCServicesRouteAndPersist(t *testing.T) {
	for _, name := range []string{"Gold_Dragon", "July", "Trainer4", "Trainer5"} {
		for _, magic := range []bool{false, true} {
			t.Run(name+"/"+map[bool]string{false: "physical", true: "magic"}[magic], func(t *testing.T) {
				w, p, st, packet := officialCrystalServiceFixture(t, name)
				w.clock = newFakeClock(time.Date(2026, 10, 29, 20, 0, 0, 0, time.UTC))
				w.rng = fixedRNG{value: 0}
				officialCrystalIngredients(p)
				before := officialCrystalWeapon(t, w, magic)
				p.Char.Equip[6] = before
				setAffectAt(p.Char, 23, 10, 1, 30, w.now())
				gold := p.Char.Gold
				w.onUseNPC(p.Session, packet)
				want := [6]byte{43, 2, 26, 15, 26, 15}
				if magic {
					want = [6]byte{43, 2, 60, 12, 60, 12}
				}
				if p.Char.Equip[6].Eff != want || p.Char.Equip[6].UID != before.UID ||
					p.Char.Equip[6].ActivatedUnix != before.ActivatedUnix || p.Char.Gold != gold || st.saves != 1 {
					t.Fatalf("incorrect persisted conversion: item=%+v gold=%d saves=%d", p.Char.Equip[6], p.Char.Gold, st.saves)
				}
				for _, item := range p.Char.Inv[:60] {
					if item.Index >= 421 && item.Index <= 427 {
						t.Fatal("visible crystal survived conversion")
					}
				}
				if p.Char.Inv[60].Index != 421 || itemStackAmount(p.Char.Inv[60]) != 10 {
					t.Fatal("hidden inventory was consumed")
				}
				if activePlayerAffectAt(p.Char, 23, w.now()) != nil {
					t.Fatal("affect 23 survived conversion")
				}
				affect := activePlayerAffectAt(p.Char, 9, w.now())
				if affect == nil || affect.Level != 200 || affect.Value != 5 ||
					!affect.ExpiresAt.Equal(w.now().Add(160*time.Second)) {
					t.Fatalf("crystal Magic Weapon must last exactly 160 seconds: %+v", affect)
				}
				if activePlayerAffectAt(p.Char, 9, w.now().Add(160*time.Second-time.Millisecond)) == nil ||
					activePlayerAffectAt(p.Char, 9, w.now().Add(160*time.Second)) != nil {
					t.Fatal("crystal Magic Weapon expired outside its 160-second boundary")
				}
				beforeReplay := accountStateSnapshot(p.Account)
				w.onUseNPC(p.Session, packet)
				if st.saves != 1 || !reflect.DeepEqual(beforeReplay, accountStateSnapshot(p.Account)) {
					t.Fatal("duplicate click converted weapon again without crystals")
				}
			})
		}
	}
}

func TestOfficialCrystalAdmissionPreservesEconomy(t *testing.T) {
	for _, reason := range []string{"missing type", "hidden type", "empty weapon", "ancient", "mob type 1", "mob type 3", "invalid position", "unknown catalog", "dead", "range", "visibility", "save"} {
		t.Run(reason, func(t *testing.T) {
			w, p, st, packet := officialCrystalServiceFixture(t, "July")
			officialCrystalIngredients(p)
			p.Char.Equip[6] = officialCrystalWeapon(t, w, false)
			switch reason {
			case "missing type":
				p.Char.Inv[6] = model.Item{}
			case "hidden type":
				p.Char.Inv[61], p.Char.Inv[6] = p.Char.Inv[6], model.Item{}
			case "empty weapon":
				p.Char.Equip[6] = model.Item{}
			case "ancient":
				p.Char.Equip[6].Index = 3000
			case "mob type 1", "mob type 3", "invalid position":
				index := p.Char.Equip[6].Index
				def := w.items[index]
				if reason == "invalid position" {
					def.Pos = 2
				} else {
					value := 1
					if reason == "mob type 3" {
						value = 3
					}
					def.StaticEffects = []model.StaticEffect{{Name: "EF_MOBTYPE", Value: value}}
				}
				w.items[index] = def
			case "unknown catalog":
				delete(w.items, p.Char.Equip[6].Index)
			case "dead":
				p.Char.Score.CurHP, p.Char.RuntimeScore.CurHP = 0, 0
			case "range":
				p.X += 50
			case "visibility":
				p.Visible = nil
			case "save":
				st.err = errors.New("storage unavailable")
			}
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || reason != "save" && st.saves != 0 {
				t.Fatal("rejected conversion changed persistent character state")
			}
		})
	}
}

func TestOfficialTreasureBranchesPreserveIdentityAndDependencies(t *testing.T) {
	for _, scenario := range []string{"fragments", "complete", "jewel", "save", "hidden complete"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, st, packet := circleServiceFixture(t, "Gold_Dragon")
			for i, slot := range []int{0, 1, 9, 10, 18, 19} {
				p.Char.Inv[slot] = circleTestItem(uint16(788+i), 1)
				p.Char.Inv[slot].Eff = [6]byte{7, 31, 8, 12, 9, 9}
			}
			switch scenario {
			case "fragments", "save":
				p.Char.Inv[19] = model.Item{}
			case "jewel":
				p.Char.Equip[13] = circleTestItem(4060, 1)
			case "hidden complete":
				p.Char.Inv[60], p.Char.Inv[19] = p.Char.Inv[19], model.Item{}
			}
			if scenario == "save" {
				st.err = errors.New("storage unavailable")
			}
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if scenario == "complete" || scenario == "jewel" || scenario == "save" {
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || scenario != "save" && st.saves != 0 {
					t.Fatal("unsupported/failed treasure service consumed items")
				}
				return
			}
			for slot, previous := range before.Chars[0].Inv {
				want := previous
				if slot < 60 && want.Index >= 788 && want.Index <= 793 {
					want.Index = 485
				}
				if p.Char.Inv[slot] != want {
					t.Fatalf("fragment identity/effects lost in slot %d", slot)
				}
			}
			if st.saves != 1 {
				t.Fatal("fragment conversion did not persist once")
			}
		})
	}
}

func TestOfficialNPCServicesWaitForDurableCompletion(t *testing.T) {
	for _, operation := range []string{"crystals", "fragments"} {
		for _, outcome := range []string{"success", "rejected", "unknown"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
				w, p, fallback, packet := officialCrystalServiceFixture(t, "Trainer4")
				w.clock = newFakeClock(time.Date(2026, 10, 29, 20, 0, 0, 0, time.UTC))
				w.rng = fixedRNG{}
				if operation == "crystals" {
					officialCrystalIngredients(p)
					p.Char.Equip[6] = officialCrystalWeapon(t, w, false)
				} else {
					p.Char.Inv[0] = circleTestItem(789, 1)
				}
				claimCriticalTestPlayers(t, w, p)
				gate, release := criticalGate(t)
				backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
				if outcome == "rejected" {
					backend.failure = criticalCertifiedRejection(t)
				}
				if outcome == "unknown" {
					backend.failure = errors.New("acknowledgement lost")
				}
				queue := newCriticalTestQueue(t, backend, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				before := accountStateSnapshot(p.Account)
				w.onUseNPC(p.Session, packet)
				write := awaitCriticalWrite(t, backend.entered)
				if operation == "crystals" {
					affect := activePlayerAffectAt(&write.accounts[0].Chars[p.CharSlot], 9, w.now())
					if affect == nil || !affect.ExpiresAt.Equal(w.now().Add(160*time.Second)) {
						t.Fatalf("durable crystal proposal must retain the 160-second expiry: %+v", affect)
					}
				}
				if reflect.DeepEqual(before, write.accounts[0]) || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) ||
					p.Session.QueuedPacketsForTest() != 0 || !w.criticalTradeReserved(p) {
					t.Fatal("conversion exposed before durable acknowledgement")
				}
				w.onUseNPC(p.Session, packet)
				if len(backend.entered) != 0 {
					t.Fatal("duplicate click admitted during persistence")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if fallback.saves != 0 {
					t.Fatal("critical transaction used fallback persistence")
				}
				if outcome == "success" {
					if !reflect.DeepEqual(write.accounts[0], accountStateSnapshot(p.Account)) ||
						w.criticalTradeReserved(p) || p.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("durable conversion was not installed and published")
					}
				} else if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
					t.Fatal("failed transaction changed live items")
				}
				if outcome == "unknown" && (!p.PersistencePoisoned || !w.criticalTradeReserved(p)) {
					t.Fatal("unknown transaction outcome was not fenced")
				}
			})
		}
	}
}

func TestOfficialZakumPopulationUsesInclusiveAreaAndLastPlayerID(t *testing.T) {
	w, p, st, packet := circleServiceFixture(t, "Leaky_Zakum")
	before := accountStateSnapshot(p.Account)
	w.onUseNPC(p.Session, packet)
	message, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(message) || wire.ParseHeader(message).Type != wire.OpMessageIndexed ||
		int16(binary.LittleEndian.Uint16(message[14:16])) != zakumEmptyMessage-1000 {
		t.Fatal("empty area did not use localized message")
	}
	for i, pos := range [][2]uint16{{2180, 1160}, {2296, 1270}, {2297, 1270}, {2180, 1159}, {2180, 1271}} {
		candidate, _ := networkedTestPlayer(uint16(i+2), "Population"+string(rune('A'+i)), pos[0], pos[1])
		w.players[candidate.Session] = candidate
	}
	// Dead players still occupy the area in the native helper.
	for _, candidate := range w.players {
		if candidate.ID == 3 {
			candidate.Char.Score.CurHP = 0
		}
	}
	w.onUseNPC(p.Session, packet)
	message, ok = p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(message) || wire.ParseHeader(message).Type != wire.OpMessageParameterized ||
		int16(binary.LittleEndian.Uint16(message[14:16])) != zakumPopulationMessage-1000 ||
		strings.TrimRight(string(message[16:107]), "\x00") != "PopulationB,1" {
		t.Fatal("population query did not preserve inclusive bounds or highest-ID name")
	}
	if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || st.saves != 0 {
		t.Fatal("read-only population query mutated/saved player state")
	}
	// An overlapping private runtime must not inflate the public-area count.
	w.itemInstances = map[string]*ItemInstance{"private-zakum": {MemberIDs: []uint16{3}}}
	w.onUseNPC(p.Session, packet)
	message, ok = p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(message) || strings.TrimRight(string(message[16:107]), "\x00") != "PopulationA,0" {
		t.Fatal("population query leaked a private participant or omitted a solo player")
	}
}

func TestOfficialCrystalRollBoundaries(t *testing.T) {
	for _, tc := range []struct {
		name   string
		unique int
		draws  []int
		want   [6]byte
	}{
		{"physical lowest", 0, []int{0, 0, 99, 0, 0}, [6]byte{43, 2, 26, 15, 26, 15}},
		{"physical attack", 0, []int{1, 2, 0, 15, 11}, [6]byte{43, 2, 2, 27, 2, 27}},
		{"physical critical", 0, []int{2, 3, 0, 16, 12}, [6]byte{43, 1, 74, 12, 74, 12}},
		{"physical empty second", 0, []int{100, 5, 0, 99, 70}, [6]byte{43, 0, 2, 27, 59, 0}},
		{"physical sanc one end", 0, []int{0, 0, 0, 99, 69}, [6]byte{43, 1, 26, 15, 26, 12}},
		{"magic attack", 44, []int{0, 0, 0, 15, 99}, [6]byte{43, 0, 60, 12, 60, 12}},
		{"magic critical", 47, []int{1, 2, 0, 16, 99}, [6]byte{43, 0, 74, 15, 74, 12}},
		{"magic empty", 44, []int{2, 5, 0, 99, 99}, [6]byte{43, 0, 59, 0, 59, 0}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			rng := &sequenceRNG{values: append([]int(nil), tc.draws...)}
			w := &World{rng: rng}
			before := circleTestItem(123, 1)
			got := w.rollOfficialCrystalWeapon(before, model.ItemDef{Unique: tc.unique})
			if got.Eff != tc.want || got.Index != before.Index || got.UID != before.UID || len(rng.values) != 0 {
				t.Fatalf("roll %+v, want %v; remaining draws %v", got, tc.want, rng.values)
			}
		})
	}
}

func TestOfficialCrystalPublishesCommittedAppearance(t *testing.T) {
	w, p, st, packet := officialCrystalServiceFixture(t, "July")
	w.rng = fixedRNG{}
	officialCrystalIngredients(p)
	p.Char.Equip[6] = officialCrystalWeapon(t, w, false)
	observer, _ := networkedTestPlayer(2, "Observer", p.X, p.Y)
	w.players[observer.Session] = observer
	w.playersByID[observer.ID] = observer
	w.updatePlayerSpatial(observer)
	observer.show(p.ID)
	p.show(observer.ID)
	w.onUseNPC(p.Session, packet)
	if st.saves != 1 {
		t.Fatal("conversion did not persist")
	}
	appearance, ok := observer.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(appearance) || wire.ParseHeader(appearance).Type != wire.OpUpdateEquip ||
		!reflect.DeepEqual(appearance[12:], playerAppearancePacket(p)[12:]) {
		t.Fatal("observer did not receive committed weapon appearance")
	}
	late := playerEnterViewPackets(p)
	if len(late) == 0 || binary.LittleEndian.Uint16(late[0][34+6*2:]) != bodyMesh(p.Char)[6] {
		t.Fatal("new observer did not receive committed weapon")
	}
}

func TestOfficialServiceKindsReserveOnlyVerifiedIdentities(t *testing.T) {
	npcs, _ := officialNPCDataset(t)
	for _, name := range []string{"Gold_Dragon", "July", "Trainer4", "Trainer5", "Leaky_Zakum"} {
		if _, ok := reservedNPCKind(npcs[name]); !ok {
			t.Fatalf("service %s is not reserved", name)
		}
		file := model.QuestFile{Version: model.QuestFileVersion, Quests: []model.QuestDef{simpleQuest(1, name)}}
		if _, err := indexQuests(file, []model.NPCDef{*npcs[name]}); err == nil {
			t.Fatalf("quest may shadow %s service", name)
		}
	}
	for _, name := range []string{"Helena", "Juli", "Trainer6"} {
		if _, ok := officialNPCServiceKind(npcs[name]); ok {
			t.Fatalf("unverified identity %s enabled", name)
		}
	}
	if _, ok := officialNPCServiceKind(&model.NPCDef{Name: "Carbunkle_Anct", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: 30}}); ok {
		t.Fatal("removed Ancient Carbuncle identity enables an unverified service")
	}
}
