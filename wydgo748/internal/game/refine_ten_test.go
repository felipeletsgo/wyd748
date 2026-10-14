package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

// Lac Dust refines up to +9 by the rate table and a +10 item to +11 at
// RefineTenChance. +9 -> +10 stays exclusive to the Aylin composition.
func TestRefinePowderChanceAllowsOnlyTenToEleven(t *testing.T) {
	lac := model.VolatileRule{Action: "refine", RefineMax: 9, RefineTenChance: 5}
	for _, tc := range []struct {
		level   int
		chance  int
		allowed bool
	}{
		{8, refineChance(8), true},
		{9, 0, false},
		{10, 5, true},
		{11, 0, false},
	} {
		chance, allowed := refinePowderChance(tc.level, lac)
		if chance != tc.chance || allowed != tc.allowed {
			t.Fatalf("+%d: chance=%d allowed=%t, want %d %t", tc.level, chance, allowed, tc.chance, tc.allowed)
		}
	}
	ori := model.VolatileRule{Action: "refine", RefineMax: 6}
	if _, allowed := refinePowderChance(10, ori); allowed {
		t.Fatal("a powder without refineTenChance refined a +10 item")
	}
}

// Raising +10 to +11 keeps the Diamond/Emerald/Coral/Garnet variant.
func TestSetItemSancKeepsGemVariant(t *testing.T) {
	for variant := 0; variant < 4; variant++ {
		item := model.Item{Index: 811, Eff: [6]byte{43, byte(230 + variant)}}
		if !setItemSanc(&item, 11) {
			t.Fatal("refinement was not written")
		}
		if got := int(item.Eff[1]); got != 234+variant || itemSanc(item) != 11 {
			t.Fatalf("variant %d became raw %d (+%d)", variant, got, itemSanc(item))
		}
	}
	plain := model.Item{Index: 811, Eff: [6]byte{43, 9}}
	if !setItemSanc(&plain, 10) || plain.Eff[1] != 230 {
		t.Fatalf("+9 -> +10 raw = %d, want 230", plain.Eff[1])
	}
}

func TestLacRefinesTenToElevenThroughUseItem(t *testing.T) {
	for _, tc := range []struct {
		name    string
		rng     int
		wantRaw byte
	}{{"success", 0, 237}, {"failure", 99, 233}} {
		t.Run(tc.name, func(t *testing.T) {
			rule := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9, RefineTenChance: 5}
			w, p, _ := useItemWorld(rule)
			w.rng = fixedRNG{value: tc.rng}
			w.items[811] = model.ItemDef{Index: 811, Pos: 64}
			p.Char.Inv[0] = model.Item{Index: 100}
			p.Char.Equip[6] = model.Item{Index: 811, Eff: [6]byte{43, 233}} // +10 Garnet
			pkt := useItemPacket(0, 6)
			binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)

			w.onUseItem(p.Session, pkt)

			if p.Char.Equip[6].Eff[1] != tc.wantRaw || p.Char.Inv[0].Index != 0 {
				t.Fatalf("raw=%d powder=%+v, want raw %d and the powder consumed",
					p.Char.Equip[6].Eff[1], p.Char.Inv[0], tc.wantRaw)
			}
		})
	}

	rule := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9, RefineTenChance: 5}
	w, p, _ := useItemWorld(rule)
	w.rng = fixedRNG{value: 0}
	w.items[811] = model.ItemDef{Index: 811, Pos: 64}
	p.Char.Inv[0] = model.Item{Index: 100}
	p.Char.Equip[6] = model.Item{Index: 811, Eff: [6]byte{43, 9}}
	pkt := useItemPacket(0, 6)
	binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)
	w.onUseItem(p.Session, pkt)
	if p.Char.Equip[6].Eff[1] != 9 || p.Char.Inv[0].Index != 100 {
		t.Fatal("Lac refined +9 to +10 outside the Aylin composition")
	}
}
