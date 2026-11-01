package net

import (
	"expvar"
	"io"
	stdnet "net"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func closeCount(cause CloseCause) int64 {
	if v, ok := metricSessionCloses.Get(cause.String()).(*expvar.Int); ok {
		return v.Value()
	}
	return 0
}

func serveUntilEnd(t *testing.T, s *Session, peer stdnet.Conn, act func()) {
	t.Helper()
	ended := make(chan struct{})
	go func() {
		s.Serve(func(_ *Session, packet []byte) {})
		close(ended)
	}()
	act()
	select {
	case <-ended:
	case <-time.After(2 * time.Second):
		_ = peer.Close()
		t.Fatal("Serve did not end")
	}
}

func TestFirstCloseCauseWins(t *testing.T) {
	s := NewTestSession(1, 1)
	s.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	s.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	if !s.IsClosed() || s.CloseCause() != CloseCauseOutputOverflow {
		t.Fatalf("overflow cause = %s closed=%t", s.CloseCause(), s.IsClosed())
	}
	s.CloseWithCause(CloseCausePersistence, nil)
	s.Close()
	if s.CloseCause() != CloseCauseOutputOverflow {
		t.Fatalf("later close replaced the first cause: %s", s.CloseCause())
	}
	if NewTestSession(2, 1).CloseCause() != CloseCauseNone {
		t.Fatal("open session reports a close cause")
	}
}

func TestAdmissionCausePrecedesGraceClose(t *testing.T) {
	s := NewTestSession(1, 4)
	s.RejectAdmission(wire.MessagePanel("rejected"))
	s.CloseWithCause(CloseCausePeerClosed, io.EOF)
	if s.CloseCause() != CloseCauseAdmission {
		t.Fatalf("admission cause = %s", s.CloseCause())
	}
}

// tcpSession uses loopback TCP where net.Pipe deadline errors differ from sockets.
func tcpSession(t *testing.T) (*Session, stdnet.Conn) {
	t.Helper()
	listener, err := stdnet.Listen("tcp4", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	client, err := stdnet.DialTimeout("tcp4", listener.Addr().String(), time.Second)
	if err != nil {
		t.Fatal(err)
	}
	server, err := listener.Accept()
	if err != nil {
		t.Fatal(err)
	}
	return &Session{ID: 1, conn: server, out: make(chan outboundPacket, 8), done: make(chan struct{})}, client
}

func TestServeRecordsOneCausePerSession(t *testing.T) {
	cases := []struct {
		name  string
		cause CloseCause
		act   func(*Session, stdnet.Conn)
	}{
		{"handshake", CloseCauseHandshake, func(_ *Session, peer stdnet.Conn) { _ = peer.Close() }},
		{"peer", CloseCausePeerClosed, func(_ *Session, peer stdnet.Conn) {
			writeInitCode(t, peer)
			_ = peer.Close()
		}},
		{"protocol", CloseCauseProtocol, func(_ *Session, peer stdnet.Conn) {
			writeInitCode(t, peer)
			_, _ = peer.Write([]byte{1, 0})
		}},
		{"server", CloseCauseServer, func(s *Session, peer stdnet.Conn) {
			writeInitCode(t, peer)
			s.Close()
		}},
		{"persistence", CloseCausePersistence, func(s *Session, peer stdnet.Conn) {
			writeInitCode(t, peer)
			s.CloseWithCause(CloseCausePersistence, nil)
		}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			s, peer := tcpSession(t)
			defer peer.Close()
			before := closeCount(tc.cause)
			serveUntilEnd(t, s, peer, func() { tc.act(s, peer) })
			if s.CloseCause() != tc.cause {
				t.Fatalf("cause = %s, want %s", s.CloseCause(), tc.cause)
			}
			if got := closeCount(tc.cause) - before; got != 1 {
				t.Fatalf("close counter advanced by %d, want 1", got)
			}
		})
	}
}

func TestWriterRecordsWriteCauses(t *testing.T) {
	for _, tc := range []struct {
		conn  *writerRegressionConn
		cause CloseCause
	}{
		{&writerRegressionConn{}, CloseCauseWriteError},
		{&writerRegressionConn{short: true}, CloseCauseWriteError},
		{&writerRegressionConn{deadlineErr: true}, CloseCauseWriteError},
	} {
		s := &Session{conn: tc.conn, out: make(chan outboundPacket, 1), done: make(chan struct{})}
		s.out <- outboundPacket{data: []byte{1, 2}}
		s.writeLoop()
		if s.CloseCause() != tc.cause {
			t.Fatalf("writer cause = %s, want %s", s.CloseCause(), tc.cause)
		}
	}
}

func TestBlockedWriterRecordsWriteTimeout(t *testing.T) {
	s, peer := pipeSession()
	defer peer.Close()
	s.frameWriteTimeout = 20 * time.Millisecond
	serveUntilEnd(t, s, peer, func() {
		writeInitCode(t, peer)
		s.Send(wire.Build(wire.OpPing, 0, wire.HeaderSize))
	})
	if s.CloseCause() != CloseCauseWriteTimeout {
		t.Fatalf("blocked writer cause = %s", s.CloseCause())
	}
}

func TestOutputDepthAndWaitLabels(t *testing.T) {
	for _, tc := range []struct {
		depth, capacity int
		want            string
	}{
		{1, 256, "<=25%"}, {64, 256, "<=25%"}, {65, 256, "<=50%"}, {192, 256, "<=75%"},
		{255, 256, "<100%"}, {256, 256, "full"}, {1, 1, "full"}, {0, 0, "full"},
	} {
		if got := outputDepthLabel(tc.depth, tc.capacity); got != tc.want {
			t.Fatalf("depth %d/%d = %q, want %q", tc.depth, tc.capacity, got, tc.want)
		}
	}
	if outputWaitLabel(0) != "<=1ms" || outputWaitLabel(2*time.Second) != "+Inf" {
		t.Fatal("wait labels do not match the fixed duration buckets")
	}
	if len(CloseCauseLabels()) != int(closeCauseCount)-1 || CloseCause(200).String() != "server" {
		t.Fatal("close cause labels are not a closed set")
	}
}
