package wire

import (
	"testing"

	"wydgo/internal/model"
)

func TestCitizenshipFaceProjectionDoesNotMutateItems(t *testing.T) {
	for citizen := byte(0); citizen <= 10; citizen++ {
		face := model.Item{Index: 6, Eff: [6]byte{1, 2, 3, 4, 5, 99}}
		ch := model.Char{Name: "Citizen", Citizenship: citizen}
		ch.Equip[0] = face
		projected := CharacterFaceItem(ch)
		if projected.Eff[5] != citizen || projected.Index != face.Index || ch.Equip[0] != face {
			t.Fatalf("citizenship %d mutated the item or lost its identity", citizen)
		}
		for i := 0; i < 5; i++ {
			if projected.Eff[i] != face.Eff[i] {
				t.Fatalf("face effect %d changed", i)
			}
		}
		list := CharList("account", []model.Char{ch, ch, ch, ch}, nil, 0)
		for slot := 0; slot < 4; slot++ {
			if list[32+640+slot*18*8+7] != citizen {
				t.Fatalf("selection slot %d lost citizenship %d", slot, citizen)
			}
		}
		login := EnterWorld(1, 0, ch)
		if login[16+324+7] != citizen {
			t.Fatal("world login lost citizenship")
		}
		selection := CharacterSelectionUpdate(OpCharList, 1, []model.Char{ch})
		if selection[16+640+7] != citizen {
			t.Fatal("selection refresh lost citizenship")
		}
	}
}

func TestCapeEmblemProjectionAcrossSelectionAndLogin(t *testing.T) {
	for symbol := byte(1); symbol <= model.CapeEmblemCount; symbol++ {
		ch := model.Char{Name: "Citizen", Citizenship: 1, CapeEmblem: symbol}
		ch.Equip[0] = model.Item{Index: 6, Eff: [6]byte{1, 2, 3, 4, 5, 99}}
		before := ch.Equip
		list := CharList("account", []model.Char{ch, ch, ch, ch}, nil, 0)
		for slot := 0; slot < 4; slot++ {
			if list[32+640+slot*18*8+7] != symbol {
				t.Fatal("selection lost the random emblem")
			}
		}
		if EnterWorld(1, 0, ch)[16+324+7] != symbol ||
			CharacterSelectionUpdate(OpCharList, 1, []model.Char{ch})[16+640+7] != symbol {
			t.Fatal("login or selection refresh lost the random emblem")
		}
		if ch.Equip != before || ch.Citizenship != 1 {
			t.Fatal("projection mutated authoritative state")
		}
	}
}
