package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func criticalLifecycleTestWorld(t *testing.T, queue *criticalPersistence) (*World, *Player, *flushCraftStore) {
	t.Helper()
	w, finalStore := newShutdownWorld()
	account := newShutdownAccount("held")
	account.Chars[0].UID = "held-character"
	p := addShutdownPlayer(w, 1, account)
	w.indexPlayerCharacter(p)
	if !w.claimAccountSession(p.Session, account.Name) {
		t.Fatal("fixture account admission failed")
	}
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, finalStore
}

func TestCriticalLifecycleAdmissionRejectsInvalidParticipantsWithoutEffects(t *testing.T) {
	for _, scenario := range []string{"nil-player", "duplicate-player", "missing-participant", "wrong-account",
		"closed-session", "poisoned", "wrong-slot-pointer", "missing-account-owner", "no-owner", "no-callback", "stopping", "unsupported-sidecar"} {
		t.Run(scenario, func(t *testing.T) {
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
			queue := newCriticalTestQueue(t, backend, 1, 1)
			world, player, _ := criticalLifecycleTestWorld(t, queue)
			write := criticalWrite{accounts: []*model.Account{player.Account}}
			participants := []*Player{player}
			apply := func(criticalPersistenceResult) error { t.Fatal("rejected callback ran"); return nil }
			switch scenario {
			case "nil-player":
				participants[0] = nil
			case "duplicate-player":
				write.accounts = append(write.accounts, player.Account)
				participants = append(participants, player)
			case "missing-participant":
				participants = nil
			case "wrong-account":
				write.accounts[0] = &model.Account{Name: "other"}
			case "closed-session":
				player.Session.Close()
			case "poisoned":
				player.PersistencePoisoned = true
			case "wrong-slot-pointer":
				character := *player.Char
				player.Char = &character
			case "missing-account-owner":
				delete(world.accountSessions, "held")
			case "no-owner":
				world.criticalWrites = nil
			case "no-callback":
				apply = nil
			case "stopping":
				world.shuttingDown = true
			case "unsupported-sidecar":
				write.character = &criticalCharacterState{uid: player.Char.UID}
			}
			if id, err := world.submitCriticalWrite(write, participants, apply); err == nil || id != 0 {
				t.Fatalf("invalid admission accepted: id=%d err=%v", id, err)
			}
			if len(queue.pending) != 0 || len(queue.accounts) != 0 || len(queue.characters) != 0 || len(backend.entered) != 0 || player.criticalLifecycle != nil {
				t.Fatal("rejection wrote, reserved or changed the lifecycle")
			}
		})
	}
}

func TestCriticalLifecycleCompletionRejectsIdentityReuse(t *testing.T) {
	for _, scenario := range []string{"player", "session", "account-pointer", "account-name", "account-owner",
		"character-pointer", "character-uid", "character-slot", "id-index", "uid-index", "world-membership",
		"slice-reallocation", "reset-and-same-character-reentry", "poisoned"} {
		t.Run(scenario, func(t *testing.T) {
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
			queue := newCriticalTestQueue(t, backend, 1, 1)
			t.Cleanup(release)
			world, player, _ := criticalLifecycleTestWorld(t, queue)
			applied := 0
			id, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
				func(criticalPersistenceResult) error { applied++; return nil })
			if err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			switch scenario {
			case "player":
				replacement := *player
				world.players[player.Session] = &replacement
			case "session":
				player.Session = net.NewTestSession(player.Session.ID, 8)
			case "account-pointer":
				replacement := *player.Account
				player.Account = &replacement
			case "account-name":
				player.Account.Name = "other"
			case "account-owner":
				world.accountSessions["held"] = net.NewTestSession(2, 8)
			case "character-pointer":
				replacement := *player.Char
				player.Char = &replacement
			case "character-uid":
				player.Char.UID = "other-character"
			case "character-slot":
				player.CharSlot = -1
			case "id-index":
				world.playersByID[player.ID] = &Player{ID: player.ID}
			case "uid-index":
				world.playersByCharacterUID[player.Char.UID] = &Player{}
			case "world-membership":
				player.InWorld = false
			case "slice-reallocation":
				player.Account.Chars = append([]model.Char(nil), player.Account.Chars...)
			case "reset-and-same-character-reentry":
				character, slot, id := player.Char, player.CharSlot, player.ID
				resetCharacterRuntime(player)
				player.Char, player.CharSlot, player.ID, player.InWorld = character, slot, id, true
			case "poisoned":
				player.PersistencePoisoned = true
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			world.processCriticalCompletions(1)
			if applied != 0 || !world.shuttingDown || !errors.Is(world.shutdownErr, errCriticalPersistenceIdentity) ||
				queue.accounts["held"] != id || queue.characters["held-character"] != id || len(queue.pending) != 1 {
				t.Fatalf("stale completion applied or lost fences: applied=%d error=%v", applied, world.shutdownErr)
			}
		})
	}
}

