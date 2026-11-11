package webadmin

import (
	"crypto/sha256"
	"net/http"
	"net/netip"
	"strings"
	"testing"
	"time"
)

func TestSessionInfoReturnsAndRevokesStaffSession(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	cookie, _, _ := loginTest(t, h)

	w := request(h, "GET", "/api/v1/auth/session", "", cookie, "")
	if w.Code != 200 || !strings.Contains(w.Body.String(), `"authenticated":true`) || !strings.Contains(w.Body.String(), `"account":"operator"`) {
		t.Fatalf("authenticated session info: %d %s", w.Code, w.Body.String())
	}

	entries["operator"] = Staff{Capabilities: []string{StatusCapability}}
	w = request(h, "GET", "/api/v1/auth/session", "", cookie, "")
	if w.Code != 403 || !strings.Contains(w.Body.String(), "staff_revoked") {
		t.Fatalf("revoked session info: %d %s", w.Code, w.Body.String())
	}
	key := sha256.Sum256([]byte(cookie.Value))
	h.mu.Lock()
	_, retained := h.sessions[key]
	h.mu.Unlock()
	if retained {
		t.Fatal("revoked staff session remained active")
	}
}

func TestSessionInfoEnforcesRateLimitAndPreservesAuthenticatedCapacity(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	for i := 0; i < 60; i++ {
		w := request(h, "GET", "/api/v1/auth/session", "", nil, "")
		if w.Code != 200 {
			t.Fatalf("session request %d: %d %s", i+1, w.Code, w.Body.String())
		}
	}
	w := request(h, "GET", "/api/v1/auth/session", "", nil, "")
	if w.Code != 429 || w.Header().Get("Retry-After") != "60" || !strings.Contains(w.Body.String(), "rate_limited") {
		t.Fatalf("session rate limit: %d %s retry=%q", w.Code, w.Body.String(), w.Header().Get("Retry-After"))
	}

	h, _, _, _ = newTestHandler(t)
	now := time.Now().UTC()
	h.mu.Lock()
	for i := 0; i < maxSessions; i++ {
		var key [32]byte
		key[0], key[1] = byte(i+1), byte((i+1)>>8)
		h.sessions[key] = session{Actor: "operator", LastSeen: now, Expires: now.Add(time.Hour)}
	}
	if len(h.sessions) != maxSessions {
		h.mu.Unlock()
		t.Fatalf("failed to fill session capacity: %d", len(h.sessions))
	}
	h.mu.Unlock()
	w = request(h, "GET", "/api/v1/auth/session", "", nil, "")
	if w.Code != 503 || !strings.Contains(w.Body.String(), "session_capacity") {
		t.Fatalf("full authenticated capacity: %d %s", w.Code, w.Body.String())
	}
	h.mu.Lock()
	count := len(h.sessions)
	h.mu.Unlock()
	if count != maxSessions {
		t.Fatalf("session admission evicted an authenticated session: count=%d", count)
	}
}

func TestRemoteIPParsesAddressAndAddressPortForms(t *testing.T) {
	for _, tc := range []struct {
		remote string
		want   string
		ok     bool
	}{
		{remote: "127.0.0.1:1234", want: "127.0.0.1", ok: true},
		{remote: "::1", want: "::1", ok: true},
		{remote: "[::ffff:192.0.2.7]:80", want: "192.0.2.7", ok: true},
		{remote: "invalid", ok: false},
	} {
		got, ok := remoteIP(&http.Request{RemoteAddr: tc.remote})
		if ok != tc.ok || (ok && got != netip.MustParseAddr(tc.want)) {
			t.Errorf("remoteIP(%q)=(%v,%v), want (%q,%v)", tc.remote, got, ok, tc.want, tc.ok)
		}
	}
}
