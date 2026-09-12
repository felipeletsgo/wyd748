package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestGroundExpiryHeapAddRemoveAndExpire(t *testing.T) {
	w := &World{groundItems: make(map[uint16]*GroundItem),
		groundItemCells: make(map[uint32]map[uint16]*GroundItem), groundItemCell: make(map[uint16]uint32),
		groundExpiryByID: make(map[uint16]time.Time)}
	now := time.Unix(100, 0)
	first := &GroundItem{ID: 1, Item: model.Item{Index: 412}, X: 100, Y: 100, Expire: now.Add(time.Second)}
	removed := &GroundItem{ID: 2, Item: model.Item{Index: 413}, X: 101, Y: 100, Expire: now.Add(time.Second)}
	permanent := &GroundItem{ID: 3, Item: model.Item{Index: 414}, X: 102, Y: 100, Permanent: true}
	w.registerGroundItem(first)
	w.registerGroundItem(removed)
	w.registerGroundItem(permanent)
	w.unregisterGroundItem(removed)
	w.expireGroundItems(now.Add(2 * time.Second))
	if w.groundItems[first.ID] != nil || w.groundItems[removed.ID] != nil {
		t.Fatal("item vencido/removido permaneceu no registry")
	}
	if w.groundItems[permanent.ID] == nil {
		t.Fatal("objeto permanente entrou no heap de expiracao")
	}
	if len(w.groundExpiryByID) != 0 {
		t.Fatalf("indice de expiracao reteve entradas: %+v", w.groundExpiryByID)
	}
}
