package game

import (
	"fmt"
	"log"
	"strconv"
	"strings"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Staff coordinate teleport. Server-only: it reuses the chat/whisper packets
// and the authoritative teleport path, so no client contract changes. Only
// accounts whose staff entry grants control.GotoCapability may use it; for any
// other player "/goto" keeps its previous meaning (a whisper to that name).

// gotoSearchRadius bounds the walkable-tile search around a blocked target, so
// auditing a wall or water tile lands on the nearest reachable cell.
const gotoSearchRadius = 16

// WithCommandAuthorizer supplies the staff check for privileged in-game
// commands. The authorizer receives the canonical account key and capability.
func WithCommandAuthorizer(authorize func(account, capability string) bool) WorldOption {
	return func(w *World) { w.commandAuthorizer = authorize }
}

func parseGotoCoordinates(arg string) (uint16, uint16, bool) {
	fields := strings.FieldsFunc(arg, func(r rune) bool { return r == ' ' || r == ',' || r == '\t' })
	if len(fields) != 2 {
		return 0, 0, false
	}
	x, errX := strconv.Atoi(fields[0])
	y, errY := strconv.Atoi(fields[1])
	if errX != nil || errY != nil || x <= 0 || y <= 0 || x >= model.TerrainWidth || y >= model.TerrainHeight {
		return 0, 0, false
	}
	return uint16(x), uint16(y), true
}

// nearestWalkable returns the closest walkable tile to (x, y) within radius.
func (w *World) nearestWalkable(x, y uint16, radius int) (uint16, uint16, bool) {
	for distance := 0; distance <= radius; distance++ {
		for dy := -distance; dy <= distance; dy++ {
			for dx := -distance; dx <= distance; dx++ {
				if distance > 0 && absInt(dx) != distance && absInt(dy) != distance {
					continue
				}
				nx, ny := int(x)+dx, int(y)+dy
				if nx <= 0 || ny <= 0 || nx >= model.TerrainWidth || ny >= model.TerrainHeight {
					continue
				}
				if w.terrain.Walkable(uint16(nx), uint16(ny)) {
					return uint16(nx), uint16(ny), true
				}
			}
		}
	}
	return 0, 0, false
}

// staffGoto reports whether it consumed the command.
func (w *World) staffGoto(s *net.Session, p *Player, arg string) bool {
	if w.commandAuthorizer == nil || p.Account == nil ||
		!w.commandAuthorizer(accountSessionKey(p.Account.Name), control.GotoCapability) {
		return false
	}
	x, y, ok := parseGotoCoordinates(arg)
	if !ok {
		s.Send(wire.MessagePanel(fmt.Sprintf("Usage: /goto X Y (1..%d).", model.TerrainWidth-1)))
		return true
	}
	if p.PersistencePoisoned || w.criticalTradeReserved(p) || playerCurHP(p.Char) == 0 ||
		p.Trade != nil || p.GhostShop != nil {
		s.Send(wire.MessagePanel("/goto is unavailable while dead, trading or saving."))
		return true
	}
	if w.gameplaySpaceForPlayer(p) != "" || w.itemInstanceForPlayer(p.ID) != nil ||
		w.itemInstanceExitGraceForPlayer(p.ID) != nil {
		s.Send(wire.MessagePanel("Leave the instance before using /goto."))
		return true
	}
	if _, fighting := w.cityFighters[p]; fighting {
		s.Send(wire.MessagePanel("/goto is unavailable during a city war."))
		return true
	}
	wx, wy, ok := w.nearestWalkable(x, y, gotoSearchRadius)
	if ok {
		wx, wy, ok = w.resolvePlayerTeleportDestination(p, wx, wy)
	}
	if !ok || w.positionOccupiedExcept(wx, wy, nil, p) {
		s.Send(wire.MessagePanel(fmt.Sprintf("No free walkable tile near %d,%d.", x, y)))
		return true
	}
	log.Printf("[#%d] STAFF goto account=%q char=%q (%d,%d)->(%d,%d) requested=(%d,%d)",
		s.ID, p.Account.Name, p.Char.Name, p.X, p.Y, wx, wy, x, y)
	admitted := w.requestPlayerTeleport(p, wx, wy, func() {
		w.finishAdminTeleport(p)
		s.Send(wire.MessagePanel(fmt.Sprintf("Teleported to %d,%d.", p.X, p.Y)))
	}, func() {
		s.Send(wire.MessagePanel("Teleport could not be saved; try again."))
	})
	if !admitted {
		s.Send(wire.MessagePanel("Teleport is busy; try again."))
	}
	return true
}
