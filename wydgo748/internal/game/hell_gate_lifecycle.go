package game

import (
	"log"
	"math/bits"
	"time"

	"wydgo/internal/model"
)

// Versions 2 and 3 use timed protection. Version 3 restores the 7.59 random
// Lich pair, selected portal and optional generator populations. Duration,
// combat balance and completion reward remain explicit project choices.
func hellGateVersion2(inst *ItemInstance) bool {
	cfg, ok := hellGateInstance(inst)
	return ok && (cfg.Version == 2 || cfg.Version == 3)
}

func prepareHellGateEntry(inst *ItemInstance) {
	if !hellGateVersion2(inst) || inst.ScheduleEnd.IsZero() {
		return
	}
	inst.State, inst.Remaining = "preparing", 0
	setInstanceCombatDeadline(inst, inst.HardDeadline)
}

func instanceProtectionActive(inst *ItemInstance) bool {
	return scheduledNightmare(inst) && inst.State == "combat" ||
		hellGateVersion2(inst) && inst.State == "protection"
}

func (w *World) failHellGate(inst *ItemInstance, now time.Time) {
	inst.State = "failed"
	inst.HardDeadline = now
	setInstanceCombatDeadline(inst, now)
	w.markInstanceStateDirty()
}

// Return false only when the generic engine should perform durable cleanup.
// Expiration must not discard an unsettled or retryable EXP grant.
func (w *World) tickHellGate(inst *ItemInstance, now time.Time) bool {
	if !hellGateVersion2(inst) || inst.State == "failed" {
		return false
	}
	if !inst.ExitAt.IsZero() {
		return now.Before(inst.ExitAt)
	}
	if len(inst.MemberIDs) == 0 && len(w.pendingInstanceMembers[inst.RuntimeID]) == 0 {
		w.failHellGate(inst, now)
		return false
	}
	if !inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline) {
		if inst.State != "protection" || !w.hellGateDionysAlive(inst) {
			w.failHellGate(inst, now)
			return false
		}
		if w.criticalWrites != nil {
			if err := w.queueCriticalInstanceReward(inst, now); err != nil {
				log.Printf("INSTANCE %q: Hell Gate reward pending: %v", inst.RuntimeID, err)
			}
		} else {
			w.grantHellGateExperience(inst, now)
		}
		return true
	}
	if inst.State == "preparing" && !now.Before(inst.ScheduleEnd) {
		inst.State = "entry"
		if ok, _ := w.spawnItemInstanceStage(inst, 0, now, false, true); !ok {
			w.failHellGate(inst, now)
			return false
		}
		w.markInstanceStateDirty()
	}
	if inst.State == "protection" && hellGatePopulation(inst) != nil && w.hellGateDionysAlive(inst) {
		w.replenishHellGateAttackers(inst)
	}
	return true
}

func (w *World) spawnHellGateCorridors(inst *ItemInstance) bool {
	cfg := inst.Config.HellGate
	w.removeHellGateMobs(inst)
	inst.State = "corridors"
	for index, spawn := range cfg.Corridors {
		if inst.HellGateClearedMask&hellGateBit(uint8(index+1)) != 0 {
			continue
		}
		created, ok := w.spawnHellGateWave(inst, []model.VolatileInstanceSpawn{spawn}, 0, true)
		if !ok || len(created) != 1 {
			w.removeHellGateMobs(inst)
			return false
		}
		mob := created[0]
		inst.MobIDs[mob.ID] = struct{}{}
		inst.MobQuadrants[mob.ID] = uint8(index + 1)
		inst.Remaining++
	}
	if population := hellGatePopulation(inst); population != nil {
		created, ok := w.spawnHellGateWave(inst, population.CorridorEscorts, 0, true)
		if !ok {
			w.removeHellGateMobs(inst)
			return false
		}
		for _, mob := range created {
			inst.MobIDs[mob.ID] = struct{}{}
			inst.Remaining++
		}
	}
	w.sendItemInstanceStageStatus(inst)
	return inst.Remaining > 0
}

func (w *World) spawnHellGateDefenseWave(inst *ItemInstance) bool {
	for _, spawn := range hellGateFinalSpawns(inst) {
		if !w.spawnHellGateDefenseGroup(inst, spawn) {
			return false
		}
	}
	inst.Remaining = len(inst.MobIDs)
	w.sendItemInstanceStageStatus(inst)
	return true
}

