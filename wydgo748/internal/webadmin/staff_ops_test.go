package webadmin

import (
	"context"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testStaffOps struct {
	sanctions []control.SanctionCommand
	revoked   []int64
	notices   []control.NoticeCommand
	messages  []control.PlayerMessageCommand
	shutdowns []control.ShutdownCommand
	actor     string
	authorize func() bool
}

func (f *testStaffOps) AccountSanctions(_ context.Context, account string) (control.SanctionsView, error) {
	if account == "missing" {
		return control.SanctionsView{}, control.ErrUnknownAccount
	}
	return control.SanctionsView{Version: 1, Account: account, Sanctions: []control.Sanction{}}, nil
}

func (f *testStaffOps) ApplySanction(_ context.Context, actor string, cmd control.SanctionCommand, authorize func() bool) (control.SanctionResult, error) {
	f.actor, f.authorize = actor, authorize
	if !authorize() {
		return control.SanctionResult{Code: "capability_required"}, nil
	}
	f.sanctions = append(f.sanctions, cmd)
	return control.SanctionResult{Code: "ok", Live: "offline", Sanction: control.Sanction{ID: 7, Account: cmd.Account, Kind: cmd.Kind}}, nil
}

func (f *testStaffOps) RevokeSanction(_ context.Context, actor string, id int64, _ control.SanctionRevokeCommand, authorize func() bool) (control.SanctionResult, error) {
	f.actor = actor
	if !authorize() {
		return control.SanctionResult{Code: "capability_required"}, nil
	}
	f.revoked = append(f.revoked, id)
	return control.SanctionResult{Code: "ok", Live: "offline"}, nil
}

func (f *testStaffOps) Notices(context.Context) (control.NoticesView, error) {
	return control.NoticesView{Version: 1, Notices: []control.ScheduledNotice{}}, nil
}

func (f *testStaffOps) ScheduleNotice(_ context.Context, actor string, cmd control.NoticeCommand, authorize func() bool) (control.NoticeResult, error) {
	f.actor = actor
	if !authorize() {
		return control.NoticeResult{OperationID: cmd.OperationID, Code: "capability_required"}, nil
	}
	f.notices = append(f.notices, cmd)
	return control.NoticeResult{OperationID: cmd.OperationID, Code: "ok", Delivered: 3}, nil
}

func (f *testStaffOps) CancelNotice(_ context.Context, _ string, _ int64, cmd control.NoticeCancelCommand, _ func() bool) (control.NoticeResult, error) {
	return control.NoticeResult{OperationID: cmd.OperationID, Code: "unknown_notice"}, nil
}

func (f *testStaffOps) MessagePlayer(_ context.Context, _ string, cmd control.PlayerMessageCommand, _ func() bool) (control.PlayerMessageResult, error) {
	f.messages = append(f.messages, cmd)
	return control.PlayerMessageResult{OperationID: cmd.OperationID, Code: "ok"}, nil
}

func (f *testStaffOps) ShutdownStatus(context.Context) (control.ShutdownStatus, error) {
	return control.ShutdownStatus{Version: 1, Available: true}, nil
}

func (f *testStaffOps) ShutdownCommand(_ context.Context, _ string, cmd control.ShutdownCommand, authorize func() bool) (control.ShutdownResult, error) {
	if !authorize() {
		return control.ShutdownResult{OperationID: cmd.OperationID, Code: "capability_required"}, nil
	}
	f.shutdowns = append(f.shutdowns, cmd)
	return control.ShutdownResult{OperationID: cmd.OperationID, Code: "ok", Status: control.ShutdownStatus{Version: 1, Scheduled: true}}, nil
}

type testAuditStore struct {
	mu       sync.Mutex
	entries  []control.AuditEntry
	query    control.AuditQuery
	old      int
	pruned   []time.Time
	pruneErr error
}

func (s *testAuditStore) PruneStaffAudit(_ context.Context, cutoff time.Time, limit int) (int64, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.pruned = append(s.pruned, cutoff)
	if s.pruneErr != nil {
		return 0, s.pruneErr
	}
	n := min(s.old, limit)
	s.old -= n
	return int64(n), nil
}

func (s *testAuditStore) AppendStaffAudit(_ context.Context, entry control.AuditEntry) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.entries = append(s.entries, entry)
	return nil
}

