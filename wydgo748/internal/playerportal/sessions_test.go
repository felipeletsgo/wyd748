package playerportal

import (
	"crypto/sha256"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"sync"
	"testing"
	"time"
)

func authenticatedSession(t *testing.T, h *Handler, name string) (*http.Cookie, session) {
	t.Helper()
	w := httptest.NewRecorder()
	r := httptest.NewRequest("GET", "https://wyd.example.com", nil)
	r.RemoteAddr = "192.0.2.1:1234"
	s, ok := h.issue(w, r, name)
	if !ok {
		t.Fatal("session capacity reached")
	}
	return w.Result().Cookies()[0], s
}

func TestSessionDirectoryOwnershipRevocationAndPrivacy(t *testing.T) {
	h, _ := testHandler(t)
	one, first := authenticatedSession(t, h, "Player1")
	two, second := authenticatedSession(t, h, "Player1")
	other, outsider := authenticatedSession(t, h, "Other")
	w := request(h, "GET", "/api/player/v1/sessions", "", one, "")
	var response struct{ Sessions []sessionView }
	if err := json.Unmarshal(w.Body.Bytes(), &response); err != nil {
		t.Fatal(err)
	}
	if w.Code != 200 || len(response.Sessions) != 2 || !response.Sessions[0].Current || response.Sessions[0].ID != first.ID {
		t.Fatalf("invalid directory: %d %s", w.Code, w.Body.String())
	}
	for _, secret := range []string{one.Value, two.Value, other.Value, first.CSRF, second.CSRF, outsider.ID, first.Peer, "Other"} {
		if strings.Contains(w.Body.String(), secret) {
			t.Fatal("directory leaked private session data")
		}
	}
	if w.Header().Get("Cache-Control") != "no-store" {
		t.Fatal("private response cached")
	}
	post := func(id, csrf string) *httptest.ResponseRecorder {
		return request(h, "POST", "/api/player/v1/sessions/revoke", `{"id":"`+id+`"}`, one, csrf)
	}
	if w := post(second.ID, "wrong"); w.Code != 403 {
		t.Fatal("CSRF accepted")
	}
	if w := post(outsider.ID, first.CSRF); w.Code != 404 {
		t.Fatal("cross-account revocation allowed")
	}
	if w := post(second.ID, first.CSRF); w.Code != 200 {
		t.Fatal(w.Body.String())
	}
	if w := request(h, "GET", "/api/player/v1/profile", "", two, ""); w.Code != 401 {
		t.Fatal("revoked session still works")
	}
	if w := post(second.ID, first.CSRF); w.Code != 404 {
		t.Fatal("replay changed another session")
	}
	if w := request(h, "GET", "/api/player/v1/profile", "", other, ""); w.Code != 200 {
		t.Fatal("other account revoked")
	}
	if w := request(h, "POST", "/api/player/v1/sessions/revoke-all", `{}`, one, first.CSRF); w.Code != 200 || w.Result().Cookies()[0].MaxAge != -1 {
		t.Fatal("logout all failed", w.Body.String())
	}
	if w := request(h, "GET", "/api/player/v1/sessions", "", one, ""); w.Code != 401 {
		t.Fatal("current session was not revoked")
	}
}

func TestSessionDirectoryExpiryStrictBodiesAndRestart(t *testing.T) {
	h, _ := testHandler(t)
	now := time.Date(2026, 10, 12, 12, 0, 0, 0, time.UTC)
	h.now = func() time.Time { return now }
	one, first := authenticatedSession(t, h, "Player1")
	two, _ := authenticatedSession(t, h, "Player1")
	key := sha256.Sum256([]byte(two.Value))
	s := h.sessions[key]
	s.Idle = now.Add(-31 * time.Minute)
	h.sessions[key] = s
	w := request(h, "GET", "/api/player/v1/sessions", "", one, "")
	var response struct{ Sessions []sessionView }
	_ = json.Unmarshal(w.Body.Bytes(), &response)
	if len(response.Sessions) != 1 || !response.Sessions[0].ExpiresAt.Equal(now.Add(30*time.Minute)) {
		t.Fatal(w.Body.String())
	}
	for _, body := range []string{`{}`, `{"id":"invalid"}`, `{"id":"` + first.ID + `","account":"Other"}`, strings.Repeat("x", 1025), `{ } { }`} {
		if w := request(h, "POST", "/api/player/v1/sessions/revoke", body, one, first.CSRF); w.Code != 400 {
			t.Fatalf("accepted body %q: %d", body, w.Code)
		}
	}
	if w := request(h, "POST", "/api/player/v1/sessions/revoke-all", `{"account":"Other"}`, one, first.CSRF); w.Code != 400 {
		t.Fatal("accepted extra field")
	}
	fresh, _ := testHandler(t)
	if w := request(fresh, "GET", "/api/player/v1/sessions", "", one, ""); w.Code != 401 {
		t.Fatal("session survived process restart")
	}
	anon, csrf := bootstrap(t, h)
	if w := request(h, "POST", "/api/player/v1/sessions/revoke-all", `{}`, anon, csrf); w.Code != 401 {
		t.Fatal("anonymous account mutation")
	}
	if w := request(h, "POST", "/api/player/v1/sessions/revoke", `{"id":"`+first.ID+`"}`, one, first.CSRF); w.Code != 200 || w.Result().Cookies()[0].MaxAge != -1 {
		t.Fatal("self revocation failed")
	}
}

func TestSessionRevocationConcurrentReads(t *testing.T) {
	h, _ := testHandler(t)
	one, first := authenticatedSession(t, h, "Player1")
	two, _ := authenticatedSession(t, h, "Player1")
	var wg sync.WaitGroup
	for i := 0; i < 12; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			w := request(h, "GET", "/api/player/v1/sessions", "", two, "")
			if w.Code != 200 && w.Code != 401 {
				t.Errorf("unexpected read result: %d", w.Code)
			}
		}()
	}
	if w := request(h, "POST", "/api/player/v1/sessions/revoke-all", `{}`, one, first.CSRF); w.Code != 200 {
		t.Fatal(w.Body.String())
	}
	wg.Wait()
	if w := request(h, "GET", "/api/player/v1/sessions", "", two, ""); w.Code != 401 {
		t.Fatal("revoked session restored by concurrent reader")
	}
}
