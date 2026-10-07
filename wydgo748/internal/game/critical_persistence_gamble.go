package game

import (
	"log"
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Prepare one account/instance proposal on World. Neither RNG, mutable actors
// nor sessions reach the worker. Shared pools and reciprocal trades remain
// retained until completion; an unknown result cannot pay out or unlock them.
func (w *World) queueGamble(p *Player, kind, bet uint32) error {
	if p == nil || p.Account == nil || p.Char == nil || !p.InWorld || p.Session == nil ||
		p.Session.IsClosed() || p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) ||
		p.Char != &p.Account.Chars[p.CharSlot] || playerCurHP(p.Char) == 0 ||
		(kind != 1 && kind != 2) || bet < gambleMinBet || bet > gambleMaxBet ||
		bet%gambleMinBet != 0 || p.Char.Gold < bet || w.criticalWrites == nil {
		return errCriticalPersistenceIdentity
	}
	if accountSessionKey(p.Account.Name) == "" || !bindCriticalPlayer(p).current(w) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) || w.criticalInstanceReserved() {
		return errCriticalPersistenceBusy
	}
	q := w.criticalWrites.queue
	if w.shuttingDown || q.stopping {
		return errCriticalPersistenceStopping
	}
	if len(q.pending) >= q.capacity {
		return errCriticalPersistenceFull
	}
	if _, certified := q.store.(criticalInstanceStore); !certified {
		if _, legacy := q.store.(instanceRuntimeTransactionStore); !legacy {
			return errCriticalPersistenceUnsupported
		}
	}
	if err := w.validateInstanceStateOwnership(); err != nil {
		return err
	}
	before := accountStateSnapshot(p.Account)
	proposed := accountStateSnapshot(p.Account)
	participants := []*Player{p}
	accounts := []*model.Account{proposed}
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
		participants = append(participants, peer)
		accounts = append(accounts, peerBefore)
	}
	at, generation := w.now(), w.criticalInstanceGeneration
	retained := w.instanceStateSnapshotAt(at)
	instances := w.instanceStateSnapshotAt(at)
	jackpot := uint64(instances.GambleJackpot) + uint64(bet/1000)
	if jackpot > uint64(gambleJackpotMax) {
		jackpot = uint64(gambleJackpotMax)
	}
	instances.GambleJackpot = uint32(jackpot)
	poolAdd := uint64((bet / 100) * 3)
	if instances.GamblePool >= gamblePoolMax || poolAdd >= gamblePoolMax-instances.GamblePool {
		instances.GamblePool = gamblePoolMax
	} else {
		instances.GamblePool += poolAdd
	}
	outcome := w.rollGambleWithJackpot(kind, bet, instances.GambleJackpot)
	if outcome.result[1] == 14 {
		instances.GambleJackpot = 0
	}
	poolDebit := uint64(outcome.prize / 1000)
	if poolDebit > instances.GamblePool {
		instances.GamblePool = 0
	} else {
		instances.GamblePool -= poolDebit
	}
	prepared := &proposed.Chars[p.CharSlot]
	gold, payout := gamblePayoutItems(uint64(prepared.Gold-bet) + uint64(outcome.prize))
	prepared.Gold = gold
	if _, err := materializeGamblePayout(prepared, payout); err != nil {
		p.Session.Send(wire.MessagePanel("Free more inventory slots before playing Gamble."))
		return nil // Domain rejection already delivered; nothing was admitted.
	}
	checkRetained := func() error {
		if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade ||
			(trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) ||
			w.criticalInstanceGeneration != generation || !reflect.DeepEqual(w.instanceStateSnapshotAt(at), retained) {
			return errCriticalPersistenceIdentity
		}
		return nil
	}
	_, err := w.submitCriticalWriteWithRejection(criticalWrite{accounts: accounts, instances: instances}, participants, false,
		func(criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			p.Char.Gold, p.Char.Inv = prepared.Gold, prepared.Inv
			w.gambleJackpot, w.gamblePool = instances.GambleJackpot, instances.GamblePool
			w.instanceStateDirty = false // The unchanged full aggregate was committed.
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
			if !p.Session.IsClosed() {
				p.Session.Send(wire.ResultGamble(p.ID, outcome.result, outcome.stops, int32(outcome.prize), w.gambleJackpot))
				p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			}
			log.Printf("[#%d] Gamble type=%d bet=%d prize=%d jackpot=%d", p.Session.ID, kind, bet, outcome.prize, w.gambleJackpot)
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			if !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel("The Gamble bet could not be completed."))
			}
			log.Printf("[#%d] Gamble persistence rejected: %v", p.Session.ID, result.err)
			return nil
		})
	return err
}
