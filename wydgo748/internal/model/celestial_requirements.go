package model

import "strings"

// Saved Arch-to-Celestial requirements. Live transformation, trade, item
// consumption and persistence are still validated by the game operation.
// Levels are zero-based; the portal adds one for presentation.
const (
	CelestialMinimumArchLevel   = 355
	CelestialMinimumMortalLevel = 399
	CelestialIdealStoneItem     = 5338
)

type CelestialRequirements struct {
	Arch, Level, MortalLevel, IdealStone bool
}

func (r CelestialRequirements) Met() bool {
	return r.Arch && r.Level && r.MortalLevel && r.IdealStone
}

func EvaluateCelestialRequirements(evolution string, level, mortalLevel uint32, idealStone bool) CelestialRequirements {
	return CelestialRequirements{
		Arch:  strings.ToLower(strings.TrimSpace(evolution)) == "arch",
		Level: level >= CelestialMinimumArchLevel, MortalLevel: mortalLevel >= CelestialMinimumMortalLevel,
		IdealStone: idealStone,
	}
}
