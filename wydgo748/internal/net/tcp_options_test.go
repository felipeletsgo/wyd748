package net

import (
	"errors"
	stdnet "net"
	"runtime"
	"testing"
	"time"
)

// W4/D4: accepted sockets get bounded failure detection; an unsupported option
// is reported and never refuses the connection.
func TestTCPUserTimeoutAppliesOnLinuxAndReportsElsewhere(t *testing.T) {
	s, peer := tcpSession(t)
	defer peer.Close()
	defer s.Close()
	err := setTCPUserTimeout(s.conn.(*stdnet.TCPConn), 30*time.Second)
	if runtime.GOOS == "linux" {
		if err != nil {
			t.Fatal(err)
		}
	} else if !errors.Is(err, errTCPUserTimeoutUnsupported) {
		t.Fatalf("unsupported platform returned %v", err)
	}
	configureTCP(s.conn, ListenerConfig{KeepAliveIdle: 15 * time.Second, KeepAliveInterval: 5 * time.Second,
		KeepAliveCount: 4, TCPUserTimeout: 30 * time.Second})
	if s.IsClosed() {
		t.Fatal("TCP options closed the accepted socket")
	}
	if userTimeoutMillis(30*time.Second) != 30000 || userTimeoutMillis(time.Duration(1<<62)) != 1<<31-1 {
		t.Fatal("user timeout conversion overflowed")
	}
}
