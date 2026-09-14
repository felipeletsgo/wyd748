package game

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"testing"
	"time"
	"wydgo/internal/control"
	wydnet "wydgo/internal/net"
)

func kickFixture(t *testing.T) (*World, *Player, *craftStore, control.KickCommand) {
	w, p, st := handlerTestWorld(t)
	p.Account.Name = "player"
	w.claimAccountSession(p.Session, p.Account.Name)
	c := control.KickCommand{Version: 1, OperationID: "operation-0000001", Epoch: w.moderationEpoch(), UID: p.Char.UID, Account: "player", ExpectedSessionID: p.Session.ID, Reason: "moderation test"}
	return w, p, st, c
}
func executeKick(w *World, c control.KickCommand, allowed bool) control.KickResult {
	req := &kickRequest{ctx: context.Background(), actor: "operator", command: c, authorize: func() bool { return allowed }, reply: make(chan control.KickResult, 1)}
	w.handle(command{kick: req})
	return <-req.reply
}

func TestKickDisconnectReplayAndRelogin(t *testing.T) {
	w, p, st, c := kickFixture(t)
	if r := executeKick(w, c, true); r.Code != "ok" || r.Replayed {
		t.Fatal(r)
	}
	if !p.Session.IsClosed() || len(w.players) != 0 || len(w.playersByID) != 0 || w.playersByCharacterUID[p.Char.UID] != nil || w.accountSessions["player"] != nil || st.saves != 1 {
		t.Fatal("incomplete disconnect", st.saves)
	}
	// The later transport callback must not save again or remove a new login.
	next := *p
	next.Session = wydnet.NewTestSession(99, 16)
	w.players[next.Session] = &next
	w.playersByID[next.ID] = &next
	w.claimAccountSession(next.Session, "player")
	w.onDisconnect(p.Session)
	if r := executeKick(w, c, true); r.Code != "ok" || !r.Replayed || next.Session.IsClosed() || st.saves != 1 {
		t.Fatal("replay affected new session", r, st.saves)
	}
	c.Reason = "different reason"
	if r := executeKick(w, c, true); r.Code != "operation_conflict" {
		t.Fatal(r)
	}
	c.OperationID = "operation-0000002"
	if r := executeKick(w, c, true); r.Code != "stale_target" || next.Session.IsClosed() {
		t.Fatal("old session targeted relogin", r)
	}
}

func TestKickRejectsUnsafeTargets(t *testing.T) {
	for _, which := range []string{"permission", "maintenance", "epoch", "session", "uid", "account", "offline", "closed", "capacity", "invalid"} {
		t.Run(which, func(t *testing.T) {
			w, p, st, c := kickFixture(t)
			allowed, want := true, "stale_target"
			switch which {
			case "permission":
				allowed = false
				want = "capability_required"
			case "maintenance":
				w.shuttingDown = true
				want = "server_maintenance"
			case "epoch":
				c.Epoch = strings.Repeat("0", 32)
			case "session":
				c.ExpectedSessionID++
			case "uid":
				c.UID = "other-uid"
			case "account":
				c.Account = "other"
			case "offline":
				p.InWorld = false
			case "closed":
				p.Session.Close()
			case "invalid":
				c.Reason = ""
				want = "invalid_command"
			case "capacity":
				w.kickReceipts = make(map[string]kickReceipt)
				for i := 0; i < 1024; i++ {
					w.kickReceipts[fmt.Sprint(i)] = kickReceipt{}
				}
				want = "operation_capacity"
			}
			if r := executeKick(w, c, allowed); r.Code != want || st.saves != 0 || w.players[p.Session] != p {
				t.Fatal(r, st.saves)
			}
		})
	}
}

func TestKickPersistenceFailureReceipt(t *testing.T) {
	w, p, st, c := kickFixture(t)
	st.err = errors.New("database unavailable")
	if r := executeKick(w, c, true); r.Code != "disconnected_persistence_failed" || !p.PersistencePoisoned || !p.Session.IsClosed() || len(w.players) != 0 {
		t.Fatal(r)
	}
	if r := executeKick(w, c, true); r.Code != "disconnected_persistence_failed" || !r.Replayed || st.saves != 1 {
		t.Fatal(r, st.saves)
	}
}

func TestKickQueueCancellationAndBackpressure(t *testing.T) {
	w, p, st, c := kickFixture(t)
	w.commands = make(chan command, 1)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	done := make(chan error, 1)
	go func() { _, err := w.KickCommand(ctx, "operator", c, func() bool { return true }); done <- err }()
	var queued command
	select {
	case queued = <-w.commands:
	case <-time.After(3 * time.Second):
		t.Fatal("not queued")
	}
	if commandLabel(queued) != "control.kick" {
		t.Fatal(commandLabel(queued))
	}
	if _, err := w.KickCommand(context.Background(), "operator", c, func() bool { return true }); err != control.ErrBusy {
		t.Fatal(err)
	}
	cancel()
	if err := <-done; !errors.Is(err, context.Canceled) {
		t.Fatal(err)
	}
	w.handle(queued)
	if w.kickPending.Load() || p.Session.IsClosed() || st.saves != 0 {
		t.Fatal("cancel leaked/mutated")
	}
	w.commands <- command{}
	if _, err := w.KickCommand(context.Background(), "operator", c, func() bool { return true }); err != control.ErrBusy || w.kickPending.Load() {
		t.Fatal(err)
	}
	<-w.commands
	go func() {
		_, err := w.KickCommand(context.Background(), "operator", c, func() bool { return true })
		done <- err
	}()
	select {
	case queued = <-w.commands:
	case <-time.After(3 * time.Second):
		t.Fatal("not queued again")
	}
	w.handle(queued)
	if err := <-done; err != nil || !p.Session.IsClosed() || st.saves != 1 {
		t.Fatal(err, st.saves)
	}
}

func TestKickCancellationDuringAuthorization(t *testing.T) {
	w, p, _, c := kickFixture(t)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	req := &kickRequest{ctx: ctx, actor: "operator", command: c, authorize: func() bool { cancel(); return true }, reply: make(chan control.KickResult, 1)}
	w.handleKick(req)
	if p.Session.IsClosed() || len(req.reply) != 0 || len(w.kickReceipts) != 0 {
		t.Fatal("canceled command executed")
	}
}

func TestKickEpochFromOverview(t *testing.T) {
	w, _, _, _ := kickFixture(t)
	req := &controlRequest{ctx: context.Background(), reply: make(chan control.Overview, 1)}
	w.handleControl(req)
	if snapshot := <-req.reply; snapshot.ModerationEpoch != w.kickEpoch || len(snapshot.ModerationEpoch) != 32 {
		t.Fatal(snapshot)
	}
	other := &World{}
	if other.moderationEpoch() == w.kickEpoch {
		t.Fatal("epochs reused")
	}
}
