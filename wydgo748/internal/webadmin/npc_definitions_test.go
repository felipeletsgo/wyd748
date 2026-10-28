package webadmin

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"wydgo/internal/control"
)

type npcHTTPTestSource struct {
	saved       int
	revoke      func()
	lastCommand control.NPCDefinitionCommand
}

func (f *npcHTTPTestSource) NPCDefinitions(_ context.Context, c uint8) (control.NPCDefinitionCatalogView, error) {
	return control.NPCDefinitionCatalogView{Version: 1, Available: true, Channel: c, NPCs: []control.NPCDefinitionView{}}, nil
}
func (f *npcHTTPTestSource) SaveNPCDefinition(_ context.Context, _ string, c control.NPCDefinitionCommand, auth func() bool) (control.NPCDefinitionResult, error) {
	if f.revoke != nil {
		f.revoke()
	}
	if !auth() {
		return control.NPCDefinitionResult{Code: "capability_required"}, nil
	}
	f.saved++
	f.lastCommand = c
	return control.NPCDefinitionResult{Code: "ok"}, nil
}
func TestNPCPanelHTTPBoundaries(t *testing.T) {
	h, entries, _, _ := newTestHandler(t)
	f := &npcHTTPTestSource{}
	h.cfg.NPCDefinitions = f
	path := "/api/v1/staff/npcs"
	body, _ := json.Marshal(control.NPCDefinitionCommand{Version: 1, Channel: 1, ExpectedVersion: 1, Name: "Aki", Reason: "Audit edit", Definition: json.RawMessage(`{"name":"Aki"}`)})
	if r := request(h, "GET", path, "", nil, ""); r.Code != 401 {
		t.Fatal("anonymous access", r.Code)
	}
	cookie, s, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("capability bypass", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.NPCDefinitionCapability}}
	cookie, s, _ = loginTest(t, h)
	if r := request(h, "GET", path+"?channel=2", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"channel":2`) {
		t.Fatal("channel read", r.Code)
	}
	if r := request(h, "GET", path+"?channel=0", "", cookie, ""); r.Code != 400 {
		t.Fatal("bad channel", r.Code)
	}
	if r := request(h, "PUT", path+"/Aki", string(body), cookie, ""); r.Code != 403 {
		t.Fatal("CSRF bypass", r.Code)
	}
	if r := request(h, "PUT", path+"/Other", string(body), cookie, s.CSRF); r.Code != 400 {
		t.Fatal("identity bypass", r.Code)
	}
	if r := request(h, "PUT", path+"/Aki", string(body)+" {}", cookie, s.CSRF); r.Code != 400 {
		t.Fatal("trailing JSON bypass", r.Code)
	}
	if r := request(h, "PUT", path+"/Aki", string(body), cookie, s.CSRF); r.Code != 200 || f.saved != 1 {
		t.Fatal("valid update", r.Code, r.Body.String())
	}
	var batch control.NPCDefinitionCommand
	if err := json.Unmarshal(body, &batch); err != nil {
		t.Fatal(err)
	}
	batch.Targets = []control.NPCDefinitionTarget{{Channel: 1, ExpectedVersion: 1}, {Channel: 2, ExpectedVersion: 4}}
	batchBody, _ := json.Marshal(batch)
	if r := request(h, "PUT", path+"/Aki", string(batchBody), cookie, s.CSRF); r.Code != 200 || f.saved != 2 || len(f.lastCommand.Targets) != 2 {
		t.Fatal("explicit batch was not forwarded", r.Code, r.Body.String())
	}
	batch.Targets[1].Channel = 1
	invalidBatch, _ := json.Marshal(batch)
	if r := request(h, "PUT", path+"/Aki", string(invalidBatch), cookie, s.CSRF); r.Code != 400 || f.saved != 2 {
		t.Fatal("duplicate channel reached mutation", r.Code, r.Body.String())
	}
	f.revoke = func() { entries["operator"] = Staff{Capabilities: []string{StatusCapability}} }
	if r := request(h, "PUT", path+"/Aki", string(batchBody), cookie, s.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") || f.saved != 2 {
		t.Fatal("revocation bypass", r.Code, r.Body.String())
	}
	t.Log("PASS: authentication, capability, CSRF, channel, identity, strict JSON, successful mutation and execution-time revocation")
}
