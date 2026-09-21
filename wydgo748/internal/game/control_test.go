package game

import (
	"context"
	"fmt"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	wydnet "wydgo/internal/net"
)

func TestControlReadSnapshotAndPagination(t *testing.T) {
	w := &World{commands: make(chan command, 2), players: make(map[*wydnet.Session]*Player)}
	for i := 0; i < 55; i++ {
		s := &wydnet.Session{ID: int64(i + 1)}
		w.players[s] = &Player{InWorld: true, Account: &model.Account{Name: "account"}, Char: &model.Char{UID: fmt.Sprintf("uid-%03d", i), Name: "Knight", Score: &model.Score{Level: 321}}}
	}
	done := make(chan struct{})
	go func() { defer close(done); w.safeHandle(<-w.commands) }()
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	got, err := w.Overview(ctx, control.Query{Offset: 50, Search: "knight"})
	<-done
	if err != nil || got.Online != 55 || got.Matched != 55 || len(got.Players) != 5 || got.HasMore {
		t.Fatalf("%+v %v", got, err)
	}
	for _, p := range w.players {
		p.Char.Name = "changed"
		p.Char.Score.Level = 1
	}
	if got.Players[0].Name != "Knight" || got.Players[0].Level != 321 {
		t.Fatal("snapshot shares mutable state")
	}
}

func TestControlTimeoutKeepsSingleOutstandingRead(t *testing.T) {
	w := &World{commands: make(chan command, 2)}
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Millisecond)
	defer cancel()
	if _, err := w.Overview(ctx, control.Query{}); err == nil {
		t.Fatal("missing timeout")
	}
	if _, err := w.Overview(context.Background(), control.Query{}); err != control.ErrBusy {
		t.Fatal(err)
	}
	w.safeHandle(<-w.commands)
	if w.controlPending.Load() {
		t.Fatal("cancelled query not released")
	}
	w.commands <- command{}
	w.commands <- command{}
	if _, err := w.Overview(context.Background(), control.Query{}); err != control.ErrBusy || w.controlPending.Load() {
		t.Fatal("queue saturation")
	}
}

func TestControlMaintenanceIsReadable(t *testing.T) {
	w := &World{shuttingDown: true}
	req := &controlRequest{ctx: context.Background(), reply: make(chan control.Overview, 1)}
	w.handle(command{control: req})
	if got := (<-req.reply).State; got != "maintenance" {
		t.Fatal(got)
	}
}

func TestControlPlayerLiveStateUsesRuntimeValues(t *testing.T) {
	s := &wydnet.Session{ID: 77}
	base := &model.Score{Level: 320, CurHP: 900, MaxHP: 1000, CurMP: 400, MaxMP: 500}
	runtime := &model.Score{Level: 320, CurHP: 1250, MaxHP: 1500, CurMP: 650, MaxMP: 800}
	p := &Player{
		Session: s, InWorld: true, X: 2107, Y: 2089,
		Account: &model.Account{Name: "account"},
		Char: &model.Char{UID: "uid-live", Name: "Knight", Class: 2, X: 1, Y: 2,
			Score: base, RuntimeScore: runtime, GuildID: 19, GuildRank: 3},
	}
	p.Party = &Party{Members: []*Player{p, {InWorld: true}}}
	w := &World{players: map[*wydnet.Session]*Player{s: p}}
	req := &controlRequest{ctx: context.Background(), reply: make(chan control.Overview, 1)}
	w.handleControl(req)
	got := <-req.reply
	if len(got.Players) != 1 {
		t.Fatalf("players=%d", len(got.Players))
	}
	row := got.Players[0]
	if row.SessionID != 77 || row.X != 2107 || row.Y != 2089 || row.GuildID != 19 || row.GuildRank != 3 || row.PartySize != 2 {
		t.Fatalf("runtime metadata: %+v", row)
	}
	if row.Level != 320 || row.HP != 1250 || row.MaxHP != 1500 || row.MP != 650 || row.MaxMP != 800 {
		t.Fatalf("runtime score: %+v", row)
	}
	runtime.CurHP = 1
	p.X = 999
	if row.HP != 1250 || row.X != 2107 {
		t.Fatalf("snapshot shares mutable live state: %+v", row)
	}
}

func TestControlPlayerLiveStateFallsBackToBaseScore(t *testing.T) {
	s := &wydnet.Session{ID: 88}
	p := &Player{Session: s, InWorld: true, Account: &model.Account{Name: "account"},
		Char: &model.Char{UID: "uid-base", Name: "Foema", Score: &model.Score{Level: 12, CurHP: 90, MaxHP: 100, CurMP: 45, MaxMP: 60}}}
	w := &World{players: map[*wydnet.Session]*Player{s: p}}
	req := &controlRequest{ctx: context.Background(), reply: make(chan control.Overview, 1)}
	w.handleControl(req)
	row := (<-req.reply).Players[0]
	if row.Level != 12 || row.HP != 90 || row.MaxHP != 100 || row.MP != 45 || row.MaxMP != 60 {
		t.Fatalf("base fallback: %+v", row)
	}
}

func TestControlSearchLengthUsesUnicodeCharacters(t *testing.T) {
	w := &World{commands: make(chan command, 1), players: make(map[*wydnet.Session]*Player)}
	done := make(chan struct{})
	go func() { defer close(done); w.safeHandle(<-w.commands) }()
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	if _, err := w.Overview(ctx, control.Query{Search: strings.Repeat("á", 48)}); err != nil {
		t.Fatalf("48 Unicode characters rejected: %v", err)
	}
	<-done
	if _, err := w.Overview(context.Background(), control.Query{Search: strings.Repeat("á", 49)}); err != control.ErrBusy {
		t.Fatalf("49 Unicode characters: %v", err)
	}
}

func TestAccountPresenceUsesOneAuthoritativeWorldRead(t *testing.T) {
	onlineSession := &wydnet.Session{ID: 1}
	selectingSession := &wydnet.Session{ID: 2}
	w := &World{
		commands: make(chan command, 1),
		players: map[*wydnet.Session]*Player{
			onlineSession:    {InWorld: true, Account: &model.Account{Name: "Fixture"}},
			selectingSession: {InWorld: false, Account: &model.Account{Name: "Selecting"}},
		},
	}
	done := make(chan struct{})
	go func() { defer close(done); w.safeHandle(<-w.commands) }()
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	got, err := w.AccountPresence(ctx, []string{"fixture", "offline", "FIXTURE"})
	<-done
	if err != nil || len(got) != 2 || !got["fixture"] || got["offline"] {
		t.Fatalf("presence=%v err=%v", got, err)
	}
	if _, err := w.AccountPresence(context.Background(), []string{"invalid account"}); err != control.ErrBusy {
		t.Fatalf("invalid presence query: %v", err)
	}
	w.controlPending.Store(true)
	if _, err := w.AccountPresence(context.Background(), []string{"fixture"}); err != control.ErrBusy {
		t.Fatalf("concurrent presence query: %v", err)
	}
	w.controlPending.Store(false)
}
