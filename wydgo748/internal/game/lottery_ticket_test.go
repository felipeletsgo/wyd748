package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func TestLotteryTicketAnswersUnavailableWithoutChangingTheCharacter(t *testing.T) {
	w, p := newServerWarLetterWorld(t)
	p.Char.Gold = 9_000_000
	packet := inboundPacket(wire.OpBuyLotteryTicket, lotteryTicketPacketSize)
	binary.LittleEndian.PutUint16(packet[6:8], p.ID)
	copy(packet[24:30], []byte{1, 7, 13, 22, 31, 45})
	if !w.validateInboundCommand(p.Session, packet) {
		t.Fatal("native 32-byte 0x3B8 rejected at ingress")
	}
	before := *p.Char
	w.handle(command{s: p.Session, pkt: packet})
	if p.Char.Gold != before.Gold || p.Char.Inv != before.Inv {
		t.Fatal("lottery request changed gold or inventory")
	}
	reply, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(reply) || wire.ParseHeader(reply).Type != wire.OpMessagePanel {
		t.Fatal("lottery request did not get the unavailable message")
	}
}

func TestLotteryTicketRejectsForgedRequests(t *testing.T) {
	w, p := newServerWarLetterWorld(t)
	for _, size := range []int{24, 31, 33} {
		if w.validateInboundCommand(p.Session, inboundPacket(wire.OpBuyLotteryTicket, size)) {
			t.Fatalf("0x3B8 with %d bytes was accepted", size)
		}
	}
	w, p = newServerWarLetterWorld(t)
	forged := inboundPacket(wire.OpBuyLotteryTicket, lotteryTicketPacketSize)
	binary.LittleEndian.PutUint16(forged[6:8], p.ID+1)
	w.onLotteryTicketUnavailable(p.Session, forged)
	if p.Session.QueuedPacketsForTest() != 0 || violationsFor(w, p) != 1 {
		t.Fatal("0x3B8 with another session id was answered or not recorded")
	}
}
