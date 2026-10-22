package webadmin

import (
	"context"
	"io"
	"net/http/httptest"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/staffcontent"
)

type testWorldReads struct {
	calls int
	err   error
}

func (r *testWorldReads) MapSnapshot(_ context.Context, x, y uint16, radius int) (control.MapSnapshot, error) {
	r.calls++
	if r.err != nil {
		return control.MapSnapshot{}, r.err
	}
	return control.MapSnapshot{Version: 1, X: x, Y: y, Radius: radius, Players: []control.MapPlayer{}, Mobs: []control.MapMob{}}, nil
}

func (r *testWorldReads) Instances(context.Context) (control.InstancesView, error) {
	r.calls++
	return control.InstancesView{Version: 1, Instances: []control.InstanceStatus{}}, nil
}

type testNotes struct{ added []control.AccountNoteCommand }

func (n *testNotes) ListAccountNotes(context.Context, string, int) ([]control.AccountNote, time.Time, error) {
	return []control.AccountNote{}, time.Now(), nil
}

func (n *testNotes) AddAccountNote(_ context.Context, _ string, actor string, cmd control.AccountNoteCommand) (control.AccountNote, error) {
	n.added = append(n.added, cmd)
	return control.AccountNote{ID: 5, Actor: actor, Body: cmd.Body, BugRefs: cmd.BugRefs}, nil
}

func stage3Handler(t *testing.T) (*Handler, map[string]Staff) {
	h, entries, _, _ := newTestHandler(t)
	catalog := model.Catalog{Items: map[uint16]model.ItemDef{400: {Index: 400, Name: "Small_Potion"}}}
	h.cfg.Content = staffcontent.Build([]model.NPCDef{{Name: "Wolf", Tipo: model.TipoMonstro, Carry: []model.Item{{Index: 400}}}},
		nil, catalog, model.QuestFile{}, model.QuestZoneFile{}, [model.MaxCarry]int{}, nil, time.Now())
	h.cfg.Zones = staffcontent.NewZones(model.TerrainMap{}, []model.Teleport{{SourceX: 100, SourceY: 100, DestX: 1, DestY: 2, Name: "Gate"}},
		model.QuestZoneFile{}, model.VolatileCatalog{})
	return h, entries
}

func TestContentAndZoneEndpoints(t *testing.T) {
	h, entries := stage3Handler(t)
	entries["operator"] = Staff{Capabilities: []string{StatusCapability}}
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/content/search?q=potion", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"Small Potion"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/content/items/400", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"mob":"Wolf"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	for _, path := range []string{"/api/v1/staff/content/items/9999", "/api/v1/staff/content/items/x", "/api/v1/staff/content/mobs/Nobody"} {
		if r := request(h, "GET", path, "", cookie, ""); r.Code != 404 {
			t.Fatal(path, r.Code)
		}
	}
	if r := request(h, "GET", "/api/v1/staff/world/zone?x=101&y=102", "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"name":"Gate"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/world/zone?x=5000&y=1", "", cookie, ""); r.Code != 400 {
		t.Fatal(r.Code)
	}
	if r := request(h, "GET", "/api/v1/staff/world/terrain", "", cookie, ""); r.Code != 200 || r.Header().Get("Content-Type") != "image/png" || !strings.HasPrefix(r.Body.String(), "\x89PNG") {
		t.Fatal(r.Code, r.Header().Get("Content-Type"))
	}
	if r := request(h, "GET", "/api/v1/staff/content/issues", "", cookie, ""); r.Code != 200 {
		t.Fatal(r.Code)
	}
}

