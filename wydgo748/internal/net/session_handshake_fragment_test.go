package net

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestServeAcceptsSplitInitCodeBeforeFirstPacket(t *testing.T) {
	session, client := pipeSession()
	defer client.Close()
	received := make(chan []byte, 1)
	go session.Serve(func(_ *Session, packet []byte) {
		if packet != nil {
			received <- packet
		}
	})

	var init [4]byte
	binary.LittleEndian.PutUint32(init[:], wire.InitCode)
	if _, err := client.Write(init[:2]); err != nil {
		t.Fatalf("write first handshake fragment: %v", err)
	}
	tail := append(init[2:4], encryptedTestPacket(wire.OpPing)...)
	if _, err := client.Write(tail); err != nil {
		t.Fatalf("write handshake tail and first frame: %v", err)
	}

	select {
	case packet := <-received:
		if wire.ParseHeader(packet).Type != wire.OpPing {
			t.Fatalf("first packet opcode = %#x, want %#x", wire.ParseHeader(packet).Type, wire.OpPing)
		}
	case <-time.After(time.Second):
		t.Fatal("split handshake did not deliver the first packet")
	}
}
