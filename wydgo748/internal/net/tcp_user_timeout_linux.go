//go:build linux

package net

import (
	stdnet "net"
	"syscall"
	"time"
)

// tcpUserTimeoutOption is TCP_USER_TIMEOUT from linux/tcp.h on every arch.
const tcpUserTimeoutOption = 0x12

// setTCPUserTimeout bounds how long sent data may stay unacknowledged before
// the kernel closes the socket; the writer then records a write failure.
func setTCPUserTimeout(conn *stdnet.TCPConn, timeout time.Duration) error {
	raw, err := conn.SyscallConn()
	if err != nil {
		return err
	}
	var optionErr error
	if err := raw.Control(func(fd uintptr) {
		optionErr = syscall.SetsockoptInt(int(fd), syscall.IPPROTO_TCP, tcpUserTimeoutOption, userTimeoutMillis(timeout))
	}); err != nil {
		return err
	}
	return optionErr
}