func TestMapAndInstancesAreOptionalBoundedReads(t *testing.T) {
	h, entries := stage3Handler(t)
	reads := &testWorldReads{}
	h.cfg.WorldReads = reads
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability}}
	cookie, _, _ := loginTest(t, h)
	if r := request(h, "GET", "/api/v1/staff/world/map?x=1&y=1&radius=129", "", cookie, ""); r.Code != 400 || reads.calls != 0 {
		t.Fatal(r.Code)
	}
	// Reads the World did not serve do not consume the quota.
	reads.err = control.ErrBusy
	for i := range 20 {
		if r := request(h, "GET", "/api/v1/staff/world/map?x=1&y=1&radius=32", "", cookie, ""); r.Code != 429 || !strings.Contains(r.Body.String(), "control_busy") {
			t.Fatal(i, r.Code, r.Body.String())
		}
	}
	reads.err = nil
	for i := range 12 {
		if r := request(h, "GET", "/api/v1/staff/world/map?x=1&y=1&radius=32", "", cookie, ""); r.Code != 200 {
			t.Fatal(i, r.Code)
		}
	}
	if r := request(h, "GET", "/api/v1/staff/world/map?x=1&y=1&radius=32", "", cookie, ""); r.Code != 429 {
		t.Fatal("map reads not limited", r.Code)
	}
	feed := &metricsFeed{reading: control.RuntimeReading{At: time.Now().UTC()}}
	h.cfg.Metrics = feed.read
	h.startMetricsSampler()
	defer h.Close()
	feed.advance(6, 10)
	h.sampler.observe(feed.reading)
	before := reads.calls
	if r := request(h, "GET", "/api/v1/staff/world/instances", "", cookie, ""); r.Code != 503 || reads.calls != before {
		t.Fatal("instances read under load", r.Code)
	}
}

func TestCaseNotesNeedModerationToWrite(t *testing.T) {
	h, entries := stage3Handler(t)
	notes := &testNotes{}
	h.cfg.Notes = notes
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability}}
	cookie, session, _ := loginTest(t, h)
	body := `{"version":1,"body":"Reported item loss, checked logs","bugRefs":["BUG-004"]}`
	if r := request(h, "GET", "/api/v1/staff/accounts/fixture/notes", "", cookie, ""); r.Code != 200 {
		t.Fatal(r.Code)
	}
	if r := request(h, "POST", "/api/v1/staff/accounts/fixture/notes", body, cookie, session.CSRF); r.Code != 403 || len(notes.added) != 0 {
		t.Fatal("note written without moderation permission", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability, control.MuteCapability}}
	cookie, session, _ = loginTest(t, h)
	for _, bad := range []string{`{"version":1,"body":"ok","bugRefs":[]}`, `{"version":1,"body":"valid note","bugRefs":["BUG-4"]}`,
		`{"version":1,"body":"valid\u0007note","bugRefs":[]}`} {
		if r := request(h, "POST", "/api/v1/staff/accounts/fixture/notes", bad, cookie, session.CSRF); r.Code != 400 {
			t.Fatal("accepted", bad, r.Code)
		}
	}
	if r := request(h, "POST", "/api/v1/staff/accounts/fixture/notes", body, cookie, session.CSRF); r.Code != 200 || len(notes.added) != 1 {
		t.Fatal(r.Code, r.Body.String())
	}
	// A revocation while the body is read is honored at the write.
	revoking := &revokeOnRead{body: strings.NewReader(body), revoke: func() {
		entries["operator"] = Staff{Capabilities: []string{StatusCapability, PlayersCapability}}
	}}
	r := httptest.NewRequest("POST", "https://admin.test/api/v1/staff/accounts/fixture/notes", revoking)
	r.RemoteAddr = "127.0.0.1:1234"
	r.Header.Set("Origin", "https://admin.test")
	r.Header.Set("Content-Type", "application/json")
	r.Header.Set("X-CSRF-Token", session.CSRF)
	r.AddCookie(cookie)
	recorder := httptest.NewRecorder()
	h.ServeHTTP(recorder, r)
	if recorder.Code != 403 || len(notes.added) != 1 {
		t.Fatal("note written after revocation", recorder.Code, recorder.Body.String())
	}
}

type revokeOnRead struct {
	body   io.Reader
	revoke func()
	done   bool
}

func (r *revokeOnRead) Read(p []byte) (int, error) {
	if !r.done {
		r.done = true
		r.revoke()
	}
	return r.body.Read(p)
}
