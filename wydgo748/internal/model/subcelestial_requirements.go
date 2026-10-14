package model

import "strings"

const (
	SubCelestialMinimumLevel = 120
	SubCelestialFameCost     = uint32(100)
)

// Saved requirements only: live reservations, materialization and persistence
// remain authoritative game validations. Levels are zero-based.
type SubCelestialRequirements struct {
	Celestial, Level, Fame, Sefirot, IdealStone, NoAlternate, Face, RewardSpace bool
}

func (r SubCelestialRequirements) Met() bool {
	return r.Celestial && r.Level && r.Fame && r.Sefirot && r.IdealStone && r.NoAlternate && r.Face && r.RewardSpace
}

func EvaluateSubCelestialRequirements(evolution string, level, fame uint32, sefirot uint16, ideal, noAlternate, face, space bool) SubCelestialRequirements {
	_, validSefirot := ArchClassFromSefirot(sefirot)
	return SubCelestialRequirements{
		Celestial: strings.ToLower(strings.TrimSpace(evolution)) == "celestial",
		Level:     level >= SubCelestialMinimumLevel, Fame: fame >= SubCelestialFameCost,
		Sefirot: validSefirot, IdealStone: ideal, NoAlternate: noAlternate, Face: face, RewardSpace: space,
	}
}

// The game consumes one Ideal Stone, then places the reward in an empty slot;
// it does not merge this reward into an existing Mysterious Stone stack.
func SavedIdealInventoryRequirements(items []Item) (ideal, rewardSpace bool) {
	for slot, item := range items {
		if slot >= PlayerCarrySlots {
			break
		}
		if item.Index == 0 {
			rewardSpace = true
		}
		if item.Index != CelestialIdealStoneItem {
			continue
		}
		ideal = true
		amount := byte(1)
		for effect := 0; effect < 3; effect++ {
			if item.Eff[effect*2] == 61 {
				amount = item.Eff[effect*2+1]
				break
			}
		}
		rewardSpace = rewardSpace || amount <= 1
	}
	return ideal, rewardSpace
}
