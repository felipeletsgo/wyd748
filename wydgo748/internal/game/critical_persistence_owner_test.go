package game

import (
	"context"
	"errors"
	"fmt"
	"runtime"
	"testing"
	"time"

	"wydgo/internal/model"
)

func awaitReadyCriticalResults(t *testing.T, queue *criticalPersistence, count int) {
	t.Helper()
	deadline := time.Now().Add(2 * time.Second)
	for time.Now().Before(deadline) {
		if len(queue.results) >= count {
			return
		}
		runtime.Gosched()
	}
	t.Fatalf("critical results ready: %d, want %d", len(queue.results), count)
}

func TestCriticalOwnerRunsOnlyOnExplicitOwnerDelivery(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	account := &model.Account{Name: "one"}
	applied := 0
	id, err := owner.submit(criticalWrite{accounts: []*model.Account{account}}, func(result criticalPersistenceResult) error {
		applied++
		if result.err != nil {
			t.Fatalf("successful commit: %v", result.err)
		}
		if _, err := owner.submit(criticalWrite{accounts: []*model.Account{account}}, func(criticalPersistenceResult) error { return nil }); !errors.Is(err, errCriticalPersistenceBusy) {
			t.Fatalf("fence released before continuation settled: %v", err)
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	world := &World{criticalWrites: owner}
	world.processCriticalCompletions(criticalCompletionBatchLimit)
	if applied != 0 {
		t.Fatal("continuation ran before commit")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	if applied != 0 {
		t.Fatal("worker invoked live-state continuation")
	}
	world.processCriticalCompletions(1)
	if applied != 1 || len(queue.pending) != 0 || len(owner.continuations) != 0 {
		t.Fatalf("settled completion: applied=%d queued=%d retained=%d", applied, len(queue.pending), len(owner.continuations))
	}
	if err := owner.complete(criticalPersistenceResult{id: id}); err == nil || applied != 1 {
		t.Fatal("duplicate completion was accepted or callback replayed")
	}
}

func TestCriticalOwnerRejectsMissingContinuationBeforeWriting(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	owner := newCriticalPersistenceOwner(queue)
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "one"}}}, nil); err == nil {
		t.Fatal("missing continuation admitted")
	}
	if len(queue.pending) != 0 || len(owner.continuations) != 0 || len(backend.entered) != 0 {
		t.Fatal("rejection caused a write or retained state")
	}
	if _, err := (*criticalPersistenceOwner)(nil).submit(criticalWrite{}, nil); err == nil {
		t.Fatal("missing owner admitted")
	}
}

func TestCriticalOwnerPostAcknowledgementOrderingAndFailureContainment(t *testing.T) {
	for _, mode := range []string{"success", "store-error", "apply-error", "post-error", "post-panic"} {
		t.Run(mode, func(t *testing.T) {
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
			if mode == "store-error" {
				backend.failure = errors.New("unknown store outcome")
			}
			queue := newCriticalTestQueue(t, backend, 1, 1)
			t.Cleanup(release)
			owner := newCriticalPersistenceOwner(queue)
			world := &World{criticalWrites: owner}
			applied, published := 0, 0
			id, err := owner.submitPhased(criticalWrite{accounts: []*model.Account{{Name: "one"}}},
				func(result criticalPersistenceResult) error {
					applied++
					if queue.accounts["one"] == 0 || len(queue.pending) != 1 {
						t.Fatal("apply ran outside its reservation")
					}
					if result.err != nil {
						return result.err
					}
					if mode == "apply-error" {
						return errors.New("apply failed")
					}
					return nil
				}, func(criticalPersistenceResult) error {
					published++
					if applied != 1 || queue.accounts["one"] != 0 || len(queue.pending) != 0 {
						t.Fatal("post-ack phase ran before apply/acknowledgement")
					}
					if mode == "post-error" {
						return errors.New("publication failed")
					}
					if mode == "post-panic" {
						panic("publication failed")
					}
					return nil
				})
			if err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			world.processCriticalCompletions(1)
			if applied != 0 || published != 0 {
				t.Fatal("held write ran an actor phase")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			if applied != 0 || published != 0 {
				t.Fatal("worker ran an actor phase")
			}
			world.processCriticalCompletions(1)
			wantPublished := 1
			if mode == "store-error" || mode == "apply-error" {
				wantPublished = 0
				if len(queue.pending) != 1 || queue.accounts["one"] != id {
					t.Fatal("uncertain/unapplied outcome released its fences")
				}
			}
			if applied != 1 || published != wantPublished || world.shuttingDown != (mode != "success") {
				t.Fatal("phase count or maintenance containment differs")
			}
			if mode == "success" {
				if len(owner.continuations) != 0 {
					t.Fatal("successful phases retained continuations")
				}
			} else {
				retained := owner.continuations[id]
				if retained == nil || retained.err == nil || retained.apply != nil || retained.afterAck != nil {
					t.Fatal("failed phase lost its evidence or retained live callbacks")
				}
			}
			if err := owner.complete(criticalPersistenceResult{id: id}); err == nil || published != wantPublished {
				t.Fatal("duplicate completion replayed post-ack effects")
			}
		})
	}
}

func TestCriticalOwnerRetainsFailedOutcomesWithoutReplay(t *testing.T) {
	for _, mode := range []string{"store-error", "continuation-error", "continuation-panic"} {
		t.Run(mode, func(t *testing.T) {
			failure := errors.New("injected failure")
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
			if mode == "store-error" {
				backend.failure = failure
			}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			owner := newCriticalPersistenceOwner(queue)
			applied := 0
			id, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "one"}}}, func(criticalPersistenceResult) error {
				applied++
				switch mode {
				case "continuation-error":
					return failure
				case "continuation-panic":
					panic("injected continuation panic")
				}
				return nil
			})
			if err != nil {
				t.Fatal(err)
			}
			result := awaitCriticalResult(t, queue)
			if err := owner.complete(result); err == nil {
				t.Fatal("failed outcome released")
			}
			retained := owner.continuations[id]
			if applied != 1 || retained == nil || !retained.delivered || retained.apply != nil || retained.err == nil {
				t.Fatal("failed outcome lost, retained closure, or callback count changed")
			}
			if len(queue.pending) != 1 || queue.accounts["one"] != id {
				t.Fatal("failed outcome lost fences")
			}
			if err := owner.complete(result); err == nil || applied != 1 {
				t.Fatal("failed continuation was replayed")
			}
			if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "ONE"}}}, func(criticalPersistenceResult) error { return nil }); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatalf("failed account readmitted: %v", err)
			}
		})
	}
}

