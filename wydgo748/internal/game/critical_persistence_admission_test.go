package game

import (
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

// Exercise actor admission, the authoritative reload, and character entry.
// A listening port or an account claim alone cannot prove usable admission.
func assertCriticalFreshAdmission(t *testing.T, w *World, backend *criticalTestStore, id int64) {
	t.Helper()
	session := newLoginTestSession(id, 128)
	account := newShutdownAccount(fmt.Sprintf("fresh%d", id))
	account.Chars[0].UID = account.Name + "-character"
	previousStore := w.store
	w.store = &loginReloadStore{account: account}
	defer func() { w.store = previousStore }()
	if w.authPending == nil {
		w.authPending = make(map[*gameNet.Session]bool)
	}
	w.authPending[session] = true
	w.handle(command{s: session, login: &loginResult{accountName: account.Name, account: accountStateSnapshot(account)}})
	p := w.players[session]
	if p == nil || session.IsClosed() || w.accountSessions[accountSessionKey(account.Name)] != session {
		t.Fatal("a persistence failure blocked a new authenticated connection")
	}
	w.handle(command{s: session, pkt: enterWorldPacket(0)})
	if w.criticalAccountReserved(account.Name) {
		awaitCriticalWrite(t, backend.entered)
		awaitReadyCriticalResults(t, w.criticalWrites.queue, 1)
		w.processCriticalCompletions(1)
	}
	if !p.InWorld || p.Char == nil || w.playersByID[p.ID] != p || session.IsClosed() || w.shuttingDown {
		t.Fatal("a new authenticated connection could not enter the world")
	}
}

func TestCriticalSharedIdentityFailurePreservesAdmissionAcrossChannels(t *testing.T) {
	world, held, bystander, queue, backend, _, release := containedCriticalWorld(t, nil)
	other, _ := newShutdownWorld()
	other.store, other.channel, world.channel = world.store, 2, 1
	other.criticalWrites = world.criticalWrites
	other.criticalAccountContainment = true
	cluster, err := NewChannelCluster(world.store)
	if err != nil {
		t.Fatal(err)
	}
	cluster.owner = world.criticalWrites
	for _, w := range []*World{world, other} {
		if err := cluster.attach(w); err != nil {
			t.Fatal(err)
		}
	}
	cluster.enter(world)
	applied := 0
	id, err := world.submitCriticalSharedWriteWithRejection(
		criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}}, []*Player{held},
		func(criticalPersistenceResult) error { applied++; return nil }, nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	// Reproduce the incident's stale participant guard after a confirmed write.
	held.criticalLifecycle = &criticalPlayerLifecycle{}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	cluster.enter(other)
	other.processCriticalCompletions(1)
	if applied != 0 || !held.PersistencePoisoned || !held.Session.IsClosed() ||
		bystander.Session.IsClosed() || bystander.PersistencePoisoned || world.shuttingDown || other.shuttingDown || queue.stopping {
		t.Fatal("a stale shared participant stopped another player or channel")
	}
	for i, w := range []*World{world, other} {
		cluster.enter(w)
		assertCriticalFreshAdmission(t, w, backend, int64(100+i))
		w.nextAutoSave = w.now().Add(time.Hour)
		w.tick()
	}
	world.applyCriticalCompletion(criticalPersistenceResult{id: id})
	world.applyCriticalCompletion(criticalPersistenceResult{id: 999999})
	if world.shuttingDown || other.shuttingDown || !world.criticalAccountReserved(held.Account.Name) || queue.quarantined != 1 {
		t.Fatal("late or duplicate completion stopped admission or released failed ownership")
	}
}

func TestCriticalQuarantineDoesNotExhaustLiveCapacity(t *testing.T) {
	world, held, bystander, queue, backend, _, release := containedCriticalWorld(t, nil)
	queue.capacity = 1
	release()
	for i := range 6 {
		p := held
		if i != 0 {
			account := newShutdownAccount(fmt.Sprintf("failed%d", i))
			account.Chars[0].UID = account.Name + "-character"
			p = addShutdownPlayer(world, uint16(10+i), account)
			claimCriticalTestPlayers(t, world, p)
		}
		id, err := world.submitCriticalSharedWriteWithRejection(
			criticalWrite{accounts: []*model.Account{accountStateSnapshot(p.Account)}}, []*Player{p},
			func(criticalPersistenceResult) error { return errCriticalPersistenceIdentity }, nil, nil)
		if err != nil {
			t.Fatalf("quarantined completions consumed live capacity: %v", err)
		}
		awaitCriticalWrite(t, backend.entered)
		awaitReadyCriticalResults(t, queue, 1)
		world.processCriticalCompletions(1)
		if queue.acknowledge(id) == nil || !world.criticalAccountReserved(p.Account.Name) {
			t.Fatal("quarantined ownership could be acknowledged without reconciliation")
		}
	}
	if _, err := world.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(bystander.Account)}},
		[]*Player{bystander}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	assertCriticalFreshAdmission(t, world, backend, 100)
	if queue.quarantined != 6 || len(queue.pending) != 6 || world.shuttingDown || bystander.Session.IsClosed() {
		t.Fatal("repeated failures lost fences or stopped healthy work")
	}
}

