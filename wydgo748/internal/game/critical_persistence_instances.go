package game

import (
	"errors"
	"log"
)

// Nonzero-size tokens have distinct addresses while earlier jobs retain them.
type criticalInstanceGeneration struct{ marker byte }

func (w *World) criticalInstanceReserved() bool {
	return w != nil && w.criticalWrites != nil && w.criticalWrites.queue.instances != 0
}

// queueInstanceState is the non-confirming background instance producer. Dirty
// state already belongs to World; unlike economy proposals, no client success
// depends on this flush. Only one snapshot can persist through acknowledgement.
// Disconnect ownership is the exception: it waits for a covering snapshot.
func (w *World) queueInstanceState() {
	if w.shuttingDown || w.criticalInstanceAdmissionErr != nil || w.criticalInstanceReserved() {
		return
	}
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
