package game

import (
	"encoding/binary"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const portalActivationRadius = 3

func teleportAt(teleports []model.Teleport, x, y uint16) (model.Teleport, bool) {
	for _, portal := range teleports {
		if absDiff(x, portal.SourceX) <= portalActivationRadius &&
			absDiff(y, portal.SourceY) <= portalActivationRadius {
			return portal, true
		}
	}
	return model.Teleport{}, false
}

// teleportPlayer e o caminho unico para teleportes server-side. Persiste antes
// de confirmar, recalcula visibilidade nas duas regioes e manda a coordenada
// final ao proprio client e aos observadores que continuaram no mesmo raio.
func (w *World) teleportPlayer(p *Player, x, y uint16) bool {
	x, y, ok := w.resolvePlayerTeleportDestination(p, x, y)
	if !ok {
		return false
	}
	oldX, oldY := p.X, p.Y
	p.X, p.Y = x, y
	p.Char.X, p.Char.Y = x, y
	if err := w.saveAccount(p.Account); err != nil {
		p.X, p.Y = oldX, oldY
		p.Char.X, p.Char.Y = oldX, oldY
		log.Printf("[#%d] salvar teleporte: %v", p.Session.ID, err)
		return false
	}
	w.publishPlayerTeleport(p)
	return true
}

func (w *World) resolvePlayerTeleportDestination(p *Player, x, y uint16) (uint16, uint16, bool) {
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld ||
		x == 0 || y == 0 || !w.terrain.Walkable(x, y) {
		return 0, 0, false
	}
	x, y = w.findFreePlayerPosition(x, y, 3, p)
	return x, y, w.terrain.Walkable(x, y)
}

func (w *World) publishPlayerTeleport(p *Player) {
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	w.refreshPlayerVisibility(p)
	w.sendToPlayerView(p, func() []byte {
		return wire.ActionStop(p.ID, p.X, p.Y)
	})
}

// onReqTeleport trata o MSG_ReqTeleport nativo 0x290. O pacote nao informa o
// destino: a posicao atual seleciona uma entrada autoritativa de teleports.ini.
func (w *World) onReqTeleport(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || len(pkt) != 16 {
		return
	}
	portal, ok := teleportAt(w.teleports, p.X, p.Y)
	if !ok {
		s.Send(wire.MessagePanel("Teleport unavailable from this position."))
		log.Printf("[#%d] teleporte sem rota @(%d,%d)", s.ID, p.X, p.Y)
		return
	}
	if p.Char.Gold < portal.Price {
		s.Send(wire.MessagePanel("Not enough gold for the teleport."))
		return
	}
	oldGold := p.Char.Gold
	p.Char.Gold -= portal.Price
	oldX, oldY := p.X, p.Y
	w.cancelTrade(p, "teleporte")
	if !w.teleportPlayer(p, portal.DestX, portal.DestY) {
		p.Char.Gold = oldGold
		s.Send(wire.MessagePanel("The teleport could not be completed."))
		return
	}
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	if portal.Name != "" {
		s.Send(wire.MessagePanel(portal.Name))
	}
	log.Printf("[#%d] TELEPORTE %q @(%d,%d)->(%d,%d) gold=-%d",
		s.ID, portal.Name, oldX, oldY, p.X, p.Y, portal.Price)
}

// onPKMode preserva o estado solicitado pelo 0x399 e publica o PKInfo 0x166.
// O servidor continua autoritativo: somente 0/1 e aceito.
func (w *World) onPKMode(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != 16 {
		return
	}
	value := binary.LittleEndian.Uint32(pkt[12:16])
	if value > 1 {
		w.recordSecurityViolation(s, wire.OpPKMode, "estado PK fora do contrato 0/1")
		return
	}
	enabled := value == 1
	p.PKMode = enabled
	if enabled {
		w.cancelTrade(p, "modo PK ativado")
	}
	state := uint32(0)
	message := "PK mode disabled."
	if enabled {
		state = 1
		message = "PK mode enabled."
	}
	w.sendToPlayerView(p, func() []byte {
		return wire.StandardParm(wire.OpPKInfo, p.ID, state)
	})
	s.Send(wire.MessagePanel(message))
	log.Printf("[#%d] PK mode=%t", s.ID, enabled)
}

// 0x28E/0x28F pertencem ao sistema de disputa de cidades/guildas do TMSrv.
// Reconhecer explicitamente evita tratar o pacote como desconhecido enquanto
// esse estado de guerra ainda nao existe no emulador.
func (w *World) onGuildChallenge(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) < 16 {
		return
	}
	target := binary.LittleEndian.Uint32(pkt[12:16])
	s.Send(wire.MessagePanel("City sieges are not enabled yet."))
	log.Printf("[#%d] desafio de guild ignorado com seguranca target=%d", s.ID, target)
}
