package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"
	"wydgo/internal/control"
)

type testTeleport struct {
	called        int
	actor         string
	beforeExecute func()
}

func (f *testTeleport) TeleportCommand(_ context.Context, actor string, c control.TeleportCommand, authorize func() bool) (control.TeleportResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	}
	return control.TeleportResult{OperationID: c.OperationID, Code: code}, nil
}
func teleportHTTPBody() string {
	b, _ := json.Marshal(control.TeleportCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), UID: "char-1", Account: "player", ExpectedSessionID: 77, Destination: "armia", Reason: "moderation test"})
	return string(b)
}
func TestTeleportHTTPBoundaries(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	f := &testTeleport{}
	h.cfg.Teleport = f
	path := "/api/v1/staff/players/char-1/teleport"
	body := teleportHTTPBody()
	if r := request(h, "POST", path, body, nil, ""); r.Code != 401 {
		t.Fatal(r.Code)
	}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "POST", path, body, cookie, session.CSRF); r.Code != 403 {
		t.Fatal("read-only allowed", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.TeleportCapability}}
	cookie, session, _ = loginTest(t, h)
	if r := request(h, "POST", path, body, cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"actor":"administrator"}`, body + ` {}`, strings.Replace(body, `"expectedSessionId":77`, `"expectedSessionId":0`, 1), strings.Replace(body, `"uid":"char-1"`, `"uid":"char-2"`, 1)} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.called != 0 {
			t.Fatal("invalid command forwarded", r.Code, r.Body.String())
		}
	}
	if r := request(h, "POST", path, body, cookie, session.CSRF); r.Code != 200 || f.actor != "operator" || !strings.Contains(r.Body.String(), `"code":"ok"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "moderation test") || !strings.Contains(logs.String(), "operation-0000001") || !strings.Contains(logs.String(), "char-1") {
		t.Fatal("audit missing")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	if r := request(h, "POST", path, body, cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("queued revocation bypass", r.Code)
	}
}
func TestTeleportHTTPUnavailableAndQueuedExpiry(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.TeleportCapability}}
	cookie, session, _ := loginTest(t, h)
	path := "/api/v1/staff/players/char-1/teleport"
	if r := request(h, "POST", path, teleportHTTPBody(), cookie, session.CSRF); r.Code != 503 || !strings.Contains(r.Body.String(), "moderation_unavailable") {
		t.Fatal(r.Code, r.Body.String())
	}
	f := &testTeleport{beforeExecute: func() { now := h.now().Add(16 * time.Minute); h.now = func() time.Time { return now } }}
	h.cfg.Teleport = f
	if r := request(h, "POST", path, teleportHTTPBody(), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("queued expiry bypass", r.Code, r.Body.String())
	}
}
