package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestSetItemBonusClampsDropBonusAndForcedGrade(t *testing.T) {
	for _, tc := range []struct {
		name      string
		dropBonus int
		want      [6]byte
	}{
		{"negative bonus", -8, [6]byte{itemBonusSanc, 0, itemBonusUnique, 0, itemBonusUnique, 55}},
		{"extra bonus capped at two", 40, [6]byte{itemBonusSanc, 0, itemBonusMagic, 8, itemBonusHP, 20}},
		{"forced grade above level table", 1105, [6]byte{itemBonusSanc, 0, itemBonusMagic, 12, itemBonusHP, 50}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			rng := &itemBonusSequenceRNG{t: t, values: []int{7, 0, 0, 0, 55, 55, 55}}
			w := itemBonusWorld(rng)
			item := model.Item{Index: 1000}
			w.setItemBonus(&item, model.ItemDef{Index: item.Index, Pos: 2}, 0, false, tc.dropBonus, false)
			if item.Eff != tc.want {
				t.Fatalf("eff=%v, want %v", item.Eff, tc.want)
			}
		})
	}
}

func TestSetItemBonusUsesReducedTierAndGradeRules(t *testing.T) {
	def := model.ItemDef{Index: 1000, Pos: 2, ReqLevel: 160}
	t.Run("non-reduced level is offset at 210", func(t *testing.T) {
		rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 99, 55, 99, 10, 55}}
		w := itemBonusWorld(rng)
		item := model.Item{Index: def.Index}
		w.setItemBonus(&item, def, 210, false, 0, false)
		if want := [6]byte{itemBonusSanc, 0, itemBonusUnique, 55, itemBonusUnique, 10}; item.Eff != want {
			t.Fatalf("eff=%v, want %v", item.Eff, want)
		}
	})

	t.Run("reduced rewards cap the tier and grade", func(t *testing.T) {
		rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 0, 0, 10}}
		w := itemBonusWorld(rng)
		item := model.Item{Index: def.Index}
		w.setItemBonus(&item, def, 260, true, 0, false)
		if want := [6]byte{itemBonusSanc, 2, itemBonusAttSpeed, 12, itemBonusHP, 40}; item.Eff != want {
			t.Fatalf("eff=%v, want %v", item.Eff, want)
		}
	})

	t.Run("reduced zero grades still produce a valid option", func(t *testing.T) {
		rng := &itemBonusSequenceRNG{t: t, values: []int{2, 0, 99, 99, 99, 10}}
		w := itemBonusWorld(rng)
		item := model.Item{Index: def.Index, Eff: [6]byte{}}
		w.setItemBonus(&item, model.ItemDef{Index: item.Index, Pos: 2}, 0, true, 0, false)
		if want := [6]byte{itemBonusSanc, 0, itemBonusUnique, 0, itemBonusHP, 10}; item.Eff != want {
			t.Fatalf("eff=%v, want %v", item.Eff, want)
		}
		for _, bound := range rng.bounds {
			if bound == 128 {
				t.Fatalf("reduced zero grade used the unique fallback: rolls=%v", rng.bounds)
			}
		}
	})

	t.Run("boots retain the native zero-valued first option", func(t *testing.T) {
		rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 99, 55, 99, 55, 99}}
		w := itemBonusWorld(rng)
		item := model.Item{Index: def.Index}
		w.setItemBonus(&item, model.ItemDef{Index: item.Index, Pos: 32}, 0, false, 0, false)
		if want := [6]byte{itemBonusSanc, 0, itemBonusDamage2, 0, itemBonusUnique, 99}; item.Eff != want {
			t.Fatalf("eff=%v, want %v", item.Eff, want)
		}
	})
}

