package game

import (
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func hasActiveAffect(ch *model.Char, affectType byte) bool {
	if ch == nil {
		return false
	}
	now := time.Now()
	for i := range ch.Affects {
		if ch.Affects[i].Type == affectType && ch.Affects[i].ExpiresAt.After(now) {
			return true
		}
	}
	return false
}

func (w *World) skillPlayerTargets(caster *Player, req skillCastRequest, skill model.SkillDef) []*Player {
	primary := w.playerByID(req.TargetID)
	castRange := maxInt(attackRange, skill.Range)
	if primary == nil || primary == caster || !primary.InWorld || primary.Char == nil ||
		playerCurHP(primary.Char) == 0 || sameSupportGroup(caster, primary) ||
		chebyshev(caster.X, caster.Y, primary.X, primary.Y) > castRange {
		return nil
	}
	targets := []*Player{primary}
	limit := clampInt(skill.MaxTarget, 1, 13)
	if skill.Index == 95 {
		limit = 1
	}
	if limit == 1 {
		return targets
	}
	for _, candidate := range w.players {
		if len(targets) >= limit {
			break
		}
		if candidate == caster || candidate == primary || !candidate.InWorld || candidate.Char == nil ||
			playerCurHP(candidate.Char) == 0 || sameSupportGroup(caster, candidate) {
			continue
		}
		if chebyshev(primary.X, primary.Y, candidate.X, candidate.Y) <= maxInt(1, skill.Range) {
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
		Fire:    effectiveExtended(ch).ResistFire,
		Ice:     effectiveExtended(ch).ResistIce,
		Sacred:  effectiveExtended(ch).ResistHoly,
		Thunder: effectiveExtended(ch).ResistThunder,
	}
}

func setOwnedAffect(ch *model.Char, ownerID uint16, affectType byte, value, level, durationUnits int) bool {
	if !setAffect(ch, affectType, value, level, durationUnits) {
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
	w.sendToPlayerView(p, func() []byte { return wire.VisualEquip(p.ID, bodyMesh(p.Char)) })
}

func (w *World) executePlayerSkill(caster *Player, targets []*Player, skill model.SkillDef, mastery int, motion byte) {
	baseDamage := w.baseSkillDamage(caster.Char, skill)
	if skill.Index == 30 {
		baseDamage += int(playerCurHP(caster.Char))
		setPlayerCurHP(caster.Char, playerCurHP(caster.Char)/6+1)
	}
	if skill.Index == 22 {
		baseDamage = explosionBashBaseDamage(baseDamage, playerInt(caster.Char), int(playerCurMP(caster.Char)))
		setPlayerCurMP(caster.Char, 0)
		caster.X, caster.Y = w.findFreePlayerPosition(targets[0].X, targets[0].Y, 2, caster)
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
	hitCount := skillHitCount(skill)
	for _, target := range targets {
		target.LastAttackerID = caster.ID
		if directDamage {
			w.cancelTrade(target, "personagem foi atacado")
		}
		blocked := hasActiveAffect(target.Char, 19) // Imunidade bloqueia affects agressivos.
		if directDamage {
			damageValue := skillFinalDamage(baseDamage, playerDefense(target.Char), skillDamageMastery(caster.Char))
			damageValue = applySkillResistance(damageValue, skill.InstanceType, playerElementalResists(target.Char), false)
			perHit := uint32(clampInt(damageValue, 1, int(maxExtendedStat)))
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
				target.DeadAt = time.Now()
				w.receiveDeathLetter(target, caster.Char.Name, "jogador")
			}
		} else {
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID})
		}
		if !blocked && skill.AffectType > 0 {
			setOwnedAffect(target.Char, caster.ID, byte(skill.AffectType), skill.AffectValue, mastery, skill.AffectTime)
		}
		if !blocked && skill.TickType > 0 {
			setOwnedAffect(target.Char, caster.ID, byte(skill.TickType), skill.TickValue, mastery, skill.AffectTime)
		}
		if skill.Index == 49 { // Chamas Etereas: queima mana e dissipa buffs defensivos.
			setPlayerCurMP(target.Char, 0)
			removePlayerAffectTypes(target.Char, 14, 16, 18, 19, 32)
		}
		if skill.Index == 6 { // Furia Divina puxa o jogador.
			target.X, target.Y = w.findFreePlayerPosition(caster.X, caster.Y, 2, target)
			target.Char.X, target.Char.Y = target.X, target.Y
			w.refreshPlayerVisibility(target)
			w.sendToPlayerView(target, func() []byte { return wire.ActionStop(target.ID, target.X, target.Y) })
		}
	}
	primary := targets[0]
	w.sendToPlayerView(primary, func() []byte {
		return spectralPacket(caster.Char, wire.SkillHits(caster.ID, caster.X, caster.Y, primary.X, primary.Y,
			caster.Char.Exp, playerCombatMP(caster.Char), int16(skill.Index), motion,
			skillVisualLevel(mastery), skill.MaxTarget, wireTargets))
	})
	for _, hit := range wideHits {
		hit := hit
		w.sendToPlayerView(hit.target, func() []byte {
			return spectralPacket(caster.Char, wire.SkillHitExtended(caster.ID, hit.target.ID, caster.X, caster.Y, hit.target.X, hit.target.Y,
				hit.damage, caster.Char.Exp, playerCombatMP(caster.Char), int16(skill.Index), motion, skillVisualLevel(mastery)))
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
		w.sendToPlayerView(target, func() []byte { return wire.VisualEquip(target.ID, bodyMesh(target.Char)) })
	}
	w.syncPlayerScoreAndVitals(caster)
	for _, target := range targets {
		if playerCurHP(target.Char) == 0 {
			w.sendToPlayerView(target, func() []byte {
				return wire.CNFMobKill(target.ID, caster.ID, target.Char.Exp)
			})
		}
	}
	log.Printf("[#%d] executou PvP skill=%d %q alvos=%d", caster.Session.ID, skill.Index, skill.Name, len(targets))
}
