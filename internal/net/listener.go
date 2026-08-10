package net

import (
	"errors"
	"log"
	stdnet "net"
	"sync"
	"sync/atomic"
	"time"
)

var connSeq atomic.Int64

const defaultOutputQueueSize = 256

// ListenerConfig contem somente protecoes operacionais do transporte. Regras
// de gameplay e autenticacao continuam no World.
type ListenerConfig struct {
	OutputQueueSize      int
	MaxConnections       int
	MaxConnectionsPerIP  int
	HandshakeTimeout     time.Duration
	SessionIdleTimeout   time.Duration
	FrameReadTimeout     time.Duration
	InboundPacketsPerSec int
	InboundBytesPerSec   int
}

type connectionLimiter struct {
	mu       sync.Mutex
	total    int
	perIP    map[string]int
	maxTotal int
	maxPerIP int
}

func newConnectionLimiter(maxTotal, maxPerIP int) *connectionLimiter {
	return &connectionLimiter{perIP: make(map[string]int), maxTotal: maxTotal, maxPerIP: maxPerIP}
}

func (l *connectionLimiter) acquire(ip string) bool {
	l.mu.Lock()
	defer l.mu.Unlock()
	if (l.maxTotal > 0 && l.total >= l.maxTotal) ||
		(l.maxPerIP > 0 && l.perIP[ip] >= l.maxPerIP) {
		return false
	}
	l.total++
	l.perIP[ip]++
	return true
}

func (l *connectionLimiter) release(ip string) {
	l.mu.Lock()
	defer l.mu.Unlock()
	if l.total > 0 {
		l.total--
	}
	if l.perIP[ip] <= 1 {
		delete(l.perIP, ip)
	} else {
		l.perIP[ip]--
	}
}

// ListenWithConfig limita sockets antes do InitCode. O limite de pacotes da
// Session so comeca depois do handshake e, sozinho, nao protege contra
// Slowloris nem exaustao de descritores.
func ListenWithConfig(addr string, cfg ListenerConfig, onConn func(*Session)) error {
	if cfg.OutputQueueSize < 1 {
		cfg.OutputQueueSize = defaultOutputQueueSize
	}
	ln, err := stdnet.Listen("tcp", addr)
	if err != nil {
		return err
	}
	log.Printf("WYD-Go TMSrv escutando em %s", addr)
	limiter := newConnectionLimiter(cfg.MaxConnections, cfg.MaxConnectionsPerIP)
	for {
		c, err := ln.Accept()
		if err != nil {
			if errors.Is(err, stdnet.ErrClosed) {
				return nil
			}
			log.Printf("accept: %v", err)
			continue
		}
		ip := remoteIP(c.RemoteAddr())
		if !limiter.acquire(ip) {
			_ = c.Close()
			continue
		}
		s := &Session{
			ID:                      connSeq.Add(1),
			conn:                    c,
			remoteIP:                ip,
			out:                     make(chan []byte, cfg.OutputQueueSize),
			done:                    make(chan struct{}),
			handshakeTimeout:        cfg.HandshakeTimeout,
			idleTimeout:             cfg.SessionIdleTimeout,
			frameReadTimeout:        cfg.FrameReadTimeout,
			maxInboundPacketsPerSec: cfg.InboundPacketsPerSec,
			maxInboundBytesPerSec:   cfg.InboundBytesPerSec,
		}
		go func() {
			defer limiter.release(ip)
			onConn(s)
		}()
	}
}

func remoteIP(addr stdnet.Addr) string {
	if addr == nil {
		return ""
	}
	host, _, err := stdnet.SplitHostPort(addr.String())
	if err != nil {
		return addr.String()
	}
	return host
}
