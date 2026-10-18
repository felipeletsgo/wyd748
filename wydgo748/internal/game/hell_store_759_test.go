package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func attachDionysStore(w *World, p *Player, merchant *Mob) *ItemInstance {
	merchant.Def.Name = "HG_Dionys"
	merchant.Def.Equip.Rosto.Index = 51
	merchant.HP = 150_000
	merchant.InstanceID = "dionys-store-test"
	inst := &ItemInstance{
		RuntimeID: merchant.InstanceID,
		Config: model.VolatileInstance{StateMachine: "hell_gate", HellGate: &model.VolatileHellGate{Version: 3},
			Stages: []model.VolatileInstanceStage{{X: p.X, Y: p.Y, AreaRadius: 10}}},
		State: "protection", HardDeadline: w.now().Add(time.Minute),
		MemberIDs: []uint16{p.ID}, NPCIDs: map[uint16]struct{}{merchant.ID: {}},
	}
	if w.itemInstances == nil {
		w.itemInstances = make(map[string]*ItemInstance)
	}
	w.itemInstances[inst.RuntimeID] = inst
	w.rebuildPlayerInstanceIndex()
	return inst
}

func TestDionys759WeightedBoundaries(t *testing.T) {
	for _, tc := range []struct {
		weights []int
		rolls   []int
		want    int
	}{
		{[]int{3300, 3300, 3400}, []int{0}, 1},
		{[]int{3300, 3300, 3400}, []int{3299}, 1},
		{[]int{3300, 3300, 3400}, []int{3300}, 2},
		{[]int{3300, 3300, 3400}, []int{6599}, 2},
		{[]int{3300, 3300, 3400}, []int{6600}, 3},
		{[]int{3300, 3300, 3400}, []int{9999}, 3},
		{[]int{1000, 2000}, []int{100, 99}, 0},
		{[]int{1000, 2000}, []int{100, 100}, 1},
		{[]int{1000, 2000}, []int{100, 1099}, 1},
		{[]int{1000, 2000}, []int{100, 1100}, 2},
		{[]int{1000, 2000}, []int{100, 3099}, 2},
		{[]int{1000, 2000}, []int{100, 3100}, 0},
		{[]int{0, 0}, []int{100, 100}, 0},
	} {
		w := &World{rng: &sequenceRNG{values: tc.rolls}}
		if got := w.hell759Pick(tc.weights...); got != tc.want {
			t.Fatalf("weights %v rolls %v: got %d, want %d", tc.weights, tc.rolls, got, tc.want)
		}
	}
}

func TestDionys759LotteryGoldenAndCatalog(t *testing.T) {
	items, _ := hellStoreCatalog(t)
	w := &World{items: items}
	for _, pool := range [][]uint16{hell759Weapons[0][:], hell759Weapons[1][:], hell759Armor[0][:], hell759Armor[1][:], {540, 541, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562}} {
		for _, index := range pool {
			if _, exists := items[index]; !exists {
				t.Fatalf("historical item %d missing from active catalog", index)
			}
		}
	}
	for _, tc := range []struct {
		category int
		rolls    []int
		want     model.Item
	}{
		{hellStoreWeapon, nil, model.Item{Index: 869, Eff: [6]byte{43, 0, 2, 27, 2, 18}}},
		{hellStoreArmor, nil, model.Item{Index: 1192, Eff: [6]byte{43, 0, 4, 30, 3, 20}}},
		{hellStoreAmulet, nil, model.Item{Index: 540}},
		{hellStoreRandom, nil, model.Item{Index: 869, Eff: [6]byte{43, 0, 2, 27, 2, 18}}},
		{hellStoreWeapon, []int{9999, 0, 9999, 0, 5399, 9999, 0, 6499, 0, 0, 4999}, model.Item{Index: 900, Eff: [6]byte{43, 0, 74, 18, 60, 16}}},
	} {
		rng := &sequenceRNG{values: append([]int(nil), tc.rolls...)}
		w.rng = rng
		got, ok := w.rollDionysStoreItem(tc.category)
		if !ok || got != tc.want || len(rng.values) != 0 {
			t.Fatalf("category %d: got %+v (%v), want %+v; unused rolls %v", tc.category, got, ok, tc.want, rng.values)
		}
	}
	w.items = nil
	if _, ok := w.rollDionysStoreItem(hellStoreWeapon); ok {
		t.Fatal("missing catalog accepted")
	}
	if _, ok := w.rollDionysStoreItem(0); ok {
		t.Fatal("invalid category accepted")
	}
}

func TestDionys759StoreLifecycleAndFailedSave(t *testing.T) {
	for _, reason := range []string{"success", "admission", "deadline", "dead", "nonmember", "unowned", "other-runtime", "save-failure"} {
		t.Run(reason, func(t *testing.T) {
			w, p, st, merchant := hellStoreWorld(t)
			inst := attachDionysStore(w, p, merchant)
			rng := &criticalPurchaseRNG{}
			w.rng = rng
			switch reason {
			case "admission":
				inst.State = "admission"
			case "deadline":
				inst.HardDeadline = w.now()
			case "dead":
				merchant.HP, merchant.Dead = 0, true
			case "nonmember":
				inst.MemberIDs = nil
				w.rebuildPlayerInstanceIndex()
			case "unowned":
				delete(inst.NPCIDs, merchant.ID)
			case "other-runtime":
				merchant.InstanceID = "other"
			case "save-failure":
				st.err = errors.New("disk full")
			}
			gold, inv := p.Char.Gold, p.Char.Inv
			w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, merchant.ID, hellStoreAmulet, 7))
			if reason == "success" {
				if p.Char.Gold != gold-5_000_000 || p.Char.Inv[7].Index != 540 || p.Char.Inv[7].UID == "" || st.saves != 1 {
					t.Fatal("Dionys did not sell the historical lottery result")
				}
				assertHellStorePublication(t, p)
			} else {
				if p.Char.Gold != gold || p.Char.Inv != inv {
					t.Fatal("rejected purchase changed economic state")
				}
				if reason != "save-failure" && (rng.calls != 0 || st.saves != 0) {
					t.Fatal("unavailable store consumed randomness or saved")
				}
			}
		})
	}
}

func TestCriticalDionys759PurchasePublishesOnlyAfterCommit(t *testing.T) {
	w, p, fallback, merchant, backend, queue, rng, release := criticalHellStoreFixture(t)
	attachDionysStore(w, p, merchant)
	gold, inv := p.Char.Gold, p.Char.Inv
	request := hellStorePacket(p.ID, merchant.ID, hellStoreAmulet, 7)
	w.onHellStoreBuy(p.Session, request)
	write := awaitCriticalWrite(t, backend.entered)
	item := write.accounts[0].Chars[p.CharSlot].Inv[7]
	if item.Index != 540 || item.UID == "" || write.accounts[0].Chars[p.CharSlot].Gold != gold-5_000_000 {
		t.Fatal("invalid historical purchase proposal")
	}
	rolls := rng.calls
	w.onHellStoreBuy(p.Session, request)
	if p.Char.Gold != gold || p.Char.Inv != inv || rng.calls != rolls || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("purchase mutated or rerolled before commit")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Gold != gold-5_000_000 || p.Char.Inv[7] != item {
		t.Fatal("committed purchase missing")
	}
	assertHellStorePublication(t, p)
}
