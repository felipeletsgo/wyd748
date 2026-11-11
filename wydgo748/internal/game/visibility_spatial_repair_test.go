package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestEnsureAuxVisibilitySpatialIndexesRepairsMissingEntries(t *testing.T) {
	w := &World{}
	indexedItem := &GroundItem{ID: 1, X: 64, Y: 96}
	missingItem := &GroundItem{ID: 2, X: 128, Y: 160}
	w.registerGroundItem(indexedItem)
	w.groundItems[missingItem.ID] = missingItem

	indexedShop := &GhostShop{ID: 3, X: 192, Y: 224}
	missingShop := &GhostShop{ID: 4, X: 256, Y: 288}
	w.registerGhostShop(indexedShop)
	w.ghostShops[missingShop.ID] = missingShop

	w.ensureAuxVisibilitySpatialIndexes()
	if w.groundItemCell[missingItem.ID] != spatialKey(missingItem.X, missingItem.Y) ||
		w.groundItemCells[spatialKey(missingItem.X, missingItem.Y)][missingItem.ID] != missingItem {
		t.Fatal("missing ground item was not registered in its spatial cell")
	}
	if w.ghostShopCell[missingShop.ID] != spatialKey(missingShop.X, missingShop.Y) ||
		w.ghostShopCells[spatialKey(missingShop.X, missingShop.Y)][missingShop.ID] != missingShop {
		t.Fatal("missing ghost shop was not registered in its spatial cell")
	}

	w.ensureAuxVisibilitySpatialIndexes()
	if len(w.groundItemCell) != len(w.groundItems) || len(w.ghostShopCell) != len(w.ghostShops) {
		t.Fatal("rechecking complete spatial indexes changed their cardinality")
	}
}

func TestApplyGhostShopVisibilityRegionsShowsAndHidesIndexedShops(t *testing.T) {
	p, session := networkedTestPlayer(1, "ShopObserver", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	shop := &GhostShop{ID: 25001, Name: "Seller", Title: "Shop", X: 2100, Y: 2100,
		Score: model.Score{Version: model.ScoreVersion, MaxHP: 1, CurHP: 1}}
	w.registerGhostShop(shop)
	region := visibilityBounds{minX: 2100, maxX: 2100, minY: 2100, maxY: 2100}
	regions := [4]visibilityBounds{region}

	w.applyGhostShopVisibilityRegions(p, regions, 1, true)
	if !p.hasVisible(shop.ID) || session.QueuedPacketsForTest() == 0 {
		t.Fatal("entering region did not show its registered ghost shop")
	}
	packetCount := session.QueuedPacketsForTest()
	w.applyGhostShopVisibilityRegions(p, regions, 1, false)
	if !p.hasVisible(shop.ID) || session.QueuedPacketsForTest() != packetCount {
		t.Fatal("shop inside the retained view was hidden or republished")
	}
	p.X, p.Y = 4000, 4000
	w.applyGhostShopVisibilityRegions(p, regions, 1, false)
	if p.hasVisible(shop.ID) {
		t.Fatal("shop outside the player's view remained visible")
	}

	p.X, p.Y = 2100, 2100
	cell := w.ghostShopCells[spatialKey(shop.X, shop.Y)]
	stale := &GhostShop{ID: 25002, X: 2100, Y: 2100}
	outside := &GhostShop{ID: 25003, X: 2101, Y: 2100}
	cell[stale.ID] = stale
	cell[25004] = nil
	cell[outside.ID] = outside
	w.ghostShops[outside.ID] = outside
	w.applyGhostShopVisibilityRegions(p, regions, 1, true)
	if !p.hasVisible(shop.ID) || p.hasVisible(stale.ID) || p.hasVisible(outside.ID) || p.hasVisible(25004) {
		t.Fatal("region materialized a stale, nil, or out-of-bounds shop")
	}
}
