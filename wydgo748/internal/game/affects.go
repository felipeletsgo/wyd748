package game

import (
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func canReplaceAffect(current model.Affect, value, level int, expires time.Time, now time.Time) bool {
	if current.Type == 0 || !current.ExpiresAt.After(now) {
		return true
	}
	// Native SetAffect preserves both potency and remaining duration. A weaker
	// or shorter recast cannot reduce an existing buff/debuff.
	if level < current.Level || level == current.Level && value < current.Value {
		return false
	}
	return !expires.Before(current.ExpiresAt)
}

func setAffect(ch *model.Char, affectType byte, value, level, durationUnits int) bool {
	return setAffectAt(ch, affectType, value, level, durationUnits, time.Now())
}

func setAffectAt(ch *model.Char, affectType byte, value, level, durationUnits int, now time.Time) bool {
	if ch == nil || affectType == 0 {
		return false
	}
	if durationUnits < 1 {
		durationUnits = 1
	}
	expires := now.Add(time.Duration(durationUnits*8) * time.Second)
	slot := -1
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectType {
			// Native SetAffect overwrites a BM form in either direction; its
			// Value is a form ID, so the potency check would lock Eden (5) in.
			if affectType != affectBMTransform && !canReplaceAffect(ch.Affects[i], value, level, expires, now) {
				return false
			}
			slot = i
			break
		}
		if slot < 0 && (ch.Affects[i].Type == 0 || !ch.Affects[i].ExpiresAt.After(now)) {
			slot = i
		}
	}
	if slot < 0 {
		return false
	}
	ch.Affects[slot] = model.Affect{Type: affectType, ClientType: affectType, Value: value, Level: level,
		ExpiresAt: expires, NextTick: now.Add(8 * time.Second)}
	return true
}

// setFaceAffectAt directly replaces the face transformation (volatiles 70-77).
// It does not use canReplaceAffect, which compares potency: here Value is a
// mesh, and switching between Gremlin(202) and Troll(212) is valid in either
// direction. Value carries the mesh; bodyMesh applies it.
func setFaceAffectAt(ch *model.Char, mesh, durationUnits int, now time.Time) bool {
	if ch == nil || mesh <= 0 {
		return false
	}
	if durationUnits < 1 {
		durationUnits = 1
	}
	expires := now.Add(time.Duration(durationUnits*8) * time.Second)
	slot := -1
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectFaceTransform {
			slot = i
			break
		}
		if slot < 0 && (ch.Affects[i].Type == 0 || !ch.Affects[i].ExpiresAt.After(now)) {
			slot = i
		}
	}
	if slot < 0 {
		return false
	}
	ch.Affects[slot] = model.Affect{Type: affectFaceTransform, ClientType: affectFaceTransformClientType,
		Value: mesh, ExpiresAt: expires, NextTick: now.Add(8 * time.Second)}
	return true
}

// removeFaceAffect removes the face transformation and reports whether one existed.
func removeFaceAffect(ch *model.Char) bool {
	if ch == nil {
		return false
	}
	removed := false
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectFaceTransform {
			ch.Affects[i] = model.Affect{}
			removed = true
		}
	}
	return removed
}

// activePlayerAffect returns the active affect of the requested type, or nil.
// It mirrors activeMobAffect for mobs.
func activePlayerAffect(ch *model.Char, affectType byte) *model.Affect {
	return activePlayerAffectAt(ch, affectType, time.Now())
}

func activePlayerAffectAt(ch *model.Char, affectType byte, now time.Time) *model.Affect {
	if ch == nil {
		return nil
	}
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectType && ch.Affects[i].ExpiresAt.After(now) {
			return &ch.Affects[i]
		}
	}
	return nil
}

// absorbManaControlDamageAt implements the requested server rule for affect 18:
// 90% of incoming damage consumes MP at 1:1, and the rest reaches HP. Missing
// mana spills into HP in the same hit, without a reserve or a free reduction.
// Call after other mitigations, once per resolved hit, before applying HP/death.
// Integer remainders go to HP; use uint64 so wide damage cannot overflow.
func absorbManaControlDamageAt(ch *model.Char, damage uint32, now time.Time) uint32 {
	if damage == 0 || ch == nil || playerCurHP(ch) == 0 ||
		activePlayerAffectAt(ch, 18, now) == nil {
		return damage
	}
	const manaPercent = 90
	absorbed := minU32(playerCurMP(ch), uint32(uint64(damage)*manaPercent/100))
	if absorbed > 0 {
		spendPlayerMP(ch, absorbed)
	}
	return damage - absorbed
}

// accumulateAffect adds time to an existing affect (or creates one), with a cap.
// It follows the WYD 7.48 roast chicken and EXP chest pattern: Affect.Time += X
// up to a limit, rejecting further use at the cap. addUnits and maxUnits are
// eight-second units. It returns false without consumption when already capped.
func accumulateAffect(ch *model.Char, affectType byte, value, level, addUnits, maxUnits int) bool {
	return accumulateAffectAt(ch, affectType, value, level, addUnits, maxUnits, time.Now())
}

func accumulateAffectAt(ch *model.Char, affectType byte, value, level, addUnits, maxUnits int, now time.Time) bool {
	if ch == nil || affectType == 0 || maxUnits < 1 {
		return false
	}
	if addUnits < 1 {
		addUnits = 1
	}
	const unit = 8 * time.Second
	empty := -1
	for i := range ch.Affects {
		a := &ch.Affects[i]
		if a.Type == affectType && a.ExpiresAt.After(now) {
			remaining := int((a.ExpiresAt.Sub(now) + unit - 1) / unit)
			if remaining >= maxUnits {
				return false // Already capped: further use/consumption is rejected.
			}
			total := minInt(remaining+addUnits, maxUnits)
			a.Value, a.Level = value, level
			a.ExpiresAt = now.Add(time.Duration(total) * unit)
			if a.NextTick.IsZero() {
				a.NextTick = now.Add(unit)
			}
			return true
		}
		if empty < 0 && (a.Type == 0 || !a.ExpiresAt.After(now)) {
			empty = i
		}
	}
	if empty < 0 {
		return false
	}
	total := minInt(addUnits, maxUnits)
	ch.Affects[empty] = model.Affect{Type: affectType, ClientType: affectType, Value: value, Level: level,
		ExpiresAt: now.Add(time.Duration(total) * unit), NextTick: now.Add(unit)}
	return true
}

type volatileBuffResult uint8

const (
	volatileBuffRejected volatileBuffResult = iota
	volatileBuffApplied
	volatileBuffAlreadyActive
)

