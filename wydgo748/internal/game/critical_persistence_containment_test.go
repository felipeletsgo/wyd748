package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func containedCriticalWorld(t *testing.T, failure error) (*World, *Player, *Player, *criticalPersistence, *criticalTestStore, *fakeClock, func()) {
	t.Helper()
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate, failure: failure}
	queue := newCriticalTestQueue(t, backend, 1, 4)
	t.Cleanup(release)
	world, held, _ := criticalLifecycleTestWorld(t, queue)
	world.criticalAccountContainment = true
	clock := newFakeClock(time.Date(2026, 10, 5, 12, 0, 0, 0, time.UTC))
	world.clock = clock
	account := newShutdownAccount("bystander")
	account.Chars[0].UID = "bystander-character"
	bystander := addShutdownPlayer(world, 2, account)
	claimCriticalTestPlayers(t, world, bystander)
	return world, held, bystander, queue, backend, clock, release
}

func TestCriticalAccountContainmentIsolatesOnlyUnknownParticipants(t *testing.T) {
	world, held, bystander, queue, backend, clock, release := containedCriticalWorld(t,
		errors.New("commit acknowledgement lost"))
	applied := false
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
		[]*Player{held}, func(criticalPersistenceResult) error { applied = true; return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	unknownID := queue.accounts[accountSessionKey(held.Account.Name)]
	if applied || world.shuttingDown || world.shutdownErr != nil {
		t.Fatal("account-only unknown outcome applied state or stopped the whole World")
	}
	if !held.PersistencePoisoned || !held.Session.IsClosed() || !world.criticalPlayerReserved(held) {
		t.Fatal("unknown participant was not isolated behind its retained fence")
	}
	if bystander.PersistencePoisoned || bystander.Session.IsClosed() || world.criticalPlayerReserved(bystander) {
		t.Fatal("containment affected an unrelated player")
	}
	// The bystander keeps playing and persisting during the quarantine.
	backend.failure = nil
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(bystander.Account)}},
		[]*Player{bystander}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatalf("unrelated account could not persist during quarantine: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)

	world.handle(command{s: held.Session})
	if world.players[held.Session] != held {
		t.Fatal("disconnect removed the isolated player before outcome reconciliation")
	}
	for _, delay := range []time.Duration{time.Minute, 24 * time.Hour, 30 * 24 * time.Hour} {
		clock.Advance(delay)
		world.tick()
		if !world.criticalAccountReserved("held") || queue.pending[unknownID] == nil || len(world.criticalQuarantine) != 1 {
			t.Fatal("elapsed time released an unknown outcome's fences or capacity")
		}
		if world.claimAccountSession(gameNet.NewTestSession(99, 8), "held") {
			t.Fatal("an unresolved account was allowed to log in again")
		}
		if world.players[held.Session] != held || len(world.criticalDisconnects) != 1 {
			t.Fatal("unresolved disconnect discarded its containment state")
		}
	}
	// Unrelated account autosaves may run on the advanced clock; they are not
	// evidence of an overwrite of the poisoned participant.
	select {
	case write := <-backend.entered:
		for _, account := range write.accounts {
			if accountSessionKey(account.Name) == accountSessionKey(held.Account.Name) {
				t.Fatal("isolated RAM was saved after an unknown outcome")
			}
		}
	default:
	}
}

func TestCriticalAccountContainmentKeepsSharedAggregatesGlobal(t *testing.T) {
	world, held, _, queue, backend, _, release := containedCriticalWorld(t,
		errors.New("instance commit acknowledgement lost"))
	world.criticalWrites.queue.store = &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{backend}}
	snapshot := world.instanceStateSnapshot()
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)},
		instances: snapshot}, []*Player{held}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if !world.shuttingDown || world.shutdownErr == nil {
		t.Fatal("unknown shared instance outcome did not enter global maintenance")
	}
}
