package game

import (
	"encoding/binary"
	"reflect"
	"strings"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCharacterTransferUnavailablePreservesAccountAndRepliesWithNativeError(t *testing.T) {
	w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe", Gold: 1234})
	before := append([]model.Char(nil), p.Account.Chars...)
	request := inboundPacket(wire.OpCharacterTransfer, 52)
	binary.LittleEndian.PutUint32(request[16:20], 0)
	copy(request[20:36], "Felipe")
	copy(request[36:52], "Felipe")
	w.handle(command{s: session, pkt: request})
	if !reflect.DeepEqual(p.Account.Chars, before) || p.InWorld || p.CharSlot != -1 {
		t.Fatal("transfer rejection changed the character or session")
	}
	if session.QueuedPacketsForTest() != 2 {
		t.Fatalf("expected the transfer reply and a notice, got %d packets", session.QueuedPacketsForTest())
	}
	response, ok := session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(response) || len(response) != 52 ||
		wire.ParseHeader(response).Type != wire.OpCharacterTransfer ||
		binary.LittleEndian.Uint32(response[12:16]) != 4 ||
		binary.LittleEndian.Uint32(response[16:20]) != 0 {
		t.Fatal("0xFAA reply does not match the client generic error")
	}
	notice, ok := session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(notice) || wire.ParseHeader(notice).Type != wire.OpMessagePanel ||
		!strings.Contains(string(notice[12:]), "Character transfer is not available") {
		t.Fatal("the unavailable notice must follow the 0xFAA reply")
	}
}

func TestCharacterTransferUnavailableRejectsMalformedAndWorldReplay(t *testing.T) {
	w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe"})
	for _, size := range []int{51, 53} {
		if w.validateInboundCommand(session, inboundPacket(wire.OpCharacterTransfer, size)) {
			t.Fatalf("0xFAA with %d bytes was accepted", size)
		}
	}
	for _, slot := range []uint32{4, ^uint32(0)} {
		badSlot := inboundPacket(wire.OpCharacterTransfer, 52)
		binary.LittleEndian.PutUint32(badSlot[16:20], slot)
		w.onCharacterTransferUnavailable(session, badSlot)
		if session.QueuedPacketsForTest() != 0 {
			t.Fatalf("invalid slot %d got a reply", slot)
		}
	}
	badResult := inboundPacket(wire.OpCharacterTransfer, 52)
	binary.LittleEndian.PutUint32(badResult[12:16], 1)
	w.onCharacterTransferUnavailable(session, badResult)
	if session.QueuedPacketsForTest() != 0 {
		t.Fatal("packet with a reply Result was accepted as a request")
	}
	valid := inboundPacket(wire.OpCharacterTransfer, 52)
	p.InWorld = true
	if w.validateInboundCommand(session, valid) {
		t.Fatal("transfer repeated in the world was accepted")
	}
	if session.QueuedPacketsForTest() != 0 || p.Account.Chars[0].Name != "Felipe" {
		t.Fatal("refused request changed the queue or account")
	}
}
