package playerportal

import (
	"context"
	"errors"
	"fmt"
	"io"
	"net/http"
	"strings"
	"sync/atomic"
	"testing"
	"time"
)

type verifierFunc func(context.Context, string, string) error

func (f verifierFunc) Verify(ctx context.Context, token, peer string) error {
	return f(ctx, token, peer)
}

func TestTurnstileAuthenticationActionsCannotBeInterchanged(t *testing.T) {
	for _, expected := range []string{"login", "staff_login"} {
		for _, actual := range []string{"register", "login", "staff_login"} {
			v, err := NewTurnstileAction("test-secret", "https://wyd.example.com", expected)
			if err != nil {
				t.Fatal(err)
			}
			v.client.Transport = transportFunc(func(*http.Request) (*http.Response, error) {
				return &http.Response{StatusCode: 200, Body: io.NopCloser(strings.NewReader(fmt.Sprintf(`{"success":true,"hostname":"wyd.example.com","action":%q}`, actual)))}, nil
			})
			err = v.Verify(context.Background(), "test-token", "192.0.2.1")
			if expected == actual && err != nil || expected != actual && !errors.Is(err, errVerificationRejected) {
				t.Fatalf("action confusion: %s / %s: %v", expected, actual, err)
			}
		}
	}
	if _, err := NewTurnstileAction("test-secret", "https://wyd.example.com", "unknown"); err == nil {
		t.Fatal("unknown action accepted")
	}
}

type transportFunc func(*http.Request) (*http.Response, error)

func (f transportFunc) RoundTrip(r *http.Request) (*http.Response, error) { return f(r) }

func TestTurnstileConfiguration(t *testing.T) {
	for _, secret := range []string{"", " secret", "secret\n", "1x0000000000000000000000000000000AA", "2x0000000000000000000000000000000AA", "3x0000000000000000000000000000000AA", strings.Repeat("x", 257)} {
		if _, err := NewTurnstile(secret, "https://wyd.example.com"); err == nil {
			t.Fatal("accepted invalid production configuration")
		}
	}
	for _, origin := range []string{"http://wyd.example.com", "http://localhost", "http://192.0.2.1", "https://wyd.example.com/", "https://user@wyd.example.com", "https://wyd.example.com?", "https://wyd.example.com/#x"} {
		if _, err := NewTurnstile("test-secret", origin); err == nil {
			t.Fatal("accepted invalid origin", origin)
		}
	}
	for _, origin := range []string{"http://127.0.0.1:4321", "http://[::1]:4321"} {
		if _, err := NewTurnstile("test-secret", origin); err != nil {
			t.Fatal("literal-loopback development origin rejected", err)
		}
	}
}

