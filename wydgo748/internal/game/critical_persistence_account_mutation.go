package game

import (
	"log"
	"maps"
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type criticalAccountMutationHooks struct {
	check  func() error
	reject func()
	// Lifecycle cleanup must run even when the transport closed while saving.
	afterAck func()
}

// A producer prepares its account on the actor and installs only its changed
// fields after acknowledgement. Reciprocal trades retain both account snapshots
// and offers; admission and certified rejection never cancel a live trade.
func (w *World) queueCriticalAccountMutation(p *Player, proposed *model.Account, operation, failure string,
	apply, publish func(), cancelTrade ...bool) error {
	return w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, operation, failure, apply, publish,
		len(cancelTrade) != 0 && cancelTrade[0], criticalAccountMutationHooks{})
}

func (w *World) queueCriticalAccountWrite(p *Player, proposed *model.Account, write criticalWrite,
	operation, failure string, apply, publish func(), cancelTrade bool, hooks criticalAccountMutationHooks) error {
	if p == nil || p.Account == nil || p.Char == nil || proposed == nil || proposed == p.Account || apply == nil ||
		!bindCriticalPlayer(p).current(w) || accountSessionKey(proposed.Name) != accountSessionKey(p.Account.Name) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	participants, accounts := []*Player{p}, []*model.Account{proposed}
	before, trade := accountStateSnapshot(p.Account), p.Trade
	coinsBefore := maps.Clone(p.SpecialCoins)
	var peer *Player
	var peerBefore *model.Account
	var peerCoins map[string]uint32
	var peerTrade *TradeState
	var offer, peerOffer TradeState
	if trade != nil {
		peer = w.playerByID(trade.OpponentID)
		if peer == nil || peer == p || peer.Account == nil || !bindCriticalPlayer(peer).current(w) ||
			peer.Trade == nil || peer.Trade.OpponentID != p.ID {
			return errCriticalPersistenceIdentity
		}
		peerBefore, peerTrade = accountStateSnapshot(peer.Account), peer.Trade
		peerCoins = maps.Clone(peer.SpecialCoins)
		offer, peerOffer = *trade, *peerTrade
		participants, accounts = append(participants, peer), append(accounts, peerBefore)
	}
	check := func() error {
		if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !maps.Equal(p.SpecialCoins, coinsBefore) || p.Trade != trade ||
			(trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || !maps.Equal(peer.SpecialCoins, peerCoins))) {
			return errCriticalPersistenceIdentity
		}
		if hooks.check != nil {
			return hooks.check()
		}
		return nil
	}
	if err := check(); err != nil {
		return err
	}
	write.accounts = accounts
	_, err := w.submitCriticalWriteWithRejection(write, participants, false,
		func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			apply()
			return nil
		}, func(criticalPersistenceResult) error {
			if cancelTrade && trade != nil {
				p.Trade, peer.Trade = nil, nil
				for _, participant := range participants {
					if !participant.Session.IsClosed() {
						participant.Session.Send(wire.CloseTrade(participant.ID))
					}
				}
			}
			if hooks.afterAck != nil {
				hooks.afterAck()
			}
			if !p.Session.IsClosed() && publish != nil {
				publish()
			}
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			if hooks.reject != nil {
				hooks.reject()
			}
			if !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel(failure))
				w.resendInventory(p)
			}
			log.Printf("[#%d] %s persistence rejected: %v", p.Session.ID, operation, result.err)
			return nil
		})
	return err
}
