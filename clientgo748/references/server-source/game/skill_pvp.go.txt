package game

import (
	"log"
	"sort"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func hasActiveAffect(ch *model.Char, affectType byte) bool {
	return hasActiveAffectAt(ch, affectType, time.Now())
}

func hasActiveAffectAt(ch *model.Char, affectType byte, now time.Time) bool {
	if ch == nil {
		return false
	}
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectType && ch.Affects[i].ExpiresAt.After(now) {
			return true
		}
	}
	return false
}

func (w *World) skillPlayerTargets(caster *Player, req skillCastRequest, skill model.SkillDef) []*Player {
	primary := w.playerByID(req.TargetID)
	if !w.canInitiatePvP(caster, primary) {
		return nil
	}
	castRange := maxInt(attackRange, skill.Range)
	if primary == nil || primary == caster || !primary.InWorld || primary.Char == nil ||
		playerCurHP(primary.Char) == 0 || sameSupportGroup(caster, primary) ||
		!w.playersShareGameplaySpace(caster, primary) ||
		chebyshev(caster.X, caster.Y, primary.X, primary.Y) > castRange ||
		!w.combatLineOfSight(caster.X, caster.Y, primary.X, primary.Y) {
		return nil
	}
	targets := []*Player{primary}
	limit := offensiveSkillTargetLimit(caster.Char, skill)
	if limit == 1 {
		return targets
	}
	area := offensiveSkillAreaRadius(caster.Char, skill)
	if area == 0 {
		secondary := w.playerByID(req.SecondaryTargetID)
		if secondary != nil && secondary != primary && secondary != caster && secondary.InWorld &&
			secondary.Char != nil && playerCurHP(secondary.Char) > 0 &&
			!sameSupportGroup(caster, secondary) && w.playersShareGameplaySpace(caster, secondary) &&
			chebyshev(caster.X, caster.Y, secondary.X, secondary.Y) <= maxInt(1, skill.Range) &&
			w.combatLineOfSight(caster.X, caster.Y, secondary.X, secondary.Y) {
			targets = append(targets, secondary)
		}
		return targets
	}
	centerX, centerY := primary.X, primary.Y
	if skill.TargetType == 5 {
		centerX, centerY = caster.X, caster.Y
	}
	candidates := w.nearbyWorldPlayers(centerX, centerY, area)
	sort.Slice(candidates, func(i, j int) bool {
		di := chebyshev(centerX, centerY, candidates[i].X, candidates[i].Y)
		dj := chebyshev(centerX, centerY, candidates[j].X, candidates[j].Y)
		if di != dj {
			return di < dj
		}
		return candidates[i].ID < candidates[j].ID
	})
	for _, candidate := range candidates {
		if len(targets) >= limit {
			break
		}
		if candidate == caster || candidate == primary || !candidate.InWorld || candidate.Char == nil ||
			playerCurHP(candidate.Char) == 0 || sameSupportGroup(caster, candidate) ||
			!w.playersShareGameplaySpace(caster, candidate) {
			continue
		}
		losX, losY := primary.X, primary.Y
		if skill.TargetType == 5 {
			losX, losY = caster.X, caster.Y
		}
		if skillAreaContains(caster.X, caster.Y, primary.X, primary.Y, candidate.X, candidate.Y, skill, area) &&
			w.combatLineOfSight(losX, losY, candidate.X, candidate.Y) {
			targets = append(targets, candidate)
		}
	}
	return targets
}

func playerElementalResists(ch *model.Char) model.ElementalResists {
	if ch == nil {
		return model.ElementalResists{}
	}
	return model.ElementalResists{
		Fire:    effectiveScore(ch).ResistFire,
		Ice:     effectiveScore(ch).ResistIce,
		Sacred:  effectiveScore(ch).ResistHoly,
		Thunder: effectiveScore(ch).ResistThunder,
	}
}

func setOwnedAffect(ch *model.Char, ownerID uint16, affectType byte, value, level, durationUnits int) bool {
	return setOwnedAffectAt(ch, ownerID, affectType, value, level, durationUnits, time.Now())
}

