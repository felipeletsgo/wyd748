package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type guardKind byte

const (
	guardNone guardKind = iota
	guardAsder
	guardSophia
)

func (w *World) sendGuardTimers(owner *Player) {
	if owner != nil && owner.InWorld && owner.Char != nil && owner.Session != nil {
		owner.Session.Send(playerAffectsPacketAt(owner, w.now()))
	}
}

func (w *World) clearGuardTimer(guard *Mob) {
	if guard.guardKind == guardNone || guard.ExpiresAt.IsZero() {
		return
	}
	owner := w.playerByID(guard.SummonerID)
	if owner != nil && owner.guardTimers[guard.guardKind-1] == guard.ExpiresAt {
		owner.guardTimers[guard.guardKind-1] = time.Time{}
		w.sendGuardTimers(owner)
	}
}

func (w *World) guardCanDrawMonsters(owner *Player, guard *Mob) bool {
	if owner == nil || !owner.InWorld || owner.Char == nil ||
		w.gameplaySpaceForPlayer(owner) != mobGameplaySpace(guard) ||
		chebyshev(guard.X, guard.Y, owner.X, owner.Y) > summonLeashDistance {
		return false
	}
	switch guard.guardKind {
	case guardAsder:
		return playerCurHP(owner.Char) > 0
	case guardSophia:
		return playerCurHP(owner.Char) == 0
	}
	return false
}

// Use normal perception range and preserve other players' targets and scripted AI.
func (w *World) tauntForContractGuard(owner *Player, guard *Mob) *Mob {
	var nearest *Mob
	bestDistance := mobAggroRange + 1
	for _, mob := range w.nearbyMobs(guard.X, guard.Y, mobAggroRange) {
		if !w.hostileMob(mob) || mob.Dead || mob.HP == 0 ||
			mob.Def.IsCityGuard() || mob.GuildWarTower || mob.crystalRole != crystalNone ||
			(mob.TargetID != 0 && mob.TargetID != owner.ID && mob.TargetID != guard.ID) ||
			w.bossFor(mob.ID) != nil || isKefraBoss(mob) || w.criticalGuildMobReserved(mob) ||
			!mob.insideHomeLeash() || !w.guardMonsterSpaceAllowed(mob, owner, guard) ||
			!w.combatLineOfSight(mob.X, mob.Y, guard.X, guard.Y) {
			continue
		}
		mob.TargetID = guard.ID
		if !mob.Awake {
			mob.Awake = true
			w.activeMobs[mob.ID] = mob
		}
		distance := chebyshev(mob.X, mob.Y, guard.X, guard.Y)
		if nearest == nil || distance < bestDistance || distance == bestDistance && mob.ID < nearest.ID {
			nearest, bestDistance = mob, distance
		}
	}
	return nearest
}

func (w *World) guardMonsterSpaceAllowed(mob *Mob, owner *Player, guard *Mob) bool {
	if mobGameplaySpace(mob) != mobGameplaySpace(guard) || sameKingdomGuard(mob, owner) {
		return false
	}
	if inst := w.instanceForMob(mob); mob.InstanceID != "" {
		if inst == nil || !instanceMemberInStage(inst, owner) || !w.instanceMobInsideArea(mob) ||
			inst.RuneTrack != nil || instanceProtectionActive(inst) {
			return false
		}
	}
	return guard.guardKind == guardSophia || !hasActiveAffectAt(owner.Char, 28, w.now())
}

func (w *World) retainedContractTank(mob *Mob, distance int) *Mob {
	guard := w.summons[mob.TargetID]
	if guard == nil || guard.guardKind == guardNone || guard.Dead || guard.HP == 0 ||
		!guard.ExpiresAt.After(w.now()) || chebyshev(mob.X, mob.Y, guard.X, guard.Y) > distance {
		return nil
	}
	owner := w.playerByID(guard.SummonerID)
	if !w.guardCanDrawMonsters(owner, guard) || !w.guardMonsterSpaceAllowed(mob, owner, guard) {
		return nil
	}
	return guard
}

func (w *World) setMobRetaliationTarget(mob *Mob, attackerID uint16) {
	if tank := w.retainedContractTank(mob, mobRetentionRange); tank == nil || tank.SummonerID != attackerID {
		mob.TargetID = attackerID
	}
}

func (w *World) mobAttackContractTank(mob, tank *Mob, now time.Time) {
	damage := min(tank.HP, w.levelingInstanceDamage(mob, mobPhysicalHitMobAt(mob, tank, w.intn, now)))
	tank.HP -= damage
	w.sendToMobView(mob, func() []byte {
		return wire.AttackHitWide(mob.ID, tank.ID, mob.X, mob.Y, tank.X, tank.Y,
			damage, guardMaxHPAt(tank, now), 0, mob.Def.Score.MaxMP)
	})
	if tank.HP == 0 {
		mob.TargetID = 0
		w.destroyPlayerSummon(tank, mob.ID)
	}
}

