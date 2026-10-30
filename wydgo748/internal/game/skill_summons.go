package game

import (
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type summonTemplate struct {
	name                    string
	face                    uint16
	maxSummons              int
	baseDefense, baseAttack int
	baseHP                  int
	minDamage, maxDamage    int
	minDefense, maxDefense  int
	minHP, maxHP            int
}

var summonTemplates = [...]summonTemplate{
	{"Condor", 206, 7, 15, 70, 60, 40, 150, 50, 75, 100, 400},
	{"Boar", 225, 6, 20, 70, 100, 40, 125, 50, 150, 125, 400},
	{"Wolf", 226, 6, 40, 80, 7000, 40, 200, 50, 125, 125, 400},
	{"Bear", 227, 5, 60, 100, 7000, 40, 200, 50, 200, 150, 400},
	{"Tiger", 244, 5, 30, 150, 100, 40, 250, 50, 175, 150, 400},
	{"Gorilla", 245, 4, 45, 100, 200, 40, 225, 50, 250, 175, 400},
	{"Black Dragon", 307, 3, 80, 100, 350, 50, 250, 50, 250, 174, 400},
	{"Succubus", 396, 2, 30, 250, 200, 70, 250, 60, 200, 180, 250},
}

func summonCount(instanceValue, mastery int) int {
	mastery = maxInt(0, mastery)
	switch instanceValue {
	case 1, 2:
		return mastery / 30
	case 3, 4, 5:
		return mastery / 40
	case 6, 7:
		return mastery / 80
	case 8:
		return 1
	default:
		return 0
	}
}

// removePlayerSummons despawns every summon belonging to a departing owner.
// Runtime player IDs are reused, so leaving orphaned summons would transfer
// control to the next player admitted with that ID.
func (w *World) removePlayerSummons(ownerID uint16) {
	if ownerID == 0 {
		return
	}
	var summons []*Mob
	for _, m := range w.summons {
		if m != nil && m.SummonerID == ownerID {
			summons = append(summons, m)
		}
	}
	for _, m := range summons {
		for _, viewer := range w.players {
			w.hideMob(viewer, m, 0)
		}
		m.Dead = true
		w.removeMobInstance(m)
	}
}

func (w *World) removeContractSummons(ownerID uint16) {
	var found []*Mob
	for _, m := range w.summons {
		if m != nil && m.SummonerID == ownerID && m.SummonKind == summonKindContract {
			found = append(found, m)
		}
	}
	for _, m := range found {
		for _, viewer := range w.players {
			w.hideMob(viewer, m, 0)
		}
		m.Dead = true
		w.removeMobInstance(m)
	}
}

// rebindSummonGameplaySpace closes the small transition window in which a
// player enters/leaves a private instance while an already materialized
// summon still carries the previous public/runtime identity.
func (w *World) rebindSummonGameplaySpace(summon *Mob, space string) {
	if summon == nil || summon.InstanceID == space {
		return
	}
	for _, viewer := range w.players {
		w.hideMob(viewer, summon, 0)
	}
	summon.InstanceID = space
	w.publishMobSpawn(summon)
}

type contractSummonPlan struct {
	owner    *Player
	previous []*Mob
	summon   *Mob
}

// planContractSummon validates and prepares replacement without changing World.
// Item consumption can then be persisted with entity creation already prepared.
func (w *World) planContractSummon(owner *Player, t *model.VolatileSummon) (*contractSummonPlan, bool) {
	if owner == nil || owner.Char == nil || w.criticalPlayerReserved(owner) || t == nil || t.Face == 0 || t.HP == 0 {
		return nil, false
	}
	x, y := w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, summonFollowDistance)
	if !w.terrain.Walkable(x, y) {
		return nil, false
	}
	attackRun := uint32(clampInt(int(t.MoveSpeed), 1, 15))
	def := &model.NPCDef{
		Name: t.Name + "^", Tipo: model.TipoMonstro,
		Equip: model.Equip{Rosto: model.Item{Index: t.Face}, Arma: model.Item{Index: t.Weapon}, Escudo: model.Item{Index: t.Shield}},
		Score: &model.Score{Version: model.ScoreVersion, Level: playerLevel(owner.Char),
			Attack: t.Attack, Defense: t.Defense, MaxHP: t.HP, MaxMP: 100, AttackRun: attackRun},
	}
	def.Score.CurHP, def.Score.CurMP = t.HP, 100
	mobID := w.allocMobID()
	if mobID == 0 {
		return nil, false
	}
	m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: t.HP, GenerIndex: -1,
		InstanceID: w.playerRuntimeInstanceID(owner.ID),
		SummonerID: owner.ID, SummonKind: summonKindContract, SummonRange: t.AttackRange}
	plan := &contractSummonPlan{owner: owner, summon: m}
	for _, existing := range w.summons {
		if existing != nil && existing.SummonerID == owner.ID &&
			existing.SummonKind == summonKindContract {
			plan.previous = append(plan.previous, existing)
		}
	}
	return plan, true
}

