package game

import (
	"errors"
	"log"
	"reflect"
	"slices"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Lindy prepares inventory, unlock flags and optional Fame on detached state.
// The reciprocal trade remains open, but both accounts stay reserved until ack.
func (w *World) queueLindyCraft(p *Player, req combineRequest, cape model.Item) error {
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
		if pos < 0 || pos >= model.PlayerCarrySlots || used[pos] || p.Char.Inv[pos] != item || item.Index == blockedCombineItem {
			return errors.New("critical Lindy composition: stale or invalid ingredient")
		}
		used[pos] = true
	}
	if !lindyMaterialStack(req.Items[0]) || !lindyMaterialStack(req.Items[1]) || req.Items[2].Index != 4127 {
		return errors.New("critical Lindy composition: invalid material package")
	}
	for i := 3; i <= 6; i++ {
		if req.Items[i].Index != 413 || itemStackAmount(req.Items[i]) > 1 {
			return errors.New("critical Lindy composition: invalid powder")
		}
	}
	level, valid := lindyLevelUnlock(p.Char)
	if !valid {
		return errors.New("critical Lindy composition: no locked Arch limit")
	}
	if level == archLockLevel355 {
		_, exists := w.items[cape.Index]
		if !exists || cape.Index != lindyCapeIndex(p.Char) || cape.UID == "" ||
			(p.Char.Equip[15].UID != "" && cape.UID != p.Char.Equip[15].UID) ||
			cape.Eff != [6]byte{54, 16} || cape.ActivatedUnix != 0 || cape.ExpiresUnix != 0 {
			return errors.New("critical Lindy composition: invalid prepared cape")
		}
	} else if cape != (model.Item{}) || counterBalance(p, fameCounter) < 1 || p.Char.UID == "" {
		return errors.New("critical Lindy composition: invalid Fame or character identity")
	}

	oldInv, oldCape, oldGold := p.Char.Inv, p.Char.Equip[15], p.Char.Gold
	old355, old370, oldEvolution := p.Char.ArchLevel355, p.Char.ArchLevel370, p.Char.Evolution
	oldCoins := copyCounters(p)
	proposed := accountStateSnapshot(p.Account)
	prepared := &proposed.Chars[p.CharSlot]
	coins := copyCounters(p)
	changed := make(map[int]struct{}, 7)
	consumeCombineItems(prepared, req, 0, 6, changed)
	write := criticalWrite{accounts: []*model.Account{proposed}}
	if level == archLockLevel355 {
		prepared.ArchLevel355, prepared.Equip[15] = true, cape
		w.recalcPlayer(prepared)
	} else {
		prepared.ArchLevel370 = true
		detached := &Player{Char: prepared, SpecialCoins: coins}
		if !spendCounters(detached, map[string]uint32{fameCounter: 1}) {
			return errors.New("critical Lindy composition: insufficient Fame")
		}
		write.character = &criticalCharacterState{uid: prepared.UID, state: buildCharState(detached, w.now())}
	}
	positions := make([]int, 0, len(changed))
	for pos := range changed {
		positions = append(positions, pos)
	}
	slices.Sort(positions)
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
		peerTrade = peer.Trade
		offer, peerOffer = *trade, *peerTrade
		peerBefore = accountStateSnapshot(peer.Account)
		participants = append(participants, peer)
		write.accounts = append(write.accounts, peerBefore)
	}
	_, err := w.submitCriticalWritePhased(write, participants, false,
		func(criticalPersistenceResult) error {
			if p.Char.Inv != oldInv || p.Char.Equip[15] != oldCape || p.Char.Gold != oldGold ||
				p.Char.ArchLevel355 != old355 || p.Char.ArchLevel370 != old370 || p.Char.Evolution != oldEvolution ||
				p.Char.Score == nil || p.Char.Score.Level != level || !reflect.DeepEqual(p.SpecialCoins, oldCoins) ||
				p.Trade != trade || (trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) {
				return errCriticalPersistenceIdentity
			}
			for _, pos := range positions {
				p.Char.Inv[pos] = prepared.Inv[pos]
			}
			if level == archLockLevel355 {
				p.Char.ArchLevel355, p.Char.Equip[15] = true, cape
				w.recalcPlayer(p.Char)
			} else {
				p.Char.ArchLevel370, p.SpecialCoins = true, coins
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if !p.Session.IsClosed() {
				for _, pos := range positions {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
				}
				if level == archLockLevel355 {
					p.Session.Send(wire.SendItem(p.ID, placeEquip, 15, cape))
				}
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				if level == archLockLevel355 {
					w.syncPlayerScoreAndVitals(p)
				}
				w.sendCombineResult(p, 1)
				p.Session.Send(wire.MessagePanel("Your level limit has been lifted."))
			}
			log.Printf("[#%d] ARCH unlocked level %d (fame=%d)", p.Session.ID, level+1, counterBalance(p, fameCounter))
			return nil
		})
	return err
}
