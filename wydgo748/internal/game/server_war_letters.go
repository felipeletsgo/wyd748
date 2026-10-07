package game

import (
	"encoding/binary"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The native 7.48 client (FUN_004662c5) sends 0xED7 after the Declaration of
// War letter (4030) prompt and 0xED8 after the War Rejection letter (4031)
// prompt: a 16-byte MSG_STANDARDPARM whose Parm@12 is the typed target
// channel. Inter-channel war needs authoritative coordination between server
// instances, which WYD-Go does not have. The server therefore accepts the
// native request, validates it, and answers that the feature is unavailable;
// the letter is not consumed and no war state is created.
const (
	serverWarLetterPacketSize       = 16
	declarationOfWarLetterIndex     = 4030
	warRejectionLetterIndex         = 4031
	serverWarUnavailableMessage     = "Server war is not available on this server."
	serverWarLetterTargetParmOffset = 12
)

func serverWarLetterFor(opcode uint16) uint16 {
	switch opcode {
	case wire.OpDeclareServerWar:
		return declarationOfWarLetterIndex
	case wire.OpRefuseServerWar:
		return warRejectionLetterIndex
	default:
		return 0
	}
}

func (w *World) onServerWarLetterUnavailable(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != serverWarLetterPacketSize {
		return
	}
	header := wire.ParseHeader(pkt)
	if header.ID != p.ID {
		w.recordSecurityViolation(s, header.Type, "session id mismatch")
		return
	}
	channel := int32(binary.LittleEndian.Uint32(pkt[serverWarLetterTargetParmOffset:]))
	if channel <= 0 {
		w.recordSecurityViolation(s, header.Type, "invalid target channel")
		return
	}
	letter := serverWarLetterFor(header.Type)
	if letter == 0 || !carriesItem(p.Char, letter) {
		w.recordSecurityViolation(s, header.Type, "war letter not in inventory")
		return
	}
	s.Send(wire.MessagePanel(serverWarUnavailableMessage))
}

func carriesItem(ch *model.Char, index uint16) bool {
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index == index {
			return true
		}
	}
	return false
}
