package accountapi

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"net/http/httptest"
	"net/netip"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestHealthEndpointsReturnJSONAndSecurityHeaders(t *testing.T) {
	h := New(nil)
	for _, path := range []string{"/health", "/healthz"} {
		t.Run(path, func(t *testing.T) {
			req := httptest.NewRequest(http.MethodGet, path, nil)
			w := httptest.NewRecorder()
			h.ServeHTTP(w, req)
			if w.Code != http.StatusOK {
				t.Fatalf("status=%d", w.Code)
			}
			var body map[string]string
			if err := json.Unmarshal(w.Body.Bytes(), &body); err != nil {
				t.Fatal(err)
			}
			if body["status"] != "ok" {
				t.Fatalf("body=%v", body)
			}
			if w.Header().Get("X-Content-Type-Options") == "" ||
				w.Header().Get("Cache-Control") != "no-store" || w.Header().Get("Referrer-Policy") != "no-referrer" {
				t.Fatalf("security headers incompletos: %v", w.Header())
			}
		})
	}
}

type apiContractStore struct {
	exists bool
	existsErr error
	createErr error
	created *model.Account
}
func (s *apiContractStore) AccountNameExists(string) (bool, error) { return s.exists, s.existsErr }
func (s *apiContractStore) CreateAccount(acc *model.Account) error { s.created = acc; return s.createErr }

func apiRequest(body string) *http.Request {
	r := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(body))
	r.RemoteAddr = "192.0.2.10:1234"
	r.Header.Set("Content-Type", "application/json")
	return r
}

func TestReadinessCoversNilAndSuccessfulDependency(t *testing.T) {
	for _, ready := range []func(context.Context) error{nil, func(context.Context) error { return nil }} {
		h := NewWithConfig(&apiContractStore{}, Config{Ready: ready})
		rec := httptest.NewRecorder()
		h.ServeHTTP(rec, httptest.NewRequest(http.MethodGet, "/readyz", nil))
		if rec.Code != http.StatusOK || !strings.Contains(rec.Body.String(), "ready") {
			t.Fatalf("readiness=%d %s", rec.Code, rec.Body.String())
		}
	}
}

func TestCreateAccountAPICoversRateBodySlotAndDomainFailures(t *testing.T) {
	valid := `{"username":"Valid1","password":"Valid1","passwordConfirmation":"Valid1"}`

	// O limiter vem antes da leitura do corpo.
	h := &Handler{store: &apiContractStore{}, limiter: newRateLimiter(1), hashSlots: make(chan struct{}, 1)}
	rec := httptest.NewRecorder()
	h.createAccount(rec, apiRequest(`{}`))
	rec = httptest.NewRecorder()
	h.createAccount(rec, apiRequest(valid))
	if rec.Code != http.StatusTooManyRequests {
		t.Fatalf("rate limit status=%d body=%s", rec.Code, rec.Body.String())
	}

	// Um segundo objeto JSON e explicitamente recusado.
	h = &Handler{store: &apiContractStore{}, limiter: newRateLimiter(10), hashSlots: make(chan struct{}, 1)}
	rec = httptest.NewRecorder()
	h.createAccount(rec, apiRequest(`{"username":"Valid1","password":"Valid1","passwordConfirmation":"Valid1"} {}`))
	if rec.Code != http.StatusBadRequest || !strings.Contains(rec.Body.String(), "somente um objeto") {
		t.Fatalf("JSON extra status=%d body=%s", rec.Code, rec.Body.String())
	}

	// Corpo valido com todos os workers caros ocupados recebe 503.
	h.hashSlots <- struct{}{}
	rec = httptest.NewRecorder()
	h.createAccount(rec, apiRequest(valid))
	if rec.Code != http.StatusServiceUnavailable {
		t.Fatalf("hash slot ocupado status=%d body=%s", rec.Code, rec.Body.String())
	}
	<-h.hashSlots

	// Validacao de dominio vira 422 com campo.
	rec = httptest.NewRecorder()
	h.createAccount(rec, apiRequest(`{"username":"abc","password":"Valid1","passwordConfirmation":"Valid1"}`))
	if rec.Code != http.StatusUnprocessableEntity || !strings.Contains(rec.Body.String(), "username") {
		t.Fatalf("validation status=%d body=%s", rec.Code, rec.Body.String())
	}

	// Nome indisponivel nao executa commit.
	conflictStore := &apiContractStore{exists: true}
	h = &Handler{store: conflictStore, limiter: newRateLimiter(10), hashSlots: make(chan struct{}, 1)}
	rec = httptest.NewRecorder()
	h.createAccount(rec, apiRequest(valid))
	if rec.Code != http.StatusConflict {
		t.Fatalf("conflict status=%d body=%s", rec.Code, rec.Body.String())
	}

	// Falha da dependencia vira 500 sem expor o erro interno.
	backend := errors.New("database offline")
	failureStore := &apiContractStore{existsErr: backend}
	h = &Handler{store: failureStore, limiter: newRateLimiter(10), hashSlots: make(chan struct{}, 1)}
	rec = httptest.NewRecorder()
	h.createAccount(rec, apiRequest(valid))
	if rec.Code != http.StatusInternalServerError || strings.Contains(rec.Body.String(), backend.Error()) {
		t.Fatalf("backend status=%d body=%s", rec.Code, rec.Body.String())
	}
}

