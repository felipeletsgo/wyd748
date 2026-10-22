package webadmin

import (
	"context"
	"net/http"
	"strconv"
	"strings"
	"time"
	"unicode"

	"wydgo/internal/control"
	"wydgo/internal/store"
)

// Stage 3 panel endpoints (DOCS/server/staff-operations.md): the static
// content index and zone diagnostics are built once at start and never touch
// the World; the map and instance monitor are bounded World reads behind the
// overview gate and the circuit breaker; case notes live in PostgreSQL.

func (h *Handler) contentSearch(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "content.search", h.cfg.Content != nil, "content_unavailable", StatusCapability); !ok {
		return
	}
	query := r.URL.Query().Get("q")
	if len([]rune(query)) > 48 || strings.IndexFunc(query, unicode.IsControl) >= 0 {
		writeError(w, 400, "invalid_query")
		return
	}
	writeJSON(w, 200, map[string]any{"version": 1, "builtAt": h.cfg.Content.BuiltAt, "hits": h.cfg.Content.Search(query, 50)})
}

func (h *Handler) contentItem(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "content.item", h.cfg.Content != nil, "content_unavailable", StatusCapability); !ok {
		return
	}
	id, err := strconv.ParseUint(r.PathValue("id"), 10, 16)
	refs, found := h.cfg.Content.Item(uint16(id))
	if err != nil || !found {
		writeError(w, 404, "content_not_found")
		return
	}
	writeJSON(w, 200, refs)
}

func (h *Handler) contentMob(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "content.mob", h.cfg.Content != nil, "content_unavailable", StatusCapability); !ok {
		return
	}
	name := r.PathValue("name")
	refs, found := h.cfg.Content.Mob(name)
	if len(name) > 64 || !found {
		writeError(w, 404, "content_not_found")
		return
	}
	writeJSON(w, 200, refs)
}

func (h *Handler) contentIssues(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "content.issues", h.cfg.Content != nil, "content_unavailable", StatusCapability); !ok {
		return
	}
	writeJSON(w, 200, map[string]any{"version": 1, "builtAt": h.cfg.Content.BuiltAt, "issues": h.cfg.Content.Issues})
}

func parseCell(w http.ResponseWriter, r *http.Request) (uint16, uint16, bool) {
	x, errX := strconv.ParseUint(r.URL.Query().Get("x"), 10, 16)
	y, errY := strconv.ParseUint(r.URL.Query().Get("y"), 10, 16)
	if errX != nil || errY != nil || x > 4095 || y > 4095 {
		writeError(w, 400, "invalid_query")
		return 0, 0, false
	}
	return uint16(x), uint16(y), true
}

func (h *Handler) worldZone(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "world.zone", h.cfg.Zones != nil, "content_unavailable", StatusCapability); !ok {
		return
	}
	x, y, ok := parseCell(w, r)
	if !ok {
		return
	}
	writeJSON(w, 200, h.cfg.Zones.At(x, y))
}

func (h *Handler) worldTerrain(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "world.terrain", h.cfg.Zones != nil, "content_unavailable", StatusCapability); !ok {
		return
	}
	image, err := h.cfg.Zones.TerrainPNG()
	if err != nil {
		writeError(w, 503, "content_unavailable")
		return
	}
	w.Header().Set("Content-Type", "image/png")
	w.Header().Set("Content-Length", strconv.Itoa(len(image)))
	_, _ = w.Write(image)
}

func (h *Handler) worldMap(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.staffGate(w, r, "world.map", h.cfg.WorldReads != nil, "world_unavailable", PlayersCapability)
	if !ok {
		return
	}
	x, y, ok := parseCell(w, r)
	if !ok {
		return
	}
	radius, err := strconv.Atoi(r.URL.Query().Get("radius"))
	if err != nil || radius < 1 || radius > control.MaxMapRadius {
		writeError(w, 400, "invalid_query")
		return
	}
	if !h.optionalReadAllowed(w) {
		return
	}
	// A map view is a deliberate look, not a stream: 12 served reads a
	// minute. A read the World did not serve (busy, timeout) is refunded.
	quota := "world-map:" + s.Actor
	if !h.allow(quota, 12) {
		w.Header().Set("Retry-After", "5")
		writeError(w, 429, "rate_limited")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	snapshot, err := h.cfg.WorldReads.MapSnapshot(ctx, x, y, radius)
	if err != nil {
		h.refund(quota)
		staffOpsTransportError(w, err)
		return
	}
	writeJSON(w, 200, snapshot)
}

func (h *Handler) worldInstances(w http.ResponseWriter, r *http.Request) {
	if _, _, ok := h.staffGate(w, r, "world.instances", h.cfg.WorldReads != nil, "world_unavailable", StatusCapability); !ok {
		return
	}
	if !h.optionalReadAllowed(w) {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	view, err := h.cfg.WorldReads.Instances(ctx)
	if err != nil {
		staffOpsTransportError(w, err)
		return
	}
	writeJSON(w, 200, view)
}

func (h *Handler) accountNotes(w http.ResponseWriter, r *http.Request) {
	s, key, ok := h.staffGate(w, r, "account.notes", h.cfg.Notes != nil, "history_unavailable",
		PlayersCapability, control.BanCapability, control.MuteCapability)
	if !ok {
		return
	}
	account, ok := pathAccount(w, r)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	if r.Method == http.MethodGet {
		notes, asOf, err := h.cfg.Notes.ListAccountNotes(ctx, account, 50)
		if err != nil {
			historyError(w, err)
			return
		}
		writeJSON(w, 200, control.AccountNotes{Version: 1, AsOf: asOf.UTC(), Account: account, Notes: notes})
		return
	}
	// Writing notes is a moderation action: ban or mute permission, checked
	// before reading the body and again, with the session, right before the
	// write so a revocation during the request is honored.
	permitted := func() bool {
		return h.recheck(key, s, control.BanCapability)() || h.recheck(key, s, control.MuteCapability)()
	}
	if !permitted() {
		h.audit(r, "account.note", s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return
	}
	var cmd control.AccountNoteCommand
	if !decodeStrict(w, r, 8192, &cmd) {
		return
	}
	if cmd.Validate() != nil {
		writeError(w, 400, "invalid_command")
		return
	}
	if !permitted() {
		h.audit(r, "account.note", s.Actor, "forbidden", "target_account", account)
		writeError(w, 403, "capability_required")
		return
	}
	note, err := h.cfg.Notes.AddAccountNote(ctx, account, s.Actor, cmd)
	if err != nil {
		if store.IsAccountMissing(err) {
			writeError(w, 404, "unknown_account")
			return
		}
		h.audit(r, "account.note", s.Actor, "unavailable", "target_account", account)
		writeError(w, 503, "history_unavailable")
		return
	}
	// The audit names the note, not its text.
	h.audit(r, "account.note", s.Actor, "ok", "target_account", account, "note_id", note.ID,
		"length", len([]rune(note.Body)), "bug_refs", strings.Join(note.BugRefs, ","))
	writeJSON(w, 200, note)
}
