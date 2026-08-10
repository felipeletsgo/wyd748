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
		`{"username":"ApiConta","password":"Senha123!","passwordConfirmation":"Senha123!"}`))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, req)
	if rec.Code != http.StatusCreated {
		t.Fatalf("status=%d body=%s", rec.Code, rec.Body.String())
	}
	acc, err := st.LoadAccount("apiconta")
	if err != nil {
		t.Fatal(err)
	}
	if acc.PasswordHash == "" || len(acc.Chars) != 0 {
		t.Fatalf("conta da API invalida: %+v", acc)
	}
}

func TestForwardedForRequiresTrustedProxy(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/healthz", nil)
	req.RemoteAddr = "127.0.0.1:1234"
	req.Header.Set("X-Forwarded-For", "198.51.100.9")
	if got := clientIP(req, nil); got != "127.0.0.1" {
		t.Fatalf("header nao confiavel virou autoridade: %q", got)
	}
	trusted := []netip.Prefix{netip.MustParsePrefix("127.0.0.0/8")}
	if got := clientIP(req, trusted); got != "198.51.100.9" {
		t.Fatalf("proxy declarado nao foi reconhecido: %q", got)
	}
}

func TestReadinessReportsDependencyFailure(t *testing.T) {
	h := NewWithConfig(store.NewJSONStore(t.TempDir()), Config{Ready: func(context.Context) error {
		return errors.New("postgres unavailable")
	}})
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, httptest.NewRequest(http.MethodGet, "/readyz", nil))
	if rec.Code != http.StatusServiceUnavailable {
		t.Fatalf("readiness=%d, quer 503", rec.Code)
	}
}

func TestCreateAccountAPIRejectsWrongContentType(t *testing.T) {
	h := New(store.NewJSONStore(t.TempDir()))
	req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(`{}`))
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, req)
	if rec.Code != http.StatusUnsupportedMediaType {
		t.Fatalf("status=%d, quer 415", rec.Code)
	}
}
