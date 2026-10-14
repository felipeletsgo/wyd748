package playerportal

import (
	"context"
	"errors"
	"fmt"
	"testing"
	"time"
	"wydgo/internal/account"
	"wydgo/internal/authlimit"
)

type limiterFunc func(context.Context, []authlimit.Limit) (time.Duration, error)

func (f limiterFunc) Admit(ctx context.Context, limits []authlimit.Limit) (time.Duration, error) {
	return f(ctx, limits)
}

func TestLoginProtectionFailsBeforePasswordAccess(t *testing.T) {
	for _, scenario := range []string{"missing-token", "rejected", "provider-outage", "missing-verifier", "missing-limiter", "database-outage", "quota"} {
		t.Run(scenario, func(t *testing.T) {
			h, backend := testHandler(t)
			cookie, csrf := bootstrap(t, h)
			token, expected := "token", 503
			switch scenario {
			case "missing-token":
				token, expected = "", 403
			case "rejected":
				expected = 403
				h.cfg.LoginVerifier = verifierFunc(func(context.Context, string, string) error { return errVerificationRejected })
			case "provider-outage":
				h.cfg.LoginVerifier = verifierFunc(func(context.Context, string, string) error { return errors.New("private-secret") })
			case "missing-verifier":
				h.cfg.LoginVerifier = nil
			case "missing-limiter":
				h.cfg.AuthenticationLimiter = nil
			case "database-outage":
				h.cfg.AuthenticationLimiter = limiterFunc(func(context.Context, []authlimit.Limit) (time.Duration, error) {
					return 0, errors.New("private-db-secret")
				})
			case "quota":
				expected = 429
				h.cfg.AuthenticationLimiter = limiterFunc(func(context.Context, []authlimit.Limit) (time.Duration, error) { return 20 * time.Minute, nil })
			}
			w := request(h, "POST", "/api/player/v1/login", fmt.Sprintf(`{"username":"Player1","password":"secret123","turnstileToken":%q}`, token), cookie, csrf)
			if w.Code != expected || backend.authenticationReads != 0 {
				t.Fatalf("protection bypass: %d, reads %d: %s", w.Code, backend.authenticationReads, w.Body.String())
			}
			if scenario == "quota" && w.Header().Get("Retry-After") != "1200" {
				t.Fatal("retry delay missing")
			}
		})
	}
}

func TestNewPasswordPolicyPreservesExistingNativeCredentials(t *testing.T) {
	h, backend := testHandler(t)
	cookie, csrf := bootstrap(t, h)
	w := request(h, "POST", "/api/player/v1/accounts", `{"username":"NewPlayer","password":"short1","passwordConfirmation":"short1","turnstileToken":"token"}`, cookie, csrf)
	if w.Code != 422 || len(backend.accounts) != 0 || backend.registrationChecks != 0 {
		t.Fatalf("weak new password accepted: %d %s", w.Code, w.Body.String())
	}
	if _, err := account.Create(backend, "Existing", "old123", "old123"); err != nil {
		t.Fatal(err)
	}
	w = request(h, "POST", "/api/player/v1/login", `{"username":"Existing","password":"old123","turnstileToken":"token"}`, cookie, csrf)
	if w.Code != 200 {
		t.Fatalf("existing password broken: %d %s", w.Code, w.Body.String())
	}
}
