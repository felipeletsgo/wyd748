package net

import (
	"io"
	stdnet "net"
	"testing"
	"time"
)

type writerRegressionConn struct {
	closed      bool
	short       bool
	deadlineErr bool
	deadline    time.Time
}

func (*writerRegressionConn) Read([]byte) (int, error) { return 0, io.EOF }
func (c *writerRegressionConn) Write(p []byte) (int, error) {
	if c.short {
		return len(p) - 1, nil
	}
	return 0, io.ErrClosedPipe
}
func (c *writerRegressionConn) Close() error                  { c.closed = true; return nil }
func (*writerRegressionConn) LocalAddr() stdnet.Addr          { return &stdnet.TCPAddr{} }
func (*writerRegressionConn) RemoteAddr() stdnet.Addr         { return &stdnet.TCPAddr{} }
func (*writerRegressionConn) SetDeadline(time.Time) error     { return nil }
func (*writerRegressionConn) SetReadDeadline(time.Time) error { return nil }
func (c *writerRegressionConn) SetWriteDeadline(deadline time.Time) error {
	c.deadline = deadline
	if c.deadlineErr {
		return io.ErrClosedPipe
	}
	return nil
}

func TestWriterFailureClosesConnectionAndLifetime(t *testing.T) {
	for _, c := range []*writerRegressionConn{{}, {short: true}, {deadlineErr: true}} {
		s := &Session{conn: c, out: make(chan []byte, 1), done: make(chan struct{})}
		s.out <- []byte{1, 2}
		s.writeLoop()
		if !c.closed || !s.IsClosed() || c.deadline.IsZero() {
			t.Fatal("writer failure did not set a deadline and close both lifetimes")
		}
	}
}

func TestBlockedWriterDeadlineReleasesReader(t *testing.T) {
	s, peer := pipeSession()
	defer peer.Close()
	s.frameWriteTimeout = 20 * time.Millisecond
	ended := make(chan struct{})
	go s.Serve(func(session *Session, packet []byte) {
		if packet == nil {
			close(ended)
		}
	})
	// Reuse the established handshake helper; the peer then stops reading.
	writeInitCode(t, peer)
	s.out <- []byte{1}
	select {
	case <-ended:
		if !s.IsClosed() {
			t.Fatal("disconnect callback preceded lifetime closure")
		}
	case <-time.After(time.Second):
		t.Fatal("blocked writer did not release Serve")
	}
}
