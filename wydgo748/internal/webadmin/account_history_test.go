package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/logreader"
)

type testHistory struct{ pruned int }

func (testHistory) ListAccountLogins(_ context.Context, account string, _ int) ([]control.AccountLogin, time.Time, error) {
	return []control.AccountLogin{{At: time.Now(), ClientIP: "203.0.113.7", Channel: 1}}, time.Now(), nil
}

func (testHistory) ListStaffAuditForAccount(_ context.Context, account string, _ int) ([]control.AuditEntry, time.Time, error) {
	return []control.AuditEntry{{ID: 3, Action: "account.sanction", Result: "ok", Details: map[string]any{"target_account": account}}}, time.Now(), nil
}

func (h *testHistory) PruneAccountLogins(context.Context, time.Time, int) (int64, error) {
	h.pruned++
	return 0, nil
}

type testStorage struct{}

func (testStorage) ReadCharacterStorage(_ context.Context, account, uid string) (control.CharacterStorage, error) {
	return control.CharacterStorage{Version: 1, Account: account, UID: uid, Equipment: []control.StoredItem{{Slot: 6, ItemID: 801}},
		Inventory: []control.StoredItem{{Slot: 0, ItemID: 400}}, Cargo: []control.StoredItem{}}, nil
}

type testLogs struct {
	request logreader.Request
	reply   logreader.Response
}

func (l *testLogs) Do(_ context.Context, r logreader.Request) (logreader.Response, error) {
	l.request = r
	return l.reply, nil
}

func TestAccountPageEndpoints(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	h.cfg.AccountHistory, h.cfg.Storage = &testHistory{}, testStorage{}
	h.cfg.ItemName = func(id uint16) string { return map[uint16]string{801: "Sword_of_Test"}[id] }
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability}}
	cookie, _, _ := loginTest(t, h)
	r := request(h, "GET", "/api/v1/staff/accounts/fixture/logins", "", cookie, "")
	var logins control.AccountLogins
	if r.Code != 200 || json.Unmarshal(r.Body.Bytes(), &logins) != nil || logins.IPVisible || logins.Logins[0].ClientIP != "" {
		t.Fatal("IP leaked without capability", r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/accounts/fixture/actions", "", cookie, ""); r.Code != 403 {
		t.Fatal("staff actions need staff.audit.read", r.Code)
	}
	r = request(h, "GET", "/api/v1/staff/players/fixture-1/storage?account=Fixture", "", cookie, "")
	if r.Code != 200 || !strings.Contains(r.Body.String(), `"name":"Sword of Test"`) || !strings.Contains(r.Body.String(), `"account":"fixture"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/players/fixture-1/storage?account=bad%20name", "", cookie, ""); r.Code != 400 {
		t.Fatal(r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.AccountIPCapability, control.AuditReadCapability}}
	cookie, _, _ = loginTest(t, h)
	r = request(h, "GET", "/api/v1/staff/accounts/fixture/logins", "", cookie, "")
	if json.Unmarshal(r.Body.Bytes(), &logins) != nil || !logins.IPVisible || logins.Logins[0].ClientIP != "203.0.113.7" {
		t.Fatal(r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/accounts/fixture/actions", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), "account.sanction") {
		t.Fatal(r.Code, r.Body.String())
	}
}

func TestServerLogsAndBackupsUseOnlyTheHelper(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	logs := &testLogs{reply: logreader.Response{Version: 1, Entries: []logreader.Entry{{Cursor: "c1", Message: "LOGIN"}}, NextCursor: "c1"}}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability}}
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/logs?source=game", "", cookie, ""); r.Code != 403 {
		t.Fatal("server.status must not grant logs", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.LogsCapability}}
	cookie, _, _ = loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/logs?source=game", "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "logs_unavailable") {
		t.Fatal("logs without a helper", r.Code)
	}
	h.cfg.Logs = logs
	for _, bad := range []string{"source=sshd", "source=game&limit=501", "source=game&since=yesterday", "source=game&priority=9"} {
		if r := request(h, "GET", "/api/v1/staff/logs?"+bad, "", cookie, ""); r.Code != 400 {
			t.Fatal("accepted", bad, r.Code)
		}
	}
	r := request(h, "GET", "/api/v1/staff/logs?source=player&query=LOGIN&limit=50&priority=3", "", cookie, "")
	if r.Code != 200 || logs.request.Source != "player" || logs.request.Limit != 50 || logs.request.Priority != 3 ||
		logs.request.Until.Sub(logs.request.Since) != time.Hour || !strings.Contains(r.Body.String(), `"nextCursor":"c1"`) {
		t.Fatal(r.Code, r.Body.String(), logs.request)
	}
	logs.reply = logreader.Response{Version: 1, Error: "cursor_expired"}
	if r := request(h, "GET", "/api/v1/staff/logs?source=game&cursor=c1", "", cookie, ""); r.Code != 410 {
		t.Fatal(r.Code)
	}
	at := time.Date(2026, 10, 22, 4, 15, 0, 0, time.UTC)
	logs.reply = logreader.Response{Version: 1, Backup: &logreader.Receipt{At: at, Archive: "wydgo-x.dump", Bytes: 9}}
	if r := request(h, "GET", "/api/v1/staff/backups", "", cookie, ""); r.Code != 200 || logs.request.Kind != logreader.KindBackup ||
		!strings.Contains(r.Body.String(), "wydgo-x.dump") || !strings.Contains(r.Body.String(), `"restoreTest":null`) {
		t.Fatal(r.Code, r.Body.String())
	}
}

func TestLoginRetentionRunsWithAuditRetention(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	history := &testHistory{}
	h.cfg.AccountHistory = history
	if _, more := h.pruneLogins(); more || history.pruned != 1 {
		t.Fatal(history.pruned)
	}
}
