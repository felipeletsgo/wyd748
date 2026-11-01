package game

import (
	"context"
	"log"
	"slices"
	"time"

	"wydgo/internal/control"
)

// MODERNIZACAO_COMPATIVEL: staff view of quarantined critical writes and an
// audited request for an immediate exact-outcome check (W4 item 10). The
// check uses the same final-status rule as the periodic one; nothing here
// releases a fence without a final status from PostgreSQL.
type persistenceRecoveryRequest struct {
	ctx       context.Context
	actor     string
	command   *control.PersistenceRecheckCommand
	authorize func() bool
	reply     chan control.PersistenceRecheckResult
}

func (w *World) PersistenceRecoveryStatus(ctx context.Context) (control.PersistenceRecoveryStatus, error) {
	r, err := w.enqueuePersistenceRecovery(&persistenceRecoveryRequest{ctx: ctx})
	return r.Status, err
}

func (w *World) PersistenceRecheck(ctx context.Context, actor string, cmd control.PersistenceRecheckCommand,
	authorize func() bool) (control.PersistenceRecheckResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.PersistenceRecheckResult{}, err
	}
	return w.enqueuePersistenceRecovery(&persistenceRecoveryRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}

func (w *World) enqueuePersistenceRecovery(req *persistenceRecoveryRequest) (control.PersistenceRecheckResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.PersistenceRecheckResult{}, err
	}
	if !w.recoveryPending.CompareAndSwap(false, true) {
		return control.PersistenceRecheckResult{}, control.ErrBusy
	}
	req.reply = make(chan control.PersistenceRecheckResult, 1)
	select {
	case w.commands <- command{recovery: req, queuedAt: time.Now()}:
	default:
		w.recoveryPending.Store(false)
		return control.PersistenceRecheckResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.PersistenceRecheckResult{}, req.ctx.Err()
	}
}

// recoveryWorlds returns every channel World. The cluster runs them on this
// goroutine, so reading their quarantine state here is safe.
func (w *World) recoveryWorlds() []*World {
	if w.cluster != nil {
		return w.cluster.worlds
	}
	return []*World{w}
}

func (w *World) persistenceRecoverySnapshot() control.PersistenceRecoveryStatus {
	status := control.PersistenceRecoveryStatus{Version: 1, AsOf: w.now().UTC(),
		Quarantined: []control.QuarantinedOperation{}, Released: metricCriticalQuarantineReleased.Value()}
	for _, world := range w.recoveryWorlds() {
		if world.criticalWrites == nil {
			continue
		}
		for id, since := range world.criticalQuarantine {
			op := control.QuarantinedOperation{ID: id, Channel: world.channel, Since: since.UTC(),
				Accounts: []string{}, Checkable: world.criticalQuarantineXID[id] != "",
				LastStatus: world.quarantineLastStatus[id]}
			if pending := world.criticalWrites.queue.pending[id]; pending != nil {
				op.Accounts = slices.Clone(pending.keys)
				op.Shared = pending.instances || pending.guilds
			}
			if continuation := world.criticalWrites.continuations[id]; continuation != nil && continuation.sharedWorld {
				op.Shared = true
			}
			status.Quarantined = append(status.Quarantined, op)
		}
	}
	slices.SortFunc(status.Quarantined, func(a, b control.QuarantinedOperation) int {
		return a.Since.Compare(b.Since)
	})
	return status
}

func (w *World) handlePersistenceRecovery(req *persistenceRecoveryRequest) {
	defer w.recoveryPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.PersistenceRecheckResult{Code: "ok", Status: w.persistenceRecoverySnapshot()}
	if req.command == nil {
		req.reply <- result
		return
	}
	c := *req.command
	result.OperationID = c.OperationID
	finish := func(code string) {
		result.Code, result.Status = code, w.persistenceRecoverySnapshot()
		req.reply <- result
	}
	switch {
	case c.Validate() != nil:
		finish("invalid_command")
		return
	case req.actor == "" || req.authorize == nil || !req.authorize():
		finish("capability_required")
		return
	}
	checkable, running := 0, 0
	now := w.now()
	for _, world := range w.recoveryWorlds() {
		if len(world.criticalQuarantineXID) == 0 {
			continue
		}
		checkable++
		if world.quarantineCheckRunning {
			running++
			continue
		}
		world.quarantineCheckAt = time.Time{}
		world.checkQuarantinedCommits(now)
		if world.quarantineCheckRunning {
			result.Started++
		}
	}
	code := "ok"
	switch {
	case checkable == 0:
		code = "nothing_to_check"
	case result.Started == 0 && running > 0:
		code = "check_running"
	case result.Started == 0:
		code = "check_unavailable"
	}
	log.Printf("[web-admin] persistence-recheck actor=%q operation=%q reason=%q result=%q started=%d",
		req.actor, c.OperationID, c.Reason, code, result.Started)
	finish(code)
}
