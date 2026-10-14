package model

import "testing"

func TestCelestialSavedRequirements(t *testing.T) {
	for _, evolution := range []string{"", "mortal", "arch", " ARCH ", "celestial", "subcelestial", "unknown"} {
		for _, level := range []uint32{0, 354, 355, 356, 399, ^uint32(0)} {
			for _, mortal := range []uint32{0, 398, 399, ^uint32(0)} {
				for _, stone := range []bool{false, true} {
					r := EvaluateCelestialRequirements(evolution, level, mortal, stone)
					arch := evolution == "arch" || evolution == " ARCH "
					if r.Arch != arch || r.Level != (level >= 355) || r.MortalLevel != (mortal >= 399) || r.IdealStone != stone ||
						r.Met() != (arch && level >= 355 && mortal >= 399 && stone) {
						t.Fatalf("requirements for %q/%d/%d/%t: %+v", evolution, level, mortal, stone, r)
					}
				}
			}
		}
	}
}
