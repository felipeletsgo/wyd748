package playerportal

import (
	"encoding/json"
	"fmt"
	"net/http/httptest"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

func TestLimitNetworkCanonicalization(t *testing.T) {
	for peer, want := range map[string]string{
		"192.0.2.1": "192.0.2.1", "::ffff:192.0.2.1": "192.0.2.1",
		"2001:db8:1:2::1": "2001:db8:1:2::/64", "2001:db8:1:2::abcd": "2001:db8:1:2::/64",
		"2001:db8:1:3::1": "2001:db8:1:3::/64", "fe80::1%eth0": "fe80::/64", "invalid": "unknown",
	} {
		if got := limitNetwork(peer); got != want {
			t.Errorf("network %q: got %q, want %q", peer, got, want)
		}
	}
}

func TestRegistrationNetworkWindowsAndRejectedAttempts(t *testing.T) {
	var l registrationLimiter
	now := time.Date(2026, 10, 12, 12, 0, 0, 0, time.UTC)
	for i := 0; i < 5; i++ {
		if wait := l.admit("192.0.2.1", now); wait != 0 {
			t.Fatal("early hourly rejection", wait)
		}
	}
	for i := 0; i < 100; i++ {
		if wait := l.admit("192.0.2.1", now.Add(10*time.Second)); wait != time.Hour-10*time.Second {
			t.Fatal("rejection changed the lockout", wait)
		}
	}
	if l.buckets["global:day"].count != 5 || l.buckets["network:192.0.2.1:day"].count != 5 {
		t.Fatal("rejected requests consumed quota")
	}
	if wait := l.admit("192.0.2.2", now); wait != 0 {
		t.Fatal("one network blocked another", wait)
	}
	if wait := l.admit("192.0.2.1", now.Add(time.Hour-time.Nanosecond)); wait != time.Nanosecond {
		t.Fatal("hour boundary admitted early", wait)
	}
	if wait := l.admit("192.0.2.1", now.Add(time.Hour)); wait != 0 {
		t.Fatal("hour quota did not recover", wait)
	}

	l = registrationLimiter{}
	for i := 0; i < 10; i++ {
		if wait := l.admit("192.0.2.1", now.Add(time.Duration(i/5)*time.Hour)); wait != 0 {
			t.Fatal("early daily rejection", i, wait)
		}
	}
	if wait := l.admit("192.0.2.1", now.Add(3*time.Hour)); wait != 21*time.Hour {
		t.Fatal("daily limit missing", wait)
	}
	if wait := l.admit("192.0.2.1", now.Add(24*time.Hour)); wait != 0 {
		t.Fatal("daily quota did not recover", wait)
	}
}

func TestRegistrationGlobalWindows(t *testing.T) {
	now := time.Date(2026, 10, 12, 12, 0, 0, 0, time.UTC)
	for _, tc := range []struct {
		name            string
		count           int
		at              func(int) time.Duration
		blockedAt, wait time.Duration
	}{
		{"minute", 30, func(int) time.Duration { return 0 }, 0, time.Minute},
		{"hour", 300, func(i int) time.Duration { return time.Duration(i/30) * time.Minute }, 10 * time.Minute, 50 * time.Minute},
		{"day", 1000, func(i int) time.Duration {
			return time.Duration(i/300)*time.Hour + time.Duration((i%300)/30)*time.Minute
		}, 4 * time.Hour, 20 * time.Hour},
	} {
		t.Run(tc.name, func(t *testing.T) {
			var l registrationLimiter
			for i := 0; i < tc.count; i++ {
				if wait := l.admit(fmt.Sprintf("network-%d", i), now.Add(tc.at(i))); wait != 0 {
					t.Fatal("early global rejection", i, wait)
				}
			}
			if wait := l.admit("new-network", now.Add(tc.blockedAt)); wait != tc.wait {
				t.Fatal("global limit missing", wait, tc.wait)
			}
			if wait := l.admit("new-network", now.Add(tc.blockedAt+tc.wait)); wait != 0 {
				t.Fatal("global quota did not recover", wait)
			}
		})
	}
}

func TestRegistrationLimiterConcurrencyAndBoundedStorage(t *testing.T) {
	var l registrationLimiter
	now := time.Now()
	var admitted atomic.Int32
	var wg sync.WaitGroup
	for i := 0; i < 100; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			if l.admit("192.0.2.1", now) == 0 {
				admitted.Add(1)
			}
		}()
	}
	wg.Wait()
	if admitted.Load() != 5 || len(l.buckets) != 5 {
		t.Fatal("concurrent quota bypass", admitted.Load(), len(l.buckets))
	}
	l = registrationLimiter{buckets: make(map[string]registrationBucket)}
	for i := 0; i < registrationBucketCapacity; i++ {
		l.buckets[strconv.Itoa(i)] = registrationBucket{expires: now.Add(time.Hour)}
	}
	if wait := l.admit("192.0.2.1", now); wait == 0 || len(l.buckets) != registrationBucketCapacity {
		t.Fatal("storage overflow did not fail closed")
	}
	if wait := l.admit("192.0.2.1", now.Add(time.Hour)); wait != 0 || len(l.buckets) != 5 {
		t.Fatal("expired storage was not reclaimed", wait, len(l.buckets))
	}
	if wait := l.admit("unknown", now); wait == 0 {
		t.Fatal("unknown network was admitted")
	}
}

