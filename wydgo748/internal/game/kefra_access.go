package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const kefraEntriesRequired = "You have no Kefra entries left. Bring a Locked Seal to Survivor."

// The hall entrance is distinct from the boss-controlled passage to Uxmal.
// Match the complete native 4x4 portal block, not only its configured tile.
func isKefraHallEntrance(portal model.Teleport) bool {
	return samePortalAttributeBlock(portal.SourceX, portal.SourceY, 2366, 3893)
}

// Survivor alone exchanges Locked Seals. Stepping on the hall portal spends an
// existing entry; carrying a seal is not permission to bypass that exchange.
func (w *World) enterKefraHall(p *Player, portal model.Teleport) {
	fail := func() { p.Session.Send(wire.MessagePanel("The teleport could not be completed.")) }
	if p.Account == nil || p.Char.UID == "" || p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) {
		fail()
		return
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	entry := &Player{SpecialCoins: copyCounters(p)}
	if !spendCounters(entry, map[string]uint32{kefraTicketCounter: 1}) {
		p.Session.Send(wire.MessagePanel(kefraEntriesRequired))
		return
	}
	if ch.Gold < portal.Price {
		p.Session.Send(wire.MessagePanel("Not enough gold for the teleport."))
		return
	}
	ch.Gold -= portal.Price
	oldX, oldY := p.X, p.Y
	publish := func() {
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		if portal.Name != "" {
			p.Session.Send(wire.MessagePanel(portal.Name))
		}
		log.Printf("[#%d] TELEPORT %q @(%d,%d)->(%d,%d) gold=-%d Kefra entries=%d",
			p.Session.ID, portal.Name, oldX, oldY, p.X, p.Y, portal.Price, counterBalance(p, kefraTicketCounter))
	}
	if w.criticalWrites != nil {
		if err := w.queuePlayerTeleportWithCounters(p, proposed, entry.SpecialCoins,
			portal.DestX, portal.DestY, "Kefra hall entry", publish); err != nil {
			log.Printf("[#%d] Kefra hall admission rejected: %v", p.Session.ID, err)
			fail()
		}
		return
	}
	x, y, ok := w.resolvePlayerTeleportDestination(p, portal.DestX, portal.DestY)
	if !ok || w.positionOccupiedExcept(x, y, nil, p) {
		fail()
		return
	}
	if _, atomic := w.store.(playerStateStore); !atomic {
		// Saving only the account would restore the old allowance on relogin.
		fail()
		return
	}
	ch.X, ch.Y = x, y
	stagePlayerHomeCity(&Player{Char: ch, X: x, Y: y})
	state := buildCharState(p, w.now())
	state.SpecialCoins = entry.SpecialCoins
	if err := w.saveAccountAndCharStateForUID(proposed, p.Char.UID, state); err != nil {
		log.Printf("[#%d] save Kefra hall entry: %v", p.Session.ID, err)
		fail()
		return
	}
	w.cancelTrade(p, "teleport")
	p.Char.Gold = ch.Gold
	p.SpecialCoins = entry.SpecialCoins
	p.X, p.Y, p.Char.X, p.Char.Y = x, y, x, y
	if p.Char.Score != nil && ch.Score != nil {
		p.Char.Score.Merchant = ch.Score.Merchant
	}
	if p.Char.RuntimeScore != nil && ch.RuntimeScore != nil {
		p.Char.RuntimeScore.Merchant = ch.RuntimeScore.Merchant
	}
	clearAirMove(p)
	clearPublishedPlayerMove(p)
	w.publishPlayerTeleport(p)
	publish()
}
