package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"testing"

	"wydgo/internal/model"
)

// W2PP "Refinar brincos": Lac on the equipped earring at +9/+10 succeeds on
// 16 of 100 rolls and destroys the earring on failure.
func TestEarringLacRefinementToTen(t *testing.T) {
	for index := uint16(591); index <= 595; index++ {
		for _, tc := range []struct {
			name        string
			rng         int
			saveFailure bool
			wantLevel   int
			wantPowder  uint16
		}{
			{"success", 0, false, 10, 0},
			{"last-success-roll", 15, false, 10, 0},
			{"first-failure-roll", 16, false, 0, 0},
			{"failure", 99, false, 0, 0},
			{"save-failure", 0, true, 9, 413},
		} {
			t.Run(fmt.Sprintf("%d/%s", index, tc.name), func(t *testing.T) {
				rule := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9, RefineTenChance: 5}
				w, p, st := useItemWorld(rule)
				w.rng = fixedRNG{value: tc.rng}
				w.items[413] = model.ItemDef{Index: 413}
				w.volatiles.Items[413], w.volatiles.ItemCodes[413] = rule, 5
				w.items[index] = model.ItemDef{Index: index, Pos: 256}
				p.Char.Inv[0] = model.Item{Index: 413, UID: "lac-unit"}
				p.Char.Equip[8] = model.Item{Index: index, UID: "earring-unit", Eff: [6]byte{43, 9, 2, 20, 60, 10}}
				before := p.Char.Equip[8]
				if tc.saveFailure {
					st.err = errors.New("refinement storage failure")
				}
				packet := useItemPacket(0, 8)
				binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
				w.onUseItem(p.Session, packet)
				after := p.Char.Equip[8]
				if tc.wantLevel == 0 {
					if after != (model.Item{}) || p.Char.Inv[0].Index != 0 || st.saves != 1 {
						t.Fatalf("failed earring was not destroyed: earring=%+v powder=%+v saves=%d", after, p.Char.Inv[0], st.saves)
					}
					return
				}
				if itemSanc(after) != tc.wantLevel || after.Index != index || after.UID != before.UID ||
					after.Eff[2] != 2 || after.Eff[3] != 20 || after.Eff[4] != 60 || after.Eff[5] != 10 ||
					p.Char.Inv[0].Index != tc.wantPowder || st.saves != 1 {
					t.Fatalf("earring=%+v powder=%+v saves=%d", after, p.Char.Inv[0], st.saves)
				}
				if tc.wantPowder == 0 {
					w.onUseItem(p.Session, packet)
					if st.saves != 1 || p.Char.Equip[8] != after {
						t.Fatal("consumed powder replayed")
					}
				}
			})
		}
	}
}

func TestEarringPowderExceptionIsNarrow(t *testing.T) {
	lac := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9, RefineTenChance: 5}
	for _, tc := range []struct {
		name              string
		level, code, mask int
		targetType, slot  uint32
		wantAllowed       bool
		wantChance        int
	}{
		{"earring", 9, 5, 256, placeEquip, 8, true, 16},
		{"weapon", 9, 5, 64, placeEquip, 6, false, 0},
		{"armor", 9, 5, 4, placeEquip, 2, false, 0},
		{"necklace", 9, 5, 512, placeEquip, 9, false, 0},
		{"wrong-slot", 9, 5, 256, placeEquip, 9, false, 0},
		{"wrong-mask", 9, 5, 512, placeEquip, 8, false, 0},
		{"inventory", 9, 5, 256, placeInv, 8, false, 0},
		{"cargo", 9, 5, 256, 2, 8, false, 0},
		{"ori", 9, 4, 256, placeEquip, 8, false, 0},
		{"unregistered", 9, 0, 256, placeEquip, 8, false, 0},
		{"ten-earring", 10, 5, 256, placeEquip, 8, true, 16},
		{"ten-weapon-keeps-existing-rule", 10, 5, 64, placeEquip, 6, true, 5},
		{"eleven-not-extended", 11, 5, 256, placeEquip, 8, false, 0},
	} {
		t.Run(tc.name, func(t *testing.T) {
			item := model.Item{Index: 591}
			setItemSanc(&item, tc.level)
			chance, allowed := equipmentPowderChance(item, model.ItemDef{Pos: tc.mask}, tc.targetType, tc.slot, tc.code, lac)
			if chance != tc.wantChance || allowed != tc.wantAllowed {
				t.Fatalf("chance=%d allowed=%t", chance, allowed)
			}
		})
	}
}

func TestEarringLacRefinementTenToEleven(t *testing.T) {
	for _, tc := range []struct {
		name      string
		rng       int
		wantLevel int
	}{{"success", 0, 11}, {"failure-breaks", 16, 0}} {
		t.Run(tc.name, func(t *testing.T) {
			rule := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9, RefineTenChance: 5}
			w, p, _ := useItemWorld(rule)
			w.rng = fixedRNG{value: tc.rng}
			w.items[413] = model.ItemDef{Index: 413}
			w.volatiles.Items[413], w.volatiles.ItemCodes[413] = rule, 5
			w.items[591] = model.ItemDef{Index: 591, Pos: 256}
			p.Char.Inv[0] = model.Item{Index: 413, UID: "lac-unit"}
			p.Char.Equip[8] = model.Item{Index: 591, UID: "earring-unit"}
			setItemSanc(&p.Char.Equip[8], 10)
			setItemSancRaw(&p.Char.Equip[8], 233) // +10 Garnet keeps its gem.
			packet := useItemPacket(0, 8)
			binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
			w.onUseItem(p.Session, packet)
			after := p.Char.Equip[8]
			if tc.wantLevel == 0 {
				if after != (model.Item{}) {
					t.Fatalf("failed earring survived: %+v", after)
				}
				return
			}
			raw, _ := sancRaw(after)
			if itemSanc(after) != 11 || raw != 237 {
				t.Fatalf("earring=%+v raw=%d", after, raw)
			}
		})
	}
}
