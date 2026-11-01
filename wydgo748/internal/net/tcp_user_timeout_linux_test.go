//go:build linux

package net

import (
	stdnet "net"
	"syscall"
	"testing"
	"time"
)

func TestTCPUserTimeoutIsVisibleToTheKernel(t *testing.T) {
	s, peer := tcpSession(t)
	defer peer.Close()
	defer s.Close()
	tcp := s.conn.(*stdnet.TCPConn)
	if err := setTCPUserTimeout(tcp, 30*time.Second); err != nil {
		t.Fatal(err)
	}
	raw, err := tcp.SyscallConn()
	if err != nil {
		t.Fatal(err)
	}
	var got int
	var optionErr error
	if err := raw.Control(func(fd uintptr) {
		got, optionErr = syscall.GetsockoptInt(int(fd), syscall.IPPROTO_TCP, tcpUserTimeoutOption)
	}); err != nil || optionErr != nil || got != 30000 {
		t.Fatalf("TCP_USER_TIMEOUT=%d err=%v %v", got, err, optionErr)
	}
}
