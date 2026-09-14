package game

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"log"
	"time"
	"wydgo/internal/control"
)

// MODERNIZACAO_COMPATIVEL: reuse the normal authoritative disconnect lifecycle.
type kickReceipt struct {
	command control.KickCommand
	result  control.KickResult
}
type kickRequest struct {
	ctx       context.Context
	actor     string
	command   control.KickCommand
	authorize func() bool
	reply     chan control.KickResult
}

// Only called on the World goroutine. A new process invalidates old targets.
func (w *World) moderationEpoch() string {
	if w.kickEpoch == "" {
		var b [16]byte
		if _, err := rand.Read(b[:]); err != nil {
			panic(err)
		}
		w.kickEpoch = hex.EncodeToString(b[:])
	}
	return w.kickEpoch
}

func (w *World) KickCommand(ctx context.Context, actor string, c control.KickCommand, authorize func() bool) (control.KickResult, error) {
	if err := c.Validate(); err != nil {
		return control.KickResult{}, err
	}
	if err := ctx.Err(); err != nil {
		return control.KickResult{}, err
	}
	if !w.kickPending.CompareAndSwap(false, true) {
		return control.KickResult{}, control.ErrBusy
	}
	req := &kickRequest{ctx: ctx, actor: actor, command: c, authorize: authorize, reply: make(chan control.KickResult, 1)}
	select {
	case w.commands <- command{kick: req, queuedAt: time.Now()}:
	default:
		w.kickPending.Store(false)
		return control.KickResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-ctx.Done():
		return control.KickResult{}, ctx.Err()
	}
}

func (w *World) handleKick(req *kickRequest) {
	defer w.kickPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	c := req.command
	r := control.KickResult{OperationID: c.OperationID}
	finish := func(code string) { r.Code = code; req.reply <- r }
	if c.Validate() != nil {
		finish("invalid_command")
		return
	}
	if req.actor == "" || req.authorize == nil || !req.authorize() {
		finish("capability_required")
		return
	}
	if req.ctx.Err() != nil {
		return
	}
	if c.Epoch != w.moderationEpoch() {
		finish("stale_target")
		return
	}
	key := req.actor + ":" + c.OperationID
	if receipt, exists := w.kickReceipts[key]; exists {
		if receipt.command != c {
			finish("operation_conflict")
			return
		}
		r = receipt.result
		r.Replayed = true
		req.reply <- r
		return
	}
	if w.shuttingDown {
		finish("server_maintenance")
		return
	}
	// Receipts are never evicted in an epoch: a delayed retry cannot kick again.
	if len(w.kickReceipts) >= 1024 {
		finish("operation_capacity")
		return
	}
	r.Code = "stale_target"
	for s, p := range w.players {
		if s == nil || s.ID != c.ExpectedSessionID || p == nil || p.Session != s || !p.InWorld || p.Char == nil || p.Account == nil ||
			p.Char.UID != c.UID || accountSessionKey(p.Account.Name) != c.Account || s.IsClosed() {
			continue
		}
		// Close first so pending packets cannot act while cleanup/persistence runs.
		s.Close()
		w.onDisconnect(s)
		r.Code = "ok"
		if p.PersistencePoisoned {
			r.Code = "disconnected_persistence_failed"
		}
		break
	}
	if w.kickReceipts == nil {
		w.kickReceipts = make(map[string]kickReceipt)
	}
	w.kickReceipts[key] = kickReceipt{command: c, result: r}
	log.Printf("[web-admin] kick actor=%q operation=%q reason=%q result=%q uid=%q account=%q session=%d", req.actor, c.OperationID, c.Reason, r.Code, c.UID, c.Account, c.ExpectedSessionID)
	req.reply <- r
}
