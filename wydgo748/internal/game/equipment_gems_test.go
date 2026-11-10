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
	if got.dropPercent != 8 || got.expPercent != 2 || got.forceDamage != 40 ||
		got.absorbDamage != 40 {
		t.Fatalf("incorrect Ancient bonuses: %+v", got)
	}
}

func TestEquipmentGemBonusesIncludeNativeNecklaceSlot(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		640: {Index: 640, Grade: 6},
	}}
	ch := &model.Char{}
	ch.Equip[9] = model.Item{Index: 640, Eff: [6]byte{43, 234}}
	if got := w.equipmentGemBonuses(ch); got != (equipmentGemBonus{dropPercent: 8, forceDamage: 40}) {
		t.Fatalf("necklace gem bonus=%+v, want diamond drop bonus", got)
	}
}

func TestKRAncientWeaponsStackCatalogAndGemBonuses(t *testing.T) {
	w := &World{items: reportTestCatalog(t).Items}
	p := &Player{Char: &model.Char{}}
	p.Char.Equip[6] = model.Item{Index: 3702, Eff: [6]byte{43, 231}} // Emerald Karicass +10
	if got := w.equipmentGemBonuses(p.Char).forceDamage; got != 140 {
		t.Fatalf("Karicass piercing=%d, want 60 catalog + 40 Ancient + 40 refinement", got)
	}
	p.Char.Equip[6] = model.Item{Index: 3644, Eff: [6]byte{43, 253}} // Garnet Cronus Saver +15
	if got := w.weaponCriticalDamage(p); got != 37 {
		t.Fatalf("Cronus Saver critical damage=%d, want 37", got)
	}
	if got := w.equipmentGemBonuses(p.Char).absorbDamage; got != 280 {
		t.Fatalf("Cronus Saver absorption=%d, want 40 Ancient + 240 refinement", got)
	}
	p.Char.Score = testScore(model.Score{Attack: 1000, Accuracy: 1000})
	target := &Player{Char: &model.Char{Score: testScore(model.Score{})}}
	roll := fixedRNG{value: 0}.Intn
	for _, hit := range []struct {
		critical bool
		attack   int
	}{{false, 1000}, {true, 1670}} {
		got := playerPhysicalFollowupHitPlayerWithRNG(p, target, false, hit.critical, w.weaponCriticalDamage(p), roll)
		want := uint32(hitDamageWithRNG(hit.attack, 0, 0, roll))
		if !got.Hit || got.Damage != want {
			t.Fatalf("critical=%t: hit=%+v, want %d damage", hit.critical, got, want)
		}
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
