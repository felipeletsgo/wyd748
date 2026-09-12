package model

import "fmt"

const (
	MaxEXPRatePercent       uint32 = 100_000
	MaxPartyEXPBonusPercent uint32 = 100

	// CompositorRefineLevels sao os tres refinos que o Compositor aceita no
	// material: +7, +8 e +9. O indice do bonus e refino - CompositorMinRefine.
	CompositorRefineLevels = 3
	CompositorMinRefine    = 7
)

// GameplayConfig concentra regras globais de balanceamento. Os valores usam
// percentuais inteiros para manter os calculos deterministas.
type GameplayConfig struct {
	EXPMinimum           uint32
	EXPRatePercent       uint32
	PartyEXPBonusPercent uint32

	// Compositor: a chance de sucesso e CompositorBaseChance mais o bonus de
	// cada um dos quatro materiais, escolhido pelo refino do material. O teto
	// com os padroes e 10 + 4*8 = 42%.
	CompositorBaseChance   uint32
	CompositorRefineChance [CompositorRefineLevels]uint32
}

func DefaultGameplayConfig() GameplayConfig {
	return GameplayConfig{
		EXPMinimum:             10_000,
		EXPRatePercent:         100,
		PartyEXPBonusPercent:   2,
		CompositorBaseChance:   10,
		CompositorRefineChance: [CompositorRefineLevels]uint32{3, 5, 8},
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
	if c.CompositorBaseChance > 100 {
		return fmt.Errorf("compositor_base=%d excede 100", c.CompositorBaseChance)
	}
	for i, bonus := range c.CompositorRefineChance {
		if bonus > 100 {
			return fmt.Errorf("compositor_refine_%d=%d excede 100",
				i+CompositorMinRefine, bonus)
		}
	}
	return nil
}
