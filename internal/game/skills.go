package game

import (
	"encoding/binary"
	"log"
	"math/rand"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const ultimateSkillGold = 2_000_000

func (w *World) onLearnSkill(s *net.Session, p *Player, itemIndex int) {
	w.onLearnSkillAtMaster(s, p, itemIndex, 0)
}

// onLearnSkillAtMaster valida a requisicao de aprendizado tanto da loja
// legada (0x379) quanto do fluxo real do client 7.48 (ApplyBonus tipo 2).
// O TargetID recebido nunca escolhe o mestre: apenas deve confirmar a loja
// que o servidor ja marcou como aberta para este jogador.
func (w *World) onLearnSkillAtMaster(s *net.Session, p *Player, itemIndex int, requestedMasterID uint16) {
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	if requestedMasterID != 0 && requestedMasterID != p.ShopNPC {
		log.Printf("[#%d] skill mestre invalido recebido=%d aberto=%d", s.ID, requestedMasterID, p.ShopNPC)
		return
	}
	master := w.mobByID(p.ShopNPC)
	if master == nil || master.Def.Extended == nil || master.Def.Extended.Merchant != skillMasterMerchant {
		log.Printf("[#%d] aprender skill sem mestre aberto", s.ID)
		return
	}
	found := false
	for _, item := range master.Def.Vende {
		if int(item.Index) == itemIndex {
			found = true
			break
		}
	}
	if !found || itemIndex < 5000 || itemIndex > 5095 {
		log.Printf("[#%d] skill item=%d nao pertence ao mestre aberto", s.ID, itemIndex)
		return
	}
	globalSkill := itemIndex - 5000
	skillClass, localSkill := globalSkill/24, globalSkill%24
	if skillClass != int(p.Char.Class) {
		log.Printf("[#%d] tentou aprender skill de outra classe: item=%d", s.ID, itemIndex)
		return
	}
	skill, ok := w.skills[globalSkill]
	itemDef, itemOK := w.items[uint16(itemIndex)]
	if !ok || !itemOK {
		log.Printf("[#%d] skill %d ausente do catalogo", s.ID, globalSkill)
		return
	}
	bit := uint32(1) << localSkill
	if p.Char.LearnedSkill&bit != 0 {
		return
	}
	w.recalcPlayer(p.Char)
	if int(playerSkillPoints(p.Char)) < skill.SkillPoint || int(playerLevel(p.Char)) < itemDef.ReqLevel ||
		int(playerMastery(p.Char, 1)) < itemDef.ReqInt ||
		int(playerMastery(p.Char, 2)) < itemDef.ReqDex ||
		int(playerMastery(p.Char, 3)) < itemDef.ReqCon {
		log.Printf("[#%d] requisitos insuficientes para skill=%d pts=%d/%d level=%d/%d mastery=%v",
			s.ID, globalSkill, playerSkillPoints(p.Char), skill.SkillPoint,
			playerLevel(p.Char), itemDef.ReqLevel, effectiveExtended(p.Char).Mastery)
		return
	}
	oldGold, oldLearned := p.Char.Gold, p.Char.LearnedSkill
	if localSkill == 7 || localSkill == 15 || localSkill == 23 {
		if p.Char.LearnedSkill&((1<<7)|(1<<15)|(1<<23)) != 0 {
			return
		}
		branchStart := localSkill - 7
		for i := branchStart; i < localSkill; i++ {
			if p.Char.LearnedSkill&(uint32(1)<<i) == 0 {
				return
			}
		}
		if p.Char.Gold < ultimateSkillGold {
			return
		}
		p.Char.Gold -= ultimateSkillGold
	}
	p.Char.LearnedSkill |= bit
	w.recalcPlayer(p.Char)
	filterShortSkills(p.Char)
	// Persist-before-confirm: aprender skill deduz gold (skill final) e altera a
	// mascara persistida; grava ANTES de confirmar ao client e reverte gold+skill
	// se o disco falhar, como refino/trade/quest.
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Gold, p.Char.LearnedSkill = oldGold, oldLearned
		w.recalcPlayer(p.Char)
		filterShortSkills(p.Char)
		log.Printf("[#%d] salvar skill aprendida: %v", s.ID, err)
		return
	}
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
	log.Printf("[#%d] aprendeu skill=%d %q custo=%d pontos restantes=%d", s.ID,
		globalSkill, skill.Name, skill.SkillPoint, playerSkillPoints(p.Char))
}

