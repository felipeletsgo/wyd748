package game

import (
	"errors"
	"fmt"
	"log"
	"math"
	"reflect"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type nativeConsumableRejection struct{ message string }

func (e *nativeConsumableRejection) Error() string { return "native consumable rejected: " + e.message }

func criticalNativeConsumable(action string) bool {
	return action == "refine_equipped" || action == "nightmare_ticket" || action == "mastery_reset" || action == "arch_crystal"
}

// Domain checks and all persisted changes run on a private account, on World.
// Trade cancellation belongs to post-commit publication, never admission.
func (w *World) prepareNativeConsumable(p *Player, slot byte, rule model.VolatileRule, code int) (*model.Account, error) {
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	reject := func(message string) (*model.Account, error) {
		return nil, &nativeConsumableRejection{message: message}
	}
	switch rule.Action {
	case "arch_crystal":
		step, ok := archCrystalStep(ch.Inv[slot].Index)
		if !ok {
			return reject("")
		}
		if !isArch(ch) {
			return reject("Only a God can use this crystal.")
		}
		if ch.Score == nil || ch.Score.Level < archCrystalMinLevel {
			return reject("You must reach level 355 to take this oath.")
		}
		if ch.ArchCrystals >= step {
			return reject("You have already done it.")
		}
		if ch.ArchCrystals != step-1 {
			return reject("You must take the oaths in order.")
		}
		if ch.Exp < archCrystalExpCost {
			return reject("You need 100 million experience for this oath.")
		}
		ch.ArchCrystals, ch.Exp = step, ch.Exp-archCrystalExpCost
		ch.Score.Level = levelForExp(ch.Exp)
		syncProgression(ch)
	case "refine_equipped":
		if rule.TargetSlot < 0 || rule.TargetSlot >= len(ch.Equip) {
			return reject("")
		}
		target := &ch.Equip[rule.TargetSlot]
		level := itemSanc(*target)
		if !containsItemIndex(rule.TargetItems, target.Index) || level >= rule.RefineMax || !setItemSanc(target, level+1) {
			return reject("")
		}
	case "nightmare_ticket":
		now := w.now()
		readyAt := time.Unix(ch.LastNightmareUnix, 0).Add(time.Duration(rule.CooldownSeconds) * time.Second)
		if ch.LastNightmareUnix > 0 && now.Before(readyAt) {
			return reject(fmt.Sprintf("Nightmare Letter available in %s.", readyAt.Sub(now).Round(time.Minute)))
		}
		if rule.Amount <= 0 || uint64(rule.Amount) > uint64(math.MaxUint32-ch.NightmareTickets) {
			return reject("Nightmare ticket limit reached.")
		}
		ch.NightmareTickets += uint32(rule.Amount)
		ch.LastNightmareUnix = now.Unix()
	case "mastery_reset":
		if ch.Score == nil || !matchesEvolution(ch, "mortal") || ch.Score.Level < rule.MinLevel ||
			ch.Score.Level >= rule.MaxLevelExclusive || questCompleted(ch, masteryResetFlag(code)) || rule.Amount <= 0 {
			return reject("")
		}
		limit := uint32(rule.Amount)
		for branch := 1; branch <= 2; branch++ {
			if ch.Score.Mastery[branch] > limit {
				ch.Score.Mastery[branch] -= limit
			} else {
				ch.Score.Mastery[branch] = 0
			}
		}
		ch.LearnedSkill &= 0xFF000000
		filterShortSkills(ch)
		markQuestCompleted(ch, masteryResetFlag(code))
	default:
		return reject("")
	}
	if rule.Consume {
		consumeOne(&ch.Inv[slot])
	}
	if rule.Action != "nightmare_ticket" {
		w.recalcPlayer(ch)
	}
	return proposed, nil
}

func (w *World) admitNativeConsumable(p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	if w.criticalTradeReserved(p) {
		return
	}
	if err := w.queueNativeConsumable(p, item, slot, rule, code); err != nil {
		// The client may have optimistically consumed one unit. Rejections
		// resynchronize the retained authoritative slot, without publishing gain.
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			return
		}
		log.Printf("[#%d] %s admission rejected: %v", p.Session.ID, rule.Action, err)
		p.Session.Send(wire.MessagePanel("Item use is unavailable. Please try again later."))
	}
}

