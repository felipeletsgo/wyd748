package game

import (
	"log"
	"time"
)

// WithCriticalAccountContainment selects account-local containment: an
// unknown or unverifiable account-only outcome isolates only its participants
// instead of putting the whole World into maintenance. Shared guild and
// instance aggregates and shared gameplay objects still fail globally, because
// their RAM cannot be reconciled per account.
func WithCriticalAccountContainment() WorldOption {
	return func(w *World) { w.criticalAccountContainment = true }
}

// containCriticalFailureLocally discards the RAM of every participant of a
// failed account-only operation: they are poisoned and disconnected without a
// final save. Fences and queue capacity remain held for verified reconciliation,
// never elapsed time. It returns false when containment must be global.
func (w *World) containCriticalFailureLocally(id uint64, err error) bool {
	if !w.criticalAccountContainment || w.criticalWrites == nil {
		return false
	}
	queue := w.criticalWrites.queue
	pending := queue.pending[id]
	continuation := w.criticalWrites.continuations[id]
	if pending == nil || continuation == nil || continuation.sharedWorld || queue.instances == id || queue.guilds == id {
		// Post-acknowledgement failures, unclassified ownership and shared
		// state stay global. An account timer cannot reconcile a ground object
		// or finalize/resurrect an unpublished mob death.
		return false
	}
	keys := make(map[string]bool, len(pending.keys))
	for _, key := range pending.keys {
		keys[key] = true
	}
	isolated := 0
	for _, p := range w.players {
		if p == nil || p.Account == nil || !keys[accountSessionKey(p.Account.Name)] {
			continue
		}
		p.PersistencePoisoned = true
		if p.Session != nil {
			p.Session.Close()
		}
		isolated++
	}
	if w.criticalQuarantine == nil {
		w.criticalQuarantine = make(map[uint64]time.Time)
	}
	// This timestamp is diagnostic, not a release deadline. A lost COMMIT can
	// still finish after the worker returns; a new login/write must not race it.
	w.criticalQuarantine[id] = w.now()
	metricCriticalQuarantined.Add(1)
	log.Printf("CRITICAL: persistence completion %d failed; isolated %d player(s) of %d account(s); verified reconciliation required: %v",
		id, isolated, len(pending.keys), err)
	return true
}
