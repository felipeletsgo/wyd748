package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestRequestPlayerTeleportRejectsInvalidLegacyDestinationWithoutMutation(t *testing.T) {
	w, p, store := useItemWorld(model.VolatileRule{})
	oldX, oldY := p.X, p.Y
	acknowledged := false
	if w.requestPlayerTeleport(p, 0, 0, func() { acknowledged = true }, nil) {
		t.Fatal("invalid destination was admitted")
	}
	if p.X != oldX || p.Y != oldY || p.Char.X != oldX || p.Char.Y != oldY || store.saves != 0 || acknowledged {
		t.Fatalf("rejected teleport mutated state: pos=(%d,%d) char=(%d,%d) saves=%d ack=%t",
			p.X, p.Y, p.Char.X, p.Char.Y, store.saves, acknowledged)
	}
}

func TestRequestPlayerTeleportRejectsMissingCriticalIdentity(t *testing.T) {
	w := &World{criticalWrites: &criticalPersistenceOwner{}}
	if w.requestPlayerTeleport(nil, 100, 100, nil, nil) {
		t.Fatal("nil player was admitted for a critical teleport")
	}
	p, _ := networkedTestPlayer(1, "Teleport", 2100, 2100)
	p.Account = nil
	if w.requestPlayerTeleport(p, 100, 100, nil, nil) {
		t.Fatal("player without an account was admitted for a critical teleport")
	}
}