func setOwnedAffectAt(ch *model.Char, ownerID uint16, affectType byte, value, level, durationUnits int, now time.Time) bool {
	if !setAffectAt(ch, affectType, value, level, durationUnits, now) {
		return false
	}
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectType {
			ch.Affects[i].OwnerID = ownerID
			return true
		}
	}
	return false
}

func setOwnedAffectForPlayerAt(ch *model.Char, owner *Player, affectType byte, value, level, durationUnits int, now time.Time) bool {
	if owner == nil || owner.Char == nil || strings.TrimSpace(owner.Char.UID) == "" {
		return false
	}
	if !setOwnedAffectAt(ch, owner.ID, affectType, value, level, durationUnits, now) {
		return false
	}
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectType {
			ch.Affects[i].OwnerCharacterUID = strings.TrimSpace(owner.Char.UID)
			return true
		}
	}
	return false
}

func removePlayerAffectTypes(ch *model.Char, types ...byte) bool {
	changed := false
	for i := range ch.Affects {
		for _, affectType := range types {
			if ch.Affects[i].Type == affectType {
				ch.Affects[i] = model.Affect{}
				changed = true
				break
			}
		}
	}
	return changed
}

func (w *World) breakHideOnAttack(p *Player) {
	if p == nil || p.Char == nil || !removePlayerAffectTypes(p.Char, 28) {
		return
	}
	w.recalcPlayer(p.Char)
	w.publishPlayerAffects(p)
	w.sendToPlayerView(p, func() []byte { return playerAppearancePacket(p) })
}

