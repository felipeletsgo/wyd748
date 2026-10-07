package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCriticalShutdownWaitsForOwnerBeforeFinalSnapshotAndSharesResult(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	world, finalStore := newShutdownWorld()
	account := newShutdownAccount("held")
	player := addShutdownPlayer(world, 1, account)
	world.criticalWrites = owner
	applied := false
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{account}}, func(criticalPersistenceResult) error {
		if finalStore.saves != 0 || finalStore.flushes != 0 || !world.shuttingDown {
			t.Fatal("final snapshot/flush preceded owner settlement")
		}
		applied = true
		return nil
	}); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	first, second := make(chan error, 1), make(chan error, 1)
	world.handle(command{shutdown: first})
	world.handle(command{shutdown: second})
	if !queue.stopping || !world.shutdownStarted || world.shutdownFinished || applied || finalStore.saves != 0 || finalStore.flushes != 0 {
		t.Fatal("shutdown reported final persistence while critical work was pending")
	}
	for _, done := range []chan error{first, second} {
		select {
		case <-done:
			t.Fatal("repeated shutdown bypassed pending drain")
		default:
		}
	}
	// Neither periodic ticks nor a queued physical disconnect may remove/mutate
	// the aggregate needed by the pending completion and final snapshot.
	world.tick()
	world.handle(command{s: player.Session})
	if world.mobTickCounter != 0 || world.players[player.Session] != player || !player.Session.IsClosed() || finalStore.saves != 0 {
		t.Fatal("shutdown tick/disconnect mutated or persisted pending state")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	for _, done := range []chan error{first, second} {
		select {
		case err := <-done:
			if err != nil {
				t.Fatal(err)
			}
		default:
			t.Fatal("settled shutdown caller did not receive its result")
		}
	}
	if !applied || !world.shutdownFinished || finalStore.saves != 1 || finalStore.flushes != 1 || len(world.shutdownWaiters) != 0 {
		t.Fatal("final snapshot/drain omitted or retained callers")
	}
	third := make(chan error, 1)
	world.runShutdown(third)
	if err := <-third; err != nil || finalStore.saves != 1 || finalStore.flushes != 1 {
		t.Fatal("completed shutdown replayed final writes or lost result")
	}
}

func TestCriticalShutdownUnknownOutcomeDoesNotOverwriteFinalState(t *testing.T) {
	gate, release := criticalGate(t)
	failure := errors.New("injected unknown commit")
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate, failure: failure}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	world, finalStore := newShutdownWorld()
	account := newShutdownAccount("uncertain")
	addShutdownPlayer(world, 1, account)
	world.criticalWrites = owner
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{account}}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	done := make(chan error, 1)
	world.runShutdown(done)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if err := <-done; !errors.Is(err, failure) {
		t.Fatalf("shutdown lost indeterminate outcome: %v", err)
	}
	if !world.shutdownFinished || finalStore.saves != 0 || finalStore.flushes != 0 || len(queue.pending) != 1 {
		t.Fatal("uncertain state overwritten or fence discarded")
	}
}

func TestCriticalShutdownDrainsEveryAcceptedCompletionBeforeReply(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalSelectiveTestStore{criticalTestStore: &criticalTestStore{entered: make(chan criticalWrite, 2)}, blocked: gate}
	queue := newCriticalTestQueue(t, backend, 2, 2)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	world, finalStore := newShutdownWorld()
	world.criticalWrites = owner
	applied := 0
	for _, name := range []string{"slow", "fast"} {
		if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}, func(criticalPersistenceResult) error { applied++; return nil }); err != nil {
			t.Fatal(err)
		}
	}
	awaitReadyCriticalResults(t, queue, 1)
	done := make(chan error, 1)
	world.runShutdown(done)
	world.processCriticalCompletions(1)
	if applied != 1 || world.shutdownFinished || finalStore.flushes != 0 {
		t.Fatal("shutdown finished after only one accepted completion")
	}
	select {
	case <-done:
		t.Fatal("shutdown replied while another accepted store call was held")
	default:
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if err := <-done; err != nil || applied != 2 || finalStore.flushes != 1 {
		t.Fatal("shutdown did not drain every accepted completion")
	}
}

func TestCriticalShutdownTimeoutDoesNotCancelAcceptedWork(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	world, finalStore := newShutdownWorld()
	world.criticalWrites = owner
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "held"}}}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	caller := make(chan bool, 1)
	go func() { caller <- world.Shutdown(40 * time.Millisecond) }()
	select {
	case cmd := <-world.commands:
		world.handle(cmd)
	case <-time.After(time.Second):
		t.Fatal("shutdown command not delivered")
	}
	select {
	case complete := <-caller:
		if complete {
			t.Fatal("held critical transaction reported completed shutdown")
		}
	case <-time.After(time.Second):
		t.Fatal("shutdown caller exceeded timeout")
	}
	if len(queue.pending) != 1 || world.shutdownFinished || finalStore.flushes != 0 {
		t.Fatal("caller timeout cancelled pending work or ran final drain")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if !world.shutdownFinished || finalStore.flushes != 1 || len(queue.pending) != 0 {
		t.Fatal("accepted work did not settle after caller timed out")
	}
}

func TestCriticalShutdownBoundsRetainedCallers(t *testing.T) {
	world := &World{shutdownStarted: true, shuttingDown: true}
	for range shutdownWaiterLimit {
		world.runShutdown(make(chan error, 1))
	}
	refused := make(chan error, 1)
	world.runShutdown(refused)
	if err := <-refused; err == nil || len(world.shutdownWaiters) != shutdownWaiterLimit {
		t.Fatal("shutdown retained unbounded timed-out caller channels")
	}
}

func TestShutdownTickBarrierProtectsFinalSnapshotAndMaintenance(t *testing.T) {
	for _, explicit := range []bool{false, true} {
		name := "maintenance"
		if explicit {
			name = "graceful-shutdown"
		}
		t.Run(name, func(t *testing.T) {
			world, finalStore := newShutdownWorld()
			player := addShutdownPlayer(world, 1, newShutdownAccount("stopped"))
			beforeHP := player.Char.Score.CurHP
			world.nextAutoSave = time.Time{}
			world.nextQuestZoneReset = time.Time{}
			if explicit {
				done := make(chan error, 1)
				world.runShutdown(done)
				if err := <-done; err != nil {
					t.Fatal(err)
				}
			} else {
				world.shuttingDown = true
			}
			saves := finalStore.saves
			world.tick()
			world.handle(command{s: player.Session, pkt: wire.Build(wire.OpCharacterLogout, player.ID, 12)})
			if world.mobTickCounter != 0 || finalStore.saves != saves || player.Char.Score.CurHP != beforeHP || !world.nextQuestZoneReset.IsZero() {
				t.Fatal("tick/packet mutated or autosaved after shutdown barrier")
			}
		})
	}
}
