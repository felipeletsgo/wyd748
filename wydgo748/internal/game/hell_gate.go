package game

import (
	"strings"
	"time"

	"wydgo/internal/model"
)

// hellGateInstance identifies Hell Gate without spreading name comparisons
// throughout the generic room engine.
func hellGateInstance(inst *ItemInstance) (*model.VolatileHellGate, bool) {
	if inst == nil || !strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate") ||
		inst.Config.HellGate == nil {
		return nil, false
	}
	return inst.Config.HellGate, true
}

func (w *World) spawnHellGateWave(inst *ItemInstance, spawns []model.VolatileInstanceSpawn,
	quadrant uint8, publish bool) ([]*Mob, bool) {
	if inst == nil || len(spawns) == 0 {
		return nil, false
	}
	cfg, _ := hellGateInstance(inst)
	created := make([]*Mob, 0)
	reserved := make(map[uint32]struct{})
	rollback := func() {
		for _, mob := range created {
			if inst.MobQuadrants != nil {
				delete(inst.MobQuadrants, mob.ID)
			}
			if publish {
				w.publishMobRemoval(mob)
			}
			w.removeMobInstance(mob)
		}
	}
	for _, spawn := range spawns {
		def := w.npcDefByName(spawn.NPC)
		if def == nil || !def.IsMonster() || def.Score == nil || spawn.Count <= 0 {
			rollback()
			return nil, false
		}
		searchRadius := inst.Config.AreaRadius
		if quadrant != 0 && cfg != nil {
			for _, q := range cfg.Quadrants {
				if uint8(q.ID) == quadrant {
					searchRadius = q.AreaRadius
					break
				}
			}
		}
		if searchRadius <= 0 {
			searchRadius = 1
		}
		searchRadius = instanceSpawnRadius(spawn, searchRadius)
		formation := spawn
		for n := 0; n < spawn.Count; n++ {
			baseX, baseY := inst.Config.X, inst.Config.Y
			if spawn.X != 0 && spawn.Y != 0 {
				baseX, baseY = spawn.X, spawn.Y
			}
			if hellGatePopulation(inst) != nil && spawn.Formation759 != 0 {
				var valid bool
				baseX, baseY, valid = hellGateFormationAnchor(formation, n)
				if !valid {
					rollback()
					return nil, false
				}
			}
			// Explicit coordinates are authoritative, but a small deterministic
			// search resolves a tile occupied by a previous member/mob.
			var ok bool
			var x, y uint16
			for distance := 0; distance <= searchRadius; distance++ {
				x, y, ok = findHellGatePosition(w, inst.RuntimeID, baseX, baseY, distance, reserved)
				if ok {
					break
				}
			}
			if !ok {
				rollback()
				return nil, false
			}
			if n == 0 && spawn.Formation759 == 1 {
				formation.X, formation.Y = x, y
			}
			mobID := w.allocMobID()
			if mobID == 0 {
				rollback()
				return nil, false
			}
			mob := &Mob{ID: mobID, Def: def, X: x, Y: y,
				HP: def.Score.MaxHP, GenerIndex: -1, InstanceID: inst.RuntimeID}
			w.appendMobInstance(mob)
			w.mobsByID[mob.ID] = mob
			reserved[uint32(x)<<16|uint32(y)] = struct{}{}
			created = append(created, mob)
			if quadrant != 0 {
				if inst.MobQuadrants == nil {
					inst.MobQuadrants = make(map[uint16]uint8)
				}
				inst.MobQuadrants[mob.ID] = quadrant
			}
		}
	}
	for _, mob := range created {
		if publish {
			w.publishMobSpawn(mob)
		} else {
			w.registerMobSpatial(mob)
		}
	}
	return created, true
}

func findHellGatePosition(w *World, instanceID string, baseX, baseY uint16, distance int,
	reserved map[uint32]struct{}) (uint16, uint16, bool) {
	if distance < 0 {
		distance = 0
	}
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
			key := uint32(x)<<16 | uint32(y)
			if _, used := reserved[key]; used || w.findMobPositionOccupied(instanceID, x, y) ||
				!w.terrain.Walkable(x, y) || !w.terrain.LineOfSight(baseX, baseY, x, y) {
				continue
			}
			return x, y, true
		}
	}
	return 0, 0, false
}

func (w *World) findMobPositionOccupied(instanceID string, x, y uint16) bool {
	if strings.TrimSpace(instanceID) == "" {
		return w.positionOccupied(x, y, nil)
	}
	return w.positionOccupiedInGameplaySpace(x, y, instanceID, nil, nil, nil)
}

