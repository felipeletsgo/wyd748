package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// W7: a background autosave holds the persistence boundary but not input. A
// later write of the same account parks and starts only after the snapshot
// is acknowledged, so an older snapshot never commits after a newer write.

func TestCriticalBackgroundParksLaterWriteUntilAcknowledged(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	q := newCriticalTestQueue(t, backend, 2, 4)
	t.Cleanup(release)
	snapshot, err := q.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "Account", CargoGold: 1}}})
	if err != nil {
		t.Fatal(err)
	}
	if first := awaitCriticalWrite(t, backend.entered); first.accounts[0].CargoGold != 1 {
		t.Fatal("background snapshot did not start first")
	}
	if q.holdsInput(q.accounts["account"]) {
		t.Fatal("background snapshot holds gameplay input")
	}
	if _, err := q.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "account"}}}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("second background snapshot: %v", err)
	}
	newer, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "ACCOUNT", CargoGold: 2}}})
	if err != nil {
		t.Fatalf("write behind background snapshot: %v", err)
	}
	if !q.holdsInput(q.accounts["account"]) || q.pending[newer].parked == nil || q.parked != 1 {
		t.Fatal("parked write does not own the account")
	}
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "account"}}}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("write behind a parked write: %v", err)
	}
	release()
	if result := awaitCriticalResult(t, q); result.id != snapshot || result.err != nil {
		t.Fatalf("snapshot result: %+v", result)
	}
	select {
	case write := <-backend.entered:
		t.Fatalf("parked write started before acknowledgement: %+v", write)
	case <-time.After(20 * time.Millisecond):
	}
	if err := q.acknowledge(snapshot); err != nil {
		t.Fatal(err)
	}
	if write := awaitCriticalWrite(t, backend.entered); write.accounts[0].CargoGold != 2 {
		t.Fatal("parked write lost its newer snapshot")
	}
	if result := awaitCriticalResult(t, q); result.id != newer || result.err != nil {
		t.Fatalf("parked result: %+v", result)
	}
	if err := q.acknowledge(newer); err != nil || len(q.accounts) != 0 || q.parked != 0 {
		t.Fatalf("parked acknowledgement: %v accounts=%d parked=%d", err, len(q.accounts), q.parked)
	}
}

func TestCriticalBackgroundRefusedBehindOrdinaryWrite(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
	q := newCriticalTestQueue(t, backend, 1, 3)
	t.Cleanup(release)
	id, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "account"}}})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	if _, err := q.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "account"}}}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("background behind ordinary write: %v", err)
	}
	if _, err := q.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "a"}, {Name: "b"}}}); err == nil {
		t.Fatal("background snapshot accepted two accounts")
	}
	release()
	awaitCriticalResult(t, q)
	if err := q.acknowledge(id); err != nil {
		t.Fatal(err)
	}
}

func TestCriticalBackgroundMultiplePredecessorsAndClosedAdmission(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	q := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 2, 4)
	t.Cleanup(release)
	first, err := q.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "a"}}})
	if err != nil {
		t.Fatal(err)
	}
	second, err := q.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "b"}}})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	awaitCriticalWrite(t, backend.entered)
	both, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "a"}, {Name: "b"}}})
	if err != nil || q.pending[both].waits != 2 {
		t.Fatalf("two-account write behind two snapshots: %v", err)
	}
	// Admission closes with a parked job: the job channel must stay open.
	q.closeAdmission()
	if q.jobsClosed {
		t.Fatal("admission closed the job channel under a parked write")
	}
	release()
	awaitCriticalResult(t, q)
	awaitCriticalResult(t, q)
	if err := q.acknowledge(first); err != nil || q.pending[both].parked == nil {
		t.Fatalf("parked write started with one predecessor pending: %v", err)
	}
	if err := q.acknowledge(second); err != nil {
		t.Fatal(err)
	}
	if write := awaitCriticalWrite(t, backend.entered); len(write.accounts) != 2 {
		t.Fatal("parked two-account write did not start")
	}
	if result := awaitCriticalResult(t, q); result.id != both || result.err != nil || !q.jobsClosed {
		t.Fatalf("parked result %+v closed=%t", result, q.jobsClosed)
	}
}

func TestCriticalBackgroundFailureRefusesParkedWriteUnsent(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate,
		failure: errors.New("unknown snapshot outcome")}
	q := newCriticalTestQueue(t, backend, 1, 3)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(q)
	snapshot, err := owner.submitBackground(criticalWrite{accounts: []*model.Account{{Name: "account"}}},
		func(result criticalPersistenceResult) error { return result.err },
		func(criticalPersistenceResult) error { return nil })
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	var parkedResult error
	parked, err := owner.submitWithRejection(criticalWrite{accounts: []*model.Account{{Name: "account"}}},
		func(result criticalPersistenceResult) error { return result.err }, nil,
		func(result criticalPersistenceResult) error { parkedResult = result.err; return nil })
	if err != nil {
		t.Fatal(err)
	}
	release()
	result := awaitCriticalResult(t, q)
	if result.id != snapshot || store.IsWriteRejected(result.err) {
		t.Fatalf("snapshot result: %+v", result)
	}
	if err := owner.complete(result); err == nil {
		t.Fatal("unknown snapshot outcome settled")
	}
	if !q.holdsInput(snapshot) {
		t.Fatal("failed snapshot does not hold input")
	}
	refused := awaitCriticalResult(t, q)
	if refused.id != parked || !store.IsUnsentWrite(refused.err) || len(backend.entered) != 0 {
		t.Fatalf("parked write after failed snapshot: %+v entered=%d", refused, len(backend.entered))
	}
	if err := owner.complete(refused); err != nil || !store.IsUnsentWrite(parkedResult) {
		t.Fatalf("unsent rejection did not settle through its continuation: %v %v", err, parkedResult)
	}
	if q.accounts["account"] != snapshot || q.parked != 0 {
		t.Fatal("unsent rejection released the failed snapshot's fence")
	}
}

