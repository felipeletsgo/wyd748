package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testGlobalDrop struct {
	called        int
	actor         string
	beforeExecute func()
}

func (f *testGlobalDrop) GlobalDropStatus(context.Context) (control.GlobalDropStatus, error) {
	return control.GlobalDropStatus{Version: 1, Epoch: strings.Repeat("a", 32), AsOf: time.Now()}, nil
}
func (f *testGlobalDrop) GlobalDropCommand(_ context.Context, actor string, c control.GlobalDropCommand, authorize func() bool) (control.GlobalDropResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	}
	return control.GlobalDropResult{OperationID: c.OperationID, Code: code}, nil
}

func TestGlobalDropHTTPAuthCSRFAndRevocation(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	f := &testGlobalDrop{}
	h.cfg.GlobalDrop = f
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.GlobalDropCapability}}
	cookie, session, _ := loginTest(t, h)
	c := control.GlobalDropCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), Action: "start", Reason: "test weekend", ItemID: 500, RatePercent: 50, MaxDrops: 10000}
	body, _ := json.Marshal(c)
	path := "/api/v1/staff/events/global-drop"
	if r := request(h, "GET", path, "", nil, ""); r.Code != 401 {
		t.Fatal(r.Code)
	}
	if r := request(h, "POST", path, string(body), cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"actor":"administrator"}`, string(body) + ` {}`, strings.Replace(string(body), `"ratePercent":50`, `"ratePercent":101`, 1)} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.called != 0 {
			t.Fatal("invalid JSON forwarded", r.Code, r.Body.String())
		}
	}
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" || !strings.Contains(r.Body.String(), `"code":"ok"`) {
		t.Fatal("command failed", r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "test weekend") || !strings.Contains(logs.String(), "operation-0000001") {
		t.Fatal("audit missing reason/id")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("queued revocation bypass", r.Code, r.Body.String())
	}
}

func TestGlobalDropHTTPReadOnlyAndExpiry(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	cookie, _, _ := loginTest(t, h)
	path := "/api/v1/staff/events/global-drop"
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("read-only staff allowed", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.GlobalDropCapability}}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "events_unavailable") {
		t.Fatal("standalone falsely advertised writes", r.Code)
	}
	f := &testGlobalDrop{}
	h.cfg.GlobalDrop = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 {
		t.Fatal("status", r.Code)
	}
	c := control.GlobalDropCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), Action: "start", Reason: "test event", ItemID: 500, RatePercent: 100, MaxDrops: 1}
	body, _ := json.Marshal(c)
	f.beforeExecute = func() { now := h.now().Add(16 * time.Minute); h.now = func() time.Time { return now } }
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("expired queued session accepted", r.Code, r.Body.String())
	}
}
