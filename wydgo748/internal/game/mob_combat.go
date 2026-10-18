package game

import (
	"log"
	"strings"
	"time"

	"wydgo/internal/wire"
)

// TMSrv uses GetEnemyFromView while a mob is in MOB_PEACE. Normal acquisition
// uses a 9x9 window (Chebyshev radius 4); activation at radius 16 only wakes
// the AI and grants neither aggro nor additional attack range.
const (
	mobAggroRange      = 4
	mobAttackRange     = 1
	mobRetentionRange  = 7
	mobLeashRange      = 16
	mobActivationRange = 16
	mobAttackInterval  = 1500 * time.Millisecond
)

// tickMobCombat follows StandingByProcessor -> SetBattle -> BattleProcessor.
// activeMobs contains proximity-awakened monsters; acquisition and pursuit run
// every second. World owns target, HP and position together on one goroutine.
func (w *World) tickMobCombat(now time.Time, shard, shardCount int, allowMovement bool) {
	for _, m := range w.activeMobs {
		if w.criticalGuildMobReserved(m) {
			continue
		}
		if shardCount > 1 && int(m.ID)%shardCount != shard {
			continue
		}
		if m == nil || m.Dead || m.Def == nil || m.SummonerID != 0 || m.GuildWarTower {
			continue
		}
		if m.Def.IsCityGuard() {
			w.tickCityGuard(m, now, allowMovement)
			continue
		}
		if !m.Def.IsMonster() {
			continue
		}
		if guardian := w.hellGateMarchTarget(m); guardian != nil {
			m.TargetID = guardian.ID
			if allowMovement && !now.Before(m.NextMove) {
				w.marchHellGateAttacker(m, guardian, now)
			}
			continue
		}
		// Movement, death and disconnect update the grid by event. Also retire
		// mobs whose last nearby player died without moving.
		nearbyTarget := false
		if m.InstanceID != "" {
			nearbyTarget = w.instanceMobHasNearbyMember(m, mobActivationRange) ||
				w.nightmareProtectorTarget(m, mobActivationRange) != nil
		} else {
			nearbyTarget = w.hasNearbyPlayerInGameplaySpace(m.X, m.Y, mobActivationRange, "")
		}
		if !nearbyTarget {
			m.Awake = false
			m.TargetID = 0
			delete(w.activeMobs, m.ID)
			continue
		}
		if m.InstanceID != "" && !w.instanceMobInsideArea(m) {
			// Stale references or failed movement must not release an instance mob
			// into the public map. Return to the room center before retargeting.
			m.TargetID = 0
			if allowMovement {
				x, y, _, ok := instanceMobArea(w.instanceForMob(m), m)
				if ok {
					w.moveMobToward(m, x, y, 0, now)
				}
			}
			continue
		}
		if !m.insideHomeLeash() {
			m.TargetID = 0
			home := m.Segments[0]
			if allowMovement && home.X != 0 && home.Y != 0 {
				w.moveMobToward(m, home.X, home.Y, 0, now)
			}
			continue
		}
		if guardian := w.retainedNightmareProtector(m, mobRetentionRange); guardian != nil {
			if allowMovement && !now.Before(m.NextMove) && chebyshev(m.X, m.Y, guardian.X, guardian.Y) > mobAttackRange {
				w.moveMobToward(m, guardian.X, guardian.Y, mobAttackRange, now)
			}
			continue
		}
		var target *Player
		if m.InstanceID != "" {
			target = w.playerByID(m.TargetID)
		} else {
			target = w.playerByID(m.TargetID)
		}
		// Keep the retained target while its write is unsettled; do not turn
		// a persistence pause into target loss or a chase/deadline mutation.
		if w.criticalPlayerReserved(target) {
			continue
		}
		allowedTarget := w.mobCanTargetPlayer(m, target)
		if !allowedTarget || chebyshev(m.X, m.Y, target.X, target.Y) > mobRetentionRange {
			m.TargetID = 0
			if m.InstanceID != "" {
				target = w.nearestInstanceMember(m)
			} else {
				target = w.nearestPlayerMatching(m.X, m.Y, mobAggroRange, "",
					func(p *Player) bool { return w.mobCanTargetPlayer(m, p) && !sameKingdomGuard(m, p) })
			}
			guardian := w.nightmareProtectorTarget(m, mobAggroRange)
			if guardian != nil && (target == nil || chebyshev(m.X, m.Y, guardian.X, guardian.Y) < chebyshev(m.X, m.Y, target.X, target.Y)) {
				m.TargetID = guardian.ID
				if allowMovement && !now.Before(m.NextMove) && chebyshev(m.X, m.Y, guardian.X, guardian.Y) > mobAttackRange {
					w.moveMobToward(m, guardian.X, guardian.Y, mobAttackRange, now)
				}
				continue
			}
			if target == nil {
				continue
			}
			if w.criticalPlayerReserved(target) {
				continue
			}
			m.TargetID = target.ID
		}

		distance := chebyshev(m.X, m.Y, target.X, target.Y)
		if allowMovement && distance > mobAttackRange {
			w.chasePlayer(m, target, now)
		}
	}
}