func TestCriticalLifecycleDisconnectDefersCleanupUntilOwnerDelivery(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, player, finalStore := criticalLifecycleTestWorld(t, queue)
	session := player.Session
	applied := 0
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error {
			if world.players[session] != player || world.accountSessions["held"] != session ||
				!session.IsClosed() || !world.criticalPlayerReserved(player) || finalStore.saves != 0 {
				t.Fatal("disconnect cleanup preceded owner settlement")
			}
			applied++
			return nil
		}); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	world.handle(command{s: session})
	world.handle(command{s: session})
	// A queued logout must not save/reset the retained character.
	world.handle(command{s: session, pkt: inboundPacket(wire.OpCharacterLogout, 12)})
	if len(world.criticalDisconnects) != 1 || world.players[session] != player ||
		world.playersByID[player.ID] != player || world.playersByCharacterUID[player.Char.UID] != player ||
		player.PersistencePoisoned || !session.IsClosed() || finalStore.saves != 0 ||
		world.claimAccountSession(net.NewTestSession(2, 8), " HELD ") {
		t.Fatal("pending disconnect released identity, saved, poisoned or readmitted")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	if applied != 0 || finalStore.saves != 0 || len(world.criticalDisconnects) != 1 {
		t.Fatal("worker completion cleaned up before actor delivery")
	}
	world.processCriticalCompletions(1)
	awaitCriticalWrite(t, backend.entered)
	if applied != 1 || finalStore.saves != 0 || world.players[session] != player ||
		!world.criticalPlayerReserved(player) || world.claimAccountSession(net.NewTestSession(3, 8), "held") {
		t.Fatal("final save blocked World or released ownership before its acknowledgement")
	}
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if finalStore.saves != 0 || len(world.criticalDisconnects) != 0 ||
		world.players[session] != nil || world.playersByID[player.ID] != nil ||
		world.playersByCharacterUID[player.Char.UID] != nil || world.accountSessions["held"] != nil || world.shuttingDown {
		t.Fatal("settled disconnect omitted cleanup or repeated the final save")
	}
	world.processCriticalCompletions(1)
	if finalStore.saves != 0 || len(backend.entered) != 0 || !world.claimAccountSession(net.NewTestSession(3, 8), "held") {
		t.Fatal("settled account not readmitted or disconnect replayed")
	}
}

func TestCriticalLifecycleUnknownOutcomeDoesNotPublishOrReleaseDisconnect(t *testing.T) {
	gate, release := criticalGate(t)
	failure := errors.New("injected unknown durable outcome")
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate, failure: failure}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, player, finalStore := criticalLifecycleTestWorld(t, queue)
	applied := 0
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { applied++; return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	world.handle(command{s: player.Session})
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	world.handle(command{s: player.Session})
	world.resumeCriticalDisconnects(8)
	if applied != 0 || !errors.Is(world.shutdownErr, failure) || !player.PersistencePoisoned ||
		finalStore.saves != 0 || world.players[player.Session] != player || len(world.criticalDisconnects) != 1 ||
		world.accountSessions["held"] != player.Session || !world.criticalPlayerReserved(player) {
		t.Fatal("unknown commit published, overwritten or released disconnected identity")
	}
}

func TestCriticalLifecycleReservedPacketCannotLogout(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, player, finalStore := criticalLifecycleTestWorld(t, queue)
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	character, lifecycle := player.Char, player.criticalLifecycle
	world.handle(command{s: player.Session, pkt: inboundPacket(wire.OpCharacterLogout, 12)})
	if player.Char != character || player.criticalLifecycle != lifecycle || !player.InWorld ||
		player.Session.IsClosed() || finalStore.saves != 0 || player.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("reserved logout saved, reset or published")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	// Acknowledgement replays the deferred logout, which submits its own
	// detached write and leaves the world only after that commit.
	world.processCriticalCompletions(1)
	if !player.InWorld || player.Char != character {
		t.Fatal("deferred logout left the world before its own commit")
	}
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if player.InWorld || player.Char != nil || player.criticalLifecycle != nil || finalStore.saves != 0 {
		t.Fatal("logout did not resume after successful owner settlement")
	}
}

