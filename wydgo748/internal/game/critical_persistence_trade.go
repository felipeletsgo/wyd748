package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Plan both aggregates privately, including seal ownership and account-local ID
// remapping. Workers receive neither live players nor trade continuations.
func (w *World) queueTrade(a, b *Player, aInv, bInv [model.MaxCarry]model.Item, aGold, bGold uint32) error {
	for _, p := range []*Player{a, b} {
		if p == nil || p.Account == nil || p.Char == nil || p.Trade == nil ||
			p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
			return errCriticalPersistenceIdentity
		}
	}
	aTrade, bTrade := a.Trade, b.Trade
	aOffer, bOffer := *aTrade, *bTrade
	aAccount, bAccount := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
	aCopy := &Player{Account: aAccount, Char: &aAccount.Chars[a.CharSlot]}
	bCopy := &Player{Account: bAccount, Char: &bAccount.Chars[b.CharSlot]}
	aCopy.Char.Inv, bCopy.Char.Inv = aInv, bInv
	aCopy.Char.Gold, bCopy.Char.Gold = aGold, bGold
	if err := w.transferTradeCapsules(aCopy, bCopy, aOffer.Items); err != nil {
		return err
	}
	if err := w.transferTradeCapsules(bCopy, aCopy, bOffer.Items); err != nil {
		return err
	}
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{aAccount, bAccount}}, []*Player{a, b},
		func(criticalPersistenceResult) error {
			// Identity binding protects accounts/characters; this separately binds
			// the shared trade context before either economic result is installed.
			if a.Trade != aTrade || b.Trade != bTrade || *aTrade != aOffer || *bTrade != bOffer {
				return errCriticalPersistenceIdentity
			}
			a.Char.Inv, b.Char.Inv = aCopy.Char.Inv, bCopy.Char.Inv
			a.Char.Gold, b.Char.Gold = aGold, bGold
			a.Account.CelestialCapsules, b.Account.CelestialCapsules = aAccount.CelestialCapsules, bAccount.CelestialCapsules
			a.Trade, b.Trade = nil, nil
			for _, p := range []*Player{a, b} {
				if !p.Session.IsClosed() {
					p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
				}
			}
			for _, p := range []*Player{a, b} {
				if !p.Session.IsClosed() {
					p.Session.Send(wire.CloseTrade(p.ID))
				}
			}
			log.Printf("TRADE committed: %s(%d gold) <-> %s(%d gold)", a.Char.Name, aOffer.Gold, b.Char.Name, bOffer.Gold)
			return nil
		})
	return err
}
