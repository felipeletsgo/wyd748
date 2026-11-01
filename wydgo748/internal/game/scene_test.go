package game

import (
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

// W3/D3: an arrival among more entities than the output queue holds must not
// overflow. Creations wait for room, the scene completes on later ticks and
// no update reaches an entity the client has not created yet.
func crowdedArrivalWorld(t *testing.T, mobs, queue int) (*World, *Player, []*Mob) {
	t.Helper()
	list := make([]*Mob, 0, mobs)
	for i := range mobs {
		list = append(list, &Mob{ID: uint16(1000 + i), X: uint16(2090 + i%20), Y: uint16(2090 + i/20),
			Def: testNPCDef(model.Score{AttackRun: 1})})
	}
	w := testSpatialWorld(list)
	p, _ := networkedTestPlayer(1, "Arrival", 2100, 2100)
	p.Session = gameNet.NewTestSession(1, queue)
	w.players[p.Session], w.playersByID[p.ID] = p, p
	return w, p, list
}

func TestCrowdedArrivalDefersCreationsInsteadOfOverflowing(t *testing.T) {
	const mobs, queue = 120, 64
	w, p, list := crowdedArrivalWorld(t, mobs, queue)
	w.refreshPlayerVisibility(p)
	if p.Session.IsClosed() || !p.sceneIncomplete || len(p.Visible) == 0 || len(p.Visible) >= mobs {
		t.Fatalf("closed=%t incomplete=%t visible=%d", p.Session.IsClosed(), p.sceneIncomplete, len(p.Visible))
	}
	if free := p.Session.QueueFree(); free < sceneQueueReserve(queue) {
		t.Fatalf("scene publication used the control reserve: free=%d", free)
	}
	var hidden *Mob
	for _, m := range list {
		if !p.hasVisible(m.ID) {
			hidden = m
			break
		}
	}
	queued := p.Session.QueuedPacketsForTest()
	w.sendToMobView(hidden, func() []byte { return wire.ActionStop(hidden.ID, hidden.X, hidden.Y) })
	if p.Session.QueuedPacketsForTest() != queued {
		t.Fatal("an update reached an entity that was not created yet")
	}
	for tick := 0; p.sceneIncomplete || tick == 0; tick++ {
		if tick > mobs {
			t.Fatal("the deferred scene never completed")
		}
		drainPackets(p) // the writer empties the queue between ticks
		w.continueIncompleteScenes()
	}
	if p.Session.IsClosed() || len(p.Visible) != mobs {
		t.Fatalf("completed scene closed=%t visible=%d of %d", p.Session.IsClosed(), len(p.Visible), mobs)
	}
}

func TestDeferredSceneDropsCreationsAfterTeleport(t *testing.T) {
	w, p, _ := crowdedArrivalWorld(t, 120, 64)
	w.refreshPlayerVisibility(p)
	if !p.sceneIncomplete {
		t.Fatal("fixture did not defer creations")
	}
	drainPackets(p)
	p.X, p.Y = 3000, 3000 // a teleport away before the scene completes
	w.continueIncompleteScenes()
	if p.sceneIncomplete || len(p.Visible) != 0 || p.Session.IsClosed() {
		t.Fatalf("teleport kept stale creations: incomplete=%t visible=%d", p.sceneIncomplete, len(p.Visible))
	}
	removals := 0
	for {
		pkt, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if wire.Decrypt(pkt) && wire.ParseHeader(pkt).Type == wire.ParseHeader(wire.RemoveMob(1, 0)).Type {
			removals++
		}
	}
	if removals == 0 {
		t.Fatal("published entities were not removed after the teleport")
	}
}
