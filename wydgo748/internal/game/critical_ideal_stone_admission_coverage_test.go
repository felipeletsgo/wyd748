package game

import (
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestCriticalKingIdealStoneCraftCommitsCompleteRecipe(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	prepareKingIdealRecipe(p)
	before := accountStateSnapshot(p.Account)

	if !w.craftIdealStoneAtKing(p.Session, p) {
		t.Fatal("complete King recipe was not admitted")
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	proposed := proposal.accounts[0].Chars[p.CharSlot]
	for stone := uint16(5334); stone <= 5337; stone++ {
		if countInventoryItem(&proposed, stone) != 0 {
			t.Fatalf("proposed account retained Secret Stone %d", stone)
		}
	}
	if proposed.Equip[10].Index != 0 || proposed.Equip[sefirotSlot].Index != 0 ||
		countInventoryItem(&proposed, idealStoneItem) != 1 {
		t.Fatal("proposal did not replace the required equipment and ingredients with one Ideal Stone")
	}
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
		p.Char.Equip[10].Index != 1742 || p.Char.Equip[sefirotSlot].Index != 1761 {
		t.Fatal("uncommitted Ideal Stone recipe changed the live character")
	}

	settleCriticalCelestialWrite(t, w, queue, release)
	for stone := uint16(5334); stone <= 5337; stone++ {
		if countInventoryItem(p.Char, stone) != 0 {
			t.Fatalf("committed account retained Secret Stone %d", stone)
		}
	}
	if p.Char.Equip[10].Index != 0 || p.Char.Equip[sefirotSlot].Index != 0 ||
		countInventoryItem(p.Char, idealStoneItem) != 1 {
		t.Fatal("commit did not install the Ideal Stone recipe result")
	}
}

func TestKingIdealRecipeRejectsSecretStoneInHiddenInventorySlot(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	prepareKingIdealRecipe(p)
	p.Char.Inv[model.PlayerCarrySlots] = p.Char.Inv[0]
	p.Char.Inv[0] = model.Item{}
	before := accountStateSnapshot(p.Account)
	if w.criticalTradeReserved(p) {
		t.Fatal("King recipe fixture unexpectedly holds a critical persistence reservation")
	}

	accepted := w.craftIdealStoneAtKing(p.Session, p)
	if accepted && len(backend.entered) > 0 {
		awaitCriticalWrite(t, backend.entered)
		settleCriticalCelestialWrite(t, w, queue, release)
	}
	if accepted || len(queue.pending) != 0 || len(backend.entered) != 0 ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
		t.Fatalf("recipe accepted an ingredient outside visible carry inventory: accepted=%t pending=%d writes=%d packets=%d",
			accepted, len(queue.pending), len(backend.entered), p.Session.QueuedPacketsForTest())
	}
}

func TestCriticalIdealStoneCraftReportsUnavailablePersistence(t *testing.T) {
	w, p, backend, queue, _ := criticalCelestialQueueFixture(t, 399)
	prepareKingIdealRecipe(p)
	before := accountStateSnapshot(p.Account)
	w.shuttingDown = true

	if !w.craftIdealStoneAtKing(p.Session, p) {
		t.Fatal("complete King recipe was rejected before persistence admission")
	}
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 0 ||
		len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("stopping persistence mutated the recipe or failed to notify the player")
	}
}
