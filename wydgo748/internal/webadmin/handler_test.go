package webadmin

import (
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/json"
	"fmt"
	"log/slog"
	"net/http"
	"net/http/httptest"
	"net/netip"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/control"
	"wydgo/internal/model"
)

const testAdminPIN = "001234"

type testAccounts struct{ hash string }

func (s testAccounts) LoadAccount(name string) (*model.Account, error) {
	if name != "operator" && name != "player" {
		return nil, os.ErrNotExist
	}
	return &model.Account{Name: name, PasswordHash: s.hash}, nil
}

type testControl struct {
	actor string
	err   error
}

func (c *testControl) Overview(_ context.Context, actor string, q control.Query) (control.Overview, error) {
	c.actor = actor
	return control.Overview{Version: 1, AsOf: time.Now().UTC(), State: "running", Online: 2, Authenticated: 3, Mobs: 128, Players: []control.Player{{UID: "fixture-1", Name: "Sentinela", Account: "fixture", Level: 120, Class: 1, X: 2100, Y: 2100}}, Matched: 1, Offset: q.Offset}, c.err
}

func (c *testControl) AccountPresence(_ context.Context, actor string, accounts []string) (map[string]bool, error) {
	c.actor = actor
	result := make(map[string]bool, len(accounts))
	for _, accountName := range accounts {
		result[strings.ToLower(accountName)] = strings.EqualFold(accountName, "fixture")
	}
	return result, c.err
}

type testPersistent struct {
	account      string
	uid          string
	value        PersistentPlayer
	err          error
	directory    AccountDirectory
	directoryErr error
	search       string
	cursor       string
	limit        int
}

func (p *testPersistent) Accounts(_ context.Context, search, cursor string, limit int) (AccountDirectory, error) {
	p.search, p.cursor, p.limit = search, cursor, limit
	return p.directory, p.directoryErr
}

func (p *testPersistent) Player(_ context.Context, accountName, uid string) (PersistentPlayer, error) {
	p.account = accountName
	p.uid = uid
	return p.value, p.err
}

type testSession struct {
	Authenticated bool   `json:"authenticated"`
	CSRF          string `json:"csrf"`
	Account       string `json:"account"`
}

func request(h http.Handler, method, path, body string, cookie *http.Cookie, csrf string) *httptest.ResponseRecorder {
	r := httptest.NewRequest(method, "https://admin.test"+path, strings.NewReader(body))
	r.RemoteAddr = "127.0.0.1:1234"
	r.Header.Set("Origin", "https://admin.test")
	r.Header.Set("Content-Type", "application/json")
	r.Header.Set("X-CSRF-Token", csrf)
	if cookie != nil {
		r.AddCookie(cookie)
	}
	w := httptest.NewRecorder()
	h.ServeHTTP(w, r)
	return w
}
func readTestSession(t *testing.T, w *httptest.ResponseRecorder) testSession {
	t.Helper()
	if w.Code != 200 {
		t.Fatalf("session: %d %s", w.Code, w.Body.String())
	}
	var s testSession
	if err := json.Unmarshal(w.Body.Bytes(), &s); err != nil {
		t.Fatal(err)
	}
	return s
}
func newTestHandler(t *testing.T) (*Handler, map[string]Staff, *testControl, *bytes.Buffer) {
	t.Helper()
	hash, err := account.HashPassword("TestPass10")
	if err != nil {
		t.Fatal(err)
	}
	entries := map[string]Staff{"operator": {Capabilities: []string{StatusCapability, PlayersCapability}}}
	c := &testControl{}
	now := time.Now().UTC()
	p := &testPersistent{
		value: PersistentPlayer{Version: 1, AsOf: now, Account: "fixture", UID: "fixture-1", Name: "Sentinela", Level: 120},
		directory: AccountDirectory{Version: 1, AsOf: now, Accounts: []AccountSummary{{
			Username: "fixture", CreatedAt: now.Add(-time.Hour), UpdatedAt: now,
			Characters: []AccountCharacterSummary{{UID: "fixture-1", Slot: 0, Name: "Sentinela", Class: 1, Level: 120}},
		}}},
	}
	logs := &bytes.Buffer{}
	h, err := New(Config{Origin: "https://admin.test", AdminAccessPIN: testAdminPIN, Accounts: testAccounts{hash}, Registration: newTestRegistration(), Staff: func() (map[string]Staff, error) { return entries, nil }, Control: c, Persistent: p, Audit: slog.New(slog.NewJSONHandler(logs, nil))})
	if err != nil {
		t.Fatal(err)
	}
	return h, entries, c, logs
}

