package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestHuntressTwoWeaponPercentBoundariesAndEligibility(t *testing.T) {
	const learned = uint32(1 << (82 - 72))
	for _, tc := range []struct {
		name    string
		class   uint8
		unique  int
		learned uint32
		trade   uint32
		attack  uint32
		want    uint32
	}{
		{"zero_trade", 3, 42, learned, 0, 2000, 2200},
		{"one_trade", 3, 42, learned, 1, 2000, 2201},
		{"nineteen_trade", 3, 42, learned, 19, 2000, 2219},
		{"twenty_trade", 3, 42, learned, 20, 2000, 2220},
		{"hundred_trade", 3, 42, learned, 100, 2000, 2300},
		{"before_cap", 3, 43, learned, 319, 2000, 2519},
		{"cap", 3, 43, learned, 320, 2000, 2520},
		{"over_cap", 3, 43, learned, 999, 2000, 2520},
		{"spirit_change", 3, 42, learned | 1<<(87-72), 320, 2000, 2520},
		{"capture_finisher", 3, 43, learned | 1<<(95-72), 320, 2000, 2520},
		{"not_learned", 3, 42, 0, 320, 2000, 2000},
		{"wrong_skill", 3, 42, 1 << (90 - 72), 320, 2000, 2000},
		{"wrong_weapon", 3, 7, learned, 320, 2000, 2000},
		{"wrong_class", 0, 42, learned, 320, 2000, 2000},
		{"zero_attack", 3, 42, learned, 320, 0, 0},
		{"fractional_result", 3, 42, learned, 1, 1001, 1101},
		{"score_cap", 3, 43, learned, 320, maxScoreValue, maxScoreValue},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w := &World{items: map[uint16]model.ItemDef{100: {Index: 100, Unique: tc.unique}}}
			ch := &model.Char{Class: tc.class, LearnedSkill: tc.learned,
				Score: testScore(model.Score{Attack: tc.attack, Mastery: [4]uint32{0, 0, tc.trade, 0}})}
			ch.Equip[6] = model.Item{Index: 100}
			ch.RuntimeScore = testScore(*ch.Score)
			w.applyPassiveSkills(ch)
			if got := playerAttack(ch); got != int(tc.want) {
				t.Fatalf("attack=%d, want %d", got, tc.want)
			}
			if ch.Score.Attack != tc.attack {
				t.Fatal("passive changed persisted attack")
			}
		})
	}
}

func TestHuntressTwoWeaponRecalculationEquipmentAndReload(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, Unique: 42, Pos: 64, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 100}}},
		101: {Index: 101, Unique: 7, Pos: 64, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 100}}},
		102: {Index: 102, Pos: 512, StaticEffects: []model.StaticEffect{{Name: "EF_SPECIAL3", Value: 120}}},
	}}
	ch := &model.Char{Class: 3, Score: testScore(model.Score{
		Level: 100, Str: 100, Dex: 100, Int: 10, Con: 50, Attack: 1000,
		Mastery: [4]uint32{0, 50, 200, 0},
	})}
	ch.Equip[6], ch.Equip[9] = model.Item{Index: 100}, model.Item{Index: 102}
	w.recalcPlayer(ch)
	base := playerAttack(ch)
	ch.LearnedSkill = 1 << (82 - 72)
	w.recalcPlayer(ch)
	want := base * 126 / 100
	if playerMastery(ch, 2) != 320 || playerAttack(ch) != want {
		t.Fatalf("equipped mastery=%d attack=%d, want 320/%d", playerMastery(ch, 2), playerAttack(ch), want)
	}
	for range 10 {
		w.recalcPlayer(ch)
		if playerAttack(ch) != want {
			t.Fatal("repeated recalculation stacked the passive")
		}
	}
	reloaded := *ch
	reloaded.RuntimeScore = nil
	w.recalcPlayer(&reloaded)
	if playerAttack(&reloaded) != want || reloaded.Score.Attack != 1000 {
		t.Fatal("reload changed the passive or persisted attack")
	}
	if got := w.baseSkillDamage(ch, model.SkillDef{Index: 95, InstanceType: 1}); got != want {
		t.Fatalf("Rapid Hit base=%d, want updated physical attack %d", got, want)
	}
	ch.Equip[9] = model.Item{}
	w.recalcPlayer(ch)
	if playerAttack(ch) != base*120/100 {
		t.Fatal("removing Trade equipment did not update the percentage")
	}
	ch.Equip[6] = model.Item{Index: 101}
	w.recalcPlayer(ch)
	if playerAttack(ch) != base {
		t.Fatal("changing to an ineligible weapon retained the percentage")
	}
	ch.Equip[6] = model.Item{Index: 100}
	ch.LearnedSkill = 0
	w.recalcPlayer(ch)
	if playerAttack(ch) != base {
		t.Fatal("forgetting the passive retained its percentage")
	}
}
