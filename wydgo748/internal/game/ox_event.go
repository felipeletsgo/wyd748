package game

import (
	"context"
	"log"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The OX quiz arena follows the 7.59 TMSRV staff commands. "quizmode" lets
// the Entrance_Ticket (1737, volatile 51) teleport to its quizArena; "quizdoor" locks
// or opens the six OX gates (InitItem rows 35..40, sub_561C10); "quizresultx"
// and "quizresulto" revive and move every player standing on the losing side
// to (1341,1465) (sub_566230).
const (
	oxExitX, oxExitY = 1341, 1465
	oxGateX          = 1341
)

var (
	oxSideX   = warRect{1296, 1481, oxGateX - 1, 1518}
	oxSideO   = warRect{oxGateX + 1, 1481, 1387, 1518}
	oxGateRow = warRect{oxGateX, 1481, oxGateX, 1518}
)

// oxGates returns the OX arena gates (471 on column 1341).
func (w *World) oxGates() []*GroundItem {
	var gates []*GroundItem
	for _, g := range w.nearbyGroundItems(oxGateX, 1500, 20) {
		if g != nil && g.Permanent && g.Item.Index == arenaGateItem && oxGateRow.contains(g.X, g.Y) {
			gates = append(gates, g)
		}
	}
	return gates
}

// setOXDoors ports sub_561C10: 1 locks every gate, 0 opens them.
func (w *World) setOXDoors(locked bool) int {
	state := gateOpen
	if locked {
		state = gateClosed
	}
	changed := 0
	for _, g := range w.oxGates() {
		if w.setFixtureState(g, state) {
			changed++
		}
	}
	return changed
}

func (w *World) oxPlayers(side warRect) []*Player {
	var out []*Player
	for _, p := range w.nearbyWorldPlayers(1341, 1500, 50) {
		if p != nil && p.InWorld && p.Char != nil && side.contains(p.X, p.Y) {
			out = append(out, p)
		}
	}
	return out
}

// clearOXSide moves the losing side out of the arena; the dead are revived.
func (w *World) clearOXSide(side warRect) int {
	moved := 0
	for _, p := range w.oxPlayers(side) {
		if w.moveOutOfArena(p, oxExitX, oxExitY) {
			moved++
		}
	}
	return moved
}

// oxTicketRule reports the OX entry of a rule with quizArena while quiz mode
// runs. EXTENSAO_COORDENADA: outside quiz mode this project keeps the
// Entrance_Ticket as the Big Cube ticket (instances.json rule 51) instead of
// the 7.59 _NN_Only_quiz_time reply.
func (w *World) oxTicketRule(item model.Item) (model.VolatileRule, bool) {
	if !w.oxQuizMode {
		return model.VolatileRule{}, false
	}
	rule, _, ok := w.volatiles.Rule(item.Index)
	if !ok || rule.QuizArena == nil {
		return model.VolatileRule{}, false
	}
	return model.VolatileRule{Action: "ox_ticket", Consume: true, X: rule.QuizArena.X, Y: rule.QuizArena.Y}, true
}

// useOXTicket consumes the ticket and teleports into the OX arena; critical
// persistence admits it through admitTeleportConsumable.
func (w *World) useOXTicket(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule) {
	if _, known := w.items[item.Index]; !known {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if w.criticalWrites != nil {
		w.admitTeleportConsumable(p, slot, rule, useItemRequest{})
		return
	}
	oldItem := *item
	if rule.Consume {
		consumeOne(item)
	}
	if !w.teleportPlayer(p, rule.X, rule.Y) {
		*item = oldItem
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
}

type oxEventRequest struct {
	ctx       context.Context
	actor     string
	command   *control.OXEventCommand
	authorize func() bool
	reply     chan control.OXEventResult
}

func (w *World) OXEventStatus(ctx context.Context) (control.OXEventStatus, error) {
	r, err := w.enqueueOXEvent(&oxEventRequest{ctx: ctx})
	return r.Status, err
}

func (w *World) OXEventCommand(ctx context.Context, actor string, cmd control.OXEventCommand,
	authorize func() bool) (control.OXEventResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.OXEventResult{}, err
	}
	return w.enqueueOXEvent(&oxEventRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}

func (w *World) enqueueOXEvent(req *oxEventRequest) (control.OXEventResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.OXEventResult{}, err
	}
	if !w.oxEventPending.CompareAndSwap(false, true) {
		return control.OXEventResult{}, control.ErrBusy
	}
	req.reply = make(chan control.OXEventResult, 1)
	select {
	case w.commands <- command{oxEvent: req, queuedAt: time.Now()}:
	default:
		w.oxEventPending.Store(false)
		return control.OXEventResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.OXEventResult{}, req.ctx.Err()
	}
}

func (w *World) oxEventSnapshot() control.OXEventStatus {
	s := control.OXEventStatus{Version: 1, AsOf: w.now().UTC(), QuizMode: w.oxQuizMode,
		PlayersX: len(w.oxPlayers(oxSideX)),
		PlayersO: len(w.oxPlayers(oxSideO))}
	gates := w.oxGates()
	s.Doors, s.DoorsLocked = len(gates), len(gates) > 0
	for _, g := range gates {
		if g.State != gateClosed {
			s.DoorsLocked = false
		}
	}
	return s
}

func (w *World) handleOXEvent(req *oxEventRequest) {
	defer w.oxEventPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.OXEventResult{Code: "ok", Status: w.oxEventSnapshot()}
	if req.command == nil {
		req.reply <- result
		return
	}
	c := *req.command
	result.OperationID = c.OperationID
	finish := func(code string) {
		result.Code, result.Status = code, w.oxEventSnapshot()
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
	if req.ctx.Err() != nil {
		return
	}
	code := "ok"
	switch c.Action {
	case "start":
		// Repeated starts must not unlock the gates during a live question.
		if w.oxQuizMode {
			code = "unchanged"
		} else if len(w.oxGates()) != 6 {
			code = "doors_unavailable"
		} else {
			w.setOXDoors(false)
			w.oxQuizMode = true
		}
	case "mode_on", "mode_off":
		on := c.Action == "mode_on"
		if w.oxQuizMode == on {
			code = "unchanged"
		}
		w.oxQuizMode = on
	case "doors_lock", "doors_open":
		if len(w.oxGates()) == 0 {
			code = "doors_unavailable"
		} else if w.setOXDoors(c.Action == "doors_lock") == 0 {
			code = "unchanged"
		}
	case "result_x":
		result.Moved = w.clearOXSide(oxSideX)
	case "result_o":
		result.Moved = w.clearOXSide(oxSideO)
	}
	log.Printf("[web-admin] ox-event actor=%q operation=%q action=%q reason=%q result=%q moved=%d",
		req.actor, c.OperationID, c.Action, c.Reason, code, result.Moved)
	finish(code)
}
