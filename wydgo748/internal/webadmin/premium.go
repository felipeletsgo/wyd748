package webadmin

import (
	"context"
	"encoding/json"
	"errors"
	"io"
	"mime"
	"net/http"
	"strings"
	"time"

	"wydgo/internal/control"
)

// Premium shop staff endpoints (DOCS/protocol/premium-shop.md).

// premiumStaff authorizes a premium request for one capability and returns the
// session plus a re-check used immediately before the World mutates state.
func (h *Handler) premiumStaff(w http.ResponseWriter, r *http.Request, capability, event string) (session, func() bool, bool) {
	s, ok := h.authorized(w, r)
	if !ok {
		return session{}, nil, false
	}
	entries, err := h.cfg.Staff()
	staff := entries[s.Actor]
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return session{}, nil, false
	}
	if staff.fingerprint() != s.StaffFingerprint || !staff.permits(capability) {
		h.audit(r, event, s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return session{}, nil, false
	}
	if h.cfg.Premium == nil {
		writeError(w, 503, "premium_unavailable")
		return session{}, nil, false
	}
	if !h.allow("premium:"+s.Actor, 30) {
		writeError(w, 429, "rate_limited")
		return session{}, nil, false
	}
	if r.Method == http.MethodGet {
		return s, nil, true
	}
	key, csrfSession, ok := h.csrf(w, r)
	if !ok {
		return session{}, nil, false
	}
	if csrfSession.Actor != s.Actor || csrfSession.StaffFingerprint != s.StaffFingerprint {
		writeError(w, 403, "staff_revoked")
		return session{}, nil, false
	}
	mediaType, _, err := mime.ParseMediaType(r.Header.Get("Content-Type"))
	if err != nil || mediaType != "application/json" {
		writeError(w, 415, "invalid_content_type")
		return session{}, nil, false
	}
	authorize := func() bool {
		entries, err := h.cfg.Staff()
		entry, exists := entries[s.Actor]
		if err != nil || !exists || entry.fingerprint() != s.StaffFingerprint || !entry.permits(capability) {
			return false
		}
		h.mu.Lock()
		defer h.mu.Unlock()
		current, exists := h.sessions[key]
		now := h.now()
		return exists && current.Actor == s.Actor && current.CSRF == s.CSRF && current.Peer == s.Peer &&
			current.StaffFingerprint == s.StaffFingerprint && now.Before(current.Expires) && now.Sub(current.LastSeen) < 15*time.Minute
	}
	return s, authorize, true
}

func decodeStrict(w http.ResponseWriter, r *http.Request, limit int64, target any) bool {
	r.Body = http.MaxBytesReader(w, r.Body, limit)
	decoder := json.NewDecoder(r.Body)
	decoder.DisallowUnknownFields()
	var extra any
	if decoder.Decode(target) != nil || decoder.Decode(&extra) != io.EOF {
		writeError(w, 400, "invalid_command")
		return false
	}
	return true
}

func premiumTransportError(w http.ResponseWriter, err error) {
	switch {
	case errors.Is(err, control.ErrUnknownAccount):
		writeError(w, 404, "unknown_account")
	case errors.Is(err, control.ErrPremiumUnavailable):
		writeError(w, 503, "premium_unavailable")
	case errors.Is(err, control.ErrBusy):
		writeError(w, 429, "control_busy")
	default:
		writeError(w, 503, "server_unavailable")
	}
}

func (h *Handler) premiumShop(w http.ResponseWriter, r *http.Request) {
	s, authorize, ok := h.premiumStaff(w, r, control.PremiumCatalogCapability, "premium.catalog")
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		view, err := h.cfg.Premium.PremiumCatalog(ctx)
		if err != nil {
			premiumTransportError(w, err)
			return
		}
		writeJSON(w, 200, view)
		return
	}
	var cmd control.PremiumCatalogCommand
	if !decodeStrict(w, r, 8192, &cmd) {
		return
	}
	if cmd.Validate() != nil {
		writeError(w, 400, "invalid_command")
		return
	}
	h.audit(r, "premium.catalog", s.Actor, "requested", "expected_version", cmd.ExpectedCatalogVersion,
		"items", len(cmd.Items), "reason", cmd.Reason)
	result, err := h.cfg.Premium.SavePremiumCatalog(ctx, s.Actor, cmd, authorize)
	if err != nil {
		h.audit(r, "premium.catalog", s.Actor, "outcome_unknown")
		premiumTransportError(w, err)
		return
	}
	h.audit(r, "premium.catalog", s.Actor, result.Code, "catalog_version", result.Catalog.CatalogVersion)
	writeJSON(w, 200, result)
}

func (h *Handler) donateAccount(w http.ResponseWriter, r *http.Request) {
	s, authorize, ok := h.premiumStaff(w, r, control.DonateCreditCapability, "donate.credit")
	if !ok {
		return
	}
	account := strings.ToLower(r.PathValue("account"))
	if len(account) < 1 || len(account) > 12 || strings.IndexFunc(account, func(c rune) bool {
		return !(c >= 'a' && c <= 'z' || c >= '0' && c <= '9')
	}) >= 0 {
		writeError(w, 400, "invalid_account")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		view, err := h.cfg.Premium.DonateAccount(ctx, account)
		if err != nil {
			premiumTransportError(w, err)
			return
		}
		writeJSON(w, 200, view)
		return
	}
	var cmd control.DonateCreditCommand
	if !decodeStrict(w, r, 2048, &cmd) {
		return
	}
	if cmd.Validate() != nil || cmd.Account != account {
		writeError(w, 400, "invalid_command")
		return
	}
	h.audit(r, "donate.credit", s.Actor, "requested", "request_id", cmd.RequestID, "account", cmd.Account,
		"amount", cmd.Amount, "reason", cmd.Reason)
	result, err := h.cfg.Premium.CreditDonate(ctx, s.Actor, cmd, authorize)
	if err != nil {
		h.audit(r, "donate.credit", s.Actor, "outcome_unknown", "request_id", cmd.RequestID)
		premiumTransportError(w, err)
		return
	}
	h.audit(r, "donate.credit", s.Actor, result.Code, "request_id", cmd.RequestID, "credit_id", result.CreditID,
		"replayed", result.Replayed)
	writeJSON(w, 200, result)
}
