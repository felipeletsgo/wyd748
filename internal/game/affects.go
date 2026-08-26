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
	// SetAffect nativo preserva tanto a potencia quanto o tempo restante. Um
	// recast mais fraco ou mais curto nao pode reduzir um buff/debuff existente.
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
			if !canReplaceAffect(ch.Affects[i], value, level, expires, now) {
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

// setFaceAffect grava a transformacao de rosto (volatiles 70-77) DIRETO, sempre
// substituindo a atual. Nao usa canReplaceAffect: aquela compara potencia
// (Value), o que aqui e um mesh -- trocar de Gremlin(202) para Troll(212) e
// valido nas duas direcoes. Value carrega o mesh; bodyMesh o aplica.
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
	ch.Affects[slot] = model.Affect{Type: affectFaceTransform, ClientType: affectFaceTransform,
		Value: mesh, ExpiresAt: expires, NextTick: now.Add(8 * time.Second)}
	return true
}

// removeFaceAffect tira a transformacao de rosto; devolve se havia alguma ativa.
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

// activePlayerAffect devolve o affect ativo do tipo pedido, ou nil. Espelha o
// activeMobAffect usado pelos mobs.
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

// accumulateAffect SOMA tempo a um affect existente (ou cria um novo), com teto.
// Porta o padrao do frango assado e do bau de EXP do W2PP, que fazem
// Affect.Time += X ate um limite e recusam "usar mais" quando ja no teto. addUnits
// e maxUnits sao blocos de 8 s. Retorna false (sem consumir) quando ja saturado.
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
				return false // ja no teto: "nao pode usar/comer mais"
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

// applyVolatileBuff aplica o pacote inteiro ou nada. Chocolate/Candy ocupam
// quatro slots simultaneos; aceitar apenas parte deles deixaria um estado que
// nunca existiu no handler nativo. O resultado distingue uma recusa causada por
// affect ativo de erro de configuracao/falta de slot, permitindo feedback fiel
// sem consumir o item.
func (w *World) applyVolatileBuff(ch *model.Char, rule model.VolatileRule) volatileBuffResult {
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
	}
	return volatileBuffApplied
}

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

func (w *World) supportTargets(p *Player, req skillCastRequest, skill model.SkillDef) []*Player {
	if p == nil || p.Char == nil {
		return nil
	}
	// An explicit target is an intent, not a hint. Never silently turn a stale,
	// out-of-range or cross-runtime target into a self cast.
	if req.TargetID != 0 && req.TargetID != p.ID {
		target := w.playerByID(req.TargetID)
		if target == nil || !target.InWorld || target.Char == nil ||
			!w.playersShareGameplaySpace(p, target) ||
			(!sameSupportGroup(p, target) && skill.Index != 47) ||
			chebyshev(p.X, p.Y, target.X, target.Y) > maxInt(6, skill.Range) {
			return nil
		}
	}
	if skill.Party != 0 && skill.MaxTarget > 1 && p.Party != nil {
		result := make([]*Player, 0, len(p.Party.Members))
		for _, member := range p.Party.Members {
			if member != nil && member.InWorld && member.Char != nil &&
				w.playersShareGameplaySpace(p, member) &&
				chebyshev(p.X, p.Y, member.X, member.Y) <= maxInt(6, skill.Range) {
				result = append(result, member)
			}
		}
		if len(result) > 0 {
			return result
		}
	}
	if target := w.playerByID(req.TargetID); target != nil && target.InWorld &&
		target.Char != nil && w.playersShareGameplaySpace(p, target) &&
		(sameSupportGroup(p, target) || skill.Index == 47) &&
		chebyshev(p.X, p.Y, target.X, target.Y) <= maxInt(6, skill.Range) {
		return []*Player{target}
	}
	if req.TargetID == 0 || req.TargetID == p.ID {
		return []*Player{p}
	}
	return nil
}

func cleansePlayer(ch *model.Char) bool {
	changed := false
	for i := range ch.Affects {
		switch ch.Affects[i].Type {
		case 1, 3, 5, 7, 10, 12, 20, 22:
			ch.Affects[i] = model.Affect{}
			changed = true
		}
	}
	return changed
}

