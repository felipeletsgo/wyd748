package game

import (
	"time"

	"wydgo/internal/model"
)

// queueTimedItemExpiry removes expired instances in a private account. The
// account fence also covers inactive characters and Cargo until actor delivery.
func (w *World) queueTimedItemExpiry(p *Player, now time.Time) error {
	if p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		return errCriticalPersistenceIdentity
	}
	proposed := accountStateSnapshot(p.Account)
	removals := expireTimedAccountItems(proposed, now)
	if len(removals) == 0 {
		return nil
	}
	_, err := w.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p}, false,
		func(criticalPersistenceResult) error {
			// Do not replace the live account or unrelated score/position fields.
			for _, removal := range removals {
				if removal.place == placeStorage {
					p.Account.Cargo[removal.slot] = model.Item{}
				} else if removal.place == placeEquip {
					p.Account.Chars[removal.charIndex].Equip[removal.slot] = model.Item{}
				} else {
					p.Account.Chars[removal.charIndex].Inv[removal.slot] = model.Item{}
				}
			}
			return nil
		}, func(criticalPersistenceResult) error {
			w.publishTimedItemExpiry(p, removals)
			return nil
		})
	return err
}
