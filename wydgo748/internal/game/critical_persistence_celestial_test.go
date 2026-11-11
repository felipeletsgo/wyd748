package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func criticalCelestialQueueFixture(t *testing.T, level uint32) (*World, *Player, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _ := newCelestialWorld(t, level)
	w.clock = newFakeClock(time.Unix(100, 0))
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("Celestial fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, backend, queue, release
}

func settleCriticalCelestialWrite(t *testing.T, w *World, queue *criticalPersistence, release func()) {
	t.Helper()
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
}

func TestCriticalCelestialAdmissionRejectionsResyncWithoutPersistence(t *testing.T) {
	w, p, backend, queue, _ := criticalCelestialQueueFixture(t, 399)
	for {
		if _, ok := p.Session.DequeuePacketForTest(); !ok {
			break
		}
	}
	p.Char.Inv[0] = model.Item{Index: 1}
	before := accountStateSnapshot(p.Account)
	w.admitCelestialConsumable(p, 0, "celestial_ideal")
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 0 || len(backend.entered) != 0 {
		t.Fatal("wrong-item rejection changed account state or started persistence")
	}
	if p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("wrong-item rejection did not resynchronize the used slot")
	}

	for {
		if _, ok := p.Session.DequeuePacketForTest(); !ok {
			break
		}
	}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	p.Char.Score.Level = model.CelestialMinimumArchLevel - 1
	w.admitCelestialConsumable(p, 0, "celestial_ideal")
	if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Char.Inv[0].Index != idealStoneItem {
		t.Fatal("business-rule rejection submitted or consumed the Ideal Stone")
	}
	if p.Session.QueuedPacketsForTest() < 2 {
		t.Fatal("business-rule rejection did not send its message and slot resync")
	}

	for {
		if _, ok := p.Session.DequeuePacketForTest(); !ok {
			break
		}
	}
	w.admitCelestialConsumable(p, 0, "unknown_celestial_action")
	if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() < 2 {
		t.Fatal("invalid action did not report unavailable and resynchronize")
	}
}

func TestCriticalSubCelestialCreationCommitsDetachedSnapshot(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 120)
	p.Char.Evolution = "celestial"
	p.Char.Score.Level = 120
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
	p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
	stone := model.Item{Index: idealStoneItem}
	p.Char.Inv[0] = stone
	account, char, score := p.Account, p.Char, p.Char.Score
	before := accountStateSnapshot(account)

	if err := w.queueCelestialConsumable(p, 0, "celestial_ideal"); err != nil {
		t.Fatalf("queue SubCelestial creation: %v", err)
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	if len(proposal.accounts) != 1 || proposal.accounts[0] == account {
		t.Fatal("Celestial proposal did not use a detached account")
	}
	proposed := proposal.accounts[0].Chars[p.CharSlot]
	if proposed.AlternateCelestial == nil || proposed.AlternateCelestial.Evolution != "subcelestial" ||
		proposed.AlternateCelestial.Class != 3 || proposed.AlternateCelestial.Face.Index != 18 ||
		proposed.Inv[0].Index != mysteriousStoneItem || itemStackAmount(proposed.Inv[0]) != 10 || proposed.Equip[sefirotSlot].Index != 0 ||
		counterBalance(&Player{SpecialCoins: proposal.character.state.SpecialCoins}, fameCounter) != 0 {
		t.Fatalf("incorrect detached SubCelestial result: form=%+v inv=%+v amount=%d sefirot=%+v fame=%d",
			proposed.AlternateCelestial, proposed.Inv[0], itemStackAmount(proposed.Inv[0]), proposed.Equip[sefirotSlot],
			counterBalance(&Player{SpecialCoins: proposal.character.state.SpecialCoins}, fameCounter))
	}
	if !reflect.DeepEqual(accountStateSnapshot(account), before) || p.Char != char || p.Char.Score != score ||
		p.Char.Inv[0] != stone || p.Char.AlternateCelestial != nil || counterBalance(p, fameCounter) != subCelestialFameCost ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("uncommitted SubCelestial creation changed actor state or sent packets")
	}

	settleCriticalCelestialWrite(t, w, queue, release)
	if p.Char != char || p.Char.Score != score || p.Char.AlternateCelestial == nil ||
		p.Char.AlternateCelestial.Evolution != "subcelestial" || p.Char.Inv[0].Index != mysteriousStoneItem ||
		itemStackAmount(p.Char.Inv[0]) != 10 || p.Char.Equip[sefirotSlot].Index != 0 || counterBalance(p, fameCounter) != 0 {
		t.Fatal("committed SubCelestial creation did not preserve actor-owned identities and rewards")
	}
}

