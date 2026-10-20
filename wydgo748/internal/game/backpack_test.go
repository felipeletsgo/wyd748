package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

// WYD 7.48 Backpack (skill 85, help 5085) opens Cargo from anywhere: a
// Huntress who learned it moves items to Cargo without a banker ID.
func TestBackpackOpensCargoWithoutBanker(t *testing.T) {
	for _, tc := range []struct {
		name    string
		learned bool
	}{{"learned", true}, {"not learned", false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			w.items[413] = model.ItemDef{Index: 413}
			p.Char.Class = 3
			p.Char.LearnedSkill = 0
			if tc.learned {
				p.Char.LearnedSkill = 1 << (backpackSkill - 72)
			}
			p.Char.Inv[0] = model.Item{Index: 413, UID: "lac"}
			p.Account.Cargo[0] = model.Item{}
			packet := criticalCargoSwapRequest(p, placeInv, 0, placeStorage, 0)
			binary.LittleEndian.PutUint32(packet[16:20], 0) // no banker ID
			w.onSwapItem(p.Session, packet)
			moved := p.Account.Cargo[0].UID == "lac" && p.Char.Inv[0].Index == 0
			if moved != tc.learned {
				t.Fatalf("moved=%t cargo=%+v inv=%+v", moved, p.Account.Cargo[0], p.Char.Inv[0])
			}
		})
	}
}