// applySupportSkill porta o caminho nao-agressivo de _MSG_Attack da W2PP.
// O client escolhe/mostra o alvo, mas o servidor valida grupo, alcance e valores.
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
	// Personagens deste emulador ainda sao Mortais. A W2PP/7.59 aplica 140%
	// e limita Cura/Recuperar a 548 HP nessa classe de personagem.
	return minInt(548, heal*14/10)
}

func (w *World) applySupportSkill(p *Player, req skillCastRequest, skill model.SkillDef, mastery int) []supportSkillResult {
	now := w.now()
	targets := w.supportTargets(p, req, skill)
	changed := make([]supportSkillResult, 0, len(targets))
	for _, target := range targets {
		if playerCurHP(target.Char) == 0 && skill.Index != 31 && skill.Index != 99 {
			continue
		}
		oldMaxHP := playerMaxHP(target.Char)
		oldHP := playerCurHP(target.Char)
		applied := false
		switch skill.Index {
		case 25: // Desintoxicar
			applied = cleansePlayer(target.Char)
		case 27, 29: // Cura / Recuperar
			heal := foemaHealAmount(skill.Index, mastery, skill.InstanceValue)
			restorePlayerHP(target.Char, uint32(heal))
			applied = true
		case 31: // Renascimento Foema.
			setPlayerCurMP(p.Char, 0)
			if playerCurHP(target.Char) == 0 && w.intn(2) == 0 {
				setPlayerCurHP(target.Char, playerMaxHP(target.Char))
				setPlayerCurMP(target.Char, playerMaxMP(target.Char))
				target.DeadAt = time.Time{}
				applied = true
			}
		case 99: // Ressurreicao Sephira: somente autocast do morto (Secrets 7.54).
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
		case 42: // Teleporte: traz o membro selecionado para junto do caster.
			if target != p {
				target.X, target.Y = w.findFreeGameplayPosition(p, target, p.X, p.Y, 4)
				target.Char.X, target.Char.Y = target.X, target.Y
				w.refreshPlayerVisibility(target)
				w.sendToPlayerView(target, func() []byte {
					return wire.ActionStop(target.ID, target.X, target.Y)
				})
			}
			applied = true
		case 47: // Cancelamento remove Imunidade (Affect 19).
			for i := range target.Char.Affects {
				if target.Char.Affects[i].Type == 19 {
					target.Char.Affects[i] = model.Affect{}
					applied = true
				}
			}
		case 56, 57, 58, 59, 60, 61, 62, 63:
			applied = w.castSummon(p, skill, mastery)
		default:
			affectType, value, ok := skillAffect(skill)
			if ok {
				// SkillData.csv preserva o identificador cru 50 da Armadura Critica,
				// mas a regra autoritativa usa affect 31 e o executavel 7.48 exige o
				// visual 24. Normalize antes de persistir para não perder os stats.
				if skill.Index == 15 {
					affectType = 31
				}
				applied = setAffectAt(target.Char, affectType, value, mastery, skill.AffectTime, now)
				if applied && skill.Index == 15 { // Critical Armor TK usa o visual 24 no 7.48.
					for i := range target.Char.Affects {
						if target.Char.Affects[i].Type == affectType {
							target.Char.Affects[i].ClientType = 24
							break
						}
					}
				}
			}
		}
		if !applied {
			continue
		}
		// Cura e Recuperar alteram somente o HP corrente. Recalcular todo o score
		// aqui fazia um simples heal reaplicar/migrar stats e pontos do personagem.
		if skill.Index != 27 && skill.Index != 29 {
			w.recalcPlayer(target.Char)
		}
		if skill.Index == 75 { // Invisibilidade remove o jogador da lista de aggro.
			// Somente mobs acordados podem possuir alvo. Evite um scan global de
			// milhares de NPCs para uma skill disparada pelo jogador.
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
		log.Printf("[#%d] suporte %q afetou=%d mastery=%d", p.Session.ID, skill.Name, len(changed), mastery)
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

// applyAffectStats recalcula sempre a partir do score limpo; renovar/remover um
// buff nunca acumula bonus permanentemente.
func (w *World) applyAffectStats(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	w.applyExtendedAffectStats(ch)
}

// applyExtendedAffectStats aplica os buffs sobre uma copia runtime do sidecar.
// A base persistida nunca e modificada, portanto renovar/recalcular um buff nao
// acumula bonus. Os calculos usam int64 para nao estourar antes do clamp.
func (w *World) applyExtendedAffectStats(ch *model.Char) {
	if ch == nil || ch.Score == nil {
		return
	}
	if ch.RuntimeScore == nil {
		applyScore(ch)
	}
	e := ch.RuntimeScore
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
	for i := range ch.Affects {
		a := &ch.Affects[i]
		if a.Type == 0 || !a.ExpiresAt.After(now) {
			continue
		}
		switch a.Type {
		case 1: // Lentidao
			run := maxInt(1, int(e.AttackRun&0x0F)-a.Value)
			e.AttackRun = e.AttackRun&0xF0 | uint32(run)
		case 2: // Velocidade
			run := minInt(15, int(e.AttackRun&0x0F)+a.Value)
			e.AttackRun = e.AttackRun&0xF0 | uint32(run)
		case 3: // Perseguicao
			e.ResistFire = uint32(maxInt(0, int(e.ResistFire)-a.Value))
			e.ResistIce = uint32(maxInt(0, int(e.ResistIce)-a.Value))
			e.ResistHoly = uint32(maxInt(0, int(e.ResistHoly)-a.Value))
			e.ResistThunder = uint32(maxInt(0, int(e.ResistThunder)-a.Value))
		case 4: // Buff de dano (Kappa/Competente/Mental, comidas, Coragem...).
			// O W2PP (Basedef.cpp:3900) da um bonus fixo; o felipe optou por
			// ESCALAR por tier. Value = % de bonus aplicado a ataque fisico e
			// magico, configurado por item/codigo em volatiles.json.
			e.Attack = mul(e.Attack, 100+a.Value)
			e.MagicAttack = mul(e.MagicAttack, 100+a.Value)
		case 5:
			e.Dex = mul(e.Dex, 100-clampInt(a.Value, 0, 100))
		case 7:
			penalty := a.Level/10 + 20
			attackSpeed := maxInt(0, int(e.AttackRun>>4)-penalty)
			e.AttackRun = uint32(attackSpeed<<4) | e.AttackRun&0x0F
			e.Int = add(e.Int, int64(-penalty))
		case 9:
			e.Attack = add(e.Attack, int64((a.Level*5/20+a.Value)*3/2))
		case 10:
			e.Attack = add(e.Attack, int64(-(a.Level/5 + a.Value)))
		case 11:
			e.Defense = add(e.Defense, int64(a.Level/3+a.Value))
		case 12:
			e.Defense = mul(e.Defense, 100-clampInt(a.Value, 0, 100))
		case 13:
			e.Attack = mul(e.Attack, 100+a.Level/10+a.Value)
			e.Defense = mul(e.Defense, 90)
		case 14: // Possuido
			value := a.Level*3/4 + a.Value
			e.Con = add(e.Con, int64(value))
			e.MaxHP = add(e.MaxHP, int64(value)*8)
		case 15: // Toque de Athena
			value := uint32(maxInt(0, a.Level/10+a.Value))
			for j := range e.Mastery {
				e.Mastery[j] = minU32(maxScoreValue, e.Mastery[j]+value)
			}
		case 16: // Transformacoes BM; pTransBonus da W2PP
			type transformBonus struct {
				minDamage, maxDamage, minDefense, maxDefense int
				minHP, maxHP, run, attackSpeed               int
			}
			bonuses := [...]transformBonus{
				{110, 130, 95, 105, 95, 105, 1, 20},
				{80, 100, 100, 110, 110, 140, 0, 0},
				{100, 120, 105, 115, 100, 120, 1, 20},
				{90, 110, 110, 125, 105, 110, 0, 20},
				{105, 120, 110, 120, 105, 115, 3, 20},
			}
			idx := a.Value - 1
			if idx >= 0 && idx < len(bonuses) {
				b := bonuses[idx]
				damageAdd, defenseAdd, hpAdd := 0, 0, 0
				resistAdd, attackSpeedAdd := 0, 0
				switch idx {
				case 0:
					if learnedLocal(ch, 65-48) {
						damageAdd = 50
						e.Critical = clampScoreValue(e.Critical + 11)
					}
				case 1:
					if learnedLocal(ch, 67-48) {
						hpAdd, resistAdd, attackSpeedAdd = 30, 20, 20
					}
				case 2:
					if learnedLocal(ch, 69-48) {
						damageAdd, defenseAdd, hpAdd = 30, 3, 5
						resistAdd, attackSpeedAdd = 20, 15
					}
				case 3:
					defenseAdd = 10
				case 4:
					damageAdd, hpAdd, resistAdd, attackSpeedAdd = 40, 17, 5, 17
					e.Critical = clampScoreValue(e.Critical + 5)
				}
				interpolate := func(minimum, maximum int) int {
					return minimum + (maximum-minimum)*a.Level/200
				}
				e.Attack = mul(e.Attack, interpolate(b.minDamage+damageAdd, b.maxDamage+damageAdd))
				e.Defense = mul(e.Defense, interpolate(b.minDefense+defenseAdd, b.maxDefense+defenseAdd))
				e.MaxHP = mul(e.MaxHP, interpolate(b.minHP+hpAdd, b.maxHP+hpAdd))
				e.ResistFire = uint32(clampInt(int(e.ResistFire)+resistAdd, 0, 100))
				e.ResistIce = uint32(clampInt(int(e.ResistIce)+resistAdd, 0, 100))
				e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+resistAdd, 0, 100))
				e.ResistThunder = uint32(clampInt(int(e.ResistThunder)+resistAdd, 0, 100))
				run := minInt(15, int(e.AttackRun&0x0F)+b.run)
				attackSpeed := minInt(15, int(e.AttackRun>>4)+(b.attackSpeed+attackSpeedAdd)/10)
				e.AttackRun = uint32(attackSpeed<<4 | run)
			}
		case 18: // Controle de Mana
			e.SaveMana = uint32(minInt(99, int(e.SaveMana)+a.Level/10+a.Value))
		case 21:
			e.Defense = add(e.Defense, int64(-(a.Level/3 + 10)))
			e.Attack = mul(e.Attack, 100+a.Level/10+a.Value)
		case 24:
			e.Defense = add(e.Defense, int64(e.Defense)/4+int64(a.Value))
		case 25:
			e.Defense = add(e.Defense, int64(a.Value+a.Level+int(playerMastery(ch, 1))+1))
			resist := (a.Value + a.Level/4) / 10
			e.ResistFire = uint32(clampInt(int(e.ResistFire)+resist, 0, 100))
			e.ResistIce = uint32(clampInt(int(e.ResistIce)+resist, 0, 100))
			e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+resist, 0, 100))
			e.ResistThunder = uint32(clampInt(int(e.ResistThunder)+resist, 0, 100))
		case 26:
			e.Evasion = add(e.Evasion, int64(a.Level+a.Value*10))
		case 29: // Limite da Alma Celestial/SubCelestial.
			// O Secrets calcula os percentuais sobre o bStatus cru, nao sobre
			// equipamentos nem sobre o resultado de outro buff. SoulInfo e
			// compartilhado pelas duas formas; o affect em si e por forma.
			bonuses := [...][4]int{
				{120, 0, 0, 0}, {0, 120, 0, 0},
				{0, 0, 120, 0}, {0, 0, 0, 120},
				{80, 0, 0, 40}, {0, 80, 0, 40},
				{0, 0, 80, 40}, {40, 0, 0, 80},
				{0, 40, 0, 80}, {0, 0, 40, 80},
			}
			config := int(ch.SoulInfo) - 1
			if isCelestialEvolution(ch) && config >= 0 && config < len(bonuses) {
				raw := ch.Score
				strAdd := int64(raw.Str/100) * int64(bonuses[config][0])
				intAdd := int64(raw.Int/100) * int64(bonuses[config][1])
				dexAdd := int64(raw.Dex/100) * int64(bonuses[config][2])
				conAdd := int64(raw.Con/100) * int64(bonuses[config][3])
				e.Str = add(e.Str, strAdd)
				e.Int = add(e.Int, intAdd)
				e.Dex = add(e.Dex, dexAdd)
				e.Con = add(e.Con, conAdd)
				e.Attack = add(e.Attack, strAdd/3)
				e.MaxHP = add(e.MaxHP, conAdd*2)
				e.MaxMP = add(e.MaxMP, intAdd*2)
			}
		case affectCourage:
			// Courage nao modifica score. O bonus fixo pertence ao resultado de
			// cada hit PvE e e aplicado em combat.go/skills.go.
		case 31:
			e.Defense = add(e.Defense, int64(a.Level/2+a.Value))
		case 35: // Bigger/Health Potion: bonus nativo de 10% no HP maximo.
			bonus := a.Value
			if bonus <= 0 {
				bonus = 10
			}
			e.MaxHP = mul(e.MaxHP, 100+bonus)
		case 37:
			e.Attack = add(e.Attack, int64(playerMastery(ch, 2)))
		case 38:
			mana := e.MaxMP / 2
			e.MaxHP = add(e.MaxHP, int64(mana))
			e.MaxMP -= mana
		}
	}
	if e.CurHP > e.MaxHP {
		e.CurHP = e.MaxHP
		ch.Score.CurHP = e.CurHP
	}
	if e.CurMP > e.MaxMP {
		e.CurMP = e.MaxMP
		ch.Score.CurMP = e.CurMP
	}
	normalizeRuntimeScore(ch)
}

