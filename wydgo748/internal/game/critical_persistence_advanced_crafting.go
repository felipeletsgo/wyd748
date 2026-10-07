package game

import (
	"errors"
	"log"
	"reflect"
	"slices"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

var errAdvancedRecipe = errors.New("critical composition: invalid advanced recipe")

type advancedCraftPlan struct {
	account *model.Account
	coins   map[string]uint32
	inv     map[int]struct{}
	equip   map[int]struct{}
	code    uint32
	message string
	sidecar bool
}

// Prepare every recipe on an isolated aggregate. UID creation and probability
// decisions happen once, on World, never on workers or completion delivery.
func (w *World) prepareAdvancedCraft(p *Player, req combineRequest, artisan string) (*advancedCraftPlan, error) {
	plan := &advancedCraftPlan{account: accountStateSnapshot(p.Account), coins: copyCounters(p),
		inv: make(map[int]struct{}), equip: make(map[int]struct{}), code: 1}
	ch := &plan.account.Chars[p.CharSlot]
	detached := &Player{Char: ch, SpecialCoins: plan.coins}
	consume := func(from, to int) { consumeCombineItems(ch, req, from, to, plan.inv) }
	put := func(pos int, item model.Item) error {
		if !w.putCraftResult(detached, pos, item, plan.inv) {
			return errAdvancedRecipe
		}
		return nil
	}
	decide := func(chance int) bool {
		roll := w.rollPercent(chance)
		plan.message = roll.message()
		if !roll.Success {
			plan.code = 2
		}
		return roll.Success
	}
	if artisan == "Ehre" {
		if odinRune(req.Items[0].Index) && odinRune(req.Items[1].Index) &&
			req.Items[2].Index == 413 && itemStackAmount(req.Items[2]) >= 10 &&
			exactRecipeTailEmpty(req, 3) {
			consume(0, 2)
			item := model.Item{Index: mysteriousStoneItem}
			setItemAmount(&item, 10)
			return plan, put(int(req.Pos[2]), item)
		}
		for index, recipe := range soulRecipes {
			if exactRecipe(req, recipe[:]) {
				consume(0, 2)
				ch.SoulInfo = uint8(index + 1)
				return plan, nil
			}
		}
		if !exactRecipe(req, []uint16{697, 697, 3338}) || itemSanc(req.Items[2]) >= 9 ||
			!advancedEvolution(ch, "celestial", "subcelestial") || playerLevel(ch) < 39 || ch.Exp < 5_000_000 {
			return nil, errAdvancedRecipe
		}
		level, chance := int(playerLevel(ch)), 100
		switch {
		case level <= 149:
			chance = 30
		case level <= 159:
			chance = 35
		case level <= 169:
			chance = 40
		case level <= 179:
			chance = 50
		case level <= 189:
			chance = 70
		}
		success, target := decide(chance), req.Items[2]
		if success && !setItemSanc(&target, itemSanc(target)+1) {
			return nil, errAdvancedRecipe
		}
		consume(0, 2)
		if success {
			ch.Inv[int(req.Pos[2])] = target
		}
		ch.Exp -= 5_000_000
		return plan, nil
	}
	if artisan != "Odin" {
		return nil, errAdvancedRecipe
	}
	var output uint16
	chance := odinSecretStoneRandomChance
	for _, recipe := range secretStoneRecipes {
		if exactRecipe(req, recipe[:7]) {
			output, chance = recipe[7], odinSecretStoneCorrectChance
			break
		}
	}
	// Fury takes precedence over the seven-rune random fallback.
	if exactRecipe(req, []uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}) {
		consume(0, 6)
		return plan, put(int(req.Pos[0]), model.Item{Index: 3020})
	}
	if sevenOdinRunes(req) {
		if ch.Gold < odinSecretStoneGoldCost {
			return nil, errAdvancedRecipe
		}
		success := decide(chance)
		if success && output == 0 {
			output = uint16(5334 + w.intn(4))
		}
		consume(0, 6)
		ch.Gold -= odinSecretStoneGoldCost
		if success {
			return plan, put(int(req.Pos[0]), model.Item{Index: output})
		}
		return plan, nil
	}
	for _, recipe := range []struct {
		inputs []uint16
		output uint16
	}{
		{[]uint16{413, 413, 413, 413, 413, 413, 413}, 5134},
		{[]uint16{421, 422, 423, 424, 425, 426, 427}, 4032},
	} {
		if exactRecipe(req, recipe.inputs) {
			consume(0, 6)
			return plan, put(int(req.Pos[0]), model.Item{Index: recipe.output})
		}
	}
	if exactRecipe(req, []uint16{4127, 4127, 5135, 413, 413, 413, 413}) &&
		ch.Equip[15].Index >= 3197 && ch.Equip[15].Index <= 3199 &&
		advancedEvolution(ch, "celestial", "subcelestial") && itemSanc(ch.Equip[15]) <= 9 {
		cape := ch.Equip[15]
		if !setItemSanc(&cape, itemSanc(cape)+1) {
			return nil, errAdvancedRecipe
		}
		consume(0, 6)
		ch.Equip[15], plan.equip[15] = cape, struct{}{}
		w.recalcPlayer(ch)
		return plan, nil
	}
	if exactRecipe(req, []uint16{4127, 4127, 5135, 5113, 5129, 5112, 5110}) &&
		advancedEvolution(ch, "celestial") && playerLevel(ch) == 39 &&
		!ch.CelestialLevel40Unlocked && counterBalance(detached, fameCounter) >= 200 && ch.UID != "" {
		consume(0, 6)
		if !spendCounters(detached, map[string]uint32{fameCounter: 200}) {
			return nil, errAdvancedRecipe
		}
		ch.CelestialLevel40Unlocked, plan.sidecar = true, true
		return plan, nil
	}
	if req.Items[0].Index != 4043 || req.Items[1].Index != 4043 || req.Items[2].Index == 0 ||
		req.Items[7].Index != 0 || odinTargetBlocked(req.Items[2].Index) {
		return nil, errAdvancedRecipe
	}
	level := itemSanc(req.Items[2])
	if level < 11 || level > 14 {
		return nil, errAdvancedRecipe
	}
	chance = odinRefineBonus(level)
	for i := 3; i <= 6; i++ {
		if req.Items[i].Index == 3338 {
			chance += itemSanc(req.Items[i]) + 2
		} else if req.Items[i].Index != 0 {
			return nil, errAdvancedRecipe
		}
	}
	newLevel := level - 1
	if decide(chance) {
		newLevel = level + 1
	}
	target := req.Items[2]
	if !setItemSanc(&target, newLevel) {
		return nil, errAdvancedRecipe
	}
	consume(0, 1)
	consume(3, 6)
	ch.Inv[int(req.Pos[2])], plan.inv[int(req.Pos[2])] = target, struct{}{}
	return plan, nil
}

