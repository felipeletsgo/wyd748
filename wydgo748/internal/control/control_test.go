package control

import (
	"context"
	"expvar"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"time"
)

type sourceFunc func(context.Context, Query) (Overview, error)

func (f sourceFunc) Overview(c context.Context, q Query) (Overview, error) { return f(c, q) }

func TestPrivateContract(t *testing.T) {
	key := strings.Repeat("k", 32)
	setExpvarInt("postgres_async_queue_depth", 7)
	setExpvarInt("postgres_async_failures_total", 8)
	setExpvarInt("world_panics_total", 9)
	calls := 0
	h, _ := NewHandler(sourceFunc(func(_ context.Context, q Query) (Overview, error) {
		calls++
		return Overview{Version: 1, AsOf: time.Now().UTC(), Players: []Player{{
			UID: "uid", Name: "Knight", Account: "account", Class: 2, Level: 320,
			HP: 1250, MaxHP: 1500, MP: 650, MaxMP: 800, GuildID: 19, GuildRank: 3,
			SessionID: 77, PartySize: 2, X: 2107, Y: 2089,
		}}, Offset: q.Offset}, nil
	}), key)
	s := httptest.NewServer(h)
	defer s.Close()
	r, _ := http.Get(s.URL + "/v1/overview")
	if r.StatusCode != 401 {
		t.Fatal(r.Status)
	}
	r.Body.Close()
	if calls != 0 {
		t.Fatal("unauthenticated query reached source")
	}
	c, err := NewClient(s.URL, key)
	if err != nil {
		t.Fatal(err)
	}
	got, err := c.Overview(context.Background(), "staff", Query{Offset: 50})
	if err != nil || got.Offset != 50 || len(got.Players) != 1 {
		t.Fatalf("%+v %v", got, err)
	}
	if got.Diagnostics.Version != 1 || got.Diagnostics.AsOf.IsZero() || got.Diagnostics.PostgresQueueDepth != 7 || got.Diagnostics.PersistenceFailures != 8 || got.Diagnostics.Panics != 9 {
		t.Fatalf("diagnostics contract: %+v", got.Diagnostics)
	}
	p := got.Players[0]
	if p.HP != 1250 || p.MaxHP != 1500 || p.MP != 650 || p.MaxMP != 800 || p.GuildID != 19 || p.GuildRank != 3 || p.SessionID != 77 || p.PartySize != 2 || p.X != 2107 || p.Y != 2089 {
		t.Fatalf("live player contract: %+v", p)
	}
	if _, err = c.Overview(context.Background(), "staff", Query{}); err != ErrBusy {
		t.Fatalf("rate limit: %v", err)
	}
}

func setExpvarInt(name string, value int64) {
	v := expvar.Get(name)
	metric, ok := v.(*expvar.Int)
	if !ok {
		metric = expvar.NewInt(name)
	}
	metric.Set(value)
}

func TestAddressAndQueryRejections(t *testing.T) {
	for _, addr := range []string{"0.0.0.0:8081", "localhost:8081", "192.168.1.2:8081", "127.0.0.1:0"} {
		if LoopbackAddress(addr) == nil {
			t.Fatal(addr)
		}
	}
	for _, base := range []string{"http://localhost:8081", "http://127.0.0.1:8081/path", "http://user@127.0.0.1:8081", "http://127.0.0.1:8081?foo"} {
		if _, err := NewClient(base, strings.Repeat("k", 32)); err == nil {
			t.Fatal(base)
		}
	}
	if _, err := ParseQuery(map[string][]string{"search": {strings.Repeat("á", 48)}}); err != nil {
		t.Fatalf("48 Unicode characters rejected: %v", err)
	}
	if _, err := ParseQuery(map[string][]string{"search": {strings.Repeat("á", 49)}}); err == nil {
		t.Fatal("49 Unicode characters accepted")
	}
}

func TestClientDoesNotFollowRedirect(t *testing.T) {
	target := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { t.Error("followed redirect with secret") }))
	defer target.Close()
	s := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { http.Redirect(w, r, target.URL, 302) }))
	defer s.Close()
	c, _ := NewClient(s.URL, strings.Repeat("k", 32))
	if _, err := c.Overview(context.Background(), "staff", Query{}); err == nil {
		t.Fatal("redirect accepted")
	}
}
