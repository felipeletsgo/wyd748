package webadmin

import (
	"encoding/json"
	"net/http"
	"strings"
	"sync"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type testRegistration struct {
	mu       sync.Mutex
	accounts map[string]*model.Account
}

func newTestRegistration() *testRegistration {
	return &testRegistration{accounts: make(map[string]*model.Account)}
}

func (s *testRegistration) AccountNameExists(name string) (bool, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	_, exists := s.accounts[strings.ToLower(name)]
	return exists, nil
}

func (s *testRegistration) CreateAccount(value *model.Account) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	key := strings.ToLower(value.Name)
	if _, exists := s.accounts[key]; exists {
		return store.ErrAccountExists
	}
	copy := *value
	copy.Chars = append([]model.Char(nil), value.Chars...)
	s.accounts[key] = &copy
	return nil
}

func anonymousSession(t *testing.T, h *Handler) (*http.Cookie, testSession) {
	t.Helper()
	w := request(h, "GET", "/api/v1/auth/session", "", nil, "")
	s := readTestSession(t, w)
	cookies := w.Result().Cookies()
	if len(cookies) != 1 {
		t.Fatalf("expected session cookie, got %d", len(cookies))
	}
	return cookies[0], s
}

func TestCreateAccount(t *testing.T) {
	h, _, _, logs := newTestHandler(t)
	cookieBox, session := anonymousSession(t, h)
	body := `{"username":"NewPlayer","password":"SafePass9!","passwordConfirmation":"SafePass9!","adminPin":"001234"}`
	w := request(h, "POST", "/api/v1/accounts", body, cookieBox, session.CSRF)
	if w.Code != 201 {
		t.Fatalf("create account: %d %s", w.Code, w.Body.String())
	}
	var response createAccountResponse
	if err := json.Unmarshal(w.Body.Bytes(), &response); err != nil {
		t.Fatal(err)
	}
	if response.Version != 1 || response.Status != "created" || response.Username != "NewPlayer" {
		t.Fatalf("unexpected response: %+v", response)
	}
	registration := h.cfg.Registration.(*testRegistration)
	registration.mu.Lock()
	created := registration.accounts["newplayer"]
	registration.mu.Unlock()
	if created == nil || created.PasswordHash == "" || created.PasswordHash == "SafePass9!" || len(created.Chars) != 0 {
		t.Fatalf("invalid stored account: %+v", created)
	}
	if strings.Contains(logs.String(), "SafePass9!") || strings.Contains(logs.String(), testAdminPIN) {
		t.Fatal("audit leaked a registration secret")
	}

	w = request(h, "POST", "/api/v1/accounts", body, cookieBox, session.CSRF)
	if w.Code != 409 || !strings.Contains(w.Body.String(), "username_unavailable") {
		t.Fatalf("duplicate account: %d %s", w.Code, w.Body.String())
	}
}

func TestCreateAccountRejectsInvalidRequests(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	cookieBox, session := anonymousSession(t, h)
	tests := []struct {
		name string
		body string
		csrf string
		want int
		code string
	}{
		{"missing csrf", `{"username":"player2","password":"SafePass9!","passwordConfirmation":"SafePass9!","adminPin":"001234"}`, "", 403, "invalid_csrf"},
		{"invalid pin", `{"username":"player2","password":"SafePass9!","passwordConfirmation":"SafePass9!","adminPin":"999999"}`, session.CSRF, 401, "invalid_admin_pin"},
		{"password mismatch", `{"username":"player2","password":"SafePass9!","passwordConfirmation":"OtherPas9!","adminPin":"001234"}`, session.CSRF, 422, "validation_error"},
		{"password above native limit", `{"username":"toolongpass","password":"12345678901","passwordConfirmation":"12345678901","adminPin":"001234"}`, session.CSRF, 422, "validation_error"},
		{"unknown field", `{"username":"player2","password":"SafePass9!","passwordConfirmation":"SafePass9!","adminPin":"001234","role":"admin"}`, session.CSRF, 400, "invalid_request"},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			w := request(h, "POST", "/api/v1/accounts", test.body, cookieBox, test.csrf)
			if w.Code != test.want || !strings.Contains(w.Body.String(), test.code) {
				t.Fatalf("got %d %s, want %d %s", w.Code, w.Body.String(), test.want, test.code)
			}
		})
	}
	registration := h.cfg.Registration.(*testRegistration)
	registration.mu.Lock()
	_, created := registration.accounts["toolongpass"]
	registration.mu.Unlock()
	if created {
		t.Fatal("account with password above the native limit was created")
	}
}

func TestCreateAccountReportsUnavailableWriter(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	h.cfg.Registration = nil
	cookieBox, session := anonymousSession(t, h)
	w := request(h, "POST", "/api/v1/accounts", `{}`, cookieBox, session.CSRF)
	if w.Code != 503 || !strings.Contains(w.Body.String(), "registration_unavailable") {
		t.Fatalf("unavailable registration: %d %s", w.Code, w.Body.String())
	}
}

func TestCreateAccountRateLimitsRepeatedUsername(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	cookieBox, session := anonymousSession(t, h)
	body := `{"username":"player2","password":"SafePass9!","passwordConfirmation":"OtherPas9!","adminPin":"001234"}`
	for attempt := 1; attempt <= 3; attempt++ {
		w := request(h, "POST", "/api/v1/accounts", body, cookieBox, session.CSRF)
		if w.Code != http.StatusUnprocessableEntity {
			t.Fatalf("attempt %d: got %d %s", attempt, w.Code, w.Body.String())
		}
	}
	w := request(h, "POST", "/api/v1/accounts", body, cookieBox, session.CSRF)
	if w.Code != http.StatusTooManyRequests || w.Header().Get("Retry-After") != "60" || !strings.Contains(w.Body.String(), "rate_limited") {
		t.Fatalf("rate limit: %d %s, Retry-After=%q", w.Code, w.Body.String(), w.Header().Get("Retry-After"))
	}
}
