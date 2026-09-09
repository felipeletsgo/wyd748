package model

import "testing"

func TestDefaultGameplayConfig(t *testing.T) {
	config := DefaultGameplayConfig()
	if config.EXPMinimum != 10_000 || config.EXPRatePercent != 100 ||
		config.PartyEXPBonusPercent != 2 {
		t.Fatalf("configuracao global padrao incorreta: %+v", config)
	}
	// Compositor: base 10 e bonus por refino {+7:3, +8:5, +9:8}. Com quatro
	// materiais o teto e 10+8*4 = 42.
	if config.CompositorBaseChance != 10 {
		t.Errorf("compositor base=%d, quer 10", config.CompositorBaseChance)
	}
	if want := [CompositorRefineLevels]uint32{3, 5, 8}; config.CompositorRefineChance != want {
		t.Errorf("compositor bonus=%v, quer %v", config.CompositorRefineChance, want)
	}
	ceiling := config.CompositorBaseChance + 4*config.CompositorRefineChance[2]
	if ceiling != 42 {
		t.Errorf("teto=%d, quer 42", ceiling)
	}
	if err := config.Validate(); err != nil {
		t.Fatal(err)
	}
}

func TestGameplayConfigRejectsInvalidRates(t *testing.T) {
	for _, config := range []GameplayConfig{
		{EXPRatePercent: MaxEXPRatePercent + 1},
		{EXPRatePercent: 100, PartyEXPBonusPercent: MaxPartyEXPBonusPercent + 1},
		{CompositorBaseChance: 101},
		{CompositorRefineChance: [CompositorRefineLevels]uint32{0, 0, 101}},
	} {
		if err := config.Validate(); err == nil {
			t.Fatalf("configuracao invalida aceita: %+v", config)
		}
	}
}
