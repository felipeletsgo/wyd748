package net

import (
	"bytes"
	stdnet "net"
	"testing"
	"time"

	"wydgo/internal/wire"
)

type remoteAddrConn struct {
	stdnet.Conn
	remote stdnet.Addr
}

func (c remoteAddrConn) RemoteAddr() stdnet.Addr { return c.remote }

func TestSessionRemoteIPFallsBackToObservedSocketAddress(t *testing.T) {
	server, client := stdnet.Pipe()
	defer server.Close()
	defer client.Close()
	s := &Session{conn: remoteAddrConn{Conn: server, remote: testAddr("203.0.113.10:8281")}}
	if got := s.RemoteIP(); got != "203.0.113.10" {
		t.Fatalf("RemoteIP=%q, esperado IP sem porta", got)
	}
	if got := s.RemoteAddr(); got != "203.0.113.10:8281" {
		t.Fatalf("RemoteAddr=%q", got)
	}
	var nilSession *Session
	if nilSession.RemoteIP() != "" || nilSession.RemoteAddr() != "" {
		t.Fatal("sessao nil expos endereco")
	}
}

func TestSessionSendProducesValidEncryptedFrameWithServerTick(t *testing.T) {
	s := NewTestSession(7, 1)
	pkt := wire.Build(wire.OpPing, 7, wire.HeaderSize)
	s.Send(pkt)
	if s.QueuedPacketsForTest() != 1 {
		t.Fatal("pacote valido nao entrou na fila")
	}
	encrypted := <-s.out
	decoded, checksum, err := wire.ReadPacket(bytes.NewReader(encrypted))
	if err != nil || !checksum {
		t.Fatalf("frame de saida nao decodificou: checksum=%v err=%v", checksum, err)
	}
	h := wire.ParseHeader(decoded)
	if h.Type != wire.OpPing || h.Size != wire.HeaderSize || h.Tick < 60000 {
		t.Fatalf("header finalizado incorretamente: %+v", h)
	}
}

func TestSessionWriteLoopDrainsQueueAndStopsOnClose(t *testing.T) {
	server, client := stdnet.Pipe()
	defer client.Close()
	s := &Session{ID: 1, conn: server, out: make(chan []byte, 1), done: make(chan struct{})}
	finished := make(chan struct{})
	go func() {
		s.writeLoop()
		close(finished)
	}()

	payload := []byte{1, 2, 3, 4}
	s.out <- payload
	got := make([]byte, len(payload))
	if _, err := client.Read(got); err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(got, payload) {
		t.Fatalf("writeLoop escreveu %v, esperado %v", got, payload)
	}
	s.Close()
	select {
	case <-finished:
	case <-time.After(time.Second):
		t.Fatal("writeLoop nao encerrou depois de Close")
	}
}

func TestListenWithConfigReturnsBindErrors(t *testing.T) {
	if err := ListenWithConfig("not-a-valid-listen-address", ListenerConfig{}, func(*Session) {}); err == nil {
		t.Fatal("listen address invalido foi aceito")
	}
}

func TestSessionSendRejectsOversizedFrame(t *testing.T) {
	s := NewTestSession(5, 1)
	s.Send(make([]byte, wire.MaxPacketSize+1))
	if !s.IsClosed() || s.QueuedPacketsForTest() != 0 {
		t.Fatalf("frame oversized nao falhou fechado: closed=%v queued=%d", s.IsClosed(), s.QueuedPacketsForTest())
	}
}