func (w *World) executePlayerSkill(caster *Player, targets []*Player, skill model.SkillDef, mastery int, motion byte) {
	now := w.now()
	baseDamage := w.baseSkillDamage(caster.Char, skill)
	if skill.Index == 30 {
		baseDamage += int(playerCurHP(caster.Char))
		setPlayerCurHP(caster.Char, playerCurHP(caster.Char)/6+1)
	}
	if skill.Index == 22 {
		baseDamage = explosionBashBaseDamage(baseDamage, playerInt(caster.Char), int(playerCurMP(caster.Char)))
		setPlayerCurMP(caster.Char, 0)
		caster.X, caster.Y = w.findFreeGameplayPosition(caster, caster, targets[0].X, targets[0].Y, 2)
		caster.Char.X, caster.Char.Y = caster.X, caster.Y
		w.refreshPlayerVisibility(caster)
	}
	directDamage := skill.InstanceType >= 1 && skill.InstanceType <= 5
	wireTargets := make([]wire.SkillTarget, 0, len(targets))
	type wideHit struct {
		target *Player
		damage uint32
	}
	wideHits := make([]wideHit, 0, len(targets)*skillHitCount(skill))
	killedPlayers := make([]*Player, 0, len(targets))
	hitCount := skillHitCount(skill)
	for _, target := range targets {
		if !combatRollHits(playerVersusPlayerAccuracy(caster.Char, target.Char), w.intn) {
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID, Miss: true,
				MaxHP: playerMaxHP(target.Char)})
			continue
		}
		target.LastAttackerID = caster.ID
		if directDamage {
			w.cancelTrade(target, "personagem foi atacado")
		}
		blocked := hasActiveAffectAt(target.Char, 19, now) // Imunidade bloqueia affects agressivos.
		if directDamage {
			damageValue := w.skillFinalDamage(baseDamage, playerDefense(target.Char), skillDamageMastery(caster.Char))
			damageValue = applySkillResistance(damageValue, skill.InstanceType, playerElementalResists(target.Char), false)
			perHit := uint32(clampInt(damageValue, 1, int(maxScoreValue)))
			perHit = addFlatDamage(perHit, w.equipmentGemBonuses(caster.Char).forceDamage)
			perHit = absorbFlatDamage(perHit, w.equipmentGemBonuses(target.Char).absorbDamage)
			for hit := 0; hit < hitCount && playerCurHP(target.Char) > 0; hit++ {
				// Montaria adulta viva absorve 25% de cada golpe no proprio HP.
				riderHit := uint32(w.absorbMountDamage(target, int(perHit)))
				applied := riderHit
				if applied > playerCurHP(target.Char) {
					applied = playerCurHP(target.Char)
				}
				setPlayerCurHP(target.Char, playerCurHP(target.Char)-applied)
				wideHits = append(wideHits, wideHit{target: target, damage: riderHit})
			}
			// O pacote compacto so inicia o efeito; o 0x39D estendido abaixo
			// carrega o dano calculado, inclusive overkill acima do HP restante.
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID})
			if playerCurHP(target.Char) == 0 {
				w.mountRiderDied(target)
				target.DeadAt = now
				w.receiveDeathLetter(target, caster.Char.Name, "jogador")
				killedPlayers = append(killedPlayers, target)
			}
		} else {
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID})
		}
		if !blocked && skill.AffectType > 0 {
			setOwnedAffectForPlayerAt(target.Char, caster, byte(skill.AffectType), skill.AffectValue, mastery, skill.AffectTime, now)
		}
		if !blocked && skill.TickType > 0 {
			setOwnedAffectForPlayerAt(target.Char, caster, byte(skill.TickType), skill.TickValue, mastery, skill.AffectTime, now)
		}
		if skill.Index == 49 { // Chamas Etereas: queima mana e dissipa buffs defensivos.
			setPlayerCurMP(target.Char, 0)
			removePlayerAffectTypes(target.Char, 14, 16, 18, 19, 32)
		}
		if skill.Index == 6 { // Furia Divina puxa o jogador.
			target.X, target.Y = w.findFreeGameplayPosition(caster, target, caster.X, caster.Y, 2)
			target.Char.X, target.Char.Y = target.X, target.Y
			w.refreshPlayerVisibility(target)
			w.sendToPlayerView(target, func() []byte { return wire.ActionStop(target.ID, target.X, target.Y) })
		}
	}
	w.applyPvPKills(caster, killedPlayers...)
	primary := targets[0]
	w.sendToPlayerView(primary, func() []byte {
		return spectralPacket(caster.Char, wire.SkillHits(caster.ID, caster.X, caster.Y, primary.X, primary.Y,
			caster.Char.Exp, playerCombatMP(caster.Char), int16(skill.Index), motion,
			skillVisualLevel(mastery), offensiveSkillWireMaxTargets(caster.Char, skill), wireTargets))
	})
	for _, hit := range wideHits {
		hit := hit
		w.sendToPlayerView(hit.target, func() []byte {
			return spectralPacket(caster.Char, wire.SkillHitExtended(caster.ID, hit.target.ID, caster.X, caster.Y, hit.target.X, hit.target.Y,
				hit.damage, playerMaxHP(hit.target.Char), caster.Char.Exp, playerCombatMP(caster.Char), int16(skill.Index), motion, skillVisualLevel(mastery)))
		})
	}
	// O ataque precisa chegar antes de SetHpMp/CNFMobKill. Se HP=0 for
	// publicado primeiro, o 7.48 considera o alvo morto e descarta o numero
	// flutuante, sobretudo no overkill.
	for _, target := range targets {
		dead := playerCurHP(target.Char) == 0
		w.recalcPlayer(target.Char)
		if dead {
			setPlayerCurHP(target.Char, 0)
		}
		w.syncPlayerVitals(target)
		w.updatePartyMember(target)
		w.publishPlayerAffects(target)
		w.sendToPlayerView(target, func() []byte { return playerAppearancePacket(target) })
	}
	w.syncPlayerScoreAndVitals(caster)
	for _, target := range targets {
		if playerCurHP(target.Char) == 0 {
			w.publishPlayerDeath(target, caster.ID)
		}
	}
	log.Printf("[#%d] executou PvP skill=%d %q alvos=%d", caster.Session.ID, skill.Index, skill.Name, len(targets))
}
