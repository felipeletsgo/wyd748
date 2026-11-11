package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestPutCraftResultValidatesAndChoosesInventorySlot(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[200] = model.ItemDef{Index: 200}

	changed := make(map[int]struct{})
	if w.putCraftResult(p, 0, model.Item{Index: 201}, changed) || len(changed) != 0 {
		t.Fatal("unknown catalog item was inserted")
	}
	if w.putCraftResult(p, 2, model.Item{Index: 200, UID: "invalid"}, changed) || len(changed) != 0 {
		t.Fatal("item with invalid identity was inserted")
	}
	if !w.putCraftResult(p, 2, model.Item{Index: 200}, changed) || p.Char.Inv[2].Index != 200 || p.Char.Inv[2].UID == "" {
		t.Fatal("result did not use the preferred empty slot", p.Char.Inv[2])
	}
	if _, ok := changed[2]; !ok {
		t.Fatal("preferred slot was not recorded as changed", changed)
	}

	delete(changed, 2)
	p.Char.Inv[2] = model.Item{Index: 100}
	if !w.putCraftResult(p, 2, model.Item{Index: 200}, changed) || p.Char.Inv[0].Index != 200 {
		t.Fatal("result did not fall back to the first empty slot", p.Char.Inv[0])
	}
	if _, ok := changed[0]; !ok {
		t.Fatal("fallback slot was not recorded as changed", changed)
	}
}

func TestPutCraftResultRejectsFullInventoryWithoutChanges(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[200] = model.ItemDef{Index: 200}
	for slot := range p.Char.Inv[:model.PlayerCarrySlots] {
		p.Char.Inv[slot] = model.Item{Index: 100}
	}
	before := p.Char.Inv
	changed := make(map[int]struct{})
	if w.putCraftResult(p, model.PlayerCarrySlots, model.Item{Index: 200}, changed) || p.Char.Inv != before || len(changed) != 0 {
		t.Fatal("result changed a full inventory", p.Char.Inv, changed)
	}
}
