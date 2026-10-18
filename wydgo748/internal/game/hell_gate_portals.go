package game

import (
	"strings"

	"wydgo/internal/model"
)

const hellGatePortalFallbackRadius = 32

type hellGatePortalArena struct {
	centerX uint16
	centerY uint16
	radius  int
}

func (arena hellGatePortalArena) contains(x, y uint16) bool {
	return arena.radius > 0 && chebyshev(x, y, arena.centerX, arena.centerY) <= arena.radius
}

func hellGatePortalArenaFor(cfg *model.VolatileHellGate, destX, destY uint16) hellGatePortalArena {
	arena := hellGatePortalArena{centerX: destX, centerY: destY, radius: hellGatePortalFallbackRadius}
	if cfg == nil {
		return arena
	}
	for _, quadrant := range cfg.Quadrants {
		if (quadrant.X != destX || quadrant.Y != destY) &&
			(quadrant.SpawnX != destX || quadrant.SpawnY != destY) {
			continue
		}
		centerX, centerY := quadrant.X, quadrant.Y
		if centerX == 0 || centerY == 0 {
			centerX, centerY = quadrant.SpawnX, quadrant.SpawnY
		}
		radius := quadrant.AreaRadius
		if radius <= 0 {
			radius = hellGatePortalFallbackRadius
		}
		return hellGatePortalArena{centerX: centerX, centerY: centerY, radius: radius}
	}
	return arena
}

func hellGatePortalCoordinateKey(x, y uint16) uint32 {
	return uint32(x)<<16 | uint32(y)
}

// hellGatePortalDestination keeps the correction in the room's terrain
// component. The portal destination is also a mob spawn anchor, so the anchor
// itself may be occupied while an adjacent walkable tile remains valid.
func (w *World) hellGatePortalDestination(inst *ItemInstance, portal model.Teleport, exceptPlayer *Player) (uint16, uint16, bool) {
	if w == nil || inst == nil || portal.DestX == 0 || portal.DestY == 0 ||
		!w.terrain.Walkable(portal.DestX, portal.DestY) {
		return 0, 0, false
	}
	arena := hellGatePortalArenaFor(inst.Config.HellGate, portal.DestX, portal.DestY)
	if !arena.contains(portal.DestX, portal.DestY) {
		return 0, 0, false
	}
	type node struct {
		x, y     uint16
		distance int
	}
	queue := []node{{x: portal.DestX, y: portal.DestY}}
	seen := map[uint32]struct{}{hellGatePortalCoordinateKey(portal.DestX, portal.DestY): {}}
	space := itemInstanceGameplaySpace(inst)
	// Keep the tie-break stable so the chosen tile is deterministic across
	// retries and the persisted/proposed coordinate is auditable.
	directions := [...]struct{ x, y int }{
		{1, 0}, {-1, 0}, {0, 1}, {0, -1},
		{1, 1}, {-1, 1}, {1, -1}, {-1, -1},
	}
	for cursor := 0; cursor < len(queue); cursor++ {
		current := queue[cursor]
		if !w.positionOccupiedInGameplaySpace(current.x, current.y, space, nil, exceptPlayer, nil) {
			return current.x, current.y, true
		}
		if current.distance >= arena.radius {
			continue
		}
		for _, direction := range directions {
			nx, ny := int(current.x)+direction.x, int(current.y)+direction.y
			if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
				continue
			}
			x, y := uint16(nx), uint16(ny)
			if !arena.contains(x, y) {
				continue
			}
			key := hellGatePortalCoordinateKey(x, y)
			if _, visited := seen[key]; visited || !w.terrain.Walkable(x, y) ||
				!w.terrain.RouteHeightCompatible(current.x, current.y, x, y) {
				continue
			}
			if direction.x != 0 && direction.y != 0 &&
				!w.terrain.Walkable(x, current.y) && !w.terrain.Walkable(current.x, y) {
				continue
			}
			seen[key] = struct{}{}
			queue = append(queue, node{x: x, y: y, distance: current.distance + 1})
		}
	}
	return 0, 0, false
}

