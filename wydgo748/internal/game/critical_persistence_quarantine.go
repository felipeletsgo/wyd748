package game

import (
	"errors"
	"log"
	"time"

	"wydgo/internal/store"
)

// Verified quarantine release (DOCS/server/server-correction-plan.md, W4).
// An unresolved COMMIT keeps its account fences and discards the affected RAM.
// When the store can name the exact transaction, a worker asks PostgreSQL for
// its final status later. "committed" and "aborted" are final: the durable
// state is then known, the discarded RAM is never saved, and releasing the
// fences lets the player sign in to the stored state. Shared objects, guilds
// and instances stay fenced; time alone never releases anything.

const quarantineCheckInterval = 10 * time.Second

type transactionStatusStore interface {
	TransactionStatus(xid string) (string, error)
}

type quarantineStatus struct {
	id     uint64
	status string
	err    error
}

// noteQuarantineTransaction records the exact transaction of an account-only
// quarantine whose store result named it.
func (w *World) noteQuarantineTransaction(id uint64, err error, accountOnly bool) {
	var unresolved *store.UnresolvedCommitError
	if !accountOnly || !errors.As(err, &unresolved) || unresolved.XID == "" {
		return
	}
	if w.criticalQuarantineXID == nil {
		w.criticalQuarantineXID = make(map[uint64]string)
	}
	w.criticalQuarantineXID[id] = unresolved.XID
}

// checkQuarantinedCommits starts at most one status worker per interval.
func (w *World) checkQuarantinedCommits(now time.Time) {
	if len(w.criticalQuarantineXID) == 0 || w.quarantineCheckRunning || now.Before(w.quarantineCheckAt) {
		return
	}
	statusStore, ok := w.store.(transactionStatusStore)
	if !ok || w.commands == nil {
		return
	}
	w.quarantineCheckAt = now.Add(quarantineCheckInterval)
	w.quarantineCheckRunning = true
	pending := make(map[uint64]string, len(w.criticalQuarantineXID))
	for id, xid := range w.criticalQuarantineXID {
		pending[id] = xid
	}
	go func() {
		results := make([]quarantineStatus, 0, len(pending))
		for id, xid := range pending {
			status, err := statusStore.TransactionStatus(xid)
			results = append(results, quarantineStatus{id: id, status: status, err: err})
		}
		w.commands <- command{queuedAt: time.Now(), quarantine: results}
	}()
}

func (w *World) onQuarantineStatus(results []quarantineStatus) {
	w.quarantineCheckRunning = false
	released := false
	for _, result := range results {
		if _, quarantined := w.criticalQuarantine[result.id]; quarantined {
			if w.quarantineLastStatus == nil {
				w.quarantineLastStatus = make(map[uint64]string)
			}
			status := result.status
			if result.err != nil {
				status = "read_error"
			} else if status == "" {
				status = "unknown"
			}
			w.quarantineLastStatus[result.id] = status
		}
		if result.err != nil || (result.status != "committed" && result.status != "aborted") {
			continue // still unknown: keep every fence
		}
		if w.releaseVerifiedQuarantine(result.id, result.status) {
			released = true
		}
	}
	if released {
		w.resumeCriticalDisconnects(criticalCompletionBatchLimit)
	}
}

// releaseVerifiedQuarantine drops the fences of one account-only quarantine
// whose transaction outcome is final.
func (w *World) releaseVerifiedQuarantine(id uint64, status string) bool {
	if w.criticalWrites == nil || w.criticalWrites.queue == nil {
		return false
	}
	queue := w.criticalWrites.queue
	pending := queue.pending[id]
	if pending == nil || !pending.quarantined || pending.instances || pending.guilds {
		delete(w.criticalQuarantineXID, id)
		return false
	}
	if continuation := w.criticalWrites.continuations[id]; continuation != nil && continuation.sharedWorld {
		delete(w.criticalQuarantineXID, id)
		return false
	}
	delete(queue.pending, id)
	// Another unresolved quarantine of the same participant keeps its fence.
	for _, key := range pending.keys {
		if queue.failedAccounts[key] == id {
			if other := queue.pendingHolding(id, key, false, true); other != 0 {
				queue.failedAccounts[key] = other
			} else {
				delete(queue.failedAccounts, key)
			}
		}
		if queue.accounts[key] == id {
			queue.returnKey(queue.accounts, pending, id, key, false)
		}
	}
	for _, uid := range pending.uids {
		if queue.failedCharacters[uid] == id {
			if other := queue.pendingHolding(id, uid, true, true); other != 0 {
				queue.failedCharacters[uid] = other
			} else {
				delete(queue.failedCharacters, uid)
			}
		}
		if queue.characters[uid] == id {
			queue.returnKey(queue.characters, pending, id, uid, true)
		}
	}
	queue.quarantined--
	delete(w.criticalWrites.continuations, id)
	delete(w.criticalQuarantine, id)
	delete(w.criticalQuarantineXID, id)
	delete(w.quarantineLastStatus, id)
	metricCriticalQuarantineReleased.Add(1)
	log.Printf("critical persistence: completion %d verified as %s; released %d account fence(s) for relogin",
		id, status, len(pending.keys))
	return true
}