func (w *World) commitContractSummon(plan *contractSummonPlan) {
	if plan == nil || plan.owner == nil || plan.summon == nil {
		return
	}
	for _, existing := range plan.previous {
		if existing == nil || w.mobsByID[existing.ID] != existing {
			continue
		}
		for _, viewer := range w.players {
			w.hideMob(viewer, existing, 0)
		}
		existing.Dead = true
		w.removeMobInstance(existing)
	}
	m := plan.summon
	w.appendMobInstance(m)
	w.publishMobSpawn(m)
}

func (w *World) castSummon(owner *Player, skill model.SkillDef, mastery int) bool {
	idx := skill.InstanceValue - 1
	if owner == nil || owner.Char == nil || w.criticalPlayerReserved(owner) || idx < 0 || idx >= len(summonTemplates) {
		return false
	}
	// GenerateSummon checks for a free mob slot before other gameplay rules,
	// including when Num == 0.
	if w.allocMobID() == 0 {
		return false
	}
	template := summonTemplates[idx]
	space := w.playerRuntimeInstanceID(owner.ID)
	// Party players do not occupy summon slots in the Go contract.
	wanted := minInt(summonCount(skill.InstanceValue, mastery), maxPartyMembers-1)
	current := 0
	for _, m := range w.summons {
		if m == nil || m.SummonerID != owner.ID || m.Dead || m.SummonKind == summonKindContract || m.SummonKind == summonKindMount {
			continue
		}
		if m.Def == nil {
			continue
		}
		if m.Def.Equip.Rosto.Index != template.face {
			// Preserve W2PP's rejection of another family. The Go contract
			// keeps summons outside the party and owned only by the caster.
			return false
		}
		w.rebindSummonGameplaySpace(m, space)
		oldX, oldY := m.X, m.Y
		m.X, m.Y = w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, summonFollowDistance)
		w.publishMobMove(m, oldX, oldY, uint32(m.Def.Score.AttackRun&0x0f))
		current++
		if current >= wanted {
			return false
		}
	}
	// GenerateSummon succeeds when Num == 0 and no conflict was found.
	// Preserve the source MP cost even when no creature materializes.
	if wanted == 0 {
		return true
	}
	spawned := 0
	for current < wanted {
		x, y := w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, summonFollowDistance)
		base := playerInt(owner.Char) + playerCon(owner.Char)
		attack := template.baseAttack + base*template.minDamage/60 + mastery*template.maxDamage/60
		defense := template.baseDefense + base*template.minDefense/100 + mastery*template.maxDefense/100
		hp := template.baseHP + base*template.minHP/70 + mastery*template.maxHP/70
		def := &model.NPCDef{
			Name: template.name + "^", Tipo: model.TipoMonstro,
			Equip: model.Equip{Rosto: model.Item{Index: template.face}},
			Score: &model.Score{
				Version:   model.ScoreVersion,
				Level:     playerLevel(owner.Char),
				Defense:   uint32(clampInt(defense, 0, int(maxScoreValue))),
				Attack:    uint32(clampInt(attack, 0, int(maxScoreValue))),
				MaxHP:     uint32(clampInt(hp, 1, int(maxScoreValue))),
				MaxMP:     100,
				Int:       uint32(playerInt(owner.Char)),
				Con:       uint32(playerCon(owner.Char)),
				AttackRun: 0x64,
			},
		}
		def.Score.CurHP, def.Score.CurMP = def.Score.MaxHP, def.Score.MaxMP
		mobID := w.allocMobID()
		if mobID == 0 {
			log.Printf("[#%d] summon %q interrupted: mob ID range exhausted", owner.Session.ID, template.name)
			// Partial creation already produced authoritative entities. Do
			// not refund MP and allow free summons after a subsequent failure.
			return spawned > 0
		}
		m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: def.Score.MaxHP,
			InstanceID: space,
			GenerIndex: -1, LeaderID: 0, SummonerID: owner.ID, SummonKind: summonKindBM, SummonRange: mobAttackRange}
		w.appendMobInstance(m)
		w.publishMobSpawn(m)
		current++
		spawned++
	}
	log.Printf("[#%d] summoned %q count=%d/%d", owner.Session.ID, template.name, current, wanted)
	return true
}

