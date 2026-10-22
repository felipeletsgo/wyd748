package webadmin

import (
	"context"
	"errors"
	"fmt"
	"mime"
	"net/http"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/control"
)

// Staff operations endpoints (DOCS/server/staff-operations.md).

func permitsAny(staff Staff, capabilities []string) bool {
	for _, capability := range capabilities {
		if staff.permits(capability) {
			return true
		}
	}
	return false
}

// staffGate authorizes a request holding any of the capabilities. Mutations
// also pass CSRF and content type; the returned key binds the re-check.
func (h *Handler) staffGate(w http.ResponseWriter, r *http.Request, event string, available bool, unavailable string, capabilities ...string) (session, [32]byte, bool) {
	s, ok := h.authorized(w, r)
	if !ok {
		return session{}, [32]byte{}, false
	}
	entries, err := h.cfg.Staff()
	staff := entries[s.Actor]
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return session{}, [32]byte{}, false
	}
	if staff.fingerprint() != s.StaffFingerprint || !permitsAny(staff, capabilities) {
		h.audit(r, event, s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return session{}, [32]byte{}, false
	}
	if !available {
		writeError(w, 503, unavailable)
		return session{}, [32]byte{}, false
	}
	if !h.allow("staff-ops:"+s.Actor, 60) {
		writeError(w, 429, "rate_limited")
		return session{}, [32]byte{}, false
	}
	if r.Method == http.MethodGet {
		return s, [32]byte{}, true
	}
	key, csrfSession, ok := h.csrf(w, r)
	if !ok {
		return session{}, [32]byte{}, false
	}
	if csrfSession.Actor != s.Actor || csrfSession.StaffFingerprint != s.StaffFingerprint {
		writeError(w, 403, "staff_revoked")
		return session{}, [32]byte{}, false
	}
	mediaType, _, err := mime.ParseMediaType(r.Header.Get("Content-Type"))
	if err != nil || mediaType != "application/json" {
		writeError(w, 415, "invalid_content_type")
		return session{}, [32]byte{}, false
	}
	return s, key, true
}