func guardMaxHPAt(m *Mob, now time.Time) uint32 {
	hp := m.Def.Score.MaxHP
	if m.guardKind == guardAsder {
		if growth := activeMobAffectAt(m, 14, now); growth != nil {
			con := growth.Value + growth.Level + int(m.Def.Score.Level)*4/3
			hp += uint32(2 * max(0, con))
		}
	}
	return hp
}

func (w *World) tickContractGuard(owner *Player, guard *Mob, now time.Time) {
	w.rebindSummonGameplaySpace(guard, w.playerRuntimeInstanceID(owner.ID))
	alive := validPlayerMobParticipant(owner)
	if alive {
		w.followSummonOwner(owner, guard, now)
	}

	var enemy *Mob
	if w.guardCanDrawMonsters(owner, guard) {
		enemy = w.tauntForContractGuard(owner, guard)
	}
	mastery := int(min(320, guard.Def.Score.Level+1))
	if guard.guardKind == guardSophia {
		if alive {
			w.sophiaHealOwner(owner, guard, mastery, now)
		}
		return
	}
	if !alive {
		return
	}

	maxHP := guardMaxHPAt(guard, now)
	guard.HP = min(guard.HP, maxHP)
	if aura := activeMobAffectAt(guard, 17, now); aura != nil && !now.Before(aura.NextTick) {
		aura.NextTick = now.Add(8 * time.Second)
		heal := min(maxHP-guard.HP, uint32(max(0, aura.Value+aura.Level/2)))
		if heal > 0 {
			guard.HP += heal
			w.publishMobAffects(guard)
		}
	}
	if now.Before(guard.NextAttack) {
		return
	}
	for _, index := range [...]int{3, 5, 13} {
		skill, ok := w.skills[index]
		if !ok {
			continue
		}
		typ, value := skill.AffectType, skill.AffectValue
		if index == 5 {
			typ, value = skill.TickType, skill.TickValue
		}
		if typ > 0 && activeMobAffectAt(guard, byte(typ), now) == nil &&
			setMobOwnedMobAffectAt(guard, guard, byte(typ), value, mastery, skill.AffectTime, now) {
			guard.HP += guardMaxHPAt(guard, now) - maxHP
			w.castGuardSkill(guard, guard.X, guard.Y, skill, mastery, now, wire.SkillTarget{ID: guard.ID})
			w.publishMobAffects(guard)
			return
		}
	}
	if enemy == nil {
		return
	}
	skill, ok := w.skills[19]
	if !ok || chebyshev(guard.X, guard.Y, enemy.X, enemy.Y) > skill.Range {
		return
	}
	changed := false
	for _, effect := range [][2]int{{skill.AffectType, skill.AffectValue}, {skill.TickType, skill.TickValue}} {
		if effect[0] > 0 && activeMobAffectAt(enemy, byte(effect[0]), now) == nil &&
			setPlayerOwnedMobAffectAt(enemy, owner, byte(effect[0]), effect[1], mastery, skill.AffectTime, now) {
			changed = true
		}
	}
	if changed {
		w.castGuardSkill(guard, enemy.X, enemy.Y, skill, mastery, now, wire.SkillTarget{ID: enemy.ID})
		w.publishMobAffects(enemy)
	}
}

func (w *World) sophiaHealOwner(owner *Player, guard *Mob, mastery int, now time.Time) {
	skill, ok := w.skills[27]
	if !ok || now.Before(guard.NextAttack) || playerCurHP(owner.Char) >= playerMaxHP(owner.Char) ||
		chebyshev(guard.X, guard.Y, owner.X, owner.Y) > skill.Range ||
		!w.combatLineOfSight(guard.X, guard.Y, owner.X, owner.Y) {
		return
	}
	heal := uint32(max(0, foemaHealAmount(nil, skill.Index, mastery, skill.InstanceValue)))
	restorePlayerHP(owner.Char, heal)
	w.castGuardSkill(guard, owner.X, owner.Y, skill, mastery, now,
		wire.SkillTarget{ID: owner.ID, Heal: heal, MaxHP: playerMaxHP(owner.Char)})
	w.syncPlayerVitals(owner)
	w.updatePartyMember(owner)
}

func (w *World) castGuardSkill(guard *Mob, x, y uint16, skill model.SkillDef, mastery int, now time.Time, target wire.SkillTarget) {
	guard.NextAttack = now.Add(max(mobAttackInterval, skillCooldownDuration(skill)))
	w.sendToMobView(guard, func() []byte {
		return wire.SkillHits(guard.ID, guard.X, guard.Y, x, y, 0, guard.Def.Score.MaxMP,
			int16(skill.Index), 5, skillVisualLevel(mastery), skill.MaxTarget, []wire.SkillTarget{target})
	})
}