// applyVolatileBuff applies the entire bundle or nothing. Chocolate/Candy use
// four simultaneous slots; partial acceptance would create a state absent from
// the native handler. The result distinguishes an active-affect rejection from
// configuration/missing-slot errors, allowing feedback without item consumption.
func (w *World) applyVolatileBuff(ch *model.Char, rule model.VolatileRule, source model.Item) volatileBuffResult {
	if ch == nil {
		return volatileBuffRejected
	}
	affects := rule.Affects
	if len(affects) == 0 && rule.AffectType > 0 {
		affects = []model.VolatileAffect{{
			Type: rule.AffectType, Value: rule.AffectValue,
			Level: rule.AffectLevel, DurationUnits: rule.DurationUnits,
		}}
	}
	if len(affects) == 0 {
		return volatileBuffRejected
	}
	snapshot := cloneCharacterState(ch)
	now := w.now()
	for _, affect := range affects {
		affectType, value := affect.Type, affect.Value
		if affect.SkillID > 0 {
			skill, exists := w.skills[affect.SkillID]
			if !exists || skill.AffectType <= 0 || skill.AffectType > 255 {
				*ch = snapshot
				return volatileBuffRejected
			}
			affectType, value = skill.AffectType, skill.AffectValue
		}
		var applied bool
		if rule.Accumulate {
			applied = accumulateAffectAt(ch, byte(affectType), value, affect.Level,
				affect.DurationUnits, rule.MaxDurationUnits, now)
		} else {
			applied = setAffectAt(ch, byte(affectType), value, affect.Level,
				affect.DurationUnits, now)
		}
		if !applied {
			*ch = snapshot
			if activePlayerAffectAt(ch, byte(affectType), now) != nil {
				return volatileBuffAlreadyActive
			}
			return volatileBuffRejected
		}
		// The active affect records the item instance consumed for this application.
		// For accumulating buffs, the source becomes the last unit that extended
		// the absolute deadline.
		active := activePlayerAffectAt(ch, byte(affectType), now)
		if active == nil {
			*ch = snapshot
			return volatileBuffRejected
		}
		active.SourceItemUID = source.UID
		active.SourceItemIndex = source.Index
	}
	return volatileBuffApplied
}

// TK Critical Armor is stored as score type 31 with the 7.48 visual 24,
// which separates it from Coin Armor (visual 31).
const affectCriticalArmorClientType = 24

func skillAffect(skill model.SkillDef) (byte, int, bool) {
	if skill.AffectType > 0 {
		return byte(skill.AffectType), skill.AffectValue, true
	}
	if skill.TickType > 0 {
		return byte(skill.TickType), skill.TickValue, true
	}
	return 0, 0, false
}

func sameSupportGroup(caster, target *Player) bool {
	return caster == target || caster != nil && target != nil && caster.Party != nil && caster.Party == target.Party
}

func supportSkillTargetLimit(ch *model.Char, skill model.SkillDef) int {
	limit := maxInt(1, skill.MaxTarget)
	if skill.Index != 41 && skill.Index != 44 {
		return limit
	}
	branch := ((skill.Index % 24) / 8) + 1
	limit = int(playerMastery(ch, branch))/25 + 2
	return clampInt(limit, 2, 13)
}

func (w *World) supportTargets(p *Player, req skillCastRequest, skill model.SkillDef) []*Player {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
		return nil
	}
	// Samaritan is a self buff in WYD-Go: never another player or a monster.
	if skill.Index == 3 {
		if playerCurHP(p.Char) == 0 {
			return nil
		}
		return []*Player{p}
	}
	revive := skill.Index == 31 || skill.Index == 99
	// An explicit target is an intent, not a hint. Never silently turn a stale,
	// out-of-range or cross-runtime target into a self cast.
	if req.TargetID != 0 && req.TargetID != p.ID {
		target := w.playerByID(req.TargetID)
		if target == nil || !target.InWorld || target.Char == nil || w.criticalPlayerReserved(target) ||
			(!revive && playerCurHP(target.Char) == 0) ||
			!w.playersShareGameplaySpace(p, target) ||
			(!sameSupportGroup(p, target) && skill.Index != 47) ||
			chebyshev(p.X, p.Y, target.X, target.Y) > maxInt(6, skill.Range) {
			return nil
		}
	}
	limit := supportSkillTargetLimit(p.Char, skill)
	if skill.Party != 0 && limit > 1 && p.Party != nil {
		result := make([]*Player, 0, len(p.Party.Members))
		for _, member := range p.Party.Members {
			if len(result) >= limit {
				break
			}
			if member != nil && member.InWorld && member.Char != nil &&
				(revive || playerCurHP(member.Char) > 0) &&
				w.playersShareGameplaySpace(p, member) &&
				chebyshev(p.X, p.Y, member.X, member.Y) <= maxInt(6, skill.Range) {
				// Reject the whole selected cast before charging its caster. Skipping
				// a reserved member would silently apply a partial party operation.
				if w.criticalPlayerReserved(member) {
					return nil
				}
				result = append(result, member)
			}
		}
		if len(result) > 0 {
			return result
		}
	}
	if target := w.playerByID(req.TargetID); target != nil && target.InWorld &&
		target.Char != nil && w.playersShareGameplaySpace(p, target) &&
		(revive || playerCurHP(target.Char) > 0) &&
		(sameSupportGroup(p, target) || skill.Index == 47) &&
		chebyshev(p.X, p.Y, target.X, target.Y) <= maxInt(6, skill.Range) {
		return []*Player{target}
	}
	if (req.TargetID == 0 || req.TargetID == p.ID) && (revive || playerCurHP(p.Char) > 0) {
		return []*Player{p}
	}
	return nil
}

func cleansePlayer(ch *model.Char, cureSoul bool) bool {
	changed := false
	for i := range ch.Affects {
		switch ch.Affects[i].Type {
		case 1, 3, 5, 7, 10, 12, 20:
			ch.Affects[i] = model.Affect{}
			changed = true
		case 32:
			// W2PP InstanceType 8: the caster's eighth white-magic skill permits
			// removing this affect; the target's learned skills do not.
			if cureSoul {
				ch.Affects[i] = model.Affect{}
				changed = true
			}
		}
	}
	return changed
}

// supportSkillResult describes a change from the non-aggressive _MSG_Attack path.
// The client selects/displays targets; the server validates group, range and values.
type supportSkillResult struct {
	player  *Player
	hpDelta uint32
	revived bool
}

func foemaHealAmount(skillIndex, mastery, instanceValue int) int {
	heal := mastery*3/2 + instanceValue
	if skillIndex == 27 {
		heal = mastery*2 + instanceValue
	}
	// This formula retains the Mortal heal rule: WYD 7.48 applies 140% and
	// caps Heal/Recover at 548 HP for that evolution.
	return minInt(548, heal*14/10)
}

// Support buffs last from 5 minutes (mastery 0) to 10 minutes (mastery 320),
// in units of 8 seconds. This WYD-Go rule replaces the W2PP
// (AffectTime + 1) * (100 + mastery) / 100 formula, which left some buffs
// above an hour and others at two minutes.
const (
	buffMinUnits = 38 // 304 s
	buffMaxUnits = 75 // 600 s
)

func nativeSkillAffectUnits(skill model.SkillDef, mastery int) int {
	// Mastery never exceeds 320 (point allocation, recalculation and buffs).
	return buffMinUnits + (buffMaxUnits-buffMinUnits)*clampInt(mastery, 0, 320)/320
}