func TestAccountDirectoryEndpoint(t *testing.T) {
	h, entries, controlSource, logs := newTestHandler(t)
	p := h.cfg.Persistent.(*testPersistent)
	cookie, _, _ := loginTest(t, h)
	w := request(h, "GET", "/api/v1/staff/accounts?search=FIX&limit=10", "", cookie, "")
	if w.Code != 200 || p.search != "fix" || p.cursor != "" || p.limit != 10 || controlSource.actor != "operator" {
		t.Fatalf("account directory: %d %s", w.Code, w.Body.String())
	}
	var got AccountDirectory
	if err := json.Unmarshal(w.Body.Bytes(), &got); err != nil {
		t.Fatal(err)
	}
	if got.Version != 1 || !got.PresenceAvailable || len(got.Accounts) != 1 || !got.Accounts[0].Online || got.Accounts[0].Username != "fixture" {
		t.Fatalf("unexpected directory: %+v", got)
	}
	if strings.Contains(strings.ToLower(w.Body.String()), "password") || !strings.Contains(logs.String(), `"action":"account_directory"`) {
		t.Fatal("directory exposed credentials or was not audited")
	}
	for _, path := range []string{
		"/api/v1/staff/accounts?search=inv%C3%A1lido",
		"/api/v1/staff/accounts?limit=51",
		"/api/v1/staff/accounts?search=a&search=b",
		"/api/v1/staff/accounts?unknown=value",
	} {
		w = request(h, "GET", path, "", cookie, "")
		if w.Code != 400 || !strings.Contains(w.Body.String(), "invalid_query") {
			t.Fatalf("invalid directory query accepted: %s: %d %s", path, w.Code, w.Body.String())
		}
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability}}
	w = request(h, "GET", "/api/v1/staff/accounts", "", cookie, "")
	if w.Code != 403 {
		t.Fatalf("directory without capability: %d %s", w.Code, w.Body.String())
	}
}

func TestAccountDirectorySurvivesUnavailablePresence(t *testing.T) {
	h, _, controlSource, _ := newTestHandler(t)
	cookie, _, _ := loginTest(t, h)
	controlSource.err = fmt.Errorf("world busy")
	w := request(h, "GET", "/api/v1/staff/accounts", "", cookie, "")
	if w.Code != 200 {
		t.Fatalf("account directory: %d %s", w.Code, w.Body.String())
	}
	var got AccountDirectory
	if err := json.Unmarshal(w.Body.Bytes(), &got); err != nil {
		t.Fatal(err)
	}
	if got.PresenceAvailable || len(got.Accounts) != 1 || got.Accounts[0].Online {
		t.Fatalf("unexpected directory without presence: %+v", got)
	}
}

