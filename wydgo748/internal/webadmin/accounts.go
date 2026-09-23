package webadmin

import (
	"crypto/subtle"
	"encoding/json"
	"errors"
	"io"
	"mime"
	"net/http"
	"strings"

	"wydgo/internal/account"
)

type createAccountResponse struct {
	Version  int    `json:"version"`
	Status   string `json:"status"`
	Username string `json:"username"`
}

func (h *Handler) createAccount(w http.ResponseWriter, r *http.Request) {
	_, current, ok := h.csrf(w, r)
	if !ok {
		return
	}
	if h.cfg.Registration == nil {
		writeError(w, http.StatusServiceUnavailable, "registration_unavailable")
		return
	}
	media, _, _ := mime.ParseMediaType(r.Header.Get("Content-Type"))
	if media != "application/json" {
		writeError(w, http.StatusUnsupportedMediaType, "json_required")
		return
	}
	var body struct {
		Username             string `json:"username"`
		Password             string `json:"password"`
		PasswordConfirmation string `json:"passwordConfirmation"`
		AdminPIN             string `json:"adminPin"`
	}
	r.Body = http.MaxBytesReader(w, r.Body, 1024)
	decoder := json.NewDecoder(r.Body)
	decoder.DisallowUnknownFields()
	var extra any
	if decoder.Decode(&body) != nil || decoder.Decode(&extra) != io.EOF {
		writeError(w, http.StatusBadRequest, "invalid_request")
		return
	}
	nameKey := strings.ToLower(body.Username)
	if len(nameKey) > 12 {
		writeError(w, http.StatusBadRequest, "invalid_request")
		return
	}
	if !validAdminPIN(body.AdminPIN) {
		h.audit(r, "account.create", current.Actor, "rejected")
		writeError(w, http.StatusUnauthorized, "invalid_admin_pin")
		return
	}
	if !h.allowAll(
		rateLimit{key: "registration:ip:" + h.peer(r), limit: 8},
		rateLimit{key: "registration:account:" + nameKey, limit: 3},
		rateLimit{key: "registration:global", limit: 40},
	) {
		w.Header().Set("Retry-After", "60")
		writeError(w, http.StatusTooManyRequests, "rate_limited")
		return
	}
	if subtle.ConstantTimeCompare([]byte(h.cfg.AdminAccessPIN), []byte(body.AdminPIN)) != 1 {
		h.audit(r, "account.create", current.Actor, "rejected")
		writeError(w, http.StatusUnauthorized, "invalid_admin_pin")
		return
	}
	select {
	case h.hashSlots <- struct{}{}:
		defer func() { <-h.hashSlots }()
	default:
		writeError(w, http.StatusServiceUnavailable, "registration_busy")
		return
	}
	created, err := account.Create(h.cfg.Registration, body.Username, body.Password, body.PasswordConfirmation)
	if errors.Is(err, account.ErrUsernameUnavailable) {
		h.audit(r, "account.create", current.Actor, "conflict", "target_account", nameKey)
		writeError(w, http.StatusConflict, "username_unavailable")
		return
	}
	var validation *account.ValidationError
	if errors.As(err, &validation) {
		h.audit(r, "account.create", current.Actor, "rejected", "target_account", nameKey, "field", validation.Field)
		writeJSON(w, http.StatusUnprocessableEntity, map[string]string{
			"error": "validation_error", "field": validation.Field, "message": validation.Message,
		})
		return
	}
	if err != nil {
		h.audit(r, "account.create", current.Actor, "unavailable", "target_account", nameKey)
		writeError(w, http.StatusServiceUnavailable, "registration_failed")
		return
	}
	h.audit(r, "account.create", current.Actor, "succeeded", "target_account", strings.ToLower(created.Name))
	writeJSON(w, http.StatusCreated, createAccountResponse{Version: 1, Status: "created", Username: created.Name})
}
