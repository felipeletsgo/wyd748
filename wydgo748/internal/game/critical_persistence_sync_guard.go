package game

import (
	"errors"
	"log"
)

// errSynchronousPersistenceDisabled rejects a legacy inline write while the
// critical coordinator owns persistence. The caller's existing failure path
// keeps live state unchanged; nothing reached storage.
var errSynchronousPersistenceDisabled = errors.New("synchronous persistence is disabled while the critical coordinator is active")

// syncPersistenceAllowed is the single gate for legacy World-goroutine writes.
// Without a coordinator every write remains synchronous. With one, normal
// command and tick processing must never block on storage: only the explicit
// terminal shutdown snapshot, which runs after gameplay stopped, may write
// inline. Each rejected operation is logged once and counted.
func (w *World) syncPersistenceAllowed(operation string) error {
	if w.criticalWrites == nil || w.terminalPersistence {
		return nil
	}
	metricSyncPersistenceRejected.Add(1)
	if w.syncPersistenceRejected == nil {
		w.syncPersistenceRejected = make(map[string]bool)
	}
	if !w.syncPersistenceRejected[operation] {
		w.syncPersistenceRejected[operation] = true
		log.Printf("ERROR: unmigrated synchronous persistence %q rejected under the critical coordinator", operation)
	}
	return errSynchronousPersistenceDisabled
}
