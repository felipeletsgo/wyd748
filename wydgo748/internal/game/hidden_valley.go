package game

import (
	"time"

	"wydgo/internal/wire"
)

const hiddenValleyAccessCounter = accessCounterPrefix + "hidden_ruins"

// This server-owned access boundary encloses the installed terrain's walkable
// component reached from (2281,3688). It is not a native fairy/equipment rule.
func inHiddenValley(x, y uint16) bool {
	return x >= 2191 && x <= 2292 && y >= 3600 && y <= 3695
}

func hiddenValleyAccessActive(p *Player, now time.Time) bool {
	if p == nil {
		return false
	}
	expires := p.SpecialCoins[hiddenValleyAccessCounter]
	return expires != 0 && now.Before(time.Unix(int64(expires), 0))
}

func hiddenValleyDestinationAllowed(p *Player, x, y uint16, now time.Time) bool {
	return !inHiddenValley(x, y) || hiddenValleyAccessActive(p, now)
}

// Re-check public-map stays on the existing per-second sweep. Never turn a
// deadline crossed during a durable write into an identity/commit failure:
// wait for settlement, then recall through the normal persistence boundary.
func (w *World) tickHiddenValleyAccess(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Session == nil ||
			w.gameplaySpaceForPlayer(p) != "" || w.criticalPlayerReserved(p) ||
			!inHiddenValley(p.X, p.Y) || hiddenValleyAccessActive(p, now) {
			continue
		}
		if w.recallPlayer(p, "Hidden Ruins access expired") {
			p.Session.Send(wire.MessagePanel("Hidden Ruins access expired. Returning to your home city."))
		}
	}
}
