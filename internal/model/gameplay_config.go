package model

import "fmt"

const (
	MaxEXPRatePercent       uint32 = 100_000
	MaxPartyEXPBonusPercent uint32 = 100
)

// GameplayConfig concentra regras globais de balanceamento. Os valores usam
// percentuais inteiros para manter os calculos deterministas.
type GameplayConfig struct {
	EXPMinimum           uint32
	EXPRatePercent       uint32
	PartyEXPBonusPercent uint32
}

func DefaultGameplayConfig() GameplayConfig {
	return GameplayConfig{
		EXPMinimum:           10_000,
		EXPRatePercent:       100,
		PartyEXPBonusPercent: 2,
	}
}

func (c GameplayConfig) Validate() error {
	if c.EXPRatePercent > MaxEXPRatePercent {
		return fmt.Errorf("exp_rate=%d fora do intervalo 0..%d",
			c.EXPRatePercent, MaxEXPRatePercent)
	}
	if c.PartyEXPBonusPercent > MaxPartyEXPBonusPercent {
		return fmt.Errorf("party_exp_bonus=%d excede %d",
			c.PartyEXPBonusPercent, MaxPartyEXPBonusPercent)
	}
	return nil
}
