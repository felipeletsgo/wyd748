package net

import (
	"io"
	stdnet "net"
	"testing"
	"time"
)

type auditWriteFailureConn struct{ closed bool }

func (*auditWriteFailureConn) Read([]byte) (int, error) { return 0, io.EOF }
func (*auditWriteFailureConn) Write([]byte) (int, error) { return 0, io.ErrClosedPipe }
func (c *auditWriteFailureConn) Close() error { c.closed = true; return nil }
func (*auditWriteFailureConn) LocalAddr() stdnet.Addr { return &stdnet.TCPAddr{} }
func (*auditWriteFailureConn) RemoteAddr() stdnet.Addr { return &stdnet.TCPAddr{} }
func (*auditWriteFailureConn) SetDeadline(time.Time) error { return nil }
func (*auditWriteFailureConn) SetReadDeadline(time.Time) error { return nil }
func (*auditWriteFailureConn) SetWriteDeadline(time.Time) error { return nil }

func TestAuditEvidenceWriterFailureLeavesSessionOpen(t *testing.T) {
	c := &auditWriteFailureConn{}
	s := &Session{conn: c, out: make(chan []byte, 1), done: make(chan struct{})}
	s.out <- []byte{1}
	s.writeLoop()
	if c.closed || s.IsClosed() {
		t.Fatal("audited missing writer cleanup was not reproduced")
	}
	t.Log("writer exited on an I/O error without closing the connection or session lifetime")
}
