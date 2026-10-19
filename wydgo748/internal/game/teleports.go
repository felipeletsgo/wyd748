package game

import (
	"encoding/binary"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The 7.48 client sends 0x290 when it steps on a Teleport cell (0x10) of the
// AttributeMap. Each cell stands for a 4x4 block of the world; teleports.ini
// keeps a coordinate inside that block, not an isolated activation point.
const portalAttributeBlockSize = 4

// Project choice (MODERNIZACAO_COMPATIVEL): accept one extra world tile on
// each side of the existing block, using the server position only. The native
// request, destination rules and fees are unchanged.
const portalActivationMargin = 1

func samePortalAttributeBlock(x, y, sourceX, sourceY uint16) bool {
	const blockMask = ^uint16(portalAttributeBlockSize - 1)
	return x&blockMask == sourceX&blockMask && y&blockMask == sourceY&blockMask
}

func teleportAt(teleports []model.Teleport, x, y uint16) (model.Teleport, bool) {
	// An exact block always wins over a neighboring portal's margin.
	for _, portal := range teleports {
		if samePortalAttributeBlock(x, y, portal.SourceX, portal.SourceY) {
			return portal, true
		}
	}
	for _, portal := range teleports {
		// Use signed bounds so coordinates near zero or 65535 cannot wrap.
		minX := int(portal.SourceX) / portalAttributeBlockSize * portalAttributeBlockSize
		minY := int(portal.SourceY) / portalAttributeBlockSize * portalAttributeBlockSize
		if int(x) >= minX-portalActivationMargin && int(x) <= minX+portalAttributeBlockSize-1+portalActivationMargin &&
			int(y) >= minY-portalActivationMargin && int(y) <= minY+portalAttributeBlockSize-1+portalActivationMargin {
			return portal, true
		}
	}
	return model.Teleport{}, false
}

// teleportPlayer is the single path for server-side teleports. It persists
// before confirming, recomputes visibility in both regions and sends the final
// coordinate to the client itself and to the observers still in range.
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
		log.Printf("[#%d] save teleport: %v", p.Session.ID, err)
		return false
	}
	clearAirMove(p)
	clearPublishedPlayerMove(p)
	w.publishPlayerTeleport(p)
	return true
}

func (w *World) resolvePlayerTeleportDestination(p *Player, x, y uint16) (uint16, uint16, bool) {
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld ||
		x == 0 || y == 0 || !w.terrain.Walkable(x, y) ||
		!hiddenValleyDestinationAllowed(p, x, y, w.now()) ||
		!w.newbieZoneDestinationAllowed(p, x, y) {
		return 0, 0, false
	}
	x, y = w.findFreePlayerPosition(x, y, 3, p)
	return x, y, w.terrain.Walkable(x, y) && hiddenValleyDestinationAllowed(p, x, y, w.now()) &&
		w.newbieZoneDestinationAllowed(p, x, y)
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
	w.relocateOwnerSummons(p)
}

// onReqTeleport handles the native MSG_ReqTeleport 0x290. The packet carries no
// destination: the current position selects an authoritative teleports.ini entry.
func (w *World) onReqTeleport(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || len(pkt) != 16 || w.criticalTradeReserved(p) {
		return
	}
	if w.handleHellGatePortal(p) {
		return
	}
	portal, ok := teleportAt(w.teleports, p.X, p.Y)
	if !ok {
		s.Send(wire.MessagePanel("Teleport unavailable from this position."))
		log.Printf("[#%d] teleport without a route @(%d,%d)", s.ID, p.X, p.Y)
		return
	}
	if isKefraGate(portal) && !w.kefraDefeated() {
		s.Send(wire.MessagePanel(kefraGateClosed))
		return
	}
	if isKefraHallEntrance(portal) && counterBalance(p, kefraTicketCounter) == 0 {
		s.Send(wire.MessagePanel(kefraEntriesRequired))
		return
	}
	if !hiddenValleyDestinationAllowed(p, portal.DestX, portal.DestY, w.now()) {
		s.Send(wire.MessagePanel("Activate a Hidden Ruins map before entering Hidden Valley."))
		return
	}
	if p.Char.Gold < portal.Price {
		s.Send(wire.MessagePanel("Not enough gold for the teleport."))
		return
	}
	if isKefraHallEntrance(portal) {
		w.enterKefraHall(p, portal)
		return
	}
	if w.criticalWrites != nil {
		if p.Account == nil {
			return
		}
		proposed := accountStateSnapshot(p.Account)
		proposed.Chars[p.CharSlot].Gold -= portal.Price
		oldX, oldY := p.X, p.Y
		if err := w.queuePlayerTeleport(p, proposed, portal.DestX, portal.DestY, "paid portal", func() {
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			if portal.Name != "" {
				s.Send(wire.MessagePanel(portal.Name))
			}
			log.Printf("[#%d] TELEPORT %q @(%d,%d)->(%d,%d) gold=-%d", s.ID, portal.Name, oldX, oldY, p.X, p.Y, portal.Price)
		}, nil, nil); err != nil {
			s.Send(wire.MessagePanel("The teleport could not be completed."))
		}
		return
	}
	oldGold := p.Char.Gold
	p.Char.Gold -= portal.Price
	oldX, oldY := p.X, p.Y
	w.cancelTrade(p, "teleport")
	if !w.teleportPlayer(p, portal.DestX, portal.DestY) {
		p.Char.Gold = oldGold
		s.Send(wire.MessagePanel("The teleport could not be completed."))
		return
	}
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	if portal.Name != "" {
		s.Send(wire.MessagePanel(portal.Name))
	}
	log.Printf("[#%d] TELEPORT %q @(%d,%d)->(%d,%d) gold=-%d",
		s.ID, portal.Name, oldX, oldY, p.X, p.Y, portal.Price)
}

// onPKMode keeps the state requested by 0x399. The server stays
// authoritative: only 0/1 is accepted. The 7.48 client does not dispatch a
// PKInfo 0x166 reply; the observable confirmation uses MessagePanel 0x101 only.
func (w *World) onPKMode(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != 16 {
		return
	}
	value := binary.LittleEndian.Uint32(pkt[12:16])
	if value > 1 {
		w.recordSecurityViolation(s, wire.OpPKMode, "PK state outside the 0/1 contract")
		return
	}
	enabled := value == 1
	p.PKMode = enabled
	if enabled {
		w.cancelTrade(p, "PK mode enabled")
	}
	message := "PK mode disabled."
	if enabled {
		message = "PK mode enabled."
	}
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
	if _, err := w.requireGuildLeader(p.Char); err != nil {
		fail(err.Error())
		return
	}
	// Native confirmation carries no amount. Never reinterpret its option as a
	// bid or silently spend the guild's balance. The command supplies the total.
	s.Send(wire.MessagePanel(fmt.Sprintf("Bid with /citywar %d <total guild fame>. No gold or entry fee.", city+1)))
}