func (w *World) removeHellGateMobs(inst *ItemInstance) {
	if inst == nil {
		return
	}
	for id := range inst.MobIDs {
		if mob := w.mobsByID[id]; mob != nil {
			w.publishMobRemoval(mob)
			w.removeMobInstance(mob)
		}
	}
	for id := range inst.NPCIDs {
		if npc := w.mobsByID[id]; npc != nil {
			w.publishMobRemoval(npc)
			w.removeMobInstance(npc)
		}
	}
	inst.MobIDs = make(map[uint16]struct{})
	inst.NPCIDs = make(map[uint16]struct{})
	inst.MobQuadrants = make(map[uint16]uint8)
	inst.HellGateLichIDs = make(map[uint8]uint16)
	inst.HellGateRouteDistances = nil
	inst.HellGateDefenseSpawns = nil
	inst.HellGateReplacements = nil
	inst.Remaining = 0
}

func hellGateBit(quadrant uint8) uint8 {
	if quadrant < 1 || quadrant > 8 {
		return 0
	}
	return 1 << (quadrant - 1)
}

func (w *World) spawnHellGateQuadrants(inst *ItemInstance, now time.Time) bool {
	cfg, ok := hellGateInstance(inst)
	if !ok || len(cfg.Quadrants) != 4 {
		return false
	}
	w.removeHellGateMobs(inst)
	inst.State = "lich_puzzle"
	inst.HellGateClearedMask = 0
	inst.HellGateLichSpawnedMask = 0
	inst.HellGateLichIDs = make(map[uint8]uint16)
	inst.HellGateWrongLich = false
	inst.MobIDs = make(map[uint16]struct{})
	for _, quadrant := range cfg.Quadrants {
		created, ok := w.spawnHellGateWave(inst, hellGateRoomSpawns(inst, quadrant), uint8(quadrant.ID), true)
		if !ok {
			w.removeHellGateMobs(inst)
			return false
		}
		for _, mob := range created {
			inst.MobIDs[mob.ID] = struct{}{}
			inst.Remaining++
		}
	}
	if inst.CombatDeadline.IsZero() {
		deadline := inst.HardDeadline
		if deadline.IsZero() && inst.Config.ActiveDurationSeconds > 0 {
			deadline = now.Add(time.Duration(inst.Config.ActiveDurationSeconds) * time.Second)
		}
		setInstanceCombatDeadline(inst, deadline)
	}
	w.sendItemInstanceStageStatus(inst)
	return true
}

func (w *World) spawnHellGateLich(inst *ItemInstance, quadrant uint8) bool {
	cfg, ok := hellGateInstance(inst)
	if !ok {
		return false
	}
	for _, q := range cfg.Quadrants {
		if uint8(q.ID) != quadrant {
			continue
		}
		if inst.HellGateLichSpawnedMask&hellGateBit(quadrant) != 0 {
			return true
		}
		created, ok := w.spawnHellGateWave(inst, []model.VolatileInstanceSpawn{q.Lich}, quadrant, true)
		if !ok || len(created) != 1 {
			return false
		}
		if inst.MobIDs == nil {
			inst.MobIDs = make(map[uint16]struct{})
		}
		if inst.HellGateLichIDs == nil {
			inst.HellGateLichIDs = make(map[uint8]uint16)
		}
		inst.MobIDs[created[0].ID] = struct{}{}
		inst.HellGateLichIDs[quadrant] = created[0].ID
		inst.HellGateLichSpawnedMask |= hellGateBit(quadrant)
		inst.Remaining++
		if population := hellGatePopulation(inst); population != nil {
			escorts, ok := w.spawnHellGateWave(inst, population.LichEscorts[q.ID-1], quadrant, true)
			if !ok {
				return false
			}
			for _, escort := range escorts {
				inst.MobIDs[escort.ID] = struct{}{}
				inst.Remaining++
			}
		}
		w.sendItemInstanceStageStatus(inst)
		return true
	}
	return false
}

