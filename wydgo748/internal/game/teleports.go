package game

import (
	"encoding/binary"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// O client 7.48 envia 0x290 quando pisa numa celula Teleport (0x10) do
// AttributeMap. Cada celula representa um bloco 4x4 do mundo; teleports.ini
// guarda uma coordenada dentro desse bloco, nao um ponto de ativacao isolado.
const portalAttributeBlockSize = 4

func samePortalAttributeBlock(x, y, sourceX, sourceY uint16) bool {
	const blockMask = ^uint16(portalAttributeBlockSize - 1)
	return x&blockMask == sourceX&blockMask && y&blockMask == sourceY&blockMask
}

func teleportAt(teleports []model.Teleport, x, y uint16) (model.Teleport, bool) {
	for _, portal := range teleports {
		if samePortalAttributeBlock(x, y, portal.SourceX, portal.SourceY) {
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
	if err := w.savePlayerLocation(p); err != nil {
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
	// A teleport forfeits this session's place; summon enrolls only after its
	// own successful teleport. Returning by another route cannot restore it.
	if _, fighting := w.cityFighters[p]; fighting {
		delete(w.cityFighters, p)
		p.Session.Send(wire.WarInfo())
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

func (w *World) onGuildChallenge(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	// Consume the previous interaction before any rejection, including malformed
	// packets. Only a new, validated request below may establish another context.
	contextNPC, contextCity := p.CityWarNPC, p.CityWarCity
	contextOwner, contextUntil := p.CityWarGuild, p.CityWarUntil
	clearCityWarContext(p)
	fail := func(message string) {
		s.Send(wire.MessagePanel(message))
	}
	if len(pkt) != 16 && len(pkt) != 20 {
		fail("Invalid city-war request.")
		return
	}
	opcode := uint16(wire.OpChallenge)
	if len(pkt) == 20 {
		opcode = wire.OpChallengeConfirm
	}
	header := wire.ParseHeader(pkt)
	if header.Type == wire.OpChallenge || header.Type == wire.OpChallengeConfirm {
		opcode = header.Type
	}
	if (opcode == wire.OpChallenge && len(pkt) != 16) || (opcode == wire.OpChallengeConfirm && len(pkt) != 20) {
		fail("Invalid city-war request size.")
		return
	}

	targetRaw := binary.LittleEndian.Uint32(pkt[12:16])
	if targetRaw > uint32(^uint16(0)) {
		fail("Invalid city-war NPC.")
		return
	}
	npcID := uint16(targetRaw)
	m, err := w.resolveNPCInteraction(p, npcID)
	if err != nil {
		fail(npcInteractionMessage(err))
		return
	}
	city, ok := cityCollectorCity(m)
	if !ok {
		fail("That character is not a city-war collector.")
		return
	}

	now := w.now()
	if opcode == wire.OpChallenge {
		if w.guilds == nil {
			fail("Guild registry unavailable.")
			return
		}
		w.tickGuildWars(now)
		local := w.cityWarLocal(now)
		territory := w.guilds.Wars.Cities.Territories[city]
		guild, member := w.guildOf(p.Char)
		if local.Weekday() != time.Saturday && guild != nil && member != nil && member.Rank == model.GuildRankLeader && territory.Owner == guild.ID {
			if err := w.withdrawCityTreasury(p, city); err != nil {
				fail(err.Error())
				return
			}
			return
		}
		if local.Weekday() != time.Saturday || w.guilds.Wars.Cities.Phase != "registration" {
			fail("City-war registration is open on Saturday only.")
			return
		}
		setCityWarContext(p, npcID, city, territory.Owner, now)
		s.Send(wire.GuildChallengePrompt())
		return
	}

	option := binary.LittleEndian.Uint32(pkt[16:20])
	if option != 0 {
		fail("Invalid city-war confirmation.")
		return
	}
	if contextNPC != npcID || contextCity != city || contextUntil.IsZero() || !now.Before(contextUntil) {
		fail("The city-war request expired. Speak to the collector again.")
		return
	}
	if w.guilds == nil {
		fail("Guild registry unavailable.")
		return
	}
	if contextOwner != w.guilds.Wars.Cities.Territories[city].Owner {
		fail("The city-war target changed. Speak to the collector again.")
		return
	}
	if err := w.registerCityWar(p, city); err != nil {
		fail(err.Error())
		return
	}
	s.Send(wire.MessagePanel("Application saved. Fee: 100 guild fame. The highest fame bid challenges the city."))
}