// tickActiveMobActions executes attack deadlines and advances the cached Hell
// Gate objective route at movement speed. Ordinary perception runs every second;
// the configured 1.5-second attack interval remains authoritative for DPS.
func (w *World) tickActiveMobActions(now time.Time) {
	for _, m := range w.activeMobs {
		if w.criticalGuildMobReserved(m) {
			continue
		}
		if m == nil || m.Dead || m.Def == nil || (!m.Def.IsMonster() && !m.Def.IsCityGuard()) || m.SummonerID != 0 {
			if m != nil {
				m.Awake = false
				m.TargetID = 0
				delete(w.activeMobs, m.ID)
			}
			continue
		}
		if m.GuildWarTower {
			continue
		}
		if now.Before(m.NextMove) || now.Before(m.NextAttack) {
			continue
		}
		if guardian := w.hellGateMarchTarget(m); guardian != nil &&
			chebyshev(m.X, m.Y, guardian.X, guardian.Y) > mobAttackRange {
			m.TargetID = guardian.ID
			w.marchHellGateAttacker(m, guardian, now)
			continue
		}
		if m.Def.IsCityGuard() {
			w.cityGuardAttack(m, now)
			continue
		}
		if guardian := w.retainedNightmareProtector(m, mobAttackRange); guardian != nil {
			// A protected death changes the retained instance aggregate. Do not
			// damage it behind an unsettled entry/departure or snapshot write.
			if w.criticalInstanceReserved() {
				continue
			}
			if w.combatLineOfSight(m.X, m.Y, guardian.X, guardian.Y) {
				m.NextAttack = now.Add(mobAttackInterval)
				w.mobAttackNightmareProtector(m, guardian, now)
			}
			continue
		}
		target := w.playerByID(m.TargetID)
		allowedTarget := w.mobCanTargetPlayer(m, target)
		if !allowedTarget || chebyshev(m.X, m.Y, target.X, target.Y) > mobAttackRange {
			continue
		}
		// Final attack boundary is self-contained: pathfinding/aggro may have
		// accepted the target earlier, but a mob never hits through a wall or an
		// incompatible terrain edge at the actual damage deadline.
		if !w.combatLineOfSight(m.X, m.Y, target.X, target.Y) {
			continue
		}
		m.NextAttack = now.Add(mobAttackInterval)
		w.mobAttackPlayer(m, target, now)
	}
}

func (m *Mob) insideHomeLeash() bool {
	if m == nil {
		return false
	}
	home := m.Segments[0]
	if home.X == 0 || home.Y == 0 {
		return true
	}
	return chebyshev(home.X, home.Y, m.X, m.Y) <= mobLeashRange
}

func validMobTargetAt(p *Player, now time.Time) bool {
	return p != nil && p.InWorld && p.Char != nil && playerCurHP(p.Char) > 0 &&
		!hasActiveAffectAt(p.Char, 28, now)
}

func (m *Mob) insideLeash(p *Player) bool {
	if p == nil {
		return false
	}
	home := m.Segments[0]
	if home.X == 0 || home.Y == 0 {
		return true
	}
	return chebyshev(home.X, home.Y, p.X, p.Y) <= mobLeashRange
}

func (w *World) playerByID(id uint16) *Player {
	if id == 0 {
		return nil
	}
	return w.playersByID[id]
}

func (w *World) nearestLivingPlayer(x, y uint16, maxDistance int) *Player {
	return w.nearestLivingPlayerInGameplaySpace(x, y, maxDistance, "")
}

// nearestLivingPlayerInGameplaySpace scans the spatial cells directly instead
// of materializing a temporary []Player. Selection semantics stay identical:
// valid target, exact gameplay space, Chebyshev radius, then deterministic
// distance/Player.ID ordering.
func (w *World) nearestLivingPlayerInGameplaySpace(x, y uint16, maxDistance int, space string) *Player {
	return w.nearestPlayerMatching(x, y, maxDistance, space, nil)
}

