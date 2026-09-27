package game

import "testing"

func TestWorldEntityIDsKeepMobsAndGroundObjectsDistinct(t *testing.T) {
	w := &World{
		nextMobID: 10000,
		groundItems: map[uint16]*GroundItem{
			10000: {ID: 10000},
		},
	}
	if got := w.allocMobID(); got != 10001 {
		t.Fatalf("mob ID = %d, want 10001 after occupied ground object", got)
	}

	w.nextItemID = 10000
	w.mobsByID = map[uint16]*Mob{10000: {ID: 10000}}
	w.groundItems = nil
	if got, ok := w.allocGroundItemID(400); !ok || got != 10001 {
		t.Fatalf("ground object ID = %d/%v, want 10001 after occupied mob", got, ok)
	}
}

func TestWorldEntityIDsReserveCannonAndPlayerRanges(t *testing.T) {
	w := &World{nextMobID: 15001}
	if got := w.allocMobID(); got != 15101 {
		t.Fatalf("mob ID = %d, want 15101 after reserved cannon range", got)
	}

	w.mobsByID = map[uint16]*Mob{15001: {ID: 15001}}
	if got, ok := w.allocGroundItemID(746); !ok || got != 15002 {
		t.Fatalf("cannon ID = %d/%v, want 15002 after occupied mob", got, ok)
	}

	w.nextItemID = ^uint16(0)
	w.groundItems = map[uint16]*GroundItem{^uint16(0): {ID: ^uint16(0)}}
	w.mobsByID[1000] = &Mob{ID: 1000}
	if got, ok := w.allocGroundItemID(400); !ok || got != 1001 {
		t.Fatalf("wrapped ground object ID = %d/%v, want 1001 outside player and mob ranges", got, ok)
	}
}

func TestWorldEntityIDsReserveFutureGhostShopIDs(t *testing.T) {
	w := &World{nextMobID: ghostShopIDBase + 1, nextItemID: ghostShopIDBase + 1}
	want := uint16(ghostShopIDBase) + firstMobID
	if got := w.allocMobID(); got != want {
		t.Fatalf("mob ID = %d, want %d after virtual ghost-shop range", got, want)
	}
	if got, ok := w.allocGroundItemID(400); !ok || got != want {
		t.Fatalf("ground object ID = %d/%v, want %d after virtual ghost-shop range", got, ok, want)
	}
	if !isReservedNonPlayerEntityID(ghostShopIDBase+1) ||
		!isReservedNonPlayerEntityID(ghostShopIDBase+firstMobID-1) ||
		isReservedNonPlayerEntityID(ghostShopIDBase) ||
		isReservedNonPlayerEntityID(want) {
		t.Fatal("virtual ghost-shop ID boundaries are incorrect")
	}
}
