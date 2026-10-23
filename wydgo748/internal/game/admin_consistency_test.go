package game

import (
	"context"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

func TestLoginResultRechecksMaintenance(t *testing.T) {
	now := time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC)
	for _, tc := range []struct {
		name     string
		plan     *shutdownPlan
		stopping bool
		blocked  bool
	}{
		{name: "cutoff reached during authentication", plan: &shutdownPlan{at: now.Add(control.ShutdownLoginCutoff)}, blocked: true},
		{name: "shutdown already requested", plan: &shutdownPlan{at: now.Add(time.Hour), requested: true}, blocked: true},
		{name: "final snapshot completed", stopping: true, blocked: true},
		{name: "before cutoff", plan: &shutdownPlan{at: now.Add(control.ShutdownLoginCutoff + time.Nanosecond)}},
		{name: "maintenance cancelled"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			s := newLoginTestSession(1, 64)
			defer s.Close()
			st := &loginReloadStore{account: loginTestAccount()}
			w := loginResultWorld(st, s)
			w.clock = newFakeClock(now)
			w.staffOps.shutdown, w.shuttingDown = tc.plan, tc.stopping
			w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
			if w.authPending[s] {
				t.Fatal("completed authentication remained pending")
			}
			if !tc.blocked {
				if w.players[s] == nil || st.loads != 1 {
					t.Fatal("open admission rejected a valid login")
				}
				return
			}
			if w.players[s] != nil || len(w.accountSessions) != 0 || st.loads != 0 {
				t.Fatal("pending authentication crossed the maintenance cutoff")
			}
			pkt, ok := s.DequeuePacketForTest()
			if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel || !strings.Contains(string(pkt), "maintenance") {
				t.Fatal("maintenance rejection notice missing")
			}
		})
	}
}

type orderedSanctionStore struct {
	*sanctionTestStore
	read func(context.Context, string) (store.SanctionState, error)
}

func (s *orderedSanctionStore) ActiveSanctionState(ctx context.Context, account string) (store.SanctionState, error) {
	return s.read(ctx, account)
}

func TestSanctionRefreshDiscardsOutOfOrderReads(t *testing.T) {
	active := store.PermanentSanction
	for _, tc := range []struct {
		name   string
		old    store.SanctionState
		latest store.SanctionState
	}{
		{name: "revoked ban", old: store.SanctionState{BanUntil: active}},
		{name: "new ban", latest: store.SanctionState{BanUntil: active}},
		{name: "revoked mute", old: store.SanctionState{MuteUntil: active}},
		{name: "new mute", latest: store.SanctionState{MuteUntil: active}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			started, release := make(chan struct{}), make(chan struct{})
			var once sync.Once
			unblock := func() { once.Do(func() { close(release) }) }
			defer unblock()
			var reads atomic.Int32
			backend := &orderedSanctionStore{read: func(ctx context.Context, _ string) (store.SanctionState, error) {
				if reads.Add(1) == 1 {
					close(started)
					select {
					case <-release:
						return tc.old, nil
					case <-ctx.Done():
						return store.SanctionState{}, ctx.Err()
					}
				}
				return tc.latest, nil
			}}
			w := &World{commands: make(chan command, 4)}
			stop, stopped := make(chan struct{}), make(chan struct{})
			go func() { defer close(stopped); runStaffQueue(w, stop) }()
			defer func() { close(stop); <-stopped }()
			oldResult := make(chan string, 1)
			go func() { oldResult <- w.pushSanctionState(ctx, backend, "Player", "") }()
			select {
			case <-started:
			case <-ctx.Done():
				t.Fatal("first refresh did not start")
			}
			if got := w.pushSanctionState(ctx, backend, "player", "revoke"); got != "offline" {
				t.Fatalf("latest refresh not confirmed: %s", got)
			}
			unblock()
			select {
			case got := <-oldResult:
				if got != "pending" {
					t.Fatalf("superseded refresh reported success: %s", got)
				}
			case <-ctx.Done():
				t.Fatal("first refresh did not finish")
			}
			if err := w.submitStaff(ctx, "test.sanctions", func() {
				if w.staffOps.bans["player"] != tc.latest.BanUntil || w.staffOps.mutes["player"] != tc.latest.MuteUntil {
					t.Error("stale database read replaced the latest live sanctions")
				}
			}); err != nil {
				t.Fatal(err)
			}
		})
	}
}

