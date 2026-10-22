package webadmin

import (
	"context"
	"net/http"
	"strings"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/store"
)

// Account page endpoints (DOCS/server/staff-operations.md): login history,
// staff actions targeting the account and a read-only storage view.

type StorageReader interface {
	ReadCharacterStorage(context.Context, string, string) (control.CharacterStorage, error)
}

func historyError(w http.ResponseWriter, err error) {
	if store.IsAccountMissing(err) {
		writeError(w, 404, "unknown_account")
		return
	}
	writeError(w, 503, "history_unavailable")
}

func (h *Handler) accountLogins(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.staffGate(w, r, "account.logins", h.cfg.AccountHistory != nil, "history_unavailable", PlayersCapability)
	if !ok {
		return
	}
	account, ok := pathAccount(w, r)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	logins, asOf, err := h.cfg.AccountHistory.ListAccountLogins(ctx, account, 50)
	if err != nil {
		historyError(w, err)
		return
	}
	entries, _ := h.cfg.Staff()
	ipVisible := entries[s.Actor].permits(control.AccountIPCapability)
	if !ipVisible {
		for i := range logins {
			logins[i].ClientIP = ""
		}
	}
	writeJSON(w, 200, control.AccountLogins{Version: 1, AsOf: asOf.UTC(), Account: account, IPVisible: ipVisible, Logins: logins})
}

func (h *Handler) accountActions(w http.ResponseWriter, r *http.Request) {
	_, _, ok := h.staffGate(w, r, "account.actions", h.cfg.AccountHistory != nil, "history_unavailable", control.AuditReadCapability)
	if !ok {
		return
	}
	account, ok := pathAccount(w, r)
	if !ok {
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	actions, asOf, err := h.cfg.AccountHistory.ListStaffAuditForAccount(ctx, account, 50)
	if err != nil {
		historyError(w, err)
		return
	}
	writeJSON(w, 200, control.AccountActions{Version: 1, AsOf: asOf.UTC(), Account: account, Entries: actions})
}

func (h *Handler) characterStorage(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.staffGate(w, r, "player.storage", h.cfg.Storage != nil, "storage_unavailable", PlayersCapability)
	if !ok {
		return
	}
	account := strings.ToLower(r.URL.Query().Get("account"))
	uid := r.PathValue("uid")
	if !control.ValidAccountKey(account) || uid == "" || len(uid) > 128 {
		writeError(w, 400, "invalid_target")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	snapshot, err := h.cfg.Storage.ReadCharacterStorage(ctx, account, uid)
	switch {
	case store.IsAccountMissing(err):
		writeError(w, 404, "persistent_not_found")
		return
	case err != nil:
		h.audit(r, "player.storage", s.Actor, "unavailable", "target_account", account, "target_uid", uid)
		writeError(w, 503, "storage_unavailable")
		return
	}
	if h.cfg.ItemName != nil {
		for _, items := range [][]control.StoredItem{snapshot.Equipment, snapshot.Inventory, snapshot.Cargo} {
			for i := range items {
				items[i].Name = strings.ReplaceAll(h.cfg.ItemName(items[i].ItemID), "_", " ")
			}
		}
	}
	h.audit(r, "player.storage", s.Actor, "succeeded", "target_account", account, "target_uid", uid)
	writeJSON(w, 200, snapshot)
}

// pruneLogins mirrors pruneAudit for login history.
func (h *Handler) pruneLogins() (deleted int64, more bool) {
	if h.cfg.AccountHistory == nil {
		return 0, false
	}
	cutoff := h.now().Add(-control.LoginHistoryRetention)
	for range auditPruneBatches {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		n, err := h.cfg.AccountHistory.PruneAccountLogins(ctx, cutoff, auditPruneBatch)
		cancel()
		if err != nil {
			h.cfg.Audit.Warn("web_admin_login_prune_failed", "deleted", deleted)
			return deleted, false
		}
		deleted += n
		if n < auditPruneBatch {
			return deleted, false
		}
	}
	return deleted, true
}