type skillCastRequest struct {
	Skill    int
	Motion   byte
	TargetID uint16
	TargetX  uint16
	TargetY  uint16
}

func parseAttackSkill(pkt []byte) skillCastRequest {
	// p39D_754/p36C do PacketProtocolV754: o 7.48 sempre envia SkillId@24
	// e Motion@28, inclusive quando 0x36C possui varios alvos depois de @44.
	if len(pkt) >= 30 {
		req := skillCastRequest{
			Skill:   int(int16(binary.LittleEndian.Uint16(pkt[24:26]))),
			Motion:  pkt[28],
			TargetX: binary.LittleEndian.Uint16(pkt[20:22]),
			TargetY: binary.LittleEndian.Uint16(pkt[22:24]),
		}
		// A familia p39D/p39E/p367 carrega o primeiro alvo em @44. O servidor
		// valida esse ID antes de usa-lo; dano e demais campos do client sao ignorados.
		if len(pkt) >= 46 {
			req.TargetID = binary.LittleEndian.Uint16(pkt[44:46])
		}
		return req
	}
	return skillCastRequest{Skill: -1, Motion: 5}
}

func skillManaCost(skill model.SkillDef, mastery, saveMana int) int {
	cost := skill.ManaSpent * (100 + mastery/2) / 100
	if saveMana < 0 {
		saveMana = 0
	}
	if saveMana > 100 {
		saveMana = 100
	}
	return cost * (100 - saveMana) / 100
}

// skillVisualLevel is sent to the client as SkillParm. A zero value makes the
// 7.48 client suppress several target particles even though the server has
// already accepted and applied the skill.
func skillVisualLevel(mastery int) byte {
	return byte(clampInt(mastery, 1, 255))
}

func explosionBashBaseDamage(baseDamage, intelligence, currentMP int) int {
	return baseDamage + intelligence + currentMP
}

// O TK possui duas familias magicas (Confianca e Espada Magica) e uma
// familia fisica (Trans). A divisao deve usar o indice local da classe.
func tkTransformationSkill(skillIndex int) bool {
	return skillIndex >= 0 && skillIndex < 24 && (skillIndex%24)/8 == 1
}

