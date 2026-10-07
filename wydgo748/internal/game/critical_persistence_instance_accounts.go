package game

import (
	"maps"
	"reflect"
	"slices"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Instance transactions retain the complete participant/trade closure. Only
// detached accounts and the serialized aggregate cross the worker boundary.
// Caller hooks own runtime allocation checks and field-specific installation.
func (w *World) queueCriticalInstanceAccounts(actor *Player, members []*Player,
	changes map[*Player]*model.Account, plan *criticalInstanceDeparture,
	apply, publish, release func(), checkAllocation func() error) error {
	if w.criticalWrites == nil || actor == nil || plan == nil || apply == nil {
		return errCriticalPersistenceIdentity
	}
	if w.criticalInstanceReserved() {
		return errCriticalPersistenceBusy
	}
	if err := plan.proposed.validateInstanceStateOwnership(); err != nil {
		return err
	}
	type retained struct {
		p            *Player
		account      *model.Account
		coins        map[string]uint32
		trade        *TradeState
		offer        TradeState
		party        *Party
		partyMembers []*Player
		x, y         uint16
		space        string
	}
	participants := slices.Clone(members)
	accounts := make([]*model.Account, 0, len(members)*2)
	before := make([]retained, 0, len(members)*2)
	seen := make(map[*Player]bool)
	for i := 0; i < len(participants); i++ {
		if len(participants) > criticalPersistenceMaxAccounts {
			return errCriticalPersistenceFull
		}
		p := participants[i]
		if p == nil || p.Account == nil || p.Char == nil || !p.InWorld || p.Session == nil ||
			p.Session.IsClosed() || seen[p] || !bindCriticalPlayer(p).current(w) {
			return errCriticalPersistenceIdentity
		}
		if w.criticalTradeReserved(p) {
			return errCriticalPersistenceBusy
		}
		seen[p] = true
		r := retained{p: p, account: accountStateSnapshot(p.Account), coins: maps.Clone(p.SpecialCoins),
			trade: p.Trade, party: p.Party, x: p.X, y: p.Y, space: w.playerRuntimeInstanceID(p.ID)}
		if p.Party != nil {
			r.partyMembers = slices.Clone(p.Party.Members)
			// An ineligible party member may still remove or replace the party
			// leader. Retain that lifecycle without moving or charging them.
			for _, member := range p.Party.Members {
				if !slices.Contains(participants, member) {
					participants = append(participants, member)
				}
			}
		}
		if p.Trade != nil {
			r.offer = *p.Trade
			peer := w.playerByID(p.Trade.OpponentID)
			if peer == nil || peer == p || peer.Trade == nil || peer.Trade.OpponentID != p.ID {
				return errCriticalPersistenceIdentity
			}
			if !slices.Contains(participants, peer) {
				participants = append(participants, peer)
			}
		}
		account := changes[p]
		if account == nil {
			account = accountStateSnapshot(p.Account)
		} else if account == p.Account || accountSessionKey(account.Name) != accountSessionKey(p.Account.Name) {
			return errCriticalPersistenceIdentity
		}
		accounts = append(accounts, account)
		before = append(before, r)
	}
	if len(participants) > criticalPersistenceMaxAccounts {
		return errCriticalPersistenceFull
	}
	if !seen[actor] {
		return errCriticalPersistenceIdentity
	}
	for p := range changes {
		if !seen[p] {
			return errCriticalPersistenceIdentity
		}
	}
	check := func() error {
		if err := plan.check(w); err != nil {
			return err
		}
		for _, r := range before {
			p := r.p
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), r.account) || !maps.Equal(p.SpecialCoins, r.coins) ||
				p.X != r.x || p.Y != r.y || w.playerRuntimeInstanceID(p.ID) != r.space ||
				p.Trade != r.trade || (p.Trade != nil && *p.Trade != r.offer) ||
				p.Party != r.party || (p.Party != nil && !slices.Equal(p.Party.Members, r.partyMembers)) {
				return errCriticalPersistenceIdentity
			}
		}
		if checkAllocation != nil {
			return checkAllocation()
		}
		return nil
	}
	if err := check(); err != nil {
		return err
	}
	_, err := w.submitCriticalWriteWithRejection(criticalWrite{accounts: accounts, instances: plan.snapshot()}, participants, false,
		func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			plan.install(w)
			apply()
			return nil
		}, func(criticalPersistenceResult) error {
			if release != nil {
				release()
			}
			for _, r := range before {
				if r.trade != nil {
					r.p.Trade = nil
					if !r.p.Session.IsClosed() {
						r.p.Session.Send(wire.CloseTrade(r.p.ID))
					}
				}
			}
			if publish != nil {
				publish()
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			if release != nil {
				release()
			}
			if !actor.Session.IsClosed() {
				actor.Session.Send(wire.MessagePanel("The instance change could not be saved. Your items were not changed."))
				w.resendInventory(actor)
			}
			return nil
		})
	return err
}