// Soul Limit lasts longer than other buffs: 5 minutes for a new Celestial,
// growing with the Celestial level to 15 minutes at level 199 (SubCelestial
// included). Other evolutions keep the 5-minute minimum.
const (
	soulLimitMinUnits = 38  // 304 s
	soulLimitMaxUnits = 113 // 904 s
)

func soulLimitAffectTime(_ model.SkillDef, ch *model.Char) int {
	if ch == nil || !isCelestialEvolution(ch) {
		return soulLimitMinUnits
	}
	level := clampInt(int(playerLevel(ch)), 0, int(maxCelestialLevel))
	return soulLimitMinUnits + (soulLimitMaxUnits-soulLimitMinUnits)*level/int(maxCelestialLevel)
}

func setSoulLimitAffectAt(ch *model.Char, affectType byte, value, level, durationUnits int, now time.Time) bool {
	if ch == nil || affectType == 0 {
		return false
	}
	if durationUnits < 0 {
		durationUnits = 0
	}
	expires := now.Add(time.Duration(durationUnits*8) * time.Second)
	ch.Affects[15] = model.Affect{
		Type: affectType, ClientType: affectType, Value: value, Level: level,
		ExpiresAt: expires, NextTick: now.Add(8 * time.Second),
	}
	return true
}

func (w *World) applySupportSkill(p *Player, req skillCastRequest, skill model.SkillDef, mastery int) []supportSkillResult {
	return w.applySupportSkillToTargets(p, w.supportTargets(p, req, skill), skill, mastery)
}

func (w *World) applySupportSkillToTargets(p *Player, targets []*Player, skill model.SkillDef, mastery int) []supportSkillResult {
	if p == nil || p.Char == nil || w.criticalPlayerReserved(p) {
		return nil
	}
	// Direct callers also need an all-participant preflight: revival can spend
	// caster mana and teleport/buffs mutate targets before a later target is seen.
	for _, target := range targets {
		if target == nil || target.Char == nil || w.criticalPlayerReserved(target) {
			return nil
		}
	}
	now := w.now()
	changed := make([]supportSkillResult, 0, len(targets))
	for _, target := range targets {
		if playerCurHP(target.Char) == 0 && skill.Index != 31 && skill.Index != 99 {
			continue
		}
		oldMaxHP := playerMaxHP(target.Char)
		oldHP := playerCurHP(target.Char)
		applied := false
		switch skill.Index {
		case 25: // Detoxify
			applied = cleansePlayer(target.Char, p.Char.LearnedSkill&(1<<7) != 0)
		case 27, 29: // Heal / Recover
			heal := foemaHealAmount(skill.Index, mastery, skill.InstanceValue)
			restorePlayerHP(target.Char, uint32(heal))
			applied = true
		case 31: // Foema Rebirth.
			setPlayerCurMP(p.Char, 0)
			if playerCurHP(target.Char) == 0 && w.intn(2) == 0 {
				setPlayerCurHP(target.Char, playerMaxHP(target.Char))
				setPlayerCurMP(target.Char, playerMaxMP(target.Char))
				target.DeadAt = time.Time{}
				applied = true
			}
		case 99: // Sephira Resurrection: dead caster's self-cast only (WYD 7.48).
			if target == p && playerCurHP(target.Char) == 0 {
				chance := clampInt((int(playerLevel(target.Char))+1)/5, 0, 100)
				if w.intn(100) < chance {
					halfHP := playerMaxHP(target.Char) / 2
					if halfHP == 0 {
						halfHP = 1
					}
					setPlayerCurHP(target.Char, halfHP)
					setPlayerCurMP(target.Char, playerMaxMP(target.Char)/2)
					target.DeadAt = time.Time{}
					target.LastAttackerID, target.CombatTargetID = 0, 0
					applied = true
				}
			}
		case 42: // Teleport brings the selected member next to the caster.
			if target != p {
				target.X, target.Y = w.findFreeGameplayPosition(p, target, p.X, p.Y, 4)
				target.Char.X, target.Char.Y = target.X, target.Y
				w.refreshPlayerVisibility(target)
				w.sendToPlayerView(target, func() []byte {
					return wire.ActionStop(target.ID, target.X, target.Y)
				})
			}
			applied = true
		case 47: // Cancellation removes Immunity (Affect 19).
			for i := range target.Char.Affects {
				if target.Char.Affects[i].Type == 19 {
					target.Char.Affects[i] = model.Affect{}
					applied = true
					break
				}
			}
		case 56, 57, 58, 59, 60, 61, 62, 63:
			applied = w.castSummon(p, skill, mastery)
		default:
			affectType, value, ok := skillAffect(skill)
			if ok {
				duration := nativeSkillAffectUnits(skill, mastery)
				if skill.Index == 102 {
					duration = soulLimitAffectTime(skill, p.Char)
				}
				// SkillData.csv preserves Critical Armor's raw identifier 50, but
				// the authoritative rule uses affect 31 and the 7.48 executable
				// requires visual 24. Normalize before persistence to retain stats.
				if skill.Index == 15 {
					affectType = 31
				}
				if skill.Index == 102 {
					applied = setSoulLimitAffectAt(target.Char, affectType, value, mastery, duration, now)
				} else {
					// Affect 31 adds Level/10 + Value, so Coin Armor keeps the
					// mastery level, as the official server does.
					applied = setAffectAt(target.Char, affectType, value, mastery, duration, now)
				}
				if applied && skill.Index == 15 { // TK Critical Armor uses visual 24 in 7.48.
					for i := range target.Char.Affects {
						if target.Char.Affects[i].Type == affectType {
							target.Char.Affects[i].ClientType = affectCriticalArmorClientType
							break
						}
					}
				}
			}
		}
		if !applied {
			continue
		}
		// Heal and Recover change only current HP. Recalculating the entire score
		// here made a simple heal reapply/migrate character stats and points.
		if skill.Index != 27 && skill.Index != 29 {
			w.recalcPlayer(target.Char)
		}
		if skill.Index == 75 { // Invisibility removes the player from mob aggro lists.
			// Only awake mobs can have targets. Avoid a global scan of thousands
			// of NPCs for a player-triggered skill.
			for _, m := range w.activeMobs {
				if m.TargetID == target.ID {
					m.TargetID = 0
				}
			}
		}
		if playerMaxHP(target.Char) > oldMaxHP {
			restorePlayerHP(target.Char, playerMaxHP(target.Char)-oldMaxHP)
		}
		hpDelta := uint32(0)
		if playerCurHP(target.Char) > oldHP {
			hpDelta = playerCurHP(target.Char) - oldHP
		}
		changed = append(changed, supportSkillResult{
			player: target, hpDelta: hpDelta, revived: oldHP == 0 && playerCurHP(target.Char) > 0,
		})
	}
	if len(changed) > 0 {
		log.Printf("[#%d] support %q affected=%d mastery=%d", p.Session.ID, skill.Name, len(changed), mastery)
	}
	return changed
}

