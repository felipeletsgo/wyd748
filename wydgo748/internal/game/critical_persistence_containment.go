package game

import (
	"log"
	"time"

	gameNet "wydgo/internal/net"
)

// WithCriticalAccountContainment selects account-local containment: an
// unknown or unverifiable account-only outcome isolates only its participants
// instead of putting the whole World into maintenance. Shared guild and
// instance aggregates and shared gameplay objects still fail globally, because
// their RAM cannot be reconciled per account.
func WithCriticalAccountContainment() WorldOption {
	return func(w *World) { w.criticalAccountContainment = true }
}

// A successful adapter result proves the transaction finished. If only its
// account-local continuation fails, discard RAM rather than quarantine a known
// commit forever. Never replay the callback or save a final stale snapshot.
// This runs in the same actor delivery, before another command can claim the
// account. Shared state, uncertain outcomes and replaced ownership stay fenced.
func (w *World) recoverCommittedAccountFailure(result criticalPersistenceResult, failure error) bool {
	if !w.criticalAccountContainment || w.shutdownErr != nil || w.shuttingDown || result.err != nil {
		return false
	}
	owner := w.criticalWrites
	queue := owner.queue
	pending, continuation := queue.pending[result.id], owner.continuations[result.id]
	if pending == nil || !pending.completed || continuation == nil || !continuation.delivered ||
		continuation.err == nil || continuation.sharedWorld || queue.instances == result.id || queue.guilds == result.id ||
		len(pending.keys) == 0 || len(continuation.bindings) != len(pending.keys) {
		return false
	}
	participants := make(map[string]*Player, len(pending.keys))
	for _, binding := range continuation.bindings {
		p := binding.player
		if p == nil || p.Session != binding.session || p.Account != binding.account ||
			p.criticalLifecycle != binding.lifecycle || !bindCriticalPlayer(p).current(w) {
			return false
		}
		retained := w.criticalDisconnects[p.Session]
		if retained != nil && retained.player != p {
			return false
		}
		participants[binding.accountKey] = p
	}
	for _, key := range pending.keys {
		if participants[key] == nil || queue.accounts[key] != result.id {
			return false
		}
	}
	if err := queue.acknowledge(result.id); err != nil {
		return false
	}
	delete(owner.continuations, result.id)
	if w.criticalDisconnects == nil {
		w.criticalDisconnects = make(map[*gameNet.Session]*criticalDisconnect)
	}
	for _, p := range participants {
		p.PersistencePoisoned = true
		p.Session.Close()
		delete(w.authPending, p.Session)
		delete(w.security, p.Session)
		w.releaseAuthenticatedClientSlot(p.Session)
		retained := w.criticalDisconnects[p.Session]
		if retained == nil {
			retained = &criticalDisconnect{player: p}
			w.criticalDisconnects[p.Session] = retained
		}
		retained.saved, retained.committedRecovery, retained.admissionErr = true, true, nil
	}
	// Serialized instance detachment and reciprocal trade cleanup still retain
	// account ownership until their own acknowledgements. Login then reloads
	// authoritative account/sidecar state instead of reusing discarded RAM.
	log.Printf("CRITICAL: persistence completion %d committed but account continuation failed; discarding %d session(s) for safe relogin: %v",
		result.id, len(participants), failure)
	return true
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
