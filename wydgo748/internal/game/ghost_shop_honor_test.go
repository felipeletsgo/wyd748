package game

import (
	"testing"
	"time"
)

// An active ghost shop earns its owner one honor every ten minutes.
func TestGhostShopEarnsHonorEveryTenMinutes(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	start := time.Unix(1_800_000_000, 0)
	shop := &GhostShop{ID: ghostShopIDBase + p.ID, OwnerID: p.ID}
	p.GhostShop = shop
	w.ghostShops[shop.ID] = shop

	w.tickGhostShopHonor(start)
	w.tickGhostShopHonor(start.Add(9 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 0 {
		t.Fatalf("honor before ten minutes = %d", got)
	}
	w.tickGhostShopHonor(start.Add(10 * time.Minute))
	w.tickGhostShopHonor(start.Add(20 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 2 {
		t.Fatalf("honor after twenty minutes = %d, want 2", got)
	}

	p.GhostShop = nil // closed shop earns nothing
	w.tickGhostShopHonor(start.Add(40 * time.Minute))
	if got := counterBalance(p, honorCounter); got != 2 {
		t.Fatalf("closed shop earned honor: %d", got)
	}
}