func exactRecipeTailEmpty(req combineRequest, from int) bool {
	for _, item := range req.Items[from:] {
		if item.Index != 0 {
			return false
		}
	}
	return true
}

func (w *World) admitAdvancedComposition(p *Player, req combineRequest, artisan string) {
	if err := w.queueAdvancedComposition(p, req, artisan); err != nil {
		if errors.Is(err, errAdvancedRecipe) {
			w.sendCombineResult(p, 0)
			return
		}
		log.Printf("[#%d] %s composition admission rejected: %v", p.Session.ID, artisan, err)
		p.Session.Send(wire.MessagePanel("Composition is unavailable. Please try again later."))
	}
}

func (w *World) queueAdvancedComposition(p *Player, req combineRequest, artisan string) error {
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] || p.Char.Score == nil {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	used := make(map[int]bool)
	for i, item := range req.Items {
		if item.Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		if pos < 0 || pos >= model.PlayerCarrySlots || used[pos] || p.Char.Inv[pos] != item || item.Index == blockedCombineItem {
			return errAdvancedRecipe
		}
		used[pos] = true
	}
	plan, err := w.prepareAdvancedCraft(p, req, artisan)
	if err != nil {
		return err
	}
	before, coins := accountStateSnapshot(p.Account), copyCounters(p)
	old := &before.Chars[p.CharSlot]
	prepared := &plan.account.Chars[p.CharSlot]
	write := criticalWrite{accounts: []*model.Account{plan.account}}
	if plan.sidecar {
		write.character = &criticalCharacterState{uid: prepared.UID,
			state: buildCharState(&Player{Char: prepared, SpecialCoins: plan.coins}, w.now())}
	}
	participants := []*Player{p}
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
		write.accounts = append(write.accounts, peerBefore)
	}
	positions := make([]int, 0, len(plan.inv))
	for pos := range plan.inv {
		positions = append(positions, pos)
	}
	slices.Sort(positions)
	_, err = w.submitCriticalWritePhased(write, participants, false,
		func(criticalPersistenceResult) error {
			ch := p.Char
			if ch.Score == nil || ch.Inv != old.Inv || ch.Equip != old.Equip || ch.Gold != old.Gold || ch.Exp != old.Exp ||
				ch.SoulInfo != old.SoulInfo || ch.Evolution != old.Evolution || playerLevel(ch) != playerLevel(old) ||
				ch.CelestialLevel40Unlocked != old.CelestialLevel40Unlocked || !reflect.DeepEqual(p.SpecialCoins, coins) ||
				p.Trade != trade || (trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) {
				return errCriticalPersistenceIdentity
			}
			for _, pos := range positions {
				ch.Inv[pos] = prepared.Inv[pos]
			}
			ch.Gold, ch.Exp, ch.SoulInfo = prepared.Gold, prepared.Exp, prepared.SoulInfo
			ch.CelestialLevel40Unlocked = prepared.CelestialLevel40Unlocked
			for pos := range plan.equip {
				ch.Equip[pos] = prepared.Equip[pos]
			}
			if len(plan.equip) != 0 {
				w.recalcPlayer(ch)
			}
			if plan.sidecar {
				p.SpecialCoins = plan.coins
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if !p.Session.IsClosed() {
				for _, pos := range positions {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
				}
				for pos := range plan.equip {
					p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
				}
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				if len(plan.equip) != 0 {
					w.syncPlayerScoreAndVitals(p)
				}
				w.sendCombineResultMessage(p, plan.code, plan.message)
			}
			return nil
		})
	return err
}
