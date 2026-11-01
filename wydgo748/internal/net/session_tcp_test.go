package net

import (
	"bufio"
	"encoding/binary"
	"io"
	stdnet "net"
	"testing"
	"time"

	"wydgo/internal/wire"
)

// Real loopback TCP (W3/D3 and D4 acceptance). The producer follows the same
// rule as scene publication: it sends only while the output queue has room.

func tcpTestSession(t *testing.T, queue int, writeTimeout time.Duration, readBuffer int) (*Session, stdnet.Conn) {
	t.Helper()
	ln, err := stdnet.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer ln.Close()
	accepted := make(chan stdnet.Conn, 1)
	go func() {
		c, err := ln.Accept()
		if err == nil {
			accepted <- c
		}
	}()
	client, err := stdnet.Dial("tcp", ln.Addr().String())
	if err != nil {
		t.Fatal(err)
	}
	if readBuffer > 0 {
		_ = client.(*stdnet.TCPConn).SetReadBuffer(readBuffer)
	}
	var server stdnet.Conn
	select {
	case server = <-accepted:
	case <-time.After(2 * time.Second):
		t.Fatal("accept timed out")
	}
	if readBuffer > 0 {
		_ = server.(*stdnet.TCPConn).SetWriteBuffer(readBuffer)
	}
	s := &Session{ID: 1, conn: server, out: make(chan outboundPacket, queue), done: make(chan struct{}),
		frameWriteTimeout: writeTimeout}
	go s.Serve(func(*Session, []byte) {})
	writeInitCode(t, client)
	t.Cleanup(func() { _ = client.Close(); s.Close() })
	return s, client
}

func sequencedPacket(seq uint32) []byte {
	pkt := wire.Build(wire.OpPing, 0, wire.HeaderSize+120)
	binary.LittleEndian.PutUint32(pkt[wire.HeaderSize:], seq)
	return pkt
}

func TestTCPSlowReaderReceivesOrderedBurstWithoutDisconnect(t *testing.T) {
	const total = 1500
	s, client := tcpTestSession(t, 256, 2*time.Second, 8192)
	received := make(chan error, 1)
	go func() {
		r := bufio.NewReaderSize(client, 1024)
		for want := uint32(0); want < total; want++ {
			var header [wire.HeaderSize]byte
			if _, err := io.ReadFull(r, header[:]); err != nil {
				received <- err
				return
			}
			pkt := make([]byte, binary.LittleEndian.Uint16(header[0:2]))
			copy(pkt, header[:])
			if _, err := io.ReadFull(r, pkt[wire.HeaderSize:]); err != nil {
				received <- err
				return
			}
			if !wire.Decrypt(pkt) || binary.LittleEndian.Uint32(pkt[wire.HeaderSize:]) != want {
				received <- io.ErrUnexpectedEOF
				return
			}
			if want%50 == 0 {
				time.Sleep(5 * time.Millisecond) // a slow receiver
			}
		}
		received <- nil
	}()
	deadline := time.Now().Add(10 * time.Second)
	for seq := uint32(0); seq < total; {
		if s.IsClosed() || time.Now().After(deadline) {
			t.Fatalf("session closed or stalled at %d: cause=%s", seq, s.CloseCause())
		}
		if s.QueueFree() < 8 {
			time.Sleep(time.Millisecond) // the next tick publishes the rest
			continue
		}
		s.Send(sequencedPacket(seq))
		seq++
	}
	select {
	case err := <-received:
		if err != nil {
			t.Fatalf("receiver: %v", err)
		}
	case <-time.After(10 * time.Second):
		t.Fatal("receiver did not finish")
	}
	if s.IsClosed() || s.sentPackets.Load() != total {
		t.Fatalf("closed=%t cause=%s sent=%d", s.IsClosed(), s.CloseCause(), s.sentPackets.Load())
	}
}

func TestTCPBlackholedReaderClosesWithWriteTimeout(t *testing.T) {
	s, _ := tcpTestSession(t, 256, 300*time.Millisecond, 4096)
	start := time.Now()
	for seq := uint32(0); !s.IsClosed() && time.Since(start) < 10*time.Second; seq++ {
		if s.QueueFree() < 8 {
			time.Sleep(time.Millisecond)
			continue
		}
		s.Send(sequencedPacket(seq))
	}
	select {
	case <-s.Done():
	case <-time.After(10 * time.Second):
		t.Fatal("a reader that never reads kept the session open")
	}
	if cause := s.CloseCause(); cause != CloseCauseWriteTimeout {
		t.Fatalf("close cause %s, want write timeout", cause)
	}
}
