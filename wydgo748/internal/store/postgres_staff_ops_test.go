package store

import (
	"context"
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func TestPostgresSanctionsLifecycle(t *testing.T) {
	st, _ := criticalAccountsPostgresFixture(t)
	ctx := context.Background()
	name := fmt.Sprintf("sx%010d", time.Now().UnixNano()%10_000_000_000)
	if err := st.CreateAccount(postgresTestAccount(name, "Sanctioned", model.Item{Index: 4011})); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, name) })

	mute := control.SanctionCommand{Version: 1, RequestID: name + "-mute-0001", Account: name, Kind: control.SanctionMute, DurationMinutes: 60, Reason: "spam in global"}
	first, duplicate, err := st.AddSanction(ctx, "operator", mute)
	if err != nil || duplicate || !first.Active || first.ExpiresAt == nil || first.Actor != "operator" {
		t.Fatal(first, duplicate, err)
	}
	again, duplicate, err := st.AddSanction(ctx, "operator", mute)
	if err != nil || !duplicate || again.ID != first.ID {
		t.Fatal("retry inserted a second sanction", again, err)
	}
	conflict := mute
	conflict.Reason = "another reason"
	if _, _, err := st.AddSanction(ctx, "operator", conflict); !errors.Is(err, control.ErrSanctionConflict) {
		t.Fatal(err)
	}
	missing := mute
	missing.RequestID, missing.Account = name+"-miss-0001", "nosuchacct1"
	if _, _, err := st.AddSanction(ctx, "operator", missing); !IsAccountMissing(err) {
		t.Fatal(err)
	}
	ban := control.SanctionCommand{Version: 1, RequestID: name + "-ban-00001", Account: name, Kind: control.SanctionBan, Reason: "botting"}
	permanent, _, err := st.AddSanction(ctx, "operator", ban)
	if err != nil || permanent.ExpiresAt != nil || !permanent.Active {
		t.Fatal(permanent, err)
	}
	state, err := st.ActiveSanctionState(ctx, name)
	if err != nil || !state.BanUntil.Equal(PermanentSanction) || !state.MuteUntil.After(time.Now().Add(59*time.Minute)) {
		t.Fatal(state, err)
	}
	if _, _, err := st.RevokeSanction(ctx, permanent.ID, name, control.SanctionMute, "operator", "wrong kind"); !errors.Is(err, control.ErrUnknownSanction) {
		t.Fatal("revoked through the wrong capability kind", err)
	}
	revoked, changed, err := st.RevokeSanction(ctx, permanent.ID, name, control.SanctionBan, "operator", "appeal accepted")
	if err != nil || !changed || revoked.Active || revoked.RevokedBy != "operator" || revoked.RevokedAt == nil {
		t.Fatal(revoked, err)
	}
	if _, changed, err := st.RevokeSanction(ctx, permanent.ID, name, control.SanctionBan, "operator", "again"); err != nil || changed {
		t.Fatal("second revoke changed the row", err)
	}
	state, err = st.ActiveSanctionState(ctx, name)
	if err != nil || !state.BanUntil.IsZero() || state.MuteUntil.IsZero() {
		t.Fatal(state, err)
	}
	list, _, err := st.ListSanctions(ctx, name, 50)
	if err != nil || len(list) != 2 || list[0].ID != permanent.ID || list[0].Active || !list[1].Active {
		t.Fatal(list, err)
	}
	if _, _, err := st.ListSanctions(ctx, "nosuchacct1", 50); !IsAccountMissing(err) {
		t.Fatal(err)
	}
	if _, err := st.pool.Exec(ctx, `UPDATE account_sanctions SET expires_at=now() - interval '1 second' WHERE id=$1`, first.ID); err != nil {
		t.Fatal(err)
	}
	if state, err := st.ActiveSanctionState(ctx, name); err != nil || !state.MuteUntil.IsZero() {
		t.Fatal("expired mute still active", state, err)
	}
}

func TestPostgresStaffAuditHistory(t *testing.T) {
	st, _ := criticalAccountsPostgresFixture(t)
	ctx := context.Background()
	actor := fmt.Sprintf("au%010d", time.Now().UnixNano()%10_000_000_000)
	t.Cleanup(func() { _, _ = st.pool.Exec(context.Background(), `DELETE FROM staff_audit WHERE actor=$1`, actor) })
	for i := range 3 {
		if err := st.AppendStaffAudit(ctx, control.AuditEntry{Actor: actor, Action: "server.notice", Result: "ok",
			RequestID: fmt.Sprint("request-", i), ClientIP: "127.0.0.1", Method: "POST", Path: "/api/v1/staff/notices",
			Details: map[string]any{"index": i, "message": "Hello"}}); err != nil {
			t.Fatal(err)
		}
	}
	if err := st.AppendStaffAudit(ctx, control.AuditEntry{Actor: actor}); err == nil {
		t.Fatal("entry without action accepted")
	}
	page, err := st.ListStaffAudit(ctx, control.AuditQuery{Actor: actor, Limit: 2})
	if err != nil || len(page.Entries) != 2 || page.NextBefore == 0 || page.Entries[0].Details["index"] != float64(2) {
		t.Fatal(page, err)
	}
	next, err := st.ListStaffAudit(ctx, control.AuditQuery{Actor: actor, Action: "server.notice", Before: page.NextBefore, Limit: 2})
	if err != nil || len(next.Entries) != 1 || next.NextBefore != 0 || next.Entries[0].Details["index"] != float64(0) {
		t.Fatal(next, err)
	}
	if empty, err := st.ListStaffAudit(ctx, control.AuditQuery{Actor: actor, Action: "server.shutdown", Limit: 10}); err != nil || len(empty.Entries) != 0 {
		t.Fatal(empty, err)
	}
	// Age two rows past retention; pruning removes only those, oldest first.
	if _, err := st.pool.Exec(ctx, `UPDATE staff_audit SET created_at = now() - interval '400 days'
		WHERE actor=$1 AND id IN (SELECT id FROM staff_audit WHERE actor=$1 ORDER BY id LIMIT 2)`, actor); err != nil {
		t.Fatal(err)
	}
	cutoff := time.Now().Add(-365 * 24 * time.Hour)
	if n, err := st.PruneStaffAudit(ctx, cutoff, 1); err != nil || n != 1 {
		t.Fatal(n, err)
	}
	if n, err := st.PruneStaffAudit(ctx, cutoff, 500); err != nil || n != 1 {
		t.Fatal(n, err)
	}
	if n, err := st.PruneStaffAudit(ctx, cutoff, 500); err != nil || n != 0 {
		t.Fatal(n, err)
	}
	left, err := st.ListStaffAudit(ctx, control.AuditQuery{Actor: actor, Limit: 10})
	if err != nil || len(left.Entries) != 1 || left.Entries[0].Details["index"] != float64(2) {
		t.Fatal(left, err)
	}
	if _, err := st.PruneStaffAudit(ctx, cutoff, 0); err == nil {
		t.Fatal("unbounded prune accepted")
	}
}