func TestCriticalCelestialCreationReturnsToSelectionAfterCommit(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	charSlot := p.CharSlot
	p.Char.Equip[model.CapeSlot] = model.Item{Index: 3191}
	stone := p.Char.Inv[0]
	w.items = map[uint16]model.ItemDef{idealStoneItem: {Index: idealStoneItem, Name: "Ideal Stone"}}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{idealStoneItem: {Action: "celestial_ideal", Consume: true}},
		ItemCodes: map[uint16]int{idealStoneItem: 187},
	}
	w.onUseItem(p.Session, useItemPacket(0, 0))
	proposal := awaitCriticalWrite(t, backend.entered)
	proposed := proposal.accounts[0].Chars[p.CharSlot]
	if proposed.Evolution != "celestial" || proposed.CelestialArchTier != 5 || proposed.Equip[1].Index != 3502 ||
		proposed.Equip[model.CapeSlot].Index != 3197 || proposed.Inv[0].Index != 0 || proposed.LearnedSkill != celestialSoulBit {
		t.Fatalf("incorrect detached Celestial conversion: %+v", proposed)
	}
	if p.Char.Evolution != archEvolution || p.Char.Inv[0] != stone || !p.InWorld || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("uncommitted conversion changed the live Arch or left the world")
	}
	settleCriticalCelestialWrite(t, w, queue, release)
	char := p.Account.Chars[charSlot]
	if char.Evolution != "celestial" || char.CelestialArchTier != 5 || char.Equip[1].Index != 3502 ||
		char.Equip[model.CapeSlot].Index != 3197 || char.Inv[0].Index != 0 || char.LearnedSkill != celestialSoulBit {
		t.Fatalf("committed Celestial conversion was not installed: %+v", char)
	}
	if p.InWorld || p.Char != nil || p.ID != 0 {
		t.Fatal("Celestial conversion did not end at character selection")
	}
}

func TestCriticalFuryUnlockRollSettlesOnlyAfterCommit(t *testing.T) {
	for _, test := range []struct {
		name     string
		roll     int
		unlocked bool
	}{
		{"success", celestialFuryUnlockChance - 1, true},
		{"failure", celestialFuryUnlockChance, false},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, backend, queue, release := criticalCelestialQueueFixture(t, 89)
			w.rng = fixedRNG{value: test.roll}
			p.Char.Evolution = "celestial"
			p.Char.Score.Level = 89
			p.Char.CelestialLevel40Unlocked = true
			p.Char.Equip[cytheraEquipSlot] = model.Item{Index: 3501}
			p.Char.Inv[0] = model.Item{Index: furyStoneItem}
			p.SpecialCoins = map[string]uint32{fameCounter: 500}
			before := accountStateSnapshot(p.Account)

			if err := w.queueCelestialConsumable(p, 0, "celestial_fury"); err != nil {
				t.Fatalf("queue Fury attempt: %v", err)
			}
			proposal := awaitCriticalWrite(t, backend.entered)
			proposed := proposal.accounts[0].Chars[p.CharSlot]
			if proposed.CelestialLevel90Unlocked != test.unlocked {
				t.Fatalf("proposed unlock=%t, want %t", proposed.CelestialLevel90Unlocked, test.unlocked)
			}
			if test.unlocked && proposed.Inv[0].Index != 3501 {
				t.Fatal("successful unlock did not reserve the second Cythera in the detached account")
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Char.Inv[0].Index != furyStoneItem ||
				counterBalance(p, fameCounter) != 500 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("uncommitted Fury roll changed actor state or sent packets")
			}

			settleCriticalCelestialWrite(t, w, queue, release)
			if p.Char.CelestialLevel90Unlocked != test.unlocked || p.Char.Inv[0].Index != map[bool]uint16{true: 3501, false: 0}[test.unlocked] ||
				counterBalance(p, fameCounter) != 0 {
				t.Fatal("committed Fury result did not settle its stone, Cythera, and fame")
			}
		})
	}
}

func TestCriticalIdealStoneCraftLeavesIngredientsWhenIncomplete(t *testing.T) {
	w, p, backend, queue, _ := criticalCelestialQueueFixture(t, 399)
	for i := range p.Char.Inv {
		p.Char.Inv[i] = model.Item{}
	}
	before := accountStateSnapshot(p.Account)
	w.admitIdealStoneCraft(p)
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 0 || len(backend.entered) != 0 {
		t.Fatal("incomplete Ideal Stone recipe changed state or started persistence")
	}
}

func TestCriticalCelestialBusyAdmissionUsesUnavailableResponse(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	before := accountStateSnapshot(p.Account)
	if err := w.queueCriticalAccountMutation(p, accountStateSnapshot(p.Account), "hold Celestial admission",
		"", func() {}, nil); err != nil {
		t.Fatalf("queue reservation used to hold admission: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	if err := w.queueCelestialConsumable(p, 0, "celestial_ideal"); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("reserved admission error = %v, want persistence busy", err)
	}
	w.admitCelestialConsumable(p, 0, "celestial_ideal")
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 1 || len(backend.entered) != 0 {
		t.Fatal("busy admission mutated state or started persistence")
	}
	if p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("busy admission did not return the item or unavailable message")
	}
	settleCriticalCelestialWrite(t, w, queue, release)
	if w.criticalPlayerReserved(p) || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
		t.Fatal("held persistence reservation did not settle cleanly")
	}
}