// setMobAffectWithSource applies an affect to a mob and records the stable
// source identity. Player session IDs are only a wire/display hint; a player
// source must always carry CharacterUID so a recycled ClientID cannot inherit
// the old debuff.
func setMobAffectWithSourceAt(m *Mob, ownerID uint16, ownerCharacterUID string,
	affectType byte, value, level, durationUnits int, now time.Time) bool {
	if m == nil || affectType == 0 {
		return false
	}
	expires := now.Add(time.Duration(maxInt(1, durationUnits)*8) * time.Second)
	slot := -1
	for i := range m.Affects {
		if m.Affects[i].Type == affectType {
			if !canReplaceAffect(m.Affects[i], value, level, expires, now) {
				return false
			}
			slot = i
			break
		}
		if slot < 0 && (m.Affects[i].Type == 0 || !m.Affects[i].ExpiresAt.After(now)) {
			slot = i
		}
	}
	if slot < 0 {
		return false
	}
	m.Affects[slot] = model.Affect{Type: affectType, ClientType: affectType, Value: value, Level: level,
		OwnerID: ownerID, OwnerCharacterUID: strings.TrimSpace(ownerCharacterUID),
		ExpiresAt: expires, NextTick: now.Add(8 * time.Second)}
	return true
}

func setPlayerOwnedMobAffect(m *Mob, owner *Player, affectType byte, value, level, durationUnits int) bool {
	return setPlayerOwnedMobAffectAt(m, owner, affectType, value, level, durationUnits, time.Now())
}

func setPlayerOwnedMobAffectAt(m *Mob, owner *Player, affectType byte, value, level, durationUnits int, now time.Time) bool {
	if owner == nil || owner.Char == nil || strings.TrimSpace(owner.Char.UID) == "" {
		return false
	}
	return setMobAffectWithSourceAt(m, owner.ID, owner.Char.UID, affectType, value, level, durationUnits, now)
}

func setMobOwnedMobAffectAt(m *Mob, owner *Mob, affectType byte, value, level, durationUnits int, now time.Time) bool {
	if owner == nil || owner.ID == 0 {
		return false
	}
	return setMobAffectWithSourceAt(m, owner.ID, "", affectType, value, level, durationUnits, now)
}

func (w *World) applySkillMobEffects(owner *Player, m *Mob, skill model.SkillDef, mastery int) {
	changed := false
	now := w.now()
	if skill.AffectType > 0 {
		changed = setPlayerOwnedMobAffectAt(m, owner, byte(skill.AffectType), skill.AffectValue, mastery, skill.AffectTime, now) || changed
	}
	if skill.TickType > 0 {
		changed = setPlayerOwnedMobAffectAt(m, owner, byte(skill.TickType), skill.TickValue, mastery, skill.AffectTime, now) || changed
	}
	if changed {
		w.publishMobAffects(m)
	}
}

func activeMobAffect(m *Mob, affectType byte) *model.Affect {
	return activeMobAffectAt(m, affectType, time.Now())
}

func activeMobAffectAt(m *Mob, affectType byte, now time.Time) *model.Affect {
	if m == nil {
		return nil
	}
	for i := range m.Affects {
		if m.Affects[i].Type == affectType && m.Affects[i].ExpiresAt.After(now) {
			return &m.Affects[i]
		}
	}
	return nil
}

func effectiveMobDefense(m *Mob) int {
	return effectiveMobDefenseAt(m, time.Now())
}

func effectiveMobDefenseAt(m *Mob, now time.Time) int {
	defense := int(m.Def.Score.Defense)
	if a := activeMobAffectAt(m, 12, now); a != nil {
		defense = defense * (100 - clampInt(a.Value, 0, 100)) / 100
	}
	return maxInt(0, defense)
}

// The recovered native skill path applies an additional 3/2 defense factor
// to class-1 targets. Keep it separate from physical defense because the
// factor belongs to elemental skill resolution, not to the target score.
func effectiveMobElementalDefenseAt(m *Mob, now time.Time) int {
	if m == nil || m.Def == nil || m.Def.Score == nil {
		return 0
	}
	defense := effectiveMobDefenseAt(m, now)
	if m.Def.ClassInfo == 1 {
		defense = defense * 3 / 2
	}
	return maxInt(0, defense)
}

func effectiveMobAttack(m *Mob) int {
	return effectiveMobAttackAt(m, time.Now())
}

func effectiveMobAttackAt(m *Mob, now time.Time) int {
	attack := int(m.Def.Score.Attack)
	if a := activeMobAffectAt(m, 10, now); a != nil {
		attack -= a.Level/5 + a.Value
	}
	return maxInt(1, attack)
}

func effectiveMobAttackRun(m *Mob) byte {
	return effectiveMobAttackRunAt(m, time.Now())
}

func effectiveMobAttackRunAt(m *Mob, now time.Time) byte {
	attackRun := m.Def.Score.AttackRun
	if a := activeMobAffectAt(m, 1, now); a != nil {
		run := maxInt(1, int(attackRun&0x0f)-a.Value)
		attackRun = attackRun&0xf0 | uint32(run)
	}
	return byte(attackRun & 0xff)
}

func effectiveMobResistances(m *Mob) model.ElementalResists {
	return effectiveMobResistancesAt(m, time.Now())
}

func effectiveMobResistancesAt(m *Mob, now time.Time) model.ElementalResists {
	resist := model.ElementalResists{
		Fire: m.Def.Score.ResistFire, Ice: m.Def.Score.ResistIce,
		Sacred: m.Def.Score.ResistHoly, Thunder: m.Def.Score.ResistThunder,
	}
	if a := activeMobAffectAt(m, 3, now); a != nil {
		reduce := func(value uint32) uint32 {
			penalty := uint32(maxInt(0, a.Value))
			if penalty >= value {
				return 0
			}
			return value - penalty
		}
		resist.Fire = reduce(resist.Fire)
		resist.Ice = reduce(resist.Ice)
		resist.Sacred = reduce(resist.Sacred)
		resist.Thunder = reduce(resist.Thunder)
	}
	return resist
}

func mobPublicExtended(m *Mob) *model.Score {
	return mobPublicExtendedAt(m, time.Now())
}

func mobPublicExtendedAt(m *Mob, now time.Time) *model.Score {
	if m == nil || m.Def == nil {
		return nil
	}
	ext := m.Def.MakeScore(m.HP)
	ext.Defense = uint32(effectiveMobDefenseAt(m, now))
	ext.Attack = uint32(effectiveMobAttackAt(m, now))
	ext.AttackRun = uint32(effectiveMobAttackRunAt(m, now))
	resist := effectiveMobResistancesAt(m, now)
	ext.ResistFire = resist.Fire
	ext.ResistIce = resist.Ice
	ext.ResistHoly = resist.Sacred
	ext.ResistThunder = resist.Thunder
	return ext
}

func (w *World) publishMobAffects(m *Mob) {
	if m == nil || m.Def == nil || m.Dead {
		return
	}
	now := w.now()
	w.sendToMobViewProtocol(m, func(observer *Player) []byte {
		// Mob score embeds STRUCT_SCORE, so each observer receives the ABI selected
		// at login instead of interpreting another client's score layout.
		return wire.MobScore(m.ID, mobPublicExtendedAt(m, now), m.Affects[:])
	})
}

