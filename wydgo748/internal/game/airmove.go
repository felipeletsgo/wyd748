package game

import (
	"encoding/binary"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Native 7.48 g_pAirMoveRoute[5]. Only endpoints belong to the authoritative
// contract; the intermediate route is visual.
var airMoveDestinations = [...]struct{ x, y uint16 }{
	{2112, 2051},
	{2372, 2099},
	{2220, 1714},
	{2365, 2279},
	{1826, 1771},
}

const (
	airMoveSourceChunk = 16
	airMovePacketSize  = 20
	// Two 2800 ms states surround native flight, allowing 200 ms of packet
	// latency tolerance.
	airMoveMinimumDuration = 5400 * time.Millisecond
)

func isAirMoveNPC(def *model.NPCDef) bool {
	return def != nil && def.Score != nil && def.Equip.Rosto.Index == 63 &&
		def.Score.Merchant&0xF == 7
}

func (w *World) hasVisibleAirMoveNPC(p *Player) bool {
	for _, m := range w.nearbyMobs(p.X, p.Y, viewHalfX) {
		if m.ID >= 1000 && p.hasVisible(m.ID) && isAirMoveNPC(m.Def) {
			return true
		}
	}
	return false
}

func clearAirMove(p *Player) {
	p.AirMoveActive = false
	p.AirMoveRoute = 0
	p.AirMoveStartedAt = time.Time{}
	p.AirMoveSourceX, p.AirMoveSourceY = 0, 0
}

// onAirMove receives only route and mode, not an NPC ID or destination. The UI
// requires a visible Head=63/Merchant=7 NPC in chunk 16; the table owns endpoints.
func (w *World) onAirMove(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != airMovePacketSize || w.criticalTradeReserved(p) {
		return
	}
	route := binary.LittleEndian.Uint32(pkt[12:16])
	mode := binary.LittleEndian.Uint32(pkt[16:20])
	if playerCurHP(p.Char) == 0 {
		clearAirMove(p)
		// A late finish after death is not malicious replay. The canceled trip
		// must not grant its destination.
		if mode == 2 {
			return
		}
		w.recordSecurityViolation(s, wire.OpAirMove, "flight started while dead")
		return
	}
	if binary.LittleEndian.Uint16(pkt[6:8]) != p.ID || route >= uint32(len(airMoveDestinations)) ||
		(mode != 1 && mode != 2) {
		w.recordSecurityViolation(s, wire.OpAirMove, "invalid ID, route, mode or state")
		return
	}

	if mode == 1 {
		if p.AirMoveActive || int(p.X)/128 != airMoveSourceChunk ||
			int(p.Y)/128 != airMoveSourceChunk || !w.hasVisibleAirMoveNPC(p) {
			w.recordSecurityViolation(s, wire.OpAirMove, "flight started outside native UI context")
			return
		}
		w.cancelTrade(p, "AirMove")
		clearPublishedPlayerMove(p)
		p.AirMoveActive = true
		p.AirMoveRoute = int(route)
		p.AirMoveStartedAt = w.now()
		p.AirMoveSourceX, p.AirMoveSourceY = p.X, p.Y
		return
	}

	if !p.AirMoveActive || p.AirMoveRoute != int(route) {
		w.recordSecurityViolation(s, wire.OpAirMove, "flight finished without a matching start")
		return
	}
	startedAt := p.AirMoveStartedAt
	sourceX, sourceY := p.AirMoveSourceX, p.AirMoveSourceY
	clearAirMove(p)
	if p.X != sourceX || p.Y != sourceY || w.now().Sub(startedAt) < airMoveMinimumDuration {
		w.recordSecurityViolation(s, wire.OpAirMove, "early flight finish or changed origin")
		w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
		return
	}
	dest := airMoveDestinations[route]
	if !w.requestPlayerTeleport(p, dest.x, dest.y, nil, nil) {
		s.Send(wire.MessagePanel("The flight could not be completed."))
		w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
	}
}
