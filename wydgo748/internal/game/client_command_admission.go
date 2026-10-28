package game

import (
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The localized 7.48 client writes these server keywords into 0x334.MobName.
// Reserve them before nickname lookup, even while their authoritative gameplay
// operations are unavailable. Historical 7.59 command bodies do not by
// themselves establish a durable 7.48 guild/citizenship implementation.
func (w *World) rejectUnavailableClientCommand(s *net.Session, p *Player, command string) bool {
	message := ""
	switch command {
	case "getout", "waiver", "fimcidadao":
		message = "Citizenship departure is unavailable on this server."
		// Owner policy: every guild rank receives this rejection first.
		if p != nil && p.Char != nil && p.Char.GuildID != 0 {
			message = "Leave your guild before leaving citizenship."
		}
	case "handover", "transferir":
		message = "Guild rank handover is unavailable on this server."
	case "war", "sitguerra":
		message = "The legacy guild war command is unavailable. Use /citywar or /tower for war information."
	case "item_lock", "item_unlock":
		message = "The legacy item lock command is unavailable on this server."
	default:
		return false
	}
	if s != nil {
		s.Send(wire.MessagePanel(message))
	}
	return true
}
