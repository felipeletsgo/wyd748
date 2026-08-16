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

func TestConnectionLimiterCoversTotalPerOriginAndRelease(t *testing.T) {
	l := newConnectionLimiter(2, 1)
	if !l.acquire("a") {
		t.Fatal("primeira conexao foi recusada")
	}
	if l.acquire("a") {
		t.Fatal("limite por origem foi ignorado")
	}
	if !l.acquire("b") {
		t.Fatal("segunda origem deveria preencher total")
	}
	if l.acquire("c") {
		t.Fatal("limite total foi ignorado")
	}
	l.release("a")
	if l.total != 1 || l.perOrigin["a"] != 0 {
		t.Fatalf("release final nao removeu origem: total=%d per=%v", l.total, l.perOrigin)
	}
	if !l.acquire("c") {
		t.Fatal("vaga liberada nao foi reutilizada")
	}
	l.maxPerIP = 3
	l.maxTotal = 0
	l.release("c")
	if !l.acquire("b") {
		t.Fatal("segunda conexao da origem b foi recusada com limite 3")
	}
	l.release("b")
	if l.perOrigin["b"] != 1 {
		t.Fatalf("release parcial deveria decrementar origem: %v", l.perOrigin)
	}
	l.release("b")
	l.release("b") // release extra nao pode tornar total negativo.
	if l.total != 0 {
		t.Fatalf("total negativo/inconsistente: %d", l.total)
	}
}

func TestOriginCanonicalizationCoversIPv4IPv6MappedAndInvalid(t *testing.T) {
	if _, ok := ParseOriginIP(" invalid "); ok {
		t.Fatal("IP invalido foi aceito")
	}
	mapped, ok := ParseOriginIP(" ::ffff:192.0.2.9 ")
	if !ok || mapped.String() != "192.0.2.9" {
		t.Fatalf("IPv4-mapped nao foi normalizado: %v/%v", mapped, ok)
	}
	if key, ok := OriginLimitKey("192.0.2.9"); !ok || key != "192.0.2.9" {
		t.Fatalf("IPv4 key=%q/%v", key, ok)
	}
	if key, ok := OriginLimitKey("2001:db8:abcd:1::1234"); !ok || key != "2001:db8:abcd:1::/64" {
		t.Fatalf("IPv6 /64 key=%q/%v", key, ok)
	}
	if key, ok := OriginLimitKey("bad"); ok || key != "" {
		t.Fatalf("origem invalida=%q/%v", key, ok)
	}
}

func TestServeListenerBuildsConfiguredSessionAndStopsWhenListenerCloses(t *testing.T) {
	ln, err := stdnet.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	cfg := ListenerConfig{
		OutputQueueSize: 7, MaxConnections: 4, MaxConnectionsPerIP: 4,
		HandshakeTimeout: 11 * time.Millisecond, SessionIdleTimeout: 12 * time.Millisecond,
		FrameReadTimeout: 13 * time.Millisecond, InboundPacketsPerSec: 14, InboundBytesPerSec: 15,
	}
	gotSession := make(chan *Session, 1)
	serveDone := make(chan error, 1)
	go func() {
		serveDone <- serveListener(ln, cfg, func(s *Session) {
			gotSession <- s
		})
	}()

	client, err := stdnet.Dial("tcp", ln.Addr().String())
	if err != nil {
		_ = ln.Close()
		t.Fatal(err)
	}
	defer client.Close()
	var s *Session
	select {
	case s = <-gotSession:
	case <-time.After(time.Second):
		_ = ln.Close()
		t.Fatal("listener nao entregou Session")
	}
	if s.ID == 0 || s.remoteIP != "127.0.0.1" || cap(s.out) != 7 ||
		s.handshakeTimeout != cfg.HandshakeTimeout || s.idleTimeout != cfg.SessionIdleTimeout ||
		s.frameReadTimeout != cfg.FrameReadTimeout || s.maxInboundPacketsPerSec != 14 || s.maxInboundBytesPerSec != 15 {
		t.Fatalf("Session configurada incorretamente: %+v", s)
	}
	s.Close()
	if err := ln.Close(); err != nil {
		t.Fatal(err)
	}
	select {
	case err := <-serveDone:
		if err != nil {
			t.Fatalf("listener fechado retornou erro: %v", err)
		}
	case <-time.After(time.Second):
		t.Fatal("serveListener nao encerrou com ErrClosed")
	}
}

func TestRemoteIPCoversNilHostPortAndRawAddress(t *testing.T) {
	if remoteIP(nil) != "" {
		t.Fatal("remoteIP(nil) deveria ser vazio")
	}
	if got := remoteIP(testAddr("203.0.113.1:8281")); got != "203.0.113.1" {
		t.Fatalf("remoteIP hostport=%q", got)
	}
	if got := remoteIP(testAddr("raw-address")); got != "raw-address" {
		t.Fatalf("remoteIP raw=%q", got)
	}
}