func TestRegistrationMalformedFloodAndSessionRotation(t *testing.T) {
	h, b := testHandler(t)
	c, csrf := bootstrap(t, h)
	for i := 0; i < 5; i++ {
		if i > 0 && i < 3 {
			c, csrf = bootstrap(t, h)
		}
		body := fmt.Sprintf(`{"username":"Flood%d","unexpected":true}`, i)
		if w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf); w.Code != 400 {
			t.Fatal("malformed attempt", w.Code)
		}
	}
	c, csrf = bootstrap(t, h)
	r := httptest.NewRequest("POST", "https://wyd.example.com/api/player/v1/accounts", nil)
	r.RemoteAddr = "192.0.2.1:42"
	r.AddCookie(c)
	r.Header.Set("Origin", h.cfg.Origin)
	r.Header.Set("Content-Type", "application/json")
	r.Header.Set("X-CSRF-Token", csrf)
	r.Header.Set("X-Forwarded-For", "203.0.113.99") // Public peers cannot spoof quota identity.
	r.Body = unreadBody{}
	w := httptest.NewRecorder()
	h.ServeHTTP(w, r)
	if w.Code != 429 || w.Header().Get("Retry-After") != "3600" || len(b.accounts) != 0 {
		t.Fatal("malformed/session/name flood bypassed admission", w.Code, w.Header())
	}
	if len(h.slots) != 0 || len(h.registrationSlots) != 0 {
		t.Fatal("rejected request occupied password work")
	}
}

type unreadBody struct{}

func (unreadBody) Read([]byte) (int, error) { panic("rejected registration body was read") }
func (unreadBody) Close() error             { return nil }

func TestRegistrationIPv6RotationAndTrustedProxy(t *testing.T) {
	h, b := testHandler(t)
	h.cfg.TrustLoopbackProxy = true
	now := h.now()
	h.now = func() time.Time { return now }
	for i := 1; i <= 6; i++ {
		// Let old anonymous sessions expire while staying inside the hourly quota.
		now = now.Add(5 * time.Minute)
		peer := fmt.Sprintf("2001:db8:1:2::%x", i)
		r := httptest.NewRequest("GET", "https://wyd.example.com/api/player/v1/session", nil)
		r.RemoteAddr = "127.0.0.1:42"
		r.Header.Set("X-Forwarded-For", peer)
		w := httptest.NewRecorder()
		h.ServeHTTP(w, r)
		if w.Code != 200 {
			t.Fatal("proxy bootstrap", w.Code)
		}
		var data struct{ CSRF string }
		if err := json.Unmarshal(w.Body.Bytes(), &data); err != nil {
			t.Fatal(err)
		}
		r = httptest.NewRequest("POST", "https://wyd.example.com/api/player/v1/accounts", strings.NewReader(`{"turnstileToken":"test-token"}`))
		r.RemoteAddr = "127.0.0.1:42"
		r.Header.Set("X-Forwarded-For", peer)
		r.Header.Set("Origin", h.cfg.Origin)
		r.Header.Set("Content-Type", "application/json")
		r.Header.Set("X-CSRF-Token", data.CSRF)
		r.AddCookie(w.Result().Cookies()[0])
		w = httptest.NewRecorder()
		h.ServeHTTP(w, r)
		want := 422
		if i == 6 {
			want = 429
		}
		if w.Code != want {
			t.Fatal("IPv6 rotation", i, w.Code, want)
		}
	}
	if len(b.accounts) != 0 {
		t.Fatal("invalid requests wrote accounts")
	}
}

func TestRegistrationQuotaDoesNotBlockLogin(t *testing.T) {
	h, _ := testHandler(t)
	c, csrf := bootstrap(t, h)
	for i := 0; i < 5; i++ {
		if w := request(h, "POST", "/api/player/v1/accounts", `{"turnstileToken":"test-token"}`, c, csrf); w.Code != 422 {
			t.Fatal(w.Code)
		}
	}
	// Also saturate global registration quotas and occupy the registration slot.
	h.registrationLimit.buckets["global:minute"] = registrationBucket{expires: h.now().Add(time.Minute), count: 30}
	h.registrationSlots <- struct{}{}
	h.slots <- struct{}{}
	w := request(h, "POST", "/api/player/v1/login", `{"username":"Missing","password":"Safe1234","turnstileToken":"test-token"}`, c, csrf)
	<-h.slots
	<-h.registrationSlots
	if w.Code != 401 {
		t.Fatal("registration load prevented login", w.Code)
	}
	if len(h.slots) != 0 {
		t.Fatal("login leaked its work slot")
	}
}

