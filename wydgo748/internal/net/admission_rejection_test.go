package net

import (
	"errors"
	"io"
	stdnet "net"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestAdmissionRejectionDeliversMessageAndClosesSocket(t *testing.T) {
	listener, err := stdnet.Listen("tcp4", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	client, err := stdnet.DialTimeout("tcp4", listener.Addr().String(), time.Second)
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()
	server, err := listener.Accept()
	if err != nil {
		t.Fatal(err)
	}
	session := &Session{ID: 1, conn: server, out: make(chan []byte, 8), done: make(chan struct{})}
	defer session.Close()
	go session.writeLoop()
	const message = "Client update required. Close the game and run the launcher to update."
	session.RejectAdmission(wire.MessagePanel(message))
	// Rejection is terminal immediately, not only after the close timer fires.
	if !session.AdmissionRejected() || session.IsAuthenticated() {
		t.Fatal("rejection did not immediately mark the socket as inadmissible")
	}
	session.RejectAdmission(wire.MessagePanel("must not be sent twice"))
	if err := client.SetReadDeadline(time.Now().Add(2 * time.Second)); err != nil {
		t.Fatal(err)
	}
	packet, checksum, err := wire.ReadPacket(client)
	if err != nil || !checksum {
		t.Fatalf("rejection message was not delivered intact: checksum=%v err=%v", checksum, err)
	}
	if wire.ParseHeader(packet).Type != wire.OpMessagePanel ||
		string(packet[12:12+len(message)]) != message {
		t.Fatal("socket received a different rejection message")
	}
	var extra [1]byte
	if n, err := client.Read(extra[:]); n != 0 || !errors.Is(err, io.EOF) {
		t.Fatalf("expected EOF after the single rejection message: n=%d err=%v", n, err)
	}
	select {
	case <-session.done:
	case <-time.After(time.Second):
		t.Fatal("rejected session did not release its lifetime")
	}
}
