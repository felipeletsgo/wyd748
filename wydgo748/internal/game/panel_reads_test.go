package game

import (
	"context"
	"expvar"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	wydnet "wydgo/internal/net"
)

func TestOverviewFiltersRunInTheSamePass(t *testing.T) {
	players := map[*wydnet.Session]*Player{}
	add := func(id int64, name string, class uint8, level uint32, guild uint16, x, y uint16) {
		s := &wydnet.Session{ID: id}
		players[s] = &Player{Session: s, InWorld: true, X: x, Y: y, Account: &model.Account{Name: name + "acc"},
			Char: &model.Char{UID: name, Name: name, Class: class, GuildID: guild, Score: &model.Score{Level: level}}}
	}
	add(1, "a", 0, 100, 7, 2100, 2100)
	add(2, "b", 1, 300, 7, 2110, 2095)
	add(3, "c", 0, 350, 9, 3000, 3000)
	w := &World{players: players}
	run := func(q control.Query) []string {
		req := &controlRequest{ctx: context.Background(), query: q, reply: make(chan control.Overview, 1)}
		w.handleControl(req)
		got := <-req.reply
		if got.Online != 3 {
			t.Fatal("online count must ignore filters", got.Online)
		}
		names := []string{}
		for _, p := range got.Players {
			names = append(names, p.Name)
		}
		return names
	}
	cases := map[string]struct {
		q    control.Query
		want string
	}{
		"class 0 is a real filter": {control.Query{HasClass: true, Class: 0}, "a,c"},
		"level range":              {control.Query{MinLevel: 200, MaxLevel: 320}, "b"},
		"guild":                    {control.Query{GuildID: 7}, "a,b"},
		"area":                     {control.Query{AreaX: 2105, AreaY: 2100, Radius: 10}, "a,b"},
		"combined":                 {control.Query{GuildID: 7, HasClass: true, Class: 1, Search: "b"}, "b"},
		"no filter":                {control.Query{}, "a,b,c"},
	}
	for name, tc := range cases {
		got := ""
		for i, n := range run(tc.q) {
			if i > 0 {
				got += ","
			}
			got += n
		}
		if got != tc.want {
			t.Fatalf("%s: got %s want %s", name, got, tc.want)
		}
	}
}

func TestBossStatusReportsLivePhase(t *testing.T) {
	w, _ := bossAdminFixture(t)
	mobID := w.bossSpawns[0].mobID
	if w.bosses == nil {
		w.bosses = map[uint16]*BossRuntime{}
	}
	if w.bosses[mobID] == nil {
		w.bosses[mobID] = &BossRuntime{}
	}
	w.bosses[mobID].Phase = 3
	if got := w.bossesSnapshot().Bosses[0]; !got.Alive || got.Phase != 3 {
		t.Fatal(got)
	}
	defeatAdminBoss(w)
	if got := w.bossesSnapshot().Bosses[0]; got.Alive || got.Phase != 0 {
		t.Fatal("dead boss reports a phase", got)
	}
}

func TestLoopLagIsBucketed(t *testing.T) {
	buckets := expvar.Get("world_loop_lag_buckets").(*expvar.Map)
	before := int64(0)
	if v, ok := buckets.Get("<=50ms").(*expvar.Int); ok {
		before = v.Value()
	}
	start := time.Now()
	lastTickAt.Store(start.Add(-worldTickInterval - 40*time.Millisecond).UnixNano())
	observeTick(start, time.Millisecond)
	if v, ok := buckets.Get("<=50ms").(*expvar.Int); !ok || v.Value() != before+1 {
		t.Fatal("lag not recorded in its bucket")
	}
}

func TestMapSnapshotIsBoundedAndShareTheOverviewGate(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Account.Name = "player"
	snapshot := w.mapSnapshot(p.X, p.Y, 16)
	if len(snapshot.Players) != 1 || snapshot.Players[0].Account != "player" || snapshot.Truncated {
		t.Fatal(snapshot)
	}
	if far := w.mapSnapshot(4000, 4000, 16); len(far.Players) != 0 {
		t.Fatal("player outside the square was returned")
	}
	w.commands = make(chan command, 1)
	w.controlPending.Store(true) // an overview read is in flight
	if _, err := w.MapSnapshot(context.Background(), p.X, p.Y, 16); err != control.ErrBusy {
		t.Fatal("map read did not share the overview gate", err)
	}
	w.controlPending.Store(false)
	if _, err := w.MapSnapshot(context.Background(), p.X, p.Y, control.MaxMapRadius+1); err == nil {
		t.Fatal("oversized radius accepted")
	}
	done := make(chan error, 1)
	go func() { _, err := w.MapSnapshot(context.Background(), p.X, p.Y, 16); done <- err }()
	w.handle(<-w.commands)
	if err := <-done; err != nil || w.controlPending.Load() {
		t.Fatal("gate not released", err)
	}
}

func TestInstancesViewCopiesState(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	deadline := time.Date(2026, 10, 22, 12, 5, 0, 0, time.UTC)
	w.itemInstances = map[string]*ItemInstance{"water-n:1": {RuntimeID: "water-n:1", Config: model.VolatileInstance{ID: "water-n", Name: "Water N", X: 100, Y: 200, AreaRadius: 12, MaxPlayers: 8},
		LeaderID: p.ID, MemberIDs: []uint16{p.ID, 999}, MobIDs: map[uint16]struct{}{1: {}, 2: {}}, CurrentStage: 3, State: "combat", CombatDeadline: deadline}}
	view := w.instancesView()
	if len(view.Instances) != 1 {
		t.Fatal(view)
	}
	got := view.Instances[0]
	if got.Name != "Water N" || got.Mobs != 2 || got.Stage != 3 || len(got.Members) != 2 || !got.Members[0].Leader ||
		got.Members[1].Name != "(offline)" || got.CombatEnds == nil || !got.CombatEnds.Equal(deadline) || got.ExitAt != nil {
		t.Fatal(got)
	}
}
