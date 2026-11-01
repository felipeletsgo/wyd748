package game

import (
	"wydgo/internal/wire"
)

// receiveDeathLetter sends the death letter before the client confirms the death.
func (w *World) receiveDeathLetter(victim *Player, killerName, killerKind string) {
	if victim == nil || victim.Session == nil || killerName == "" {
		return
	}
	kind := w.msg("_WG_Killer_Mob")
	if killerKind == "player" {
		kind = w.msg("_WG_Killer_Player")
	}
	victim.Session.Send(wire.MessageWhisper(0, killerName, w.msg("_WG_Killed_By", kind, killerName), 0))
}
