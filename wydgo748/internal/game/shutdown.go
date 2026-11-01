package game

import (
	"errors"
	"fmt"
	"log"
	"sync"
	"time"

	"wydgo/internal/model"
)

// Shutdown drains pending saves through a game-loop command. World owns the
// mutable state, so the drain must not race with a handler or save stale data.

// flushStore drains pending asynchronous writes.
type flushStore interface {
	Flush()
}

// Shutdown waits for final persistence or the timeout. False means the drain
// failed or did not complete in time; callers must report an incomplete shutdown.
func (w *World) Shutdown(timeout time.Duration) bool {
	done := make(chan error, 1)
	timer := time.NewTimer(timeout)
	defer timer.Stop()
	select {
	case w.commands <- command{queuedAt: time.Now(), shutdown: done}:
	case <-timer.C:
		log.Print("shutdown: game loop did not accept the command before timeout")
		return false
	}
	select {
	case err := <-done:
		if err != nil {
			log.Printf("shutdown: incomplete final persistence: %v", err)
			return false
		}
		return true
	case <-timer.C:
		log.Print("shutdown: timeout during final persistence")
		return false
	}
}

const shutdownWaiterLimit = 64

// runShutdown closes admission but never waits on workers on the World actor.
// Completion delivery resumes the final drain once all accepted writes settle.
func (w *World) runShutdown(done chan error) {
	if w.cluster != nil {
		// Close all admission before any terminal snapshot, including requests
		// that arrive through an individual world's administrative interface.
		for _, other := range w.cluster.worlds {
			other.shuttingDown = true
		}
	}
	if w.shutdownFinished {
		replyShutdown(done, w.shutdownErr)
		return
	}
	if done != nil {
		// Internal callers can time out without cancelling a durable operation.
		// Bound retained reply channels even if they repeatedly request shutdown.
		if len(w.shutdownWaiters) >= shutdownWaiterLimit {
			replyShutdown(done, errors.New("shutdown: pending caller capacity exhausted"))
			return
		}
		w.shutdownWaiters = append(w.shutdownWaiters, done)
	}
	if w.shutdownStarted {
		return
	}
	w.shutdownStarted = true
	w.shuttingDown = true
	if w.criticalWrites != nil {
		w.criticalWrites.queue.closeAdmission()
	}
	w.finishShutdownIfReady()
}

func replyShutdown(done chan error, err error) {
	if done != nil {
		done <- err
		close(done)
	}
}

func (w *World) finishShutdownIfReady() {
	if !w.shutdownStarted || w.shutdownFinished {
		return
	}
	if w.criticalWrites != nil {
		for id := range w.criticalWrites.queue.pending {
			continuation := w.criticalWrites.continuations[id]
			if continuation == nil || !continuation.delivered {
				return
			}
		}
		for id, pending := range w.criticalWrites.queue.pending {
			if pending.quarantined {
				w.shutdownErr = errors.Join(w.shutdownErr, fmt.Errorf("critical completion %d: %w",
					id, w.criticalWrites.continuations[id].err))
			}
		}
	}
	if w.shutdownErr == nil {
		w.persistShutdownState()
	}
	// Never overwrite an uncertain durable outcome with a final RAM snapshot.
	w.shutdownFinished = true
	for _, waiter := range w.shutdownWaiters {
		replyShutdown(waiter, w.shutdownErr)
	}
	w.shutdownWaiters = nil
}

