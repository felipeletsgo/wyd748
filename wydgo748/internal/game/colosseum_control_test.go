package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/control"
)

func runColosseumCommand(w *World, cmd control.ColosseumCommand, allowed bool) control.ColosseumResult {
	req := &colosseumRequest{ctx: context.Background(), actor: "operator", command: &cmd, authorize: func() bool { return allowed }, reply: make(chan control.ColosseumResult, 1)}
	w.handleColosseum(req)
	return <-req.reply
}

func colosseumStartCommand(w *World, tier string) control.ColosseumCommand {
	s := w.colosseumSnapshot()
	return control.ColosseumCommand{Version: 1, OperationID: "operation-0000001", Epoch: s.Epoch, ExpectedRevision: s.Revision, Tier: tier, Reason: "Start today's event early"}
}

func TestColosseumManualStartTimelineAndScheduledSuppression(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 17, 48, 30, 0, time.UTC))
	cmd := colosseumStartCommand(w, "N")
	if r := runColosseumCommand(w, cmd, false); r.Code != "capability_required" || r.Status.Active {
		t.Fatalf("unauthorized: %+v", r)
	}
	r := runColosseumCommand(w, cmd, true)
	if r.Code != "ok" || !r.Status.Active || r.Status.Tier != "N" || !r.Status.Schedule[0].Claimed || r.Status.EndsAt.Sub(clock.Now()) != 12*time.Minute {
		t.Fatalf("start: %+v", r)
	}
	colosseumFixturesState(t, w, steelWindowItem, gateClosed)
	clock.Advance(time.Minute)
	w.tickColosseum(clock.Now())
	if w.colosseum.stage != 2 || colosseumMobs(w) == 0 {
		t.Fatal("first manual wave missing outside the scheduled hour")
	}
	clock.Advance(time.Minute)
	w.tickColosseum(clock.Now())
	colosseumFixturesState(t, w, arenaGateItem, gateOpen)
	clock.Advance(10 * time.Minute)
	w.tickColosseum(clock.Now())
	if w.colosseum.stage != 8 || colosseumMobs(w) != 0 {
		t.Fatal("manual event did not finish after 12 minutes")
	}
	colosseumFixturesState(t, w, steelWindowItem, gateOpen)
	if replay := runColosseumCommand(w, cmd, true); !replay.Replayed || replay.Code != "ok" || replay.Status.Active {
		t.Fatalf("replay restarted the event: %+v", replay)
	}
	// The ordinary idle reset must not erase today's claim.
	clock.Advance(time.Second)
	w.tickColosseum(clock.Now())
	clock.Advance(4*time.Minute + 29*time.Second)
	w.tickColosseum(clock.Now())
	if w.colosseum.stage != 0 {
		t.Fatal("scheduled tier repeated after a manual start")
	}
}

func TestColosseumManualGuardsAndScheduleTimezone(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 15, 0, 0, 0, time.UTC))
	w.warConfig.Timezone = "America/Sao_Paulo"
	s := w.colosseumSnapshot()
	for tier, event := range s.Schedule {
		if event.StartsAt.UTC().Hour() != 21+tier || event.StartsAt.Minute() != 5 {
			t.Fatalf("schedule timezone: %+v", s)
		}
	}
	cmd := colosseumStartCommand(w, "N")
	old := cmd
	old.Epoch = "00000000000000000000000000000000"
	if r := runColosseumCommand(w, old, true); r.Code != "stale_event" {
		t.Fatalf("old process: %+v", r)
	}
	cmd.ExpectedRevision++
	if r := runColosseumCommand(w, cmd, true); r.Code != "stale_event" {
		t.Fatalf("stale revision: %+v", r)
	}
	cmd = colosseumStartCommand(w, "N")
	cmd.OperationID = "operation-0000002"
	if r := runColosseumCommand(w, cmd, true); r.Code != "ok" {
		t.Fatal(r.Code)
	}
	other := colosseumStartCommand(w, "M")
	other.OperationID = "operation-0000003"
	if r := runColosseumCommand(w, other, true); r.Code != "event_active" {
		t.Fatalf("overlap: %+v", r)
	}
	cmd.Tier = "A"
	if r := runColosseumCommand(w, cmd, true); r.Code != "operation_conflict" {
		t.Fatalf("operation changed: %+v", r)
	}
	w.finishColosseum()
	w.colosseum.stage = 8
	cmd = colosseumStartCommand(w, "M")
	cmd.OperationID = "operation-0000004"
	p := addZonePlayer(w, 1, 2630, 1726, 500)
	w.colosseum.exits[p] = true
	if r := runColosseumCommand(w, cmd, true); r.Code != "event_busy" {
		t.Fatalf("exit fence: %+v", r)
	}
	delete(w.colosseum.exits, p)
	clock.Advance(10 * time.Hour)
	cmd = colosseumStartCommand(w, "M")
	cmd.OperationID = "operation-0000005"
	if r := runColosseumCommand(w, cmd, true); r.Code != "schedule_passed" {
		t.Fatalf("late start: %+v", r)
	}
}

func TestColosseumCommandCancellationAfterAuthorization(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	ctx, cancel := context.WithCancel(context.Background())
	cmd := colosseumStartCommand(w, "N")
	req := &colosseumRequest{ctx: ctx, actor: "operator", command: &cmd, authorize: func() bool { cancel(); return true }, reply: make(chan control.ColosseumResult, 1)}
	w.handleColosseum(req)
	if w.colosseum.stage != 0 || len(req.reply) != 0 {
		t.Fatal("canceled command mutated the event")
	}
}

func TestColosseumCanceledQueueDoesNotMutateWorld(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w.commands = make(chan command, 1)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	cmd := colosseumStartCommand(w, "N")
	done := make(chan error, 1)
	go func() {
		_, err := w.ColosseumCommand(ctx, "operator", cmd, func() bool { return true })
		done <- err
	}()
	var queued command
	select {
	case queued = <-w.commands:
	case <-time.After(time.Second):
		t.Fatal("command did not enter the World queue")
	}
	if _, err := w.ColosseumStatus(context.Background()); !errors.Is(err, control.ErrBusy) {
		t.Fatalf("second queued request was not rejected: %v", err)
	}
	cancel()
	select {
	case err := <-done:
		if !errors.Is(err, context.Canceled) {
			t.Fatal(err)
		}
	case <-time.After(time.Second):
		t.Fatal("canceled command did not return")
	}
	w.handleColosseum(queued.colosseum)
	if w.colosseum.stage != 0 || w.colosseumPending.Load() {
		t.Fatal("canceled queued command mutated the event or kept its queue fence")
	}
}

func TestColosseumManualClaimExpiresOnTheNextLocalDay(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 17, 0, 0, 0, time.UTC))
	if r := runColosseumCommand(w, colosseumStartCommand(w, "N"), true); r.Code != "ok" {
		t.Fatal(r.Code)
	}
	clock.Advance(12 * time.Minute)
	w.tickColosseum(clock.Now())
	clock.Advance(23*time.Hour - 12*time.Minute)
	w.tickColosseum(clock.Now()) // 16:00 next day resets the completed stage.
	clock.Advance(2*time.Hour + 5*time.Minute)
	w.tickColosseum(clock.Now())
	if w.colosseum.stage != 1 || w.colosseum.claimed[0] != "2026-10-22" {
		t.Fatal("yesterday's manual claim blocked today's scheduled event")
	}
}
