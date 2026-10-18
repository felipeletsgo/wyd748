package game

import (
	"errors"
	"log"
	"math/bits"
	"strings"
	"time"

	"wydgo/internal/wire"
)

// W2PP ProcessSecMinTimer starts a 900-second combat period when the
// four-minute admission window closes, not when the first ticket is used.
const nightmareCombatDuration = 15 * time.Minute

type nightmareSpawnPoint struct {
	X, Y   uint16
	Radius uint16
}

func scheduledNightmare(inst *ItemInstance) bool {
	if inst == nil || !sharedTimedInstance(inst.Config) || len(inst.Config.Schedule) == 0 {
		return false
	}
	switch strings.ToLower(strings.TrimSpace(inst.Config.NightmareTier)) {
	case "normal", "mystic", "arcane":
		return true
	}
	return false
}

func nightmareWaiting(inst *ItemInstance) bool {
	return scheduledNightmare(inst) && inst.State == "entry"
}

func prepareNightmareEntry(inst *ItemInstance) {
	if !scheduledNightmare(inst) || inst.ScheduleEnd.IsZero() {
		return
	}
	inst.HardDeadline = inst.ScheduleEnd.Add(nightmareCombatDuration)
	inst.Remaining = 0
	setInstanceCombatDeadline(inst, inst.ScheduleEnd)
}

func instanceDisplayedMobCount(inst *ItemInstance) uint32 {
	if strings.TrimSpace(inst.Config.NightmareTier) != "" {
		// 0x3B0 counts protected survivors, never a quota of hostile kills.
		return uint32(len(inst.Config.NightmareProtectors) - nightmareLostCount(inst))
	}
	return uint32(inst.Remaining)
}

func nightmareLostCount(inst *ItemInstance) int {
	count := min(len(inst.Config.NightmareProtectors), 8)
	return bits.OnesCount8(inst.NightmareLostMask & uint8((uint16(1)<<count)-1))
}

// The 10% loss per protected NPC is an explicit project choice, approved
// against the secondary 7.59 formula; the official guide specifies no rate.
// Scale once before party sharing, without overflowing the original EXP.
func (w *World) nightmareExperience(m *Mob, original uint32) uint32 {
	inst := w.instanceForMob(m)
	if !scheduledNightmare(inst) {
		return original
	}
	percent := uint32(100 - 10*nightmareLostCount(inst))
	return original/100*percent + original%100*percent/100
}

// Prepare friendly, mortal NPCs without publishing them or changing World.
// Critical entry reserves these exact IDs/tiles until the ticket commits.
func (w *World) planNightmareProtectors(inst *ItemInstance, tiles map[criticalTeleportTile]*Player, owner *Player) ([]*Mob, error) {
	if !scheduledNightmare(inst) {
		return nil, nil
	}
	if len(inst.Config.NightmareProtectors) > 8 {
		return nil, errors.New("Nightmare supports at most eight protected NPCs.")
	}
	if boss := inst.Config.NightmareBoss; boss != nil {
		def := w.npcDefByName(boss.Spawn.NPC)
		if !strings.EqualFold(inst.Config.NightmareTier, "arcane") ||
			boss.SecondsRemaining <= 0 || boss.SecondsRemaining >= int(nightmareCombatDuration/time.Second) ||
			boss.Spawn.Count != 1 || boss.Spawn.X == 0 || boss.Spawn.Y == 0 || boss.Spawn.Range < 0 ||
			def == nil || !def.IsMonster() || def.Score == nil || def.Score.MaxHP == 0 {
			return nil, errors.New("The Nightmare boss is not configured.")
		}
		for _, spawn := range inst.Config.Spawns {
			if strings.EqualFold(spawn.NPC, boss.Spawn.NPC) {
				return nil, errors.New("The Nightmare boss cannot be an ordinary respawn.")
			}
		}
	}
	space := itemInstanceGameplaySpace(inst)
	created := make([]*Mob, 0, len(inst.Config.NightmareProtectors))
	protectors := make(map[uint16]uint8)
	for index, spawn := range inst.Config.NightmareProtectors {
		if inst.NightmareLostMask&(1<<index) != 0 {
			continue
		}
		def := w.npcDefByName(spawn.NPC)
		if def == nil || def.IsMonster() || def.Score == nil || def.Score.MaxHP == 0 || spawn.Count != 1 {
			return nil, errors.New("A Nightmare protected NPC is not configured.")
		}
		x, y, found := spawn.X, spawn.Y, false
		for radius := 0; radius <= 4 && !found; radius++ {
			for dy := -radius; dy <= radius && !found; dy++ {
				for dx := -radius; dx <= radius; dx++ {
					nx, ny := int(spawn.X)+dx, int(spawn.Y)+dy
					if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
						continue
					}
					x, y = uint16(nx), uint16(ny)
					if _, reserved := tiles[criticalTile(x, y, space)]; reserved ||
						w.positionOccupiedInGameplaySpace(x, y, space, nil, nil, nil) {
						continue
					}
					found = true
					break
				}
			}
		}
		if !found {
			return nil, errors.New("A Nightmare protected NPC position is blocked.")
		}
		id := w.allocMobID()
		if id == 0 {
			return nil, errors.New("No Nightmare protected NPC ID is available.")
		}
		if _, used := protectors[id]; used {
			return nil, errors.New("A Nightmare protected NPC ID was allocated twice.")
		}
		created = append(created, &Mob{ID: id, Def: def, X: x, Y: y,
			HP: def.Score.MaxHP, GenerIndex: -1, InstanceID: space})
		protectors[id] = uint8(index)
		tiles[criticalTile(x, y, space)] = owner
	}
	if inst.NPCIDs == nil {
		inst.NPCIDs = make(map[uint16]struct{})
	}
	inst.NightmareProtectors = protectors
	for id := range protectors {
		inst.NPCIDs[id] = struct{}{}
	}
	return created, nil
}