func (w *World) onHellGateVersion2Killed(inst *ItemInstance, mob *Mob, quadrant uint8, now time.Time) {
	switch inst.State {
	case "entry":
		inst.HellGateVariant = w.intn(len(inst.Config.HellGate.Corridors)) + 1
		inst.HellGateClearedMask = 0
		if !w.spawnHellGateCorridors(inst) {
			w.failHellGate(inst, now)
		}
	case "corridors":
		if quadrant == 0 {
			return // Escorts never select a corridor or unlock a portal.
		}
		inst.HellGateClearedMask |= hellGateBit(quadrant)
		if int(quadrant) == inst.HellGateVariant {
			first := w.intn(4)
			inst.HellGateValidLichMask = hellGateBit(uint8(first + 1))
			if inst.Config.HellGate.Version == 3 {
				// sub_578700: choose a second distinct room, not a fixed opposite.
				second := (first + w.intn(3) + 1) % 4
				inst.HellGateValidLichMask |= hellGateBit(uint8(second + 1))
			}
			if !w.spawnHellGateQuadrants(inst, now) {
				w.failHellGate(inst, now)
			}
		}
	case "lich_puzzle":
		if inst.HellGateLichIDs[quadrant] == mob.ID {
			inst.HellGateClearedMask |= hellGateBit(quadrant)
			if inst.HellGateClearedMask&inst.HellGateValidLichMask == inst.HellGateValidLichMask {
				if !w.spawnHellGateFinal(inst, now) {
					w.failHellGate(inst, now)
				}
			} else if hellGateBit(quadrant)&inst.HellGateValidLichMask == 0 {
				inst.HellGateWrongLich = true
			}
		} else {
			for _, remainingQuadrant := range inst.MobQuadrants {
				if remainingQuadrant == quadrant {
					return
				}
			}
			if !w.spawnHellGateLich(inst, quadrant) {
				w.failHellGate(inst, now)
			}
		}
	case "protection":
		if hellGatePopulation(inst) != nil {
			if spawn, ok := inst.HellGateDefenseSpawns[mob.ID]; ok {
				delete(inst.HellGateDefenseSpawns, mob.ID)
				inst.HellGateReplacements = append(inst.HellGateReplacements, spawn)
			}
			break
		}
		// Clearing attackers is not victory: replenish pressure until the
		// event deadline without recreating or healing the protected actor.
		if inst.Remaining == 0 && !w.spawnHellGateDefenseWave(inst) {
			w.failHellGate(inst, now)
		}
	}
	w.markInstanceStateDirty()
}

func (w *World) hellGateDionysAlive(inst *ItemInstance) bool {
	if inst.HellGateDionysHP == 0 || len(inst.NPCIDs) != 1 {
		return false
	}
	for id := range inst.NPCIDs {
		mob := w.mobsByID[id]
		return mob != nil && !mob.Dead && mob.HP > 0 && mob.InstanceID == inst.RuntimeID
	}
	return false
}

func (w *World) hellGateProtectorKilled(inst *ItemInstance, mob *Mob) bool {
	if !hellGateVersion2(inst) || mob == nil {
		return false
	}
	if _, protected := inst.NPCIDs[mob.ID]; !protected {
		return false
	}
	delete(inst.NPCIDs, mob.ID)
	inst.HellGateDionysHP = 0
	w.failHellGate(inst, w.now())
	return true
}

func (w *World) restoreHellGateVersion2(inst *ItemInstance, saved model.InstanceRuntimeState, now time.Time) bool {
	if inst.Config.HellGate.Version == 3 && saved.State != "preparing" && saved.State != "entry" &&
		(inst.HellGateVariant < 1 || inst.HellGateVariant > 3) {
		return false
	}
	switch saved.State {
	case "preparing":
		inst.Remaining = 0
		return true
	case "entry":
		ok, _ := w.spawnItemInstanceStage(inst, 0, now, false, false)
		return ok
	case "corridors":
		return inst.HellGateVariant >= 1 && inst.HellGateVariant <= 3 && w.spawnHellGateCorridors(inst)
	case "protection":
		// A dead or legacy actor must never be silently resurrected on restart.
		if saved.HellGateDionysHP == 0 {
			return false
		}
		if saved.RewardGranted {
			inst.Remaining = 0
			return true
		}
		return w.spawnHellGateFinal(inst, now)
	case "lich_puzzle":
		// Restore completed room/Lich masks, not process-local entity IDs.
		required := 1
		if inst.Config.HellGate.Version == 3 {
			required = 2
			if inst.HellGateClearedMask > 0x0F || inst.HellGateLichSpawnedMask > 0x0F ||
				inst.HellGateClearedMask & ^inst.HellGateLichSpawnedMask != 0 {
				return false
			}
		}
		if inst.HellGateValidLichMask == 0 || inst.HellGateValidLichMask > 0x0F ||
			bits.OnesCount8(inst.HellGateValidLichMask) != required {
			return false
		}
		w.removeHellGateMobs(inst)
		for _, quadrant := range inst.Config.HellGate.Quadrants {
			q := uint8(quadrant.ID)
			if saved.HellGateClearedMask&hellGateBit(q) != 0 {
				continue
			}
			if saved.HellGateLichSpawned&hellGateBit(q) != 0 {
				inst.HellGateLichSpawnedMask &^= hellGateBit(q)
				if !w.spawnHellGateLich(inst, q) {
					w.removeHellGateMobs(inst)
					return false
				}
				continue
			}
			created, ok := w.spawnHellGateWave(inst, hellGateRoomSpawns(inst, quadrant), q, false)
			if !ok {
				w.removeHellGateMobs(inst)
				return false
			}
			for _, mob := range created {
				inst.MobIDs[mob.ID] = struct{}{}
				inst.Remaining++
			}
		}
		return true
	}
	return false
}
