package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const guildCityAreaMessage = "Only members of the guild that owns this city may enter this area."

// MODERNIZACAO_COMPATIVEL: use the native movement predicate (0x51D302),
// authoritative destinations and canonical membership. No wire or asset changes.
func (w *World) guildCityAreaAllowed(ch *model.Char, x, y uint16) bool {
	if !w.terrain.GuildArea(x, y) {
		return true
	}
	city := -1
	for i, zone := range homeCityZones {
		if zone.area.contains(x, y) {
			city = i
			break
		}
	}
	if city < 0 {
		return true // The native village lookup returns 5 outside these cities.
	}
	if ch == nil || ch.Score == nil {
		return false
	}
	// Level is zero-based: displayed level 400 remains subject to this check.
	if ch.Score.Level >= 400 {
		return true
	}
	if w.guilds == nil {
		return false // Missing ownership data is not an unowned city.
	}
	owner := w.guilds.Wars.Castle.Owner
	if city < len(w.guilds.Wars.Cities.Territories) {
		owner = w.guilds.Wars.Cities.Territories[city].Owner
	}
	guild, member := w.guildOf(ch)
	if guild == nil || member == nil {
		return owner == 0 // Preserve the native zero-guild/zero-owner comparison.
	}
	return guild.ID == owner
}

func (w *World) guildCityAreaStepAllowed(p *Player, x, y uint16) bool {
	return p != nil && (w.gameplaySpaceForPlayer(p) != "" || w.guildCityAreaAllowed(p.Char, x, y))
}

func (w *World) recallFromGuildCityArea(p *Player) {
	if w.recallPlayer(p, "guild city area access") && p.Session != nil {
		p.Session.Send(wire.MessagePanel(w.trFor(p, guildCityAreaMessage)))
	}
}

// Reconcile ownership or membership changes after pending writes settle.
// A mutable access decision must never invalidate an already committed write.
func (w *World) tickGuildCityAreas(now time.Time) {
	_ = now
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Session == nil ||
			w.gameplaySpaceForPlayer(p) != "" || w.criticalPlayerReserved(p) ||
			w.guildCityAreaAllowed(p.Char, p.X, p.Y) {
			continue
		}
		w.recallFromGuildCityArea(p)
	}
}
