package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func TestCriticalDisconnectAtomicSnapshotAndOwnershipUntilAcknowledgement(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
	atomic := &criticalStateTestStore{&criticalBatchTestStore{backend}}
	queue := newCriticalTestQueue(t, atomic, 1, 1)
	t.Cleanup(release)
	w, p, fallback := criticalLifecycleTestWorld(t, queue)
	p.SpecialCoins = map[string]uint32{"token": 9}
	p.Char.Affects[0] = model.Affect{Type: 1, Value: 7, ExpiresAt: w.now().Add(time.Hour)}
	expected := buildCharState(p, w.now())
	session, uid, id := p.Session, p.Char.UID, p.ID
	w.handle(command{s: session})
	write := awaitCriticalWrite(t, backend.entered)
	if write.character == nil || write.character.uid != uid || !reflect.DeepEqual(write.character.state, expected) ||
		write.accounts[0] == p.Account || &write.accounts[0].Chars[0] == p.Char {
		t.Fatal("disconnect did not submit a detached atomic account/sidecar snapshot")
	}
	write.character.state.SpecialCoins["token"] = 100
	write.character.state.Affects[0].Value = 100
	if p.SpecialCoins["token"] != 9 || p.Char.Affects[0].Value != 7 {
		t.Fatal("worker snapshot aliases actor state")
	}
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.handle(command{s: session})
		w.resumeCriticalDisconnects(8)
		if len(queue.pending) != 1 || len(backend.entered) != 0 || fallback.saves != 0 ||
			w.players[session] != p || w.playersByID[id] != p || w.playersByCharacterUID[uid] != p ||
			!w.criticalPlayerReserved(p) || w.claimAccountSession(net.NewTestSession(2, 8), "held") {
			t.Fatalf("%s repeated the save, blocked inline or released ownership", phase)
		}
	}
	w.processCriticalCompletions(1)
	if w.players[session] != nil || w.playersByID[id] != nil || w.playersByCharacterUID[uid] != nil ||
		len(w.criticalDisconnects) != 0 || len(queue.pending) != 0 || fallback.saves != 0 ||
		!w.claimAccountSession(net.NewTestSession(3, 8), "held") {
		t.Fatal("successful final acknowledgement did not clean up exactly once")
	}
}

