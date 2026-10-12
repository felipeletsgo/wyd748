package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestCriticalCommittedAccountFailureDiscardsSessionWithoutAnotherSave(t *testing.T) {
	for _, scenario := range []string{"disconnected", "open", "panic", "partial-apply", "selection"} {
		t.Run(scenario, func(t *testing.T) {
			world, held, bystander, queue, backend, _, release := containedCriticalWorld(t, nil)
			if scenario == "selection" {
				world.unindexPlayerCharacter(held)
				delete(world.playersByID, held.ID)
				held.InWorld, held.Char, held.ID, held.CharSlot = false, nil, 0, -1
			}
			proposal := accountStateSnapshot(held.Account)
			proposal.Chars[0].Gold = 12345
			calls := 0
			_, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposal}}, []*Player{held},
				func(criticalPersistenceResult) error {
					calls++
					if scenario == "panic" {
						panic("failed after confirmed commit")
					}
					if scenario == "partial-apply" {
						held.Account.Chars[0].Gold = 999
					}
					return errCriticalPersistenceIdentity
				})
			if err != nil {
				t.Fatal(err)
			}
			write := awaitCriticalWrite(t, backend.entered)
			if write.accounts[0].Chars[0].Gold != 12345 {
				t.Fatal("worker did not receive the detached committed proposal")
			}
			if scenario == "disconnected" {
				world.onDisconnect(held.Session)
				world.onDisconnect(held.Session)
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			world.processCriticalCompletions(1)
			if world.shuttingDown || world.shutdownErr != nil || world.players[held.Session] != nil ||
				!held.PersistencePoisoned || !held.Session.IsClosed() || len(queue.pending) != 0 ||
				len(world.criticalQuarantine) != 0 || len(world.criticalDisconnects) != 0 || calls != 1 {
				t.Fatal("confirmed commit left a permanently fenced session or replayed its continuation")
			}
			if world.players[bystander.Session] != bystander || bystander.PersistencePoisoned || bystander.Session.IsClosed() {
				t.Fatal("account recovery affected an unrelated session")
			}
			fresh := gameNet.NewTestSession(99, 8)
			if !world.claimAccountSession(fresh, "HeLd") {
				t.Fatal("confirmed account could not log in again")
			}
			world.onDisconnect(held.Session)
			if world.accountSessions["held"] != fresh || calls != 1 || len(backend.entered) != 0 {
				t.Fatal("late disconnect overwrote committed data or released the new account owner")
			}
		})
	}
}

func TestCriticalCommittedRecoveryRetainsOwnershipUntilInstanceAcknowledgement(t *testing.T) {
	gate, release := criticalGate(t)
	t.Cleanup(release)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	backend := &criticalDisconnectInstanceStore{criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	world, held, fallback := criticalLifecycleTestWorld(t, queue)
	world.criticalAccountContainment = true
	room := addCriticalWaterRoom(world, held)
	uid := held.Char.UID
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
		[]*Player{held}, func(criticalPersistenceResult) error { return errCriticalPersistenceIdentity }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	write := awaitCriticalWrite(t, base.entered)
	if write.instances == nil || len(write.accounts) != 0 || len(room.MemberIDs) != 0 {
		t.Fatal("recovery did not persist instance detachment without resaving poisoned RAM")
	}
	for range 3 {
		world.onDisconnect(held.Session)
		world.resumeCriticalDisconnects(8)
		if world.players[held.Session] != held || world.accountSessions["held"] != held.Session ||
			!world.criticalInstanceReserved() || !world.criticalPlayerReserved(held) ||
			world.claimAccountSession(gameNet.NewTestSession(99, 8), "held") ||
			len(base.entered) != 0 || fallback.saves != 0 || world.criticalInstanceAdmissionErr != nil {
			t.Fatal("instance cleanup unlocked ownership early, replayed a write or rejected the retained recovery identity")
		}
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if world.shuttingDown || world.players[held.Session] != nil || len(queue.pending) != 0 ||
		world.instanceStateDirty || !world.claimAccountSession(gameNet.NewTestSession(100, 8), "held") {
		t.Fatal("covering instance acknowledgement did not finish recovery")
	}
	if _, retained := world.pendingInstanceMembers[room.RuntimeID][uid]; !retained {
		t.Fatal("recovery lost durable resumable instance membership")
	}
}

func TestCriticalCommittedRecoveryNeverEvictsChangedOwnership(t *testing.T) {
	for _, scenario := range []string{"session", "account", "lifecycle", "character-index", "disconnect-player"} {
		t.Run(scenario, func(t *testing.T) {
			world, held, _, queue, backend, _, release := containedCriticalWorld(t, nil)
			id, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
				[]*Player{held}, func(criticalPersistenceResult) error { return errCriticalPersistenceIdentity })
			if err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			switch scenario {
			case "session":
				world.accountSessions["held"] = gameNet.NewTestSession(99, 8)
			case "account":
				held.Account = accountStateSnapshot(held.Account)
				held.Char = &held.Account.Chars[held.CharSlot]
			case "lifecycle":
				held.criticalLifecycle = &criticalPlayerLifecycle{}
			case "character-index":
				delete(world.playersByCharacterUID, held.Char.UID)
			case "disconnect-player":
				world.deferCriticalDisconnect(held.Session)
				world.criticalDisconnects[held.Session].player = &Player{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			world.processCriticalCompletions(1)
			if queue.pending[id] == nil || !world.criticalAccountReserved("held") || len(world.criticalQuarantine) != 1 {
				t.Fatal("changed ownership was treated as safe committed-session recovery")
			}
		})
	}
}

func TestCriticalUnknownCompletionCannotBeReclassifiedByDuplicateSuccess(t *testing.T) {
	world, held, _, queue, backend, _, release := containedCriticalWorld(t, errors.New("commit outcome unknown"))
	id, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
		[]*Player{held}, func(criticalPersistenceResult) error { return nil })
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	world.applyCriticalCompletion(criticalPersistenceResult{id: id})
	if queue.pending[id] == nil || world.players[held.Session] != held || !world.criticalAccountReserved("held") ||
		world.claimAccountSession(gameNet.NewTestSession(99, 8), "held") {
		t.Fatal("a duplicate success bypassed unknown-outcome containment")
	}
}

func TestCriticalConfirmedSharedContinuationFailureStillStopsWorld(t *testing.T) {
	world, held, _, queue, backend, _, release := containedCriticalWorld(t, nil)
	_, err := world.submitCriticalSharedWriteWithRejection(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
		[]*Player{held}, func(criticalPersistenceResult) error { return errCriticalPersistenceIdentity }, nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if !world.shuttingDown || world.shutdownErr == nil || len(queue.pending) != 1 {
		t.Fatal("confirmed account commit bypassed shared gameplay containment")
	}
}
