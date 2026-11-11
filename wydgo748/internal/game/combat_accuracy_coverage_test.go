package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestCombatAccuracyAndEvasionClamps(t *testing.T) {
	for _, tc := range []struct {
		name                   string
		dex, accuracy, evasion int
		concentration          bool
		want                   int
	}{
		{name: "base chance", want: 100},
		{name: "dex evasion", dex: 2000, want: 60},
		{name: "maximum dex evasion", dex: 4000, want: 20},
		{name: "dex is capped", dex: 5000, want: 20},
		{name: "negative dex is clamped", dex: -100, want: 100},
		{name: "negative bonuses are ignored", accuracy: -100, evasion: -100, want: 100},
		{name: "accuracy bonus", dex: 4000, accuracy: 200, want: 40},
		{name: "evasion bonus", evasion: 200, want: 80},
		{name: "evasion bonus is capped", dex: 4000, evasion: 5000, want: 20},
		{name: "minimum chance", dex: 4000, want: 20},
		{name: "concentration floor", dex: 4000, concentration: true, want: 35},
		{name: "concentration bonus", dex: 2000, concentration: true, want: 75},
		{name: "accuracy caps chance", accuracy: 1000, want: 100},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if got := combatAccuracyPercent(tc.dex, tc.accuracy, tc.evasion, tc.concentration); got != tc.want {
				t.Fatalf("combatAccuracyPercent = %d, want %d", got, tc.want)
			}
		})
	}
	for _, tc := range []struct {
		dex, evasion, want int
	}{
		{dex: -1, evasion: -10, want: 0},
		{dex: 2000, want: 40},
		{dex: 9000, want: 80},
		{dex: 0, evasion: 2000, want: 80},
	} {
		if got := combatEvasionRatingPercent(tc.dex, tc.evasion); got != tc.want {
			t.Errorf("combatEvasionRatingPercent(%d,%d) = %d, want %d", tc.dex, tc.evasion, got, tc.want)
		}
	}
	if got := combatEvasionPercent(4000, 0, 0, false); got != 80 {
		t.Fatalf("maximum combat evasion = %d, want 80", got)
	}
	if got := combatEvasionPercent(4000, 0, 1000, false); got != 0 {
		t.Fatalf("accuracy bonus did not remove evasion: %d", got)
	}
}

func TestCombatRollHitsClampsChanceAndUsesPercentRange(t *testing.T) {
	if combatRollHits(100, nil) {
		t.Fatal("nil random source produced a hit")
	}
	for _, tc := range []struct {
		name   string
		chance int
		roll   int
		want   bool
	}{
		{name: "zero chance", chance: 0, roll: 0},
		{name: "negative chance", chance: -1, roll: 0},
		{name: "chance succeeds", chance: 50, roll: 49, want: true},
		{name: "chance boundary fails", chance: 50, roll: 50},
		{name: "chance above one hundred", chance: 101, roll: 99, want: true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			called := false
			got := combatRollHits(tc.chance, func(n int) int {
				called = true
				if n != 100 {
					t.Fatalf("random range = %d, want 100", n)
				}
				return tc.roll
			})
			if !called || got != tc.want {
				t.Fatalf("combatRollHits = %v, called=%v, want %v", got, called, tc.want)
			}
		})
	}
}

func TestPlayerAndMobAccuracyUseTheirEffectiveScores(t *testing.T) {
	if playerAccuracyBonusPoints(nil) != 0 || playerEvasionBonusPoints(nil) != 0 || playerVersusPlayerAccuracy(nil, &model.Char{}) != 0 ||
		playerVersusPlayerAccuracy(&model.Char{}, nil) != 0 || playerVersusMobAccuracy(nil, &model.NPCDef{}) != 0 ||
		playerVersusMobAccuracy(&model.Char{}, nil) != 0 || mobVersusPlayerAccuracy(nil, &model.Char{}) != 0 ||
		mobVersusPlayerAccuracy(&model.NPCDef{}, nil) != 0 {
		t.Fatal("accuracy helpers returned values for missing participants")
	}

	attacker := &model.Char{Class: 1, LearnedSkill: uint32(1) << 23,
		Score: &model.Score{Version: model.ScoreVersion, Accuracy: 200}}
	attacker.RuntimeScore = &model.Score{Version: model.ScoreVersion, Accuracy: 0}
	defender := &model.Char{Score: &model.Score{Version: model.ScoreVersion, Dex: 4000}}
	if playerAccuracyBonusPoints(attacker) != 0 || playerEvasionBonusPoints(defender) != 0 {
		t.Fatal("runtime score was not preferred for combat bonuses")
	}
	if !playerHasConcentration(attacker) || playerHasConcentration(nil) || playerHasConcentration(&model.Char{Class: 0, LearnedSkill: uint32(1) << 23}) {
		t.Fatal("Concentration eligibility is incorrect")
	}
	if got := playerVersusPlayerAccuracy(attacker, defender); got != 35 {
		t.Fatalf("concentrated PvP accuracy = %d, want 35", got)
	}
	if got := offensiveSkillPvPAccuracy(attacker, defender, 22); got != 81 {
		t.Fatalf("Explosion Bash accuracy = %d, want 81", got)
	}
	if got := offensiveSkillPvPAccuracy(attacker, defender, 21); got != 35 {
		t.Fatalf("ordinary skill accuracy = %d, want 35", got)
	}
	attacker.RuntimeScore = nil
	if playerAccuracyBonusPoints(attacker) != 200 {
		t.Fatal("base score accuracy bonus was not used")
	}

	mob := &model.NPCDef{Score: &model.Score{Dex: 4000}}
	if got := playerVersusMobAccuracy(attacker, mob); got != 55 {
		t.Fatalf("player versus mob accuracy = %d, want 55", got)
	}
	mobAttacker := &model.NPCDef{Score: &model.Score{Accuracy: 200}}
	if got := mobVersusPlayerAccuracy(mobAttacker, defender); got != 40 {
		t.Fatalf("mob versus player accuracy = %d, want 40", got)
	}
}
