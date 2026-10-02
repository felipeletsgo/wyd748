package net

import (
	"errors"
	"log"
	stdnet "net"
	"net/netip"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

var connSeq atomic.Int64

const defaultOutputQueueSize = 256

// ListenerConfig holds only operational transport protections. Gameplay and
// authentication rules stay in the World.
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
	mu        sync.Mutex
	total     int
	perOrigin map[string]int
	maxTotal  int
	maxPerIP  int
}

func newConnectionLimiter(maxTotal, maxPerIP int) *connectionLimiter {
	return &connectionLimiter{perOrigin: make(map[string]int), maxTotal: maxTotal, maxPerIP: maxPerIP}
}

func (l *connectionLimiter) acquire(origin string) bool {
	l.mu.Lock()
	defer l.mu.Unlock()
	if (l.maxTotal > 0 && l.total >= l.maxTotal) ||
		(l.maxPerIP > 0 && l.perOrigin[origin] >= l.maxPerIP) {
		return false
	}
	l.total++
	l.perOrigin[origin]++
	return true
}

func (l *connectionLimiter) release(origin string) {
	l.mu.Lock()
	defer l.mu.Unlock()
	if l.total > 0 {
		l.total--
	}
	if l.perOrigin[origin] <= 1 {
		delete(l.perOrigin, origin)
	} else {
		l.perOrigin[origin]--
	}
}

// ParseOriginIP canonicalizes only the IP observed on the socket. Fields
// declared by the client never take part in this operational identity.
func ParseOriginIP(ip string) (netip.Addr, bool) {
	addr, err := netip.ParseAddr(strings.TrimSpace(ip))
	if err != nil {
		return netip.Addr{}, false
	}
	return addr.Unmap(), true
}

// OriginLimitKey groups IPv6 by /64 so temporary addresses from the same
// prefix cannot multiply the pre-auth limit. IPv4 stays individual.
func OriginLimitKey(ip string) (string, bool) {
	addr, ok := ParseOriginIP(ip)
	if !ok {
		return "", false
	}
	if addr.Is6() {
		return netip.PrefixFrom(addr, 64).Masked().String(), true
	}
	return addr.String(), true
}

// ListenWithConfig limits sockets before the InitCode. The Session packet
// limit starts only after the handshake and, on its own, does not protect
// against Slowloris or descriptor exhaustion.
func ListenWithConfig(addr string, cfg ListenerConfig, onConn func(*Session)) error {
	if cfg.OutputQueueSize < 1 {
		cfg.OutputQueueSize = defaultOutputQueueSize
	}
	ln, err := stdnet.Listen("tcp", addr)
	if err != nil {
		return err
	}
	log.Printf("WYD-Go TMSrv listening on %s", addr)
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
		originKey, validOrigin := OriginLimitKey(ip)
		if !validOrigin || !limiter.acquire(originKey) {
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
			defer limiter.release(originKey)
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