func TestCriticalLifecycleReservedMovementWaitsForOwnerAcknowledgement(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, player, _ := criticalLifecycleTestWorld(t, queue)
	clock := newFakeClock(time.Unix(100, 0))
	world.clock, world.terrain = clock, loadedFlatTerrain()
	player.MovePublished = true
	player.MoveAuthorityRoute = []byte("66")
	player.MoveAuthorityX, player.MoveAuthorityY = player.X, player.Y
	player.MoveAuthorityStepInterval = time.Second
	player.MoveAuthorityStartedAt = clock.Now()
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	clock.Advance(time.Second)
	world.handle(command{s: player.Session, pkt: inboundPacket(wire.OpPing, 12)})
	world.advanceAllPlayerMovement(clock.Now())
	if player.X != 2100 || player.MoveAuthorityStep != 0 {
		t.Fatal("reserved packet/tick movement advanced live state")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.advanceAllPlayerMovement(clock.Now())
	if player.X != 2100 || player.MoveAuthorityStep != 0 {
		t.Fatal("worker completion released movement before owner acknowledgement")
	}
	world.processCriticalCompletions(1)
	world.handle(command{s: player.Session, pkt: inboundPacket(wire.OpPing, 12)})
	if player.X != 2101 || player.MoveAuthorityStep != 1 {
		t.Fatalf("acknowledged movement did not resume: x=%d step=%d", player.X, player.MoveAuthorityStep)
	}
}

func TestCriticalLifecycleBusyAndFullAdmissionDoNotInstallTokens(t *testing.T) {
	for _, scenario := range []string{"busy", "full"} {
		t.Run(scenario, func(t *testing.T) {
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
			capacity := 1
			if scenario == "busy" {
				capacity = 2
			}
			queue := newCriticalTestQueue(t, backend, 1, capacity)
			t.Cleanup(release)
			world, player, _ := criticalLifecycleTestWorld(t, queue)
			held := player.Account
			wantError := errCriticalPersistenceBusy
			if scenario == "full" {
				held = &model.Account{Name: "unrelated"}
				wantError = errCriticalPersistenceFull
			}
			if _, err := world.criticalWrites.submit(criticalWrite{accounts: []*model.Account{held}}, func(criticalPersistenceResult) error { return nil }); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			id, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
				func(criticalPersistenceResult) error { t.Fatal("rejected callback ran"); return nil })
			if id != 0 || !errors.Is(err, wantError) || player.criticalLifecycle != nil || len(queue.pending) != 1 || len(world.criticalWrites.continuations) != 1 {
				t.Fatalf("rejected admission changed lifecycle/capacity: id=%d err=%v", id, err)
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			world.processCriticalCompletions(1)
		})
	}
}

func TestCriticalLifecycleValidatesEveryParticipantBeforePublishing(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalBatchTestStore{&criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, first, _ := criticalLifecycleTestWorld(t, queue)
	account := newShutdownAccount("second")
	account.Chars[0].UID = "second-character"
	second := addShutdownPlayer(world, 2, account)
	world.indexPlayerCharacter(second)
	if !world.claimAccountSession(second.Session, account.Name) {
		t.Fatal("second fixture admission failed")
	}
	applied := 0
	id, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{first.Account, second.Account}}, []*Player{first, second},
		func(criticalPersistenceResult) error { applied++; return nil })
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	second.Char.UID = "replacement-character"
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if applied != 0 || !errors.Is(world.shutdownErr, errCriticalPersistenceIdentity) ||
		queue.accounts["held"] != id || queue.accounts["second"] != id || !first.PersistencePoisoned || !second.PersistencePoisoned {
		t.Fatal("multi-account completion partially published or released one participant")
	}
}

func TestCriticalLifecycleSelectionSessionCanCommit(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	world, player, _ := criticalLifecycleTestWorld(t, queue)
	world.removePlayerFromWorld(player, "selection fixture")
	applied := 0
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { applied++; return nil }); err != nil {
		t.Fatal(err)
	}
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if applied != 1 || world.shuttingDown || player.InWorld || player.Char != nil || len(queue.pending) != 0 {
		t.Fatal("valid authenticated selection completion rejected or not released")
	}
}

func TestCriticalLifecycleDeferredDisconnectSurvivesExplicitShutdown(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	world, player, finalStore := criticalLifecycleTestWorld(t, queue)
	applied := 0
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { applied++; return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	world.handle(command{s: player.Session})
	done := make(chan error, 1)
	world.runShutdown(done)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if err := <-done; err != nil {
		t.Fatal(err)
	}
	world.tick()
	if applied != 1 || !world.shutdownFinished || world.players[player.Session] != player ||
		len(world.criticalDisconnects) != 1 || finalStore.saves != 1 || finalStore.flushes != 1 {
		t.Fatal("explicit shutdown repeated disconnect persistence or removed the final aggregate")
	}
}

func TestCriticalLifecycleMaintenanceSuppressesLaterSuccessfulPublication(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	world, player, finalStore := criticalLifecycleTestWorld(t, queue)
	applied := 0
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{player.Account}}, []*Player{player},
		func(criticalPersistenceResult) error { applied++; return nil }); err != nil {
		t.Fatal(err)
	}
	awaitReadyCriticalResults(t, queue, 1)
	priorError := errors.New("earlier operation outcome unknown")
	world.shuttingDown, world.shutdownErr = true, priorError
	world.processCriticalCompletions(1)
	if applied != 0 || !errors.Is(world.shutdownErr, priorError) || !player.PersistencePoisoned ||
		len(queue.pending) != 1 || finalStore.saves != 0 {
		t.Fatal("later successful result published into maintenance or lost the earlier failure")
	}
}
