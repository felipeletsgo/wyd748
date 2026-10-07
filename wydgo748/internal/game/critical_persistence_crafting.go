package game

import (
	"errors"
	"log"
	"reflect"
	"slices"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Tiny changes inventory and gold only. Plan once on a detached aggregate;
// neither admission nor a worker may mutate live state or publish the roll.
func (w *World) queueTinyCraft(p *Player, req combineRequest, resultItem model.Item, roll percentRoll) error {
	return w.queueInventoryComposition(p, req, resultItem, roll, "Tiny")
}

// Inventory-only artisans share persistence ownership, not recipe semantics.
// Tiny and skill crafting cancel trade only after acknowledgement; other
// artisans retain it.
func (w *World) queueInventoryComposition(p *Player, req combineRequest, resultItem model.Item, roll percentRoll, artisan string) error {
	if p == nil || !p.InWorld || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	used := make(map[int]bool, combineSlots)
	for i, item := range req.Items {
		if item.Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		if pos < 0 || pos >= model.PlayerCarrySlots || used[pos] || p.Char.Inv[pos] != item ||
			(item.Index == blockedCombineItem && artisan != "Extracao") {
			return errors.New("critical composition: stale or invalid ingredient")
		}
		used[pos] = true
	}
	var expected model.Item
	var chance, from, to int
	var cost uint32
	var valid bool
	var breakdown string
	skill := artisan == "Alquimia" || artisan == "Extracao"
	var skillInputs [4]uint32
	if skill {
		index := 84
		if artisan == "Extracao" {
			index = 83
		}
		if !skillCraftAllowed(p, index) {
			return errCriticalPersistenceIdentity
		}
		skillInputs = [4]uint32{uint32(p.Char.Class), p.Char.LearnedSkill,
			p.Char.SecondaryLearnedSkill, uint32(playerMastery(p.Char, 2))}
	}
	switch artisan {
	case "Tiny":
		var err error
		expected, chance, err = w.tinyRecipe(req)
		valid, from, to, cost = err == nil, 2, 7, tinyGoldCost
	case "Compositor":
		expected, chance, breakdown, valid = w.compositorRecipe(req)
		from, to = 0, 7
	case "Agatha":
		expected, chance, valid = w.agathaRecipe(req)
		from, to = 2, 5
	case "Aylin":
		expected, chance, valid = w.aylinRecipe(req)
		from, to, cost = 2, 6, aylinGoldCost
	case "Alquimia":
		expected, chance, valid = alchemyResult(p, req)
		from, to = 0, 7
	case "Extracao":
		var catalyst int
		expected, chance, catalyst, valid = w.extractionRecipe(p, int(req.Pos[0]))
		valid = valid && req.Items[0].Index != 0 && catalyst == int(req.Pos[1]) && req.Items[1].Index == extracaoCatalyst
		for _, item := range req.Items[2:] {
			valid = valid && item.Index == 0
		}
		from, to = 0, 1
	default:
		return errors.New("critical composition: unsupported artisan")
	}
	success := roll.Roll <= roll.Chance
	if artisan == "Extracao" {
		success = roll.Roll < roll.Chance
	}
	if !valid || expected != resultItem || roll.Chance != clampInt(chance, 0, 100) || roll.Roll < 1 || roll.Roll > 100 ||
		roll.Success != success || p.Char.Gold < cost {
		return errors.New("critical composition: invalid prepared result or balance")
	}
	oldInv, oldGold := p.Char.Inv, p.Char.Gold
	proposed := accountStateSnapshot(p.Account)
	prepared := &proposed.Chars[p.CharSlot]
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(prepared, req, from, to, changed)
	if artisan == "Agatha" {
		main := int(req.Pos[0])
		prepared.Inv[main] = model.Item{}
		changed[main] = struct{}{}
	}
	code := uint32(2)
	if roll.Success {
		main, donor := int(req.Pos[0]), int(req.Pos[1])
		prepared.Inv[main] = resultItem
		changed[main] = struct{}{}
		if artisan != "Compositor" {
			prepared.Inv[donor] = model.Item{}
			changed[donor] = struct{}{}
		}
		code = 1
	}
	prepared.Gold -= cost
	positions := make([]int, 0, len(changed))
	for pos := range changed {
		positions = append(positions, pos)
	}
	slices.Sort(positions)

	participants := []*Player{p}
	accounts := []*model.Account{proposed}
	trade := p.Trade
	var peer *Player
	var peerTrade *TradeState
	var peerBefore *model.Account
	var offer, peerOffer TradeState
	if trade != nil {
		peer = w.playerByID(trade.OpponentID)
		if peer == nil || peer == p || !peer.InWorld || peer.Account == nil || peer.Trade == nil ||
			peer.Trade.OpponentID != p.ID {
			return errCriticalPersistenceIdentity
		}
		peerTrade = peer.Trade
		offer, peerOffer = *trade, *peerTrade
		peerBefore = accountStateSnapshot(peer.Account)
		participants = append(participants, peer)
		accounts = append(accounts, peerBefore)
	}
	_, err := w.submitCriticalWritePhased(criticalWrite{accounts: accounts}, participants, false,
		func(criticalPersistenceResult) error {
			// Validate retained economic and shared contexts before installing any
			// field. Account/session generations are checked by the owner binding.
			if skill && (p.Char.Score == nil || skillInputs != [4]uint32{uint32(p.Char.Class),
				p.Char.LearnedSkill, p.Char.SecondaryLearnedSkill, uint32(playerMastery(p.Char, 2))}) {
				return errCriticalPersistenceIdentity
			}
			if p.Char.Inv != oldInv || p.Char.Gold != oldGold || p.Trade != trade ||
				(trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
					!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) {
				return errCriticalPersistenceIdentity
			}
			for _, pos := range positions {
				p.Char.Inv[pos] = prepared.Inv[pos]
			}
			p.Char.Gold = prepared.Gold
			return nil
		}, func(criticalPersistenceResult) error {
			// Acknowledgement and publication share one actor delivery. No tick,
			// trade completion or disconnect cleanup can interleave here.
			if trade != nil && (artisan == "Tiny" || skill) {
				p.Trade, peer.Trade = nil, nil
				for _, participant := range participants {
					if participant.InWorld && !participant.Session.IsClosed() {
						participant.Session.Send(wire.CloseTrade(participant.ID))
					}
				}
				log.Printf("TRADE canceled for player=%d: %s composition", p.ID, artisan)
			}
			if !p.Session.IsClosed() {
				for _, pos := range positions {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), prepared.Inv[pos]))
				}
				if artisan != "Extracao" {
					p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
					w.sendCombineResultMessage(p, code, roll.message())
				}
			}
			if artisan == "Tiny" {
				logCraftResult(p.Session, roll, p.Char.Gold)
			} else if artisan == "Compositor" {
				log.Printf("[#%d] CRAFT Compositor success=%t roll=%d/%d (base %d + %s)",
					p.Session.ID, roll.Success, roll.Roll, roll.Chance, w.gameplay.CompositorBaseChance, breakdown)
			}
			return nil
		})
	return err
}