func TestSanctionRefreshVersionsAreAccountScoped(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	started, release := make(chan struct{}), make(chan struct{})
	var once sync.Once
	unblock := func() { once.Do(func() { close(release) }) }
	defer unblock()
	backend := &orderedSanctionStore{read: func(ctx context.Context, account string) (store.SanctionState, error) {
		if account == "player" {
			close(started)
			select {
			case <-release:
				return store.SanctionState{BanUntil: store.PermanentSanction}, nil
			case <-ctx.Done():
				return store.SanctionState{}, ctx.Err()
			}
		}
		return store.SanctionState{}, nil
	}}
	w := &World{commands: make(chan command, 4)}
	stop, stopped := make(chan struct{}), make(chan struct{})
	go func() { defer close(stopped); runStaffQueue(w, stop) }()
	defer func() { close(stop); <-stopped }()
	result := make(chan string, 1)
	go func() { result <- w.pushSanctionState(ctx, backend, "player", "") }()
	select {
	case <-started:
	case <-ctx.Done():
		t.Fatal("first account refresh did not start")
	}
	if got := w.pushSanctionState(ctx, backend, "other", ""); got != "offline" {
		t.Fatal("second account refresh failed", got)
	}
	unblock()
	select {
	case got := <-result:
		if got != "offline" {
			t.Fatal("another account incorrectly invalidated the refresh", got)
		}
	case <-ctx.Done():
		t.Fatal("first account refresh did not finish")
	}
}

func TestLoginResultDiscardsSanctionsChangedDuringAuthentication(t *testing.T) {
	for _, kind := range []string{control.SanctionBan, control.SanctionMute} {
		t.Run(kind, func(t *testing.T) {
			s := newLoginTestSession(1, 64)
			defer s.Close()
			st := &loginReloadStore{account: loginTestAccount()}
			w := loginResultWorld(st, s)
			w.commands = make(chan command, 4)
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			stop, stopped := make(chan struct{}), make(chan struct{})
			go func() { defer close(stopped); runStaffQueue(w, stop) }()
			defer func() { close(stop); <-stopped }()
			// Authentication already holds an active sanction snapshot when a
			// revocation is refreshed through the real asynchronous World queue.
			old := &loginResult{accountName: "Felipe", account: loginTestAccount()}
			if kind == control.SanctionBan {
				old.sanctions.BanUntil = store.PermanentSanction
			} else {
				old.sanctions.MuteUntil = store.PermanentSanction
			}
			backend := &sanctionTestStore{}
			if got := w.pushSanctionState(ctx, backend, "felipe", "revoke"); got != "offline" {
				t.Fatal("revocation refresh failed", got)
			}
			if err := w.submitStaff(ctx, "test.login", func() {
				w.onLoginResult(s, old)
				if w.players[s] != nil || st.loads != 0 || w.authPending[s] || len(w.accountSessions) != 0 {
					t.Error("stale authentication was admitted")
				}
				if len(w.staffOps.bans) != 0 || len(w.staffOps.mutes) != 0 {
					t.Error("old authentication resurrected a revoked sanction")
				}
				pkt, ok := s.DequeuePacketForTest()
				if !ok || !wire.Decrypt(pkt) || !strings.Contains(string(pkt), "Login temporarily unavailable") {
					t.Error("stale authentication did not ask for a fresh attempt")
				}
				// A fresh authentication is not permanently locked out by the
				// revocation tombstone, and does not inherit the old mute.
				fresh := newLoginTestSession(2, 64)
				defer fresh.Close()
				w.authPending[fresh] = true
				w.onLoginResult(fresh, &loginResult{accountName: "Felipe", account: loginTestAccount(),
					sanctionVersion: w.staffOps.sanctionVersions["felipe"]})
				if w.players[fresh] == nil || len(w.staffOps.mutes) != 0 {
					t.Error("fresh authentication after revocation was not admitted cleanly")
				}
			}); err != nil {
				t.Fatal(err)
			}
		})
	}
}
