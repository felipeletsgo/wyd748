package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestInstanceAreaOccupiedRespectsRuntimeAndEntryIgnore(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	leader.X, leader.Y = 2200, 2200
	member.X, member.Y = 2200, 2200
	w.itemInstances["room-a"] = &ItemInstance{RuntimeID: "room-a", MemberIDs: []uint16{leader.ID}}
	w.setPlayerInstanceIndex(leader.ID, "room-a")

	cfg := &model.VolatileInstance{ID: "room-a", X: 2200, Y: 2200, AreaRadius: 8}
	if !w.instanceAreaOccupied(cfg, "room-a") {
		t.Fatal("player in the same runtime did not block room entry")
	}
	if w.instanceAreaOccupied(cfg, "room-b") {
		t.Fatal("player in another runtime blocked room entry")
	}

	cfg.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: 2199, MinY: 2199, MaxX: 2201, MaxY: 2201}}
	if w.instanceAreaOccupied(cfg, "room-a", leader) {
		t.Fatal("entering member in a declared entry area was not ignored")
	}
	if !w.instanceAreaOccupied(cfg, "room-a") {
		t.Fatal("entry-area presence was ignored without an explicit player exclusion")
	}

	leader.InWorld = false
	if w.instanceAreaOccupied(cfg, "room-a") {
		t.Fatal("player outside the world blocked room entry")
	}
	if !w.instanceAreaOccupied(nil, "room-a") {
		t.Fatal("missing config must fail closed")
	}
}

func TestClearItemInstanceHUDSendsBothResetSignals(t *testing.T) {
	player, _ := networkedTestPlayer(1, "HUD", 2100, 2100)
	before := player.Session.QueuedPacketsForTest()
	clearItemInstanceHUD(player)
	if got := player.Session.QueuedPacketsForTest(); got != before+2 {
		t.Fatalf("expected two HUD reset packets, got %d (before %d)", got-before, before)
	}

	clearItemInstanceHUD(nil)
	player.Session = nil
	clearItemInstanceHUD(player)
}

func TestInstanceAdmissionOnlySchedulePolicies(t *testing.T) {
	if instanceAdmissionOnlySchedule(nil) {
		t.Fatal("nil instance must not reserve an admission-only schedule")
	}
	if !instanceAdmissionOnlySchedule(&ItemInstance{Config: model.VolatileInstance{
		Mode: "shared_timed_zone", StateMachine: "hell_gate",
	}}) {
		t.Fatal("shared Hell Gate schedule should close admission only")
	}
	if !instanceAdmissionOnlySchedule(&ItemInstance{Config: model.VolatileInstance{
		Mode: "shared_timed_zone", NightmareTier: "normal",
		Schedule: []model.VolatileInstanceWindow{{StartMinute: 10, DurationSeconds: 30}},
	}}) {
		t.Fatal("scheduled Nightmare should close admission only")
	}
	if instanceAdmissionOnlySchedule(&ItemInstance{Config: model.VolatileInstance{
		Mode: "shared_timed_zone", StateMachine: "other",
	}}) {
		t.Fatal("ordinary shared timed zone must keep its schedule as a combat limit")
	}
}

func TestCompleteItemInstanceRejectsExpiredAndClosedRooms(t *testing.T) {
	w, _, _, st, clock := instanceTestWorld()
	now := clock.Now()
	cases := []struct {
		name string
		inst *ItemInstance
	}{
		{name: "already closed", inst: &ItemInstance{ExitAt: now}},
		{name: "combat deadline", inst: &ItemInstance{CombatDeadline: now}},
		{name: "hard deadline", inst: &ItemInstance{HardDeadline: now}},
		{name: "schedule deadline", inst: &ItemInstance{
			Config: model.VolatileInstance{Mode: "private_chain"}, ScheduleEnd: now,
		}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			w.completeItemInstance(tc.inst, now)
			if tc.inst.RewardGranted || !tc.inst.ExitAt.Equal(now) && !tc.inst.ExitAt.IsZero() ||
				st.gameSaves != 0 {
				t.Fatalf("closed room changed reward state or persisted: inst=%+v saves=%d",
					tc.inst, st.gameSaves)
			}
		})
	}
	w.completeItemInstance(nil, now)
}

func TestCompleteItemInstanceHonorsAdmissionOnlySchedule(t *testing.T) {
	w, _, _, st, clock := instanceTestWorld()
	now := clock.Now()
	inst := &ItemInstance{
		Config: model.VolatileInstance{
			ID: "nightmare-admission-ended", Mode: "shared_timed_zone",
			NightmareTier: "normal", RewardItem: 0,
			Schedule: []model.VolatileInstanceWindow{{StartMinute: 10, DurationSeconds: 60}},
		},
		RuntimeID: "nightmare-admission-ended", ScheduleEnd: now,
	}
	w.itemInstances[inst.RuntimeID] = inst
	w.completeItemInstance(inst, now)
	if !inst.RewardGranted || inst.ExitAt.IsZero() || st.saves != 1 || len(st.snapshots) != 1 {
		t.Fatalf("closed admission window prevented an active encounter from completing: inst=%+v rewardSaves=%d snapshots=%d",
			inst, st.saves, len(st.snapshots))
	}
}