// A trade between a player whose autosave is in flight and a partner waits
// behind the snapshot, then commits the newer state. Input is not deferred.
func TestCriticalTradeParksBehindPartnerAutosave(t *testing.T) {
	w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, 0, 0, "none")
	beforeA, beforeB := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
	wantA, wantB := criticalTradeExpected(a, b, beforeA, beforeB, "none")
	runCriticalAutosaveSlice(w, a, w.now())
	if snapshot := awaitCriticalWrite(t, backend.entered); len(snapshot.accounts) != 1 || snapshot.character == nil {
		t.Fatal("autosave did not start")
	}
	if w.criticalPlayerReserved(a) || !w.criticalPlayerWriteReserved(a) ||
		w.deferCriticalCommand(command{s: a.Session, pkt: []byte{1}}) {
		t.Fatal("autosave held gameplay input or lost its persistence boundary")
	}
	w.commitTrade(a, b)
	if len(queue.pending) != 2 || len(backend.entered) != 0 || !w.criticalPlayerReserved(a) || !w.criticalPlayerReserved(b) {
		t.Fatal("trade did not park behind the autosave with both participants held")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	proposal := awaitCriticalWrite(t, backend.entered)
	if !reflect.DeepEqual(proposal.accounts, accountPersistenceSnapshots(wantA, wantB)) {
		t.Fatal("parked trade lost its proposal")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !reflect.DeepEqual(accountStateSnapshot(a.Account), wantA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), wantB) ||
		len(queue.pending) != 0 || w.criticalPlayerReserved(a) || w.criticalPlayerReserved(b) ||
		fallback.saves != 0 || fallback.batchSaves != 0 {
		t.Fatal("parked trade did not settle exactly once")
	}
	assertCriticalTradePublication(t, a)
	assertCriticalTradePublication(t, b)
}

// Double fault: the autosave outcome is unknown while a trade is parked. The
// trade never reaches storage, its participants are isolated, and only the
// unknown autosave keeps a fence until its exact outcome is verified.
func TestCriticalParkedTradeAfterUnknownAutosaveReleasesOnlyPartner(t *testing.T) {
	w, a, b, _, backend, queue, release := criticalTradeFixture(t, 0, 0, "none")
	w.criticalAccountContainment = true
	backend.failure = errors.New("unknown autosave outcome")
	runCriticalAutosaveSlice(w, a, w.now())
	awaitCriticalWrite(t, backend.entered)
	snapshot := queue.accounts[accountSessionKey(a.Account.Name)]
	w.commitTrade(a, b)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	aKey, bKey := accountSessionKey(a.Account.Name), accountSessionKey(b.Account.Name)
	if len(backend.entered) != 0 || !a.PersistencePoisoned || !a.Session.IsClosed() || !b.Session.IsClosed() {
		t.Fatal("parked trade reached storage or its participants stayed live")
	}
	if queue.failedAccounts[aKey] != snapshot || queue.failedAccounts[bKey] != 0 || len(queue.pending) != 1 ||
		w.criticalAccountReserved(b.Account.Name) || !w.criticalAccountReserved(a.Account.Name) {
		t.Fatalf("fences after unsent trade: a=%d b=%d pending=%d", queue.failedAccounts[aKey], queue.failedAccounts[bKey], len(queue.pending))
	}
	if !w.releaseVerifiedQuarantine(snapshot, "aborted") || w.criticalAccountReserved(a.Account.Name) || len(queue.pending) != 0 {
		t.Fatal("verified autosave outcome did not release the last fence")
	}
}

// The player may leave the world or change character while the snapshot is
// written. Completion acknowledges durability only and isolates nobody.
func TestCriticalBackgroundAutosaveSettlesAfterIdentityChange(t *testing.T) {
	w, p, _, backend, queue, release := criticalAutosaveFixture(t, 0)
	w.criticalAccountContainment = true
	runCriticalAutosaveSlice(w, p, w.now())
	awaitCriticalWrite(t, backend.entered)
	p.criticalLifecycle = &criticalPlayerLifecycle{}
	p.InWorld = false
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.PersistencePoisoned || p.Session.IsClosed() || len(queue.pending) != 0 || w.criticalPlayerWriteReserved(p) ||
		len(w.criticalQuarantine) != 0 {
		t.Fatal("identity change during autosave isolated the player or kept its fence")
	}
}