func TestTurnstileSiteverifyContract(t *testing.T) {
	for _, tc := range []struct {
		name, body string
		status     int
		want       error
	}{
		{"valid", `{"success":true,"hostname":"wyd.example.com","action":"register"}`, 200, nil},
		{"wrong-host", `{"success":true,"hostname":"other.example.com","action":"register"}`, 200, errVerificationRejected},
		{"wrong-action", `{"success":true,"hostname":"wyd.example.com","action":"login"}`, 200, errVerificationRejected},
		{"missing-binding", `{"success":true}`, 200, errVerificationRejected},
		{"expired-or-replayed", `{"success":false,"error-codes":["timeout-or-duplicate"]}`, 200, errVerificationRejected},
		{"invalid-token", `{"success":false,"error-codes":["invalid-input-response"]}`, 200, errVerificationRejected},
		{"invalid-secret", `{"success":false,"error-codes":["invalid-input-secret"]}`, 200, errVerificationUnavailable},
		{"provider-failure", `{"success":false,"error-codes":["internal-error"]}`, 200, errVerificationUnavailable},
		{"contradictory-success", `{"success":true,"hostname":"wyd.example.com","action":"register","error-codes":["timeout-or-duplicate"]}`, 200, errVerificationRejected},
		{"non-json", `<html>error</html>`, 200, errVerificationUnavailable},
		{"trailing-json", `{"success":true} {}`, 200, errVerificationUnavailable},
		{"oversized-response", strings.Repeat(" ", 8193), 200, errVerificationUnavailable},
		{"http-failure", `{}`, 503, errVerificationUnavailable},
		{"redirect", `{}`, 302, errVerificationUnavailable},
	} {
		t.Run(tc.name, func(t *testing.T) {
			v, err := NewTurnstile("test-secret", "https://wyd.example.com")
			if err != nil {
				t.Fatal(err)
			}
			calls := 0
			v.client.Transport = transportFunc(func(r *http.Request) (*http.Response, error) {
				calls++
				if r.URL.String() != turnstileEndpoint || r.Method != "POST" || r.Header.Get("Content-Type") != "application/x-www-form-urlencoded" {
					t.Fatal("unexpected provider request")
				}
				if err := r.ParseForm(); err != nil {
					t.Fatal(err)
				}
				if r.PostForm.Get("secret") != "test-secret" || r.PostForm.Get("response") != "test-token" || r.PostForm.Get("remoteip") != "192.0.2.1" {
					t.Fatal("incorrect verification form")
				}
				deadline, ok := r.Context().Deadline()
				if !ok || time.Until(deadline) > 3*time.Second {
					t.Fatal("unbounded verification")
				}
				return &http.Response{StatusCode: tc.status, Header: http.Header{"Location": {"https://other.example.com"}}, Body: io.NopCloser(strings.NewReader(tc.body))}, nil
			})
			if err := v.Verify(context.Background(), "test-token", "::ffff:192.0.2.1"); !errors.Is(err, tc.want) {
				t.Fatal("verification result", err, tc.want)
			}
			if calls != 1 {
				t.Fatal("provider retry or redirected secret", calls)
			}
		})
	}
}

func TestTurnstileBoundsCancellationAndSecretRedaction(t *testing.T) {
	v, _ := NewTurnstile("test-secret", "https://wyd.example.com")
	v.client.Transport = transportFunc(func(r *http.Request) (*http.Response, error) {
		return nil, errors.New("test-secret test-token private error")
	})
	if err := v.Verify(context.Background(), "test-token", "192.0.2.1"); err != errVerificationUnavailable || strings.Contains(err.Error(), "test-secret") {
		t.Fatal("private error leaked", err)
	}
	v.client.Transport = transportFunc(func(r *http.Request) (*http.Response, error) {
		t.Fatal("invalid input reached provider")
		return nil, nil
	})
	for _, token := range []string{"", " ", " token", strings.Repeat("x", 2049)} {
		if err := v.Verify(context.Background(), token, "192.0.2.1"); err != errVerificationRejected {
			t.Fatal("invalid token accepted", err)
		}
	}
	if err := v.Verify(context.Background(), "test-token", "unknown"); err != errVerificationRejected {
		t.Fatal("invalid peer accepted", err)
	}
	v.client.Transport = transportFunc(func(r *http.Request) (*http.Response, error) { <-r.Context().Done(); return nil, r.Context().Err() })
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Millisecond)
	defer cancel()
	if err := v.Verify(ctx, "test-token", "192.0.2.1"); err != errVerificationUnavailable {
		t.Fatal("cancellation did not fail closed", err)
	}
}

