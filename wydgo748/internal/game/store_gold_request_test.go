package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func TestStoreGoldRequestAnswersUnavailableWithoutMovingGold(t *testing.T) {
	w, p := newServerWarLetterWorld(t)
	p.Char.Gold = 5000
	packet := inboundPacket(wire.OpStoreGoldRequest, storeGoldRequestPacketSize)
	binary.LittleEndian.PutUint16(packet[6:8], p.ID)
	binary.LittleEndian.PutUint32(packet[12:16], 1000)
	if !w.validateInboundCommand(p.Session, packet) {
		t.Fatal("native 16-byte 0x3D1 rejected at ingress")
	}
	w.handle(command{s: p.Session, pkt: packet})
	if p.Char.Gold != 5000 {
		t.Fatalf("gold changed to %d", p.Char.Gold)
	}
	reply, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(reply) || wire.ParseHeader(reply).Type != wire.OpMessagePanel {
		t.Fatal("0x3D1 did not get the unavailable message")
	}

	w, p = newServerWarLetterWorld(t)
	forged := inboundPacket(wire.OpStoreGoldRequest, storeGoldRequestPacketSize)
	binary.LittleEndian.PutUint16(forged[6:8], p.ID+1)
	w.onStoreGoldRequestUnavailable(p.Session, forged)
	if p.Session.QueuedPacketsForTest() != 0 || violationsFor(w, p) != 1 {
		t.Fatal("0x3D1 with another session id was answered or not recorded")
	}
}
