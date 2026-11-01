package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testPersistenceRecovery struct {
	called        int
	actor         string
	beforeExecute func()
}

func (f *testPersistenceRecovery) PersistenceRecoveryStatus(context.Context) (control.PersistenceRecoveryStatus, error) {
	return control.PersistenceRecoveryStatus{Version: 1, AsOf: time.Now(), Quarantined: []control.QuarantinedOperation{
		{ID: 7, Channel: 1, Since: time.Now(), Accounts: []string{"fenced"}, Checkable: true},
	}}, nil
}

func (f *testPersistenceRecovery) PersistenceRecheck(_ context.Context, actor string, c control.PersistenceRecheckCommand,
	authorize func() bool) (control.PersistenceRecheckResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	}
	return control.PersistenceRecheckResult{OperationID: c.OperationID, Code: code}, nil
}

func TestPersistenceRecoveryHTTPAuthCSRFAndRevocation(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	path := "/api/v1/staff/persistence/quarantine"
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("staff without the capability read the quarantine", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.PersistenceRecoveryCapability}}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "persistence_recovery_unavailable") {
		t.Fatal("standalone falsely advertised recovery", r.Code)
	}
	f := &testPersistenceRecovery{}
	h.cfg.PersistenceRecovery = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"accounts":["fenced"]`) {
		t.Fatal("status", r.Code, r.Body.String())
	}
	c := control.PersistenceRecheckCommand{Version: 1, OperationID: "operation-0000001", Reason: "support ticket"}
	body, _ := json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"unlock":true}`, string(body) + ` {}`, strings.Replace(string(body), `"support ticket"`, `"x"`, 1)} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.called != 0 {
			t.Fatal("invalid command forwarded", r.Code, r.Body.String())
		}
	}
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" ||
		!strings.Contains(r.Body.String(), `"code":"ok"`) {
		t.Fatal("recheck failed", r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "support ticket") || !strings.Contains(logs.String(), "persistence_recovery.recheck") {
		t.Fatal("audit missing reason/action")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	c.OperationID = "operation-0000002"
	body, _ = json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("queued revocation bypass", r.Code, r.Body.String())
	}
}
