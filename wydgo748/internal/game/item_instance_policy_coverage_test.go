package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestItemInstanceFinishPolicies(t *testing.T) {
	tests := []struct {
		name      string
		config    model.VolatileInstance
		policy    string
		completes bool
	}{
		{name: "explicit timeout", config: model.VolatileInstance{FinishPolicy: " Finish_On_Timeout "}, policy: "finish_on_timeout"},
		{name: "shared timed default", config: model.VolatileInstance{Mode: " shared_timed_zone "}, policy: "respawn_until_timeout"},
		{name: "multi stage default", config: model.VolatileInstance{Stages: []model.VolatileInstanceStage{{}, {}}}, policy: "advance_on_clear", completes: true},
		{name: "single stage default", config: model.VolatileInstance{}, policy: "finish_on_clear", completes: true},
		{name: "state machine", config: model.VolatileInstance{FinishPolicy: "state_machine"}, policy: "state_machine"},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := instanceFinishPolicy(tt.config); got != tt.policy {
				t.Fatalf("finish policy = %q, want %q", got, tt.policy)
			}
			if got := instanceCompletesOnClear(&ItemInstance{Config: tt.config}); got != tt.completes {
				t.Fatalf("completes on clear = %v, want %v", got, tt.completes)
			}
		})
	}
	if instanceCompletesOnClear(nil) {
		t.Fatal("nil instance completed on clear")
	}
}

func TestItemInstanceStageHelpersHandleEmptyExplicitAndFallbackStages(t *testing.T) {
	if got := instanceStages(nil); got != nil {
		t.Fatalf("nil config stages = %v, want nil", got)
	}
	configured := []model.VolatileInstanceStage{{Name: "First", AreaRadius: 3}, {Name: "Second", AreaRadius: 5}}
	if got := instanceStages(&model.VolatileInstance{Stages: configured}); len(got) != 2 || got[0].Name != "First" || got[1].Name != "Second" {
		t.Fatalf("explicit stages were not retained: %+v", got)
	}
	fallback := instanceStages(&model.VolatileInstance{Name: "Room", X: 2100, Y: 2200,
		SpawnX: 2101, SpawnY: 2201, AreaRadius: 7, DurationSeconds: 90,
		Spawns: []model.VolatileInstanceSpawn{{NPC: "Mob", Count: 2}}})
	if len(fallback) != 1 || fallback[0].Name != "Room" || fallback[0].X != 2100 || fallback[0].Y != 2200 ||
		fallback[0].SpawnX != 2101 || fallback[0].SpawnY != 2201 || fallback[0].AreaRadius != 7 || fallback[0].DurationSeconds != 90 {
		t.Fatalf("legacy stage fallback lost room fields: %+v", fallback)
	}
	if got := instanceSpawnCount(nil); got != 0 {
		t.Fatalf("nil stage spawn count = %d, want zero", got)
	}
	if got := instanceSpawnCount(&model.VolatileInstance{Stages: []model.VolatileInstanceStage{}}); got != 0 {
		t.Fatalf("empty stage spawn count = %d, want zero", got)
	}
	count := instanceSpawnCount(&model.VolatileInstance{Stages: []model.VolatileInstanceStage{
		{Spawns: []model.VolatileInstanceSpawn{{Count: 2}, {Count: 3}}},
		{Spawns: []model.VolatileInstanceSpawn{{Count: 10}}},
	}})
	if count != 5 {
		t.Fatalf("first-stage spawn count = %d, want 5", count)
	}
}

func TestInstanceMobAreaAndStageRejectMissingInstances(t *testing.T) {
	if (*World)(nil).instanceForMob(&Mob{InstanceID: "room"}) != nil {
		t.Fatal("nil world resolved an instance")
	}
	if _, ok := instanceStageForMob(nil); ok {
		t.Fatal("nil instance exposed a stage")
	}
	inst := &ItemInstance{RuntimeID: "room", CurrentStage: 0,
		Config: model.VolatileInstance{X: 2100, Y: 2200, AreaRadius: 2}}
	world := &World{itemInstances: map[string]*ItemInstance{"room": inst}}
	mob := &Mob{ID: 9, InstanceID: "room", X: 2102, Y: 2200}
	if world.instanceForMob(nil) != nil || world.instanceForMob(&Mob{}) != nil || world.instanceMobInsideArea(nil) {
		t.Fatal("missing mob instance identity resolved a room")
	}
	if _, ok := instanceStageForMob(inst); !ok || !world.instanceMobInsideArea(mob) || !world.instanceMobStepAllowed(mob, 2102, 2200) {
		t.Fatal("valid legacy stage rejected a mob on the room boundary")
	}
	if world.instanceMobInsideArea(&Mob{ID: 9, InstanceID: "room", X: 2103, Y: 2200}) ||
		world.instanceMobStepAllowed(mob, 2103, 2200) {
		t.Fatal("room boundary allowed a mob to leave its radius")
	}
	if world.instanceMobInsideArea(&Mob{ID: 10, InstanceID: "missing"}) ||
		world.instanceMobStepAllowed(&Mob{ID: 10, InstanceID: "missing"}, 2100, 2200) {
		t.Fatal("mob without an instance was treated as inside a room")
	}
	inst.CurrentStage = 1
	if _, ok := instanceStageForMob(inst); ok {
		t.Fatal("out-of-range current stage was accepted")
	}
}