// applyAffectStats always recalculates from the clean score; renewing/removing
// a buff never permanently accumulates bonuses. It returns attack-speed bonus
// percentage points so equipment recalculation can quantize the final sum once.
func (w *World) applyAffectStats(ch *model.Char) int {
	if ch == nil {
		return 0
	}
	ensureScore(ch)
	return w.applyExtendedAffectStats(ch)
}

// applyExtendedAffectStats applies buffs to a runtime copy of the sidecar.
// The persisted base is never modified, so renewing/recalculating a buff cannot
// accumulate bonuses. Calculations use int64 to avoid overflow before clamping.
func (w *World) applyExtendedAffectStats(ch *model.Char) int {
	if ch == nil || ch.Score == nil {
		return 0
	}
	if ch.RuntimeScore == nil {
		applyScore(ch)
	}
	e := ch.RuntimeScore
	attackSpeedPoints := int(e.AttackRun>>4&0x0F) * 10
	now := w.now()
	mul := func(value uint32, percent int) uint32 {
		if percent < 0 {
			percent = 0
		}
		result := int64(value) * int64(percent) / 100
		if result > int64(maxScoreValue) {
			return maxScoreValue
		}
		return uint32(result)
	}
	add := func(value uint32, delta int64) uint32 {
		result := int64(value) + delta
		if result < 0 {
			return 0
		}
		if result > int64(maxScoreValue) {
			return maxScoreValue
		}
		return uint32(result)
	}
	fixedMortalSoul := false
	for i := range ch.Affects {
		a := &ch.Affects[i]
		if a.Type == 0 || !a.ExpiresAt.After(now) {
			continue
		}
		switch a.Type {
		case 1: // Slow
			run := maxInt(1, int(e.AttackRun&0x0F)-a.Value)
			e.AttackRun = e.AttackRun&0xF0 | uint32(run)
		case 2: // Speed
			run := minInt(15, int(e.AttackRun&0x0F)+a.Value)
			e.AttackRun = e.AttackRun&0xF0 | uint32(run)
		case 3: // Pursuit
			e.ResistFire = uint32(maxInt(0, int(e.ResistFire)-a.Value))
			e.ResistIce = uint32(maxInt(0, int(e.ResistIce)-a.Value))
			e.ResistHoly = uint32(maxInt(0, int(e.ResistHoly)-a.Value))
			e.ResistThunder = uint32(maxInt(0, int(e.ResistThunder)-a.Value))
		case 4: // Damage buffs (Kappa/Competent/Mental, food, Courage...).
			// WYD 7.48 (Basedef.cpp:3900) uses a fixed bonus; this project's rule
			// scales by tier. Value is the percentage bonus to physical and magic
			// attack, configured per item/code in volatiles.json.
			e.Attack = mul(e.Attack, 100+a.Value)
			e.MagicAttack = mul(e.MagicAttack, 100+a.Value)
		case 5:
			e.Dex = mul(e.Dex, 100-clampInt(a.Value, 0, 100))
		case 8: // Berserk; native 7.48 FUN_0054FDB9 and help 5009.
			attackSpeedPoints += a.Level/10 + 10
		case 7:
			penalty := a.Level/10 + 20
			attackSpeedPoints -= penalty
			e.Int = add(e.Int, int64(-penalty))
		case 9:
			e.Attack = add(e.Attack, int64((a.Level*5/20+a.Value)*3/2))
		case 10:
			e.Attack = add(e.Attack, int64(-(a.Level/5 + a.Value)))
		case 11:
			e.Defense = add(e.Defense, int64(a.Level/3+a.Value))
		case 12:
			e.Defense = mul(e.Defense, 100-clampInt(a.Value, 0, 100))
		case 13: // Assault; official TMSRVIN sub_4BB4C0.
			bonus := a.Level/10 + a.Value
			if serverSkillLearned(ch, 15) { // Critical Armor.
				bonus += 5
			}
			e.Attack = mul(e.Attack, 100+bonus)
			e.MaxHP = mul(e.MaxHP, 90)
		case 14: // Growth; official TMSRVIN sub_4BB4C0 and sub_4CE1C0.
			level := int(playerLevel(ch))
			if !mortalSoulCharacter(ch) && ch.LearnedSkill&celestialSoulBit != 0 {
				level += 400
			}
			con := int64(a.Value + a.Level + level*4/3)
			var stats int64
			if serverSkillLearned(ch, 7) { // Destiny.
				con += int64(a.Level)
				stats = 200
			}
			e.Con = add(e.Con, con+stats)
			e.Str = add(e.Str, stats)
			e.Int = add(e.Int, stats)
			e.Dex = add(e.Dex, stats)
			// sub_4CE1C0 adds 2 HP per extra CON and 2 MP per extra INT.
			e.MaxHP = add(e.MaxHP, 2*(con+stats))
			e.MaxMP = add(e.MaxMP, 2*stats)
		case 15: // Athena's Touch
			value := uint32(maxInt(0, a.Level/10+a.Value))
			for j := range e.Mastery {
				// Same mastery cap as recalculation and W2PP Type 15.
				e.Mastery[j] = uint32(minInt(320, int(e.Mastery[j])+int(value)))
			}
		case 16: // BM transformations; official TMSRVIN sub_4BB4C0, table 0x640660.
			// The 7.48 client form table at 0x5C36B0 holds the same base values.
			type transformBonus struct {
				minDamage, maxDamage, minDefense, maxDefense int
				minHP, maxHP, run, attackSpeed, resist       int
			}
			bonuses := [...]transformBonus{
				{110, 130, 95, 105, 95, 105, 1, 20, 5},
				{95, 105, 105, 115, 110, 140, 0, 0, 20},
				{110, 120, 90, 110, 100, 120, 1, 20, 5},
				{115, 125, 115, 125, 105, 115, 0, 20, 20},
				{118, 133, 110, 120, 113, 133, 2, 20, 17},
			}
			idx := a.Value - 1
			if idx >= 0 && idx < len(bonuses) {
				b := bonuses[idx]
				damageAdd, defenseAdd, hpAdd := 0, 0, 0
				resistAdd, attackSpeedAdd := 0, 0
				// The official server tests local bit 21 (Mist of Hell, 69) for
				// the Wolf, Bear and Astaroth bonuses; 65 and 67 add nothing here.
				formPassive := learnedLocal(ch, 69-48)
				switch idx {
				case 0:
					if formPassive {
						damageAdd, resistAdd = 10, b.resist
						// Native raw critical 50 + 5*(Special[2]/50), in tenths of a percent.
						e.Critical = addCritical(e.Critical, 50+5*(int(playerMastery(ch, 2))/50))
					}
				case 1:
					if formPassive {
						hpAdd, attackSpeedAdd, resistAdd = 20, 20, b.resist
					}
				case 2:
					if formPassive {
						damageAdd, defenseAdd, hpAdd = 10, 5, 5
						attackSpeedAdd, resistAdd = 20, b.resist
					}
				case 3:
					resistAdd = b.resist
				case 4:
					attackSpeedAdd, resistAdd = 10, b.resist
				}
				if learnedLocal(ch, 71-48) { // Eden learned: every form.
					damageAdd += 5
					defenseAdd += 2
				}
				interpolate := func(minimum, maximum int) int {
					return minimum + (maximum-minimum)*a.Level/200
				}
				e.Attack = mul(e.Attack, interpolate(b.minDamage+damageAdd, b.maxDamage+damageAdd))
				e.Defense = mul(e.Defense, interpolate(b.minDefense+defenseAdd, b.maxDefense+defenseAdd))
				if idx == 0 {
					e.Defense = add(e.Defense, 5) // Native Wolf flat defense.
				}
				e.MaxHP = mul(e.MaxHP, interpolate(b.minHP+hpAdd, b.maxHP+hpAdd))
				e.ResistFire = uint32(clampInt(int(e.ResistFire)+resistAdd, 0, 100))
				e.ResistIce = uint32(clampInt(int(e.ResistIce)+resistAdd, 0, 100))
				e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+resistAdd, 0, 100))
				e.ResistThunder = uint32(clampInt(int(e.ResistThunder)+resistAdd, 0, 100))
				run := minInt(15, int(e.AttackRun&0x0F)+b.run)
				attackSpeedPoints += b.attackSpeed + attackSpeedAdd
				e.AttackRun = e.AttackRun&0xF0 | uint32(run)
			}
		case 21: // Transfer Armor Class; official TMSRVIN sub_4BB4C0.
			bonus := a.Level/10 + a.Value
			if ch.Class == 3 && learnedLocal(ch, 95-72) { // Rapid Hit, server bit 7.
				bonus += 5
			}
			e.Defense = add(e.Defense, int64(-(a.Level/3 + 10)))
			e.Attack = mul(e.Attack, 100+bonus)
		case 24: // Rescue; official TMSRVIN sub_4BB4C0.
			percent, resist := 100, 0
			if _, _, ok := w.rightHandShield(ch); ok {
				percent, resist = percent+30, 30
			}
			if serverSkillLearned(ch, 15) { // Critical Armor.
				percent += 10
			}
			e.Defense = mul(add(e.Defense, int64(a.Level)), percent)
			e.ResistFire = uint32(clampInt(int(e.ResistFire)+resist, 0, 100))
			e.ResistIce = uint32(clampInt(int(e.ResistIce)+resist, 0, 100))
			e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+resist, 0, 100))
			e.ResistThunder = uint32(clampInt(int(e.ResistThunder)+resist, 0, 100))
		case 25:
			e.Defense = add(e.Defense, int64(a.Value+a.Level+int(playerMastery(ch, 1))+1))
			resist := (a.Value + a.Level/4) / 10
			e.ResistFire = uint32(clampInt(int(e.ResistFire)+resist, 0, 100))
			e.ResistIce = uint32(clampInt(int(e.ResistIce)+resist, 0, 100))
			e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+resist, 0, 100))
			e.ResistThunder = uint32(clampInt(int(e.ResistThunder)+resist, 0, 100))
		case 26:
			e.Evasion = add(e.Evasion, int64(a.Level+a.Value*10))
		case 29: // Soul Limit; the learned bit selects the Mortal branch.
			// The existing Celestial branch calculates from raw bStatus.
			// The official server's Mortal branch uses the current attributes.
			// SoulInfo is shared between forms; affects belong to each form.
			bonuses := [...][4]int{
				{120, 0, 0, 0}, {0, 120, 0, 0},
				{0, 0, 120, 0}, {0, 0, 0, 120},
				{80, 0, 0, 40}, {0, 80, 0, 40},
				{0, 0, 80, 40}, {40, 0, 0, 80},
				{0, 40, 0, 80}, {0, 0, 40, 80},
			}
			config := int(ch.SoulInfo) - 1
			mortalLearned := mortalSoulCharacter(ch) && ch.LearnedSkill&celestialSoulBit != 0
			if mortalSoulCharacter(ch) && !mortalLearned {
				fixedMortalSoul = true
			}
			if mortalLearned {
				// Official TMSRVIN sub_4BB4C0, affect 29, multiplier 1.
				// Celestial SoulInfo mapping/formulas remain unchanged below.
				bonuses = [10][4]int{
					{40, 0, 0, 30}, {0, 40, 0, 30}, {0, 0, 40, 30},
					{40, 0, 30, 0}, {0, 40, 30, 0}, {0, 0, 30, 40},
					{60, 0, 0, 0}, {0, 60, 0, 0}, {0, 0, 0, 60}, {0, 0, 60, 0},
				}
				if config < 0 || config >= len(bonuses) {
					config = 0
				}
			}
			if (isCelestialEvolution(ch) || mortalLearned) && config >= 0 && config < len(bonuses) {
				raw := ch.Score
				if mortalLearned {
					raw = e // The official Mortal branch includes equipped stats.
				}
				strAdd := int64(raw.Str/100) * int64(bonuses[config][0])
				intAdd := int64(raw.Int/100) * int64(bonuses[config][1])
				dexAdd := int64(raw.Dex/100) * int64(bonuses[config][2])
				conAdd := int64(raw.Con/100) * int64(bonuses[config][3])
				if mortalLearned {
					strAdd = int64(raw.Str) * int64(bonuses[config][0]) / 100
					intAdd = int64(raw.Int) * int64(bonuses[config][1]) / 100
					dexAdd = int64(raw.Dex) * int64(bonuses[config][2]) / 100
					conAdd = int64(raw.Con) * int64(bonuses[config][3]) / 100
					// The official Soul branch caps each primary attribute at 60,000.
					strAdd = int64(minU32(add(raw.Str, strAdd), 60000)) - int64(raw.Str)
					intAdd = int64(minU32(add(raw.Int, intAdd), 60000)) - int64(raw.Int)
					dexAdd = int64(minU32(add(raw.Dex, dexAdd), 60000)) - int64(raw.Dex)
					conAdd = int64(minU32(add(raw.Con, conAdd), 60000)) - int64(raw.Con)
				}
				e.Str = add(e.Str, strAdd)
				e.Int = add(e.Int, intAdd)
				e.Dex = add(e.Dex, dexAdd)
				e.Con = add(e.Con, conAdd)
				// The extra points feed the same derived formulas as base
				// attributes (recalcExtendedPlayer): STR/3 + DEX/8 physical
				// attack, the class INT share of magic attack (with the current
				// amplification), 2 HP per CON and 2 MP per INT. DEX adds no
				// defense. Attack speed (DEX/5) and evasion read the final DEX.
				physicalAdd := strAdd/3 + dexAdd/8
				e.Attack = add(e.Attack, physicalAdd)
				var magicAdd int64
				switch ch.Class {
				case 1, 2:
					magicAdd = intAdd/3 + intAdd/30
				case 0:
					magicAdd = intAdd/4 + intAdd/40
				default:
					magicAdd = physicalAdd
				}
				e.MagicAttack = add(e.MagicAttack, magicAdd*(100+4*int64(e.MagicAmp))/100*5/4)
				e.MaxHP = add(e.MaxHP, conAdd*2)
				e.MaxMP = add(e.MaxMP, intAdd*2)
			}
		case affectCourage:
			// Courage does not modify score. Its fixed bonus belongs to each
			// PvE hit's result and is applied in combat.go/skills.go.
		case 31:
			if a.ClientType == affectCriticalArmorClientType {
				// TK Critical Armor; native 7.48 help 5015: defense
				// +(7 + Trans/50)% and critical +(3 + Trans/50)%.
				bonus := a.Level / 50
				e.Defense = mul(e.Defense, 107+bonus)
				e.Critical = addCritical(e.Critical, 30+10*bonus)
				break
			}
			// Coin Armor; official TMSRVIN sub_4BB4C0.
			defense := a.Level/10 + a.Value
			if ch.Class == 3 && learnedLocal(ch, 87-72) { // Spirit Change, server bit 15.
				defense += 100
			}
			e.Defense = add(e.Defense, int64(defense))
		case 35: // Bigger/Health Potion: native 10% bonus to maximum HP.
			bonus := a.Value
			if bonus <= 0 {
				bonus = 10
			}
			e.MaxHP = mul(e.MaxHP, 100+bonus)
		case 37:
			e.Attack = add(e.Attack, int64(playerMastery(ch, 2)))
		case 38: // Spirit Change; official TMSRVIN sub_4BB4C0.
			special := int64(playerMastery(ch, 2))
			level := int64(playerLevel(ch))
			var mana, hp int64
			if mortalSoulCharacter(ch) {
				mana = int64(e.MaxMP)/4 + (special+level)/4
				hp = 750 + mana
			} else {
				if ch.LearnedSkill&celestialSoulBit != 0 {
					level += 400
				}
				mana = int64(e.MaxMP) / 4
				hp = special/2 + level*2 + 750 + mana
			}
			e.MaxHP = add(e.MaxHP, hp)
			e.MaxMP = add(e.MaxMP, -mana)
		}
	}
	if fixedMortalSoul {
		// Apply the fixed score after the other effects so later HP modifiers
		// cannot turn the 10,000 limit into a percentage bonus. Base state is
		// preserved and normal recalculation restores it on expiration.
		intDelta := int64(2000) - int64(e.Int)
		physicalDelta := int64(2000/3) - int64(e.Str/3)
		e.Attack = add(e.Attack, physicalDelta)
		magicDelta := physicalDelta
		switch ch.Class {
		case 0:
			magicDelta = int64(2000/4+2000/40) - int64(e.Int/4+e.Int/40)
		case 1, 2:
			magicDelta = int64(2000/3+2000/30) - int64(e.Int/3+e.Int/30)
		}
		e.MagicAttack = add(e.MagicAttack, magicDelta*(100+4*int64(e.MagicAmp))/100*5/4)
		e.MaxMP = add(e.MaxMP, intDelta*2)
		e.Str, e.Int, e.MaxHP = 2000, 2000, 10000
	}
	if e.CurHP > e.MaxHP {
		e.CurHP = e.MaxHP
		ch.Score.CurHP = e.CurHP
	}
	if e.CurMP > e.MaxMP {
		e.CurMP = e.MaxMP
		ch.Score.CurMP = e.CurMP
	}
	e.AttackRun = packedAttackSpeed(attackSpeedPoints) | e.AttackRun&0x0F
	normalizeRuntimeScore(ch)
	return attackSpeedPoints
}

