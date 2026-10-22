package game

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"log"
	"strings"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

type colosseumReceipt struct {
	command control.ColosseumCommand
	result  control.ColosseumResult
}

type colosseumRequest struct {
	ctx       context.Context
	actor     string
	command   *control.ColosseumCommand
	authorize func() bool
	reply     chan control.ColosseumResult
}

func (w *World) ColosseumStatus(ctx context.Context) (control.ColosseumStatus, error) {
	r, err := w.enqueueColosseum(&colosseumRequest{ctx: ctx})
	return r.Status, err
}

func (w *World) ColosseumCommand(ctx context.Context, actor string, cmd control.ColosseumCommand, authorize func() bool) (control.ColosseumResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.ColosseumResult{}, err
	}
	return w.enqueueColosseum(&colosseumRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}

func (w *World) enqueueColosseum(req *colosseumRequest) (control.ColosseumResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.ColosseumResult{}, err
	}
	if !w.colosseumPending.CompareAndSwap(false, true) {
		return control.ColosseumResult{}, control.ErrBusy
	}
	req.reply = make(chan control.ColosseumResult, 1)
	select {
	case w.commands <- command{colosseum: req, queuedAt: time.Now()}:
	default:
		w.colosseumPending.Store(false)
		return control.ColosseumResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.ColosseumResult{}, req.ctx.Err()
	}
}

func (w *World) colosseumSnapshot() control.ColosseumStatus {
	now := w.now()
	s := control.ColosseumStatus{Version: 1, AsOf: now.UTC(), Schedule: []control.ColosseumSchedule{}}
	c := w.colosseum
	if c == nil {
		return s
	}
	if c.epoch == "" {
		var b [16]byte
		if _, err := rand.Read(b[:]); err != nil {
			panic(err)
		}
		c.epoch = hex.EncodeToString(b[:])
		if c.revision == 0 {
			c.revision = 1
		}
	}
	s.Epoch, s.Revision, s.Enabled, s.Stage = c.epoch, c.revision, c.config.Enabled, c.stage
	s.Active = c.stage >= 1 && c.stage <= 7
	s.PendingExits = len(c.exits)
	if s.Active {
		s.Tier = string("NMA"[c.tier])
		end := c.endsAt.UTC()
		s.EndsAt = &end
	}
	loc, err := w.serverLocation()
	if err != nil {
		s.Enabled = false
		return s
	}
	s.Timezone = loc.String()
	local := now.In(loc)
	for tier := 0; tier < model.ColosseumTiers; tier++ {
		start := time.Date(local.Year(), local.Month(), local.Day(), int(c.config.EventHour)+tier+1, 5, 0, 0, loc)
		s.Schedule = append(s.Schedule, control.ColosseumSchedule{Tier: string("NMA"[tier]), StartsAt: start, Claimed: c.claimed[tier] == local.Format("2006-01-02")})
	}
	return s
}

func (w *World) handleColosseum(req *colosseumRequest) {
	defer w.colosseumPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	r := control.ColosseumResult{Code: "ok", Status: w.colosseumSnapshot()}
	if req.command == nil {
		req.reply <- r
		return
	}
	cmd := *req.command
	r.OperationID = cmd.OperationID
	finish := func(code string) { r.Code = code; r.Status = w.colosseumSnapshot(); req.reply <- r }
	if cmd.Validate() != nil {
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
	if w.shuttingDown {
		finish("server_maintenance")
		return
	}
	c := w.colosseum
	if c == nil || !r.Status.Enabled {
		finish("event_disabled")
		return
	}
	if cmd.Epoch != c.epoch {
		finish("stale_event")
		return
	}
	key := req.actor + ":" + cmd.OperationID
	if receipt, ok := c.receipts[key]; ok {
		if receipt.command != cmd {
			finish("operation_conflict")
			return
		}
		r = receipt.result
		r.Replayed = true
		r.Status = w.colosseumSnapshot()
		req.reply <- r
		return
	}
	if len(c.receipts) >= 1024 {
		finish("operation_capacity")
		return
	}
	tier := strings.Index("NMA", cmd.Tier)
	code := "ok"
	scheduled := r.Status.Schedule[tier]
	switch {
	case cmd.ExpectedRevision != c.revision:
		code = "stale_event"
	case r.Status.Active:
		code = "event_active"
	case len(c.exits) != 0:
		code = "event_busy"
	case scheduled.Claimed:
		code = "already_started"
	case !w.now().Before(scheduled.StartsAt):
		code = "schedule_passed"
	default:
		loc, _ := w.serverLocation()
		w.startColosseum(tier, w.now().In(loc))
	}
	r.Code, r.Status = code, w.colosseumSnapshot()
	if c.receipts == nil {
		c.receipts = make(map[string]colosseumReceipt)
	}
	c.receipts[key] = colosseumReceipt{command: cmd, result: r}
	log.Printf("[web-admin] colosseum actor=%q operation=%q tier=%q reason=%q result=%q", req.actor, cmd.OperationID, cmd.Tier, cmd.Reason, code)
	req.reply <- r
}