type summonCombatTarget struct {
	id     uint16
	x, y   uint16
	mob    *Mob
	user   *Player
	paused bool
}

const summonCommandRange = 30

func (w *World) summonTarget(owner *Player, id uint16) summonCombatTarget {
	if owner == nil || owner.Char == nil || w.criticalPlayerReserved(owner) || id == 0 || id == owner.ID {
		return summonCombatTarget{}
	}
	if id >= 1000 {
		m := w.mobByID(id)
		if m == nil || m.HP == 0 || !w.playerCanInteractWithMob(owner, m) ||
			chebyshev(owner.X, owner.Y, m.X, m.Y) > summonCommandRange {
			return summonCombatTarget{}
		}
		return summonCombatTarget{id: id, x: m.X, y: m.Y, mob: m}
	}
	p := w.playerByID(id)
	if !w.canInitiatePvP(owner, p) || !validMobTargetAt(p, w.now()) || p == owner ||
		!w.playersShareGameplaySpace(owner, p) ||
		p.Party != nil && p.Party == owner.Party ||
		chebyshev(owner.X, owner.Y, p.X, p.Y) > summonCommandRange {
		return summonCombatTarget{}
	}
	return summonCombatTarget{id: id, x: p.X, y: p.Y, user: p}
}

func (w *World) ownerSummonTarget(owner *Player) summonCombatTarget {
	if owner == nil || w.criticalPlayerReserved(owner) {
		return summonCombatTarget{paused: true}
	}
	if w.criticalPlayerReserved(w.playerByID(owner.CombatTargetID)) {
		return summonCombatTarget{paused: true}
	}
	if target := w.summonTarget(owner, owner.CombatTargetID); target.id != 0 {
		if target.mob != nil && w.criticalKillParticipantsReserved(owner) {
			return summonCombatTarget{paused: true}
		}
		return target
	}
	// A held target is not an invalid command. Preflight the fallback before
	// clearing either command, so a paused tick makes no owner-state changes.
	if w.criticalPlayerReserved(w.playerByID(owner.LastAttackerID)) {
		return summonCombatTarget{paused: true}
	}
	target := w.summonTarget(owner, owner.LastAttackerID)
	if target.mob != nil && w.criticalKillParticipantsReserved(owner) {
		return summonCombatTarget{paused: true}
	}
	owner.CombatTargetID = 0
	if target.id != 0 {
		return target
	}
	owner.LastAttackerID = 0
	return summonCombatTarget{}
}

