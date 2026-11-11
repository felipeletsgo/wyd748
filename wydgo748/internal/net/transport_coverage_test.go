package net

import (
	"errors"
	"expvar"
	stdnet "net"
	"strings"
	"sync/atomic"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestSessionNilAndClosedHelperPaths(t *testing.T) {
	var nilSession *Session
	if nilSession.IsClosed() || nilSession.Done() != nil || nilSession.QueueFree() != 0 ||
		nilSession.QueueCapacity() != 0 || nilSession.QueuedPacketsForTest() != 0 {
		t.Fatal("nil session helpers returned active state")
	}
	if _, ok := nilSession.DequeuePacketForTest(); ok {
		t.Fatal("nil session returned a queued packet")
	}
	if nilSession.RemoteAddr() != "" || nilSession.CloseCause() != CloseCauseNone {
		t.Fatal("nil session exposed connection or close state")
	}
	nilSession.CloseWithCause(CloseCauseServer, nil)
	nilSession.recordClose(CloseCauseServer, errors.New("ignored"))
	nilSession.closeDone()

	empty := &Session{}
	if empty.IsClosed() || empty.Done() != nil || empty.QueueFree() != 0 || empty.QueueCapacity() != 0 ||
		empty.QueuedPacketsForTest() != 0 {
		t.Fatal("session without channels returned active queue state")
	}
	if _, ok := empty.DequeuePacketForTest(); ok {
		t.Fatal("empty session returned a queued packet")
	}
	queued := NewTestSession(1, 1)
	if _, ok := queued.DequeuePacketForTest(); ok {
		t.Fatal("empty outbound queue returned a packet")
	}
	empty.closeDone()
	empty.CloseWithCause(CloseCauseNone, nil)
	if empty.IsClosed() || empty.CloseCause() != CloseCauseNone {
		t.Fatal("session without a lifetime channel changed close state")
	}

	if got := remoteIP(nil); got != "" {
		t.Fatalf("nil remote address=%q", got)
	}
	if got := remoteIP(testAddr("unparsed-origin")); got != "unparsed-origin" {
		t.Fatalf("unparsed remote address fallback=%q", got)
	}
}

type readDeadlineErrorConn struct {
	stdnet.Conn
	err error
}

func (c readDeadlineErrorConn) SetReadDeadline(time.Time) error { return c.err }

func TestMarkAuthenticatedHandlesClosedSessionsAndDeadlineErrors(t *testing.T) {
	closed := NewTestSession(1, 1)
	closed.Close()
	closed.MarkAuthenticated()
	if closed.IsAuthenticated() {
		t.Fatal("closed session became authenticated")
	}

	server, client := stdnet.Pipe()
	defer client.Close()
	deadlineErr := errors.New("deadline rejected")
	session := &Session{conn: readDeadlineErrorConn{Conn: server, err: deadlineErr}, done: make(chan struct{})}
	session.MarkAuthenticated()
	if !session.IsClosed() || session.CloseCause() != CloseCauseReadError || !strings.Contains(session.closeDetail, deadlineErr.Error()) {
		t.Fatalf("deadline failure did not close the session with a read error: cause=%v detail=%q", session.CloseCause(), session.closeDetail)
	}
}

func TestCloseRecordValidationAndUnspecifiedFallback(t *testing.T) {
	session := &Session{ID: 1, out: make(chan outboundPacket, 1)}
	session.recordClose(CloseCauseNone, errors.New("no cause"))
	if session.CloseCause() != CloseCauseNone {
		t.Fatal("empty close cause was recorded")
	}
	session.recordClose(closeCauseCount, errors.New("invalid cause"))
	if session.CloseCause() != CloseCauseServer {
		t.Fatalf("invalid cause normalized to %v", session.CloseCause())
	}
	session.finishCloseRecord()

	unspecified := &Session{ID: 2, out: make(chan outboundPacket, 1)}
	unspecified.finishCloseRecord()
}

type transientFailureListener struct{ calls atomic.Int32 }

func (l *transientFailureListener) Accept() (stdnet.Conn, error) {
	if l.calls.Add(1) == 1 {
		return nil, errors.New("temporary accept failure")
	}
	return nil, stdnet.ErrClosed
}
func (*transientFailureListener) Close() error      { return nil }
func (*transientFailureListener) Addr() stdnet.Addr { return testAddr("test-listener") }

func TestServeListenerContinuesAfterAcceptFailure(t *testing.T) {
	listener := &transientFailureListener{}
	if err := ServeListenerWithConfig(listener, ListenerConfig{}, func(*Session) {
		t.Fatal("no connection should be accepted")
	}); err != nil {
		t.Fatalf("closed listener returned error: %v", err)
	}
	if got := listener.calls.Load(); got != 2 {
		t.Fatalf("Accept calls=%d, want retry then closed-socket exit", got)
	}
}

func TestServeEnforcesInboundRateWindow(t *testing.T) {
	t.Run("packet limit", func(t *testing.T) {
		assertInboundLimitCloses(t, 1, 1024)
	})
	t.Run("byte limit", func(t *testing.T) {
		assertInboundLimitCloses(t, 10, wire.HeaderSize)
	})
	t.Run("counter resets after one second", func(t *testing.T) {
		session, client := pipeSession()
		session.maxInboundPacketsPerSec = 1
		session.maxInboundBytesPerSec = 1024
		received := make(chan []byte, 4)
		go session.Serve(func(_ *Session, packet []byte) { received <- packet })
		writeInitCode(t, client)
		packet := encryptedTestPacket(wire.OpPing)
		if _, err := client.Write(packet); err != nil {
			t.Fatal(err)
		}
		waitForTransportPacket(t, received, false)
		time.Sleep(1200 * time.Millisecond)
		if _, err := client.Write(packet); err != nil {
			t.Fatal(err)
		}
		waitForTransportPacket(t, received, false)
		_ = client.Close()
		waitForTransportPacket(t, received, true)
	})
}

func assertInboundLimitCloses(t *testing.T, packetLimit, byteLimit int) {
	t.Helper()
	session, client := pipeSession()
	session.maxInboundPacketsPerSec = packetLimit
	session.maxInboundBytesPerSec = byteLimit
	received := make(chan []byte, 4)
	go session.Serve(func(_ *Session, packet []byte) { received <- packet })
	writeInitCode(t, client)
	packet := encryptedTestPacket(wire.OpPing)
	if _, err := client.Write(packet); err != nil {
		t.Fatal(err)
	}
	waitForTransportPacket(t, received, false)
	if _, err := client.Write(packet); err != nil {
		t.Fatal(err)
	}
	waitForTransportPacket(t, received, true)
	_ = client.Close()
}

func waitForTransportPacket(t *testing.T, received <-chan []byte, wantDisconnect bool) {
	t.Helper()
	select {
	case packet := <-received:
		if (packet == nil) != wantDisconnect {
			t.Fatalf("disconnect=%t, want %t", packet == nil, wantDisconnect)
		}
	case <-time.After(time.Second):
		t.Fatal("session did not report the expected packet state")
	}
}

func TestClosedSessionAccessorsAndWriterExit(t *testing.T) {
	session := NewTestSession(1, 1)
	session.Close()
	session.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	if session.QueueFree() != 1 || session.QueuedPacketsForTest() != 0 {
		t.Fatal("closed session accepted a packet")
	}

	writer := &Session{out: make(chan outboundPacket), done: make(chan struct{})}
	close(writer.out)
	writer.writeLoop()
	if !writer.IsClosed() {
		t.Fatal("writer exit did not end the session lifetime")
	}
	if got := expvar.Get("net_output_queue_high_water").String(); got == "" {
		t.Fatal("queue high-water expvar returned no value")
	}
	if got := expvar.Get("net_output_wait_max_micros").String(); got == "" {
		t.Fatal("queue wait expvar returned no value")
	}
}

func TestBoundedCloseDetailTruncatesLongErrors(t *testing.T) {
	got := boundedCloseDetail(errors.New(strings.Repeat("x", maxCloseDetailBytes+10)))
	if len(got) != maxCloseDetailBytes {
		t.Fatalf("close detail length=%d, want %d", len(got), maxCloseDetailBytes)
	}
}

func TestConfigureTCPContinuesAfterKeepAliveOptionFailure(t *testing.T) {
	listener, err := stdnet.ListenTCP("tcp", &stdnet.TCPAddr{IP: stdnet.ParseIP("127.0.0.1")})
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	client, err := stdnet.DialTCP("tcp", nil, listener.Addr().(*stdnet.TCPAddr))
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()
	accepted, err := listener.AcceptTCP()
	if err != nil {
		t.Fatal(err)
	}
	if err := accepted.Close(); err != nil {
		t.Fatal(err)
	}
	configureTCP(accepted, ListenerConfig{KeepAliveIdle: time.Second})
}