func (w *World) tickMobAffects(now time.Time, shard, shardCount int) {
	accountsToSave := make(map[*model.Account]struct{})
	killPlans := make([]*killRewardPlan, 0)
	for _, m := range w.activeMobs {
		if w.criticalGuildMobReserved(m) {
			continue
		}
		if shardCount > 1 && int(m.ID)%shardCount != shard {
			continue
		}
		if m == nil || m.Dead || m.HP == 0 {
			continue
		}
		expired := false
		for i := range m.Affects {
			a := &m.Affects[i]
			if a.Type == 0 {
				continue
			}
			// Check the retained identity before source validity: a disconnected
			// reserved owner is intentionally still indexed until acknowledgement.
			if w.criticalPlayerReserved(w.affectOwnerPlayer(a)) {
				continue
			}
			playerOwner, _, sourceOK := w.resolveMobAffectSource(m, a)
			if !sourceOK {
				*a = model.Affect{}
				expired = true
				continue
			}
			if !a.ExpiresAt.After(now) {
				*a = model.Affect{}
				expired = true
				continue
			}
			if now.Before(a.NextTick) {
				continue
			}
			if a.Type == 20 && playerOwner != nil && w.criticalKillParticipantsReserved(playerOwner) {
				continue
			}
			a.NextTick = now.Add(8 * time.Second)
			if a.Type != 20 { // Poison/bleeding.
				continue
			}
			damage := uint32(clampInt(a.Level/2+a.Value, 1, int(m.HP)))
			m.HP -= damage
			if m.HP == 0 {
				if playerOwner != nil {
					owner := playerOwner
					for _, account := range uniqueKillAccounts(owner,
						w.partyExpShares(owner, 1, w.gameplay.PartyEXPBonusPercent)) {
						accountsToSave[account] = struct{}{}
					}
					if plan := w.planMobKill(owner, m, damage, damage); plan != nil {
						killPlans = append(killPlans, plan)
					}
				} else {
					w.killMobWithoutPlayer(m)
				}
				break
			}
			w.sendToMobViewProtocol(m, func(observer *Player) []byte {
				// Periodic effects share the same protocol boundary as direct hits.
				return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP,
					m.Def.Score.MaxMP, m.Def.Score.MaxMP)
			})
		}
		if expired && !m.Dead && m.HP > 0 {
			w.publishMobAffects(m)
		}
	}
	if len(killPlans) > 0 {
		accounts := make([]*model.Account, 0, len(accountsToSave))
		for account := range accountsToSave {
			accounts = append(accounts, account)
		}
		w.commitKillRewardBatch(nil, killPlans, accounts, "affect kills")
	}
}

