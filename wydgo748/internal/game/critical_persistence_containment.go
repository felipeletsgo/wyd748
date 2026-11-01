package game

import (
	"log"
	"time"

	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
)

// WithCriticalAccountContainment isolates failed persistence operations.
// Accounts and shared objects retain their fences until reconciliation.
// Unrelated players and login admission remain available across channels.
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
		p.Session.CloseWithCause(gameNet.CloseCausePersistence, nil)
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

// containCriticalFailureLocally fences failed accounts and shared aggregates.
// Existing object reservations remain held; callbacks never replay or roll back
// an uncertain transaction. Quarantine does not stop admission or healthy ticks.
func (w *World) containCriticalFailureLocally(id uint64, err error) bool {
	if !w.criticalAccountContainment || w.criticalWrites == nil {
		return false
	}
	queue := w.criticalWrites.queue
	continuation := w.criticalWrites.continuations[id]
	if continuation == nil || continuation.reservation == nil {
		// A late or duplicate result has no new authority over live state.
		log.Printf("critical persistence: ignored unowned completion %d: %v", id, err)
		return true
	}
	pending := continuation.reservation
	if pending.quarantined {
		return true
	}
	// Publication can fail after acknowledge released its original fences.
	// Restore ownership without overwriting a successor admitted by publication.
	// Failure fences survive that successor's acknowledgement as well.
	queue.pending[id] = pending
	pending.quarantined = true
	queue.quarantined++
	if queue.failedAccounts == nil {
		queue.failedAccounts = make(map[string]uint64)
		queue.failedCharacters = make(map[string]uint64)
	}
	keys := make(map[string]bool, len(pending.keys))
	for _, key := range pending.keys {
		keys[key] = true
		queue.failedAccounts[key] = id
	}
	for _, uid := range pending.uids {
		queue.failedCharacters[uid] = id
	}
	if pending.instances {
		queue.failedInstances = id
	}
	if pending.guilds {
		queue.failedGuilds = id
	}
	isolated := 0
	worlds := []*World{w}
	if w.cluster != nil {
		worlds = w.cluster.worlds
	}
	bound := make(map[*Player]bool, len(continuation.bindings))
	for _, binding := range continuation.bindings {
		bound[binding.player] = true
	}
	for _, world := range worlds {
		for _, p := range world.players {
			if p == nil || !bound[p] && (p.Account == nil || !keys[accountSessionKey(p.Account.Name)]) {
				continue
			}
			p.PersistencePoisoned = true
			if p.Session != nil {
				p.Session.CloseWithCause(gameNet.CloseCausePersistence, nil)
			}
			isolated++
		}
	}
	if w.criticalQuarantine == nil {
		w.criticalQuarantine = make(map[uint64]time.Time)
	}
	// Time never releases ownership. Completed failures consume no worker slot,
	// so repeated unrelated faults cannot fill the live persistence queue.
	w.criticalQuarantine[id] = w.now()
	accountOnly := !continuation.sharedWorld && !pending.instances && !pending.guilds
	w.noteQuarantineTransaction(id, err, accountOnly)
	metricCriticalQuarantined.Add(1)
	log.Printf("CRITICAL: persistence completion %d failed; isolated %d player(s) of %d account(s); shared=%t instances=%t guilds=%t; admission remains open; verified reconciliation required: %v",
		id, isolated, len(pending.keys), continuation.sharedWorld, pending.instances, pending.guilds, err)
	if accountOnly && store.IsUnsentWrite(err) {
		// No adapter received this write, so its outcome is a known abort. The
		// players are isolated above; a failed predecessor keeps its own fence.
		w.releaseVerifiedQuarantine(id, "unsent")
	}
	return true
}
