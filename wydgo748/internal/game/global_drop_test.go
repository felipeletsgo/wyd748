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

func eventCommand(w *World, id string) control.GlobalDropCommand {
	s := w.globalDropSnapshot()
	return control.GlobalDropCommand{Version: 1, OperationID: id, Epoch: s.Epoch, ExpectedEventID: s.EventID,
		Action: "start", Reason: "fixture event", ItemID: 500, RatePercent: 100, MaxDrops: 2}
}
func executeEvent(w *World, c control.GlobalDropCommand, allowed bool) control.GlobalDropResult {
	r := &globalDropRequest{ctx: context.Background(), actor: "operator", command: &c,
		authorize: func() bool { return allowed }, reply: make(chan control.GlobalDropResult, 1)}
	w.handleGlobalDrop(r)
	return <-r.reply
}

func TestGlobalDropLifecycleAndReplay(t *testing.T) {
	w, p, m, _ := killRewardTestWorld(t, nil)
	m.Def.Carry = nil
	c := eventCommand(w, "start-operation-0001")
	c.MaxDrops = 1
	if r := executeEvent(w, c, true); r.Code != "ok" || !r.Status.Active {
		t.Fatalf("start: %+v", r)
	}
	w.killMobState(p, m, 500, 100)
	s := w.globalDropSnapshot()
	if s.Active || s.Dropped != 1 || s.EndedBy != "quantity" || p.Char.Inv[0].Index != 500 || p.Char.Inv[0].UID == "" {
		t.Fatalf("delivery: %+v inv=%+v", s, p.Char.Inv[0])
	}
	r := executeEvent(w, c, true)
	if r.Code != "ok" || !r.Replayed || r.Status.Active || r.Status.Dropped != 1 {
		t.Fatalf("retry restarted event: %+v", r)
	}
	c.RatePercent = 25
	if r := executeEvent(w, c, true); r.Code != "operation_conflict" {
		t.Fatalf("payload conflict: %+v", r)
	}
	w.killMobState(p, m, 500, 100)
	if w.globalDrop.status.Dropped != 1 {
		t.Fatal("same death awarded twice")
	}
}

func TestGlobalDropBatchReservesCapAndRollback(t *testing.T) {
	for _, fail := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rollback"}[fail], func(t *testing.T) {
			var err error
			if fail {
				err = errors.New("store offline")
			}
			w, p, m, st := killRewardTestWorld(t, err)
			m.Def.Carry = nil
			c := eventCommand(w, "batch-operation-0001")
			c.MaxDrops = 1
			executeEvent(w, c, true)
			other := *m
			other.ID++
			plans := []*killRewardPlan{w.planMobKill(p, m, 500, 100), w.planMobKill(p, &other, 500, 100)}
			if w.globalDrop.reserved != 1 || len(plans[0].drops) != 1 || len(plans[1].drops) != 0 {
				t.Fatal("batch exceeded cap")
			}
			w.commitKillRewardBatch(p, plans, nil, "global drop test")
			if w.globalDrop.reserved != 0 || st.saves != 1 {
				t.Fatal("reservation leaked or extra commit")
			}
			if fail {
				if w.globalDrop.status.Dropped != 0 || !w.globalDrop.status.Active || p.Char.Inv[0].Index != 0 || m.Dead || other.Dead {
					t.Fatal("reward/counter not rolled back")
				}
			} else if w.globalDrop.status.Dropped != 1 || w.globalDrop.status.Active {
				t.Fatal("cap not finalized")
			}
		})
	}
}

func TestGlobalDropChanceBoundariesAndEligibility(t *testing.T) {
	for _, tc := range []struct {
		rate, roll int
		want       bool
	}{{1, 0, true}, {1, 1, false}, {50, 49, true}, {50, 50, false}, {100, 99, true}} {
		w, p, m, _ := killRewardTestWorld(t, nil)
		c := eventCommand(w, "chance-operation-01")
		c.RatePercent = tc.rate
		executeEvent(w, c, true)
		w.rng = fixedRNG{value: tc.roll}
		drops := w.planGlobalDrop(p, m)
		if (len(drops) == 1) != tc.want {
			t.Fatalf("rate=%d roll=%d drops=%d", tc.rate, tc.roll, len(drops))
		}
	}
	w, p, m, _ := killRewardTestWorld(t, nil)
	executeEvent(w, eventCommand(w, "eligible-operation-1"), true)
	m.Def.Tipo = model.TipoNPC
	if len(w.planGlobalDrop(p, m)) != 0 {
		t.Fatal("NPC eligible")
	}
	m.Def.Tipo = model.TipoMonstro
	m.GuildWarTower = true
	if len(w.planGlobalDrop(p, m)) != 0 {
		t.Fatal("tower eligible")
	}
	m.GuildWarTower = false
	p.PersistencePoisoned = true
	if len(w.planGlobalDrop(p, m)) != 0 {
		t.Fatal("poisoned account eligible")
	}
}