func (w *World) spawnNightmareProtectors(inst *ItemInstance) bool {
	created, err := w.planNightmareProtectors(inst, make(map[criticalTeleportTile]*Player), nil)
	if err != nil {
		log.Printf("INSTANCE %q: %v", inst.Config.ID, err)
		return false
	}
	for _, mob := range created {
		w.appendMobInstance(mob)
		w.mobsByID[mob.ID] = mob
		w.registerMobSpatial(mob)
	}
	return true
}

func (w *World) nightmareProtectorKilled(inst *ItemInstance, m *Mob) bool {
	index, protected := inst.NightmareProtectors[m.ID]
	if !protected {
		return false
	}
	inst.NightmareLostMask |= 1 << index
	delete(inst.NightmareProtectors, m.ID)
	delete(inst.NPCIDs, m.ID)
	w.markInstanceStateDirty()
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld && member.Session != nil {
			member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, instanceDisplayedMobCount(inst)))
		}
	}
	return true
}

// Protected targets participate only in their own event's protection phase.
// They do not become globally attackable NPCs or targets for other instances.
func (w *World) nightmareProtectorTarget(m *Mob, maxDistance int) *Mob {
	inst := w.instanceForMob(m)
	if !instanceProtectionActive(inst) ||
		!w.now().Before(inst.HardDeadline) || m == nil || m.Dead || m.Def == nil || !m.Def.IsMonster() {
		return nil
	}
	var best *Mob
	bestDistance := maxDistance + 1
	for id := range inst.NPCIDs {
		target := w.mobsByID[id]
		if target == nil || target.Dead || target.HP == 0 || target.InstanceID != m.InstanceID {
			continue
		}
		distance := chebyshev(m.X, m.Y, target.X, target.Y)
		if distance <= maxDistance && (distance < bestDistance || distance == bestDistance && (best == nil || id < best.ID)) {
			best, bestDistance = target, distance
		}
	}
	return best
}

func (w *World) retainedNightmareProtector(m *Mob, maxDistance int) *Mob {
	inst := w.instanceForMob(m)
	if !instanceProtectionActive(inst) || !w.now().Before(inst.HardDeadline) {
		return nil
	}
	if _, exists := inst.NPCIDs[m.TargetID]; !exists {
		return nil
	}
	target := w.mobsByID[m.TargetID]
	if target == nil || target.Dead || target.HP == 0 || target.InstanceID != m.InstanceID ||
		chebyshev(m.X, m.Y, target.X, target.Y) > maxDistance {
		return nil
	}
	return target
}

func (w *World) mobAttackNightmareProtector(m, target *Mob, now time.Time) {
	score := m.Def.Score
	damage := uint32(max(0, hitDamageWithRNG(effectiveMobAttackAt(m, now)+int(score.Str)/2+
		int(score.Dex)/4+int(score.Level), int(target.Def.Score.Defense), 0, w.intn)))
	damage = w.levelingInstanceDamage(m, damage)
	damage = min(damage, target.HP)
	target.HP -= damage
	if inst := w.instanceForMob(target); hellGateVersion2(inst) {
		inst.HellGateDionysHP = target.HP
		w.markInstanceStateDirty()
	}
	w.sendToMobView(m, func() []byte {
		return wire.AttackHitWide(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
			damage, target.Def.Score.MaxHP, 0, m.Def.Score.MaxMP)
	})
	if target.HP == 0 {
		m.TargetID = 0
		w.killMobWithoutPlayer(target)
	}
}

