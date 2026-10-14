package model

import "testing"

func TestArchRequirements(t *testing.T) {
	for _, evolution := range []string{"", " \t", "arch", "ARCH", "celestial", "subcelestial", "mortal", "unknown"} {
		for _, level := range []uint32{0, ArchMinimumLevel - 1, ArchMinimumLevel, 399, 4294967295} {
			for _, stone := range []uint16{0, ArchEternalStoneItem - 1, ArchEternalStoneItem, ArchEternalStoneItem + 1} {
				for _, sefirot := range []uint16{0, 1759, 1760, 1761, 1762, 1763, 1764, 65535} {
					for _, free := range []bool{false, true} {
						r := EvaluateArchRequirements(evolution, level, stone, sefirot, free)
						mortal := evolution == "" || evolution == " \t"
						validClass := sefirot >= 1760 && sefirot <= 1763
						if r.Mortal != mortal || r.Level != (level >= 370) || r.EternalStone != (stone == 1742) || r.Sefirot != validClass || r.FreeSlot != free || r.Met() != (mortal && level >= 370 && stone == 1742 && validClass && free) {
							t.Fatalf("incorrect gates: %q %d %d %d %t: %+v", evolution, level, stone, sefirot, free, r)
						}
						class, ok := ArchClassFromSefirot(sefirot)
						if ok != validClass || ok && class != int(sefirot)-1760 || !ok && class != 0 {
							t.Fatalf("incorrect Sefirot class: %d %d %t", sefirot, class, ok)
						}
					}
				}
			}
		}
	}
}
