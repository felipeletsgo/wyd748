package net

import (
	"errors"
	"log"
	stdnet "net"
	"sync"
	"time"
)

// TCP failure detection (DOCS/server/server-correction-plan.md, W4/D4). A
// blackholed peer can keep a session, and its account claim, for minutes:
// Linux retransmits unacknowledged data for about 15 minutes by default, and
// Go's default keepalive needs about 150 seconds for an idle peer. Shorter
// keepalive probes and TCP_USER_TIMEOUT bound both cases.

var errTCPUserTimeoutUnsupported = errors.New("TCP_USER_TIMEOUT is not supported on this platform")

var tcpOptionWarning sync.Once

// configureTCP applies the listener's keepalive and user timeout to one
// accepted socket. A failed option is logged once and never refuses the peer.
func configureTCP(conn stdnet.Conn, cfg ListenerConfig) {
	tcp, ok := conn.(*stdnet.TCPConn)
	if !ok {
		return
	}
	if cfg.KeepAliveIdle > 0 {
		err := tcp.SetKeepAliveConfig(stdnet.KeepAliveConfig{
			Enable: true, Idle: cfg.KeepAliveIdle, Interval: cfg.KeepAliveInterval, Count: cfg.KeepAliveCount,
		})
		if err != nil {
			warnTCPOption("keepalive", err)
		}
	}
	if cfg.TCPUserTimeout > 0 {
		if err := setTCPUserTimeout(tcp, cfg.TCPUserTimeout); err != nil {
			warnTCPOption("user timeout", err)
		}
	}
}

func warnTCPOption(option string, err error) {
	tcpOptionWarning.Do(func() { log.Printf("TCP %s option not applied: %v", option, err) })
}

// userTimeoutMillis converts the configured bound for setsockopt.
func userTimeoutMillis(timeout time.Duration) int {
	return int(min(timeout, time.Duration(1<<31-1)*time.Millisecond) / time.Millisecond)
}
