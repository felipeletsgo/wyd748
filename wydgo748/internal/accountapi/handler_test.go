package accountapi

import (
	"context"
	"errors"
	"net/http"
	"net/http/httptest"
	"net/netip"
	"strings"
	"testing"

	"wydgo/internal/store"
)

func TestCreateAccountAPI(t *testing.T) {
	st := store.NewJSONStore(t.TempDir())
	h := New(st)
	req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(
		`{"username":"ApiUser","password":"Pass123!","passwordConfirmation":"Pass123!"}`))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, req)
	if rec.Code != http.StatusCreated {
		t.Fatalf("status=%d body=%s", rec.Code, rec.Body.String())
	}
	acc, err := st.LoadAccount("apiuser")
	if err != nil {
		t.Fatal(err)
	}
	if acc.PasswordHash == "" || len(acc.Chars) != 0 {
		t.Fatalf("invalid API account: %+v", acc)
	}
}

func TestForwardedForRequiresTrustedProxy(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/healthz", nil)
	req.RemoteAddr = "127.0.0.1:1234"
	req.Header.Set("X-Forwarded-For", "198.51.100.9")
	if got := clientIP(req, nil); got != "127.0.0.1" {
		t.Fatalf("untrusted header became authoritative: %q", got)
	}
	trusted := []netip.Prefix{netip.MustParsePrefix("127.0.0.0/8")}
	if got := clientIP(req, trusted); got != "198.51.100.9" {
		t.Fatalf("declared proxy was not recognized: %q", got)
	}
}

func TestForwardedForUsesRightmostUntrustedHop(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/healthz", nil)
	req.RemoteAddr = "127.0.0.1:1234"
	req.Header.Set("X-Forwarded-For", "198.51.100.9, 203.0.113.7")
	trusted := []netip.Prefix{
		netip.MustParsePrefix("127.0.0.0/8"),
		netip.MustParsePrefix("203.0.113.0/24"),
	}
	if got := clientIP(req, trusted); got != "198.51.100.9" {
		t.Fatalf("proxy chain selected the wrong hop: %q", got)
	}

	// The leftmost value is client-controlled. An untrusted real proxy hop must
	// take precedence over the forged prefix.
	req.Header.Set("X-Forwarded-For", "192.0.2.66, 198.51.100.20")
	if got := clientIP(req, trusted); got != "198.51.100.20" {
		t.Fatalf("forged XFF bypassed the limiter: %q", got)
	}
}

func TestForwardedForInvalidChainFallsBackToPeer(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/healthz", nil)
	req.RemoteAddr = "127.0.0.1:1234"
	req.Header.Set("X-Forwarded-For", "invalid, 198.51.100.20")
	trusted := []netip.Prefix{netip.MustParsePrefix("127.0.0.0/8")}
	if got := clientIP(req, trusted); got != "127.0.0.1" {
		t.Fatalf("invalid chain did not fall back to peer: %q", got)
	}
}

func TestReadinessReportsDependencyFailure(t *testing.T) {
	h := NewWithConfig(store.NewJSONStore(t.TempDir()), Config{Ready: func(context.Context) error {
		return errors.New("postgres unavailable")
	}})
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, httptest.NewRequest(http.MethodGet, "/readyz", nil))
	if rec.Code != http.StatusServiceUnavailable {
		t.Fatalf("readiness=%d, want 503", rec.Code)
	}
}

func TestCreateAccountAPIRejectsWrongContentType(t *testing.T) {
	h := New(store.NewJSONStore(t.TempDir()))
	req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(`{}`))
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, req)
	if rec.Code != http.StatusUnsupportedMediaType {
		t.Fatalf("status=%d, want 415", rec.Code)
	}
}

func TestInvalidBodyDoesNotReserveOrWaitForHashSlot(t *testing.T) {
	h := &Handler{
		store:     store.NewJSONStore(t.TempDir()),
		limiter:   newRateLimiter(10),
		hashSlots: make(chan struct{}, 1),
	}
	// Simulate all PBKDF2 workers being busy. Malformed JSON must be rejected
	// before this expensive boundary, without returning 503.
	h.hashSlots <- struct{}{}
	req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(`{"username":`))
	req.RemoteAddr = "192.0.2.10:1234"
	rec := httptest.NewRecorder()

	h.createAccount(rec, req)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("invalid body depended on the hash slot: status=%d body=%s",
			rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), `"error":"invalid JSON"`) {
		t.Fatalf("expected English JSON error, got %s", rec.Body.String())
	}
}

func TestCreateAccountAPIEnglishValidation(t *testing.T) {
	h := New(store.NewJSONStore(t.TempDir()))
	for _, tc := range []struct {
		name, body, field, message string
	}{
		{"username", `{"username":"abc","password":"Pass123!","passwordConfirmation":"Pass123!"}`, "username", "use 4 to 12 letters or digits"},
		{"password", `{"username":"ValidUser","password":"abc","passwordConfirmation":"abc"}`, "password", "use 4 to 10 ASCII characters without spaces"},
		{"confirmation", `{"username":"ValidUser","password":"Pass123!","passwordConfirmation":"Other123!"}`, "passwordConfirmation", "passwords do not match"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(tc.body))
			req.Header.Set("Content-Type", "application/json")
			rec := httptest.NewRecorder()
			h.ServeHTTP(rec, req)
			if rec.Code != http.StatusUnprocessableEntity || !strings.Contains(rec.Body.String(), `"field":"`+tc.field+`"`) || !strings.Contains(rec.Body.String(), `"error":"`+tc.message+`"`) {
				t.Fatalf("unexpected validation response: status=%d body=%s", rec.Code, rec.Body.String())
			}
		})
	}
}
