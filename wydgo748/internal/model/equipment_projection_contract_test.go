package model

import "testing"

func TestDyedRefinementPreservesCompactAppearance(t *testing.T) {
	for raw := 230; raw <= 253; raw++ {
		for dye := 116; dye <= 125; dye++ {
			item := Item{Index: 100, Eff: [6]byte{byte(dye), byte(raw)}}
			visual, ancient := VisualItemCode(item, false), AncientCode(item)
			if int(visual>>12) != (raw-230)/4+10 || int(ancient&15)+115 != dye || int(ancient>>4) != (raw-230)%4+1 {
				t.Fatalf("raw=%d dye=%d visual=%x ancient=%x", raw, dye, visual, ancient)
			}
		}
	}
}
