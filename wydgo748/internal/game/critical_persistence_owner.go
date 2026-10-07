package game

import (
	"errors"
	"fmt"
	"log"

	"wydgo/internal/store"
)

const criticalCompletionBatchLimit = 8

// criticalPersistenceOwner retains gameplay continuations on the World actor.
// NewWorld initializes it when the backend certifies the required atomic writes.
type criticalPersistenceOwner struct {
	queue         *criticalPersistence
	continuations map[uint64]*criticalContinuation
}

type criticalContinuation struct {
	// Shared gameplay ownership is independent of the durable aggregate type.
	// Ground objects and pending mob deaths cannot be discarded per account.
	sharedWorld bool
	apply       func(criticalPersistenceResult) error
	afterAck    func(criticalPersistenceResult) error
	reject      func(criticalPersistenceResult) error
	delivered   bool
	err         error
}

func newCriticalPersistenceOwner(queue *criticalPersistence) *criticalPersistenceOwner {
	return &criticalPersistenceOwner{queue: queue, continuations: make(map[uint64]*criticalContinuation)}
}

// submit registers the continuation before the actor can consume its result.
// Workers never receive this callback. Rejection does not invoke it.
func (owner *criticalPersistenceOwner) submit(write criticalWrite, apply func(criticalPersistenceResult) error) (uint64, error) {
	return owner.submitPhased(write, apply, nil)
}

func (owner *criticalPersistenceOwner) submitPhased(write criticalWrite, apply, afterAck func(criticalPersistenceResult) error) (uint64, error) {
	return owner.submitWithRejection(write, apply, afterAck, nil)
}

// Rejection recovery is opt-in. Its callback must verify retained live state
// and settle only failure effects; it cannot assume an ordinary error rejected
// the durable write. Workers never receive any of these actor callbacks.
func (owner *criticalPersistenceOwner) submitWithRejection(write criticalWrite, apply, afterAck, reject func(criticalPersistenceResult) error) (uint64, error) {
	if owner == nil || owner.queue == nil || apply == nil {
		return 0, errors.New("critical persistence: missing owner, queue or continuation")
	}
	id, err := owner.queue.submit(write)
	if err != nil {
		return 0, err
	}
	owner.continuations[id] = &criticalContinuation{apply: apply, afterAck: afterAck, reject: reject}
	return id, nil
}

// complete applies a result once on the actor. Only an adapter-certified
// rejection with a successful explicit rejection continuation can settle a
// failed write. Other store or pre-ack failures retain every account fence and
// pending capacity. A post-ack failure has
// already released persistence fences and requires World-wide containment
// before delivery yields. Neither failure permits automatic replay/rollback.
func (owner *criticalPersistenceOwner) complete(result criticalPersistenceResult) error {
	pending := owner.queue.pending[result.id]
	continuation := owner.continuations[result.id]
	if pending == nil || continuation == nil || continuation.delivered {
		return errors.New("critical persistence: unknown or duplicate completion")
	}
	pending.completed = true
	continuation.delivered = true
	rejected := result.err != nil && store.IsWriteRejected(result.err) &&
		!errors.Is(result.err, errCriticalPersistenceIndeterminate) && continuation.reject != nil
	if rejected {
		if err := applyCriticalContinuation(continuation.reject, result); err != nil {
			continuation.err = errors.Join(result.err, err)
		}
	} else {
		continuation.err = errors.Join(result.err, applyCriticalContinuation(continuation.apply, result))
	}
	if continuation.err != nil {
		// The continuation may capture live objects. After first delivery its
		// error and the account fences suffice; do not retain/replay its closure.
		continuation.apply = nil
		continuation.afterAck = nil
		continuation.reject = nil
		return continuation.err
	}
	if err := owner.queue.acknowledge(result.id); err != nil {
		continuation.apply = nil
		continuation.afterAck = nil
		continuation.reject = nil
		continuation.err = err
		return err
	}
	if rejected {
		// A rejected proposal has no committed success state to publish. Its
		// failure continuation settled under the fence, on this actor delivery.
		delete(owner.continuations, result.id)
		return nil
	}
	// Still inside one actor delivery: guarded gameplay reconciliation may now
	// run without its own persistence fence. No command/tick can interleave.
	if continuation.afterAck != nil {
		afterAck := continuation.afterAck
		continuation.apply, continuation.afterAck, continuation.reject = nil, nil, nil
		if err := applyCriticalContinuation(afterAck, result); err != nil {
			continuation.err = err
			return err // World contains this confirmed, non-replayable failure.
		}
	}
	delete(owner.continuations, result.id)
	return nil
}

func applyCriticalContinuation(apply func(criticalPersistenceResult) error, result criticalPersistenceResult) (err error) {
	defer func() {
		if failure := recover(); failure != nil {
			err = fmt.Errorf("critical persistence: continuation panic after persistence: %v", failure)
		}
	}()
	return apply(result)
}

// criticalCompletionChannel is nil when the coordinator is disabled. A nil
// select case introduces no worker or polling goroutine.
func (w *World) criticalCompletionChannel() <-chan criticalPersistenceResult {
	if w.criticalWrites == nil {
		return nil
	}
	return w.criticalWrites.queue.results
}

func (w *World) processCriticalCompletions(limit int) {
	if w.criticalWrites == nil {
		return
	}
	for range limit {
		result, ok := w.criticalWrites.queue.poll()
		if !ok {
			return
		}
		w.applyCriticalCompletion(result)
	}
}

func (w *World) applyCriticalCompletion(result criticalPersistenceResult) {
	if err := w.criticalWrites.complete(result); err != nil && !w.containCriticalFailureLocally(result.id, err) {
		// Without account containment, or for shared aggregates and
		// post-acknowledgement failures, fail the entire World closed. Unknown
		// durable outcomes must not feed autosaves or accept further gameplay
		// mutations. Accepted workers still drain.
		log.Printf("CRITICAL: persistence completion %d failed; entering maintenance: %v", result.id, err)
		w.criticalWrites.queue.closeAdmission()
		w.shutdownErr = errors.Join(w.shutdownErr, fmt.Errorf("critical completion %d: %w", result.id, err))
		w.shuttingDown = true
		for _, player := range w.players {
			if player != nil {
				player.PersistencePoisoned = true
				if player.Session != nil {
					player.Session.Close()
				}
			}
		}
	}
	w.resumeCriticalDisconnects(criticalCompletionBatchLimit)
	// Replay packets that waited behind this reservation before any newer
	// packet of the same session can run.
	w.resumeCriticalDeferredCommands(criticalDeferredReplayLimit)
	w.finishShutdownIfReady()
}
