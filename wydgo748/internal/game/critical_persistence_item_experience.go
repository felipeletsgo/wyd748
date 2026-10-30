package game

import (
	"errors"
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Experience rewards are planned once on World. Workers receive only the
// account proposal; neither progression nor item consumption is visible early.
func (w *World) queueItemExperience(p *Player, slot byte, rule model.VolatileRule, code int) error {
	if p == nil || p.Account == nil || p.Char == nil || p.Char.Score == nil || !bindCriticalPlayer(p).current(w) || int(slot) >= model.PlayerCarrySlots ||
		p.Char.Inv[slot].Index == 0 || !criticalItemExperience(rule.Action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	reward, gold := rule.Exp, uint32(0)
	if rule.Action == "grant_next_level" {
		var allowed bool
		reward, allowed = nextLevelItemExperience(ch)
		if !allowed {
			return &nativeConsumableRejection{}
		}
	}
	if rule.Action == "quest_reward" {
		if !questRewardAllowed(ch, rule) {
			return &nativeConsumableRejection{message: "This reward cannot be used at this level or evolution."}
		}
		reward = expWithDoubleBuffAt(ch, scaledQuestExperience(rule.Exp, w.gameplay), w.now())
		gold = rule.Gold
		if reward == 0 {
			return &nativeConsumableRejection{message: "Experience gain is disabled."}
		}
	}
	oldHP, oldMP := playerCurHP(ch), playerCurMP(ch)
	levels, applied := grantExp(ch, reward)
	if applied == 0 {
		return &nativeConsumableRejection{}
	}
	cytheraChanged := levels > 0 && updateCelestialCythera(ch)
	beforeGold := ch.Gold
	if gold > 0 {
		if gold >= maxCharacterGold || ch.Gold > maxCharacterGold-gold {
			ch.Gold = maxCharacterGold
		} else {
			ch.Gold += gold
		}
	}
	goldApplied := ch.Gold - beforeGold
	index := ch.Inv[slot].Index
	if rule.Consume {
		consumeOne(&ch.Inv[slot])
	}
	recalculate := func(character *model.Char) {
		w.recalcPlayer(character)
		if oldHP > 0 {
			setPlayerCurHP(character, minU32(oldHP, playerMaxHP(character)))
		}
		if oldMP > 0 {
			setPlayerCurMP(character, minU32(oldMP, playerMaxMP(character)))
		}
	}
	recalculate(ch)
	return w.queueCriticalAccountMutation(p, proposed, rule.Action,
		"Save failed. The reward was not consumed.", func() {
			p.Char.Inv[slot], p.Char.Equip[1] = ch.Inv[slot], ch.Equip[1]
			p.Char.Exp, p.Char.Gold, p.Char.Score.Level = ch.Exp, ch.Gold, ch.Score.Level
			p.Char.CP = ch.CP
			syncProgression(p.Char)
			recalculate(p.Char)
		}, func() {
			w.syncPlayerVitals(p)
			w.updatePartyMember(p)
			if levels > 0 {
				p.Session.Send(playerScorePacket(p))
				w.syncPlayerChaos(p)
			}
			if cytheraChanged {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, 1, p.Char.Equip[1]))
				w.refreshAppearance(p)
			}
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			resyncUsedItem(p.Session, p, slot)
			if rule.Action == "quest_reward" {
				p.Session.Send(wire.MessagePanel(fmt.Sprintf("Reward received: +%d EXP and +%d gold.", applied, goldApplied)))
			}
			log.Printf("[#%d] %s item=%d volatile=%d experience=+%d gold=+%d levels=+%d",
				p.Session.ID, rule.Action, index, code, applied, goldApplied, levels)
		}, true)
}

func criticalItemExperience(action string) bool {
	return action == "grant_exp" || action == "quest_reward" || action == "grant_next_level"
}

func (w *World) admitItemExperience(p *Player, slot byte, rule model.VolatileRule, code int) {
	if err := w.queueItemExperience(p, slot, rule, code); err != nil {
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			return
		}
		p.Session.Send(wire.MessagePanel("Reward use is unavailable. Please try again later."))
		log.Printf("[#%d] %s admission rejected: %v", p.Session.ID, rule.Action, err)
	}
}