// Idle summons stay close to their owner: they walk back beyond
// summonFollowDistance and, beyond summonLeashDistance (a fast mount, a
// blocked path), they are placed next to the owner instead of wandering
// through the city. Combat is not restricted.
const (
	summonFollowDistance = 2
	summonLeashDistance  = 8
)

// relocateOwnerSummons places the owner's summons next to the owner after a
// teleport. Walking back made them wander lost through the city.
func (w *World) relocateOwnerSummons(owner *Player) {
	if owner == nil || !owner.InWorld {
		return
	}
	space := w.playerRuntimeInstanceID(owner.ID)
	for _, summon := range w.summons {
		if summon == nil || summon.Dead || summon.SummonerID != owner.ID ||
			summon.SummonKind == summonKindThornWall {
			continue
		}
		w.relocateSummon(owner, summon, space)
	}
}

func (w *World) relocateSummon(owner *Player, summon *Mob, space string) {
	w.rebindSummonGameplaySpace(summon, space)
	oldX, oldY := summon.X, summon.Y
	summon.X, summon.Y = w.findFreeGameplayPosition(owner, nil, owner.X, owner.Y, summonFollowDistance)
	summon.TargetID = 0
	speed := uint32(0)
	if summon.Def != nil {
		speed = uint32(summon.Def.Score.AttackRun & 0x0f)
	}
	w.publishMobMove(summon, oldX, oldY, speed)
}

// followSummonOwner keeps an idle summon close to its owner. It does not
// apply in combat: a fighting summon chases its target freely.
func (w *World) followSummonOwner(owner *Player, summon *Mob, now time.Time) {
	distance := chebyshev(summon.X, summon.Y, owner.X, owner.Y)
	if distance > summonLeashDistance {
		w.relocateSummon(owner, summon, w.playerRuntimeInstanceID(owner.ID))
		return
	}
	if distance > summonFollowDistance {
		w.moveSummonToward(summon, owner.X, owner.Y, summonFollowDistance, now)
	}
}

func (w *World) moveSummonToward(summon *Mob, x, y uint16, stopDistance int, now time.Time) {
	if now.Before(summon.NextMove) {
		return
	}
	w.moveMobToward(summon, x, y, stopDistance, now)
}

