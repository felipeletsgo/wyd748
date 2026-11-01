package net

import (
	"bytes"
	"io"
	"testing"
	"time"

	"wydgo/internal/wire"
)

// W3/D3: the writer joins packets already queued into fewer socket writes,
// keeps their order and frames, and bounds retained bytes per session.
func TestWriterCoalescesQueuedPacketsInOrder(t *testing.T) {
	s, peer := tcpSession(t)
	defer peer.Close()
	const packets = 50
	s.out = make(chan outboundPacket, packets)
	var want [][]byte
	for i := range packets {
		pkt := wire.Build(wire.OpPing, uint16(i), wire.HeaderSize)
		s.Send(pkt)
		want = append(want, pkt) // Send finalizes in place; this is the wire form.
	}
	before := metricOutputWrites.Value()
	go s.writeLoop()
	defer s.Close()
	_ = peer.SetReadDeadline(time.Now().Add(2 * time.Second))
	got := make([]byte, packets*wire.HeaderSize)
	if _, err := io.ReadFull(peer, got); err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(got, bytes.Join(want, nil)) {
		t.Fatal("coalesced writes changed packet order or frames")
	}
	deadline := time.Now().Add(time.Second)
	for s.sentPackets.Load() != packets && time.Now().Before(deadline) {
		time.Sleep(time.Millisecond)
	}
	if writes := metricOutputWrites.Value() - before; writes < 1 || writes >= packets || s.queuedBytes.Load() != 0 {
		t.Fatalf("writes=%d for %d queued packets, retained bytes=%d", writes, packets, s.queuedBytes.Load())
	}
}

func TestSendBoundsRetainedBytes(t *testing.T) {
	s := &Session{ID: 1, out: make(chan outboundPacket, 64), done: make(chan struct{}), maxQueuedBytes: 3 * wire.HeaderSize}
	for range 3 {
		s.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	}
	if s.IsClosed() || s.QueueFree() != 61 || s.QueueCapacity() != 64 {
		t.Fatal("packets within the byte bound closed the session")
	}
	s.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	if !s.IsClosed() || s.CloseCause() != CloseCauseOutputOverflow || s.queuedBytes.Load() != 3*wire.HeaderSize {
		t.Fatalf("byte bound: closed=%t cause=%s retained=%d", s.IsClosed(), s.CloseCause(), s.queuedBytes.Load())
	}
	if _, ok := s.DequeuePacketForTest(); !ok || s.queuedBytes.Load() != 2*wire.HeaderSize {
		t.Fatal("dequeue did not release retained bytes")
	}
}