// baseSkillDamage porta BASE_GetSkillDamage da source 7.59 para os
// dados/structs do Go. Weather fica zero enquanto clima nao for modelado.
func (w *World) baseSkillDamage(ch *model.Char, skill model.SkillDef) int {
	local := skill.Index % 24
	kind := local/8 + 1
	special := int(playerMastery(ch, kind))
	level, base := clampInt(int(playerLevel(ch)), 0, 400), skill.InstanceValue
	weapon := w.equipmentDamage(ch)
	damage := 0
	if skill.InstanceType == 0 {
		switch skill.Index {
		case 11:
			damage = special/10 + skill.AffectValue
		case 13:
			damage = 3*special + skill.AffectValue
		case 41:
			damage = special/25 + 2
		case 43:
			damage = skill.AffectValue + special/3 + 15
		case 44:
			damage = 5 * (special/3 + 15)
		case 45:
			damage = special/10 + skill.AffectValue
		}
		return maxInt(1, damage)
	}
	if skill.InstanceType < 1 || skill.InstanceType > 5 {
		switch skill.InstanceType {
		case 6:
			damage = 3*special/2 + base
			if skill.Index == 29 && ch.LearnedSkill&0x80 != 0 {
				damage = 120 * damage / 100
			}
		case 11:
			damage = base
		default:
			damage = 2 * int(effectiveExtended(ch).MagicAmp)
		}
		return maxInt(1, damage)
	}

	adc := special%10 > 5
	levelPart := level / 2
	if adc {
		levelPart = level
	}
	// O MagicAttack BASE do personagem (ch.Extended.MagicAttack) integra o core
	// magico exatamente como no calculo do MagicAttack exibido (equipment.go
	// magicCore = base.MagicAttack + ...). Sem ele, um char com muito magic
	// persistido mostrava o valor alto mas a skill saia so com o termo de INT.
	magicAttackBase := 0
	if ch.Extended != nil {
		magicAttackBase = int(ch.Extended.MagicAttack)
	}
	if skill.Index == 97 {
		damage = 15*level + base
	} else {
		switch ch.Class {
		case 0:
			if tkTransformationSkill(skill.Index) {
				damage = 3*weapon + 3*playerStr(ch) + levelPart + special + base
			} else {
				damage = levelPart + special + base + weapon +
					playerInt(ch)/4 + playerInt(ch)/40 + magicAttackBase
			}
		case 1, 2:
			damage = playerInt(ch)/30 + playerInt(ch)/3 + levelPart + special + base + magicAttackBase
			if adc {
				damage += special
			}
		case 3:
			if skill.Index == 79 {
				return maxInt(1, playerAttack(ch))
			}
			damage = 3*weapon + 3*playerStr(ch) + level/2 + special + base
		}
	}
	if skill.Index != 97 {
		if ch.Class != 3 && (ch.Class != 0 || !tkTransformationSkill(skill.Index)) {
			damage = (4*int(effectiveExtended(ch).MagicAmp) + 100) * damage / 100
		}
		damage = 5 * damage / 4
	}
	branch := local / 8
	if int(ch.Class) < 4 && ch.LearnedSkill&(uint32(1)<<(branch*8+7)) != 0 {
		multipliers := [4][3]int{{115, 120, 115}, {110, 115, 115}, {110, 100, 100}, {110, 110, 120}}
		damage = damage * multipliers[ch.Class][branch] / 100
	}
	return maxInt(1, damage)
}

func skillUsesMagic(ch *model.Char, skill model.SkillDef) bool {
	if ch == nil || skill.InstanceType < 1 || skill.InstanceType > 5 {
		return false
	}
	return ch.Class != 3 && (ch.Class != 0 || !tkTransformationSkill(skill.Index))
}

func resistanceForInstance(instance int, resist model.ElementalResists) int {
	switch instance {
	case 1, 2:
		return int(resist.Fire)
	case 3:
		return int(resist.Ice)
	case 4:
		return int(resist.Sacred)
	case 5:
		return int(resist.Thunder)
	default:
		return 0
	}
}

// applySkillResistance replica _MSG_Attack 7.59. Mobs usam metade da
// resistencia antes do multiplicador (150-resist)%, exatamente como a source.
func applySkillResistance(damage, instance int, resist model.ElementalResists, targetIsMob bool) int {
	if instance < 1 || instance > 5 {
		return damage
	}
	value := clampInt(resistanceForInstance(instance, resist), 0, 100)
	if targetIsMob {
		value /= 2
	}
	return damage * (150 - value) / 100
}

func skillFinalDamage(damage, defense, mastery int) int {
	damage -= defense / 2
	if mastery > 15 {
		mastery = 15
	}
	rangeSize := 21 - mastery
	if rangeSize < 1 {
		rangeSize = 1
	}
	damage = damage * (rand.Intn(rangeSize) + mastery + 90) / 100
	if damage < -50 {
		damage = 0
	} else if damage < 0 {
		damage = (damage + 50) / 10
	} else if damage <= 45 {
		damage = damage*5/4 + 5
	}
	if damage < 1 {
		damage = 1
	}
	if damage > int(maxExtendedStat) {
		damage = int(maxExtendedStat)
	}
	return damage
}

