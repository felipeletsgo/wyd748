package game

import (
	"errors"
	"time"

	"wydgo/internal/model"
)

// Wave preparation only changes the detached instance and the admission's
// allocation plan. Entity IDs are not published or inserted into World here.
func (w *World) planCriticalInstanceFirstWave(inst *ItemInstance, stage model.VolatileInstanceStage,
	at time.Time, tiles map[criticalTeleportTile]*Player, owner *Player) ([]*Mob, error) {
	space := itemInstanceGameplaySpace(inst)
	ignored := make(map[*Player]struct{}, len(inst.MemberIDs))
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil {
			ignored[member] = struct{}{}
		}
	}
	findFree := func(baseX, baseY uint16) (uint16, uint16, bool) {
		radius := stage.AreaRadius
		if radius < 4 {
			radius = 4
		}
		for distance := 0; distance <= radius; distance++ {
			for dy := -distance; dy <= distance; dy++ {
				for dx := -distance; dx <= distance; dx++ {
					if distance > 0 && absInt(dx) != distance && absInt(dy) != distance {
						continue
					}
					nx, ny := int(baseX)+dx, int(baseY)+dy
					if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
						continue
					}
					x, y := uint16(nx), uint16(ny)
					tile := criticalTile(x, y, space)
					if tiles[tile] != nil || w.positionOccupiedInGameplaySpace(x, y, space, nil, nil, ignored) {
						continue
					}
					return x, y, true
				}
			}
		}
		return 0, 0, false
	}
	created := make([]*Mob, 0)
	ids := make(map[uint16]struct{})
	for _, spawn := range stage.Spawns {
		def := w.npcDefByName(spawn.NPC)
		if spawn.Count <= 0 || def == nil || !def.IsMonster() || def.Score == nil {
			return nil, errors.New("An instance monster is not configured.")
		}
		for n := 0; n < spawn.Count; n++ {
			spawnX, spawnY := stage.SpawnX, stage.SpawnY
			if spawn.X != 0 && spawn.Y != 0 {
				spawnX, spawnY = spawn.X, spawn.Y
			}
			x, y, ok := findFree(spawnX, spawnY)
			if !ok {
				return nil, errors.New("The instance monster area is blocked.")
			}
			var id uint16
			for attempts := 0; attempts < 65536; attempts++ {
				id = w.allocMobID()
				if id == 0 {
					break
				}
				if _, used := ids[id]; !used {
					break
				}
				id = 0
			}
			if id == 0 {
				return nil, errors.New("No instance monster ID is available.")
			}
			ids[id] = struct{}{}
			runtimeID := inst.RuntimeID
			if runtimeID == "" {
				runtimeID = inst.Config.ID
			}
			created = append(created, &Mob{ID: id, Def: def, X: x, Y: y,
				HP: def.Score.MaxHP, GenerIndex: -1, InstanceID: runtimeID})
			tiles[criticalTile(x, y, space)] = owner
		}
	}
	inst.CurrentStage, inst.Remaining = 0, len(created)
	inst.MobIDs = ids
	inst.TransitionAt, inst.TransitionDeadline = time.Time{}, time.Time{}
	inst.QuizAt, inst.QuizDeadline = time.Time{}, time.Time{}
	inst.BossSpawned = false
	var deadline time.Time
	if _, hellGate := hellGateInstance(inst); hellGate && inst.Config.ActiveDurationSeconds > 0 {
		deadline = inst.HardDeadline
	} else if duration := stageDuration(inst.Config, stage); duration > 0 {
		deadline = at.Add(time.Duration(duration) * time.Second)
	}
	if !instanceAdmissionOnlySchedule(inst) && !inst.ScheduleEnd.IsZero() && inst.ScheduleEnd.Before(deadline) {
		deadline = inst.ScheduleEnd
	}
	if !inst.HardDeadline.IsZero() && inst.HardDeadline.Before(deadline) {
		deadline = inst.HardDeadline
	}
	setInstanceCombatDeadline(inst, deadline)
	if isBigCubeInstance(inst) {
		if stage.Quiz == nil || stage.Quiz.DurationSeconds <= 0 || len(inst.MemberIDs) == 0 {
			return nil, errors.New("The instance question is not configured.")
		}
		inst.State = "big_cube_quiz"
		inst.QuizAt = at.Add(time.Duration(stage.Quiz.DurationSeconds) * time.Second)
		inst.QuizDeadline = inst.QuizAt
	}
	return created, nil
}
