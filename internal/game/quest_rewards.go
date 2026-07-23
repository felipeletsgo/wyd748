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
	return clone
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
		s.Send(wire.MessagePanel("Esta recompensa nao pode ser usada neste nivel ou evolucao."))
		return
	}

	reward := expWithDoubleBuff(p.Char, scaledQuestExperience(rule.Exp, w.gameplay))
	if reward == 0 {
		// exp_rate=0 desativa o ganho: preserve a caixa e o gold para nao confirmar
		// uma recompensa incompleta.
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("O ganho de experiencia esta desativado."))
		return
	}

	snapshot := cloneCharacterState(p.Char)
	oldItemIndex := item.Index
	oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
	levels, applied := grantExp(p.Char, reward)
	if applied == 0 {
		*p.Char = snapshot
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if rule.Gold >= maxCharacterGold || p.Char.Gold > maxCharacterGold-rule.Gold {
		p.Char.Gold = maxCharacterGold
	} else {
		p.Char.Gold += rule.Gold
	}
	goldApplied := p.Char.Gold - snapshot.Gold
	if rule.Consume {
		consumeOne(item)
	}

	// grantExp recalcula o score cru. Reaplique equipamentos/buffs e preserve os
	// recursos atuais antes de persistir, para o JSON nao guardar HP/MP transitivo.
	w.recalcPlayer(p.Char)
	if oldHP > 0 {
		setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
	}
	if oldMP > 0 {
		setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
	}

	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		log.Printf("[#%d] ERRO ao salvar recompensa de quest item=%d: %v", s.ID, oldItemIndex, err)
		s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
		s.Send(wire.MessagePanel("Falha ao salvar. A recompensa nao foi consumida."))
		return
	}

	w.syncPlayerVitals(p)
	w.updatePartyMember(p)
	if levels > 0 {
		s.Send(wire.UpdateScore(p.ID, *p.Char))
	}
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
	s.Send(wire.MessagePanel(fmt.Sprintf("Recompensa recebida: +%d EXP e +%d gold.", applied, goldApplied)))
	log.Printf("[#%d] QUEST-REWARD item=%d volatile=%d exp=+%d gold=+%d level=%d +%d",
		s.ID, oldItemIndex, code, applied, goldApplied, p.Char.Extended.Level, levels)
}
