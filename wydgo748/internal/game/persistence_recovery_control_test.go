package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/store"
)

// W4 item 10: staff see each quarantine and may start an exact-outcome check.
// The check releases accounts only on a final status, never by itself.
func TestPersistenceRecoveryStatusAndRecheck(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 0)
	release()
	w.criticalAccountContainment = true
	statuses := &statusStore{batchGameStore: fallback, status: "in progress"}
	w.store, w.commands = statuses, make(chan command, 4)
	backend.failure = &store.UnresolvedCommitError{XID: "4343", Cause: errors.New("commit acknowledgement lost")}
	runCriticalAutosaveSlice(w, p, w.now())
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)

	status := w.persistenceRecoverySnapshot()
	if len(status.Quarantined) != 1 || !status.Quarantined[0].Checkable || status.Quarantined[0].Shared ||
		len(status.Quarantined[0].Accounts) != 1 || status.Quarantined[0].Accounts[0] != accountSessionKey(p.Account.Name) {
		t.Fatalf("quarantine status: %+v", status.Quarantined)
	}
	recheck := func(authorized bool) control.PersistenceRecheckResult {
		t.Helper()
		req := &persistenceRecoveryRequest{ctx: context.Background(), actor: "operator",
			command:   &control.PersistenceRecheckCommand{Version: 1, OperationID: "operation-0000001", Reason: "support ticket"},
			authorize: func() bool { return authorized }, reply: make(chan control.PersistenceRecheckResult, 1)}
		w.quarantineCheckAt = w.now().Add(time.Hour) // the periodic check is not due
		w.handlePersistenceRecovery(req)
		return <-req.reply
	}
	deliver := func() {
		t.Helper()
		select {
		case cmd := <-w.commands:
			w.handle(cmd)
		case <-time.After(2 * time.Second):
			t.Fatal("quarantine status was not delivered")
		}
	}
	if result := recheck(false); result.Code != "capability_required" || result.Started != 0 || w.quarantineCheckRunning {
		t.Fatalf("unauthorized recheck: %+v", result)
	}
	if result := recheck(true); result.Code != "ok" || result.Started != 1 {
		t.Fatalf("recheck: %+v", result)
	}
	if result := recheck(true); result.Code != "check_running" {
		t.Fatalf("second recheck while running: %+v", result)
	}
	deliver()
	if status := w.persistenceRecoverySnapshot(); len(status.Quarantined) != 1 || status.Quarantined[0].LastStatus != "in progress" ||
		queue.quarantined != 1 {
		t.Fatalf("in-progress check released or lost status: %+v", status.Quarantined)
	}
	statuses.mu.Lock()
	statuses.status = "committed"
	statuses.mu.Unlock()
	if result := recheck(true); result.Started != 1 {
		t.Fatalf("recheck after unknown status: %+v", result)
	}
	deliver()
	if status := w.persistenceRecoverySnapshot(); len(status.Quarantined) != 0 || queue.quarantined != 0 {
		t.Fatalf("committed status did not release: %+v", status.Quarantined)
	}
	if result := recheck(true); result.Code != "nothing_to_check" {
		t.Fatalf("recheck with nothing quarantined: %+v", result)
	}
}
