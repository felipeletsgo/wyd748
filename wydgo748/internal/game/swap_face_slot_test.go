package game

import (
	"testing"

	"wydgo/internal/model"
)

// Equip slot 0 holds the character face. No 7.48 client path names it in a
// swap; a crafted 0x376 must not move the face into the inventory or replace
// it with an inventory item (A04-03).
func TestSwapRejectsEquipFaceSlotEndpoints(t *testing.T) {
	cases := []struct {
		name            string
		srcType, srcPos byte
		dstType, dstPos byte
		inventoryIndex  uint16
	}{
		{"face to empty inventory", placeEquip, 0, placeInv, 5, 0},
		{"face to occupied inventory", placeEquip, 0, placeInv, 5, 4300},
		{"empty inventory to face", placeInv, 5, placeEquip, 0, 0},
		{"inventory item to face", placeInv, 5, placeEquip, 0, 4300},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st, session := newTimedSwapWorld(t, nil)
			face := model.Item{Index: 1}
			p.Char.Equip[0] = face
			p.Char.Inv[5] = model.Item{Index: tc.inventoryIndex}
			before := p.Char.Inv[5]
			w.onSwapItem(session, timedSwapPacket(tc.srcType, tc.srcPos, tc.dstType, tc.dstPos))
			if p.Char.Equip[0] != face {
				t.Fatalf("face slot changed: %+v", p.Char.Equip[0])
			}
			if p.Char.Inv[5] != before {
				t.Fatalf("inventory slot changed: %+v", p.Char.Inv[5])
			}
			if st.saves != 0 {
				t.Fatalf("rejected swap persisted %d times", st.saves)
			}
		})
	}
}

// A regular equipment swap still works after the face-slot rejection.
func TestSwapStillMovesRegularEquipment(t *testing.T) {
	w, p, st, session := newTimedSwapWorld(t, nil)
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	if p.Char.Equip[fairySlot].Index != 4300 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
		t.Fatalf("regular equip swap failed: equip=%+v inv=%+v saves=%d",
			p.Char.Equip[fairySlot], p.Char.Inv[0], st.saves)
	}
}
