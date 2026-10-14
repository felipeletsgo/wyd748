package model

import "strings"

// Arch gates are shared by the authoritative king interaction and the saved
// website advisor. Levels are internal (zero-based), not displayed levels.
const (
	ArchMinimumLevel     = 370
	ArchEternalStoneItem = 1742
	ArchFirstSefirotItem = 1760
	ArchEternalStoneSlot = 10
	ArchSefirotSlot      = 11
	ArchCharacterSlots   = 4
)

type ArchRequirements struct {
	Mortal, Level, EternalStone, Sefirot, FreeSlot bool
}

func (r ArchRequirements) Met() bool {
	return r.Mortal && r.Level && r.EternalStone && r.Sefirot && r.FreeSlot
}

// The input is the persisted evolution value: an empty value means Mortal.
// In particular, an unknown/nonempty value must not pass a Mortal-only gate.
func EvaluateArchRequirements(evolution string, level uint32, stone, sefirot uint16, freeSlot bool) ArchRequirements {
	_, validSefirot := ArchClassFromSefirot(sefirot)
	return ArchRequirements{
		Mortal: strings.TrimSpace(evolution) == "", Level: level >= ArchMinimumLevel,
		EternalStone: stone == ArchEternalStoneItem, Sefirot: validSefirot, FreeSlot: freeSlot,
	}
}

func ArchClassFromSefirot(itemID uint16) (int, bool) {
	class := int(itemID) - ArchFirstSefirotItem
	if class < 0 || class > 3 {
		return 0, false
	}
	return class, true
}
