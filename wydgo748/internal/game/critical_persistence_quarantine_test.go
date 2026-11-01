package game

import (
	"errors"
	"sync"
	"testing"
	"time"

	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
)

type statusStore struct {
	*batchGameStore
	mu     sync.Mutex
	status string
	asked  []string
}

func (s *statusStore) TransactionStatus(xid string) (string, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.asked = append(s.asked, xid)
	return s.status, nil
}

// W4: an account-only quarantine keeps its fences while the exact transaction
// is unresolved and releases them once PostgreSQL reports a final status.
func TestVerifiedQuarantineReleaseNeedsFinalTransactionStatus(t *testing.T) {
	for _, final := range []string{"committed", "aborted"} {
		t.Run(final, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 0)
			release()
			w.criticalAccountContainment = true
			statuses := &statusStore{batchGameStore: fallback, status: "in progress"}
			w.store, w.commands = statuses, make(chan command, 4)
			backend.failure = &store.UnresolvedCommitError{XID: "4242", Cause: errors.New("commit acknowledgement lost")}
			runCriticalAutosaveSlice(w, p, w.now())
			awaitCriticalWrite(t, backend.entered)
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			key := accountSessionKey(p.Account.Name)
			if queue.quarantined != 1 || queue.failedAccounts[key] == 0 || w.criticalQuarantineXID == nil || !p.Session.IsClosed() {
				t.Fatalf("unresolved commit was not quarantined with its transaction: quarantined=%d xid=%v", queue.quarantined, w.criticalQuarantineXID)
			}
			check := func(status string) {
				t.Helper()
				statuses.mu.Lock()
				statuses.status = status
				statuses.mu.Unlock()
				w.quarantineCheckAt = time.Time{}
				w.checkQuarantinedCommits(w.now())
				select {
				case cmd := <-w.commands:
					w.handle(cmd)
				case <-time.After(2 * time.Second):
					t.Fatal("quarantine status was not delivered")
				}
			}
			check("in progress")
			if queue.quarantined != 1 || queue.failedAccounts[key] == 0 {
				t.Fatal("an in-progress transaction released the quarantine")
			}
			check(final)
			if queue.quarantined != 0 || queue.failedAccounts[key] != 0 || len(queue.pending) != 0 || len(w.criticalQuarantineXID) != 0 {
				t.Fatalf("final status %q did not release the account fences", final)
			}
			w.onDisconnect(p.Session)
			if w.criticalAccountReserved(p.Account.Name) || !w.claimAccountSession(gameNet.NewTestSession(77, 8), p.Account.Name) {
				t.Fatal("the account could not sign in again after verified release")
			}
			if fallback.saves != 0 || fallback.batchSaves != 0 {
				t.Fatal("discarded RAM was saved after the release")
			}
		})
	}
}
