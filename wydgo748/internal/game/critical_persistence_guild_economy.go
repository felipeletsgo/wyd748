package game

import (
	"fmt"
	"log"
	"reflect"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func (w *World) criticalGuildSnapshot() *model.GuildRegistry {
	return w.guilds.Clone()
}

// The actor prepares a private payout and guild debit. The worker sees neither
// live state nor sessions; every response follows durable commit and actor ack.
func (w *World) queueCityTreasury(p *Player, city int) error {
	if w.guildFameWars() {
		return fmt.Errorf("gold treasury collection is disabled for guild-fame wars")
	}
	if w.criticalWrites == nil || w.guilds == nil || p == nil || p.Account == nil || p.Char == nil ||
		!p.InWorld || p.Session == nil || p.Session.IsClosed() || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
		city < 0 || city >= len(cityWarZones) || !bindCriticalPlayer(p).current(w) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) || w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	q := w.criticalWrites.queue
	if w.shuttingDown || q.stopping {
		return errCriticalPersistenceStopping
	}
	if len(q.pending) >= q.capacity {
		return errCriticalPersistenceFull
	}
	if _, ok := q.store.(criticalGameStateStore); !ok {
		return errCriticalPersistenceUnsupported
	}
	// Keep this internal entry point subject to the same domain checks as the
	// collector handler; no alternative caller may bypass Saturday or ownership.
	if w.cityWarLocal(w.now()).Weekday() == time.Saturday {
		return fmt.Errorf("treasury collection is closed during Saturday registration")
	}
	guild, err := w.requireGuildLeader(p.Char)
	if err != nil {
		return err
	}
	territory := w.guilds.Wars.Cities.Territories[city]
	if territory.Owner != guild.ID || territory.Treasury == 0 || territory.Treasury > model.GuildWarTreasuryCap {
		return fmt.Errorf("city treasury has no collectible balance for this owner")
	}
	before, proposed := accountStateSnapshot(p.Account), accountStateSnapshot(p.Account)
	registry := w.guilds
	retained, proposedGuilds := w.criticalGuildSnapshot(), w.criticalGuildSnapshot()
	participants, accounts := []*Player{p}, []*model.Account{proposed}
	trade := p.Trade
	var peer *Player
	var peerTrade *TradeState
	var peerBefore *model.Account
	var offer, peerOffer TradeState
	if trade != nil {
		peer = w.playerByID(trade.OpponentID)
		if peer == nil || peer == p || !peer.InWorld || peer.Account == nil || peer.Session == nil || peer.Session.IsClosed() ||
			peer.Trade == nil || peer.Trade.OpponentID != p.ID || !bindCriticalPlayer(peer).current(w) ||
			accountSessionKey(peer.Account.Name) == accountSessionKey(p.Account.Name) {
			return errCriticalPersistenceIdentity
		}
		peerTrade, peerBefore = peer.Trade, accountStateSnapshot(peer.Account)
		offer, peerOffer = *trade, *peerTrade
		participants, accounts = append(participants, peer), append(accounts, peerBefore)
	}
	prepared := &proposed.Chars[p.CharSlot]
	amount, units := territory.Treasury, 0
	if amount < cityWarChequeValue {
		if uint64(prepared.Gold)+amount > uint64(maxCharacterGold) {
			return fmt.Errorf("not enough room for the treasury gold")
		}
		prepared.Gold += uint32(amount)
	} else {
		limit := amount / cityWarChequeValue
		for slot := 0; slot < model.PlayerCarrySlots && uint64(units) < limit; slot++ {
			if prepared.Inv[slot].Index != 0 {
				continue
			}
			item, err := materializeItem(model.Item{Index: cityWarChequeItem})
			if err != nil {
				return fmt.Errorf("create treasury cheque: %w", err)
			}
			prepared.Inv[slot] = item
			units++
		}
		if units == 0 {
			return fmt.Errorf("inventory is full for treasury cheques")
		}
		amount = uint64(units) * cityWarChequeValue
	}
	remaining := territory.Treasury - amount
	proposedGuilds.Wars.Cities.Territories[city].Treasury = remaining
	checkRetained := func() error {
		if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
			!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade ||
			(trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) {
			return errCriticalPersistenceIdentity
		}
		return nil
	}
	_, err = w.submitCriticalWriteWithRejection(criticalWrite{guilds: proposedGuilds, accounts: accounts}, participants, false,
		func(criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			p.Char.Gold, p.Char.Inv = prepared.Gold, prepared.Inv
			w.guilds.Wars.Cities.Territories[city].Treasury = remaining
			return nil
		}, func(criticalPersistenceResult) error {
			if !p.Session.IsClosed() {
				p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				message := fmt.Sprintf("Collected %d gold from the city treasury.", amount)
				if units != 0 {
					message = fmt.Sprintf("Collected %d billion-gold cheque(s) from the city treasury.", units)
				}
				p.Session.Send(wire.MessagePanel(message))
			}
			log.Printf("[#%d] collected city treasury %s: gold=%d cheques=%d remaining=%d", p.Session.ID,
				cityWarZones[city].name, amount, units, remaining)
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			if !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel("The city treasury collection could not be completed."))
			}
			log.Printf("[#%d] city treasury persistence rejected: %v", p.Session.ID, result.err)
			return nil
		})
	return err
}