func skillDamageMastery(ch *model.Char) int {
	if ch != nil && ch.Class == 0 && learnedLocal(ch, 14) { // Nocao de Combate
		return clampInt(int(playerMastery(ch, 2))/20, 0, 15)
	}
	return 0
}

func (w *World) skillMonsterTargets(p *Player, req skillCastRequest, skill model.SkillDef) []*Mob {
	bonusRange := 0
	if specialSkillLearned(p.Char, 101) {
		bonusRange = 1
	}
	castRange := uint16(maxInt(attackRange, skill.Range+bonusRange))
	primary := w.mobByID(req.TargetID)
	if primary == nil || primary.Dead || primary.HP == 0 || !w.playerCanInteractWithMob(p, primary) ||
		chebyshev(p.X, p.Y, primary.X, primary.Y) > int(castRange) ||
		!w.combatLineOfSight(p.X, p.Y, primary.X, primary.Y) {
		return nil
	}
	limit := clampInt(skill.MaxTarget, 1, 13)
	if skill.Index == 95 { // Rapid Hit: varios impactos, sempre no mesmo alvo.
		limit = 1
	}
	targets := []*Mob{primary}
	if limit == 1 {
		return targets
	}
	area := maxInt(1, skill.Range)
	for _, candidate := range w.mobs {
		if len(targets) >= limit {
			break
		}
		if candidate == primary || candidate.Dead || !w.playerCanInteractWithMob(p, candidate) {
			continue
		}
		if chebyshev(primary.X, primary.Y, candidate.X, candidate.Y) <= area {
			targets = append(targets, candidate)
		}
	}
	return targets
}

