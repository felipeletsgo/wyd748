package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestCostumesAndFairiesRejectRefinement(t *testing.T) {
	costume := model.ItemDef{Index: 4180, Mesh: model.CostumeMesh, Pos: model.CostumeSlotMask}
	fairy := model.ItemDef{Index: 769, Pos: 1 << model.FamiliarSlot}
	weapon := model.ItemDef{Index: 200, Pos: 4}
	if !refinementLocked(costume) || !refinementLocked(fairy) || refinementLocked(weapon) {
		t.Fatal("only costumes and fairies are refinement-locked")
	}

	for _, tc := range []struct {
		name string
		slot uint32
		def  model.ItemDef
	}{{"costume", model.CostumeSlot, costume}, {"fairy", model.FamiliarSlot, fairy}} {
		t.Run(tc.name, func(t *testing.T) {
			rule := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9}
			w, p, st := useItemWorld(rule)
			w.rng = fixedRNG{value: 0}
			w.items[tc.def.Index] = tc.def
			p.Char.Inv[0] = model.Item{Index: 100}
			p.Char.Equip[tc.slot] = model.Item{Index: tc.def.Index}
			pkt := useItemPacket(0, tc.slot)
			binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)

			w.onUseItem(p.Session, pkt)

			if p.Char.Inv[0].Index != 100 || itemSanc(p.Char.Equip[tc.slot]) != 0 || st.saves != 0 {
				t.Fatalf("%s accepted refinement: powder=%+v target=%+v saves=%d",
					tc.name, p.Char.Inv[0], p.Char.Equip[tc.slot], st.saves)
			}
		})
	}
}

// Production enables critical persistence; the rejection must happen while
// building the proposal, before any durable write is queued.
func TestCriticalRefinementRejectsCostumesAndFairies(t *testing.T) {
	for _, tc := range []struct {
		name string
		slot uint32
		def  model.ItemDef
	}{
		{"costume", model.CostumeSlot, model.ItemDef{Index: 4180, Mesh: model.CostumeMesh, Pos: model.CostumeSlotMask}},
		{"fairy", model.FamiliarSlot, model.ItemDef{Index: 769, Pos: 1 << model.FamiliarSlot}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _, _, _, _, _ := criticalTreasuryFixture(t, 100)
			w.warConfig.Enabled = false
			rule, _ := configureCriticalConsumableSettlement(w, p, "refine")
			w.items[tc.def.Index] = tc.def
			p.Char.Equip[tc.slot] = model.Item{Index: tc.def.Index, UID: "locked-target"}

			err := w.queueEquipmentConsumable(p, 0, useItemRequest{dstType: placeEquip, dstPos: tc.slot}, rule)

			var rejection *nativeConsumableRejection
			if !errors.As(err, &rejection) || rejection.message != "That item cannot be refined." {
				t.Fatalf("%s refinement was not rejected before admission: %v", tc.name, err)
			}
			if p.Char.Inv[0].Index != 900 || itemSanc(p.Char.Equip[tc.slot]) != 0 {
				t.Fatalf("%s refinement changed state: powder=%+v target=%+v", tc.name, p.Char.Inv[0], p.Char.Equip[tc.slot])
			}
		})
	}
}
