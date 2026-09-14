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

// MODERNIZACAO_COMPATIVEL: ordinary item delivery, no new client packet.
// Event lifetime is explicitly process-local; a restart never resumes issuance.
type globalDropState struct {
	status   control.GlobalDropStatus
	reserved int
}
type globalDropReceipt struct {
	command control.GlobalDropCommand
	result  control.GlobalDropResult
}
type globalDropRequest struct {
	ctx       context.Context
	actor     string
	command   *control.GlobalDropCommand
	authorize func() bool
	reply     chan control.GlobalDropResult
}

func (w *World) GlobalDropStatus(ctx context.Context) (control.GlobalDropStatus, error) {
	r, err := w.enqueueGlobalDrop(&globalDropRequest{ctx: ctx})
	return r.Status, err
}
func (w *World) GlobalDropCommand(ctx context.Context, actor string, cmd control.GlobalDropCommand, authorize func() bool) (control.GlobalDropResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.GlobalDropResult{}, err
	}
	return w.enqueueGlobalDrop(&globalDropRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}
func (w *World) enqueueGlobalDrop(req *globalDropRequest) (control.GlobalDropResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.GlobalDropResult{}, err
	}
	if !w.globalDropPending.CompareAndSwap(false, true) {
		return control.GlobalDropResult{}, control.ErrBusy
	}
	req.reply = make(chan control.GlobalDropResult, 1)
	select {
	case w.commands <- command{globalDrop: req, queuedAt: time.Now()}:
	default:
		w.globalDropPending.Store(false)
		return control.GlobalDropResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.GlobalDropResult{}, req.ctx.Err()
	}
}
func (w *World) globalDropSnapshot() control.GlobalDropStatus {
	if w.globalDropEpoch == "" {
		var b [16]byte
		if _, err := rand.Read(b[:]); err != nil {
			panic(err)
		}
		w.globalDropEpoch = hex.EncodeToString(b[:])
	}
	w.tickGlobalDrop(w.now())
	s := control.GlobalDropStatus{}
	if w.globalDrop != nil {
		s = w.globalDrop.status
	}
	s.Version, s.Epoch, s.AsOf = 1, w.globalDropEpoch, w.now().UTC()
	return s
}
func (w *World) handleGlobalDrop(req *globalDropRequest) {
	defer w.globalDropPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.GlobalDropResult{Code: "ok", Status: w.globalDropSnapshot()}
	if req.command == nil {
		req.reply <- result
		return
	}
	c := *req.command
	result.OperationID = c.OperationID
	finish := func(code string) { result.Code = code; req.reply <- result }
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
	if w.shuttingDown {
		finish("server_maintenance")
		return
	}
	if c.Epoch != w.globalDropEpoch {
		finish("stale_event")
		return
	}
	key := req.actor + ":" + c.OperationID
	if receipt, exists := w.globalDropReceipts[key]; exists {
		if receipt.command != c {
			finish("operation_conflict")
			return
		}
		result = receipt.result
		result.Replayed = true
		// Return current state, not a stale copy from before later kills/stops.
		result.Status = w.globalDropSnapshot()
		req.reply <- result
		return
	}
	// Exhausting the bounded receipt cache must never prevent stopping the
	// current event. This permits at most one extra receipt until restart.
	canStop := c.Action == "stop" && result.Status.Active && c.ExpectedEventID == result.Status.EventID
	if len(w.globalDropReceipts) >= 1024 && !canStop {
		finish("operation_capacity")
		return
	}
	code := "ok"
	switch {
	case c.ExpectedEventID != result.Status.EventID:
		code = "stale_event"
	case c.Action == "start" && result.Status.Active:
		code = "event_active"
	case c.Action == "stop" && !result.Status.Active:
		code = "event_inactive"
	case c.Action == "start":
		item, exists := w.items[c.ItemID]
		if !exists || strings.TrimSpace(item.Name) == "" {
			code = "unknown_item"
			break
		}
		now := w.now().UTC()
		var eventID [16]byte
		if _, err := rand.Read(eventID[:]); err != nil {
			code = "event_id_unavailable"
			break
		}
		s := control.GlobalDropStatus{EventID: hex.EncodeToString(eventID[:]), Active: true, ItemID: c.ItemID, ItemName: item.Name,
			RatePercent: c.RatePercent, MaxDrops: c.MaxDrops, StartedAt: &now, Actor: req.actor}
		if c.DurationSeconds > 0 {
			end := now.Add(time.Duration(c.DurationSeconds) * time.Second)
			s.EndsAt = &end
		}
		w.globalDrop = &globalDropState{status: s}
	case c.Action == "stop":
		w.endGlobalDrop("manual")
	}
	result.Code, result.Status = code, w.globalDropSnapshot()
	if w.globalDropReceipts == nil {
		w.globalDropReceipts = make(map[string]globalDropReceipt)
	}
	w.globalDropReceipts[key] = globalDropReceipt{command: c, result: result}
	log.Printf("[web-admin] global-drop actor=%q operation=%q action=%q reason=%q result=%q item=%d rate=%d max=%d seconds=%d", req.actor, c.OperationID, c.Action, c.Reason, code, c.ItemID, c.RatePercent, c.MaxDrops, c.DurationSeconds)
	req.reply <- result
}
func (w *World) endGlobalDrop(reason string) {
	if w.globalDrop == nil || !w.globalDrop.status.Active {
		return
	}
	s := &w.globalDrop.status
	s.Active, s.EndedBy = false, reason
	log.Printf("[event] global-drop id=%q ended=%q dropped=%d", s.EventID, reason, s.Dropped)
}
func (w *World) tickGlobalDrop(now time.Time) {
	if w.globalDrop != nil && w.globalDrop.status.EndsAt != nil && !now.Before(*w.globalDrop.status.EndsAt) {
		w.endGlobalDrop("time")
	}
}

// Reservation prevents a multi-target kill from exceeding the cap before the
// shared account transaction is committed. Failed saves release reservations.
type globalDropReservation struct {
	event   *globalDropState
	settled bool
}

func (w *World) settleGlobalDrop(r *globalDropReservation, delivered bool) {
	if r == nil || r.settled {
		return
	}
	r.settled = true
	r.event.reserved--
	if delivered {
		r.event.status.Dropped++
	}
	if r.event == w.globalDrop && r.event.status.MaxDrops > 0 && r.event.status.Dropped >= r.event.status.MaxDrops {
		w.endGlobalDrop("quantity")
	}
}
func (w *World) planGlobalDrop(p *Player, m *Mob) []plannedDrop {
	w.tickGlobalDrop(w.now())
	e := w.globalDrop
	if e == nil || !e.status.Active || p == nil || p.Char == nil || p.Account == nil || p.PersistencePoisoned || m == nil || m.Def == nil || m.Def.Tipo != model.TipoMonstro || m.GuildWarTower {
		return nil
	}
	if e.status.MaxDrops > 0 && e.status.Dropped+e.reserved >= e.status.MaxDrops {
		return nil
	}
	if w.intn(100) >= e.status.RatePercent {
		return nil
	}
	item, err := materializeItem(model.Item{Index: e.status.ItemID})
	if err != nil {
		return nil
	}
	pos := addToInv(p.Char, item)
	e.reserved++
	return []plannedDrop{{player: p, inventoryPos: pos, item: item, x: m.X, y: m.Y,
		instanceID: strings.TrimSpace(m.InstanceID), source: "global-drop", sourceSlot: -1,
		globalDrop: &globalDropReservation{event: e}}}
}