func TestGlobalDropGroundFallbackAndFailedCreation(t *testing.T) {
	for _, fail := range []bool{false, true} {
		w, p, m, _ := killRewardTestWorld(t, nil)
		for i := 0; i < model.PlayerCarrySlots; i++ {
			p.Char.Inv[i] = model.Item{Index: 500}
		}
		m.Def.Carry = nil
		m.InstanceID = "test-instance"
		c := eventCommand(w, "ground-operation-01")
		c.MaxDrops = 1
		executeEvent(w, c, true)
		drops := w.planGlobalDrop(p, m)
		if len(drops) != 1 || drops[0].inventoryPos != -1 {
			t.Fatal("expected ground reservation")
		}
		if fail {
			delete(w.items, 500)
		} // ground creation rejects a missing catalog item
		w.publishPlannedDrops(drops)
		w.publishPlannedDrops(nil)
		if w.globalDrop.reserved != 0 {
			t.Fatal("ground reservation leaked")
		}
		if fail {
			if w.globalDrop.status.Dropped != 0 || !w.globalDrop.status.Active {
				t.Fatal("failed creation counted")
			}
		} else {
			if w.globalDrop.status.Dropped != 1 || len(w.groundItems) != 1 {
				t.Fatal("ground reward missing")
			}
			for _, item := range w.groundItems {
				if item.InstanceID != m.InstanceID {
					t.Fatal("drop escaped instance")
				}
			}
		}
	}
}

func TestGlobalDropValidationStopTimeAndEpoch(t *testing.T) {
	w, _, _, _ := killRewardTestWorld(t, nil)
	c := eventCommand(w, "validation-start-01")
	if r := executeEvent(w, c, false); r.Code != "capability_required" || r.Status.Active {
		t.Fatalf("authorization: %+v", r)
	}
	bad := c
	bad.ItemID = 65535
	if r := executeEvent(w, bad, true); r.Code != "unknown_item" {
		t.Fatalf("catalog: %+v", r)
	}
	c.OperationID = "validation-start-02"
	c.MaxDrops = 0
	c.DurationSeconds = 3600
	r := executeEvent(w, c, true)
	if r.Code != "ok" || r.Status.EndsAt == nil {
		t.Fatalf("timed event: %+v", r)
	}
	w.tickGlobalDrop(r.Status.EndsAt.Add(-time.Nanosecond))
	if !w.globalDrop.status.Active {
		t.Fatal("ended before deadline")
	}
	w.tickGlobalDrop(*r.Status.EndsAt)
	if w.globalDrop.status.Active || w.globalDrop.status.EndedBy != "time" {
		t.Fatal("deadline not enforced")
	}
	c = eventCommand(w, "validation-start-03")
	if r := executeEvent(w, c, true); r.Code != "ok" {
		t.Fatal(r)
	}
	stop := control.GlobalDropCommand{Version: 1, OperationID: "validation-stop-001", Epoch: c.Epoch, ExpectedEventID: "old-event", Action: "stop", Reason: "stop fixture"}
	if r := executeEvent(w, stop, true); r.Code != "stale_event" || !r.Status.Active {
		t.Fatal(r)
	}
	stop.OperationID = "validation-stop-002"
	stop.ExpectedEventID = w.globalDropSnapshot().EventID
	if r := executeEvent(w, stop, true); r.Code != "ok" || r.Status.Active || r.Status.EndedBy != "manual" {
		t.Fatal(r)
	}
	w.globalDropEpoch = strings.Repeat("f", 32)
	if r := executeEvent(w, c, true); r.Code != "stale_event" {
		t.Fatal("restart accepted old operation", r)
	}
}

func TestGlobalDropQueueCancellationAndMaintenance(t *testing.T) {
	w, _, _, _ := killRewardTestWorld(t, nil)
	w.commands = make(chan command, 1)
	c := eventCommand(w, "queued-operation-01")
	ctx, cancel := context.WithCancel(context.Background())
	done := make(chan error, 1)
	go func() { _, err := w.GlobalDropCommand(ctx, "operator", c, func() bool { return true }); done <- err }()
	queued := <-w.commands
	if _, err := w.GlobalDropStatus(context.Background()); !errors.Is(err, control.ErrBusy) {
		t.Fatal("unbounded admission", err)
	}
	cancel()
	if err := <-done; !errors.Is(err, context.Canceled) {
		t.Fatal(err)
	}
	w.handle(queued)
	if w.globalDrop != nil || w.globalDropPending.Load() {
		t.Fatal("canceled command applied or admission stuck")
	}
	w.shuttingDown = true
	if r := executeEvent(w, c, true); r.Code != "server_maintenance" || r.Status.Active {
		t.Fatal(r)
	}
}

func TestGlobalDropStopAtReceiptCapacity(t *testing.T) {
	w, _, _, _ := killRewardTestWorld(t, nil)
	if r := executeEvent(w, eventCommand(w, "capacity-start-0001"), true); r.Code != "ok" {
		t.Fatal(r)
	}
	for i := len(w.globalDropReceipts); i < 1024; i++ {
		w.globalDropReceipts[fmt.Sprint(i)] = globalDropReceipt{}
	}
	s := w.globalDropSnapshot()
	c := control.GlobalDropCommand{Version: 1, Epoch: s.Epoch, ExpectedEventID: s.EventID, OperationID: "capacity-stop-0001", Action: "stop", Reason: "test stop"}
	if r := executeEvent(w, c, true); r.Code != "ok" || r.Status.Active {
		t.Fatal("cannot stop at capacity", r)
	}
	if r := executeEvent(w, c, true); !r.Replayed {
		t.Fatal("stop retry", r)
	}
	if r := executeEvent(w, eventCommand(w, "capacity-start-0002"), true); r.Code != "operation_capacity" {
		t.Fatal("unbounded receipts", r)
	}
}
