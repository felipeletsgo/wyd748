package accountapi

import (
	"net/http"
	"net/http/httptest"
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

func TestCreateAccountAPIRejectsWrongContentType(t *testing.T) {
	h := New(store.NewJSONStore(t.TempDir()))
	req := httptest.NewRequest(http.MethodPost, "/v1/accounts", strings.NewReader(`{}`))
	rec := httptest.NewRecorder()
	h.ServeHTTP(rec, req)
	if rec.Code != http.StatusUnsupportedMediaType {
		t.Fatalf("status=%d, quer 415", rec.Code)
	}
}
