package game

import "testing"

func TestOreAcceptsUniqueCoversNativeDSetFamilies(t *testing.T) {
	for _, test := range []struct {
		variant int
		allowed []int
	}{
		{variant: 0, allowed: []int{5, 14, 24, 34}},
		{variant: 1, allowed: []int{6, 15, 25, 35}},
		{variant: 2, allowed: []int{7, 16, 26, 36}},
		{variant: 3, allowed: []int{8, 17, 27, 37}},
	} {
		for _, unique := range test.allowed {
			if !oreAcceptsUnique(test.variant, unique) {
				t.Errorf("variant %d rejected allowed unique %d", test.variant, unique)
			}
		}
		for _, unique := range []int{0, 1, 10, 20, 30, 40, 99} {
			if oreAcceptsUnique(test.variant, unique) {
				t.Errorf("variant %d accepted unrelated unique %d", test.variant, unique)
			}
		}
	}
	for _, variant := range []int{-1, 4} {
		if oreAcceptsUnique(variant, 5) {
			t.Errorf("invalid variant %d accepted a unique", variant)
		}
	}
}
