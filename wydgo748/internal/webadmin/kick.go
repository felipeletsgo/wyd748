package webadmin

import (
	"context"
	"encoding/json"
	"io"
	"mime"
	"net/http"
	"time"
	"wydgo/internal/control"
)

func (h *Handler) kick(w http.ResponseWriter, r *http.Request) {
	s, ok := h.authorized(w, r)
	if !ok {
		return
	}
	entries, err := h.cfg.Staff()
	staff := entries[s.Actor]
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return
	}
	if staff.fingerprint() != s.StaffFingerprint || !staff.permits(control.KickCapability) {
		h.audit(r, "player.kick", s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return
	}
	if h.cfg.Kick == nil {
		writeError(w, 503, "moderation_unavailable")
		return
	}
	if !h.allow("kick:"+s.Actor, 30) {
		writeError(w, 429, "rate_limited")
		return
	}
	key, csrfSession, ok := h.csrf(w, r)
	if !ok {
		return
	}
	if csrfSession.Actor != s.Actor || csrfSession.StaffFingerprint != s.StaffFingerprint {
		writeError(w, 403, "staff_revoked")
		return
	}
	mediaType, _, err := mime.ParseMediaType(r.Header.Get("Content-Type"))
	if err != nil || mediaType != "application/json" {
		writeError(w, 415, "invalid_content_type")
		return
	}
	r.Body = http.MaxBytesReader(w, r.Body, 2048)
	d := json.NewDecoder(r.Body)
	d.DisallowUnknownFields()
	var cmd control.KickCommand
	var extra any
	if d.Decode(&cmd) != nil || d.Decode(&extra) != io.EOF || cmd.Validate() != nil || cmd.UID != r.PathValue("uid") {
		writeError(w, 400, "invalid_command")
		return
	}
	// Recheck at execution, not just admission: queued commands cannot outlive
	// logout, expiry, peer binding, or revoked/modified staff permissions.
	authorize := func() bool {
		entries, err := h.cfg.Staff()
		entry, exists := entries[s.Actor]
		if err != nil || !exists || entry.fingerprint() != s.StaffFingerprint || !entry.permits(control.KickCapability) {
			return false
		}
		h.mu.Lock()
		defer h.mu.Unlock()
		current, exists := h.sessions[key]
		now := h.now()
		return exists && current.Actor == s.Actor && current.CSRF == s.CSRF && current.Peer == s.Peer && current.StaffFingerprint == s.StaffFingerprint && now.Before(current.Expires) && now.Sub(current.LastSeen) < 15*time.Minute
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	h.audit(r, "player.kick", s.Actor, "requested", "operation_id", cmd.OperationID, "reason", cmd.Reason, "uid", cmd.UID, "account", cmd.Account, "session_id", cmd.ExpectedSessionID)
	result, err := h.cfg.Kick.KickCommand(ctx, s.Actor, cmd, authorize)
	if err != nil {
		h.audit(r, "player.kick", s.Actor, "outcome_unknown", "operation_id", cmd.OperationID)
		globalDropTransportError(w, err)
		return
	}
	h.audit(r, "player.kick", s.Actor, result.Code, "operation_id", cmd.OperationID, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}
