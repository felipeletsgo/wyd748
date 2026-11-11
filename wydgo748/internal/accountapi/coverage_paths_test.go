package accountapi

import (
	"context"
	"errors"
	"fmt"
	"net/http"
	"net/http/httptest"
	"net/netip"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type scriptedRegistrationStore struct {
	exists    bool
	existsErr error
	createErr error
	created   *model.Account
}

func (s *scriptedRegistrationStore) AccountNameExists(string) (bool, error) {
	return s.exists, s.existsErr
}

func (s *scriptedRegistrationStore) CreateAccount(acc *model.Account) error {
	if s.createErr != nil {
		return s.createErr
	}
	s.created = acc
	return nil
}

func TestReadinessReturnsReadyWhenDependencySucceeds(t *testing.T) {
	called := false
	h := NewWithConfig(nil, Config{Ready: func(ctx context.Context) error {
		called = true
		if _, ok := ctx.Deadline(); !ok {
			t.Fatal("readiness dependency has no timeout")
		}
		return nil
	}})
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, httptest.NewRequest(http.MethodGet, "/readyz", nil))
	if rec.Code != http.StatusOK || !strings.Contains(rec.Body.String(), `"status":"ready"`) {
		t.Fatalf("readiness=%d body=%s, want ready", rec.Code, rec.Body.String())
	}
	if !called {
		t.Fatal("readiness dependency was not called")
	}

	// The handler must also report ready when no dependency check is configured.
	rec = httptest.NewRecorder()
	New(nil).ServeHTTP(rec, httptest.NewRequest(http.MethodGet, "/readyz", nil))
	if rec.Code != http.StatusOK || !strings.Contains(rec.Body.String(), `"status":"ready"`) {
		t.Fatalf("unconfigured readiness=%d body=%s, want ready", rec.Code, rec.Body.String())
	}
}

func TestCreateAccountRejectsExtraJSONAndUnknownFields(t *testing.T) {
	for _, tc := range []struct {
		name, body, message string
	}{
		{"second value", `{}` + ` {}`, "send exactly one JSON object"},
		{"unknown field", `{"username":"ValidUser","password":"Pass123!","passwordConfirmation":"Pass123!","extra":true}`, "invalid JSON"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			h := New(store.NewJSONStore(t.TempDir()))
			req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(tc.body))
			req.Header.Set("Content-Type", "application/json")
			rec := httptest.NewRecorder()
			h.ServeHTTP(rec, req)
			if rec.Code != http.StatusBadRequest || !strings.Contains(rec.Body.String(), `"error":"`+tc.message+`"`) {
				t.Fatalf("status=%d body=%s, want bad request %q", rec.Code, rec.Body.String(), tc.message)
			}
		})
	}
}

func TestCreateAccountRejectsBusyHashPool(t *testing.T) {
	h := &Handler{store: &scriptedRegistrationStore{}, limiter: newRateLimiter(10), hashSlots: make(chan struct{}, 1)}
	h.hashSlots <- struct{}{}
	req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(
		`{"username":"ValidUser","password":"Pass123!","passwordConfirmation":"Pass123!"}`))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	h.createAccount(rec, req)
	if rec.Code != http.StatusServiceUnavailable || !strings.Contains(rec.Body.String(), `"error":"service busy; please try again"`) {
		t.Fatalf("status=%d body=%s, want busy response", rec.Code, rec.Body.String())
	}
}

func TestCreateAccountRateLimitReturnsTooManyRequests(t *testing.T) {
	h := NewWithConfig(&scriptedRegistrationStore{}, Config{RequestsPerMinute: 1})
	request := func() *httptest.ResponseRecorder {
		req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader("{"))
		req.Header.Set("Content-Type", "application/json")
		rec := httptest.NewRecorder()
		h.ServeHTTP(rec, req)
		return rec
	}
	if got := request().Code; got != http.StatusBadRequest {
		t.Fatalf("first request status=%d, want 400", got)
	}
	if rec := request(); rec.Code != http.StatusTooManyRequests || !strings.Contains(rec.Body.String(), `"error":"too many requests`) {
		t.Fatalf("second request status=%d body=%s, want rate limit", rec.Code, rec.Body.String())
	}
}