// nearestPlayerMatching is nearestLivingPlayerInGameplaySpace restricted to
// players accepted by accept (nil accepts all), so a mob skips players it may
// not target, such as members of its own kingdom, instead of locking onto them.
func (w *World) nearestPlayerMatching(x, y uint16, maxDistance int, space string, accept func(*Player) bool) *Player {
	if maxDistance < 0 {
		return nil
	}
	space = strings.TrimSpace(space)
	cx, cy := int(x)/spatialCellSize, int(y)/spatialCellSize
	cr := maxDistance/spatialCellSize + 1
	now := w.now()
	var best *Player
	bestDistance := maxDistance + 1
	for yy := cy - cr; yy <= cy+cr; yy++ {
		for xx := cx - cr; xx <= cx+cr; xx++ {
			if xx < 0 || yy < 0 {
				continue
			}
			for _, p := range w.playerCells[uint32(xx)<<16|uint32(yy)] {
				if !validMobTargetAt(p, now) || w.gameplaySpaceForPlayer(p) != space || accept != nil && !accept(p) {
					continue
				}
				distance := chebyshev(x, y, p.X, p.Y)
				if distance <= maxDistance && (distance < bestDistance ||
					distance == bestDistance && (best == nil || p.ID < best.ID)) {
					best, bestDistance = p, distance
				}
			}
		}
	}
	return best
}

func (w *World) chasePlayer(m *Mob, target *Player, now time.Time) {
	if now.Before(m.NextMove) {
		return
	}
	w.moveMobToward(m, target.X, target.Y, mobAttackRange, now)
}

func (w *World) mobAttackPlayer(m *Mob, target *Player, now time.Time) {
	if m == nil || m.Def == nil || !m.Def.IsMonster() || target == nil || target.Char == nil || w.criticalPlayerReserved(target) {
		return
	}
	hit := mobPhysicalHitPlayerAt(m, target.Char, w.intn, now)
	if !hit.Hit {
		w.sendToMobView(m, func() []byte {
			return wire.AttackHitWideResult(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
				0, playerMaxHP(target.Char), 0, m.Def.Score.MaxMP, 0, true)
		})
		return
	}
	damage := hit.Damage
	// FlagLocal=0 applies damage and displays the floating number in 7.48.
	// Zero damage retains the attack animation and represents MISS.
	w.applyMobDamageToPlayer(m, target, damage, now, func(applied uint32) []byte {
		return wire.AttackHitWide(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
			applied, playerMaxHP(target.Char), 0, m.Def.Score.MaxMP)
	})
}

// applyMobDamageToPlayer applies precomputed damage, mount absorption,
// animation, death and synchronization. Ordinary hits and boss spells share
// this boundary, differing only in damage calculation and packet construction.
func (w *World) applyMobDamageToPlayer(m *Mob, target *Player, damage uint32,
	now time.Time, build func(applied uint32) []byte) {
	if m == nil || m.Def == nil || !m.Def.IsMonster() || target == nil || target.Char == nil || w.criticalPlayerReserved(target) {
		return
	}
	target.LastAttackerID = m.ID
	damage = w.levelingInstanceDamage(m, damage)
	damage = absorbFlatDamage(damage, w.playerFlatDamageAbsorb(target.Char))
	// A living adult mount absorbs 25% of damage into its own HP.
	damage = uint32(w.absorbMountDamage(target, int(damage)))
	damage = absorbManaControlDamageAt(target.Char, damage, now)
	currentHP := playerCurHP(target.Char)
	lethal := damage >= currentHP
	if lethal {
		w.cancelTrade(target, "character died")
		w.receiveDeathLetter(target, m.Def.Name, "mob")
		w.mountRiderDied(target)
		setPlayerCurHP(target.Char, 0)
		target.DeadAt = now
		m.TargetID = 0
	} else {
		setPlayerCurHP(target.Char, currentHP-damage)
	}

	w.sendToMobView(m, func() []byte { return build(damage) })
	// AttackHit animates/applies damage; the following SetHpMp confirms
	// authoritative HP to the target and all observers.
	w.syncPlayerVitals(target)
	w.updatePartyMember(target)
	if lethal {
		// SetHpMp only updates the bar. The 7.48 CNFMobKill handler zeros human
		// HP and calls TMHuman::Die(), showing the death pose/screen without
		// removing the character from the scene. Preserve MobKilled ordering.
		w.publishPlayerDeath(target, m.ID)
		log.Printf("[#%d] died to mob id=%d %q (dmg=%d)", target.Session.ID, m.ID, m.Def.Name, damage)
	}
}
