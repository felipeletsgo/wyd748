package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// cloneCharacterState faz o snapshot profundo necessario para operacoes que
// podem subir level. grantExp altera EXP, ExtendedScore e campos derivados;
// restaurar apenas o item deixaria progressao aplicada quando o save falhasse.
func cloneCharacterState(ch *model.Char) model.Char {
	clone := *ch
	clone.QuestsDone = append([]int32(nil), ch.QuestsDone...)
	if ch.Extended != nil {
		extended := *ch.Extended
		clone.Extended = &extended
	}
	if ch.ExtendedRuntime != nil {
		runtime := *ch.ExtendedRuntime
		clone.ExtendedRuntime = &runtime
	}
	if ch.AlternateCelestial != nil {
		alternate := *ch.AlternateCelestial
		if ch.AlternateCelestial.Extended != nil {
			extended := *ch.AlternateCelestial.Extended
			alternate.Extended = &extended
		}
		clone.AlternateCelestial = &alternate
	}
	return clone
}

// expRewardOutcome informa POR QUE grantItemExpReward parou, para cada efeito
// logar/mensagear na sua propria voz sem duplicar a transacao. OK=false com
// Err==nil significa "sem ganho" (EXP no teto Mortal); Err!=nil e falha de
// persistencia -- nos dois casos o char e o item ja foram restaurados.
type expRewardOutcome struct {
	Levels int
	Exp    uint32
	Gold   uint32
	OK     bool
	Err    error
}

// grantItemExpReward concentra a transacao "consumir item -> ganhar EXP (e gold)"
// dos baus de XP (grant_exp) e das caixas de quest (quest_reward). Faz SNAPSHOT
// COMPLETO do personagem porque grantExp altera level, ExtendedScore e pontos --
// nao so a EXP; restaurar apenas o item deixaria progressao aplicada numa falha
// de save. O chamador ja validou os requisitos e calculou `exp` (bruto para os
// baus; escalado por rate/buff para as caixas). Persist-before-confirm: nada e
// confirmado ao client antes do save.
func (w *World) grantItemExpReward(s *net.Session, p *Player, item *model.Item,
	slot byte, exp, gold uint32, consume bool) expRewardOutcome {
	snapshot := cloneCharacterState(p.Char)
	// O recalc CRU dentro de grantExp clampa HP/MP no max sem buffs; preserva os
	// valores atuais para um jogador buffado nao perder recurso ao ganhar EXP.
	oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)

	levels, applied := grantExp(p.Char, exp)
	if applied == 0 {
		// EXP no teto Mortal: nada a conceder. Restaura (grantExp ja tocou o
		// runtime) e reenvia o item intacto, sem prometer um ganho inexistente.
		*p.Char = snapshot
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return expRewardOutcome{}
	}
	cytheraChanged := levels > 0 && updateCelestialCythera(p.Char)

	var goldApplied uint32
	if gold > 0 {
		before := p.Char.Gold
		if gold >= maxCharacterGold || p.Char.Gold > maxCharacterGold-gold {
			p.Char.Gold = maxCharacterGold
		} else {
			p.Char.Gold += gold
		}
		goldApplied = p.Char.Gold - before
	}
	if consume {
		consumeOne(item)
	}

	// grantExp recalcula o score cru. Reaplique equipamento/affects e restabeleca
	// os recursos atuais antes de persistir, para o JSON nao guardar HP/MP cru.
	w.recalcPlayer(p.Char)
	if oldHP > 0 {
		setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
	}
	if oldMP > 0 {
		setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
	}

	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
		return expRewardOutcome{Err: err}
	}

	w.syncPlayerVitals(p)
	w.updatePartyMember(p)
	if levels > 0 {
		s.Send(wire.UpdateScore(p.ID, *p.Char))
	}
	if cytheraChanged {
		s.Send(wire.SendItem(p.ID, placeEquip, 1, p.Char.Equip[1]))
		w.refreshAppearance(p)
	}
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
	return expRewardOutcome{Levels: levels, Exp: applied, Gold: goldApplied, OK: true}
}

func questRewardAllowed(ch *model.Char, rule model.VolatileRule) bool {
	if ch == nil || ch.Extended == nil {
		return false
	}
	if rule.MortalOnly && strings.TrimSpace(ch.Evolution) != "" {
		return false
	}
	level := ch.Extended.Level
	return level >= rule.MinLevel && level < rule.MaxLevelExclusive
}

// useQuestReward porta o Vol 191: valida a caixa pelo indice configurado,
// concede EXP/gold e so confirma o consumo depois da persistencia. O maximo de
// level e exclusivo, preservando inclusive as sobreposicoes nativas dos tiers.
func (w *World) useQuestReward(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule, code int) {
	if !questRewardAllowed(p.Char, rule) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This reward cannot be used at this level or evolution."))
		return
	}

	reward := expWithDoubleBuff(p.Char, scaledQuestExperience(rule.Exp, w.gameplay))
	if reward == 0 {
		// exp_rate=0 desativa o ganho: preserve a caixa e o gold para nao confirmar
		// uma recompensa incompleta.
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Experience gain is disabled."))
		return
	}

	oldItemIndex := item.Index
	res := w.grantItemExpReward(s, p, item, slot, reward, rule.Gold, rule.Consume)
	if !res.OK {
		if res.Err != nil {
			log.Printf("[#%d] ERRO ao salvar recompensa de quest item=%d: %v", s.ID, oldItemIndex, res.Err)
			s.Send(wire.MessagePanel("Save failed. The reward was not consumed."))
		}
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("Reward received: +%d EXP and +%d gold.", res.Exp, res.Gold)))
	log.Printf("[#%d] QUEST-REWARD item=%d volatile=%d exp=+%d gold=+%d level=%d +%d",
		s.ID, oldItemIndex, code, res.Exp, res.Gold, p.Char.Extended.Level, res.Levels)
}
