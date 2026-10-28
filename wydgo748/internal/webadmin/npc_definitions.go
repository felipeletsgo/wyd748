package webadmin

import (
	"context"
	"errors"
	"mime"
	"net/http"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/control"
)

func (h *Handler) npcDefinitionStaff(w http.ResponseWriter, r *http.Request) (session, func() bool, bool) {
	s, ok := h.authorized(w, r)
	if !ok {
		return session{}, nil, false
	}
	entries, err := h.cfg.Staff()
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return session{}, nil, false
	}
	staff := entries[s.Actor]
	if staff.fingerprint() != s.StaffFingerprint || !staff.permits(control.NPCDefinitionCapability) {
		h.audit(r, "npc.definition", s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return session{}, nil, false
	}
	if h.cfg.NPCDefinitions == nil {
		writeError(w, 503, "npc_unavailable")
		return session{}, nil, false
	}
	if !h.allow("npc-definition:"+s.Actor, 30) {
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
		currentStaff, err := h.cfg.Staff()
		entry, exists := currentStaff[s.Actor]
		if err != nil || !exists || entry.fingerprint() != s.StaffFingerprint || !entry.permits(control.NPCDefinitionCapability) {
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

func staffNPCChannel(r *http.Request) (uint8, bool) {
	value := strings.TrimSpace(r.URL.Query().Get("channel"))
	if value == "" {
		return 1, true
	}
	n, err := strconv.ParseUint(value, 10, 8)
	return uint8(n), err == nil && n > 0
}

func (h *Handler) npcDefinitions(w http.ResponseWriter, r *http.Request) {
	s, authorize, ok := h.npcDefinitionStaff(w, r)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 5*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		channel, valid := staffNPCChannel(r)
		if !valid {
			writeError(w, 400, "invalid_channel")
			return
		}
		view, err := h.cfg.NPCDefinitions.NPCDefinitions(ctx, channel)
		if err != nil {
			if errors.Is(err, control.ErrBusy) {
				writeError(w, 429, "control_busy")
			} else {
				writeError(w, 503, "server_unavailable")
			}
			return
		}
		writeJSON(w, 200, view)
		return
	}
	var cmd control.NPCDefinitionCommand
	if !decodeStrict(w, r, 70*1024, &cmd) {
		return
	}
	if cmd.Validate() != nil || cmd.Name != r.PathValue("name") {
		writeError(w, 400, "invalid_command")
		return
	}
	h.audit(r, "npc.definition", s.Actor, "requested", "npc", cmd.Name, "channel", cmd.Channel,
		"expected_version", cmd.ExpectedVersion, "targets", cmd.Targets, "restore_source", cmd.RestoreSource, "reason", cmd.Reason)
	result, err := h.cfg.NPCDefinitions.SaveNPCDefinition(ctx, s.Actor, cmd, authorize)
	if err != nil {
		h.audit(r, "npc.definition", s.Actor, "outcome_unknown", "npc", cmd.Name, "channel", cmd.Channel)
		if errors.Is(err, control.ErrBusy) {
			writeError(w, 429, "control_busy")
		} else {
			writeError(w, 503, "server_unavailable")
		}
		return
	}
	h.audit(r, "npc.definition", s.Actor, result.Code, "npc", cmd.Name, "channel", cmd.Channel, "version", result.NPC.Version)
	writeJSON(w, 200, result)
}