func (w *World) tickSummonCombat(now time.Time) {
	for _, summon := range w.summons {
		if summon == nil || summon.Dead || summon.HP == 0 || summon.SummonerID == 0 {
			continue
		}
		owner := w.playerByID(summon.SummonerID)
		if owner == nil || owner.Char == nil || !owner.InWorld || w.criticalPlayerReserved(owner) {
			continue
		}
		passive := summon.SummonKind == summonKindThornWall || summon.SummonKind == summonKindMount ||
			summon.Def != nil && isCriaPetFace(summon.Def.Equip.Rosto.Index)
		var target summonCombatTarget
		if !passive {
			target = w.ownerSummonTarget(owner)
			if target.paused {
				continue
			}
		}
		w.rebindSummonGameplaySpace(summon, w.playerRuntimeInstanceID(owner.ID))
		if summon.SummonKind == summonKindThornWall {
			continue
		}
		// Baby mount pets are passive: follow the owner and never attack.
		if summon.SummonKind == summonKindMount || summon.Def != nil && isCriaPetFace(summon.Def.Equip.Rosto.Index) {
			w.followSummonOwner(owner, summon, now)
			continue
		}
		if target.id == 0 {
			summon.TargetID = 0
			// Without a combat command, follow the BM without initiating aggro.
			w.followSummonOwner(owner, summon, now)
			continue
		}
		summon.TargetID = target.id
		attackRange := summon.SummonRange
		if attackRange <= 0 {
			attackRange = mobAttackRange
		}
		if chebyshev(summon.X, summon.Y, target.x, target.y) > attackRange {
			w.moveSummonToward(summon, target.x, target.y, attackRange, now)
			continue
		}
		if !w.combatLineOfSight(summon.X, summon.Y, target.x, target.y) {
			w.moveSummonToward(summon, target.x, target.y, 1, now)
			continue
		}
		if now.Before(summon.NextMove) {
			continue
		}
		if now.Before(summon.NextAttack) {
			continue
		}
		summon.NextAttack = now.Add(mobAttackInterval)
		if target.mob != nil {
			chance := combatAccuracyPercent(int(target.mob.Def.Score.Dex),
				int(summon.Def.Score.Accuracy), int(target.mob.Def.Score.Evasion), false)
			if !combatRollHits(chance, w.intn) {
				w.sendToMobView(summon, func() []byte {
					return wire.AttackHitWideResult(summon.ID, target.mob.ID, summon.X, summon.Y,
						target.mob.X, target.mob.Y, 0, target.mob.Def.Score.MaxHP, 0,
						summon.Def.Score.MaxMP, 0, true)
				})
				continue
			}
			damage := mobPhysicalHitMobAt(summon, target.mob, w.intn, now)
			if isCombatSummon(target.mob) {
				damage = summonPvPDamage(damage)
			}
			oldHP := target.mob.HP
			if damage >= target.mob.HP {
				target.mob.HP = 0
			} else {
				target.mob.HP -= damage
			}
			w.sendToMobView(summon, func() []byte {
				return wire.AttackHitWide(summon.ID, target.mob.ID, summon.X, summon.Y, target.mob.X, target.mob.Y,
					damage, target.mob.Def.Score.MaxHP, 0, summon.Def.Score.MaxMP)
			})
			if target.mob.HP == 0 {
				w.killMobState(owner, target.mob, damage, minU32(damage, oldHP))
			}
			continue
		}
		// W2PP GetFunc: no summon PvP damage unless both cells allow PvP.
		if !w.terrain.PvPZone(summon.X, summon.Y) || !w.terrain.PvPZone(target.user.X, target.user.Y) {
			continue
		}
		hit := mobPhysicalHitPlayerAt(summon, target.user.Char, w.intn, now)
		if !hit.Hit {
			w.sendToPlayerView(target.user, func() []byte {
				return wire.AttackHitWideResult(summon.ID, target.user.ID, summon.X, summon.Y,
					target.user.X, target.user.Y, 0, playerMaxHP(target.user.Char), 0,
					summon.Def.Score.MaxMP, 0, true)
			})
			continue
		}
		damage := summonPvPDamage(hit.Damage)
		damage = uint32(w.absorbMountDamage(target.user, int(damage)))
		damage = absorbManaControlDamageAt(target.user.Char, damage, now)
		w.cancelTrade(target.user, "character was attacked")
		currentHP := playerCurHP(target.user.Char)
		if damage >= currentHP {
			w.receiveDeathLetter(target.user, owner.Char.Name, "player")
			w.mountRiderDied(target.user)
			setPlayerCurHP(target.user.Char, 0)
			target.user.DeadAt = now
			w.applyPvPKills(owner, target.user)
		} else {
			setPlayerCurHP(target.user.Char, currentHP-damage)
		}
		target.user.LastAttackerID = owner.ID
		w.sendToPlayerView(target.user, func() []byte {
			return wire.AttackHitWide(summon.ID, target.user.ID, summon.X, summon.Y, target.user.X, target.user.Y,
				damage, playerMaxHP(target.user.Char), 0, summon.Def.Score.MaxMP)
		})
		w.syncPlayerVitals(target.user)
		w.updatePartyMember(target.user)
		if playerCurHP(target.user.Char) == 0 {
			w.publishPlayerDeath(target.user, summon.ID)
		}
	}
}

// summonPvPDamage applies the native AI attack factor between players and
// summons: 30% (TMSRVIN sub_4DECC0, W2PP GetAttack, Clan 4).
func summonPvPDamage(damage uint32) uint32 {
	return damage * 3 / 10
}
