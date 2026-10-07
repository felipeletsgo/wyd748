package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// cloneCharacterState retains progression, nested scores and sidecar fields.
// Restoring only an item would leave a level-up applied after a failed save.
func cloneCharacterState(ch *model.Char) model.Char {
	clone := *ch
	clone.QuestsDone = append([]int32(nil), ch.QuestsDone...)
	if ch.Score != nil {
		extended := *ch.Score
		clone.Score = &extended
	}
	if ch.RuntimeScore != nil {
		runtime := *ch.RuntimeScore
		clone.RuntimeScore = &runtime
	}
	if ch.AlternateCelestial != nil {
		alternate := *ch.AlternateCelestial
		if ch.AlternateCelestial.Score != nil {
			extended := *ch.AlternateCelestial.Score
			alternate.Score = &extended
		}
		clone.AlternateCelestial = &alternate
	}
	return clone
}

// expRewardOutcome distinguishes a capped reward from a persistence failure.
// Both outcomes leave the character and item unchanged in the legacy path.
type expRewardOutcome struct {
	Levels int
	Exp    uint32
	Gold   uint32
	OK     bool
	Err    error
}

// grantItemExpReward is the synchronous nil-owner reward transaction. Callers
// validate requirements and calculate raw or rate/buff-adjusted experience.
// The coordinated path prepares the same changes in queueItemExperience.
func (w *World) grantItemExpReward(s *net.Session, p *Player, item *model.Item,
	slot byte, exp, gold uint32, consume bool) expRewardOutcome {
	snapshot := cloneCharacterState(p.Char)
	// Preserve buffed resources across grantExp's unmodified score calculation.
	oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)

	levels, applied := grantExp(p.Char, exp)
	if applied == 0 {
		// The cap granted nothing; restore runtime fields and resync the item.
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

	// Reapply equipment and affects, then restore resources before persistence.
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
		s.Send(playerScorePacket(p))
		w.syncPlayerChaos(p)
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
	if ch == nil || ch.Score == nil {
		return false
	}
	if rule.MortalOnly && strings.TrimSpace(ch.Evolution) != "" {
		return false
	}
	level := ch.Score.Level
	return level >= rule.MinLevel && level < rule.MaxLevelExclusive
}

// useQuestReward validates Vol 191 using its exclusive upper level bound.
// Reward publication and consumption follow the persistence acknowledgement.
func (w *World) useQuestReward(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule, code int) {
	if w.criticalTradeReserved(p) {
		return
	}
	if w.criticalWrites != nil {
		w.admitItemExperience(p, slot, rule, code)
		return
	}
	if !questRewardAllowed(p.Char, rule) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This reward cannot be used at this level or evolution."))
		return
	}

	reward := expWithDoubleBuffAt(p.Char, scaledQuestExperience(rule.Exp, w.gameplay), w.now())
	if reward == 0 {
		// A disabled rate must preserve both the box and its accompanying gold.
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Experience gain is disabled."))
		return
	}

	oldItemIndex := item.Index
	res := w.grantItemExpReward(s, p, item, slot, reward, rule.Gold, rule.Consume)
	if !res.OK {
		if res.Err != nil {
			log.Printf("[#%d] failed to save quest reward item=%d: %v", s.ID, oldItemIndex, res.Err)
			s.Send(wire.MessagePanel("Save failed. The reward was not consumed."))
		}
		return
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("Reward received: +%d EXP and +%d gold.", res.Exp, res.Gold)))
	log.Printf("[#%d] QUEST-REWARD item=%d volatile=%d exp=+%d gold=+%d level=%d +%d",
		s.ID, oldItemIndex, code, res.Exp, res.Gold, p.Char.Score.Level, res.Levels)
}