func (w *World) tickPlayerAffects(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || w.criticalPlayerReserved(p) {
			continue
		}
		expired, hpChanged := false, false
		// Area damage can kill and submit a reward write that reserves p.
		// Run it after every other mutation of p in this tick, with its
		// deadline already advanced, so the persisted proposal is final.
		var areaDue []int
		for i := range p.Char.Affects {
			a := &p.Char.Affects[i]
			// All player-owned hostile affects share the same runtime boundary,
			// not only the DoT type. Resolve by stable UID whenever available so
			// a recycled ClientID cannot keep a stale debuff alive.
			if affectHasPlayerOwner(a) {
				owner := w.affectOwnerPlayer(a)
				if w.criticalPlayerReserved(owner) {
					continue
				}
				if owner == nil || !w.playersShareGameplaySpace(owner, p) {
					*a = model.Affect{}
					expired = true
					continue
				}
				if a.Type == 20 {
					if allowed, handled := w.guildWarPvP(owner, p); handled && !allowed {
						*a = model.Affect{}
						expired = true
						continue
					}
				}
			}
			if a.Type == 17 && a.ExpiresAt.After(now) && !now.Before(a.NextTick) {
				oldHP := playerCurHP(p.Char)
				heal := a.Level/2 + a.Value
				restorePlayerHP(p.Char, uint32(maxInt(0, heal)))
				a.NextTick = now.Add(8 * time.Second)
				hpChanged = oldHP != playerCurHP(p.Char)
			}
			if (a.Type == 22 || a.Type == 23) && a.ExpiresAt.After(now) && !now.Before(a.NextTick) {
				if w.criticalKillParticipantsReserved(p) {
					continue
				}
				areaDue = append(areaDue, i)
			}
			if a.Type == 20 && a.ExpiresAt.After(now) && !now.Before(a.NextTick) {
				// A DoT may outlive a teleport or an instance transition. Never
				// let an affect created in one private gameplay space damage a
				// character in another; remove the stale cross-runtime affect.
				owner := w.affectOwnerPlayer(a)
				currentHP := playerCurHP(p.Char)
				damage := uint32(clampInt(a.Level/2+a.Value, 1, maxInt(1, int(currentHP)-1)))
				if currentHP > 1 {
					damage = absorbManaControlDamageAt(p.Char, damage, now)
					setPlayerCurHP(p.Char, currentHP-damage)
					hpChanged = true
					if owner != nil {
						w.sendToPlayerView(p, func() []byte {
							return wire.AttackHit(owner.ID, p.ID, owner.X, owner.Y, p.X, p.Y,
								damage, playerMaxHP(p.Char), owner.Char.Exp,
								playerCombatMP(owner.Char))
						})
					}
				}
				a.NextTick = now.Add(8 * time.Second)
			}
			if a.Type != 0 && !a.ExpiresAt.After(now) {
				*a = model.Affect{}
				expired = true
			}
		}
		if hpChanged {
			w.syncPlayerVitals(p)
		}
		if expired {
			w.recalcPlayer(p.Char)
			w.publishPlayerAffects(p)
			w.syncPlayerVitals(p)
			w.sendToPlayerView(p, func() []byte { return playerAppearancePacket(p) })
			// Removing the in-memory affect and waiting for autosave would allow
			// a quick relog to resurrect it. Queue the expired character sidecar
			// immediately for persistence outside the game loop.
			w.saveCharStateAsync(p)
		}
		for _, i := range areaDue {
			if w.criticalPlayerReserved(p) {
				break // a previous area kill reserved p; retry on a later tick
			}
			a := &p.Char.Affects[i]
			skillIndex := 37
			if a.Type == 23 {
				skillIndex = 54
			}
			a.NextTick = now.Add(8 * time.Second)
			w.tickAreaDamageAffect(p, a, skillIndex)
		}
	}
}

