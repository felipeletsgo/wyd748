package game

import (
	"encoding/binary"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// g_pAirMoveRoute[5] do client nativo 7.48. Somente os pontos terminais
// pertencem ao contrato autoritativo; o percurso intermediario e visual.
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
	// Dois estados de 2800 ms cercam o voo nativo. Deixamos 200 ms de
	// tolerancia para a diferenca de latencia entre os dois pacotes.
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

// onAirMove recebe somente a rota e o modo. Nao ha NPC ID nem destino no
// pacote: a abertura da UI depende de um NPC visivel com Head=63/Merchant=7
// no chunk 16, e o destino e sempre derivado da tabela nativa acima.
func (w *World) onAirMove(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != airMovePacketSize {
		return
	}
	route := binary.LittleEndian.Uint32(pkt[12:16])
	mode := binary.LittleEndian.Uint32(pkt[16:20])
	if playerCurHP(p.Char) == 0 {
		clearAirMove(p)
		// O fim tardio apos morte nao e um replay malicioso; o servidor
		// ja cancelou a viagem e nao deve conceder o destino.
		if mode == 2 {
			return
		}
		w.recordSecurityViolation(s, wire.OpAirMove, "inicio durante morte")
		return
	}
	if binary.LittleEndian.Uint16(pkt[6:8]) != p.ID || route >= uint32(len(airMoveDestinations)) ||
		(mode != 1 && mode != 2) {
		w.recordSecurityViolation(s, wire.OpAirMove, "ID, rota, modo ou estado invalido")
		return
	}

	if mode == 1 {
		if p.AirMoveActive || int(p.X)/128 != airMoveSourceChunk ||
			int(p.Y)/128 != airMoveSourceChunk || !w.hasVisibleAirMoveNPC(p) {
			w.recordSecurityViolation(s, wire.OpAirMove, "inicio fora da UI nativa")
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
		w.recordSecurityViolation(s, wire.OpAirMove, "fim sem inicio correspondente")
		return
	}
	startedAt := p.AirMoveStartedAt
	sourceX, sourceY := p.AirMoveSourceX, p.AirMoveSourceY
	clearAirMove(p)
	if p.X != sourceX || p.Y != sourceY || w.now().Sub(startedAt) < airMoveMinimumDuration {
		w.recordSecurityViolation(s, wire.OpAirMove, "fim antecipado ou origem alterada")
		w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
		return
	}
	dest := airMoveDestinations[route]
	if !w.teleportPlayer(p, dest.x, dest.y) {
		s.Send(wire.MessagePanel("The flight could not be completed."))
		w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
	}
}