func TestCreateAccountMapsStoreConflictsAndFailures(t *testing.T) {
	storageFailure := errors.New("storage unavailable")
	for _, tc := range []struct {
		name      string
		store     *scriptedRegistrationStore
		wantCode  int
		wantError string
	}{
		{"existing name", &scriptedRegistrationStore{exists: true}, http.StatusConflict, "username unavailable"},
		{"concurrent duplicate", &scriptedRegistrationStore{createErr: store.ErrAccountExists}, http.StatusConflict, "username unavailable"},
		{"lookup failure", &scriptedRegistrationStore{existsErr: storageFailure}, http.StatusInternalServerError, "internal error"},
		{"write failure", &scriptedRegistrationStore{createErr: storageFailure}, http.StatusInternalServerError, "internal error"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			h := New(tc.store)
			req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(
				`{"username":"ValidUser","password":"Pass123!","passwordConfirmation":"Pass123!"}`))
			req.Header.Set("Content-Type", "application/json")
			rec := httptest.NewRecorder()
			h.ServeHTTP(rec, req)
			if rec.Code != tc.wantCode || !strings.Contains(rec.Body.String(), `"error":"`+tc.wantError+`"`) {
				t.Fatalf("status=%d body=%s, want status=%d error=%q", rec.Code, rec.Body.String(), tc.wantCode, tc.wantError)
			}
		})
	}
}

func TestRateLimiterSweepsExpiredEntriesAndResetsWindow(t *testing.T) {
	now := time.Now()
	limiter := newRateLimiter(1)
	limiter.lastSweep = now.Add(-time.Minute)
	limiter.visitors["expired"] = rateEntry{count: 1, reset: now.Add(-time.Second)}
	limiter.visitors["active"] = rateEntry{count: 0, reset: now.Add(time.Minute)}
	if !limiter.Allow("new") {
		t.Fatal("new visitor was rejected after expired entry sweep")
	}
	if _, ok := limiter.visitors["expired"]; ok {
		t.Fatal("expired visitor remained after sweep")
	}
	if _, ok := limiter.visitors["active"]; !ok {
		t.Fatal("active visitor was removed during sweep")
	}
	if !limiter.Allow("active") {
		t.Fatal("active visitor was rejected before its window expired")
	}

	limiter.lastSweep = time.Now()
	limiter.visitors["window-ended"] = rateEntry{count: 1, reset: time.Now().Add(-time.Second)}
	if !limiter.Allow("window-ended") {
		t.Fatal("visitor was rejected after its rate window expired")
	}
}

func TestRateLimiterRejectsNewVisitorsAtCapacityButAllowsKnownVisitor(t *testing.T) {
	limiter := newRateLimiter(1)
	limiter.lastSweep = time.Now()
	for i := 0; i < maxRateLimiterVisitors; i++ {
		limiter.visitors[fmt.Sprintf("visitor-%05d", i)] = rateEntry{reset: time.Now().Add(time.Minute)}
	}
	if limiter.Allow("new-visitor") {
		t.Fatal("new visitor was accepted after the map reached its bound")
	}
	if !limiter.Allow("visitor-00000") {
		t.Fatal("known visitor was rejected while the map was at capacity")
	}
}

func TestClientIPHandlesMalformedPeerAndTrustedForwardedChain(t *testing.T) {
	badPeer := httptest.NewRequest(http.MethodGet, "/", nil)
	badPeer.RemoteAddr = "unparsed-peer"
	if got := clientIP(badPeer, nil); got != "unparsed-peer" {
		t.Fatalf("malformed peer=%q, want preserved address", got)
	}

	trusted := []netip.Prefix{netip.MustParsePrefix("127.0.0.0/8")}
	req := httptest.NewRequest(http.MethodGet, "/", nil)
	req.RemoteAddr = "127.0.0.1:1234"
	req.Header.Add("X-Forwarded-For", "127.0.0.2")
	req.Header.Add("X-Forwarded-For", "127.0.0.3")
	if got := clientIP(req, trusted); got != "127.0.0.2" {
		t.Fatalf("all-trusted forwarded chain selected %q, want first hop", got)
	}
}
