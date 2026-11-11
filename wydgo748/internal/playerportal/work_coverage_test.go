package playerportal

import (
	"net/http/httptest"
	"strings"
	"testing"
	"time"
)

func TestWorkEnforcesUsernameRateLimitsAndConcurrency(t *testing.T) {
	now := time.Date(2026, 11, 11, 12, 0, 0, 0, time.UTC)
	newHandler := func() *Handler {
		return &Handler{
			rates:         make(map[string]rate),
			nextRateSweep: now.Add(time.Minute),
			slots:         make(chan struct{}, 2),
			now:           func() time.Time { return now },
		}
	}
	newRequest := func() *httptest.ResponseRecorder {
		return httptest.NewRecorder()
	}
	request := httptest.NewRequest("POST", "https://portal.example/api/player/v1/login", nil)
	request.RemoteAddr = "192.0.2.18:1234"
	peerKey := "login:" + limitNetwork("192.0.2.18")

	t.Run("long username", func(t *testing.T) {
		h := newHandler()
		response := newRequest()
		if h.work(response, request, strings.Repeat("u", 13), "login") || response.Code != 422 {
			t.Fatalf("oversized username allowed=%v status=%d", response.Code == 200, response.Code)
		}
	})

	for _, test := range []struct {
		name string
		key  string
		max  int
	}{
		{name: "network limit", key: peerKey, max: 8},
		{name: "account limit", key: "login:account:player", max: 8},
		{name: "global limit", key: "hash:global", max: 60},
	} {
		t.Run(test.name, func(t *testing.T) {
			h := newHandler()
			h.rates[test.key] = rate{Start: now, Count: test.max}
			response := newRequest()
			if h.work(response, request, "Player", "login") || response.Code != 429 {
				t.Fatalf("exhausted %s limit allowed=%v status=%d", test.name, response.Code == 200, response.Code)
			}
		})
	}

	t.Run("busy slots", func(t *testing.T) {
		h := newHandler()
		h.slots <- struct{}{}
		h.slots <- struct{}{}
		response := newRequest()
		if h.work(response, request, "Player", "login") || response.Code != 503 {
			t.Fatalf("full concurrency gate allowed=%v status=%d", response.Code == 200, response.Code)
		}
	})

	t.Run("admitted request acquires a slot", func(t *testing.T) {
		h := newHandler()
		response := newRequest()
		if !h.work(response, request, "Player", "login") || len(h.slots) != 1 || response.Code != 200 {
			t.Fatalf("admission result, slot count, status = false, %d, %d", len(h.slots), response.Code)
		}
	})
}
