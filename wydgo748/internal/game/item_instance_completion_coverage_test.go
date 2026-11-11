package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestSpawnItemInstanceCompletionWaveRejectsInvalidAndUnavailableSpawns(t *testing.T) {
	for _, tc := range []struct {
		name   string
		change func(*World, *ItemInstance, *model.VolatileInstanceStage, time.Time)
	}{
		{name: "nil instance", change: func(_ *World, _ *ItemInstance, _ *model.VolatileInstanceStage, _ time.Time) {}},
		{name: "empty completion list", change: func(_ *World, _ *ItemInstance, stage *model.VolatileInstanceStage, _ time.Time) {
			stage.CompletionSpawns = nil
		}},
		{name: "schedule expired", change: func(_ *World, inst *ItemInstance, _ *model.VolatileInstanceStage, now time.Time) {
			inst.ScheduleEnd = now
		}},
		{name: "hard deadline expired", change: func(_ *World, inst *ItemInstance, _ *model.VolatileInstanceStage, now time.Time) {
			inst.HardDeadline = now
		}},
		{name: "zero count", change: func(_ *World, _ *ItemInstance, stage *model.VolatileInstanceStage, _ time.Time) {
			stage.CompletionSpawns[0].Count = 0
		}},
		{name: "missing NPC", change: func(_ *World, _ *ItemInstance, stage *model.VolatileInstanceStage, _ time.Time) {
			stage.CompletionSpawns[0].NPC = "MissingCompletionMob"
		}},
		{name: "not a monster", change: func(w *World, _ *ItemInstance, stage *model.VolatileInstanceStage, _ time.Time) {
			w.npcs = append(w.npcs, model.NPCDef{Name: "CompletionNPC"})
			stage.CompletionSpawns[0].NPC = "CompletionNPC"
		}},
		{name: "missing score", change: func(w *World, _ *ItemInstance, stage *model.VolatileInstanceStage, _ time.Time) {
			w.npcs = append(w.npcs, model.NPCDef{Name: "ScorelessCompletionMob", Tipo: model.TipoMonstro})
			stage.CompletionSpawns[0].NPC = "ScorelessCompletionMob"
		}},
		{name: "all stage positions occupied", change: func(w *World, inst *ItemInstance, stage *model.VolatileInstanceStage, _ time.Time) {
			stage.X, stage.Y, stage.SpawnX, stage.SpawnY = 2200, 2200, 2200, 2200
			stage.AreaX, stage.AreaY, stage.AreaRadius = 2200, 2200, 1
			space := itemInstanceGameplaySpace(inst)
			w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
			blocker := w.playersByID[1]
			for y := uint16(2199); y <= 2201; y++ {
				for x := uint16(2199); x <= 2201; x++ {
					w.criticalTeleportTiles[criticalTile(x, y, space)] = blocker
				}
			}
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, _, _, _, clock := instanceTestWorld()
			now := clock.Now()
			inst := &ItemInstance{RuntimeID: "completion", Config: model.VolatileInstance{ID: "completion"}}
			stage := model.VolatileInstanceStage{
				X: 2200, Y: 2200, SpawnX: 2200, SpawnY: 2200, AreaRadius: 8,
				CompletionSpawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
			}
			if tc.name != "nil instance" {
				tc.change(w, inst, &stage, now)
			}
			var target *ItemInstance
			if tc.name != "nil instance" {
				target = inst
			}
			if w.spawnItemInstanceCompletionWave(target, stage, now) || len(inst.MobIDs) != 0 || len(w.mobs) != 0 {
				t.Fatalf("invalid completion wave created entities: instance=%+v mobs=%v", inst, w.mobs)
			}
		})
	}
}
