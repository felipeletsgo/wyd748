package game

import (
	"errors"
	"fmt"
	"log"
)

// WithCriticalPersistence enables nonblocking critical persistence: every
// command and tick write is submitted to a bounded worker pool instead of
// running on the World goroutine. The store must provide the certified
// atomic adapters for every aggregate form; NewWorld fails otherwise.
func WithCriticalPersistence(workers, capacity int) WorldOption {
	return func(w *World) {
		w.criticalPersistenceWorkers = workers
		w.criticalPersistenceCapacity = capacity
	}
}

var errCriticalPersistenceStoreUnsupported = errors.New(
	"critical persistence requires certified account, character-state, instance and game-state adapters")

// criticalStoreCertified reports whether a store can serve every write form
// the migrated producers submit, without any legacy fallback.
func criticalStoreCertified(w *World) bool {
	_, accounts := w.store.(criticalAccountStore)
	_, states := w.store.(playerStateBatchStore)
	_, instances := w.store.(criticalInstanceStore)
	_, game := w.store.(criticalGameStateStore)
	return accounts && states && instances && game
}

func (w *World) startCriticalPersistence() error {
	if w.criticalPersistenceWorkers <= 0 {
		return nil
	}
	if !criticalStoreCertified(w) {
		return errCriticalPersistenceStoreUnsupported
	}
	queue, err := newCriticalPersistence(w.store, w.criticalPersistenceWorkers, w.criticalPersistenceCapacity)
	if err != nil {
		return fmt.Errorf("critical persistence: %w", err)
	}
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	log.Printf("critical persistence: %d worker(s), capacity %d, account containment=%t",
		w.criticalPersistenceWorkers, w.criticalPersistenceCapacity, w.criticalAccountContainment)
	return nil
}
