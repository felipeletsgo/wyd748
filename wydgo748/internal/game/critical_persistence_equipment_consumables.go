package game

import (
	"errors"
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalEquipmentConsumable(action string) bool {
	switch action {
	case "refine", "refine_set", "tint", "untint", "repliction":
		return true
	}
	return false
}

func (w *World) queueEquipmentConsumable(p *Player, slot byte, req useItemRequest, rule model.VolatileRule) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) ||
		int(slot) >= model.PlayerCarrySlots || p.Char.Inv[slot].Index == 0 || !criticalEquipmentConsumable(rule.Action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	detached := &Player{Char: ch, SpecialCoins: copyCounters(p)}
	reject := func(message string) error { return &nativeConsumableRejection{message: message} }
	var target *model.Item
	var targetType, targetPos uint32
	message := ""
	write := criticalWrite{}
	if rule.Action == "refine_set" {
		if ch.Score == nil || rule.MortalOnly && !matchesEvolution(ch, "mortal") {
			return reject("This item is exclusive to Mortals.")
		}
		if rule.RequiredEvolution != "" && !matchesEvolution(ch, rule.RequiredEvolution) {
			return reject("This item cannot be used by this evolution.")
		}
		if rule.MinLevel != 0 && ch.Score.Level < rule.MinLevel || rule.MaxLevelExclusive != 0 && ch.Score.Level >= rule.MaxLevelExclusive {
			return reject("This item is only available between levels 201 and 256.")
		}
		if rule.OnceQuestID != 0 && questCompleted(ch, int(rule.OnceQuestID)) {
			return reject("You have already completed this quest.")
		}
		if index := ch.Equip[1].Index; index >= 3500 && index <= 3507 {
			return reject("Cythera cannot be refined by this quest.")
		}
		level := rule.RefineMax
		if level <= 0 || level > 15 {
			level = 6
		}
		changed := 0
		for pos := 1; pos <= 5; pos++ {
			item := &ch.Equip[pos]
			if item.Index != 0 && itemSanc(*item) < level && setItemSanc(item, level) {
				changed++
			}
		}
		if changed == 0 {
			return reject("No armor piece to refine.")
		}
		if rule.OnceQuestID != 0 {
			markQuestCompleted(ch, int(rule.OnceQuestID))
		}
		write.character = &criticalCharacterState{uid: ch.UID, state: buildCharState(detached, w.now())}
		message = "Set refined!"
	} else {
		if rule.Action == "refine" {
			target, targetType, targetPos = w.destRefineTarget(detached, req)
		} else {
			target, targetType, targetPos = w.destItemTarget(detached, req)
		}
		if target == nil || target.Index == 0 || targetType == placeInv && targetPos == uint32(slot) {
			return reject("Drag the item onto a valid target.")
		}
		switch rule.Action {
		case "tint":
			if def, ok := w.items[target.Index]; !ok || !isArmorSetPiece(def) {
				return reject(tintSetOnlyMessage)
			}
			if !tintItem(target, rule.Color) {
				return reject("Dye a refined item (one with a + glow).")
			}
		case "untint":
			if !untintItem(target) {
				return reject("Use it on colored items.")
			}
		case "repliction":
			if targetType != placeEquip {
				return reject("Repliction can only be used on equipped defensive equipment.")
			}
			def, exists := w.items[target.Index]
			if !exists {
				return reject("")
			}
			if _, err := w.applyRepliction(target, def, ch.Inv[slot].Index); err != nil {
				return reject(err.Error())
			}
			message = "The item gained a new bonus!"
		case "refine":
			def, exists := w.items[target.Index]
			if !exists || def.Pos == 0 || itemAbility(*target, def, "EF_VOLATILE") != 0 || itemAbility(*target, def, "EF_NOSANC") != 0 {
				return reject("That item cannot be refined.")
			}
			level, limit := itemSanc(*target), rule.RefineMax
			if limit <= 0 || limit > 15 {
				limit = 15
			}
			if level >= limit {
				return reject("That item cannot be refined any further.")
			}
			roll := w.rollPercent(refineChance(level))
			if roll.Success && !setItemSanc(target, level+1) {
				return reject("That item cannot be refined.")
			}
			message = roll.message()
		}
	}
	if rule.Consume {
		consumeOne(&ch.Inv[slot])
	}
	equipped := rule.Action == "refine_set" || targetType == placeEquip
	if equipped {
		w.recalcPlayer(ch)
	}
	return w.queueCriticalAccountWrite(p, proposed, write, rule.Action,
		"Item use could not be saved. Please try again later.", func() {
			p.Char.Inv, p.Char.Equip, p.Char.QuestsDone = ch.Inv, ch.Equip, ch.QuestsDone
			if equipped {
				w.recalcPlayer(p.Char)
			}
		}, func() {
			resyncUsedItem(p.Session, p, slot)
			if rule.Action == "refine_set" {
				for pos := 1; pos <= 5; pos++ {
					if p.Char.Equip[pos].Index != 0 {
						p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
					}
				}
			} else {
				p.Session.Send(wire.SendItem(p.ID, byte(targetType), byte(targetPos), *target))
			}
			if equipped {
				if rule.Action == "repliction" {
					projection := clientEquipProjection(p.Char)
					p.Session.Send(wire.SelfEquip(p.ID, projection[:]))
				}
				p.Session.Send(playerScorePacket(p))
				w.syncPlayerVitalsToObservers(p)
				w.refreshAppearance(p)
			}
			if message != "" {
				p.Session.Send(wire.MessagePanel(message))
			}
			log.Printf("[#%d] equipment consumable action=%s source=%d target=%d/%d",
				p.Session.ID, rule.Action, slot, targetType, targetPos)
		}, true, criticalAccountMutationHooks{reject: func() {
			if p.Session.IsClosed() {
				return
			}
			if rule.Action == "refine_set" {
				for pos := 1; pos <= 5; pos++ {
					p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
				}
			} else if equipped {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(targetPos), p.Char.Equip[targetPos]))
			}
		}})
}

func (w *World) admitEquipmentConsumable(p *Player, slot byte, req useItemRequest, rule model.VolatileRule) {
	if err := w.queueEquipmentConsumable(p, slot, req, rule); err != nil {
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			return
		}
		p.Session.Send(wire.MessagePanel(fmt.Sprintf("%s is unavailable. Please try again later.", rule.Action)))
		log.Printf("[#%d] %s admission rejected: %v", p.Session.ID, rule.Action, err)
	}
}
