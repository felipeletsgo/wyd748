package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestEquipmentGemBonusesFromSpecialRefinement(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, Pos: 2},
		101: {Index: 101, Pos: 4},
		102: {Index: 102, Pos: 8},
		103: {Index: 103, Pos: 16},
	}}
	ch := &model.Char{}
	ch.Equip[1] = model.Item{Index: 100, Eff: [6]byte{43, 230}} // Diamond +10
	ch.Equip[2] = model.Item{Index: 101, Eff: [6]byte{43, 235}} // Emerald +11
	ch.Equip[3] = model.Item{Index: 102, Eff: [6]byte{43, 240}} // Coral +12
	ch.Equip[4] = model.Item{Index: 103, Eff: [6]byte{43, 249}} // Garnet +14

	got := w.equipmentGemBonuses(ch)
	if got.dropPercent != 8 || got.expPercent != 2 || got.forceDamage != 80 ||
		got.absorbDamage != 200 {
		t.Fatalf("incorrect gem bonuses: %+v", got)
	}
}

func TestAncientGemBonusesBelowPlusTen(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		200: {Index: 200, Pos: 64, Grade: 5},
		201: {Index: 201, Pos: 64, Grade: 6},
		202: {Index: 202, Pos: 64, Grade: 7},
		203: {Index: 203, Pos: 64, Grade: 8},
	}}
	ch := &model.Char{}
	ch.Equip[1] = model.Item{Index: 200, Eff: [6]byte{43, 7}}
	ch.Equip[2] = model.Item{Index: 201, Eff: [6]byte{43, 7}}
	ch.Equip[3] = model.Item{Index: 202, Eff: [6]byte{43, 7}}
	ch.Equip[4] = model.Item{Index: 203, Eff: [6]byte{43, 7}}

	got := w.equipmentGemBonuses(ch)
	if got.dropPercent != 8 || got.expPercent != 2 || got.forceDamage != 20 ||
		got.absorbDamage != 20 {
		t.Fatalf("incorrect Ancient bonuses: %+v", got)
	}
}

func TestEquipmentGemBonusesIncludeNativeNecklaceSlot(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		640: {Index: 640, Grade: 6},
	}}
	ch := &model.Char{}
	ch.Equip[9] = model.Item{Index: 640, Eff: [6]byte{43, 234}}
	if got := w.equipmentGemBonuses(ch); got != (equipmentGemBonus{dropPercent: 8, forceDamage: 20}) {
		t.Fatalf("necklace gem bonus=%+v, want diamond drop bonus", got)
	}
}

// The official server clears the BM bit-17 absorption before gems, so the
// W2PP BM passive absorption is not applied.
func TestBeastMasterPassivesAddNoFlatDamageAbsorb(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{100: {Index: 100, Pos: 128}}}
	ch := &model.Char{Class: 2, Score: testScore(model.Score{Mastery: [4]uint32{0, 0, 59, 59}})}
	ch.RuntimeScore = testScore(*ch.Score)
	ch.Equip[7] = model.Item{Index: 100}
	for _, skill := range []int{65, 71} {
		ch.LearnedSkill = 1 << (skill - 48)
		if got := w.playerFlatDamageAbsorb(ch); got != 0 {
			t.Fatalf("skill %d absorb=%d, want 0", skill, got)
		}
	}
}

func TestGemDamageAbsorptionAndRewardHelpers(t *testing.T) {
	if got := addFlatDamage(100, 40); got != 140 {
		t.Fatalf("piercing=%d", got)
	}
	if got := absorbFlatDamage(100, 40); got != 60 {
		t.Fatalf("absorption=%d", got)
	}
	if got := absorbFlatDamage(20, 40); got != 1 {
		t.Fatalf("absorption should preserve minimum damage, got %d", got)
	}
	if got := applyPercentReward(10_000, 2); got != 10_200 {
		t.Fatalf("EXP Coral=%d", got)
	}
	if got := applyPercentReward(^uint32(0), 8); got != ^uint32(0) {
		t.Fatalf("reward clamp=%d", got)
	}
}
