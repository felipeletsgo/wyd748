package model

import "testing"

func TestDefaultGameplayConfig(t *testing.T) {
	config := DefaultGameplayConfig()
	if config.EXPMinimum != 10_000 || config.EXPRatePercent != 100 ||
		config.PartyEXPBonusPercent != 2 {
		t.Fatalf("configuracao global padrao incorreta: %+v", config)
	}
	if err := config.Validate(); err != nil {
		t.Fatal(err)
	}
}

func TestGameplayConfigRejectsInvalidRates(t *testing.T) {
	for _, config := range []GameplayConfig{
		{EXPRatePercent: MaxEXPRatePercent + 1},
		{EXPRatePercent: 100, PartyEXPBonusPercent: MaxPartyEXPBonusPercent + 1},
	} {
		if err := config.Validate(); err == nil {
			t.Fatalf("configuracao invalida aceita: %+v", config)
		}
	}
}