// requireCapability checks one specific capability after the body selected it.
func (h *Handler) requireCapability(w http.ResponseWriter, r *http.Request, s session, event, capability string) bool {
	entries, err := h.cfg.Staff()
	staff, exists := entries[s.Actor]
	if err != nil || !exists || staff.fingerprint() != s.StaffFingerprint || !staff.permits(capability) {
		h.audit(r, event, s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return false
	}
	return true
}

// recheck re-validates staff and session immediately before execution, so a
// queued command cannot outlive logout, expiry or revoked permissions.
func (h *Handler) recheck(key [32]byte, s session, capability string) func() bool {
	return func() bool {
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
}

func staffOpsTransportError(w http.ResponseWriter, err error) {
	switch {
	case errors.Is(err, control.ErrBusy):
		writeError(w, 429, "control_busy")
	case errors.Is(err, control.ErrUnknownAccount):
		writeError(w, 404, "unknown_account")
	case errors.Is(err, control.ErrSanctionsUnavailable):
		writeError(w, 503, "sanctions_unavailable")
	case errors.Is(err, control.ErrAuditUnavailable):
		writeError(w, 503, "audit_unavailable")
	default:
		writeError(w, 503, "server_unavailable")
	}
}

func pathAccount(w http.ResponseWriter, r *http.Request) (string, bool) {
	account := strings.ToLower(r.PathValue("account"))
	if !control.ValidAccountKey(account) {
		writeError(w, 400, "invalid_account")
		return "", false
	}
	return account, true
}

func pathID(w http.ResponseWriter, r *http.Request) (int64, bool) {
	id, err := strconv.ParseInt(r.PathValue("id"), 10, 64)
	if err != nil || id < 1 || strconv.FormatInt(id, 10) != r.PathValue("id") {
		writeError(w, 400, "invalid_id")
		return 0, false
	}
	return id, true
}

func (h *Handler) sanctions(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "account.sanction", h.cfg.Sanctions != nil, "sanctions_unavailable",
		control.BanCapability, control.MuteCapability)
	if !ok {
		return
	}
	account, ok := pathAccount(w, r)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 5*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		view, err := h.cfg.Sanctions.AccountSanctions(ctx, account)
		if err != nil {
			staffOpsTransportError(w, err)
			return
		}
		writeJSON(w, 200, view)
		return
	}
	var cmd control.SanctionCommand
	if !decodeStrict(w, r, 2048, &cmd) {
		return
	}
	if cmd.Validate() != nil || cmd.Account != account {
		writeError(w, 400, "invalid_command")
		return
	}
	capability, _ := control.SanctionCapability(cmd.Kind)
	if !h.requireCapability(w, r, s, "account.sanction", capability) {
		return
	}
	h.audit(r, "account.sanction", s.Actor, "requested", "request_id", cmd.RequestID, "target_account", cmd.Account,
		"kind", cmd.Kind, "duration_minutes", cmd.DurationMinutes, "reason", cmd.Reason)
	result, err := h.cfg.Sanctions.ApplySanction(ctx, s.Actor, cmd, h.recheck(key, s, capability))
	if err != nil {
		h.audit(r, "account.sanction", s.Actor, "outcome_unknown", "request_id", cmd.RequestID)
		staffOpsTransportError(w, err)
		return
	}
	h.audit(r, "account.sanction", s.Actor, result.Code, "request_id", cmd.RequestID, "sanction_id", result.Sanction.ID,
		"live", result.Live, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}

func (h *Handler) revokeSanction(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "account.sanction.revoke", h.cfg.Sanctions != nil, "sanctions_unavailable",
		control.BanCapability, control.MuteCapability)
	if !ok {
		return
	}
	account, ok := pathAccount(w, r)
	if !ok {
		return
	}
	id, ok := pathID(w, r)
	if !ok {
		return
	}
	var cmd control.SanctionRevokeCommand
	if !decodeStrict(w, r, 2048, &cmd) {
		return
	}
	if cmd.Validate() != nil || cmd.Account != account {
		writeError(w, 400, "invalid_command")
		return
	}
	capability, _ := control.SanctionCapability(cmd.Kind)
	if !h.requireCapability(w, r, s, "account.sanction.revoke", capability) {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 5*time.Second)
	defer cancel()
	h.audit(r, "account.sanction.revoke", s.Actor, "requested", "sanction_id", id, "target_account", account,
		"kind", cmd.Kind, "reason", cmd.Reason)
	result, err := h.cfg.Sanctions.RevokeSanction(ctx, s.Actor, id, cmd, h.recheck(key, s, capability))
	if err != nil {
		h.audit(r, "account.sanction.revoke", s.Actor, "outcome_unknown", "sanction_id", id)
		staffOpsTransportError(w, err)
		return
	}
	h.audit(r, "account.sanction.revoke", s.Actor, result.Code, "sanction_id", id, "live", result.Live, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}

func (h *Handler) notices(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "server.notice", h.cfg.Notices != nil, "notices_unavailable", control.NoticeCapability)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		view, err := h.cfg.Notices.Notices(ctx)
		if err != nil {
			staffOpsTransportError(w, err)
			return
		}
		writeJSON(w, 200, view)
		return
	}
	var cmd control.NoticeCommand
	if !decodeStrict(w, r, 2048, &cmd) {
		return
	}
	if cmd.Validate() != nil {
		writeError(w, 400, "invalid_command")
		return
	}
	h.audit(r, "server.notice", s.Actor, "requested", "operation_id", cmd.OperationID, "message", cmd.Message,
		"delay_seconds", cmd.DelaySeconds, "repeat_count", cmd.RepeatCount, "repeat_every_seconds", cmd.RepeatEverySeconds)
	result, err := h.cfg.Notices.ScheduleNotice(ctx, s.Actor, cmd, h.recheck(key, s, control.NoticeCapability))
	if err != nil {
		h.audit(r, "server.notice", s.Actor, "outcome_unknown", "operation_id", cmd.OperationID)
		staffOpsTransportError(w, err)
		return
	}
	h.audit(r, "server.notice", s.Actor, result.Code, "operation_id", cmd.OperationID, "delivered", result.Delivered, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}