func (s *testAuditStore) ListStaffAudit(_ context.Context, q control.AuditQuery) (control.AuditPage, error) {
	s.query = q
	return control.AuditPage{Version: 1, Entries: []control.AuditEntry{}}, nil
}

func jsonBody(value any) string {
	b, _ := json.Marshal(value)
	return string(b)
}

func staffOpsHandler(t *testing.T, capabilities ...string) (*Handler, map[string]Staff, *testStaffOps) {
	h, entries, _, _ := newTestHandler(t)
	f := &testStaffOps{}
	h.cfg.Sanctions, h.cfg.Notices, h.cfg.Shutdown = f, f, f
	entries["operator"] = Staff{Capabilities: append([]string{StatusCapability}, capabilities...)}
	return h, entries, f
}

func TestSanctionHTTPCapabilityPerKind(t *testing.T) {
	h, entries, f := staffOpsHandler(t, control.MuteCapability)
	cookie, session, _ := loginTest(t, h)
	path := "/api/v1/staff/accounts/player/sanctions"
	mute := control.SanctionCommand{Version: 1, RequestID: "request-000000001", Account: "player", Kind: control.SanctionMute, DurationMinutes: 30, Reason: "spam in global"}
	ban := mute
	ban.Kind, ban.RequestID = control.SanctionBan, "request-000000002"
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"account":"player"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/accounts/missing/sanctions", "", cookie, ""); r.Code != 404 {
		t.Fatal(r.Code)
	}
	if r := request(h, "POST", path, jsonBody(mute), cookie, ""); r.Code != 403 || len(f.sanctions) != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	if r := request(h, "POST", path, jsonBody(ban), cookie, session.CSRF); r.Code != 403 || len(f.sanctions) != 0 {
		t.Fatal("mute capability banned an account", r.Code)
	}
	for _, bad := range []string{jsonBody(mute) + " {}", strings.Replace(jsonBody(mute), `"account":"player"`, `"account":"other"`, 1),
		strings.Replace(jsonBody(mute), `"durationMinutes":30`, `"durationMinutes":-1`, 1), `{"actor":"admin"}`} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || len(f.sanctions) != 0 {
			t.Fatal("invalid sanction forwarded", r.Code, bad)
		}
	}
	if r := request(h, "POST", path, jsonBody(mute), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" || len(f.sanctions) != 1 {
		t.Fatal(r.Code, r.Body.String())
	}
	// The execution-time re-check follows a staff file change.
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.MuteCapability, control.BanCapability}}
	if f.authorize() {
		t.Fatal("changed staff entry still authorized")
	}
	revoke := control.SanctionRevokeCommand{Version: 1, Account: "player", Kind: control.SanctionBan, Reason: "appeal accepted"}
	cookie, session, _ = loginTest(t, h)
	for _, bad := range []string{"/api/v1/staff/accounts/player/sanctions/0/revoke", "/api/v1/staff/accounts/player/sanctions/01/revoke"} {
		if r := request(h, "POST", bad, jsonBody(revoke), cookie, session.CSRF); r.Code != 400 {
			t.Fatal("invalid sanction id accepted", bad, r.Code)
		}
	}
	if r := request(h, "POST", "/api/v1/staff/accounts/player/sanctions/7/revoke", jsonBody(revoke), cookie, session.CSRF); r.Code != 200 || len(f.revoked) != 1 || f.revoked[0] != 7 {
		t.Fatal(r.Code, r.Body.String())
	}
}

