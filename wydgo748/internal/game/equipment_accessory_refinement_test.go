package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestAccessoryPositionRefinementBonus(t *testing.T) {
	items := necklaceCatalog(t)
	ankh := items[663]
	if ankh.Pos != 2048 || staticAbility(ankh, "EF_CRITICAL") != 50 {
		t.Fatal("Glory Ankh catalog entry changed")
	}
	for _, tc := range []struct {
		name string
		item model.Item
		want int
	}{
		{"unrefined", model.Item{Index: 663}, 50},
		{"plus-eight", model.Item{Index: 663, Eff: [6]byte{43, 8}}, 90},
		{"plus-nine", model.Item{Index: 663, Eff: [6]byte{43, 9}}, 100},
		{"plus-ten", model.Item{Index: 663, Eff: [6]byte{43, 230}}, 110},
		{"plus-fifteen", model.Item{Index: 663, Eff: [6]byte{43, 250}}, 200},
	} {
		t.Run(tc.name, func(t *testing.T) {
			before := itemSanc(tc.item)
			if got := itemAbility(tc.item, ankh, "EF_CRITICAL"); got != tc.want {
				t.Fatalf("critical=%d, want %d", got, tc.want)
			}
			if itemSanc(tc.item) != before {
				t.Fatal("effective refinement changed stored refinement")
			}
		})
	}

	ordinary := model.ItemDef{Pos: 64, StaticEffects: []model.StaticEffect{{Name: "EF_CRITICAL", Value: 50}}}
	if got := itemAbility(model.Item{Eff: [6]byte{43, 9}}, ordinary, "EF_CRITICAL"); got != 95 {
		t.Fatalf("ordinary +9 critical=%d, want 95", got)
	}
}