func TestNewWithConfigDefaultsAndCopiesTrustedProxies(t *testing.T) {
	trusted := []netip.Prefix{netip.MustParsePrefix("127.0.0.0/8")}
	h := NewWithConfig(&apiContractStore{}, Config{TrustedProxies: trusted})
	trusted[0] = netip.MustParsePrefix("10.0.0.0/8")
	req := httptest.NewRequest(http.MethodGet, "/health", nil)
	req.RemoteAddr = "127.0.0.1:1"
	req.Header.Set("X-Forwarded-For", "198.51.100.1")
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("handler default invalido: %d", rec.Code)
	}
}

func TestRateLimiterCoversResetSweepCapacityAndLimit(t *testing.T) {
	l := newRateLimiter(2)
	if !l.Allow("a") || !l.Allow("a") || l.Allow("a") {
		t.Fatal("limite de duas requisicoes divergiu")
	}

	now := time.Now()
	l.visitors["expired"] = rateEntry{count: 99, reset: now.Add(-time.Second)}
	l.lastSweep = now.Add(-2 * time.Minute)
	if !l.Allow("fresh") {
		t.Fatal("entrada nova apos sweep foi recusada")
	}
	if _, exists := l.visitors["expired"]; exists {
		t.Fatal("entrada expirada nao foi coletada")
	}

	full := newRateLimiter(1)
	full.lastSweep = time.Now()
	resetAt := time.Now().Add(time.Minute)
	for i := 0; i < maxRateLimiterVisitors; i++ {
		full.visitors[fmt.Sprintf("ip-%d", i)] = rateEntry{count: 1, reset: resetAt}
	}
	if full.Allow("new-key") {
		t.Fatal("cardinalidade maxima aceitou visitante novo")
	}

	reset := newRateLimiter(1)
	reset.visitors["x"] = rateEntry{count: 99, reset: time.Now().Add(-time.Second)}
	reset.lastSweep = time.Now()
	if !reset.Allow("x") || reset.visitors["x"].count != 1 {
		t.Fatal("janela expirada nao foi reiniciada")
	}
}

func TestClientIPCoversRawInvalidPeerEmptyAndFullyTrustedForwardChain(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/health", nil)
	req.RemoteAddr = "not-an-ip"
	if got := clientIP(req, nil); got != "not-an-ip" {
		t.Fatalf("peer invalido=%q", got)
	}

	trusted := []netip.Prefix{netip.MustParsePrefix("127.0.0.0/8"), netip.MustParsePrefix("10.0.0.0/8")}
	req.RemoteAddr = "127.0.0.1:1"
	req.Header.Set("X-Forwarded-For", "10.0.0.1, 10.0.0.2")
	if got := clientIP(req, trusted); got != "10.0.0.1" {
		t.Fatalf("cadeia toda confiavel=%q", got)
	}
	req.Header.Del("X-Forwarded-For")
	if got := clientIP(req, trusted); got != "127.0.0.1" {
		t.Fatalf("proxy sem chain=%q", got)
	}
	if !trustedProxyAddress(netip.MustParseAddr("127.0.0.1"), trusted) || trustedProxyAddress(netip.MustParseAddr("192.0.2.1"), trusted) {
		t.Fatal("trustedProxyAddress divergiu")
	}
}
