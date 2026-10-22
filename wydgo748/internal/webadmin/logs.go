package webadmin

import (
	"context"
	"errors"
	"net/http"
	"strconv"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/logreader"
)

// Server logs and backup receipts through the wyd-log-reader helper
// (DOCS/server/staff-operations.md). The panel never reads the journal or the
// filesystem itself; it forwards one validated request per call.

type LogReader interface {
	Do(context.Context, logreader.Request) (logreader.Response, error)
}

type LogPage struct {
	Version    int               `json:"version"`
	AsOf       time.Time         `json:"asOf"`
	Source     string            `json:"source"`
	Entries    []logreader.Entry `json:"entries"`
	NextCursor string            `json:"nextCursor"`
	Truncated  bool              `json:"truncated"`
}

type BackupStatus struct {
	Version     int                `json:"version"`
	AsOf        time.Time          `json:"asOf"`
	Backup      *logreader.Receipt `json:"backup"`
	RestoreTest *logreader.Receipt `json:"restoreTest"`
}

func logReaderError(w http.ResponseWriter, code string) {
	switch code {
	case "cursor_expired":
		writeError(w, 410, "cursor_expired")
	case "invalid_request", "invalid_source", "invalid_window":
		writeError(w, 400, "invalid_query")
	case "busy":
		w.Header().Set("Retry-After", "2")
		writeError(w, 429, "control_busy")
	default:
		writeError(w, 503, "logs_unavailable")
	}
}

func (h *Handler) serverLogs(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.staffGate(w, r, "server.logs", h.cfg.Logs != nil, "logs_unavailable", control.LogsCapability)
	if !ok {
		return
	}
	q := r.URL.Query()
	now := h.now().UTC()
	request := logreader.Request{Version: 1, Kind: logreader.KindLogs, Source: q.Get("source"), Query: q.Get("query"),
		Cursor: q.Get("cursor"), Limit: logreader.DefaultLimit, Priority: 6, Since: now.Add(-time.Hour), Until: now}
	parseTime := func(name string, target *time.Time) bool {
		raw := q.Get(name)
		if raw == "" {
			return true
		}
		value, err := time.Parse(time.RFC3339, raw)
		*target = value.UTC()
		return err == nil
	}
	parseInt := func(name string, target *int) bool {
		raw := q.Get(name)
		if raw == "" {
			return true
		}
		value, err := strconv.Atoi(raw)
		*target = value
		return err == nil
	}
	if !parseTime("since", &request.Since) || !parseTime("until", &request.Until) ||
		!parseInt("limit", &request.Limit) || !parseInt("priority", &request.Priority) || request.Validate() != nil {
		writeError(w, 400, "invalid_query")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 4*time.Second)
	defer cancel()
	response, err := h.cfg.Logs.Do(ctx, request)
	// Read access is audited without the log content or the search text.
	if err != nil || response.Error != "" {
		code := response.Error
		if errors.Is(err, logreader.ErrUnavailable) || code == "" {
			code = "logs_unavailable"
		}
		h.audit(r, "server.logs", s.Actor, code, "source", request.Source)
		logReaderError(w, code)
		return
	}
	h.audit(r, "server.logs", s.Actor, "read", "source", request.Source, "since", request.Since.Format(time.RFC3339),
		"until", request.Until.Format(time.RFC3339), "entries", len(response.Entries), "filtered", request.Query != "")
	entries := response.Entries
	if entries == nil {
		entries = []logreader.Entry{}
	}
	writeJSON(w, 200, LogPage{Version: 1, AsOf: now, Source: request.Source, Entries: entries,
		NextCursor: response.NextCursor, Truncated: response.Truncated})
}

func (h *Handler) backupStatus(w http.ResponseWriter, r *http.Request) {
	_, _, ok := h.staffGate(w, r, "server.backups", h.cfg.Logs != nil, "logs_unavailable", control.LogsCapability)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	response, err := h.cfg.Logs.Do(ctx, logreader.Request{Version: 1, Kind: logreader.KindBackup})
	if err != nil || response.Error != "" {
		logReaderError(w, response.Error)
		return
	}
	writeJSON(w, 200, BackupStatus{Version: 1, AsOf: h.now().UTC(), Backup: response.Backup, RestoreTest: response.RestoreTest})
}
