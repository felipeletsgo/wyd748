package game

import (
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The native 7.48 client opens gold prompt mode 7 ("Enter the amount of gold
// to store.", strdef row 136) for an NPC with merchant type 9 and head 51, and
// sends 0x3D1 with the amount in Parm@12 (16 bytes, FUN_0045ee28 and
// FUN_004662c5). Rune Track +6 uses this existing boundary for its historical
// donation service. All unrelated requests remain unavailable and move no gold.
const (
	storeGoldRequestPacketSize  = 16
	storeGoldUnavailableMessage = "That service is not available on this server."
)

func (w *World) onStoreGoldRequestUnavailable(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != storeGoldRequestPacketSize {
		return
	}
	if header := wire.ParseHeader(pkt); header.ID != p.ID {
		w.recordSecurityViolation(s, header.Type, "session id mismatch")
		return
	}
	if w.donateRuneTrackGold(p, pkt) {
		return
	}
	s.Send(wire.MessagePanel(storeGoldUnavailableMessage))
}