// persistShutdownState runs only after accepted critical continuations settle.
// Synchronous final writes remain intentional; normal gameplay is stopped.
func (w *World) persistShutdownState() {
	// Gameplay has stopped: the terminal snapshot is the only inline write
	// permitted while a critical coordinator exists.
	w.terminalPersistence = true
	defer func() { w.terminalPersistence = false }()

	saved, failed := 0, 0
	var failures []error
	// An aggregate contains every character on the account. A healthy sibling
	// must not accidentally persist the poisoned character's partial mutation.
	poisoned := make(map[string]bool)
	for _, p := range w.players {
		if p != nil && p.Account != nil && p.PersistencePoisoned {
			poisoned[accountSessionKey(p.Account.Name)] = true
		}
	}
	for name := range poisoned {
		failed++
		failures = append(failures, fmt.Errorf("account %q: poisoned in-memory state discarded", name))
	}
	// Normal sessions have one character per account, but fixtures and admin
	// paths may have more. Deduplicate by account and UID to save every sidecar.
	type playerStateKey struct {
		account *model.Account
		uid     string
	}
	seen := make(map[playerStateKey]struct{}, len(w.players))
	type terminalSave struct {
		account string
		write   func() error
	}
	var saves []terminalSave
	for _, p := range w.players {
		if p == nil || p.Account == nil || poisoned[accountSessionKey(p.Account.Name)] {
			continue
		}
		accountOnly := !p.InWorld || p.Char == nil
		if accountOnly && w.criticalDisconnects[p.Session] == nil {
			continue
		}
		key := playerStateKey{account: p.Account}
		if !accountOnly {
			key.uid = p.Char.UID
		}
		if _, repeated := seen[key]; repeated {
			continue
		}
		seen[key] = struct{}{}
		// Detach each final snapshot on the actor; the writes then run on
		// bounded workers. Gameplay is stopped, so no live state can change.
		var write func() error
		var err error
		if accountOnly {
			// A queue-rejected disconnect on the character-selection screen
			// still owns an account; do not discard it during terminal shutdown.
			write, err = w.prepareAccountSave(p.Account)
		} else {
			write, err = w.prepareAccountAndCharStateSave(p.Account, p.Char.UID, buildCharState(p, w.now()))
		}
		if err != nil {
			failed++
			log.Printf("shutdown: save account %q: %v", p.Account.Name, err)
			failures = append(failures, fmt.Errorf("account %q: %w", p.Account.Name, err))
			continue
		}
		if write != nil {
			saves = append(saves, terminalSave{account: p.Account.Name, write: write})
		}
	}
	started := time.Now()
	results := make([]error, len(saves))
	runBounded(len(saves), max(w.shutdownSaveWorkers, 1), func(i int) { results[i] = saves[i].write() })
	for i, err := range results {
		if err != nil {
			failed++
			log.Printf("shutdown: save account %q: %v", saves[i].account, err)
			failures = append(failures, fmt.Errorf("account %q: %w", saves[i].account, err))
			continue
		}
		saved++
	}
	log.Printf("shutdown: %d final account write(s) took %s with %d worker(s)",
		len(saves), time.Since(started).Round(time.Millisecond), max(w.shutdownSaveWorkers, 1))
	if w.instanceStateDirtyAny() {
		if err := w.persistInstanceState(); err != nil {
			failed++
			failures = append(failures, fmt.Errorf("instance state: %w", err))
			log.Printf("shutdown: save instance state: %v", err)
		} else {
			w.instanceStateDirty = false
			clear(w.instanceDirtyRuntimes)
		}
	}

	// Drain autosave writes queued before the shutdown signal.
	if fs, ok := w.store.(flushStore); ok {
		fs.Flush()
	}
	log.Printf("shutdown: %d accounts persisted, %d failures", saved, failed)
	w.shutdownErr = errors.Join(failures...)
}

// WithShutdownSaveWorkers bounds concurrent terminal saves (W5/D11). The store
// must be goroutine-safe; one worker keeps the sequential order.
func WithShutdownSaveWorkers(workers int) WorldOption {
	return func(w *World) { w.shutdownSaveWorkers = workers }
}

// runBounded calls work for every index with at most workers goroutines and
// returns after all calls finish. One worker runs inline in index order.
func runBounded(n, workers int, work func(int)) {
	if workers <= 1 || n <= 1 {
		for i := range n {
			work(i)
		}
		return
	}
	next := make(chan int)
	var running sync.WaitGroup
	for range min(workers, n) {
		running.Add(1)
		go func() {
			defer running.Done()
			for i := range next {
				work(i)
			}
		}()
	}
	for i := range n {
		next <- i
	}
	close(next)
	running.Wait()
}
