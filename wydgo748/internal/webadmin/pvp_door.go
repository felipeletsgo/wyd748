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

func (h *Handler) pvpDoor(w http.ResponseWriter, r *http.Request) {
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
	if staff.fingerprint() != s.StaffFingerprint || !staff.permits(control.PvPDoorCapability) {
		h.audit(r, "pvp_door", s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return
	}
	if h.cfg.PvPDoor == nil {
		writeError(w, 503, "pvp_door_unavailable")
		return
	}
	if !h.allow("pvp-door:"+s.Actor, 30) {
		writeError(w, 429, "rate_limited")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		status, err := h.cfg.PvPDoor.PvPDoorStatus(ctx)
		if err != nil {
			globalDropTransportError(w, err)
			return
		}
		writeJSON(w, 200, status)
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
	decoder := json.NewDecoder(r.Body)
	decoder.DisallowUnknownFields()
	var cmd control.PvPDoorCommand
	var extra any
	if decoder.Decode(&cmd) != nil || decoder.Decode(&extra) != io.EOF || cmd.Validate() != nil {
		writeError(w, 400, "invalid_command")
		return
	}
	// The World calls this immediately before mutation. Queued requests cannot
	// outlive logout, expiry, peer binding or a change to staff.json.
	authorize := func() bool {
		entries, err := h.cfg.Staff()
		entry, exists := entries[s.Actor]
		if err != nil || !exists || entry.fingerprint() != s.StaffFingerprint || !entry.permits(control.PvPDoorCapability) {
			return false
		}
		h.mu.Lock()
		defer h.mu.Unlock()
		current, exists := h.sessions[key]
		now := h.now()
		return exists && current.Actor == s.Actor && current.CSRF == s.CSRF && current.Peer == s.Peer && current.StaffFingerprint == s.StaffFingerprint && now.Before(current.Expires) && now.Sub(current.LastSeen) < 15*time.Minute
	}
	h.audit(r, "pvp_door."+cmd.Action, s.Actor, "requested", "operation_id", cmd.OperationID, "reason", cmd.Reason)
	result, err := h.cfg.PvPDoor.PvPDoorCommand(ctx, s.Actor, cmd, authorize)
	if err != nil {
		h.audit(r, "pvp_door."+cmd.Action, s.Actor, "outcome_unknown", "operation_id", cmd.OperationID)
		globalDropTransportError(w, err)
		return
	}
	h.audit(r, "pvp_door."+cmd.Action, s.Actor, result.Code, "operation_id", cmd.OperationID)
	// Lock/open is idempotent; a retry reports "unchanged" instead of acting twice.
	writeJSON(w, 200, result)
}
