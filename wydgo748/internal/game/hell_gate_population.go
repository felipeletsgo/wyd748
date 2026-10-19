package game

import "wydgo/internal/model"

// byte_63C300/byte_63C301, formation zero, used by sub_514310.
var hellGateFollowerOffsets759 = [12][2]int{
	{1, 1}, {-1, 1}, {1, -1}, {-1, -1}, {1, 0}, {-1, 0},
	{0, 1}, {0, -1}, {2, 0}, {-2, 0}, {0, 2}, {0, -2},
}

func hellGateFormationAnchor(spawn model.VolatileInstanceSpawn, index int) (uint16, uint16, bool) {
	if spawn.Formation759 == 0 || spawn.Formation759 == 1 && index == 0 {
		return spawn.X, spawn.Y, true
	}
	if spawn.Formation759 == 1 {
		index--
	}
	if spawn.Formation759 < 1 || spawn.Formation759 > 2 || index < 0 || index >= len(hellGateFollowerOffsets759) {
		return 0, 0, false
	}
	offset := hellGateFollowerOffsets759[index]
	x, y := int(spawn.X)+offset[0], int(spawn.Y)+offset[1]
	if x <= 0 || y <= 0 || x > 65535 || y > 65535 {
		return 0, 0, false
	}
	return uint16(x), uint16(y), true
}

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
	if spawn.Formation759 == 1 && len(created) > 0 {
		spawn.X, spawn.Y = created[0].X, created[0].Y
	}
	for index, mob := range created {
		member := spawn
		member.X, member.Y, _ = hellGateFormationAnchor(spawn, index)
		member.Count, member.Formation759 = 1, 0
		inst.MobIDs[mob.ID] = struct{}{}
		inst.HellGateDefenseSpawns[mob.ID] = member
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
