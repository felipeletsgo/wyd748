package game

import (
	"time"

	"wydgo/internal/model"
)

// Periodic snapshots share economic ownership and atomic account/sidecar writes.
// Completion acknowledges durability only; it must never install an older
// snapshot into live memory or publish an economic success response.
func (w *World) queueAccountAutoSave(p *Player, now time.Time) error {
	if p == nil || p.Account == nil || p.Char == nil || !p.InWorld {
		return errCriticalPersistenceIdentity
	}
	if w.criticalWrites == nil || w.criticalWrites.queue == nil {
		return errCriticalPersistenceUnsupported
	}
	write := criticalWrite{accounts: []*model.Account{p.Account}}
	if _, atomic := w.criticalWrites.queue.store.(playerStateStore); atomic {
		write.character = &criticalCharacterState{uid: p.Char.UID, state: buildCharState(p, now)}
	} else if _, sidecar := w.criticalWrites.queue.store.(charStateStore); sidecar {
		return errCriticalPersistenceUnsupported
	} else if _, sidecar := w.criticalWrites.queue.store.(asyncCharStateStore); sidecar {
		return errCriticalPersistenceUnsupported
	}
	_, err := w.submitCriticalWrite(write, []*Player{p}, func(criticalPersistenceResult) error { return nil })
	return err
}
