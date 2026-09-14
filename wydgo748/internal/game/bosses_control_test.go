package game

import (
	"context"
	"errors"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
)

func executeBosses(w *World, c control.BossesCommand, allowed bool) control.BossesResult {
	r := &bossesRequest{ctx: context.Background(), actor: "operator", command: &c, authorize: func() bool { return allowed }, reply: make(chan control.BossesResult, 1)}
	w.handle(command{bosses: r})
	return <-r.reply
}
func bossAdminFixture(t *testing.T) (*World, control.BossesCommand) {
	t.Helper()
	w, _ := bossSpawnTestWorld()
	if err := w.spawnConfiguredBosses(); err != nil {
		t.Fatal(err)
	}
	s := w.bossesSnapshot()
	return w, control.BossesCommand{Version: 1, Epoch: s.Epoch, OperationID: "boss-operation-0001", BossID: s.Bosses[0].ID, ExpectedRevision: s.Bosses[0].Revision, Action: "summon", Reason: "community encounter"}
}
func defeatAdminBoss(w *World) {
	mob := w.mobsByID[w.bossSpawns[0].mobID]
	w.finishBossMobKilled(mob, false)
	w.removeMobInstance(mob)
}

func TestBossesSummonLifecycleAndReplay(t *testing.T) {
	w, c := bossAdminFixture(t)
	if r := executeBosses(w, c, true); r.Code != "boss_alive" || len(w.mobs) != 1 {
		t.Fatal(r)
	}
	defeatAdminBoss(w)
	if r := executeBosses(w, c, true); r.Code != "boss_alive" || !r.Replayed || r.Status.Bosses[0].Alive {
		t.Fatal("failed receipt was re-executed", r)
	}
	c.OperationID = "boss-operation-0002"
	if r := executeBosses(w, c, true); r.Code != "stale_boss" {
		t.Fatal(r)
	}
	c.OperationID = "boss-operation-0003"
	c.ExpectedRevision = w.bossSpawns[0].revision
	if r := executeBosses(w, c, true); r.Code != "ok" || !r.Status.Bosses[0].Alive || r.Status.Bosses[0].RespawnAt != nil || len(w.mobs) != 1 {
		t.Fatal(r)
	}
	spawnedID := w.bossSpawns[0].mobID
	w.tickBossRespawns(w.now().Add(time.Hour))
	if len(w.mobs) != 1 || w.bossSpawns[0].mobID != spawnedID {
		t.Fatal("automatic respawn duplicated manual spawn")
	}
	defeatAdminBoss(w)
	if r := executeBosses(w, c, true); r.Code != "ok" || !r.Replayed || r.Status.Bosses[0].Alive || len(w.mobs) != 0 {
		t.Fatal("retry summoned after another death", r)
	}
	c.Reason = "changed reason"
	if r := executeBosses(w, c, true); r.Code != "operation_conflict" {
		t.Fatal(r)
	}
	if r := executeBosses(w, c, false); r.Code != "capability_required" {
		t.Fatal("replay bypassed authorization", r)
	}
}

func TestBossesRejectUnsafeCommands(t *testing.T) {
	for _, mode := range []string{"permission", "maintenance", "epoch", "unknown", "revision", "invalid", "capacity", "auto-respawn"} {
		t.Run(mode, func(t *testing.T) {
			w, c := bossAdminFixture(t)
			defeatAdminBoss(w)
			c.ExpectedRevision = w.bossSpawns[0].revision
			allowed, want := true, ""
			switch mode {
			case "permission":
				allowed, want = false, "capability_required"
			case "maintenance":
				w.shuttingDown, want = true, "server_maintenance"
			case "epoch":
				c.Epoch, want = strings.Repeat("x", 32), "stale_boss"
			case "unknown":
				c.BossID, want = "not-configured", "unknown_boss"
			case "revision":
				c.ExpectedRevision++
				want = "stale_boss"
			case "invalid":
				c.Action, want = "kill", "invalid_command"
			case "capacity":
				w.bossesReceipts = make(map[string]bossesReceipt)
				for i := 0; i < 1024; i++ {
					w.bossesReceipts[string(rune(i))] = bossesReceipt{}
				}
				want = "operation_capacity"
			case "auto-respawn":
				w.tickBossRespawns(w.now().Add(time.Minute))
				want = "stale_boss"
			}
			before := len(w.mobs)
			if r := executeBosses(w, c, allowed); r.Code != want || len(w.mobs) != before {
				t.Fatal(r, before, len(w.mobs))
			}
		})
	}
}

func TestBossesQueueCancellationAndBackpressure(t *testing.T) {
	w, c := bossAdminFixture(t)
	defeatAdminBoss(w)
	c.ExpectedRevision = w.bossSpawns[0].revision
	w.commands = make(chan command, 1)
	ctx, cancel := context.WithCancel(context.Background())
	done := make(chan error, 1)
	go func() { _, err := w.BossesCommand(ctx, "operator", c, func() bool { return true }); done <- err }()
	var queued command
	select {
	case queued = <-w.commands:
	case <-time.After(3 * time.Second):
		t.Fatal("not queued")
	}
	if commandLabel(queued) != "control.bosses" {
		t.Fatal(commandLabel(queued))
	}
	if _, err := w.BossesStatus(context.Background()); !errors.Is(err, control.ErrBusy) {
		t.Fatal(err)
	}
	cancel()
	if err := <-done; !errors.Is(err, context.Canceled) {
		t.Fatal(err)
	}
	w.handle(queued)
	if w.bossesPending.Load() || len(w.mobs) != 0 {
		t.Fatal("canceled request spawned or leaked admission")
	}
	// Full generic queue is a separate rejection and must release admission.
	w.commands <- command{}
	if _, err := w.BossesStatus(context.Background()); !errors.Is(err, control.ErrBusy) || w.bossesPending.Load() {
		t.Fatal(err)
	}
	<-w.commands
	go func() {
		_, err := w.BossesCommand(context.Background(), "operator", c, func() bool { return true })
		done <- err
	}()
	select {
	case queued = <-w.commands:
	case <-time.After(3 * time.Second):
		t.Fatal("not queued again")
	}
	w.handle(queued)
	if err := <-done; err != nil || len(w.mobs) != 1 {
		t.Fatal(err)
	}
}

func TestBossesSpawnFailureKeepsSchedule(t *testing.T) {
	w, c := bossAdminFixture(t)
	defeatAdminBoss(w)
	c.ExpectedRevision = w.bossSpawns[0].revision
	state := w.bossSpawns[0]
	deadline := state.respawnAt
	// Exhaust the allocator without introducing live mobs into the world list.
	for id := uint16(firstMobID); ; id++ {
		w.mobsByID[id] = &Mob{ID: id}
		if id == ^uint16(0) {
			break
		}
	}
	if r := executeBosses(w, c, true); r.Code != "spawn_failed" || state.mobID != 0 || !state.respawnAt.Equal(deadline) || state.revision != c.ExpectedRevision {
		t.Fatal(r)
	}
}

func TestBossesEmptySnapshot(t *testing.T) {
	w, _ := bossSpawnTestWorld()
	s := w.bossesSnapshot()
	if s.Version != 1 || len(s.Epoch) != 32 || s.Bosses == nil || len(s.Bosses) != 0 {
		t.Fatal(s)
	}
}