func (w *World) tickAreaDamageAffect(p *Player, affect *model.Affect, skillIndex int) {
	if p == nil || p.Char == nil || affect == nil || w.criticalKillParticipantsReserved(p) {
		return
	}
	targets := make([]*Mob, 0, 6)
	for _, m := range w.nearbyMobs(p.X, p.Y, 4) {
		if m == nil || m.Dead || m.HP == 0 || !w.playerCanInteractWithMob(p, m) ||
			chebyshev(p.X, p.Y, m.X, m.Y) > 4 {
			continue
		}
		targets = append(targets, m)
		if len(targets) == 6 {
			break
		}
	}
	if len(targets) == 0 {
		return
	}
	wireTargets := make([]wire.SkillTarget, 0, len(targets))
	appliedByMob := make(map[*Mob]uint32, len(targets))
	now := w.now()
	for _, m := range targets {
		damage := uint32(clampInt(affect.Level+affect.Value, 1, int(maxScoreValue)))
		if _, catalogued := w.skills[33]; skillIndex == 37 && catalogued {
			damage = w.lightningStormTickDamage(p, m, now)
		}
		appliedByMob[m] = minU32(damage, m.HP)
		if damage >= m.HP {
			m.HP = 0
		} else {
			m.HP -= damage
		}
		wireTargets = append(wireTargets, wire.SkillTarget{ID: m.ID, Damage: damage, MaxHP: m.Def.Score.MaxHP})
	}
	primary := targets[0]
	w.sendToMobView(primary, func() []byte {
		visualSkill, motion := tickAreaVisual(skillIndex)
		return wire.SkillHits(p.ID, p.X, p.Y, primary.X, primary.Y, p.Char.Exp,
			playerCombatMP(p.Char), int16(visualSkill), motion, skillVisualLevel(affect.Level), 13, wireTargets)
	})
	batchAccounts := uniqueKillAccounts(p, w.partyExpShares(p, 1, w.gameplay.PartyEXPBonusPercent))
	killPlans := make([]*killRewardPlan, 0, len(targets))
	for _, m := range targets {
		if m.HP == 0 {
			damage := uint32(affect.Level + affect.Value)
			if plan := w.planMobKill(p, m, damage, appliedByMob[m]); plan != nil {
				killPlans = append(killPlans, plan)
			}
		} else {
			w.sendToMobViewProtocol(m, func(observer *Player) []byte {
				// Area effects must not collapse TMProject748 client resources to WORDs.
				return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP,
					m.Def.Score.MaxMP, m.Def.Score.MaxMP)
			})
		}
	}
	w.commitKillRewardBatch(p, killPlans, batchAccounts, "area-affect kills")
}

// lightningStormTickDamage follows W2PP ProcessAffect Type 22: every eight
// seconds the storm attacks up to six nearby enemies as a Thunderbolt (33)
// cast by the owner, so each hit uses Thunderbolt's skill damage, mitigation
// and bonuses instead of the bare affect value (about 300 at high mastery).
func (w *World) lightningStormTickDamage(p *Player, m *Mob, now time.Time) uint32 {
	thunder, ok := w.skills[33]
	if !ok {
		return 1
	}
	damage := w.skillFinalDamage(w.baseSkillDamage(p.Char, thunder), effectiveMobElementalDefenseAt(m, now),
		skillDamageMastery(p.Char))
	damage = damage * 70 / 100 // the same Mortal PvE reduction as a cast skill
	damage = applySkillResistance(damage, thunder.InstanceType, effectiveMobResistancesAt(m, now), true)
	damage = applyCouragePvEDamageAt(p.Char, damage, skillUsesMagic(p.Char, thunder), now)
	result := addFlatDamage(uint32(clampInt(damage, 1, int(maxScoreValue))),
		w.equipmentGemBonuses(p.Char).forceDamage)
	return w.bossMitigateDamage(m, result)
}

func tickAreaVisual(skillIndex int) (int, byte) {
	if skillIndex == 37 {
		// Historical 7.59 Lightning Storm visually reproduces Thunderbolt
		// (skill 33), matching WYD 7.48 ProcessAffect Type 22.
		return 33, 254
	}
	return skillIndex, 254
}
