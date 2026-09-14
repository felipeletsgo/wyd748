package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

type testQuiz struct {
	called        int
	actor         string
	beforeExecute func()
}

func (f *testQuiz) QuizStatus(context.Context) (control.QuizStatus, error) {
	return control.QuizStatus{Version: 1, Epoch: strings.Repeat("a", 32), AsOf: time.Now()}, nil
}
func (f *testQuiz) QuizCommand(_ context.Context, actor string, c control.QuizCommand, authorize func() bool) (control.QuizResult, error) {
	f.called++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	code := "ok"
	if !authorize() {
		code = "capability_required"
	}
	return control.QuizResult{OperationID: c.OperationID, Code: code}, nil
}

func TestQuizHTTPAuthCSRFAndRevocation(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	f := &testQuiz{}
	h.cfg.Quiz = f
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.QuizCapability}}
	cookie, session, _ := loginTest(t, h)
	c := control.QuizCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), Action: "start", Reason: "test weekend", ItemID: 500, RewardQuantity: 2, DurationSeconds: 120}
	body, _ := json.Marshal(c)
	path := "/api/v1/staff/events/quiz"
	if r := request(h, "GET", path, "", nil, ""); r.Code != 401 {
		t.Fatal(r.Code)
	}
	if r := request(h, "POST", path, string(body), cookie, ""); r.Code != 403 || f.called != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{"actor":"administrator"}`, string(body) + ` {}`, strings.Replace(string(body), `"rewardQuantity":2`, `"rewardQuantity":61`, 1)} {
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

func TestQuizHTTPReadOnlyAndExpiry(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	cookie, _, _ := loginTest(t, h)
	path := "/api/v1/staff/events/quiz"
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("read-only staff allowed", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.QuizCapability}}
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "events_unavailable") {
		t.Fatal("standalone falsely advertised writes", r.Code)
	}
	f := &testQuiz{}
	h.cfg.Quiz = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 {
		t.Fatal("status", r.Code)
	}
	c := control.QuizCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), Action: "start", Reason: "test event", ItemID: 500, RewardQuantity: 1, DurationSeconds: 60}
	body, _ := json.Marshal(c)
	f.beforeExecute = func() { now := h.now().Add(16 * time.Minute); h.now = func() time.Time { return now } }
	if r := request(h, "POST", path, string(body), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("expired queued session accepted", r.Code, r.Body.String())
	}
}
