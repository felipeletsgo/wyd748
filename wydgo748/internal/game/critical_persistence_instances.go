package game

import (
	"errors"
	"log"
	"maps"

	"wydgo/internal/store"
)

// Nonzero-size tokens have distinct addresses while earlier jobs retain them.
type criticalInstanceGeneration struct{ marker byte }

func (w *World) criticalInstanceReserved() bool {
	if w == nil || w.criticalWrites == nil {
		return false
	}
	q := w.criticalWrites.queue
	return q.instances != 0 || q.failedInstances != 0 || len(q.instanceKeys) > 0 || len(q.failedInstanceKeys) > 0
}

// queueInstanceState is the non-confirming background instance producer. Dirty
// state already belongs to World; unlike economy proposals, no client success
// depends on this flush. Only one snapshot can persist through acknowledgement.
// Disconnect ownership is the exception: it waits for a covering snapshot.
func (w *World) queueInstanceState() {
	if w.shuttingDown || w.criticalInstanceAdmissionErr != nil {
		return
	}
	if !w.instanceStateDirty {
		w.queueInstanceRows()
		return
	}
	if w.criticalInstanceReserved() {
		return
	}
	captured := maps.Clone(w.instanceDirtyRuntimes)
	generation := w.criticalInstanceGeneration
	type waiter struct {
		retained *criticalDisconnect
		binding  criticalPlayerBinding
	}
	waiters := make([]waiter, 0, len(w.criticalDisconnects))
	for _, retained := range w.criticalDisconnects {
		if retained.saved && retained.instancesDetached && !retained.instancesSaved && retained.admissionErr == nil {
			binding := bindCriticalPlayer(retained.player)
			if !binding.currentIdentity(w, retained.committedRecovery) {
				w.criticalInstanceAdmissionErr = errCriticalPersistenceIdentity
				log.Printf("instance persistence admission failed; retaining identity: %v", w.criticalInstanceAdmissionErr)
				return
			}
			waiters = append(waiters, waiter{retained, binding})
		}
	}
	_, err := w.criticalWrites.submit(criticalWrite{instances: w.instanceStateSnapshot()},
		func(result criticalPersistenceResult) error {
			if result.err != nil {
				return result.err
			}
			if w.shutdownErr != nil {
				return errors.New("critical persistence: maintenance prevents instance acknowledgement")
			}
			// Check every retained identity before changing any waiter or dirty flag.
			for _, waiting := range waiters {
				if w.criticalDisconnects[waiting.binding.session] != waiting.retained ||
					!waiting.binding.currentIdentity(w, waiting.retained.committedRecovery) {
					return errCriticalPersistenceIdentity
				}
			}
			if w.criticalInstanceGeneration == generation {
				w.instanceStateDirty = false
			}
			w.clearInstanceDirt(captured)
			for _, waiting := range waiters {
				waiting.retained.instancesSaved = true
			}
			return nil
		})
	if err != nil && !errors.Is(err, errCriticalPersistenceFull) && !errors.Is(err, errCriticalPersistenceBusy) {
		// Rejected work has not reached storage. Retain dirty state/ownership,
		// log once and leave the explicit terminal snapshot available.
		w.criticalInstanceAdmissionErr = err
		log.Printf("instance persistence admission failed; retaining dirty state and ownership: %v", err)
	}
}

// queueInstanceRows flushes per-runtime dirt as a keyed write (W8). Rows
// whose runtime has a pending write wait for a later flush.
func (w *World) queueInstanceRows() {
	if len(w.instanceDirtyRuntimes) == 0 || w.criticalInstanceWholeReserved() {
		return
	}
	channel := w.instanceRowChannel()
	captured := make(map[string]uint64, len(w.instanceDirtyRuntimes))
	keys := make([]store.InstanceParticipant, 0, len(w.instanceDirtyRuntimes))
	for id, version := range w.instanceDirtyRuntimes {
		key := store.InstanceParticipant{Channel: channel, RuntimeID: id}
		name := instanceRowKey(key)
		if q := w.criticalWrites.queue; q.instanceKeys[name] != 0 || q.failedInstanceKeys[name] != 0 {
			continue
		}
		captured[id] = version
		keys = append(keys, key)
	}
	if len(keys) == 0 {
		return
	}
	_, err := w.criticalWrites.submit(criticalWrite{instances: w.instanceStateSnapshot(), instanceKeys: keys},
		func(result criticalPersistenceResult) error {
			if result.err != nil {
				return result.err
			}
			if w.shutdownErr != nil {
				return errors.New("critical persistence: maintenance prevents instance acknowledgement")
			}
			w.clearInstanceDirt(captured)
			return nil
		})
	if err != nil && !errors.Is(err, errCriticalPersistenceFull) && !errors.Is(err, errCriticalPersistenceBusy) {
		w.criticalInstanceAdmissionErr = err
		log.Printf("instance persistence admission failed; retaining dirty state and ownership: %v", err)
	}
}

// clearInstanceDirt drops dirt that no later change superseded.
func (w *World) clearInstanceDirt(captured map[string]uint64) {
	for id, version := range captured {
		if w.instanceDirtyRuntimes[id] == version {
			delete(w.instanceDirtyRuntimes, id)
		}
	}
}