func TestCriticalWorldCompletionDrainIsBounded(t *testing.T) {
	const count = criticalCompletionBatchLimit + 3
	backend := &criticalTestStore{entered: make(chan criticalWrite, count)}
	queue := newCriticalTestQueue(t, backend, 1, count)
	owner := newCriticalPersistenceOwner(queue)
	applied := 0
	for i := range count {
		if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: fmt.Sprintf("account-%d", i)}}}, func(criticalPersistenceResult) error { applied++; return nil }); err != nil {
			t.Fatal(err)
		}
	}
	awaitReadyCriticalResults(t, queue, count)
	world := &World{criticalWrites: owner}
	world.processCriticalCompletions(criticalCompletionBatchLimit)
	if applied != criticalCompletionBatchLimit || len(queue.pending) != 3 {
		t.Fatal("unbounded or incomplete completion drain")
	}
	world.processCriticalCompletions(criticalCompletionBatchLimit)
	if applied != count || len(queue.pending) != 0 {
		t.Fatal("remaining completions lost")
	}
}

func TestCriticalWorldBatchAndTicksAdvanceDuringHeldSave(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	applied := false
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "held"}}}, func(criticalPersistenceResult) error { applied = true; return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	world := worldWithNetworkedPlayers()
	world.criticalWrites = owner
	world.commands = make(chan command, 8)
	reply := make(chan map[string]bool, 1)
	request := command{accountPresence: &accountPresenceRequest{ctx: context.Background(), accounts: []string{"unrelated"}, reply: reply}}
	world.processCommandBatch(request, make(chan time.Time))
	select {
	case <-reply:
	default:
		t.Fatal("unrelated command did not progress during held save")
	}
	world.tick()
	if world.mobTickCounter != 1 || applied {
		t.Fatal("tick stalled or completion applied before commit")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCommandBatch(request, make(chan time.Time))
	if !applied || len(queue.pending) != 0 {
		t.Fatal("busy command path did not apply ready completion")
	}
}

func TestCriticalWorldFailureEntersMaintenanceAndPreservesShutdownError(t *testing.T) {
	failure := errors.New("unknown durable outcome")
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: failure}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	owner := newCriticalPersistenceOwner(queue)
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "one"}}}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitReadyCriticalResults(t, queue, 1)
	world, player, _ := handlerTestWorld(t)
	world.criticalWrites = owner
	world.processCriticalCompletions(1)
	if !world.shuttingDown || !queue.stopping || !player.PersistencePoisoned || !player.Session.IsClosed() {
		t.Fatal("uncertain persistence did not fail closed")
	}
	if !errors.Is(world.shutdownErr, failure) || len(queue.pending) != 1 {
		t.Fatal("uncertain outcome lost or acknowledged")
	}
	done := make(chan error, 1)
	world.runShutdown(done)
	if err := <-done; !errors.Is(err, failure) {
		t.Fatalf("shutdown hid uncertain outcome: %v", err)
	}
}

func TestCriticalWorldDormantOwnerHasNoActiveChannel(t *testing.T) {
	world := &World{}
	if world.criticalCompletionChannel() != nil {
		t.Fatal("dormant completion channel is active")
	}
	world.processCriticalCompletions(criticalCompletionBatchLimit)
}
