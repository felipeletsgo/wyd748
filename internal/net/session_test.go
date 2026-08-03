package net

import (
	"encoding/binary"
	stdnet "net"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func pipeSession() (*Session, stdnet.Conn) {
	server, client := stdnet.Pipe()
	return &Session{
		ID: 1, conn: server, out: make(chan []byte, 8), done: make(chan struct{}),
	}, client
}

func writeInitCode(t *testing.T, conn stdnet.Conn) {
	t.Helper()
	var init [4]byte
	binary.LittleEndian.PutUint32(init[:], wire.InitCode)
	if _, err := conn.Write(init[:]); err != nil {
		t.Fatalf("escrever InitCode: %v", err)
	}
}

func encryptedTestPacket(opcode uint16) []byte {
	packet := wire.Build(opcode, 0, wire.HeaderSize)
	wire.FinishPacket(packet, 3)
	return packet
}

func TestServeAcceptsValidHandshakeAndPacket(t *testing.T) {
	session, client := pipeSession()
	received := make(chan []byte, 2)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })

	writeInitCode(t, client)
	if _, err := client.Write(encryptedTestPacket(wire.OpPing)); err != nil {
		t.Fatalf("escrever pacote: %v", err)
	}
	_ = client.Close()

	select {
	case packet := <-received:
		if packet == nil || wire.ParseHeader(packet).Type != wire.OpPing {
			t.Fatalf("pacote recebido=%v", packet)
		}
	case <-time.After(time.Second):
		t.Fatal("pacote valido nao chegou ao handler")
	}
}

func TestServeRejectsInvalidInitCode(t *testing.T) {
	session, client := pipeSession()
	received := make(chan []byte, 1)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })

	if _, err := client.Write([]byte{1, 2, 3, 4}); err != nil {
		t.Fatalf("escrever InitCode: %v", err)
	}
	_ = client.Close()
	select {
	case packet := <-received:
		if packet != nil {
			t.Fatal("handshake invalido entregou pacote ao game")
		}
	case <-time.After(time.Second):
		t.Fatal("sessao nao encerrou apos InitCode invalido")
	}
}

func TestServeRejectsBadChecksumBeforeHandler(t *testing.T) {
	session, client := pipeSession()
	received := make(chan []byte, 1)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })

	writeInitCode(t, client)
	packet := encryptedTestPacket(wire.OpPing)
	packet[3] ^= 0x01
	if _, err := client.Write(packet); err != nil {
		t.Fatalf("escrever pacote: %v", err)
	}
	_ = client.Close()
	select {
	case delivered := <-received:
		if delivered != nil {
			t.Fatal("checksum invalido chegou ao handler")
		}
	case <-time.After(time.Second):
		t.Fatal("sessao nao encerrou apos checksum invalido")
	}
}

func TestSendOverflowDoesNotPanicWithoutSocket(t *testing.T) {
	session := NewTestSession(1, 1)
	session.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	session.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	if session.QueuedPacketsForTest() != 1 {
		t.Fatal("overflow alterou a fila ja enfileirada")
	}
}

func TestSessionSendRejectsMalformedBuilderWithoutPanic(t *testing.T) {
	session := NewTestSession(99, 1)
	session.Send([]byte{1, 2, 3})
	if !session.IsClosed() {
		t.Fatal("sessao deveria fechar ao receber pacote de saida truncado")
	}
	if got := session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("pacote truncado entrou na fila: %d", got)
	}
}

func TestServeRejectsOversizedFrameBeforeBodyAllocation(t *testing.T) {
	session, client := pipeSession()
	received := make(chan []byte, 1)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })

	writeInitCode(t, client)
	var size [2]byte
	binary.LittleEndian.PutUint16(size[:], wire.MaxPacketSize+1)
	_, _ = client.Write(size[:])
	_ = client.Close()
	select {
	case packet := <-received:
		if packet != nil {
			t.Fatal("frame oversized chegou ao handler")
		}
	case <-time.After(time.Second):
		t.Fatal("sessao nao encerrou apos frame oversized")
	}
}

func TestSessionDoneAndIsClosedFollowServeLifetime(t *testing.T) {
	session, client := pipeSession()
	finished := make(chan []byte, 1)
	go func() {
		session.Serve(func(_ *Session, packet []byte) {
			finished <- packet
		})
	}()
	writeInitCode(t, client)
	_ = client.Close()
	select {
	case packet := <-finished:
		if packet != nil {
			t.Fatal("pacote inesperado")
		}
	case <-time.After(time.Second):
		t.Fatal("handler de desconexao nao foi chamado")
	}
	select {
	case <-session.done:
		if !session.IsClosed() {
			t.Fatal("Done fechou mas IsClosed retornou falso")
		}
	case <-time.After(time.Second):
		t.Fatal("Done nao fechou com o fim da sessao")
	}
}