func (w *World) queueNativeConsumable(p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) error {
	if p == nil || !p.InWorld || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
		int(slot) >= model.PlayerCarrySlots || item != &p.Char.Inv[slot] || item.Index == 0 || !criticalNativeConsumable(rule.Action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed, err := w.prepareNativeConsumable(p, slot, rule, code)
	if err != nil {
		return err
	}
	before := accountStateSnapshot(p.Account)
	prepared := &proposed.Chars[p.CharSlot]
	participants := []*Player{p}
	accounts := []*model.Account{proposed}
	trade := p.Trade
	var peer *Player
	var peerTrade *TradeState
	var peerBefore *model.Account
	var offer, peerOffer TradeState
	if trade != nil {
		peer = w.playerByID(trade.OpponentID)
		if peer == nil || peer == p || !peer.InWorld || peer.Account == nil || peer.Trade == nil || peer.Trade.OpponentID != p.ID {
			return errCriticalPersistenceIdentity
		}
		peerTrade, peerBefore = peer.Trade, accountStateSnapshot(peer.Account)
		offer, peerOffer = *trade, *peerTrade
		participants = append(participants, peer)
		accounts = append(accounts, peerBefore)
	}
	checkRetained := func() error {
		if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade ||
			(trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) {
			return errCriticalPersistenceIdentity
		}
		return nil
	}
	_, err = w.submitCriticalWriteWithRejection(criticalWrite{accounts: accounts}, participants, false,
		func(criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			ch := p.Char
			ch.Inv[slot] = prepared.Inv[slot]
			switch rule.Action {
			case "arch_crystal":
				ch.ArchCrystals, ch.Exp, ch.Score.Level = prepared.ArchCrystals, prepared.Exp, prepared.Score.Level
				syncProgression(ch)
				w.recalcPlayer(ch)
			case "refine_equipped":
				ch.Equip[rule.TargetSlot] = prepared.Equip[rule.TargetSlot]
				w.recalcPlayer(ch)
			case "nightmare_ticket":
				ch.NightmareTickets, ch.LastNightmareUnix = prepared.NightmareTickets, prepared.LastNightmareUnix
			case "mastery_reset":
				ch.Score.Mastery = prepared.Score.Mastery
				ch.LearnedSkill, ch.ShortSkill = prepared.LearnedSkill, prepared.ShortSkill
				ch.QuestsDone = append([]int32(nil), prepared.QuestsDone...)
				w.recalcPlayer(ch)
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if trade != nil {
				p.Trade, peer.Trade = nil, nil
				for _, participant := range participants {
					if !participant.Session.IsClosed() {
						participant.Session.Send(wire.CloseTrade(participant.ID))
					}
				}
			}
			if p.Session.IsClosed() {
				return nil
			}
			ch, session := p.Char, p.Session
			switch rule.Action {
			case "arch_crystal":
				session.Send(wire.SendItem(p.ID, placeInv, slot, ch.Inv[slot]))
				session.Send(playerScorePacketForChar(p, ch))
				session.Send(wire.UpdateEtc(p.ID, *ch))
				w.syncPlayerVitalsToObservers(p)
				if ch.ArchCrystals == archCrystalCount {
					session.Send(wire.MessagePanel("The four oaths are complete."))
				}
				log.Printf("[#%d] Arch crystal oath=%d/%d item=%d volatile=%d experience=%d", session.ID,
					ch.ArchCrystals, archCrystalCount, before.Chars[p.CharSlot].Inv[slot].Index, code, ch.Exp)
			case "refine_equipped":
				session.Send(wire.SendItem(p.ID, placeEquip, byte(rule.TargetSlot), ch.Equip[rule.TargetSlot]))
				session.Send(wire.SendItem(p.ID, placeInv, slot, ch.Inv[slot]))
				session.Send(playerScorePacket(p))
				w.refreshAppearance(p)
				session.Send(wire.MessagePanel("Refine successful!"))
			case "nightmare_ticket":
				session.Send(wire.SendItem(p.ID, placeInv, slot, ch.Inv[slot]))
				session.Send(wire.MessagePanel(fmt.Sprintf("%d Nightmare entries received.", rule.Amount)))
			case "mastery_reset":
				session.Send(playerScorePacketForChar(p, ch))
				session.Send(wire.UpdateEtc(p.ID, *ch))
				session.Send(wire.SetShortSkill(p.ID, ch.ShortSkill))
				session.Send(wire.SendItem(p.ID, placeInv, slot, ch.Inv[slot]))
				session.Send(wire.MessagePanel("Skills initialized."))
			}
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			// No mutation or trade cancellation occurred. Restore only the used
			// slot that the client may have optimistically consumed, then allow
			// a new intention after actor acknowledgement. Never replay this one.
			if !p.Session.IsClosed() {
				resyncUsedItem(p.Session, p, slot)
				p.Session.Send(wire.MessagePanel("Item use could not be saved. Please try again later."))
			}
			log.Printf("[#%d] %s persistence rejected: %v", p.Session.ID, rule.Action, result.err)
			return nil
		})
	return err
}