func TestPersistentPlayerEndpoint(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	p := h.cfg.Persistent.(*testPersistent)
	cookie, _, _ := loginTest(t, h)
	w := request(h, "GET", "/api/v1/staff/players/fixture-1/persistent?account=fixture", "", cookie, "")
	if w.Code != 200 || p.account != "fixture" || p.uid != "fixture-1" {
		t.Fatalf("persistent player: %d %s", w.Code, w.Body.String())
	}
	var got PersistentPlayer
	if err := json.Unmarshal(w.Body.Bytes(), &got); err != nil {
		t.Fatal(err)
	}
	if got.Version != 1 || got.Account != "fixture" || got.UID != "fixture-1" || got.Name != "Sentinela" {
		t.Fatalf("unexpected projection: %+v", got)
	}

	for _, path := range []string{
		"/api/v1/staff/players/fixture-1/persistent",
		"/api/v1/staff/players/fixture-1/persistent?account=fixture&account=other",
	} {
		w = request(h, "GET", path, "", cookie, "")
		if w.Code != 400 || !strings.Contains(w.Body.String(), "invalid_target") {
			t.Fatalf("invalid target accepted: %d %s", w.Code, w.Body.String())
		}
	}

	p.err = ErrPersistentPlayerNotFound
	w = request(h, "GET", "/api/v1/staff/players/missing/persistent?account=fixture", "", cookie, "")
	if w.Code != 404 || !strings.Contains(w.Body.String(), "persistent_not_found") {
		t.Fatalf("missing persistent state: %d %s", w.Code, w.Body.String())
	}
	p.err = context.DeadlineExceeded
	w = request(h, "GET", "/api/v1/staff/players/fixture-1/persistent?account=fixture", "", cookie, "")
	if w.Code != 503 || !strings.Contains(w.Body.String(), "persistent_unavailable") {
		t.Fatalf("persistent failure: %d %s", w.Code, w.Body.String())
	}
	if !strings.Contains(logs.String(), `"action":"persistent_player"`) {
		t.Fatal("persistent reads were not audited")
	}
	if !strings.Contains(logs.String(), `"target_account":"fixture"`) || !strings.Contains(logs.String(), `"target_uid":"fixture-1"`) || !strings.Contains(logs.String(), `"request_id":`) || !strings.Contains(logs.String(), `"client_ip":"127.0.0.1"`) {
		t.Fatal("persistent audit is missing target/request metadata")
	}

	entries["operator"] = Staff{Capabilities: []string{StatusCapability}}
	w = request(h, "GET", "/api/v1/staff/players/fixture-1/persistent?account=fixture", "", cookie, "")
	if w.Code != 403 {
		t.Fatalf("staff capability change should revoke session: %d", w.Code)
	}
}

func TestAnonymousSessionsCannotExhaustAuthenticatedCapacity(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	for i := 0; i < 20; i++ {
		w := request(h, "GET", "/api/v1/auth/session", "", nil, "")
		if w.Code != 200 {
			t.Fatalf("anonymous session %d: %d %s", i, w.Code, w.Body.String())
		}
	}
	h.mu.Lock()
	anonymous := 0
	for _, s := range h.sessions {
		if s.Actor == "" {
			anonymous++
		}
	}
	h.mu.Unlock()
	if anonymous > maxAnonymousPerPeer {
		t.Fatalf("same peer retained %d anonymous sessions; max %d", anonymous, maxAnonymousPerPeer)
	}

	now := h.now()
	h.mu.Lock()
	h.sessions = make(map[[32]byte]session, maxSessions)
	for i := 0; i < maxSessions; i++ {
		key := sha256.Sum256([]byte(fmt.Sprintf("anon-%d", i)))
		h.sessions[key] = session{Peer: fmt.Sprintf("192.0.2.%d", i%200+1), LastSeen: now.Add(time.Duration(i) * time.Millisecond), Expires: now.Add(5 * time.Minute)}
	}
	h.mu.Unlock()
	r := httptest.NewRequest("POST", "https://admin.test/api/v1/auth/login", nil)
	r.RemoteAddr = "127.0.0.1:1234"
	w := httptest.NewRecorder()
	if !h.issue(w, r, [32]byte{}, session{Actor: "operator"}) {
		t.Fatal("anonymous sessions blocked an authenticated session")
	}
	h.mu.Lock()
	defer h.mu.Unlock()
	if len(h.sessions) != maxSessions {
		t.Fatalf("unexpected session count after authenticated admission: %d", len(h.sessions))
	}
}

func TestLoginRateLimitDoesNotBurnGlobalBudgetOnNarrowRejection(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	for i := 0; i < 10; i++ {
		if !h.allowAll(
			rateLimit{key: "login:ip:192.0.2.1", limit: 10},
			rateLimit{key: fmt.Sprintf("login:account:user%d", i), limit: 5},
			rateLimit{key: "login:global", limit: 60},
		) {
			t.Fatalf("attempt %d unexpectedly rejected", i)
		}
	}
	if h.allowAll(
		rateLimit{key: "login:ip:192.0.2.1", limit: 10},
		rateLimit{key: "login:account:other", limit: 5},
		rateLimit{key: "login:global", limit: 60},
	) {
		t.Fatal("per-IP limit did not reject attempt")
	}
	if got := h.rates["login:global"].count; got != 10 {
		t.Fatalf("rejected attempt consumed global budget: %d", got)
	}
	if !h.allowAll(
		rateLimit{key: "login:ip:198.51.100.1", limit: 10},
		rateLimit{key: "login:account:other", limit: 5},
		rateLimit{key: "login:global", limit: 60},
	) {
		t.Fatal("different source was blocked by rejected source")
	}
}

