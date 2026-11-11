package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestConsumableAdmissionAdaptersResyncOnRejections(t *testing.T) {
	tests := []struct {
		name      string
		itemIndex uint16
		admit     func(*World, *Player)
	}{
		{
			name: "item reward generic error",
			admit: func(w *World, p *Player) {
				w.admitItemReward(p, 0, model.VolatileRule{Action: "unknown"})
			},
		},
		{
			name:      "item reward gameplay rejection",
			itemIndex: 1000,
			admit: func(w *World, p *Player) {
				w.admitItemReward(p, 0, model.VolatileRule{Action: "loot_box"})
			},
		},
		{
			name: "mount generic error",
			admit: func(w *World, p *Player) {
				w.admitMountConsumable(p, 0, model.VolatileRule{}, useItemRequest{}, false)
			},
		},
		{
			name:      "mount gameplay rejection",
			itemIndex: 1000,
			admit: func(w *World, p *Player) {
				w.admitMountConsumable(p, 0, model.VolatileRule{MountAction: "feed"}, useItemRequest{}, false)
			},
		},
		{
			name: "sidecar generic error",
			admit: func(w *World, p *Player) {
				w.admitSidecarConsumable(p, 0, model.VolatileRule{Action: "unknown"})
			},
		},
		{
			name:      "sidecar gameplay rejection",
			itemIndex: 1000,
			admit: func(w *World, p *Player) {
				w.admitSidecarConsumable(p, 0, model.VolatileRule{Action: "grant_counter"})
			},
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			player, _ := networkedTestPlayer(1, "Player", 2100, 2100)
			player.Char.Inv[0].Index = tc.itemIndex
			w := worldWithNetworkedPlayers(player)

			tc.admit(w, player)

			if got := player.Session.QueuedPacketsForTest(); got == 0 {
				t.Fatal("rejected item use did not resynchronize the client")
			}
			if player.Char.Inv[0].Index != tc.itemIndex {
				t.Fatalf("rejected item use changed source slot: got %d, want %d", player.Char.Inv[0].Index, tc.itemIndex)
			}
		})
	}
}

func TestSyncPersistenceAllowedTracksAndDeduplicatesCoordinatorRejections(t *testing.T) {
	w := &World{}
	if err := w.syncPersistenceAllowed("account"); err != nil {
		t.Fatalf("legacy synchronous persistence should be allowed without a coordinator: %v", err)
	}
	w.criticalWrites = &criticalPersistenceOwner{}
	first := w.syncPersistenceAllowed("account")
	second := w.syncPersistenceAllowed("account")
	other := w.syncPersistenceAllowed("guild state")
	if !errors.Is(first, errSynchronousPersistenceDisabled) ||
		!errors.Is(second, errSynchronousPersistenceDisabled) ||
		!errors.Is(other, errSynchronousPersistenceDisabled) {
		t.Fatalf("coordinated synchronous writes were not rejected: %v / %v / %v", first, second, other)
	}
	if len(w.syncPersistenceRejected) != 2 || !w.syncPersistenceRejected["account"] || !w.syncPersistenceRejected["guild state"] {
		t.Fatalf("rejected operation tracking=%v", w.syncPersistenceRejected)
	}
	w.terminalPersistence = true
	if err := w.syncPersistenceAllowed("shutdown snapshot"); err != nil {
		t.Fatalf("terminal persistence should allow synchronous writes: %v", err)
	}
}
