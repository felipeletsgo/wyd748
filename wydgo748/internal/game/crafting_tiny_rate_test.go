package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestTinyChanceFromServer759Table(t *testing.T) {
	cases := []struct {
		name string
		id   uint16
		sanc int
		want int
	}{
		{"first tier +9", 2483, 9, 26},
		{"first tier +10", 2723, 10, 38},
		{"first tier +11", 2875, 11, 43},
		{"second tier +9", 2487, 9, 30},
		{"second tier +10", 2727, 10, 42},
		{"second tier +11", 2894, 11, 48},
		{"third tier +9", 3600, 9, 34},
		{"third tier +10", 3700, 10, 46},
		{"third tier +11", 3800, 11, 54},
		{"donor below +9", 2483, 8, 0},
		{"invalid donor", 2722, 9, 0},
		{"outside third tier", 3801, 9, 0},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if got := tinyChance(tc.id, tc.sanc); got != tc.want {
				t.Fatalf("donor=%d refinement=%d: got %d%%, want %d%%", tc.id, tc.sanc, got, tc.want)
			}
		})
	}
}

func TestTinyRecipeUsesDonorTierAndRefinement(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		2722: {Index: 2722, Grade: 5, Pos: 64},
		2723: {Index: 2723, Grade: 5, Pos: 64, StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}},
		2727: {Index: 2727, Grade: 5, Pos: 64},
		702:  {Index: 702, Grade: 5, Pos: 1},
	}}
	main, donor, catalyst := model.Item{Index: 2723}, model.Item{Index: 2727}, model.Item{Index: 702}
	setItemSanc(&main, 9)
	setItemSanc(&catalyst, 9)
	for _, tc := range []struct {
		sanc int
		want int
	}{
		{8, 0}, {9, 30}, {10, 42}, {11, 48}, {12, 48},
	} {
		setItemSanc(&donor, tc.sanc)
		req := combineRequest{}
		req.Items[0], req.Items[1], req.Items[2] = main, donor, catalyst
		result, chance, err := w.tinyRecipe(req)
		if tc.want == 0 {
			if err == nil {
				t.Fatalf("donor +%d should be rejected: result=%+v chance=%d", tc.sanc, result, chance)
			}
			continue
		}
		if err != nil || chance != tc.want || itemSanc(result) != 7 {
			t.Fatalf("donor +%d: result=%+v chance=%d error=%v; want +7 and %d%%", tc.sanc, result, chance, err, tc.want)
		}
	}

	setItemSanc(&main, 10)
	req := combineRequest{}
	req.Items[0], req.Items[1], req.Items[2] = main, donor, catalyst
	if _, _, err := w.tinyRecipe(req); err == nil {
		t.Fatal("main weapon above +9 was accepted")
	}
	setItemSanc(&main, 9)
	req.Items[0] = main
	req.Items[1].Index = 2722
	if _, _, err := w.tinyRecipe(req); err == nil {
		t.Fatal("unsupported donor ID was accepted")
	}
}
