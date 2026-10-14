package webadmin

import (
	"context"
	"errors"
	"fmt"
	"net/http"
	"strings"
	"testing"
	"time"
	"wydgo/internal/authlimit"
)

type securityLimiter func(context.Context, []authlimit.Limit) (time.Duration, error)

func (f securityLimiter) Admit(ctx context.Context, limits []authlimit.Limit) (time.Duration, error) {
	return f(ctx, limits)
}

func TestPublishedRegistrationCannotBypassProtection(t *testing.T) {
	for _, scenario := range []string{"anonymous", "revoked", "quota", "storage-outage", "short-password"} {
		t.Run(scenario, func(t *testing.T) {
			h, entries, _, _ := newTestHandler(t)
			cookie, session := anonymousSession(t, h)
			if scenario != "anonymous" {
				cookie, session, _ = loginTest(t, h)
			}
			expected, password := http.StatusUnauthorized, "SafePass9!"
			switch scenario {
			case "revoked":
				delete(entries, "operator")
				expected = 403
			case "quota":
				h.cfg.AuthenticationLimiter = securityLimiter(func(context.Context, []authlimit.Limit) (time.Duration, error) { return time.Hour, nil })
				expected = 429
			case "storage-outage":
				h.cfg.AuthenticationLimiter = nil
				expected = 503
			case "short-password":
				password, expected = "short", 422
			}
			body := fmt.Sprintf(`{"username":"NewPlayer","password":%q,"passwordConfirmation":%q,"adminPin":%q}`, password, password, testAdminPIN)
			w := request(h, "POST", "/api/v1/accounts", body, cookie, session.CSRF)
			if w.Code != expected || len(h.cfg.Registration.(*testRegistration).accounts) != 0 {
				t.Fatalf("registration bypass: %d %s", w.Code, w.Body.String())
			}
		})
	}
}

type securityVerifier func(context.Context, string, string) error

func (f securityVerifier) Verify(ctx context.Context, token, peer string) error {
	return f(ctx, token, peer)
}

func TestPublishedStaffLoginProtection(t *testing.T) {
	for _, scenario := range []string{"accepted", "missing-token", "rejected-token", "missing-limiter", "missing-verifier", "storage-outage", "quota"} {
		t.Run(scenario, func(t *testing.T) {
			h, _, _, logs := newTestHandler(t)
			// HTTPS must require protection even when the opt-in flag is omitted.
			h.cfg.RequireLoginProtection = false
			h.cfg.AuthenticationLimiter = securityLimiter(func(context.Context, []authlimit.Limit) (time.Duration, error) { return 0, nil })
			h.cfg.LoginVerifier = securityVerifier(func(context.Context, string, string) error { return nil })
			session := request(h, "GET", "/api/v1/auth/session", "", nil, "")
			csrf := readTestSession(t, session).CSRF
			cookie := session.Result().Cookies()[0]
			token, expected := "test-token", 200
			switch scenario {
			case "missing-token":
				token, expected = "", 403
			case "rejected-token":
				expected = 403
				h.cfg.LoginVerifier = securityVerifier(func(context.Context, string, string) error { return errors.New("private-token-secret") })
			case "missing-limiter":
				expected = 503
				h.cfg.AuthenticationLimiter = nil
			case "missing-verifier":
				expected = 503
				h.cfg.LoginVerifier = nil
			case "storage-outage":
				expected = 503
				h.cfg.AuthenticationLimiter = securityLimiter(func(context.Context, []authlimit.Limit) (time.Duration, error) {
					return 0, errors.New("private-db-secret")
				})
			case "quota":
				expected = 429
				h.cfg.AuthenticationLimiter = securityLimiter(func(context.Context, []authlimit.Limit) (time.Duration, error) { return time.Hour, nil })
			}
			body := fmt.Sprintf(`{"username":"operator","password":"TestPass10","adminPin":%q,"turnstileToken":%q}`, testAdminPIN, token)
			w := request(h, "POST", "/api/v1/auth/login", body, cookie, csrf)
			if w.Code != expected || strings.Contains(w.Body.String(), "private-") || strings.Contains(logs.String(), token) && token != "" {
				t.Fatalf("staff protection failed: %d %s", w.Code, w.Body.String())
			}
		})
	}
}
