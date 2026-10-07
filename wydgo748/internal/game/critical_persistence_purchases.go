package game

import "wydgo/internal/model"

// Hell Gate purchases do not credit a treasury. Other ticket/merchant callers
// remain synchronous until their guild/shared-resource operations are migrated.
func (w *World) queueHellStorePurchase(p *Player, item model.Item, dst int, price uint64, option int) error {
	if p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		return errCriticalPersistenceIdentity
	}
	proposed := accountStateSnapshot(p.Account)
	character := &proposed.Chars[p.CharSlot]
	character.Inv[dst] = item
	character.Gold -= uint32(price)
	gold := character.Gold
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			// The actor holds this account/character through acknowledgement.
			// Preserve all unrelated live state and the already materialized UID.
			p.Char.Inv[dst], p.Char.Gold = item, gold
			publishHellStorePurchase(p, option, item, price, dst)
			return nil
		})
	return err
}
