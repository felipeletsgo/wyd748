package playerportal

import (
	"context"
	"crypto/sha256"
	"encoding/json"
	"errors"
	"net/http"
	"net/http/httptest"
	"os"
	"strings"
	"sync"
	"testing"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/authlimit"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

type memoryBackend struct {
	mu                  sync.Mutex
	accounts            map[string]*model.Account
	charStates          map[string]*model.CharState
	profileName         string
	rankingCalls        int
	guildSnapshot       PublicGuildSnapshot
	guildCalls          int
	guildWait           <-chan struct{}
	registrationChecks  int
	authenticationReads int
	fail                bool
}

func (b *memoryBackend) LoadAccount(name string) (*model.Account, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.authenticationReads++
	if b.fail {
		return nil, errors.New("private database details")
	}
	a, ok := b.accounts[strings.ToLower(name)]
	if !ok {
		return nil, os.ErrNotExist
	}
	return a, nil
}
func (b *memoryBackend) AccountNameExists(name string) (bool, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.registrationChecks++
	_, ok := b.accounts[strings.ToLower(name)]
	return ok, nil
}
func (b *memoryBackend) CreateAccount(a *model.Account) error {
	b.mu.Lock()
	defer b.mu.Unlock()
	key := strings.ToLower(a.Name)
	if _, ok := b.accounts[key]; ok {
		return store.ErrAccountExists
	}
	b.accounts[key] = a
	return nil
}
func (b *memoryBackend) Profile(_ context.Context, name string) (Profile, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.profileName = name
	if b.fail {
		return Profile{}, errors.New("private details")
	}
	return Profile{Username: name, Characters: []Character{}}, nil
}
func (b *memoryBackend) Ranking(context.Context) ([]Character, error) {
	b.mu.Lock()
	defer b.mu.Unlock()
	b.rankingCalls++
	if b.fail {
		return nil, errors.New("private details")
	}
	return []Character{{Name: "Knight", Level: 49, Evolution: "mortal"}}, nil
}
func (b *memoryBackend) Ping(context.Context) error {
	b.mu.Lock()
	defer b.mu.Unlock()
	if b.fail {
		return errors.New("private details")
	}
	return nil
}
func testHandler(t *testing.T) (*Handler, *memoryBackend) {
	t.Helper()
	b := &memoryBackend{accounts: make(map[string]*model.Account)}
	verification := verifierFunc(func(context.Context, string, string) error { return nil })
	h, err := New(Config{Origin: "https://wyd.example.com", Backend: b, RegistrationVerifier: verification, LoginVerifier: verification, AuthenticationLimiter: limiterFunc(func(context.Context, []authlimit.Limit) (time.Duration, error) { return 0, nil })})
	if err != nil {
		t.Fatal(err)
	}
	return h, b
}
func request(h *Handler, method, path, body string, cookie *http.Cookie, csrf string) *httptest.ResponseRecorder {
	r := httptest.NewRequest(method, "https://wyd.example.com"+path, strings.NewReader(body))
	r.RemoteAddr = "192.0.2.1:1234"
	if cookie != nil {
		r.AddCookie(cookie)
	}
	r.Header.Set("Origin", "https://wyd.example.com")
	r.Header.Set("Content-Type", "application/json")
	r.Header.Set("X-CSRF-Token", csrf)
	w := httptest.NewRecorder()
	h.ServeHTTP(w, r)
	return w
}
func bootstrap(t *testing.T, h *Handler) (*http.Cookie, string) {
	t.Helper()
	w := request(h, "GET", "/api/player/v1/session", "", nil, "")
	if w.Code != 200 {
		t.Fatal(w.Code, w.Body.String())
	}
	var data struct{ CSRF string }
	if err := json.Unmarshal(w.Body.Bytes(), &data); err != nil {
		t.Fatal(err)
	}
	return w.Result().Cookies()[0], data.CSRF
}
func TestOriginAndPostBoundaries(t *testing.T) {
	h, b := testHandler(t)
	for _, origin := range []string{"http://example.com", "https://example.com/", "https://example.com?", "https://user@example.com", "https://example.com/#x"} {
		if _, err := New(Config{Origin: origin, Backend: b}); err == nil {
			t.Errorf("accepted %q", origin)
		}
	}
	if _, err := New(Config{Origin: "http://127.0.0.1:4321", DevHTTP: true, Backend: b}); err != nil {
		t.Fatal(err)
	}
	if _, err := New(Config{Origin: "http://localhost:4321", DevHTTP: true, Backend: b}); err == nil {
		t.Fatal("accepted nonliteral development host")
	}
	c, csrf := bootstrap(t, h)
	if !c.Secure || !c.HttpOnly || c.SameSite != http.SameSiteStrictMode || c.Path != "/" || !strings.HasPrefix(c.Name, "__Host-") {
		t.Fatal("unsafe cookie", c)
	}
	for _, tc := range []struct {
		host, origin, content, token string
		status                       int
	}{
		{"evil.example", "https://wyd.example.com", "application/json", csrf, 403},
		{"wyd.example.com", "https://evil.example", "application/json", csrf, 403},
		{"wyd.example.com", "https://wyd.example.com", "text/plain", csrf, 415},
		{"wyd.example.com", "https://wyd.example.com", "application/json", "bad", 403},
	} {
		r := httptest.NewRequest("POST", "https://wyd.example.com/api/player/v1/login", strings.NewReader(`{}`))
		r.Host = tc.host
		r.RemoteAddr = "192.0.2.1:1234"
		r.AddCookie(c)
		r.Header.Set("Origin", tc.origin)
		r.Header.Set("Content-Type", tc.content)
		r.Header.Set("X-CSRF-Token", tc.token)
		w := httptest.NewRecorder()
		h.ServeHTTP(w, r)
		if w.Code != tc.status {
			t.Errorf("got %d want %d", w.Code, tc.status)
		}
	}
	for _, body := range []string{`{"extra":true}`, `{} {}`, strings.Repeat("x", 1025)} {
		if w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf); w.Code != 400 {
			t.Errorf("bad body: %d", w.Code)
		}
	}
}
func TestRegistrationLoginOwnershipRotationAndLogout(t *testing.T) {
	h, b := testHandler(t)
	c, csrf := bootstrap(t, h)
	body := `{"username":"Player1","password":"secret123","passwordConfirmation":"secret123","turnstileToken":"test-token"}`
	if w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf); w.Code != 201 {
		t.Fatal(w.Code, w.Body.String())
	}
	a := b.accounts["player1"]
	if a.PasswordHash == "secret123" || a.PasswordHash == "" || len(a.Chars) != 0 || a.Donate != 0 {
		t.Fatal("unsafe registration")
	}
	if ok, err := account.VerifyPassword(a.PasswordHash, "secret123"); err != nil || !ok {
		t.Fatal("incompatible game credentials", err)
	}
	if w := request(h, "POST", "/api/player/v1/accounts", body, c, csrf); w.Code != 409 {
		t.Fatal(w.Code)
	}
	if w := request(h, "GET", "/api/player/v1/profile", "", c, ""); w.Code != 401 {
		t.Fatal("anonymous profile", w.Code)
	}
	w := request(h, "POST", "/api/player/v1/login", `{"username":"Player1","password":"secret123","turnstileToken":"test-token"}`, c, csrf)
	if w.Code != 200 {
		t.Fatal(w.Code, w.Body.String())
	}
	auth := w.Result().Cookies()[0]
	var sessionData struct{ CSRF string }
	_ = json.Unmarshal(w.Body.Bytes(), &sessionData)
	if auth.Value == c.Value || sessionData.CSRF == csrf {
		t.Fatal("session fixation")
	}
	if w := request(h, "GET", "/api/player/v1/profile?username=OtherAccount", "", auth, ""); w.Code != 200 || b.profileName != "Player1" || strings.Contains(w.Body.String(), "passwordHash") {
		t.Fatal("ownership/projection breach", w.Body.String())
	}
	if w := request(h, "POST", "/api/player/v1/logout", `{}`, c, csrf); w.Code != 403 {
		t.Fatal("old session remains valid")
	}
	if w := request(h, "POST", "/api/player/v1/logout", `{}`, auth, sessionData.CSRF); w.Code != 200 || w.Result().Cookies()[0].MaxAge != -1 {
		t.Fatal("logout", w.Code)
	}
	if w := request(h, "GET", "/api/player/v1/profile", "", auth, ""); w.Code != 401 {
		t.Fatal("logout did not revoke")
	}
}
func TestAnonymousBindingExpiryAndSessionCaps(t *testing.T) {
	h, _ := testHandler(t)
	now := time.Now()
	h.now = func() time.Time { return now }
	c, _ := bootstrap(t, h)
	r := httptest.NewRequest("GET", "https://wyd.example.com/api/player/v1/session", nil)
	r.AddCookie(c)
	r.RemoteAddr = "192.0.2.2:1"
	if _, _, ok := h.current(r); ok {
		t.Fatal("anonymous session transferred")
	}
	c, _ = bootstrap(t, h)
	now = now.Add(21 * time.Minute)
	if w := request(h, "GET", "/api/player/v1/profile", "", c, ""); w.Code != 401 {
		t.Fatal("expired session")
	}
	for i := 0; i < 4; i++ {
		bootstrap(t, h)
	}
	if w := request(h, "GET", "/api/player/v1/session", "", nil, ""); w.Code != 503 {
		t.Fatal("anonymous capacity", w.Code)
	}
}
func TestProxyCacheFailuresAndConcurrency(t *testing.T) {
	h, b := testHandler(t)
	r := httptest.NewRequest("GET", "https://wyd.example.com", nil)
	r.RemoteAddr = "192.0.2.1:42"
	r.Header.Set("X-Forwarded-For", "203.0.113.9")
	h.cfg.TrustLoopbackProxy = true
	if h.peer(r) != "192.0.2.1" {
		t.Fatal("trusted public spoof")
	}
	r.RemoteAddr = "127.0.0.1:42"
	if h.peer(r) != "203.0.113.9" {
		t.Fatal("proxy missing")
	}
	r.Header.Set("X-Forwarded-For", "203.0.113.9, 1.1.1.1")
	if h.peer(r) != "127.0.0.1" {
		t.Fatal("trusted chain")
	}
	var wg sync.WaitGroup
	for i := 0; i < 20; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			if w := request(h, "GET", "/api/player/v1/ranking", "", nil, ""); w.Code != 200 {
				t.Errorf("ranking: %d", w.Code)
			}
		}()
	}
	wg.Wait()
	if b.rankingCalls != 1 {
		t.Fatal("cache stampede", b.rankingCalls)
	}
	b.fail = true
	h.rankAt = time.Time{}
	for _, path := range []string{"/api/player/v1/ranking", "/readyz"} {
		w := request(h, "GET", path, "", nil, "")
		if w.Code != 503 || strings.Contains(w.Body.String(), "private") {
			t.Fatal("failure not contained", w.Body.String())
		}
	}
	for i := 0; i < 8; i++ {
		if !h.allow("limit", 8) {
			t.Fatal("early rate limit")
		}
	}
	if h.allow("limit", 8) {
		t.Fatal("rate limit missing")
	}
	for i := 0; i < 4096; i++ {
		h.rates[string(rune(i))+"bounded"] = rate{Start: h.now()}
	}
	if h.allow("new key", 1) {
		t.Fatal("unbounded limiter")
	}
	// Tokens are indexed by their digest, not retained as browser cookie values.
	h, _ = testHandler(t)
	c, _ := bootstrap(t, h)
	if _, ok := h.sessions[sha256.Sum256([]byte(c.Value))]; !ok {
		t.Fatal("session digest missing")
	}
}
func TestRegistrationPayloadRejectsGameState(t *testing.T) {
	a := &model.Account{Name: "Player1", PasswordHash: "hash", Chars: []model.Char{}}
	data, err := registrationPayload(a)
	if err != nil {
		t.Fatal(err)
	}
	var restored model.Account
	if err := json.Unmarshal(data, &restored); err != nil {
		t.Fatal(err)
	}
	if len(restored.Cargo) != 128 {
		t.Fatal("native cargo shape")
	}
	for _, mutate := range []func(*model.Account){func(a *model.Account) { a.Donate = 1 }, func(a *model.Account) { a.CargoGold = 1 }, func(a *model.Account) { a.Cargo[0].Index = 1 }, func(a *model.Account) { a.Chars = []model.Char{{Name: "Injected"}} }} {
		copy := *a
		mutate(&copy)
		if _, err := registrationPayload(&copy); err == nil {
			t.Fatal("accepted state injection")
		}
	}
}
