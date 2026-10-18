package game

import (
	"time"

	"wydgo/internal/wire"
)

func (w *World) cityGuardCanTarget(guard, target *Mob, radius int) bool {
	if guard == nil || guard.Def == nil || !guard.Def.IsCityGuard() || guard.Dead ||
		guard.SummonerID != 0 || guard.InstanceID != "" || target == guard ||
		!w.hostileMob(target) || target.Dead || target.HP == 0 || target.Def.Score == nil ||
		target.GuildWarTower || target.InstanceID != "" || w.criticalGuildMobReserved(target) {
		return false
	}
	home := guard.Segments[0]
	return chebyshev(guard.X, guard.Y, target.X, target.Y) <= radius &&
		(home.X == 0 || home.Y == 0 || chebyshev(home.X, home.Y, target.X, target.Y) <= mobLeashRange)
}

func (w *World) tickCityGuard(guard *Mob, now time.Time, allowMovement bool) {
	if !guard.insideHomeLeash() {
		guard.TargetID = 0
		if allowMovement {
			home := guard.Segments[0]
			w.moveMobToward(guard, home.X, home.Y, 0, now)
		}
		return
	}
	target := w.mobsByID[guard.TargetID]
	if !w.cityGuardCanTarget(guard, target, mobRetentionRange) {
		guard.TargetID = 0
		target = nil
		bestDistance := mobAggroRange + 1
		for _, candidate := range w.nearbyMobs(guard.X, guard.Y, mobAggroRange) {
			if !w.cityGuardCanTarget(guard, candidate, mobAggroRange) {
				continue
			}
			distance := chebyshev(guard.X, guard.Y, candidate.X, candidate.Y)
			if distance < bestDistance || distance == bestDistance && (target == nil || candidate.ID < target.ID) {
				target, bestDistance = candidate, distance
			}
		}
		if target != nil {
			guard.TargetID = target.ID
		}
	}
	if target != nil && allowMovement && !now.Before(guard.NextMove) &&
		chebyshev(guard.X, guard.Y, target.X, target.Y) > mobAttackRange {
		w.moveMobToward(guard, target.X, target.Y, mobAttackRange, now)
	}
}

func (w *World) cityGuardAttack(guard *Mob, now time.Time) {
	target := w.mobsByID[guard.TargetID]
	if guard.Def.Score == nil || !w.cityGuardCanTarget(guard, target, mobAttackRange) ||
		!w.combatLineOfSight(guard.X, guard.Y, target.X, target.Y) {
		return
	}
	guard.NextAttack = now.Add(mobAttackInterval)
	score := guard.Def.Score
	damage := uint32(max(0, hitDamageWithRNG(effectiveMobAttackAt(guard, now)+int(score.Str)/2+
		int(score.Dex)/4+int(score.Level), int(target.Def.Score.Defense), 0, w.intn)))
	damage = min(damage, target.HP)
	target.HP -= damage
	w.sendToMobView(guard, func() []byte {
		return wire.AttackHitWide(guard.ID, target.ID, guard.X, guard.Y, target.X, target.Y,
			damage, target.Def.Score.MaxHP, 0, score.MaxMP)
	})
	if target.HP == 0 {
		guard.TargetID = 0
		// City defense is not a player kill and grants no player loot or EXP.
		w.killMobWithoutPlayer(target)
	}
}
