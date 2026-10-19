package game

import (
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// guildDisableCommand recognizes only the native 0x333 bare tokens. The
// comparison is deliberately case-sensitive and does not accept slash or
// argument variants.
func guildDisableCommand(message string) (disabled bool, ok bool) {
	switch message {
	case "guildon":
		return false, true
	case "guildoff":
		return true, true
	default:
		return false, false
	}
}

// handleGuildDisableCommand consumes the exact native tokens before ordinary
// local-chat routing. The Player is resolved from the authenticated session
// map; the packet header ID is never used as the actor identity.
func (w *World) handleGuildDisableCommand(s *net.Session, p *Player, message string) bool {
	disabled, ok := guildDisableCommand(message)
	if !ok {
		return false
	}
	if s == nil || p == nil || p.Session != s || p.Char == nil || !p.InWorld {
		return true
	}
	if p.GuildDisable == disabled {
		return true
	}

	p.GuildDisable = disabled
	w.syncPlayerScoreAndVitals(p)
	var parm uint32
	if disabled {
		parm = 1
	}
	p.Session.Send(wire.StandardParm(wire.OpGuildDisable, p.ID, parm))
	return true
}