func TestRegistrationVerificationRejectsBeforePasswordOrDatabase(t *testing.T) {
	for _, tc := range []struct {
		name, token string
		verifier    RegistrationVerifier
		want        int
	}{
		{"unconfigured", "test-token", nil, 503},
		{"missing", "", verifierFunc(func(context.Context, string, string) error { t.Fatal("missing token reached provider"); return nil }), 403},
		{"oversized", strings.Repeat("x", 2049), verifierFunc(func(context.Context, string, string) error { t.Fatal("oversized token reached provider"); return nil }), 403},
		{"rejected", "test-token", verifierFunc(func(context.Context, string, string) error { return errVerificationRejected }), 403},
		{"unavailable", "test-token", verifierFunc(func(context.Context, string, string) error { return errors.New("private-secret") }), 503},
	} {
		t.Run(tc.name, func(t *testing.T) {
			h, b := testHandler(t)
			h.cfg.RegistrationVerifier = tc.verifier
			c, csrf := bootstrap(t, h)
			body := `{"username":"Player1","password":"secret123","passwordConfirmation":"secret123","turnstileToken":"` + tc.token + `"}`
			w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf)
			if w.Code != tc.want || strings.Contains(w.Body.String(), "private-secret") {
				t.Fatal(w.Code, w.Body.String())
			}
			if len(b.accounts) != 0 || b.registrationChecks != 0 || len(h.slots) != 0 || len(h.registrationSlots) != 0 {
				t.Fatal("verification rejection reached account work or leaked capacity")
			}
			if len(h.registrationLimit.buckets) == 0 {
				t.Fatal("verification failure did not count against quota")
			}
		})
	}
}

func TestRegistrationMaximumTokenReplayAndBodyBoundaries(t *testing.T) {
	h, b := testHandler(t)
	var calls atomic.Int32
	h.cfg.RegistrationVerifier = verifierFunc(func(_ context.Context, token, peer string) error {
		if len(token) != 2048 || peer != "192.0.2.1" {
			t.Fatal("incorrect token or trusted peer")
		}
		if calls.Add(1) > 1 {
			return errVerificationRejected
		}
		return nil
	})
	c, csrf := bootstrap(t, h)
	body := `{"username":"Player1","password":"secret123","passwordConfirmation":"secret123","turnstileToken":"` + strings.Repeat("x", 2048) + `"}`
	if w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf); w.Code != 201 {
		t.Fatal(w.Code, w.Body.String())
	}
	if w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf); w.Code != 403 {
		t.Fatal("replayed verification accepted", w.Code)
	}
	if len(b.accounts) != 1 || b.registrationChecks != 1 {
		t.Fatal("replay reached database")
	}
	if w := request(h, "POST", "/api/player/v1/accounts", strings.Repeat(" ", 4097)+body, c, csrf); w.Code != 400 {
		t.Fatal("unbounded registration body", w.Code)
	}
	if w := request(h, "POST", "/api/player/v1/login", body, c, csrf); w.Code != 400 {
		t.Fatal("registration fields/body admitted to login", w.Code)
	}
	if calls.Load() != 2 {
		t.Fatal("body rejection reached provider")
	}
}

func TestRegistrationVerificationConcurrencyLeavesLoginCapacity(t *testing.T) {
	h, _ := testHandler(t)
	c, csrf := bootstrap(t, h)
	entered, release := make(chan struct{}), make(chan struct{})
	h.cfg.RegistrationVerifier = verifierFunc(func(context.Context, string, string) error { close(entered); <-release; return errVerificationRejected })
	done := make(chan int, 1)
	go func() {
		done <- request(h, "POST", "/api/player/v1/accounts", `{"turnstileToken":"test-token"}`, c, csrf).Code
	}()
	select {
	case <-entered:
	case <-time.After(time.Second):
		t.Fatal("verification did not start")
	}
	if len(h.slots) != 0 {
		t.Fatal("external verification occupied password slots")
	}
	if w := request(h, "POST", "/api/player/v1/accounts", `{"turnstileToken":"test-token"}`, c, csrf); w.Code != 503 {
		t.Fatal("unbounded verification concurrency", w.Code)
	}
	close(release)
	if code := <-done; code != 403 {
		t.Fatal(code)
	}
	if len(h.registrationSlots) != 0 {
		t.Fatal("verification slot leaked")
	}
}
