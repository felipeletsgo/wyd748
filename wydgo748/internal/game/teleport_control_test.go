package game

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"testing"
	"time"
	"wydgo/internal/control"
	"wydgo/internal/model"
)

func teleportFixture(t *testing.T) (*World, *Player, *craftStore, control.TeleportCommand) {
	w, p, st, k := kickFixture(t)
	c := control.TeleportCommand{Version: k.Version, OperationID: k.OperationID, Epoch: k.Epoch, UID: k.UID, Account: k.Account, ExpectedSessionID: k.ExpectedSessionID, Reason: k.Reason, Destination: "armia"}
	return w, p, st, c
}
func executeTeleport(w *World, c control.TeleportCommand, allowed bool) control.TeleportResult {
	req := &teleportRequest{ctx: context.Background(), actor: "operator", command: c, authorize: func() bool { return allowed }, reply: make(chan control.TeleportResult, 1)}
	w.handle(command{teleport: req})
	return <-req.reply
}
func TestTeleportControlCitiesAndReplay(t *testing.T) {
	for i, destination := range []string{"armia", "azran", "erion", "nippleheim"} {
		t.Run(destination, func(t *testing.T) {
			w, p, st, c := teleportFixture(t)
			c.Destination = destination
			p.MoveAuthorityRoute = []byte{'1'}
			p.ShopNPC, p.CargoNPC = 77, 88
			r := executeTeleport(w, c, true)
			zone := cityWarZones[i]
			if r.Code != "ok" || r.X != zone.exitX || r.Y != zone.exitY || p.X != r.X || p.Char.X != r.X || p.Char.Y != r.Y || st.saves != 1 || p.Session.IsClosed() {
				t.Fatal(r, st.saves)
			}
			if len(p.MoveAuthorityRoute) != 0 {
				t.Fatal("old movement survived teleport")
			}
			if p.ShopNPC != 0 || p.CargoNPC != 0 {
				t.Fatal("old NPC interaction survived teleport")
			}
			// Retrying cannot move even the same session back after it has walked away.
			p.X, p.Y = 2101, 2102
			replay := executeTeleport(w, c, true)
			if !replay.Replayed || replay.X != r.X || replay.Y != r.Y || p.X != 2101 || st.saves != 1 {
				t.Fatal(replay, st.saves)
			}
			c.Reason = "different request"
			if r := executeTeleport(w, c, true); r.Code != "operation_conflict" {
				t.Fatal(r)
			}
		})
	}
}
func TestTeleportControlRejections(t *testing.T) {
	for _, which := range []string{"permission", "maintenance", "epoch", "session", "uid", "account", "offline", "closed", "capacity", "invalid", "trade", "shop", "poison", "dead", "instance", "grace", "indexed", "city", "tower", "fighter"} {
		t.Run(which, func(t *testing.T) {
			w, p, st, c := teleportFixture(t)
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
				c.UID = "other"
			case "account":
				c.Account = "other"
			case "offline":
				p.InWorld = false
			case "closed":
				p.Session.Close()
			case "invalid":
				c.Destination = "arbitrary"
				want = "invalid_command"
			case "capacity":
				w.teleportReceipts = make(map[string]teleportReceipt)
				for i := 0; i < 1024; i++ {
					w.teleportReceipts[fmt.Sprint(i)] = teleportReceipt{}
				}
				want = "operation_capacity"
			case "trade":
				p.Trade = &TradeState{}
				want = "player_busy"
			case "shop":
				p.GhostShop = &GhostShop{}
				want = "player_busy"
			case "poison":
				p.PersistencePoisoned = true
				want = "player_busy"
			case "dead":
				setPlayerCurHP(p.Char, 0)
				want = "player_busy"
			case "instance", "grace":
				inst := &ItemInstance{RuntimeID: "test", MemberIDs: []uint16{p.ID}}
				if which == "grace" {
					inst.ExitAt = w.now().Add(time.Minute)
				}
				w.itemInstances = map[string]*ItemInstance{"test": inst}
				want = "restricted_area"
			case "indexed":
				w.itemInstances = map[string]*ItemInstance{"test": {RuntimeID: "test", MemberIDs: []uint16{p.ID}}}
				w.setPlayerInstanceIndex(p.ID, "test")
				want = "restricted_area"
			case "city":
				p.X, p.Y = 205, 220
				want = "restricted_area"
			case "tower":
				p.X, p.Y = 2450, 1855
				want = "restricted_area"
			case "fighter":
				w.cityFighters = map[*Player]cityFighter{p: {}}
				want = "restricted_area"
			}
			x, y := p.X, p.Y
			if r := executeTeleport(w, c, allowed); r.Code != want || st.saves != 0 || p.X != x || p.Y != y {
				t.Fatal(r, st.saves)
			}
		})
	}
}
func TestTeleportControlRollbackAndFullDestination(t *testing.T) {
	w, p, st, c := teleportFixture(t)
	p.MoveAuthorityRoute = []byte{'1'}
	p.ShopNPC, p.CargoNPC = 77, 88
	st.err = errors.New("database unavailable")
	if r := executeTeleport(w, c, true); r.Code != "persistence_failed" || p.X != 2100 || p.Y != 2100 || p.Char.X != 2100 || p.Char.Y != 2100 {
		t.Fatal(r)
	}
	if len(p.MoveAuthorityRoute) != 1 || p.ShopNPC != 77 || p.CargoNPC != 88 {
		t.Fatal("failed teleport changed movement or NPC interaction")
	}
	st.err = nil
	if r := executeTeleport(w, c, true); r.Code != "persistence_failed" || !r.Replayed || st.saves != 1 {
		t.Fatal(r, st.saves)
	}
	w, p, st, c = teleportFixture(t)
	z := cityWarZones[0]
	for x := z.exitX - 3; x <= z.exitX+3; x++ {
		for y := z.exitY - 3; y <= z.exitY+3; y++ {
			m := &Mob{ID: uint16(len(w.mobsByID) + 100), X: x, Y: y, HP: 1, Def: &model.NPCDef{}}
			w.mobsByID[m.ID] = m
			w.mobs = append(w.mobs, m)
			w.registerMobSpatial(m)
		}
	}
	if r := executeTeleport(w, c, true); r.Code != "destination_unavailable" || st.saves != 0 || p.X != 2100 {
		t.Fatal(r)
	}
}
func TestTeleportQueueCancellationAndBackpressure(t *testing.T) {
	w, p, st, c := teleportFixture(t)
	w.commands = make(chan command, 1)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	done := make(chan error, 1)
	go func() { _, err := w.TeleportCommand(ctx, "operator", c, func() bool { return true }); done <- err }()
	var queued command
	select {
	case queued = <-w.commands:
	case <-time.After(3 * time.Second):
		t.Fatal("not queued")
	}
	if commandLabel(queued) != "control.teleport" {
		t.Fatal(commandLabel(queued))
	}
	if _, err := w.TeleportCommand(context.Background(), "operator", c, func() bool { return true }); err != control.ErrBusy {
		t.Fatal(err)
	}
	cancel()
	if err := <-done; !errors.Is(err, context.Canceled) {
		t.Fatal(err)
	}
	w.handle(queued)
	if w.teleportPending.Load() || p.Session.IsClosed() || st.saves != 0 {
		t.Fatal("cancel leaked/mutated")
	}
	w.commands <- command{}
	if _, err := w.TeleportCommand(context.Background(), "operator", c, func() bool { return true }); err != control.ErrBusy || w.teleportPending.Load() {
		t.Fatal(err)
	}
	<-w.commands
	go func() {
		_, err := w.TeleportCommand(context.Background(), "operator", c, func() bool { return true })
		done <- err
	}()
	select {
	case queued = <-w.commands:
	case <-time.After(3 * time.Second):
		t.Fatal("not queued again")
	}
	w.handle(queued)
	if err := <-done; err != nil || p.Session.IsClosed() || st.saves != 1 {
		t.Fatal(err, st.saves)
	}
}

func TestTeleportCancellationDuringAuthorization(t *testing.T) {
	w, p, _, c := teleportFixture(t)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	req := &teleportRequest{ctx: ctx, actor: "operator", command: c, authorize: func() bool { cancel(); return true }, reply: make(chan control.TeleportResult, 1)}
	w.handleTeleport(req)
	if p.X != 2100 || p.Session.IsClosed() || len(req.reply) != 0 || len(w.teleportReceipts) != 0 {
		t.Fatal("canceled command executed")
	}
}
