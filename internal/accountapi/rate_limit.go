package accountapi

import (
	"net"
	"net/http"
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
	entry := l.visitors[ip]
	if entry.reset.IsZero() || !now.Before(entry.reset) {
		entry = rateEntry{reset: now.Add(time.Minute)}
	}
	entry.count++
	l.visitors[ip] = entry
	return entry.count <= l.limit
}

func clientIP(r *http.Request) string {
	host, _, err := net.SplitHostPort(r.RemoteAddr)
	if err != nil {
		host = r.RemoteAddr
	}
	parsed := net.ParseIP(host)
	// A API escuta em loopback por padrao. Quando um proxy local encaminha a
	// requisicao, aceita o primeiro X-Forwarded-For validado como IP.
	if parsed != nil && parsed.IsLoopback() {
		if first := strings.TrimSpace(strings.Split(r.Header.Get("X-Forwarded-For"), ",")[0]); net.ParseIP(first) != nil {
			return first
		}
	}
	return host
}
