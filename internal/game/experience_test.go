package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestScaledMobExperienceAppliesMinimumThenRate(t *testing.T) {
	config := model.GameplayConfig{
		EXPMinimum: 10_000, EXPRatePercent: 200, PartyEXPBonusPercent: 2,
	}
	for _, test := range []struct {
		base uint32
		want uint32
	}{
		{base: 1_000, want: 20_000},
		{base: 5_000, want: 20_000},
		{base: 6_000, want: 20_000},
		{base: 12_000, want: 24_000},
	} {
		if got := scaledMobExperience(test.base, config); got != test.want {
			t.Fatalf("base=%d recompensa=%d, quer %d", test.base, got, test.want)
		}
	}
}

func TestScaledMobExperienceSupportsDisabledFloor(t *testing.T) {
	config := model.GameplayConfig{EXPRatePercent: 50}
	if got := scaledMobExperience(9_999, config); got != 4_999 {
		t.Fatalf("rate 50%%=%d, quer 4999", got)
	}
}

func TestScaledMobExperienceRateZeroDisablesReward(t *testing.T) {
	config := model.GameplayConfig{EXPMinimum: 10_000, EXPRatePercent: 0}
	if got := scaledMobExperience(1_000_000, config); got != 0 {
		t.Fatalf("rate zero concedeu %d EXP", got)
	}
}

func TestScaledMobExperienceSaturatesWithoutOverflow(t *testing.T) {
	config := model.GameplayConfig{EXPRatePercent: model.MaxEXPRatePercent}
	if got := scaledMobExperience(^uint32(0), config); got != ^uint32(0) {
		t.Fatalf("recompensa saturada=%d", got)
	}
}