func TestPeerUsesForwardedChainOnlyFromTrustedProxy(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	r := httptest.NewRequest("GET", "https://admin.test/api/v1/auth/session", nil)
	r.RemoteAddr = "127.0.0.1:4321"
	r.Header.Set("X-Forwarded-For", "203.0.113.9")
	if got := h.peer(r); got != "127.0.0.1" {
		t.Fatalf("untrusted forwarding header used: %s", got)
	}
	h.trustedProxies = []netip.Prefix{netip.MustParsePrefix("127.0.0.1/32")}
	if got := h.peer(r); got != "203.0.113.9" {
		t.Fatalf("trusted proxy client IP: %s", got)
	}
	r.Header.Set("X-Forwarded-For", "203.0.113.9, 198.51.100.7")
	if got := h.peer(r); got != "198.51.100.7" {
		t.Fatalf("untrusted intermediate proxy should terminate chain: %s", got)
	}
	h.trustedProxies = append(h.trustedProxies, netip.MustParsePrefix("198.51.100.0/24"))
	if got := h.peer(r); got != "203.0.113.9" {
		t.Fatalf("trusted proxy chain did not resolve client: %s", got)
	}
}
func loginTest(t *testing.T, h *Handler) (*http.Cookie, testSession, *http.Cookie) {
	t.Helper()
	w := request(h, "GET", "/api/v1/auth/session", "", nil, "")
	s := readTestSession(t, w)
	old := w.Result().Cookies()[0]
	w = request(h, "POST", "/api/v1/auth/login", `{"username":"operator","password":"TestPass10","adminPin":"`+testAdminPIN+`"}`, old, s.CSRF)
	s = readTestSession(t, w)
	return w.Result().Cookies()[0], s, old
}
func TestSessionSecurityAndRevocation(t *testing.T) {
	h, entries, c, logs := newTestHandler(t)
	cookie, s, old := loginTest(t, h)
	if !s.Authenticated || s.Account != "operator" || !cookie.Secure || !cookie.HttpOnly || cookie.SameSite != http.SameSiteLaxMode || cookie.Name != "__Host-wyd_session" || cookie.Value == old.Value {
		t.Fatal("session/cookie contract")
	}
	w := request(h, "GET", "/api/v1/staff/overview", "", old, "")
	if w.Code != 401 {
		t.Fatal("session fixation")
	}
	w = request(h, "GET", "/api/v1/staff/overview", "", cookie, "")
	if w.Code != 200 || c.actor != "operator" || w.Header().Get("Cache-Control") != "no-store" {
		t.Fatalf("overview: %d %s", w.Code, w.Body)
	}
	w = request(h, "POST", "/api/v1/auth/logout", "{}", cookie, "")
	if w.Code != 403 {
		t.Fatal("missing CSRF allowed")
	}
	delete(entries, "operator")
	w = request(h, "GET", "/api/v1/staff/overview", "", cookie, "")
	if w.Code != 403 {
		t.Fatal("revocation failed")
	}
	if strings.Contains(logs.String(), "TestPass10") || strings.Contains(logs.String(), testAdminPIN) || strings.Contains(logs.String(), cookie.Value) {
		t.Fatal("secrets in audit")
	}
}
func TestLogoutAndReloginWithAdminPIN(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	cookie, s, _ := loginTest(t, h)
	w := request(h, "POST", "/api/v1/auth/logout", "{}", cookie, s.CSRF)
	if w.Code != 200 || w.Result().Cookies()[0].MaxAge != -1 {
		t.Fatal("logout")
	}
	w = request(h, "GET", "/api/v1/staff/overview", "", cookie, "")
	if w.Code != 401 {
		t.Fatal("logged out session accepted")
	}
	w = request(h, "GET", "/api/v1/auth/session", "", nil, "")
	anon := readTestSession(t, w)
	w = request(h, "POST", "/api/v1/auth/login", `{"username":"operator","password":"TestPass10","adminPin":"`+testAdminPIN+`"}`, w.Result().Cookies()[0], anon.CSRF)
	if w.Code != 200 {
		t.Fatalf("re-login with configured PIN failed: %d %s", w.Code, w.Body.String())
	}
}
func TestAuthRejectionsAndExpiry(t *testing.T) {
	h, _, c, _ := newTestHandler(t)
	for _, tc := range []struct{ host, origin string }{{"evil.test", "https://admin.test"}, {"admin.test", "https://evil.test"}} {
		r := httptest.NewRequest("POST", "https://"+tc.host+"/api/v1/auth/login", nil)
		r.Header.Set("Origin", tc.origin)
		w := httptest.NewRecorder()
		h.ServeHTTP(w, r)
		if w.Code != 403 {
			t.Fatal("cross origin/host allowed")
		}
	}
	w := request(h, "GET", "/api/v1/auth/session", "", nil, "")
	s := readTestSession(t, w)
	cookie := w.Result().Cookies()[0]
	for _, body := range []string{`{"extra":true}`, `{} {}`, strings.Repeat(" ", 1025) + `{}`} {
		w = request(h, "POST", "/api/v1/auth/login", body, cookie, s.CSRF)
		if w.Code != 400 {
			t.Fatalf("malformed accepted: %d", w.Code)
		}
	}
	w = request(h, "POST", "/api/v1/auth/login", `{"username":"operator","password":"wrong","adminPin":"001234"}`, cookie, s.CSRF)
	if w.Code != 401 {
		t.Fatal("password rejection")
	}
	for _, body := range []string{
		`{"username":"operator","password":"12345678901","adminPin":"001234"}`,
		`{"username":"operator","password":"TestPass10","adminPin":"999999"}`,
		`{"username":"operator","password":"TestPass10","adminPin":"12345"}`,
		`{"username":"operator","password":"TestPass10","adminPin":"12a456"}`,
		`{"username":"player","password":"TestPass10","adminPin":"001234"}`,
	} {
		w = request(h, "POST", "/api/v1/auth/login", body, cookie, s.CSRF)
		if w.Code != 401 || !strings.Contains(w.Body.String(), "invalid_credentials") {
			t.Fatalf("invalid login accepted: %d %s", w.Code, w.Body.String())
		}
	}
	cookie, _, _ = loginTest(t, h)
	c.err = context.DeadlineExceeded
	w = request(h, "GET", "/api/v1/staff/overview", "", cookie, "")
	if w.Code != 503 || !strings.Contains(w.Body.String(), "server_unavailable") {
		t.Fatal("failure contract")
	}
	future := time.Now().Add(16 * time.Minute)
	h.now = func() time.Time { return future }
	w = request(h, "GET", "/api/v1/staff/overview", "", cookie, "")
	if w.Code != 401 {
		t.Fatal("idle expiry")
	}
}
func TestAdminPINAndStaffValidation(t *testing.T) {
	for _, pin := range []string{"", "12345", "1234567890123", "12a456"} {
		if validAdminPIN(pin) {
			t.Fatalf("invalid PIN accepted: %q", pin)
		}
	}
	if !validAdminPIN("001234") || !validAdminPIN("123456789012") {
		t.Fatal("valid numeric PIN rejected")
	}
	path := filepath.Join(t.TempDir(), "staff.json")
	for _, body := range []string{`{}`, `{"Operator":{}}`, `{"operator":{"totpSecret":"legacy","capabilities":["server.status"]}}`, `{"operator":{"capabilities":["admin.*"]}}`} {
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
		if _, err := ReadStaff(path); err == nil {
			t.Fatal("invalid staff accepted")
		}
	}
	body := `{"operator":{"capabilities":["server.status","moderation.player.search"]}}`
	if err := os.WriteFile(path, []byte(body), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := ReadStaff(path); err != nil {
		t.Fatal(err)
	}
}
