package webadmin

import (
	"context"
	"errors"
	"io"
	"log/slog"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"time"
)

func minimalHandlerConfig() Config {
	return Config{
		Origin:         "https://admin.test",
		AdminAccessPIN: testAdminPIN,
		Accounts:       testAccounts{},
		Control:        &testControl{},
		Persistent:     &testPersistent{},
		Staff:          func() (map[string]Staff, error) { return map[string]Staff{}, nil },
		Audit:          slog.New(slog.NewTextHandler(io.Discard, nil)),
	}
}

func TestNewRejectsInvalidOriginAndRequiredDependencies(t *testing.T) {
	staffFailure := errors.New("staff file unavailable")
	tests := []struct {
		name   string
		change func(*Config)
		want   string
	}{
		{name: "malformed origin", change: func(c *Config) { c.Origin = "://bad" }, want: "origin"},
		{name: "origin path", change: func(c *Config) { c.Origin = "https://admin.test/path" }, want: "exact public origin"},
		{name: "origin credentials", change: func(c *Config) { c.Origin = "https://user@admin.test" }, want: "exact public origin"},
		{name: "origin query", change: func(c *Config) { c.Origin = "https://admin.test?mode=1" }, want: "exact public origin"},
		{name: "origin fragment", change: func(c *Config) { c.Origin = "https://admin.test#section" }, want: "exact public origin"},
		{name: "production requires HTTPS", change: func(c *Config) { c.Origin = "http://admin.test" }, want: "HTTPS origin required"},
		{name: "dev requires loopback", change: func(c *Config) { c.Origin, c.DevHTTP = "http://admin.test", true }, want: "literal loopback origin"},
		{name: "dev requires HTTP", change: func(c *Config) { c.DevHTTP = true }, want: "literal loopback origin"},
		{name: "accounts required", change: func(c *Config) { c.Accounts = nil }, want: "accounts, control"},
		{name: "control required", change: func(c *Config) { c.Control = nil }, want: "accounts, control"},
		{name: "persistent reader required", change: func(c *Config) { c.Persistent = nil }, want: "accounts, control"},
		{name: "staff callback required", change: func(c *Config) { c.Staff = nil }, want: "accounts, control"},
		{name: "audit logger required", change: func(c *Config) { c.Audit = nil }, want: "accounts, control"},
		{name: "PIN shape", change: func(c *Config) { c.AdminAccessPIN = "12345" }, want: "exactly 6 digits"},
		{name: "PIN characters", change: func(c *Config) { c.AdminAccessPIN = "12a456" }, want: "exactly 6 digits"},
		{name: "staff load", change: func(c *Config) { c.Staff = func() (map[string]Staff, error) { return nil, staffFailure } }, want: staffFailure.Error()},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			cfg := minimalHandlerConfig()
			tc.change(&cfg)
			if _, err := New(cfg); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("New error=%v, want %q", err, tc.want)
			}
		})
	}
}

func TestNewSupportsLoopbackHTTPDevelopmentCookie(t *testing.T) {
	cfg := minimalHandlerConfig()
	cfg.Origin, cfg.DevHTTP = "http://127.0.0.1:8080", true
	h, err := New(cfg)
	if err != nil {
		t.Fatal(err)
	}
	defer h.Close()
	if h.cookie != "wyd_local_session" || h.host != "127.0.0.1:8080" {
		t.Fatalf("development origin setup: cookie=%q host=%q", h.cookie, h.host)
	}
}

func TestReadyEndpointCoversAbsentFailedAndHealthyChecks(t *testing.T) {
	for _, tc := range []struct {
		name  string
		ready func(context.Context) error
		want  int
	}{
		{name: "missing check", want: http.StatusServiceUnavailable},
		{name: "failed check", ready: func(context.Context) error { return errors.New("database unavailable") }, want: http.StatusServiceUnavailable},
		{name: "healthy", ready: func(context.Context) error { return nil }, want: http.StatusOK},
	} {
		t.Run(tc.name, func(t *testing.T) {
			cfg := minimalHandlerConfig()
			cfg.Ready = tc.ready
			h, err := New(cfg)
			if err != nil {
				t.Fatal(err)
			}
			defer h.Close()
			r := httptest.NewRequest(http.MethodGet, "https://admin.test/readyz", nil)
			w := httptest.NewRecorder()
			h.ServeHTTP(w, r)
			if w.Code != tc.want {
				t.Fatalf("ready status=%d body=%s, want %d", w.Code, w.Body.String(), tc.want)
			}
		})
	}
}

func TestAllowAllExpiresKeysAndRejectsCapacityWithoutPartialCharge(t *testing.T) {
	now := time.Date(2026, 11, 11, 12, 0, 0, 0, time.UTC)
	h := &Handler{rates: map[string]rate{"expired": {until: now, count: 1}}, now: func() time.Time { return now }}
	if !h.allowAll(rateLimit{key: "fresh", limit: 1}) {
		t.Fatal("expired rate state blocked a fresh request")
	}
	if _, ok := h.rates["expired"]; ok {
		t.Fatal("expired rate key was not pruned")
	}

	h.rates = make(map[string]rate, 2048)
	for i := 0; i < 2048; i++ {
		h.rates[string(rune(i+1))] = rate{until: now.Add(time.Minute), count: 1}
	}
	if h.allowAll(rateLimit{key: "new", limit: 2}, rateLimit{key: "existing", limit: 1}) {
		t.Fatal("rate limiter accepted a new key beyond its bounded capacity")
	}
	if _, charged := h.rates["new"]; charged {
		t.Fatal("capacity rejection partially charged the request")
	}
}