func TestRegistrationConcurrentHTTPFlood(t *testing.T) {
	h, b := testHandler(t)
	c, csrf := bootstrap(t, h)
	var admitted, denied atomic.Int32
	var wg sync.WaitGroup
	for i := 0; i < 60; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			w := request(h, "POST", "/api/player/v1/accounts", `{"unknown":true}`, c, csrf)
			switch w.Code {
			case 400:
				admitted.Add(1)
			case 429:
				denied.Add(1)
			default:
				t.Errorf("unexpected flood response: %d", w.Code)
			}
		}()
	}
	wg.Wait()
	if admitted.Load() != 5 || denied.Load() != 55 || len(b.accounts) != 0 {
		t.Fatal("HTTP flood bypassed quota", admitted.Load(), denied.Load())
	}
}

func TestRegistrationFiveAccountsAndSixthRejected(t *testing.T) {
	h, b := testHandler(t)
	now := h.now()
	h.now = func() time.Time { return now }
	c, csrf := bootstrap(t, h)
	for i := 1; i <= 6; i++ {
		body := fmt.Sprintf(`{"username":"Player%d","password":"secret123","passwordConfirmation":"secret123","turnstileToken":"test-token"}`, i)
		w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf)
		want := 201
		if i == 6 {
			want = 429
			if w.Header().Get("Retry-After") != "3600" {
				t.Fatal("missing hourly retry", w.Header())
			}
		}
		if w.Code != want {
			t.Fatal("registration boundary", i, w.Code, w.Body.String())
		}
	}
	if len(b.accounts) != 5 {
		t.Fatal("unexpected persisted account count", len(b.accounts))
	}
}

func TestRegistrationCSRFRejectionDoesNotChargeVictim(t *testing.T) {
	h, _ := testHandler(t)
	c, _ := bootstrap(t, h)
	if w := request(h, "POST", "/api/player/v1/accounts", `{}`, c, "invalid"); w.Code != 403 {
		t.Fatal("invalid CSRF was accepted", w.Code)
	}
	if len(h.registrationLimit.buckets) != 0 {
		t.Fatal("CSRF rejection charged registration quota")
	}
}

func TestRegistrationBusyReleasesSharedSlots(t *testing.T) {
	for _, registrationBusy := range []bool{true, false} {
		h, _ := testHandler(t)
		c, csrf := bootstrap(t, h)
		if registrationBusy {
			h.registrationSlots <- struct{}{}
		} else {
			h.slots <- struct{}{}
			h.slots <- struct{}{}
		}
		w := request(h, "POST", "/api/player/v1/accounts", `{"turnstileToken":"test-token"}`, c, csrf)
		if w.Code != 503 {
			t.Fatal("busy service accepted registration", w.Code)
		}
		if !registrationBusy && len(h.registrationSlots) != 0 {
			t.Fatal("registration slot leaked")
		}
		if registrationBusy && len(h.slots) != 0 {
			t.Fatal("busy registration occupied login capacity")
		}
	}
}

func TestLocalReadinessSurvivesPublicFlood(t *testing.T) {
	h, _ := testHandler(t)
	h.cfg.TrustLoopbackProxy = true
	h.rates["read:global"] = rate{Start: h.now(), Count: 1200}
	for _, tc := range []struct {
		remote, forwarded, path string
		want                    int
	}{
		{"127.0.0.1:42", "", "/readyz", 200},
		{"[::1]:42", "", "/healthz", 200},
		{"192.0.2.1:42", "", "/readyz", 429},
		{"127.0.0.1:42", "192.0.2.1", "/readyz", 429},
		{"127.0.0.1:42", "invalid", "/readyz", 429},
		{"127.0.0.1:42", "", "/api/player/v1/ranking", 429},
	} {
		r := httptest.NewRequest("GET", "https://wyd.example.com"+tc.path, nil)
		r.RemoteAddr = tc.remote
		r.Header.Set("X-Forwarded-For", tc.forwarded)
		w := httptest.NewRecorder()
		h.ServeHTTP(w, r)
		if w.Code != tc.want {
			t.Fatal("health exemption", tc, w.Code)
		}
		if w.Code == 429 && w.Header().Get("Retry-After") != "60" {
			t.Fatal("missing retry header")
		}
	}
}

func TestRateSweepIsAmortizedAndCurrentKeyExpires(t *testing.T) {
	h, _ := testHandler(t)
	now := time.Now()
	h.now = func() time.Time { return now }
	h.nextRateSweep = now.Add(time.Minute)
	h.rates["expired"] = rate{Start: now.Add(-time.Minute), Count: 1}
	if !h.allow("active", 1) {
		t.Fatal("request rejected")
	}
	if _, ok := h.rates["expired"]; !ok {
		t.Fatal("swept before scheduled interval")
	}
	if !h.allow("expired", 1) {
		t.Fatal("current expired key did not reset")
	}
	now = now.Add(time.Minute)
	if !h.allow("next", 1) || len(h.rates) != 1 {
		t.Fatal("scheduled sweep failed", h.rates)
	}
}
