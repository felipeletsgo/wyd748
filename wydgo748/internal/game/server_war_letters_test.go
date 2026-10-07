package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func serverWarLetterPacket(opcode, id uint16, channel int32) []byte {
	packet := inboundPacket(opcode, serverWarLetterPacketSize)
	binary.LittleEndian.PutUint16(packet[6:8], id)
	binary.LittleEndian.PutUint32(packet[12:16], uint32(channel))
	return packet
}

func newServerWarLetterWorld(t *testing.T) (*World, *Player) {
	t.Helper()
	w, p, _ := newEnterWorldPlayer(t, 100, 100)
	p.Char = &p.Account.Chars[0]
	p.CharSlot = 0
	p.InWorld = true
	p.ID = 1000
	return w, p
}

func violationsFor(w *World, p *Player) int {
	if state := w.security[p.Session]; state != nil {
		return state.violations
	}
	return 0
}

func TestServerWarLettersAnswerUnavailableAndKeepTheLetter(t *testing.T) {
	for opcode, letter := range map[uint16]uint16{
		wire.OpDeclareServerWar: declarationOfWarLetterIndex,
		wire.OpRefuseServerWar:  warRejectionLetterIndex,
	} {
		w, p := newServerWarLetterWorld(t)
		p.Char.Inv[5].Index = letter
		packet := serverWarLetterPacket(opcode, p.ID, 2)
		if !w.validateInboundCommand(p.Session, packet) {
			t.Fatalf("0x%X: native 16-byte request rejected at ingress", opcode)
		}
		w.handle(command{s: p.Session, pkt: packet})
		if p.Char.Inv[5].Index != letter {
			t.Fatalf("0x%X consumed letter %d", opcode, letter)
		}
		if got := p.Session.QueuedPacketsForTest(); got != 1 {
			t.Fatalf("0x%X: want one reply, got %d", opcode, got)
		}
		reply, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(reply) || wire.ParseHeader(reply).Type != wire.OpMessagePanel {
			t.Fatalf("0x%X: reply is not a message panel", opcode)
		}
		if violationsFor(w, p) != 0 {
			t.Fatalf("0x%X: valid request recorded a violation", opcode)
		}
	}
}

func TestServerWarLettersRejectForgedRequests(t *testing.T) {
	cases := []struct {
		name    string
		letter  uint16
		id      uint16
		channel int32
	}{
		{"no letter", 0, 1000, 2},
		{"wrong letter", warRejectionLetterIndex, 1000, 2},
		{"other session id", declarationOfWarLetterIndex, 1001, 2},
		{"zero channel", declarationOfWarLetterIndex, 1000, 0},
		{"negative channel", declarationOfWarLetterIndex, 1000, -1},
	}
	for _, tc := range cases {
		w, p := newServerWarLetterWorld(t)
		p.Char.Inv[0].Index = tc.letter
		w.onServerWarLetterUnavailable(p.Session, serverWarLetterPacket(wire.OpDeclareServerWar, tc.id, tc.channel))
		if p.Session.QueuedPacketsForTest() != 0 || violationsFor(w, p) != 1 {
			t.Fatalf("%s: replies=%d violations=%d; want 0 replies and 1 violation",
				tc.name, p.Session.QueuedPacketsForTest(), violationsFor(w, p))
		}
	}
	w, p := newServerWarLetterWorld(t)
	for _, size := range []int{15, 17} {
		if w.validateInboundCommand(p.Session, inboundPacket(wire.OpDeclareServerWar, size)) {
			t.Fatalf("0xED7 with %d bytes was accepted", size)
		}
	}
	p.InWorld = false
	if w.validateInboundCommand(p.Session, serverWarLetterPacket(wire.OpRefuseServerWar, p.ID, 2)) {
		t.Fatal("0xED8 accepted outside the world phase")
	}
}