func (w *World) onSkillAttack(p *Player, req skillCastRequest) {
	skillIndex, motion := req.Skill, req.Motion
	local := skillIndex - int(p.Char.Class)*24
	special := specialSkillLearned(p.Char, skillIndex)
	if !special && (local < 0 || local >= 24 || p.Char.LearnedSkill&(uint32(1)<<local) == 0) {
		return
	}
	skill, ok := w.skills[skillIndex]
	if !ok || skill.Passive != 0 && skillIndex != 47 {
		return
	}
	now := time.Now()
	if p.SkillReady == nil {
		p.SkillReady = make(map[int]time.Time)
	}
	if now.Before(p.SkillReady[skillIndex]) {
		return
	}
	kind := local/8 + 1
	if special {
		kind = 4
	}
	mastery := int(playerMastery(p.Char, kind))
	var targets []*Mob
	var playerTargets []*Player
	if skill.Aggressive != 0 {
		if skillIndex == 98 {
			if !w.canCastThornWall(p, req, skill) {
				return
			}
		} else {
			playerTargets = w.skillPlayerTargets(p, req, skill)
			if len(playerTargets) == 0 {
				targets = w.skillMonsterTargets(p, req, skill)
			}
			if len(targets) == 0 && len(playerTargets) == 0 {
				return
			}
		}
	}
	if skillIndex == 97 && (w.groundCannonAt(p.X, p.Y) == nil || len(targets) == 0 ||
		chebyshev(p.X, p.Y, targets[0].X, targets[0].Y) < 4) {
		return
	}
	mana := skillManaCost(skill, mastery, int(effectiveExtended(p.Char).SaveMana))
	if playerCurMP(p.Char) < uint32(mana) {
		return
	}
	if skillIndex == 83 { // Alquimia: W2PP cobra 10 * level ao abrir a composicao.
		cost := playerLevel(p.Char) * 10
		if p.Char.Gold < cost {
			return
		}
		p.Char.Gold -= cost
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	}
	if !spendPlayerMP(p.Char, uint32(mana)) {
		return
	}
	if skill.Aggressive != 0 {
		w.breakHideOnAttack(p)
	}
	// SkillData.Delay e expresso em segundos tanto pelo client quanto no TMSrv.
	// Multiplicar por 250 ms permitia quatro casts no periodo de um.
	delay := time.Duration(skill.Delay) * time.Second
	p.SkillReady[skillIndex] = now.Add(delay)
	if motion == 0 {
		motion = 5
	}
	if skillIndex == 98 {
		if w.castThornWall(p, req, skill, mastery, motion) {
			w.syncPlayerScoreAndVitals(p)
			log.Printf("[#%d] criou Vinha skill=98 @(%d,%d)", p.Session.ID, req.TargetX, req.TargetY)
		}
		return
	}

	if skill.Aggressive == 0 {
		affected := w.applySupportSkill(p, req, skill, mastery)
		primary := p
		wireTargets := []wire.SkillTarget{{ID: p.ID}}
		if len(affected) > 0 {
			primary = affected[0].player
			wireTargets = make([]wire.SkillTarget, 0, len(affected))
			for _, result := range affected {
				// O protocolo representa cura como Damage short negativo; o
				// builder faz essa conversao e aplica a mesma escala do dano.
				wireTargets = append(wireTargets, wire.SkillTarget{
					ID: result.player.ID, Heal: result.hpDelta,
					MaxHP: playerMaxHP(result.player.Char)})
			}
		}
		w.sendToPlayerView(p, func() []byte {
			return spectralPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, primary.X, primary.Y,
				p.Char.Exp, playerCombatMP(p.Char), int16(skillIndex), motion, skillVisualLevel(mastery),
				skill.MaxTarget, wireTargets))
		})
		for _, result := range affected {
			target := result.player
			// O dono PRECISA do 0x181 alem do 0x336. Os dois wrappers do patch
			// wide alimentam o sidecar que o HUD le, e o 0x336 sozinho nao
			// estava bastando: com so ele, as barras de HP e MP ficavam quase
			// zeradas -- o desenho caia para o WORD escalado (29.322) sobre o
			// MaxHP wide (1.202.183), 2,4% da barra.
			//
			// O custo e o flicker de um redesenho nativo a mais por cast. E o
			// lado certo desse trade: piscar incomoda, barra vazia engana.
			w.publishPlayerAffects(target)
			w.syncPlayerVitals(target)
			w.updatePartyMember(target)
			if result.revived {
				// Ressurreicao e a excecao intencional: o client conserva o
				// TMHuman morto e so volta ao estado vivo apos RemoveMob(type 3)
				// seguido de uma materializacao nova. Atualizacoes comuns nunca
				// passam por este caminho.
				w.rematerializePlayerAfterRevive(target)
			} else {
				w.sendToPlayerView(target, func() []byte { return playerAppearancePacket(target) })
			}
		}
		w.syncPlayerScoreAndVitals(p)
		log.Printf("[#%d] usou skill=%d %q mp=-%d cooldown=%s", p.Session.ID,
			skillIndex, skill.Name, mana, delay)
		return
	}
	if len(playerTargets) > 0 {
		p.CombatTargetID = playerTargets[0].ID
		w.executePlayerSkill(p, playerTargets, skill, mastery, motion)
		return
	}
	p.CombatTargetID = targets[0].ID
	type skillResult struct {
		mob        *Mob
		calculated uint32
		applied    uint32
	}
	results := make([]skillResult, 0, len(targets))
	wireTargets := make([]wire.SkillTarget, 0, len(targets))
	type wideSkillHit struct {
		mob    *Mob
		damage uint32
	}
	wideHits := make([]wideSkillHit, 0, len(targets))
	baseDamage := w.baseSkillDamage(p.Char, skill)
	if skillIndex == 30 { // Julgamento Divino: soma o HP e sacrifica 5/6 dele.
		baseDamage += int(playerCurHP(p.Char))
		setPlayerCurHP(p.Char, playerCurHP(p.Char)/6+1)
	}
	if skillIndex == 22 { // Explosion Bash: converte toda a mana atual e INT em dano.
		baseDamage = explosionBashBaseDamage(baseDamage, playerInt(p.Char), int(playerCurMP(p.Char)))
		setPlayerCurMP(p.Char, 0)
		if len(targets) > 0 {
			p.X, p.Y = w.findFreeGameplayPosition(p, p, targets[0].X, targets[0].Y, 2)
			p.Char.X, p.Char.Y = p.X, p.Y
			w.refreshPlayerVisibility(p)
			w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
		}
	}
	magicDamage := skillUsesMagic(p.Char, skill)
	directDamage := skill.InstanceType >= 1 && skill.InstanceType <= 5
	hitCount := skillHitCount(skill)
	for _, target := range targets {
		target.TargetID = p.ID
		appliedTotal := uint32(0)
		calculatedTotal := uint32(0)
		if directDamage {
			damageValue := skillFinalDamage(baseDamage, effectiveMobDefense(target), skillDamageMastery(p.Char))
			damageValue = damageValue * 70 / 100 // reducao PvE Mortal da _MSG_Attack 7.59
			damageValue = applySkillResistance(damageValue, skill.InstanceType, effectiveMobResistances(target), true)
			damageValue = applyCouragePvEDamage(p.Char, damageValue, magicDamage)
			damageValue = int(addFlatDamage(uint32(clampInt(damageValue, 1, int(maxExtendedStat))),
				w.equipmentGemBonuses(p.Char).forceDamage))
			perHitCalculated := uint32(clampInt(damageValue, 1, int(maxExtendedStat)))
			calculatedWide := uint64(perHitCalculated) * uint64(hitCount)
			if calculatedWide > uint64(maxExtendedStat) {
				calculatedWide = uint64(maxExtendedStat)
			}
			calculatedTotal = uint32(calculatedWide)
			hpBeforeSkill := target.HP
			// Escudo de boss absorve cada golpe da skill.
			perHitCalculated = w.bossMitigateDamage(target, perHitCalculated)
			for hit := 0; hit < hitCount && target.HP > 0; hit++ {
				applied := minU32(perHitCalculated, target.HP)
				target.HP -= applied
				appliedTotal += applied
				// O HP recebe somente o que ainda restava no alvo, mas o numero
				// flutuante representa o dano calculado (overkill inclusive).
				// O 0x36C permanece no tamanho nativo para efeitos/alvos. O
				// numero integral segue logo depois em 0x39D estendido.
				wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID})
				wideHits = append(wideHits, wideSkillHit{mob: target, damage: perHitCalculated})
			}
			// Uma notificacao com o TOTAL da skill, nao uma por golpe: um limiar
			// de HP tem de ser atravessado uma vez so.
			if damage := hpBeforeSkill - target.HP; damage > 0 {
				w.notifyMobDamaged(target, hpBeforeSkill, p.ID, damage)
			}
		} else {
			wireTargets = append(wireTargets, wire.SkillTarget{ID: target.ID})
		}
		if skillIndex == 6 { // Furia Divina puxa o alvo para junto do caster.
			oldX, oldY := target.X, target.Y
			target.X, target.Y = w.findFreePosition(p.X, p.Y, 2)
			w.publishMobMove(target, oldX, oldY, uint32(target.Def.Extended.AttackRun&0x0f))
		}
		if skillIndex == 26 { // Flash encerra o combate/aggro do alvo.
			target.TargetID = 0
		}
		results = append(results, skillResult{mob: target, calculated: calculatedTotal, applied: appliedTotal})
		w.applySkillMobEffects(p, target, skill, mastery)
	}
	primary := targets[0]
	w.sendToMobView(primary, func() []byte {
		return spectralPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, primary.X, primary.Y,
			p.Char.Exp, playerCombatMP(p.Char), int16(skillIndex), motion, skillVisualLevel(mastery),
			skill.MaxTarget, wireTargets))
	})
	for _, hit := range wideHits {
		hit := hit
		w.sendToMobView(hit.mob, func() []byte {
			return spectralPacket(p.Char, wire.SkillHitExtended(p.ID, hit.mob.ID, p.X, p.Y, hit.mob.X, hit.mob.Y,
				hit.damage, hit.mob.Def.Extended.MaxHP, p.Char.Exp, playerCombatMP(p.Char),
				int16(skillIndex), motion, skillVisualLevel(mastery)))
		})
	}
	w.syncPlayerScoreAndVitals(p)
	log.Printf("[#%d] executou skill=%d %q alvos=%d base=%d magic=%t amp=%d mastery=%d mp=-%d",
		p.Session.ID, skillIndex, skill.Name, len(results), baseDamage, magicDamage,
		effectiveExtended(p.Char).MagicAmp, mastery, mana)
	// Capture os receptores antes da primeira morte. Um membro pode atingir o
	// level maximo no meio do lote e deixaria de aparecer se recalculassemos a
	// elegibilidade somente no final, fazendo sua ultima EXP nao ser persistida.
	batchAccounts := uniqueKillAccounts(p, w.partyExpShares(p, 1, w.gameplay.PartyEXPBonusPercent))
	kills := 0
	for _, result := range results {
		if result.mob.HP == 0 {
			w.killMobState(p, result.mob, result.calculated, result.applied, false)
			kills++
		} else {
			mob := result.mob
			w.sendToMobView(mob, func() []byte {
				return wire.SetMobHpMp(mob.ID, mob.HP, mob.Def.Extended.MaxHP,
					mob.Def.Extended.MaxMP, mob.Def.Extended.MaxMP)
			})
			log.Printf("[#%d] skill=%d %q mob=%d dmg=%d base=%d magic=%t amp=%d mastery=%d hp=%d/%d mp=-%d", p.Session.ID,
				skillIndex, skill.Name, mob.ID, result.applied, baseDamage, magicDamage,
				effectiveExtended(p.Char).MagicAmp, mastery, mob.HP, mob.Def.Extended.MaxHP, mana)
		}
	}
	w.saveMultiKillBatch(p, kills, batchAccounts)
}

