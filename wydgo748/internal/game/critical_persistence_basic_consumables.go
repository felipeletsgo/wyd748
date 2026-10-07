package game

import (
	"errors"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalBasicConsumable(action string) bool {
	switch action {
	case "firework", "chaos_remission", "magical_pill", "learn_special_skill", "restore", "gold", "save_position", "disabled":
		return true
	}
	return false
}

// Simple account-only effects share reservation and settlement, but retain
// their own domain checks and field-level installation on the actor.
func (w *World) queueBasicConsumable(p *Player, slot byte, rule model.VolatileRule, def model.ItemDef) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) || int(slot) >= model.PlayerCarrySlots ||
		p.Char.Inv[slot].Index == 0 || !criticalBasicConsumable(rule.Action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	reject := func(message string) error { return &nativeConsumableRejection{message: message} }
	now, oldPotion := w.now(), p.LastPotion
	var motion uint16
	switch rule.Action {
	case "firework":
		if rule.CustomPattern {
			return reject("")
		}
		motion = uint16(w.intn(6))
	case "chaos_remission":
		ch.CP = model.MaxCP
	case "magical_pill":
		if ch.MagicalPillUsed {
			return reject("")
		}
		ch.MagicalPillUsed, ch.SkillPointBonus = true, ch.SkillPointBonus+9
	case "learn_special_skill":
		bit := uint(rule.LearnedBit)
		if bit < 25 || bit > 29 || ch.LearnedSkill&(uint32(1)<<bit) != 0 {
			return reject("")
		}
		ch.LearnedSkill |= uint32(1) << bit
		filterShortSkills(ch)
	case "restore":
		if !oldPotion.IsZero() && now.Sub(oldPotion) < potionCooldown {
			return reject("")
		}
		if hp, mp := applyRestore(ch, ch.Inv[slot], def, rule); hp == 0 && mp == 0 {
			return reject("")
		}
	case "gold":
		value := volatileGoldValue(rule, ch.Inv[slot], def)
		if value == 0 || value > maxCharacterGold {
			return reject("")
		}
		if ch.Gold > maxCharacterGold || value > maxCharacterGold-ch.Gold {
			return reject("Gold limit reached.")
		}
		ch.Gold += value
	case "save_position":
		ch.SavedX, ch.SavedY = p.X, p.Y
	}
	if rule.Consume {
		consumeOne(&ch.Inv[slot])
	}
	if rule.Action == "magical_pill" {
		w.recalcPlayer(ch)
	}
	return w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, rule.Action,
		"Item use could not be saved. Please try again later.", func() {
			p.Char.Inv[slot] = ch.Inv[slot]
			switch rule.Action {
			case "chaos_remission":
				p.Char.CP = ch.CP
			case "magical_pill":
				p.Char.MagicalPillUsed, p.Char.SkillPointBonus = ch.MagicalPillUsed, ch.SkillPointBonus
				w.recalcPlayer(p.Char)
			case "learn_special_skill":
				p.Char.LearnedSkill, p.Char.ShortSkill = ch.LearnedSkill, ch.ShortSkill
			case "restore":
				setPlayerCurHP(p.Char, playerCurHP(ch))
				setPlayerCurMP(p.Char, playerCurMP(ch))
				p.LastPotion = now
			case "gold":
				p.Char.Gold = ch.Gold
			case "save_position":
				p.Char.SavedX, p.Char.SavedY = ch.SavedX, ch.SavedY
			}
		}, func() {
			switch rule.Action {
			case "firework":
				w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 100, motion) })
			case "chaos_remission":
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				w.syncPlayerChaos(p)
			case "magical_pill", "learn_special_skill":
				p.Session.Send(playerScorePacket(p))
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				p.Session.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
			case "restore":
				w.syncPlayerVitals(p)
				w.updatePartyMember(p)
			case "gold":
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			case "save_position":
				p.Session.Send(wire.MessagePanel("Position saved."))
			}
			resyncUsedItem(p.Session, p, slot)
		}, true, criticalAccountMutationHooks{check: func() error {
			if rule.Action == "restore" && p.LastPotion != oldPotion {
				return errCriticalPersistenceIdentity
			}
			return nil
		}})
}

func (w *World) admitBasicConsumable(p *Player, slot byte, rule model.VolatileRule, def model.ItemDef) {
	if err := w.queueBasicConsumable(p, slot, rule, def); err != nil {
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			if rule.Action == "gold" {
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			}
			return
		}
		p.Session.Send(wire.MessagePanel("Item use is unavailable. Please try again later."))
		log.Printf("[#%d] %s admission rejected: %v", p.Session.ID, rule.Action, err)
	}
}
