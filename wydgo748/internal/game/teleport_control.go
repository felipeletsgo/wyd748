package game

import (
	"context"
	"log"
	"time"
	"wydgo/internal/control"
)

// MODERNIZACAO_COMPATIVEL: reuse the existing authoritative teleport lifecycle.
type teleportReceipt struct {
	command control.TeleportCommand
	result  control.TeleportResult
}
type teleportRequest struct {
	ctx       context.Context
	actor     string
	command   control.TeleportCommand
	authorize func() bool
	reply     chan control.TeleportResult
}

func (w *World) TeleportCommand(ctx context.Context, actor string, c control.TeleportCommand, authorize func() bool) (control.TeleportResult, error) {
	if err := c.Validate(); err != nil {
		return control.TeleportResult{}, err
	}
	if err := ctx.Err(); err != nil {
		return control.TeleportResult{}, err
	}
	if !w.teleportPending.CompareAndSwap(false, true) {
		return control.TeleportResult{}, control.ErrBusy
	}
	req := &teleportRequest{ctx: ctx, actor: actor, command: c, authorize: authorize, reply: make(chan control.TeleportResult, 1)}
	select {
	case w.commands <- command{teleport: req, queuedAt: time.Now()}:
	default:
		w.teleportPending.Store(false)
		return control.TeleportResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-ctx.Done():
		return control.TeleportResult{}, ctx.Err()
	}
}

func (w *World) handleTeleport(req *teleportRequest) {
	defer w.teleportPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	c := req.command
	r := control.TeleportResult{OperationID: c.OperationID}
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
	if receipt, exists := w.teleportReceipts[key]; exists {
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
	// Receipts are never evicted in an epoch: a delayed retry cannot teleport again.
	if len(w.teleportReceipts) >= 1024 {
		finish("operation_capacity")
		return
	}
	r.Code = "stale_target"
	for s, p := range w.players {
		if s == nil || s.ID != c.ExpectedSessionID || p == nil || p.Session != s || !p.InWorld || p.Char == nil || p.Account == nil ||
			p.Char.UID != c.UID || accountSessionKey(p.Account.Name) != c.Account || s.IsClosed() {
			continue
		}

		r.Code = w.adminTeleportPlayer(p, c.Destination)
		if r.Code == "ok" {
			r.X, r.Y = p.X, p.Y
		}
		break
	}
	if w.teleportReceipts == nil {
		w.teleportReceipts = make(map[string]teleportReceipt)
	}
	w.teleportReceipts[key] = teleportReceipt{command: c, result: r}
	log.Printf("[web-admin] teleport actor=%q operation=%q reason=%q result=%q uid=%q account=%q session=%d destination=%q x=%d y=%d", req.actor, c.OperationID, c.Reason, r.Code, c.UID, c.Account, c.ExpectedSessionID, c.Destination, r.X, r.Y)
	req.reply <- r
}

// MODERNIZACAO_COMPATIVEL: no arbitrary coordinates or bypass of instance/war admission.
func (w *World) adminTeleportPlayer(p *Player, destination string) string {
	if p.PersistencePoisoned || playerCurHP(p.Char) == 0 || p.Trade != nil || p.GhostShop != nil {
		return "player_busy"
	}
	if w.gameplaySpaceForPlayer(p) != "" || w.itemInstanceForPlayer(p.ID) != nil || w.itemInstanceExitGraceForPlayer(p.ID) != nil {
		return "restricted_area"
	}
	if _, fighting := w.cityFighters[p]; fighting {
		return "restricted_area"
	}
	if towerWarArena.contains(p.X, p.Y) {
		return "restricted_area"
	}
	for _, zone := range cityWarZones {
		if zone.arena.contains(p.X, p.Y) {
			return "restricted_area"
		}
	}
	index := -1
	switch destination {
	case "armia":
		index = 0
	case "azran":
		index = 1
	case "erion":
		index = 2
	case "nippleheim":
		index = 3
	}
	if index < 0 {
		return "invalid_command"
	}
	zone := cityWarZones[index]
	x, y, ok := w.resolvePlayerTeleportDestination(p, zone.exitX, zone.exitY)
	// The normal resolver falls back to its input if every nearby tile is full.
	// Refuse that fallback and any candidate outside the public city.
	if !ok || !zone.city.contains(x, y) || w.positionOccupiedExcept(x, y, nil, p) {
		return "destination_unavailable"
	}
	if !w.teleportPlayer(p, x, y) {
		return "persistence_failed"
	}
	// An in-flight walk must not resume from its pre-teleport origin next tick.
	clearPublishedPlayerMove(p)
	p.ShopNPC, p.ShopTax, p.CraftNPC, p.CargoNPC, p.BrowsingGhostShopID = 0, 0, 0, 0, 0
	clearCityWarContext(p)
	return "ok"
}