// handleHellGatePortal consumes every portal request from a member of a timed
// Hell Gate runtime. The client still sends the ordinary 0x290 request with no
// destination; the instance owns the route and the member's gameplay space.
func (w *World) handleHellGatePortal(p *Player) bool {
	if w == nil || p == nil {
		return false
	}
	runtimeID := w.playerRuntimeInstanceID(p.ID)
	if runtimeID == "" {
		return false
	}
	inst := w.itemInstances[runtimeID]
	if inst == nil || !hellGateVersion2(inst) || !itemInstanceHasMember(inst, p.ID) {
		return false
	}
	// A timed-event member owns the request even when the route or phase is invalid.
	// This prevents a matching global route from bypassing the instance rules.
	cfg, ok := hellGateInstance(inst)
	if !ok || p.Char == nil || p.Account == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return true
	}
	if inst.RewardGranted || !inst.ExitAt.IsZero() {
		return true
	}
	state := strings.ToLower(strings.TrimSpace(inst.State))
	if state != "lich_puzzle" && state != "protection" {
		return true
	}
	if !inst.HardDeadline.IsZero() && !w.now().Before(inst.HardDeadline) {
		return true
	}
	portal, ok := teleportAt(cfg.Portals, p.X, p.Y)
	if !ok || portal.DestX == 0 || portal.DestY == 0 {
		return true
	}
	if cfg.Version == 3 {
		// The corridor chosen by the server is the only entrance to the arena.
		// Reject before persistence and consume the request to block global fallback.
		if inst.HellGateVariant < 1 || inst.HellGateVariant > len(cfg.Portals) {
			return true
		}
		selected := cfg.Portals[inst.HellGateVariant-1]
		if portal.SourceX/4 != selected.SourceX/4 || portal.SourceY/4 != selected.SourceY/4 {
			return true
		}
	}
	if w.criticalInstanceReserved() {
		return true
	}

	space := itemInstanceGameplaySpace(inst)
	destX, destY, ok := w.hellGatePortalDestination(inst, portal, p)
	if !ok {
		return true
	}
	tile := criticalTile(destX, destY, space)
	reserved := false
	if w.criticalWrites != nil {
		if w.criticalTeleportTiles[tile] != nil {
			return true
		}
		if w.criticalTeleportTiles == nil {
			w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
		}
		w.criticalTeleportTiles[tile] = p
		reserved = true
	}

	oldX, oldY := p.X, p.Y
	oldCharX, oldCharY := p.Char.X, p.Char.Y
	oldRuntimeID := runtimeID
	p.X, p.Y = destX, destY
	p.Char.X, p.Char.Y = destX, destY
	releaseReservation := func() {
		if reserved && w.criticalTeleportTiles[tile] == p {
			delete(w.criticalTeleportTiles, tile)
		}
	}
	rollback := func() {
		p.X, p.Y = oldX, oldY
		p.Char.X, p.Char.Y = oldCharX, oldCharY
	}
	retainedCheck := func() error {
		if p.X != oldX || p.Y != oldY || w.playerRuntimeInstanceID(p.ID) != oldRuntimeID {
			return errCriticalPersistenceIdentity
		}
		if reserved && w.criticalTeleportTiles[tile] != p {
			return errCriticalPersistenceIdentity
		}
		if w.positionOccupiedInGameplaySpace(destX, destY, space, nil, p, nil) {
			return errCriticalPersistenceIdentity
		}
		return nil
	}
	committed := w.commitInstanceMutation("Hell Gate timed portal", []*model.Account{p.Account}, rollback,
		func() {
			releaseReservation()
			w.publishInstanceMemberRelocation([]*Player{p})
		}, releaseReservation, retainedCheck)
	if !committed {
		releaseReservation()
	}
	return true
}
