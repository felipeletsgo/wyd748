package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testColosseum struct {
	called        int
	actor         string
	locked        bool
	beforeExecute func()
}

func (f *testColosseum) ColosseumStatus(context.Context) (control.ColosseumStatus, error) {
	return control.ColosseumStatus{Version: 1, AsOf: time.Now(), Active: f.locked, Epoch: strings.Repeat("a", 32)}, nil
}
func (f *testColosseum) ColosseumCommand(_ context.Context, actor string, c control.ColosseumCommand, authorize func() bool) (control.ColosseumResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	} else {
		f.locked = c.Tier == "N"
	}
	return control.ColosseumResult{OperationID: c.OperationID, Code: code}, nil
}

func TestColosseumHTTPAuthCSRFAndRevocation(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	path := "/api/v1/staff/events/colosseum"
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("staff without the capability read the arena", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.ColosseumCapability}}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "colosseum_unavailable") {
		t.Fatal("standalone falsely advertised the arena", r.Code)
	}
	f := &testColosseum{}
	h.cfg.Colosseum = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"epoch":"`) {
		t.Fatal("status", r.Code, r.Body.String())
	}
	c := control.ColosseumCommand{Version: 1, OperationID: "operation-0000001", Tier: "N", Epoch: strings.Repeat("a", 32), ExpectedRevision: 1, Reason: "early tournament"}
	body, _ := json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"locked":true}`, string(body) + ` {}`, strings.Replace(string(body), `"N"`, `"Z"`, 1),
		strings.Replace(string(body), `"early tournament"`, `"x"`, 1)} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.called != 0 {
			t.Fatal("invalid command forwarded", r.Code, r.Body.String())
		}
	}
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" || !f.locked {
		t.Fatal("lock failed", r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "early tournament") || !strings.Contains(logs.String(), "colosseum.start") {
		t.Fatal("audit missing reason/action")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	c.Tier, c.OperationID = "M", "operation-0000002"
	body, _ = json.Marshal(c)
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") || !f.locked {
		t.Fatal("queued revocation bypass", r.Code, r.Body.String())
	}
}
