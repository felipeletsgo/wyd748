package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testOXEvent struct {
	called        int
	actor         string
	locked        bool
	beforeExecute func()
}

func (f *testOXEvent) OXEventStatus(context.Context) (control.OXEventStatus, error) {
	return control.OXEventStatus{Version: 1, AsOf: time.Now(), QuizMode: f.locked, Doors: 6}, nil
}
func (f *testOXEvent) OXEventCommand(_ context.Context, actor string, c control.OXEventCommand, authorize func() bool) (control.OXEventResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	} else {
		f.locked = c.Action == "mode_on"
	}
	return control.OXEventResult{OperationID: c.OperationID, Code: code}, nil
}

func TestOXEventHTTPAuthCSRFAndRevocation(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	path := "/api/v1/staff/events/ox"
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("staff without the capability read the arena", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.OXEventCapability}}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "ox_event_unavailable") {
		t.Fatal("standalone falsely advertised the arena", r.Code)
	}
	f := &testOXEvent{}
	h.cfg.OXEvent = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"doors":6`) {
		t.Fatal("status", r.Code, r.Body.String())
	}
	c := control.OXEventCommand{Version: 1, OperationID: "operation-0000001", Action: "mode_on", Reason: "ox tournament"}
	body, _ := json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"locked":true}`, string(body) + ` {}`, strings.Replace(string(body), `"mode_on"`, `"quiz"`, 1),
		strings.Replace(string(body), `"ox tournament"`, `"x"`, 1)} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.called != 0 {
			t.Fatal("invalid command forwarded", r.Code, r.Body.String())
		}
	}
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" || !f.locked {
		t.Fatal("lock failed", r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "ox tournament") || !strings.Contains(logs.String(), "ox_event.mode_on") {
		t.Fatal("audit missing reason/action")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	c.Action, c.OperationID = "mode_off", "operation-0000002"
	body, _ = json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") || !f.locked {
		t.Fatal("queued revocation bypass", r.Code, r.Body.String())
	}
}
