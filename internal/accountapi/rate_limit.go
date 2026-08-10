package accountapi

import (
	"net"
	"net/http"
	"net/netip"
	"strings"
	"sync"
	"time"
)

type rateEntry struct {
	count int
	reset time.Time
}

type rateLimiter struct {
	mu        sync.Mutex
	limit     int
	visitors  map[string]rateEntry
	lastSweep time.Time
}

const maxRateLimiterVisitors = 65536

func newRateLimiter(limit int) *rateLimiter {
	return &rateLimiter{limit: limit, visitors: make(map[string]rateEntry)}
}

func (l *rateLimiter) Allow(ip string) bool {
	l.mu.Lock()
	defer l.mu.Unlock()
	now := time.Now()
	// Expurgo amortizado: sem isto o mapa cresceria sem limite, guardando uma
	// entrada por IP distinto para sempre. No maximo uma varredura por minuto
	// remove as janelas ja expiradas, limitando o mapa aos IPs ativos.
	if now.Sub(l.lastSweep) >= time.Minute {
		for addr, e := range l.visitors {
			if !now.Before(e.reset) {
				delete(l.visitors, addr)
			}
		}
		l.lastSweep = now
	}
	if _, exists := l.visitors[ip]; !exists && len(l.visitors) >= maxRateLimiterVisitors {
		return false
	}
	entry := l.visitors[ip]
	if entry.reset.IsZero() || !now.Before(entry.reset) {
		entry = rateEntry{reset: now.Add(time.Minute)}
	}
	entry.count++
	l.visitors[ip] = entry
	return entry.count <= l.limit
}

func clientIP(r *http.Request, trustedProxies []netip.Prefix) string {
	host, _, err := net.SplitHostPort(r.RemoteAddr)
	if err != nil {
		host = r.RemoteAddr
	}
	parsed, parsedOK := netip.ParseAddr(host)
	trusted := false
	if parsedOK == nil {
		for _, prefix := range trustedProxies {
			if prefix.Contains(parsed.Unmap()) || prefix.Contains(parsed) {
				trusted = true
				break
			}
		}
	}
	// Forwarded headers so pertencem ao proxy declarado. Loopback por si so nao
	// concede confianca: testes/ferramentas locais tambem podem forjar headers.
	if trusted {
		if first := strings.TrimSpace(strings.Split(r.Header.Get("X-Forwarded-For"), ",")[0]); net.ParseIP(first) != nil {
			return first
		}
	}
	return host
}