func skillHitCount(skill model.SkillDef) int {
	if skill.Index == 95 {
		return clampInt(skill.MaxTarget, 1, 6)
	}
	return 1
}

func (w *World) saveMultiKillBatch(p *Player, kills int, accounts []*model.Account) {
	if kills <= 0 || p == nil || p.Account == nil {
		return
	}
	if len(accounts) == 0 {
		accounts = []*model.Account{p.Account}
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		log.Printf("[#%d] salvar progressao multi-alvo: %v", p.Session.ID, err)
		w.poisonAccountsAfterPersistenceFailure(accounts, "mortes multi-alvo", err)
		return
	}
	log.Printf("[#%d] lote multi-alvo salvo (%d mortes, %d conta(s))",
		p.Session.ID, kills, len(accounts))
}

func uniqueShareAccounts(shares []partyExpShare) []*model.Account {
	accounts := make([]*model.Account, 0, len(shares))
	seen := make(map[*model.Account]struct{}, len(shares))
	for _, share := range shares {
		account := share.player.Account
		if account == nil {
			continue
		}
		if _, exists := seen[account]; exists {
			continue
		}
		seen[account] = struct{}{}
		accounts = append(accounts, account)
	}
	return accounts
}

// uniqueKillAccounts inclui sempre o dono do loot, mesmo quando ele nao recebe
// EXP (por exemplo, personagem no level cap). Usar apenas os shares deixava
// gold/itens do killer fora da transacao quando outros membros ainda eram
// elegiveis para experiencia.
func uniqueKillAccounts(killer *Player, shares []partyExpShare) []*model.Account {
	accounts := make([]*model.Account, 0, len(shares)+1)
	seen := make(map[*model.Account]struct{}, len(shares)+1)
	if killer != nil && killer.Account != nil {
		seen[killer.Account] = struct{}{}
		accounts = append(accounts, killer.Account)
	}
	for _, account := range uniqueShareAccounts(shares) {
		if _, exists := seen[account]; exists {
			continue
		}
		seen[account] = struct{}{}
		accounts = append(accounts, account)
	}
	return accounts
}
