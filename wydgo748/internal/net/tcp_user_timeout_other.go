//go:build !linux

package net

import (
	stdnet "net"
	"time"
)

// Only the Linux production host supports TCP_USER_TIMEOUT here; keepalive
// still applies elsewhere.
func setTCPUserTimeout(*stdnet.TCPConn, time.Duration) error {
	return errTCPUserTimeoutUnsupported
}
