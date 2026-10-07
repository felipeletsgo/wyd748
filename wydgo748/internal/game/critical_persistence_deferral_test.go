package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalPKModePacket(enabled bool) []byte {
	packet := inboundPacket(wire.OpPKMode, 16)
	if enabled {
		binary.LittleEndian.PutUint32(packet[12:16], 1)
	}
	return packet
}

func holdCriticalLifecycleWrite(t *testing.T) (*World, *Player, *criticalPersistence, func(), chan criticalWrite) {
	t.Helper()
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, player, _ := criticalLifecycleTestWorld(t, queue)
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	return world, player, queue, release, backend.entered
}

func TestCriticalDeferredPacketsReplayInArrivalOrderAfterAcknowledgement(t *testing.T) {
	world, player, queue, release, _ := holdCriticalLifecycleWrite(t)
	world.handle(command{s: player.Session, pkt: criticalPKModePacket(true)})
	world.handle(command{s: player.Session, pkt: criticalPKModePacket(false)})
	if player.PKMode || player.Session.QueuedPacketsForTest() != 0 || world.criticalDeferredCount != 2 {
		t.Fatal("reserved packets were dispatched or discarded before acknowledgement")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	if world.criticalDeferredCount != 2 || player.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("worker completion replayed packets before World acknowledgement")
	}
	world.processCriticalCompletions(1)
	if player.PKMode || player.Session.QueuedPacketsForTest() != 2 || world.criticalDeferredCount != 0 ||
		len(world.criticalDeferred) != 0 {
		t.Fatal("acknowledgement did not replay every deferred packet in arrival order")
	}
	if got := dequeueSkillMessage(t, player); got != "PK mode enabled." {
		t.Fatalf("first replayed response=%q", got)
	}
	if got := dequeueSkillMessage(t, player); got != "PK mode disabled." {
		t.Fatalf("second replayed response=%q", got)
	}
}

func TestCriticalDeferredBacklogKeepsNewerPacketsBehindOlderOnes(t *testing.T) {
	world, player, queue, release, _ := holdCriticalLifecycleWrite(t)
	world.handle(command{s: player.Session, pkt: criticalPKModePacket(true)})
	release()
	awaitReadyCriticalResults(t, queue, 1)
	// Deliver the result without replay, then send a newer packet. The newer
	// intention must wait behind the older backlog even though the reservation
	// has already been released.
	result, ok := queue.poll()
	if !ok {
		t.Fatal("held result was not delivered")
	}
	if err := world.criticalWrites.complete(result); err != nil {
		t.Fatal(err)
	}
	if world.criticalPlayerReserved(player) {
		t.Fatal("fixture reservation was not released")
	}
	world.handle(command{s: player.Session, pkt: criticalPKModePacket(false)})
	if player.PKMode || world.criticalDeferredCount != 2 {
		t.Fatal("newer packet overtook an older deferred packet")
	}
	world.resumeCriticalDeferredCommands(1)
	if !player.PKMode || world.criticalDeferredCount != 1 {
		t.Fatal("bounded replay did not run exactly the oldest packet")
	}
	world.resumeCriticalDeferredCommands(criticalDeferredReplayLimit)
	if player.PKMode || world.criticalDeferredCount != 0 {
		t.Fatal("remaining backlog did not replay in order")
	}
}

func TestCriticalDeferredPacketsFromClosedTransportNeverReplay(t *testing.T) {
	world, player, queue, release, _ := holdCriticalLifecycleWrite(t)
	world.handle(command{s: player.Session, pkt: criticalPKModePacket(true)})
	player.Session.Close()
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if player.PKMode || world.criticalDeferredCount != 0 || len(world.criticalDeferred) != 0 {
		t.Fatal("closed transport packet mutated a retained player or leaked its backlog")
	}
}

func TestCriticalDeferredBacklogLimitClosesTransport(t *testing.T) {
	world, player, _, _, _ := holdCriticalLifecycleWrite(t)
	for range criticalDeferredCommandLimit {
		world.handle(command{s: player.Session, pkt: criticalPKModePacket(true)})
	}
	if player.Session.IsClosed() || world.criticalDeferredCount != criticalDeferredCommandLimit {
		t.Fatal("backlog below the limit closed the session or lost packets")
	}
	world.handle(command{s: player.Session, pkt: criticalPKModePacket(true)})
	if !player.Session.IsClosed() || world.criticalDeferredCount != 0 || player.PKMode {
		t.Fatal("backlog overflow did not close the transport and release retained packets")
	}
}

func TestCriticalDeferralDisabledWithoutCoordinator(t *testing.T) {
	w, _ := newShutdownWorld()
	account := newShutdownAccount("legacy")
	p := addShutdownPlayer(w, 1, account)
	if w.deferCriticalCommand(command{s: p.Session, pkt: criticalPKModePacket(true)}) || w.criticalDeferredCount != 0 {
		t.Fatal("legacy World without a coordinator deferred a packet")
	}
}

// deliverCriticalLoopPing sends a no-op intention through the actual actor
// queue. While its sender is reserved, it is deferred rather than dispatched;
// economic duplicate replay is covered by the deferral tests above.
func deliverCriticalLoopPing(t *testing.T, w *World, p *Player) {
	t.Helper()
	select {
	case w.commands <- command{s: p.Session, pkt: inboundPacket(wire.OpPing, 12)}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept the ping intention")
	}
}

// discardCriticalDeferredDuplicates removes held repetitions before a test
// asserts exactly one settlement. Production replays them after
// acknowledgement as new intentions; the deferral tests above cover that.
func discardCriticalDeferredDuplicates(w *World) {
	for session := range w.criticalDeferred {
		w.dropCriticalDeferred(session)
	}
}