// W2PP MobKilled regenerates the killed Nightmare generator immediately.
// Replace one monster after its committed death, without resetting the clock,
// clearing survivors, requiring a wave clear or granting a completion reward.
func (w *World) respawnNightmareMob(inst *ItemInstance, dead *Mob, now time.Time) {
	if dead.ID == inst.NightmareBossID && inst.NightmareBossSpawned {
		delete(inst.MobIDs, dead.ID)
		inst.NightmareBossID = 0
		inst.NightmareBossKilled = true
		inst.Remaining = len(inst.MobIDs)
		w.markInstanceStateDirty()
		return
	}
	spawn, configured := inst.NightmareSpawns[dead.ID]
	delete(inst.MobIDs, dead.ID)
	delete(inst.NightmareSpawns, dead.ID)
	inst.Remaining = len(inst.MobIDs)
	fail := func() {
		log.Printf("INSTANCE %q: Nightmare monster replacement failed", inst.Config.ID)
		inst.HardDeadline = now
		setInstanceCombatDeadline(inst, now)
	}
	if !configured || dead.Def == nil || dead.Def.Score == nil {
		fail()
		return
	}
	space := itemInstanceGameplaySpace(inst)
	x, y := w.findFreeMobPosition(space, spawn.X, spawn.Y, spawn.Radius)
	// The allocator's legacy fallback must never create an overlapping mob.
	if w.positionOccupiedInGameplaySpace(x, y, space, nil, nil, nil) {
		fail()
		return
	}
	id := w.allocMobID()
	if id == 0 {
		fail()
		return
	}
	mob := &Mob{ID: id, Def: dead.Def, X: x, Y: y,
		HP: dead.Def.Score.MaxHP, GenerIndex: -1, InstanceID: space}
	w.appendMobInstance(mob)
	w.mobsByID[id] = mob
	inst.MobIDs[id] = struct{}{}
	inst.NightmareSpawns[id] = spawn
	inst.Remaining = len(inst.MobIDs)
	w.publishMobSpawn(mob)
}

// The owner chose five minutes remaining. Interpret the guide's survivor
// condition as at least one living protector, not a hostile-kill quota.
// Restore an already spawned, undefeated boss even if protectors later died.
func (w *World) spawnNightmareBoss(inst *ItemInstance, now time.Time, publish bool) bool {
	if !scheduledNightmare(inst) || inst.State != "combat" || inst.Config.NightmareBoss == nil ||
		inst.NightmareBossKilled || inst.NightmareBossID != 0 || !now.Before(inst.HardDeadline) {
		return true
	}
	boss := inst.Config.NightmareBoss
	if now.Before(inst.HardDeadline.Add(-time.Duration(boss.SecondsRemaining)*time.Second)) ||
		!inst.NightmareBossSpawned && instanceDisplayedMobCount(inst) == 0 {
		return true
	}
	def := w.npcDefByName(boss.Spawn.NPC)
	if def == nil || !def.IsMonster() || def.Score == nil || def.Score.MaxHP == 0 {
		return false
	}
	space := itemInstanceGameplaySpace(inst)
	x, y := w.findFreeMobPosition(space, boss.Spawn.X, boss.Spawn.Y,
		uint16(min(max(boss.Spawn.Range, 4), 65535)))
	if w.positionOccupiedInGameplaySpace(x, y, space, nil, nil, nil) {
		return false
	}
	id := w.allocMobID()
	if id == 0 {
		return false
	}
	mob := &Mob{ID: id, Def: def, X: x, Y: y, HP: def.Score.MaxHP,
		GenerIndex: -1, InstanceID: space}
	w.appendMobInstance(mob)
	w.mobsByID[id] = mob
	inst.MobIDs[id] = struct{}{}
	inst.NightmareBossID = id
	inst.NightmareBossSpawned = true
	inst.Remaining = len(inst.MobIDs)
	if publish {
		w.publishMobSpawn(mob)
	} else {
		w.registerMobSpatial(mob)
	}
	w.markInstanceStateDirty()
	return true
}
