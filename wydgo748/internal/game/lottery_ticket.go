package game

import (
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The native 7.48 client does not buy WYD Lottery tickets (items 3200-3299)
// with 0x379. It copies the 24-byte purchase into a 32-byte buffer, retypes it
// as 0x3B8, asks for six numbers (written as bytes at @24-@29), and sends it
// after control 0x838 confirms (FUN_00410a91, FUN_004662c5). No source records
// the draw or the prize, so the server recognizes the request, validates the
// session, and answers that the lottery is unavailable. No gold or item moves.
const (
	lotteryTicketPacketSize   = 32
	lotteryUnavailableMessage = "The WYD Lottery is not available on this server."
)

func (w *World) onLotteryTicketUnavailable(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != lotteryTicketPacketSize {
		return
	}
	if header := wire.ParseHeader(pkt); header.ID != p.ID {
		w.recordSecurityViolation(s, header.Type, "session id mismatch")
		return
	}
	s.Send(wire.MessagePanel(lotteryUnavailableMessage))
}
