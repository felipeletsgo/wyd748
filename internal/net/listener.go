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

// ParseOriginIP canonicaliza exclusivamente o IP observado no socket. Campos
// declarados pelo client nunca participam desta identidade operacional.
func ParseOriginIP(ip string) (netip.Addr, bool) {
	addr, err := netip.ParseAddr(strings.TrimSpace(ip))
	if err != nil {
		return netip.Addr{}, false
	}
	return addr.Unmap(), true
}

// OriginLimitKey agrupa IPv6 pelo /64 para impedir que enderecos temporarios
// do mesmo prefixo multipliquem o limite pre-auth. IPv4 permanece individual.
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
	return serveListener(ln, cfg, onConn)
}

// serveListener isola o loop de Accept da criacao do socket. Em producao ele
// recebe o listener TCP acima; em teste, o listener pode ser fechado de forma
// deterministica sem expor ownership novo nem alterar a Session.
func serveListener(ln stdnet.Listener, cfg ListenerConfig, onConn func(*Session)) error {
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
