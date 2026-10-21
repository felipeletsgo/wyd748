package game

import (
	"context"
	"log"
	"time"

	"wydgo/internal/control"
)

// MODERNIZACAO_COMPATIVEL: staff access to the Armia PvP door (7.59 "pkgate").
type pvpDoorRequest struct {
	ctx       context.Context
	actor     string
	command   *control.PvPDoorCommand
	authorize func() bool
	reply     chan control.PvPDoorResult
}

func (w *World) PvPDoorStatus(ctx context.Context) (control.PvPDoorStatus, error) {
	r, err := w.enqueuePvPDoor(&pvpDoorRequest{ctx: ctx})
	return r.Status, err
}

func (w *World) PvPDoorCommand(ctx context.Context, actor string, cmd control.PvPDoorCommand,
	authorize func() bool) (control.PvPDoorResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.PvPDoorResult{}, err
	}
	return w.enqueuePvPDoor(&pvpDoorRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}

func (w *World) enqueuePvPDoor(req *pvpDoorRequest) (control.PvPDoorResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.PvPDoorResult{}, err
	}
	if !w.pvpDoorPending.CompareAndSwap(false, true) {
		return control.PvPDoorResult{}, control.ErrBusy
	}
	req.reply = make(chan control.PvPDoorResult, 1)
	select {
	case w.commands <- command{pvpDoor: req, queuedAt: time.Now()}:
	default:
		w.pvpDoorPending.Store(false)
		return control.PvPDoorResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.PvPDoorResult{}, req.ctx.Err()
	}
}

func (w *World) pvpDoorSnapshot() control.PvPDoorStatus {
	s := control.PvPDoorStatus{Version: 1, AsOf: w.now().UTC(), X: pvpDoorX, Y: pvpDoorY}
	if door := w.pvpDoor(); door != nil {
		s.Available, s.Locked = true, door.State == gateClosed
	}
	return s
}

func (w *World) handlePvPDoor(req *pvpDoorRequest) {
	defer w.pvpDoorPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.PvPDoorResult{Code: "ok", Status: w.pvpDoorSnapshot()}
	if req.command == nil {
		req.reply <- result
		return
	}
	c := *req.command
	result.OperationID = c.OperationID
	finish := func(code string) {
		result.Code, result.Status = code, w.pvpDoorSnapshot()
		req.reply <- result
	}
	switch {
	case c.Validate() != nil:
		finish("invalid_command")
		return
	case req.actor == "" || req.authorize == nil || !req.authorize():
		finish("capability_required")
		return
	case w.shuttingDown:
		finish("server_maintenance")
		return
	}
	changed, found := w.setPvPDoorLocked(c.Action == "lock")
	code := "ok"
	if !found {
		code = "door_unavailable"
	} else if !changed {
		code = "unchanged"
	}
	log.Printf("[web-admin] pvp-door actor=%q operation=%q action=%q reason=%q result=%q",
		req.actor, c.OperationID, c.Action, c.Reason, code)
	finish(code)
}
