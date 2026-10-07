package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Cargo never equips items directly. Slot/lock/banker validation and eligible
// trade cancellation happen on World before this detached persistence plan.
func (w *World) queueCargoItemSwap(p *Player, st, sp, dt, dp byte, targetID uint16) error {
	if p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
		(st != placeStorage && dt != placeStorage) || st == placeEquip || dt == placeEquip {
		return errCriticalPersistenceIdentity
	}
	src, dst := playerSlotOf(p, st, sp), playerSlotOf(p, dt, dp)
	if src == nil || dst == nil {
		return errCriticalPersistenceIdentity
	}
	proposed := accountStateSnapshot(p.Account)
	planned := &Player{Account: proposed, Char: &proposed.Chars[p.CharSlot]}
	plannedSrc, plannedDst := playerSlotOf(planned, st, sp), playerSlotOf(planned, dt, dp)
	*plannedSrc, *plannedDst = *plannedDst, *plannedSrc
	sourceItem, destinationItem := *plannedSrc, *plannedDst
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			// Actor-only pointers stay fenced through identity-checked delivery.
			// Never replace the live aggregate with a normalized worker snapshot.
			*src, *dst = sourceItem, destinationItem
			if !p.Session.IsClosed() {
				p.Session.Send(wire.SwapItem(p.ID, st, sp, dt, dp, targetID))
			}
			log.Printf("[#%d] SWAP ok %d/%d <-> %d/%d", p.Session.ID, st, sp, dt, dp)
			return nil
		})
	return err
}
