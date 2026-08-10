package accountapi

import (
	"context"
	"encoding/json"
	"errors"
	"io"
	"log"
	"net/http"
	"net/netip"
	"strings"
	"time"

	"wydgo/internal/account"
)

type Handler struct {
	store          account.RegistrationStore
	limiter        *rateLimiter
	hashSlots      chan struct{}
	trustedProxies []netip.Prefix
	ready          func(context.Context) error
}

func New(store account.RegistrationStore) http.Handler {
	return NewWithConfig(store, Config{})
}

type Config struct {
	RequestsPerMinute int
	HashConcurrency   int
	TrustedProxies    []netip.Prefix
	Ready             func(context.Context) error
}

func NewWithConfig(store account.RegistrationStore, config Config) http.Handler {
	if config.RequestsPerMinute <= 0 {
		config.RequestsPerMinute = 10
	}
	if config.HashConcurrency <= 0 {
		config.HashConcurrency = 4
	}
	h := &Handler{store: store, limiter: newRateLimiter(config.RequestsPerMinute),
		hashSlots:      make(chan struct{}, config.HashConcurrency),
		trustedProxies: append([]netip.Prefix(nil), config.TrustedProxies...), ready: config.Ready}
	mux := http.NewServeMux()
	mux.HandleFunc("GET /health", h.health)
	mux.HandleFunc("GET /healthz", h.health)
	mux.HandleFunc("GET /readyz", h.readiness)
	mux.HandleFunc("POST /v1/accounts", h.createAccount)
	return securityHeaders(mux)
}

func (h *Handler) health(w http.ResponseWriter, _ *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{"status": "ok"})
}

func (h *Handler) readiness(w http.ResponseWriter, r *http.Request) {
	if h.ready != nil {
		ctx, cancel := context.WithTimeout(r.Context(), 500*time.Millisecond)
		defer cancel()
		if err := h.ready(ctx); err != nil {
			writeJSON(w, http.StatusServiceUnavailable, map[string]string{"status": "not ready"})
			return
		}
	}
	writeJSON(w, http.StatusOK, map[string]string{"status": "ready"})
}

func (h *Handler) createAccount(w http.ResponseWriter, r *http.Request) {
	if !h.limiter.Allow(clientIP(r, h.trustedProxies)) {
		writeJSON(w, http.StatusTooManyRequests, map[string]string{"error": "muitas tentativas; aguarde um minuto"})
		return
	}
	select {
	case h.hashSlots <- struct{}{}:
		defer func() { <-h.hashSlots }()
	default:
		writeJSON(w, http.StatusServiceUnavailable, map[string]string{"error": "servico ocupado; tente novamente"})
		return
	}
	r.Body = http.MaxBytesReader(w, r.Body, 4096)
	dec := json.NewDecoder(r.Body)
	dec.DisallowUnknownFields()
	var req struct {
		Username             string `json:"username"`
		Password             string `json:"password"`
		PasswordConfirmation string `json:"passwordConfirmation"`
	}
	if err := dec.Decode(&req); err != nil {
		writeJSON(w, http.StatusBadRequest, map[string]string{"error": "JSON invalido"})
		return
	}
	if err := dec.Decode(&struct{}{}); err != io.EOF {
		writeJSON(w, http.StatusBadRequest, map[string]string{"error": "envie somente um objeto JSON"})
		return
	}
	acc, err := account.Create(h.store, req.Username, req.Password, req.PasswordConfirmation)
	if err != nil {
		var validation *account.ValidationError
		switch {
		case errors.As(err, &validation):
			writeJSON(w, http.StatusUnprocessableEntity, map[string]string{
				"error": validation.Message, "field": validation.Field,
			})
		case errors.Is(err, account.ErrUsernameUnavailable):
			writeJSON(w, http.StatusConflict, map[string]string{"error": "nome de conta indisponivel"})
		default:
			log.Printf("ACCOUNT-API criar conta: %v", err)
			writeJSON(w, http.StatusInternalServerError, map[string]string{"error": "erro interno"})
		}
		return
	}
	log.Printf("ACCOUNT-API conta %q criada", acc.Name)
	writeJSON(w, http.StatusCreated, map[string]string{"status": "created", "username": acc.Name})
}

func writeJSON(w http.ResponseWriter, status int, value any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(value)
}

func securityHeaders(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("X-Content-Type-Options", "nosniff")
		w.Header().Set("Cache-Control", "no-store")
		w.Header().Set("Referrer-Policy", "no-referrer")
		if strings.HasPrefix(r.URL.Path, "/v1/") &&
			!strings.HasPrefix(strings.ToLower(r.Header.Get("Content-Type")), "application/json") {
			writeJSON(w, http.StatusUnsupportedMediaType, map[string]string{"error": "use Content-Type application/json"})
			return
		}
		next.ServeHTTP(w, r)
	})
}
