package game

import (
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

// playerScorePacket is the only private score publication path for a live
// player. Keeping the ABI choice next to the recipient prevents a source
// client from receiving a stock 28-byte STRUCT_SCORE after a later feature.
func playerScorePacket(p *Player) []byte {
	if p == nil || p.Session == nil || p.Char == nil {
		return nil
	}
	return wire.UpdateScore(p.ID, *p.Char)
}

// playerAffectsPacket follows the same recipient-owned ABI rule as score.
func playerAffectsPacket(p *Player) []byte {
	if p == nil || p.Session == nil || p.Char == nil {
		return nil
	}
	return wire.UpdateAffects(p.ID, *p.Char)
}

// observedPlayerScorePacket serializes one subject for a specific observer;
// every observer receives the same canonical score ABI.
func observedPlayerScorePacket(observer, subject *Player) []byte {
	if observer == nil || observer.Session == nil || subject == nil || subject.Char == nil {
		return nil
	}
	return wire.UpdateScore(subject.ID, *subject.Char)
}

// selectionUpdatePacket rebuilds the four-character selection aggregate after
// create/delete/evolution using the ABI negotiated by the login packet.
func selectionUpdatePacket(s *gameNet.Session, opcode, id uint16, p *Player) []byte {
	if s == nil || p == nil || p.Account == nil {
		return nil
	}
	return wire.CharacterSelectionUpdate(opcode, id, p.Account.Chars)
}

// characterListPacket is used when a feature returns to character selection;
// login and re-entry must advertise an identical aggregate layout.
func characterListPacket(s *gameNet.Session, p *Player) []byte {
	if s == nil || p == nil || p.Account == nil {
		return nil
	}
	return wire.CharList(p.Account.Name, p.Account.Chars, p.Account.Cargo[:], p.Account.CargoGold)
}
