package game

import "wydgo/internal/model"

// Historical populations are opt-in. Restored version-2 encounters retain the
// published population even when their template also contains version-3 data.
func hellGatePopulation(inst *ItemInstance) *model.VolatileHellGatePopulation {
	cfg, ok := hellGateInstance(inst)
	if !ok || cfg.Version != 3 {
		return nil
	}
	return cfg.Population759
}

func hellGateRoomSpawns(inst *ItemInstance, q model.VolatileHellGateQuadrant) []model.VolatileInstanceSpawn {
	if p := hellGatePopulation(inst); p != nil && q.ID >= 1 && q.ID <= len(p.Rooms) {
		return p.Rooms[q.ID-1]
	}
	return q.Spawns
}

func hellGateFinalSpawns(inst *ItemInstance) []model.VolatileInstanceSpawn {
	if p := hellGatePopulation(inst); p != nil {
		return p.FinalSpawns
	}
	return inst.Config.HellGate.FinalSpawns
}

func (w *World) spawnHellGateDefenseGroup(inst *ItemInstance, spawn model.VolatileInstanceSpawn) bool {
	created, ok := w.spawnHellGateWave(inst, []model.VolatileInstanceSpawn{spawn}, 0, true)
	if !ok {
		return false
	}
	if inst.HellGateDefenseSpawns == nil {
		inst.HellGateDefenseSpawns = make(map[uint16]model.VolatileInstanceSpawn)
	}
	// Replacements preserve the original generator anchor and NPC, not a dead
	// actor's last combat position. Each casualty returns independently.
	spawn.Count = 1
	for _, mob := range created {
		inst.MobIDs[mob.ID] = struct{}{}
		inst.HellGateDefenseSpawns[mob.ID] = spawn
	}
	return true
}

func (w *World) replenishHellGateAttackers(inst *ItemInstance) {
	pending := inst.HellGateReplacements
	inst.HellGateReplacements = nil
	for _, spawn := range pending {
		if !w.spawnHellGateDefenseGroup(inst, spawn) {
			// Capacity or occupancy can temporarily block a generator. Retry at
			// most once per event tick, without resetting surviving combatants.
			inst.HellGateReplacements = append(inst.HellGateReplacements, spawn)
		}
	}
	inst.Remaining = len(inst.MobIDs)
}