func TestCriticalPostAckFailureRestoresFencesWithoutStoppingAdmission(t *testing.T) {
	for _, panicAfterAck := range []bool{false, true} {
		t.Run(fmt.Sprint(panicAfterAck), func(t *testing.T) {
			world, held, bystander, queue, backend, _, release := containedCriticalWorld(t, nil)
			published := 0
			id, err := world.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
				[]*Player{held}, false, func(criticalPersistenceResult) error { return nil }, func(criticalPersistenceResult) error {
					published++
					if panicAfterAck {
						panic("publication failed")
					}
					return errors.New("publication failed")
				})
			if err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			release()
			awaitReadyCriticalResults(t, queue, 1)
			world.processCriticalCompletions(1)
			world.applyCriticalCompletion(criticalPersistenceResult{id: id})
			if published != 1 || queue.pending[id] == nil || !world.criticalAccountReserved("held") ||
				!world.criticalCharacterReserved("held-character") || world.claimAccountSession(gameNet.NewTestSession(99, 8), "held") ||
				world.shuttingDown || bystander.Session.IsClosed() || !held.Session.IsClosed() {
				t.Fatal("post-ack failure released ownership, replayed publication, or stopped another session")
			}
			if _, err := queue.submit(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}}); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatalf("a failed publication allowed a stale account overwrite: %v", err)
			}
			assertCriticalFreshAdmission(t, world, backend, 100)
		})
	}
}

func TestCriticalPostAckSuccessorCannotClearQuarantine(t *testing.T) {
	world, held, _, queue, backend, _, release := containedCriticalWorld(t, nil)
	var successor uint64
	_, err := world.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}},
		[]*Player{held}, false, func(criticalPersistenceResult) error { return nil }, func(criticalPersistenceResult) error {
			var err error
			successor, err = queue.submit(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}})
			if err != nil {
				return err
			}
			return errors.New("publication failed after admitting a follow-up write")
		})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	awaitCriticalWrite(t, backend.entered)
	awaitCriticalResult(t, queue)
	if successor == 0 || queue.acknowledge(successor) != nil || queue.accounts["held"] != 0 ||
		!world.criticalAccountReserved("held") || !world.criticalCharacterReserved("held-character") {
		t.Fatal("a successor acknowledgement released the earlier failure's fences")
	}
	if _, err := queue.submit(criticalWrite{accounts: []*model.Account{accountStateSnapshot(held.Account)}}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("successor completion allowed a quarantined account write: %v", err)
	}
	assertCriticalFreshAdmission(t, world, backend, 100)
}

func TestCriticalAggregateFailurePreservesFreshAdmission(t *testing.T) {
	for _, aggregate := range []string{"instances", "guilds"} {
		for _, phase := range []string{"store", "post-ack"} {
			t.Run(aggregate+"/"+phase, func(t *testing.T) {
				world, _, bystander, queue, backend, _, release := containedCriticalWorld(t, nil)
				queue.store = &criticalGameStateTestStore{backend}
				write := criticalWrite{}
				if aggregate == "instances" {
					queue.store = &criticalInstanceTestStore{backend}
					write.instances = world.instanceStateSnapshot()
				} else {
					write.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
				}
				if phase == "store" {
					backend.failure = errors.New("shared aggregate acknowledgement unavailable")
				}
				_, err := world.criticalWrites.submitPhased(write, func(result criticalPersistenceResult) error { return result.err },
					func(criticalPersistenceResult) error { return errors.New("shared aggregate publication failed") })
				if err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, backend.entered)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				world.processCriticalCompletions(1)
				backend.failure = nil
				if world.shuttingDown || world.shutdownErr != nil || bystander.Session.IsClosed() ||
					aggregate == "instances" && !world.criticalInstanceReserved() || aggregate == "guilds" && !world.criticalGuildReserved() {
					t.Fatal("shared aggregate failure stopped admission or released aggregate ownership")
				}
				if _, err := queue.submit(write); !errors.Is(err, errCriticalPersistenceBusy) {
					t.Fatalf("failed shared aggregate allowed an overwrite: %v", err)
				}
				assertCriticalFreshAdmission(t, world, backend, 100)
			})
		}
	}
}
