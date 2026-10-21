package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testPvPDoor struct {
	called        int
	actor         string
	locked        bool
	beforeExecute func()
}

func (f *testPvPDoor) PvPDoorStatus(context.Context) (control.PvPDoorStatus, error) {
	return control.PvPDoorStatus{Version: 1, AsOf: time.Now(), Available: true, Locked: f.locked, X: 2123, Y: 2143}, nil
}
func (f *testPvPDoor) PvPDoorCommand(_ context.Context, actor string, c control.PvPDoorCommand, authorize func() bool) (control.PvPDoorResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	} else {
		f.locked = c.Action == "lock"
	}
	return control.PvPDoorResult{OperationID: c.OperationID, Code: code}, nil
}

func TestPvPDoorHTTPAuthCSRFAndRevocation(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	path := "/api/v1/staff/world/pvp-door"
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("staff without the capability read the door", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.PvPDoorCapability}}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "pvp_door_unavailable") {
		t.Fatal("standalone falsely advertised the door", r.Code)
	}
	f := &testPvPDoor{}
	h.cfg.PvPDoor = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"available":true`) {
		t.Fatal("status", r.Code, r.Body.String())
	}
	c := control.PvPDoorCommand{Version: 1, OperationID: "operation-0000001", Action: "lock", Reason: "pvp tournament"}
	body, _ := json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"locked":true}`, string(body) + ` {}`, strings.Replace(string(body), `"lock"`, `"close"`, 1),
		strings.Replace(string(body), `"pvp tournament"`, `"x"`, 1)} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.called != 0 {
			t.Fatal("invalid command forwarded", r.Code, r.Body.String())
		}
	}
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" || !f.locked {
		t.Fatal("lock failed", r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "pvp tournament") || !strings.Contains(logs.String(), "pvp_door.lock") {
		t.Fatal("audit missing reason/action")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	c.Action, c.OperationID = "open", "operation-0000002"
	body, _ = json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") || !f.locked {
		t.Fatal("queued revocation bypass", r.Code, r.Body.String())
	}
}
