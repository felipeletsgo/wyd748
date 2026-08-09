package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

func odinSecretStoneAttempt(t *testing.T, runes [7]uint16, gold uint32, rng RNG) (*World, *Player, *craftStore) {
	t.Helper()
	defs := map[uint16]model.ItemDef{
		5334: {Index: 5334}, 5335: {Index: 5335},
		5336: {Index: 5336}, 5337: {Index: 5337},
		3020: {Index: 3020},
	}
	w, p, session, st := newCraftWorld(t, "Odin", defs, gold)
	w.rng = rng
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	for i, index := range runes {
		items[i] = model.Item{Index: index}
		pos[i] = int8(i)
	}
	placeItems(p.Char, items, pos)
	w.onCombineOdin(session, buildCombinePacket(items, pos))
	return w, p, st
}

func TestOdinSecretStoneRecipesMatchRetailRunes(t *testing.T) {
	want := [4][8]uint16{
		{5126, 5127, 5121, 5114, 5125, 5111, 5118, 5334},
		{5131, 5113, 5115, 5116, 5125, 5112, 5114, 5335},
		{5110, 5124, 5117, 5129, 5114, 5125, 5128, 5336},
		{5122, 5119, 5132, 5120, 5130, 5133, 5123, 5337},
	}
	if secretStoneRecipes != want {
		t.Fatalf("Secret Stone recipes=%v, want %v", secretStoneRecipes, want)
	}
}

func TestOdinSecretStoneCorrectSequenceUses95PercentAndCharges2M(t *testing.T) {
	recipe := secretStoneRecipes[0]
	var runes [7]uint16
	copy(runes[:], recipe[:7])
	_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost+123, fixedRNG{value: 94})
	if st.saves != 1 {
		t.Fatalf("saves=%d, want 1", st.saves)
	}
	if p.Char.Gold != 123 {
		t.Fatalf("gold=%d, want 123", p.Char.Gold)
	}
	if p.Char.Inv[0].Index != recipe[7] {
		t.Fatalf("result=%d, want %d", p.Char.Inv[0].Index, recipe[7])
	}
	for i := 1; i < 7; i++ {
		if p.Char.Inv[i].Index != 0 {
			t.Fatalf("rune slot %d was not consumed: %+v", i, p.Char.Inv[i])
		}
	}
}

func TestOdinSecretStoneCorrectSequenceRoll95FailsAndConsumes(t *testing.T) {
	recipe := secretStoneRecipes[0]
	var runes [7]uint16
	copy(runes[:], recipe[:7])
	_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 95})
	if st.saves != 1 || p.Char.Gold != 0 {
		t.Fatalf("failed craft must persist cost: saves=%d gold=%d", st.saves, p.Char.Gold)
	}
	for i := 0; i < 7; i++ {
		if p.Char.Inv[i].Index != 0 {
			t.Fatalf("failed craft did not consume rune slot %d: %+v", i, p.Char.Inv[i])
		}
	}
}

func TestOdinRandomSevenRunesUses5PercentAndRandomSecretStone(t *testing.T) {
	// Valid runes, but neither one of the four Secret Stone sequences nor Fury.
	runes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}
	_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 0})
	if st.saves != 1 || p.Char.Gold != 0 {
		t.Fatalf("random craft did not persist cost: saves=%d gold=%d", st.saves, p.Char.Gold)
	}
	if p.Char.Inv[0].Index < 5334 || p.Char.Inv[0].Index > 5337 {
		t.Fatalf("random successful craft produced %d, want a Secret Stone", p.Char.Inv[0].Index)
	}
}

func TestOdinRandomSevenRunesRoll5FailsAndConsumes(t *testing.T) {
	runes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}
	_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 5})
	if st.saves != 1 || p.Char.Gold != 0 {
		t.Fatalf("random failure did not persist cost: saves=%d gold=%d", st.saves, p.Char.Gold)
	}
	for i := 0; i < 7; i++ {
		if p.Char.Inv[i].Index != 0 {
			t.Fatalf("random failure did not consume rune slot %d: %+v", i, p.Char.Inv[i])
		}
	}
}

func TestOdinSecretStoneInsufficientGoldDoesNotConsume(t *testing.T) {
	recipe := secretStoneRecipes[0]
	var runes [7]uint16
	copy(runes[:], recipe[:7])
	_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost-1, fixedRNG{value: 0})
	if st.saves != 0 || p.Char.Gold != odinSecretStoneGoldCost-1 {
		t.Fatalf("insufficient gold mutated state: saves=%d gold=%d", st.saves, p.Char.Gold)
	}
	for i, index := range runes {
		if p.Char.Inv[i].Index != index {
			t.Fatalf("insufficient gold consumed rune slot %d", i)
		}
	}
}

func TestOdinFurySequenceIsNotCapturedByRandomSecretStoneFallback(t *testing.T) {
	runes := [7]uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}
	_, p, st := odinSecretStoneAttempt(t, runes, 0, fixedRNG{value: 99})
	if st.saves != 1 || p.Char.Inv[0].Index != furyStoneItem {
		t.Fatalf("Fury recipe was shadowed by random-rune fallback: saves=%d result=%d", st.saves, p.Char.Inv[0].Index)
	}
}

func TestSubCelestialRequires100Fame(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	p.Char.Extended.Level = 120
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost - 1}
	before := cloneCharacterState(p.Char)

	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

	if st.saves != 1 { // only the initial Celestial creation
		t.Fatalf("Sub with insufficient Fame persisted: saves=%d", st.saves)
	}
	if p.Char.AlternateCelestial != nil || p.Char.Inv[0].Index != before.Inv[0].Index ||
		counterBalance(p, fameCounter) != subCelestialFameCost-1 {
		t.Fatalf("Sub with insufficient Fame mutated state")
	}
}

func TestSubCelestialSaveFailureRestoresFameAndItems(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	p.Char.Extended.Level = 120
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
	before := cloneCharacterState(p.Char)
	st.err = errors.New("postgres unavailable")

	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

	if st.saves != 2 {
		t.Fatalf("expected failed Sub persist attempt, saves=%d", st.saves)
	}
	if p.Char.AlternateCelestial != before.AlternateCelestial || p.Char.Inv != before.Inv ||
		p.Char.Equip != before.Equip || counterBalance(p, fameCounter) != subCelestialFameCost {
		t.Fatalf("Sub rollback did not restore character and Fame")
	}
}
