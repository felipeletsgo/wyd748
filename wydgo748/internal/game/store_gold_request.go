package game

import (
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The native 7.48 client opens gold prompt mode 7 ("Enter the amount of gold
// to store.", strdef row 136) for an NPC with merchant type 9 and head 51, and
// sends 0x3D1 with the amount in Parm@12 (16 bytes, FUN_0045ee28 and
// FUN_004662c5). No source records what the server does with that gold, and
// no WYD-Go NPC has merchant type 9, so the request is recognized and answered
// as unavailable. No gold moves.
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
	s.Send(wire.MessagePanel(storeGoldUnavailableMessage))
}
