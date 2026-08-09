package game

import (
	"strings"
	"time"

	"wydgo/internal/model"
)

// hellGateInstance identifica a configuracao de Porto Infernal sem espalhar
// comparacoes de nomes pelo motor generico de salas.
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
		if def == nil || def.Extended == nil || spawn.Count <= 0 {
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
		for n := 0; n < spawn.Count; n++ {
			baseX, baseY := inst.Config.X, inst.Config.Y
			if spawn.X != 0 && spawn.Y != 0 {
				baseX, baseY = spawn.X, spawn.Y
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
			mobID := w.allocMobID()
			if mobID == 0 {
				rollback()
				return nil, false
			}
			mob := &Mob{ID: mobID, Def: def, X: x, Y: y,
				HP: def.Extended.MaxHP, GenerIndex: -1, InstanceID: inst.RuntimeID}
			w.mobs = append(w.mobs, mob)
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
				!w.terrain.Walkable(x, y) {
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
		created, ok := w.spawnHellGateWave(inst, quadrant.Spawns, uint8(quadrant.ID), true)
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
	created, ok := w.spawnHellGateWave(inst, cfg.FinalSpawns, 0, true)
	if !ok && len(cfg.FinalSpawns) > 0 {
		return false
	}
	inst.MobIDs = make(map[uint16]struct{}, len(created))
	for _, mob := range created {
		inst.MobIDs[mob.ID] = struct{}{}
	}
	inst.Remaining = len(created)
	for _, spawn := range cfg.FinalNPCs {
		def := w.npcDefByName(spawn.NPC)
		if def == nil || def.Extended == nil || def.IsMonster() || spawn.Count <= 0 {
			w.removeHellGateMobs(inst)
			return false
		}
		for n := 0; n < spawn.Count; n++ {
			x, y, found := findHellGatePosition(w, inst.RuntimeID, spawn.X, spawn.Y, 0, map[uint32]struct{}{})
			if !found {
				w.removeHellGateMobs(inst)
				return false
			}
			id := w.allocMobID()
			if id == 0 {
				w.removeHellGateMobs(inst)
				return false
			}
			mob := &Mob{ID: id, Def: def, X: x, Y: y, HP: def.Extended.MaxHP,
				GenerIndex: -1, InstanceID: inst.RuntimeID}
			w.mobs = append(w.mobs, mob)
			w.mobsByID[id] = mob
			if inst.NPCIDs == nil {
				inst.NPCIDs = make(map[uint16]struct{})
			}
			inst.NPCIDs[id] = struct{}{}
			w.publishMobSpawn(mob)
		}
	}
	if inst.Remaining == 0 {
		w.completeItemInstance(inst, now)
	} else {
		w.sendItemInstanceStageStatus(inst)
	}
	return true
}

// onHellGateMobKilled consome o evento de morte antes do motor linear. Isso
// garante que um Lich nao possa ser morto duas vezes para liberar a porta e
// que a combinacao escolhida nao seja determinada pelo pacote do cliente.
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
				// not make it a permanent lock. The native gate opens as soon as
				// the two autoritatively selected Lichs are defeated; killing an
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
	return inst != nil && strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate") &&
		inst.State == "final" && inst.Remaining == 0
}