func TestSetItemBonusAppliesStaticEffects(t *testing.T) {
	for _, tc := range []struct {
		name   string
		effect model.StaticEffect
		rolls  []int
		want   [6]byte
	}{
		{"sanc", model.StaticEffect{Name: "EF_SANC", Value: 5}, nil, [6]byte{itemBonusSanc, 5}},
		{"amount", model.StaticEffect{Name: "EF_AMOUNT", Value: 11}, nil, [6]byte{itemBonusAmount, 11}},
		{"incubate clamps", model.StaticEffect{Name: "EF_INCUBATE", Value: 8}, []int{3}, [6]byte{itemBonusIncubate, 9}},
		{"weekday", model.StaticEffect{Name: "EF_WDAY", Value: 4}, nil, [6]byte{itemBonusWDay, 4}},
		{"hour", model.StaticEffect{Name: "EF_HOUR", Value: 6}, nil, [6]byte{0, 0, itemBonusHour, 6}},
		{"minute", model.StaticEffect{Name: "EF_MIN", Value: 7}, nil, [6]byte{0, 0, 0, 0, itemBonusMin, 7}},
		{"unknown effect", model.StaticEffect{Name: "EF_UNKNOWN", Value: 9}, nil, [6]byte{}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			rng := &itemBonusSequenceRNG{t: t, values: tc.rolls}
			w := itemBonusWorld(rng)
			item := model.Item{Index: 1000}
			w.setItemBonus(&item, model.ItemDef{Index: item.Index, StaticEffects: []model.StaticEffect{tc.effect}}, 0, false, 0, false)
			if item.Eff != tc.want {
				t.Fatalf("eff=%v, want %v", item.Eff, tc.want)
			}
		})
	}
}

func TestSetItemBonusFillsUniqueItemOptionSlots(t *testing.T) {
	for _, tc := range []struct {
		name  string
		index uint16
		want  [6]byte
	}{
		{"first range start", 447, [6]byte{itemBonusUnique, 11, itemBonusUnique, 22, itemBonusUnique, 33}},
		{"first range end", 450, [6]byte{itemBonusUnique, 11, itemBonusUnique, 22, itemBonusUnique, 33}},
		{"second range start", 692, [6]byte{itemBonusUnique, 11, itemBonusUnique, 22, itemBonusUnique, 33}},
		{"second range end", 695, [6]byte{itemBonusUnique, 11, itemBonusUnique, 22, itemBonusUnique, 33}},
		{"standalone item", 753, [6]byte{itemBonusUnique, 11, itemBonusUnique, 22, itemBonusUnique, 33}},
		{"outside ranges", 446, [6]byte{}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			rng := &itemBonusSequenceRNG{t: t, values: []int{11, 22, 33}}
			w := itemBonusWorld(rng)
			item := model.Item{Index: tc.index}
			w.setItemBonus(&item, model.ItemDef{Index: tc.index}, 0, false, 0, false)
			if item.Eff != tc.want {
				t.Fatalf("eff=%v, want %v", item.Eff, tc.want)
			}
			if tc.index == 446 && len(rng.bounds) != 0 {
				t.Fatalf("unlisted item consumed rolls %v", rng.bounds)
			}
		})
	}
}

func TestSetItemBonusArmorMagicSelectsArmorClassOption(t *testing.T) {
	rng := &itemBonusSequenceRNG{t: t, values: []int{1, 0, 0, 0, 55, 55}}
	w := itemBonusWorld(rng)
	item := model.Item{Index: 1000}
	w.setItemBonus(&item, model.ItemDef{Index: item.Index, Pos: 4}, 0, false, 0, true)
	if item.Eff[2] != itemBonusAC || item.Eff[3] != 20 {
		t.Fatalf("armor-magic option=%v, want AC +20", item.Eff[2:4])
	}
}

func TestSetItemBonusClampsNegativeLevelDifference(t *testing.T) {
	rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 0, 55, 0, 55, 55}}
	w := itemBonusWorld(rng)
	item := model.Item{Index: 1000}
	w.setItemBonus(&item, model.ItemDef{Index: item.Index, Pos: 2}, -100, false, 0, false)
	if item.Eff[2] != itemBonusAttSpeed || item.Eff[3] != 12 {
		t.Fatalf("negative level was not clamped to grade-zero options: %v", item.Eff)
	}
}

func TestRollItemBonusPreservesExistingSlotZero(t *testing.T) {
	rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 0, 0, 55}}
	w := itemBonusWorld(rng)
	item := model.Item{Index: 1000, Eff: [6]byte{itemBonusSanc, 1}}
	w.rollItemBonusOptions(&item, model.ItemDef{Index: item.Index, Pos: 2}, 0, false, false, 0, 0, false)
	if item.Eff != [6]byte{itemBonusSanc, 1, itemBonusAttSpeed, 12, itemBonusUnique, 55} {
		t.Fatalf("item options=%v, slot zero was not preserved", item.Eff)
	}
	if len(rng.bounds) != 5 {
		t.Fatalf("slot zero was rerolled after it already existed: bounds=%v", rng.bounds)
	}
}
