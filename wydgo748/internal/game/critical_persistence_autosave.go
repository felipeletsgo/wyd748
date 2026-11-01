package game

import (
	"errors"
	"log"
	"time"

	"wydgo/internal/model"
)

// Repeated certified rejections back off per account, up to this delay. The
// retry stays on the account's own bucket, so recovery keeps saves spread.
const (
	autoSaveRetryMaxDelay   = 30 * time.Second
	autoSaveRejectionMaxLog = 8
)

// autoSaveRetryDelay returns the extra wait after consecutive rejections. The
// first rejection retries on the next normal cycle.
func autoSaveRetryDelay(rejections uint8) time.Duration {
	if rejections <= 1 {
		return 0
	}
	return min(accountAutoSaveInterval<<min(rejections-1, autoSaveRejectionMaxLog), autoSaveRetryMaxDelay)
}

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
	_, err := w.submitCriticalBackgroundWrite(write, p,
		func(criticalPersistenceResult) error {
			if p.autoSaveRejections > 0 {
				log.Printf("[#%d] autosave for %q committed after %d rejection(s)", p.Session.ID, p.Account.Name, p.autoSaveRejections)
			}
			p.autoSaveRejections, p.autoSaveRetryAt = 0, time.Time{}
			return nil
		}, func(result criticalPersistenceResult) error {
			// The write carried a detached snapshot and never changed live RAM.
			// Keep the session and current state; release only this operation.
			// A later slice submits a fresh snapshot, never this rejected one.
			if p.autoSaveRejections < ^uint8(0) {
				p.autoSaveRejections++
			}
			delay := autoSaveRetryDelay(p.autoSaveRejections)
			p.autoSaveRetryAt = w.now().Add(delay)
			if p.autoSaveRejections == 1 {
				log.Printf("[#%d] autosave for %q rejected before commit; retaining live state: %v",
					p.Session.ID, p.Account.Name, result.err)
			}
			return nil
		})
	return err
}

// submitCriticalBackgroundWrite admits an autosave snapshot that holds no
// gameplay input (W7). Commands keep running while it is written, so the
// player may leave the world or change character before it completes. Its
// continuations therefore check maintenance only, not the bound identity:
// they acknowledge durability of a detached snapshot and change no live
// state. A failure still isolates the bound player through containment.
func (w *World) submitCriticalBackgroundWrite(write criticalWrite, p *Player, apply, reject func(criticalPersistenceResult) error) (uint64, error) {
	if w.shuttingDown {
		return 0, errCriticalPersistenceStopping
	}
	if p == nil || p.Session == nil || p.Account == nil || p.PersistencePoisoned || p.Session.IsClosed() ||
		len(write.accounts) != 1 || write.accounts[0] == nil {
		return 0, errCriticalPersistenceIdentity
	}
	binding := bindCriticalPlayer(p)
	if binding.accountKey == "" || binding.accountKey != accountSessionKey(write.accounts[0].Name) || !binding.current(w) {
		return 0, errCriticalPersistenceIdentity
	}
	maintenance := func() error {
		if w.shutdownErr != nil {
			return errors.New("critical persistence: maintenance prevents publication")
		}
		return nil
	}
	id, err := w.criticalWrites.submitBackground(write, func(result criticalPersistenceResult) error {
		if err := maintenance(); err != nil {
			return err
		}
		if result.err != nil {
			return result.err
		}
		return apply(result)
	}, func(result criticalPersistenceResult) error {
		if err := maintenance(); err != nil {
			return err
		}
		return reject(result)
	})
	if err != nil {
		return 0, err
	}
	w.criticalWrites.continuations[id].bindings = []criticalPlayerBinding{binding}
	return id, nil
}