func TestNoticeAndShutdownHTTPBoundaries(t *testing.T) {
	h, _, f := staffOpsHandler(t, control.NoticeCapability)
	cookie, session, _ := loginTest(t, h)
	notice := control.NoticeCommand{Version: 1, OperationID: "operation-0000001", Message: "Double EXP weekend!", RepeatCount: 1}
	if r := request(h, "POST", "/api/v1/staff/notices", jsonBody(notice), cookie, session.CSRF); r.Code != 200 || len(f.notices) != 1 || !strings.Contains(r.Body.String(), `"delivered":3`) {
		t.Fatal(r.Code, r.Body.String())
	}
	notice.Message = strings.Repeat("x", control.MaxNoticeBytes+1)
	if r := request(h, "POST", "/api/v1/staff/notices", jsonBody(notice), cookie, session.CSRF); r.Code != 400 || len(f.notices) != 1 {
		t.Fatal("oversized notice forwarded", r.Code)
	}
	message := control.PlayerMessageCommand{Version: 1, OperationID: "operation-0000002", Epoch: strings.Repeat("a", 32), UID: "char-1",
		Account: "player", ExpectedSessionID: 5, Message: "Please contact staff."}
	if r := request(h, "POST", "/api/v1/staff/players/char-2/message", jsonBody(message), cookie, session.CSRF); r.Code != 400 || len(f.messages) != 0 {
		t.Fatal("path/body target mismatch forwarded", r.Code)
	}
	if r := request(h, "POST", "/api/v1/staff/players/char-1/message", jsonBody(message), cookie, session.CSRF); r.Code != 200 || len(f.messages) != 1 {
		t.Fatal(r.Code, r.Body.String())
	}
	shutdown := control.ShutdownCommand{Version: 1, OperationID: "operation-0000003", Action: "schedule", Mode: control.ShutdownModeShutdown, DelaySeconds: 600, Reason: "database upgrade"}
	if r := request(h, "POST", "/api/v1/staff/server/shutdown", jsonBody(shutdown), cookie, session.CSRF); r.Code != 403 || len(f.shutdowns) != 0 {
		t.Fatal("notice capability scheduled a shutdown", r.Code)
	}
	h2, _, f2 := staffOpsHandler(t, control.ShutdownCapability)
	cookie, session, _ = loginTest(t, h2)
	if r := request(h2, "GET", "/api/v1/staff/server/shutdown", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"available":true`) {
		t.Fatal(r.Code, r.Body.String())
	}
	shutdown.DelaySeconds = 30
	if r := request(h2, "POST", "/api/v1/staff/server/shutdown", jsonBody(shutdown), cookie, session.CSRF); r.Code != 400 || len(f2.shutdowns) != 0 {
		t.Fatal("sub-minute shutdown forwarded", r.Code)
	}
	shutdown.DelaySeconds = 600
	if r := request(h2, "POST", "/api/v1/staff/server/shutdown", jsonBody(shutdown), cookie, session.CSRF); r.Code != 200 || len(f2.shutdowns) != 1 {
		t.Fatal(r.Code, r.Body.String())
	}
	h2.cfg.Shutdown = nil
	if r := request(h2, "GET", "/api/v1/staff/server/shutdown", "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "shutdown_unavailable") {
		t.Fatal(r.Code)
	}
}

func TestAuditPersistenceAndHistory(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	store := &testAuditStore{}
	h.cfg.AuditStore = store
	h.startAuditWriter()
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/audit", "", cookie, ""); r.Code != 403 {
		t.Fatal("audit read without capability", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.AuditReadCapability}}
	cookie, _, _ = loginTest(t, h)
	for _, bad := range []string{"?limit=0", "?limit=101", "?before=-1", "?actor=Bad%20Name", "?action=drop%3B"} {
		if r := request(h, "GET", "/api/v1/staff/audit"+bad, "", cookie, ""); r.Code != 400 {
			t.Fatal("invalid audit query accepted", bad, r.Code)
		}
	}
	if r := request(h, "GET", "/api/v1/staff/audit?actor=Operator&action=account.sanction&before=90&limit=20", "", cookie, ""); r.Code != 200 {
		t.Fatal(r.Code, r.Body.String())
	}
	if store.query != (control.AuditQuery{Actor: "operator", Action: "account.sanction", Before: 90, Limit: 20}) {
		t.Fatal(store.query)
	}
	h.Close()
	h.Close()
	store.mu.Lock()
	defer store.mu.Unlock()
	found := false
	for _, entry := range store.entries {
		raw, _ := json.Marshal(entry)
		if strings.Contains(string(raw), "TestPass10") || strings.Contains(string(raw), testAdminPIN) {
			t.Fatal("credential persisted in audit", string(raw))
		}
		if entry.Action == "staff.audit" && entry.Result == "forbidden" && entry.Actor == "operator" && entry.RequestID != "" && entry.ClientIP != "" {
			found = true
		}
	}
	if !found || len(store.entries) < 3 {
		t.Fatal("audit entries not persisted", len(store.entries))
	}
	// Audit after Close must neither panic nor block.
	request(h, "GET", "/api/v1/staff/audit", "", cookie, "")
}

func TestAuditDetailsAreBounded(t *testing.T) {
	details := auditDetails([]any{"reason", strings.Repeat("é", 400), "count", 3, 7, "skip", "err", context.Canceled, "dangling"})
	reason := details["reason"].(string)
	if len(reason) > maxAuditDetailBytes || !strings.HasPrefix(reason, "é") || strings.ContainsRune(reason, '�') {
		t.Fatal(len(reason))
	}
	if details["count"] != 3 || details["err"] != "context canceled" || len(details) != 3 {
		t.Fatal(details)
	}
}

func TestStaffFileAcceptsStaffOperationCapabilities(t *testing.T) {
	path := filepath.Join(t.TempDir(), "staff.json")
	caps := []string{control.BanCapability, control.MuteCapability, control.NoticeCapability, control.ShutdownCapability, control.AuditReadCapability}
	if err := os.WriteFile(path, []byte(jsonBody(map[string]Staff{"operator": {Capabilities: caps}})), 0o600); err != nil {
		t.Fatal(err)
	}
	entries, err := ReadStaff(path)
	if err != nil || len(entries["operator"].Capabilities) != len(caps) {
		t.Fatal(err)
	}
}

func TestAuditPersistsOnlyConsequentialEvents(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	store := &testAuditStore{}
	h.cfg.AuditStore = store
	h.startAuditWriter()
	cookie, _, _ := loginTest(t, h)
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability}}
	cookie, _, _ = loginTest(t, h)
	for range 5 {
		if r := request(h, "GET", "/api/v1/staff/overview", "", cookie, ""); r.Code != 200 {
			t.Fatal(r.Code)
		}
	}
	request(h, "GET", "/api/v1/staff/audit", "", cookie, "") // forbidden: still persisted
	h.RecordServerStart("executable_sha256", "abc", "channel_id", uint32(1))
	h.Close()
	store.mu.Lock()
	defer store.mu.Unlock()
	counts := map[string]int{}
	for _, entry := range store.entries {
		counts[entry.Action+"/"+entry.Result]++
	}
	if counts["overview/succeeded"] != 0 || counts["login/succeeded"] != 2 || counts["staff.audit/forbidden"] != 1 || counts["server.start/ok"] != 1 {
		t.Fatal(counts)
	}
	for _, entry := range store.entries {
		if entry.Action == "server.start" && (entry.Details["executable_sha256"] != "abc" || entry.Actor != "") {
			t.Fatal(entry)
		}
	}
}

func TestAuditRetentionPrunesInBoundedBatches(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	now := time.Date(2026, 10, 22, 4, 0, 0, 0, time.UTC)
	h.now = func() time.Time { return now }
	store := &testAuditStore{old: auditPruneBatch*auditPruneBatches + 7}
	h.cfg.AuditStore = store
	deleted, more := h.pruneAudit()
	if deleted != auditPruneBatch*auditPruneBatches || !more || len(store.pruned) != auditPruneBatches ||
		!store.pruned[0].Equal(now.Add(-auditRetention)) {
		t.Fatal(deleted, more, len(store.pruned))
	}
	if deleted, more := h.pruneAudit(); deleted != 7 || more {
		t.Fatal("backlog not finished", deleted, more)
	}
	store.pruneErr = context.DeadlineExceeded
	if _, more := h.pruneAudit(); more {
		t.Fatal("a failing prune must wait for the next daily run")
	}
}