func (h *Handler) cancelNotice(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "server.notice.cancel", h.cfg.Notices != nil, "notices_unavailable", control.NoticeCapability)
	if !ok {
		return
	}
	id, ok := pathID(w, r)
	if !ok {
		return
	}
	var cmd control.NoticeCancelCommand
	if !decodeStrict(w, r, 1024, &cmd) {
		return
	}
	if cmd.Validate() != nil {
		writeError(w, 400, "invalid_command")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	h.audit(r, "server.notice.cancel", s.Actor, "requested", "operation_id", cmd.OperationID, "notice_id", id)
	result, err := h.cfg.Notices.CancelNotice(ctx, s.Actor, id, cmd, h.recheck(key, s, control.NoticeCapability))
	if err != nil {
		h.audit(r, "server.notice.cancel", s.Actor, "outcome_unknown", "operation_id", cmd.OperationID)
		staffOpsTransportError(w, err)
		return
	}
	h.audit(r, "server.notice.cancel", s.Actor, result.Code, "operation_id", cmd.OperationID, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}

func (h *Handler) messagePlayer(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "player.message", h.cfg.Notices != nil, "notices_unavailable", control.NoticeCapability)
	if !ok {
		return
	}
	var cmd control.PlayerMessageCommand
	if !decodeStrict(w, r, 2048, &cmd) {
		return
	}
	if cmd.Validate() != nil || cmd.UID != r.PathValue("uid") {
		writeError(w, 400, "invalid_command")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	h.audit(r, "player.message", s.Actor, "requested", "operation_id", cmd.OperationID, "uid", cmd.UID,
		"account", cmd.Account, "session_id", cmd.ExpectedSessionID, "message", cmd.Message)
	result, err := h.cfg.Notices.MessagePlayer(ctx, s.Actor, cmd, h.recheck(key, s, control.NoticeCapability))
	if err != nil {
		h.audit(r, "player.message", s.Actor, "outcome_unknown", "operation_id", cmd.OperationID)
		staffOpsTransportError(w, err)
		return
	}
	h.audit(r, "player.message", s.Actor, result.Code, "operation_id", cmd.OperationID, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}

func (h *Handler) shutdown(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "server.shutdown", h.cfg.Shutdown != nil, "shutdown_unavailable", control.ShutdownCapability)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		status, err := h.cfg.Shutdown.ShutdownStatus(ctx)
		if err != nil {
			staffOpsTransportError(w, err)
			return
		}
		writeJSON(w, 200, status)
		return
	}
	var cmd control.ShutdownCommand
	if !decodeStrict(w, r, 2048, &cmd) {
		return
	}
	if cmd.Validate() != nil {
		writeError(w, 400, "invalid_command")
		return
	}
	h.audit(r, "server.shutdown", s.Actor, "requested", "operation_id", cmd.OperationID, "action", cmd.Action,
		"mode", cmd.Mode, "delay_seconds", cmd.DelaySeconds, "reason", cmd.Reason)
	result, err := h.cfg.Shutdown.ShutdownCommand(ctx, s.Actor, cmd, h.recheck(key, s, control.ShutdownCapability))
	if err != nil {
		h.audit(r, "server.shutdown", s.Actor, "outcome_unknown", "operation_id", cmd.OperationID)
		staffOpsTransportError(w, err)
		return
	}
	h.audit(r, "server.shutdown", s.Actor, result.Code, "operation_id", cmd.OperationID, "replayed", result.Replayed)
	writeJSON(w, 200, result)
}

func validAuditFilter(value string, limit int) bool {
	return len(value) <= limit && strings.IndexFunc(value, func(r rune) bool {
		return !(r >= 'a' && r <= 'z' || r >= '0' && r <= '9' || r == '.' || r == '_' || r == '-')
	}) < 0
}

func (h *Handler) auditLog(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.staffGate(w, r, "staff.audit", h.cfg.AuditStore != nil, "audit_unavailable", control.AuditReadCapability)
	if !ok {
		return
	}
	q := r.URL.Query()
	query := control.AuditQuery{Actor: strings.ToLower(q.Get("actor")), Action: q.Get("action"), Limit: 50}
	var err error
	if v := q.Get("before"); v != "" {
		if query.Before, err = strconv.ParseInt(v, 10, 64); err != nil || query.Before < 1 {
			writeError(w, 400, "invalid_query")
			return
		}
	}
	if v := q.Get("limit"); v != "" {
		if query.Limit, err = strconv.Atoi(v); err != nil || query.Limit < 1 || query.Limit > 100 {
			writeError(w, 400, "invalid_query")
			return
		}
	}
	if !validAuditFilter(query.Actor, 12) || !validAuditFilter(query.Action, 64) {
		writeError(w, 400, "invalid_query")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 5*time.Second)
	defer cancel()
	page, err := h.cfg.AuditStore.ListStaffAudit(ctx, query)
	if err != nil {
		h.audit(r, "staff.audit", s.Actor, "unavailable")
		writeError(w, 503, "audit_unavailable")
		return
	}
	writeJSON(w, 200, page)
}

// ---- Persistent audit ----

const (
	auditQueueSize      = 1024
	maxAuditDetails     = 32
	maxAuditDetailBytes = 512
	auditRetention      = 365 * 24 * time.Hour
	auditPruneBatch     = 500
	auditPruneBatches   = 20
	auditPruneFirst     = 10 * time.Minute
	auditPruneEvery     = 24 * time.Hour
	auditPruneBacklog   = time.Hour
)