func (w *World) spawnHellGateFinal(inst *ItemInstance, now time.Time) bool {
	cfg, ok := hellGateInstance(inst)
	if !ok {
		return false
	}
	w.removeHellGateMobs(inst)
	inst.State = "final"
	if hellGateVersion2(inst) {
		inst.State = "protection"
	}
	for _, spawn := range hellGateFinalSpawns(inst) {
		if !w.spawnHellGateDefenseGroup(inst, spawn) {
			w.removeHellGateMobs(inst)
			return false
		}
	}
	inst.Remaining = len(inst.MobIDs)
	for _, spawn := range cfg.FinalNPCs {
		def := w.npcDefByName(spawn.NPC)
		if def == nil || def.Score == nil || def.IsMonster() || spawn.Count <= 0 {
			w.removeHellGateMobs(inst)
			return false
		}
		for n := 0; n < spawn.Count; n++ {
			// A member standing on the protector anchor must not cancel the
			// encounter. Stay in the anchor's local, reachable terrain component.
			var x, y uint16
			var found bool
			for distance := 0; distance <= 4; distance++ {
				x, y, found = findHellGatePosition(w, inst.RuntimeID, spawn.X, spawn.Y, distance, nil)
				if found {
					break
				}
			}
			if !found {
				w.removeHellGateMobs(inst)
				return false
			}
			id := w.allocMobID()
			if id == 0 {
				w.removeHellGateMobs(inst)
				return false
			}
			hp := def.Score.MaxHP
			if hellGateVersion2(inst) {
				if inst.HellGateDionysHP == 0 {
					inst.HellGateDionysHP = hp
				}
				hp = min(hp, inst.HellGateDionysHP)
			}
			mob := &Mob{ID: id, Def: def, X: x, Y: y, HP: hp,
				GenerIndex: -1, InstanceID: inst.RuntimeID}
			w.appendMobInstance(mob)
			w.mobsByID[id] = mob
			if inst.NPCIDs == nil {
				inst.NPCIDs = make(map[uint16]struct{})
			}
			inst.NPCIDs[id] = struct{}{}
			w.publishMobSpawn(mob)
		}
	}
	// Attackers were registered before the protector existed. Wake them now,
	// including groups outside both player proximity and normal aggro range.
	for id := range inst.MobIDs {
		w.recomputeMobActive(w.mobsByID[id])
	}
	if inst.Remaining == 0 && !hellGateVersion2(inst) {
		w.completeItemInstance(inst, now)
	} else {
		w.sendItemInstanceStageStatus(inst)
	}
	return true
}

// onHellGateMobKilled consumes deaths before the linear room engine. A Lich
// cannot unlock the gate twice, and the client cannot select the valid pair.
func (w *World) onHellGateMobKilled(inst *ItemInstance, mob *Mob, now time.Time) bool {
	cfg, ok := hellGateInstance(inst)
	if !ok || mob == nil {
		return false
	}
	if _, exists := inst.MobIDs[mob.ID]; !exists {
		return true
	}
	delete(inst.MobIDs, mob.ID)
	if inst.Remaining > 0 {
		inst.Remaining--
	}
	quadrant := inst.MobQuadrants[mob.ID]
	delete(inst.MobQuadrants, mob.ID)
	if hellGateVersion2(inst) {
		w.onHellGateVersion2Killed(inst, mob, quadrant, now)
		return true
	}
	if inst.State == "entry" && strings.EqualFold(mob.Def.Name, cfg.ControllerNPC) {
		inst.HellGateVariant = w.intn(2) + 1
		if inst.HellGateVariant == 1 {
			inst.HellGateValidLichMask = hellGateBit(2) | hellGateBit(4)
		} else {
			inst.HellGateValidLichMask = hellGateBit(1) | hellGateBit(3)
		}
		if !w.spawnHellGateQuadrants(inst, now) {
			setInstanceCombatDeadline(inst, now)
		}
		return true
	}
	if inst.State == "lich_puzzle" && quadrant != 0 {
		lichID, isLich := inst.HellGateLichIDs[quadrant]
		if isLich && lichID == mob.ID {
			inst.HellGateClearedMask |= hellGateBit(quadrant)
			if hellGateBit(quadrant)&inst.HellGateValidLichMask == 0 {
				// Keep the diagnostic flag for logging/state replication, but do
				// not make it a permanent lock. The configured gate opens when
				// the two authoritatively selected Liches are defeated; killing an
				// unselected Lich does not change that selected pair.
				inst.HellGateWrongLich = true
			}
			if inst.HellGateClearedMask&inst.HellGateValidLichMask == inst.HellGateValidLichMask {
				if !w.spawnHellGateFinal(inst, now) {
					setInstanceCombatDeadline(inst, now)
				}
				return true
			}
		} else {
			remainingQuadrant := false
			for id, q := range inst.MobQuadrants {
				if q == quadrant {
					remainingQuadrant = true
					_ = id
					break
				}
			}
			if !remainingQuadrant && inst.HellGateLichSpawnedMask&hellGateBit(quadrant) == 0 {
				if !w.spawnHellGateLich(inst, quadrant) {
					setInstanceCombatDeadline(inst, now)
				}
			}
		}
		w.sendItemInstanceStageStatus(inst)
		return true
	}
	if inst.State == "final" && inst.Remaining == 0 {
		w.completeItemInstance(inst, now)
		return true
	}
	return true
}

// hellGateAcceptsFinalState lets tickItemInstances finish the encounter even
// though the zone itself remains alive until ScheduleEnd for other entrants.
func hellGateAcceptsFinalState(inst *ItemInstance) bool {
	return inst != nil && !hellGateVersion2(inst) && strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate") &&
		inst.State == "final" && inst.Remaining == 0
}
