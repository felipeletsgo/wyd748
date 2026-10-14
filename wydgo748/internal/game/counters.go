package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// kefraTicketCounter holds the character's remaining Kefra hall entries.
// Survivor exchanges item 4127 for the allowance configured in quests.json.
const kefraTicketCounter = "kefra_ticket"

// fameCounter holds character fame, granted by Warrior's Seal (4146).
// Operations spending counters and account items must persist both aggregates
// atomically, using the account/character-state transaction helpers.
const fameCounter = "fame"

// Named counters share Player.SpecialCoins, persisted by Char.UID. Loading a
// character replaces this map rather than merging another character's balance.
// Reuse this representation instead of introducing a parallel allowance field.

// counterBalance returns zero for a nil player or counter map.
func counterBalance(p *Player, name string) uint32 {
	if p == nil || p.SpecialCoins == nil {
		return 0
	}
	return p.SpecialCoins[name]
}

// hasCounters requires every requested balance and reports a missing counter
// so callers can present a specific rejection.
func hasCounters(p *Player, required map[string]uint32) (string, bool) {
	for name, amount := range required {
		if counterBalance(p, name) < amount {
			return name, false
		}
	}
	return "", true
}

// spendCounters debits all balances or none. A partial charge must not survive
// when another required counter is insufficient.
func spendCounters(p *Player, cost map[string]uint32) bool {
	if len(cost) == 0 {
		return true
	}
	if _, ok := hasCounters(p, cost); !ok {
		return false
	}
	for name, amount := range cost {
		p.SpecialCoins[name] -= amount
		// Character-state persistence omits zeros; keep the live map consistent.
		if p.SpecialCoins[name] == 0 {
			delete(p.SpecialCoins, name)
		}
	}
	return true
}

// grantCounters credits balances, saturating at the maximum without wrapping.
func grantCounters(p *Player, reward map[string]uint32) {
	if p == nil || len(reward) == 0 {
		return
	}
	if p.SpecialCoins == nil {
		p.SpecialCoins = make(map[string]uint32, len(reward))
	}
	for name, amount := range reward {
		balance := p.SpecialCoins[name]
		if balance > model.MaxQuestCounter-amount {
			balance = model.MaxQuestCounter
		} else {
			balance += amount
		}
		p.SpecialCoins[name] = balance
	}
}

// copyCounters takes a detached snapshot for proposals and rollback. Retaining
// the live map would alias subsequent mutations instead of preserving its state.
func copyCounters(p *Player) map[string]uint32 {
	if p == nil || p.SpecialCoins == nil {
		return nil
	}
	c := make(map[string]uint32, len(p.SpecialCoins))
	for k, v := range p.SpecialCoins {
		c[k] = v
	}
	return c
}

// useCounterGrant implements the volatile "grant_counter" action. The item and
// balance are saved atomically before confirmation; a failed save restores both.
// Survivor's Locked Seal exchange is a quest recharge, not this item-use action.
func (w *World) useCounterGrant(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	if len(rule.Counters) == 0 {
		log.Printf("[#%d] grant_counter has no counters for item %d", s.ID, item.Index)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	before := *item
	countersBefore := copyCounters(p)

	grantCounters(p, rule.Counters)
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*item = before
		p.SpecialCoins = countersBefore
		log.Printf("[#%d] save counter for item %d failed: %v", s.ID, before.Index, err)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	for name, amount := range rule.Counters {
		log.Printf("[#%d] counter %q +%d = %d (item %d volatile %d)",
			s.ID, name, amount, counterBalance(p, name), before.Index, code)
	}
}

// useCounterGrantOnce records a permanent unlock. Repeated requests do not
// consume another item because authoritative state takes precedence over replay.
func (w *World) useCounterGrantOnce(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	if len(rule.Counters) == 0 {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	for name, required := range rule.Counters {
		if counterBalance(p, name) >= required {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("This unlock is already active."))
			return
		}
	}

	charBefore := cloneCharacterState(p.Char)
	countersBefore := copyCounters(p)
	if rule.Consume {
		consumeOne(item)
	}
	rewardSlots := make([]int, 0, len(rule.RewardItems))
	for _, rewardID := range rule.RewardItems {
		reward, err := materializeItem(model.Item{Index: rewardID})
		if err != nil {
			*p.Char, p.SpecialCoins = charBefore, countersBefore
			s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
			return
		}
		rewardSlot := firstFreeVisibleInventorySlot(p.Char)
		if rewardSlot < 0 {
			*p.Char, p.SpecialCoins = charBefore, countersBefore
			s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
			s.Send(wire.MessagePanel("Inventory full."))
			return
		}
		p.Char.Inv[rewardSlot] = reward
		rewardSlots = append(rewardSlots, rewardSlot)
	}
	grantCounters(p, rule.Counters)
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*p.Char, p.SpecialCoins = charBefore, countersBefore
		s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
	for _, rewardSlot := range rewardSlots {
		s.Send(wire.SendItem(
			p.ID, placeInv, byte(rewardSlot), p.Char.Inv[rewardSlot]))
	}
	s.Send(wire.MessagePanel("Event access enabled."))
}

// rechargeCovers lets an NPC offer the exchange when its recharge can cover a
// missing counter. A zero balance must not reject a player who brought the seal.
func rechargeCovers(p *Player, ch *model.Char, quest *model.QuestDef, missing string) bool {
	r := quest.Recharge
	if r == nil || r.Counter != missing {
		return false
	}
	if countInventoryItem(ch, r.Item) < 1 {
		return false
	}
	return counterBalance(p, r.Counter)+r.Amount >= quest.ConsumeCounters[r.Counter]
}

// applyQuestRecharge exchanges one item only when its balance is exhausted.
// Players with remaining entries keep their seal until another recharge is needed.
func applyQuestRecharge(p *Player, quest *model.QuestDef) bool {
	r := quest.Recharge
	if r == nil || p == nil || p.Char == nil {
		return false
	}
	if counterBalance(p, r.Counter) > 0 {
		return false
	}
	if !consumeInventoryItem(p.Char, r.Item, 1) {
		return false
	}
	grantCounters(p, map[string]uint32{r.Counter: r.Amount})
	return true
}
