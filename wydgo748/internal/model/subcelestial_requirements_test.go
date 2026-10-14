package model

import "testing"

func TestSubCelestialSavedRequirements(t *testing.T) {
	for _, evolution := range []string{"celestial", " Celestial ", "", "mortal", "arch", "subcelestial", "unknown"} {
		for _, level := range []uint32{0, 119, 120, 121, ^uint32(0)} {
			for _, fame := range []uint32{0, 99, 100, 101, ^uint32(0)} {
				for _, sefirot := range []uint16{0, 1759, 1760, 1761, 1762, 1763, 1764} {
					r := EvaluateSubCelestialRequirements(evolution, level, fame, sefirot, true, true, true, true)
					want := (evolution == "celestial" || evolution == " Celestial ") && level >= 120 && fame >= 100 && sefirot >= 1760 && sefirot <= 1763
					if r.Met() != want {
						t.Fatalf("%q level=%d fame=%d sefirot=%d: %+v", evolution, level, fame, sefirot, r)
					}
				}
			}
		}
	}
	for mask := 0; mask < 16; mask++ {
		r := EvaluateSubCelestialRequirements("celestial", 120, 100, 1763, mask&1 != 0, mask&2 != 0, mask&4 != 0, mask&8 != 0)
		if r.Met() != (mask == 15) {
			t.Fatalf("missing saved gate passed: %d %+v", mask, r)
		}
	}
}

func TestIdealInventoryRewardSpaceMatchesConsumption(t *testing.T) {
	for _, effectSlot := range []int{0, 2, 4} {
		for _, amount := range []byte{0, 1, 2, 255} {
			items := make([]Item, 64)
			for slot := range items {
				items[slot] = Item{Index: 4148}
			}
			items[62] = Item{Index: 5338}
			items[62].Eff[effectSlot] = 61
			items[62].Eff[effectSlot+1] = amount
			items[63] = Item{}
			ideal, space := SavedIdealInventoryRequirements(items)
			if !ideal || space != (amount <= 1) {
				t.Fatalf("amount=%d effect=%d: ideal=%v space=%v", amount, effectSlot, ideal, space)
			}
			items[0] = Item{}
			if _, space = SavedIdealInventoryRequirements(items); !space {
				t.Fatal("empty visible slot rejected")
			}
		}
	}
	items := make([]Item, 64)
	for slot := range items {
		items[slot] = Item{Index: 4148}
	}
	items[63] = Item{Index: 5338}
	if ideal, space := SavedIdealInventoryRequirements(items); ideal || space {
		t.Fatal("reserved Ideal counted")
	}
	items[62] = Item{Index: 5338}
	if ideal, space := SavedIdealInventoryRequirements(items); !ideal || !space {
		t.Fatal("unstacked Ideal did not free its slot")
	}
	items[62].Eff = [6]byte{61, 2, 61, 1}
	if _, space := SavedIdealInventoryRequirements(items); space {
		t.Fatal("second amount effect overrode first")
	}
}