func auditDetails(attrs []any) map[string]any {
	details := make(map[string]any, len(attrs)/2)
	for i := 0; i+1 < len(attrs) && len(details) < maxAuditDetails; i += 2 {
		key, ok := attrs[i].(string)
		if !ok || key == "" {
			continue
		}
		switch value := attrs[i+1].(type) {
		case nil, bool, int, int32, int64, uint16, uint32, uint64, float64:
			details[key] = value
		case string:
			details[key] = truncateAudit(value)
		default:
			details[key] = truncateAudit(fmt.Sprint(value))
		}
	}
	return details
}

func truncateAudit(value string) string {
	if len(value) <= maxAuditDetailBytes {
		return value
	}
	cut := maxAuditDetailBytes
	for cut > 0 && value[cut]&0xC0 == 0x80 {
		cut--
	}
	return value[:cut]
}

// startAuditWriter persists audit entries off the request path. The stderr
// log stays authoritative when PostgreSQL is slow or unavailable.
func (h *Handler) startAuditWriter() {
	if h.cfg.AuditStore == nil {
		return
	}
	h.auditQueue = make(chan control.AuditEntry, auditQueueSize)
	h.auditDone = make(chan struct{})
	go func() {
		defer close(h.auditDone)
		// Retention shares this goroutine so it never races audit writes.
		prune := time.NewTimer(auditPruneFirst)
		defer prune.Stop()
		for {
			select {
			case entry, ok := <-h.auditQueue:
				if !ok {
					return
				}
				ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
				err := h.cfg.AuditStore.AppendStaffAudit(ctx, entry)
				cancel()
				if err != nil {
					h.cfg.Audit.Warn("web_admin_audit_persist_failed", "action", entry.Action, "actor", entry.Actor)
				}
			case <-prune.C:
				_, moreAudit := h.pruneAudit()
				if _, moreLogins := h.pruneLogins(); moreAudit || moreLogins {
					prune.Reset(auditPruneBacklog)
				} else {
					prune.Reset(auditPruneEvery)
				}
			}
		}
	}()
}

// pruneAudit deletes rows past retention in a bounded number of small
// batches. more reports that old rows remain for the next run.
func (h *Handler) pruneAudit() (deleted int64, more bool) {
	cutoff := h.now().Add(-auditRetention)
	for range auditPruneBatches {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		n, err := h.cfg.AuditStore.PruneStaffAudit(ctx, cutoff, auditPruneBatch)
		cancel()
		if err != nil {
			h.cfg.Audit.Warn("web_admin_audit_prune_failed", "deleted", deleted)
			return deleted, false
		}
		deleted += n
		if n < auditPruneBatch {
			return deleted, false
		}
	}
	return deleted, true
}

// persistedAudit keeps the database for actions with consequences. Successful
// read-only requests stay in the stderr log so polling never becomes writes.
func persistedAudit(entry control.AuditEntry) bool {
	return !((entry.Method == http.MethodGet || entry.Method == http.MethodHead) && entry.Result == "succeeded")
}

func (h *Handler) persistAudit(entry control.AuditEntry) {
	if !persistedAudit(entry) {
		return
	}
	h.auditMu.Lock()
	defer h.auditMu.Unlock()
	if h.auditQueue == nil || h.auditClosed {
		return
	}
	select {
	case h.auditQueue <- entry:
	default:
		h.cfg.Audit.Warn("web_admin_audit_dropped", "action", entry.Action, "actor", entry.Actor)
	}
}

// Close flushes queued audit entries. Call it after the HTTP server stopped.
func (h *Handler) Close() {
	h.samplerOnce.Do(func() {
		if h.samplerStop != nil {
			close(h.samplerStop)
			<-h.samplerDone
		}
	})
	h.auditMu.Lock()
	if h.auditQueue == nil || h.auditClosed {
		h.auditMu.Unlock()
		return
	}
	h.auditClosed = true
	close(h.auditQueue)
	h.auditMu.Unlock()
	select {
	case <-h.auditDone:
	case <-time.After(5 * time.Second):
		h.cfg.Audit.Warn("web_admin_audit_flush_timeout")
	}
}

// RecordServerStart writes one boot receipt with the build identity. After a
// scheduled restart it confirms that a new process came up, and which build.
func (h *Handler) RecordServerStart(attrs ...any) {
	h.cfg.Audit.Info("web_admin", append([]any{"action", "server.start", "result", "ok"}, attrs...)...)
	h.persistAudit(control.AuditEntry{Action: "server.start", Result: "ok", Details: auditDetails(attrs)})
}