func TestCriticalDisconnectFullQueueFencesIntentAndRetriesOnlyAdmission(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	w, p, fallback := criticalLifecycleTestWorld(t, queue)
	if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: "occupied"}}},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	p.Char.CP, p.NextCPRecovery = -2, w.now().Add(-time.Second)
	before, deadline := cloneCharacterState(p.Char), p.NextCPRecovery
	w.handle(command{s: p.Session})
	for range 3 {
		w.resumeCriticalDisconnects(8)
		w.tickChaosRecovery(w.now())
	}
	if !reflect.DeepEqual(*p.Char, before) || !p.NextCPRecovery.Equal(deadline) ||
		!w.criticalPlayerReserved(p) || w.criticalPlayerWriteReserved(p) ||
		!w.criticalAccountStateReserved(accountPersistenceSnapshot(p.Account)) ||
		fallback.saves != 0 || len(backend.entered) != 0 || p.criticalLifecycle != nil ||
		w.claimAccountSession(net.NewTestSession(2, 8), "held") {
		t.Fatal("full admission mutated, saved inline, installed a token or lost the intent fence")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	write := awaitCriticalWrite(t, backend.entered)
	if write.accounts[0].Chars[0].CP != -2 || !w.criticalPlayerWriteReserved(p) {
		t.Fatal("freed capacity did not admit the retained final snapshot")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.players[p.Session] != nil || fallback.saves != 0 {
		t.Fatal("admission retry did not finish without a synchronous account fallback")
	}
}

func TestCriticalDisconnectUnknownFinalOutcomeRetainsIdentityWithoutReplay(t *testing.T) {
	for _, scenario := range []string{"error", "panic", "identity"} {
		t.Run(scenario, func(t *testing.T) {
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
			if scenario == "error" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			backend.panicAfterWrite = scenario == "panic"
			queue := newCriticalTestQueue(t, backend, 1, 1)
			t.Cleanup(release)
			w, p, fallback := criticalLifecycleTestWorld(t, queue)
			w.handle(command{s: p.Session})
			awaitCriticalWrite(t, backend.entered)
			if scenario == "identity" {
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.handle(command{s: p.Session})
			w.resumeCriticalDisconnects(8)
			if !w.shuttingDown || w.shutdownErr == nil || !p.PersistencePoisoned || fallback.saves != 0 ||
				w.players[p.Session] != p || w.accountSessions["held"] != p.Session ||
				len(queue.pending) != 1 || len(backend.entered) != 0 || !w.criticalPlayerReserved(p) {
				t.Fatal("uncertain final persistence replayed, overwrote or released the aggregate")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 || fallback.flushes != 0 {
				t.Fatalf("terminal shutdown overwrote an unknown final outcome: %v", err)
			}
		})
	}
}

type criticalNonAtomicSidecarStore struct{ *criticalTestStore }

func (*criticalNonAtomicSidecarStore) LoadCharState(string) (*model.CharState, error) {
	return nil, nil
}
func (s *criticalNonAtomicSidecarStore) SaveCharState(string, *model.CharState) error {
	return s.record(criticalWrite{})
}

func TestCriticalDisconnectPermanentAdmissionRejectionRetainsWithoutFallback(t *testing.T) {
	for _, scenario := range []string{"stopping", "unsupported-sidecar", "identity"} {
		t.Run(scenario, func(t *testing.T) {
			backend := &criticalTestStore{entered: make(chan criticalWrite, 2)}
			queue := newCriticalTestQueue(t, backend, 1, 1)
			w, p, fallback := criticalLifecycleTestWorld(t, queue)
			switch scenario {
			case "stopping":
				queue.closeAdmission()
			case "unsupported-sidecar":
				queue.store = &criticalNonAtomicSidecarStore{backend}
			case "identity":
				p.CharSlot = 3
			}
			w.handle(command{s: p.Session})
			for range 3 {
				w.handle(command{s: p.Session})
				w.resumeCriticalDisconnects(8)
			}
			if len(queue.pending) != 0 || len(backend.entered) != 0 || fallback.saves != 0 ||
				p.criticalLifecycle != nil || !w.criticalPlayerReserved(p) || w.players[p.Session] != p ||
				w.criticalDisconnects[p.Session].admissionErr == nil || w.shuttingDown {
				t.Fatal("permanent rejection wrote, retried, released ownership or claimed unknown commit")
			}
		})
	}
}

func TestCriticalDisconnectShutdownDrainsAcceptedFinalWrite(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	w, p, terminal := criticalLifecycleTestWorld(t, queue)
	w.handle(command{s: p.Session})
	awaitCriticalWrite(t, backend.entered)
	done := make(chan error, 1)
	w.runShutdown(done)
	if terminal.saves != 0 || w.shutdownFinished {
		t.Fatal("terminal shutdown overtook the accepted final write")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if err := <-done; err != nil || terminal.saves != 1 || terminal.flushes != 1 ||
		!w.shutdownFinished || w.players[p.Session] != p || len(queue.pending) != 0 {
		t.Fatalf("terminal snapshot did not follow final acknowledgement: %v", err)
	}
}

func TestCriticalDisconnectSelectionScreenSavesOrSurvivesRejectedShutdown(t *testing.T) {
	for _, stopping := range []bool{false, true} {
		t.Run(map[bool]string{false: "accepted", true: "stopping"}[stopping], func(t *testing.T) {
			backend := &criticalTestStore{entered: make(chan criticalWrite, 2)}
			queue := newCriticalTestQueue(t, backend, 1, 1)
			w, p, terminal := criticalLifecycleTestWorld(t, queue)
			w.unindexPlayerCharacter(p)
			delete(w.playersByID, p.ID)
			p.InWorld, p.Char, p.ID, p.CharSlot = false, nil, 0, -1
			if stopping {
				queue.closeAdmission()
			}
			w.handle(command{s: p.Session})
			if stopping {
				done := make(chan error, 1)
				w.runShutdown(done)
				if err := <-done; err != nil || terminal.saves != 1 || terminal.flushes != 1 || len(backend.entered) != 0 {
					t.Fatalf("shutdown discarded a queue-rejected account-only disconnect: %v", err)
				}
				return
			}
			write := awaitCriticalWrite(t, backend.entered)
			if write.character != nil || write.accounts[0].Name != p.Account.Name {
				t.Fatal("selection-screen disconnect submitted the wrong aggregate")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || terminal.saves != 0 {
				t.Fatal("account-only disconnect failed cleanup or saved inline")
			}
		})
	}
}