func (w *World) tickMobAffects(now time.Time, shard, shardCount int) {
	accountsToSave := make(map[*model.Account]struct{})
	killPlans := make([]*killRewardPlan, 0)
	for _, m := range w.activeMobs {
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
			a.NextTick = now.Add(8 * time.Second)
			if a.Type != 20 { // veneno/sangramento
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
		w.commitKillRewardBatch(nil, killPlans, accounts, "mortes por affect")
	}
}

func (w *World) tickPlayerAffects(now time.Time) {
	for _, p := range w.players {
		if !p.InWorld || p.Char == nil {
			continue
		}
		expired, hpChanged := false, false
		for i := range p.Char.Affects {
			a := &p.Char.Affects[i]
			// All player-owned hostile affects share the same runtime boundary,
			// not only the DoT type. Resolve by stable UID whenever available so
			// a recycled ClientID cannot keep a stale debuff alive.
			if affectHasPlayerOwner(a) {
				owner := w.affectOwnerPlayer(a)
				if owner == nil || !w.playersShareGameplaySpace(owner, p) {
					*a = model.Affect{}
					expired = true
					continue
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
				skillIndex := 37
				if a.Type == 23 {
					skillIndex = 54
				}
				w.tickAreaDamageAffect(p, a, skillIndex)
				a.NextTick = now.Add(8 * time.Second)
			}
			if a.Type == 20 && a.ExpiresAt.After(now) && !now.Before(a.NextTick) {
				// A DoT may outlive a teleport or an instance transition. Never
				// let an affect created in one private gameplay space damage a
				// character in another; remove the stale cross-runtime affect.
				owner := w.affectOwnerPlayer(a)
				currentHP := playerCurHP(p.Char)
				damage := uint32(clampInt(a.Level/2+a.Value, 1, maxInt(1, int(currentHP)-1)))
				if currentHP > 1 {
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
		}
	}
}

func (w *World) tickAreaDamageAffect(p *Player, affect *model.Affect, skillIndex int) {
	if p == nil || p.Char == nil || affect == nil {
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
	for _, m := range targets {
		damage := uint32(clampInt(affect.Level+affect.Value, 1, int(maxScoreValue)))
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
				// Area effects must not collapse source-client resources to WORDs.
				return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP,
					m.Def.Score.MaxMP, m.Def.Score.MaxMP)
			})
		}
	}
	w.commitKillRewardBatch(p, killPlans, batchAccounts, "mortes por affect de area")
}

func tickAreaVisual(skillIndex int) (int, byte) {
	if skillIndex == 37 {
		// A Lightning Storm 7.59 e um tick que reproduz visualmente Thunderbolt
		// (skill 33), exatamente como o ProcessAffect Type 22 da W2PP.
		return 33, 254
	}
	return skillIndex, 254
}
