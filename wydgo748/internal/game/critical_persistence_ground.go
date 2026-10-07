package game

import (
	"errors"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// queueGroundPickup prepares one slot or currency balance without touching live
// economy. The object remains registered until the identity-bound World commit.
// A nonzero gold value selects currency; zero selects ordinary item pickup.
func (w *World) queueGroundPickup(p *Player, g *GroundItem, requestedSlot int, gold uint32) error {
	if p == nil || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
		g == nil || w.groundItems[g.ID] != g {
		return errCriticalPersistenceIdentity
	}
	if g.criticalReserved() || w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	if g.Permanent || !p.InWorld || playerCurHP(p.Char) == 0 ||
		!w.groundItemVisibleToPlayer(p, g) || chebyshev(p.X, p.Y, g.X, g.Y) > pickupRange {
		return errors.New("critical pickup: object is not eligible")
	}
	proposed := accountStateSnapshot(p.Account)
	character := &proposed.Chars[p.CharSlot]
	beforeGold, slot, item := p.Char.Gold, -1, g.Item
	var beforeItem model.Item
	if gold != 0 {
		if beforeGold > maxCharacterGold || gold > maxCharacterGold-beforeGold {
			return errors.New("critical pickup: currency overflow")
		}
		character.Gold += gold
	} else {
		if requestedSlot < 0 || requestedSlot >= model.PlayerCarrySlots {
			return errors.New("critical pickup: invalid inventory slot")
		}
		slot = requestedSlot
		if p.Char.Inv[slot].Index != 0 {
			slot = firstFreeVisibleInventorySlot(p.Char)
			if slot < 0 {
				return errors.New("critical pickup: inventory full")
			}
			// Preserve addToInv's fallback materialization, exactly once, before
			// admission. No completion generates a new item identity.
			var err error
			item, err = materializeItem(item)
			if err != nil {
				return err
			}
		}
		beforeItem = p.Char.Inv[slot]
		character.Inv[slot] = item
	}
	context := *g
	var operation uint64
	var err error
	check := func() error {
		current := *g
		current.criticalPickup = 0
		// Existing instance cleanup may release private loot publicly. That
		// visibility-only change cannot grant a second economic claim.
		if context.InstanceID != "" && current.InstanceID == "" {
			current.InstanceID = context.InstanceID
		}
		if w.groundItems[context.ID] != g || g.criticalPickup != operation || current != context ||
			(gold != 0 && p.Char.Gold != beforeGold) || (gold == 0 && p.Char.Inv[slot] != beforeItem) {
			return errors.New("critical pickup: reserved object or economic source changed")
		}
		return nil
	}
	operation, err = w.submitCriticalSharedWriteWithRejection(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			if gold != 0 {
				p.Char.Gold = beforeGold + gold
			} else {
				p.Char.Inv[slot] = item
			}
			// No actor command can interleave removal with persistence acknowledgement.
			g.criticalPickup = 0
			w.publishItemRemove(g)
			w.unregisterGroundItem(g)
			if !p.Session.IsClosed() {
				if gold != 0 {
					p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				} else {
					p.Session.Send(wire.CNFGetItem(uint32(placeInv), uint32(slot)))
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(slot), item))
				}
			}
			log.Printf("[#%d] committed ground pickup object=%d item=%d slot=%d gold=+%d",
				p.Session.ID, context.ID, item.Index, slot, gold)
			return nil
		}, nil, func(result criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			// Only a certified rejection can release this unchanged object.
			g.criticalPickup = 0
			if !p.Session.IsClosed() {
				if gold != 0 {
					p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				} else {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(slot), beforeItem))
				}
				p.Session.Send(wire.MessagePanel("Save failed. The item was not picked up."))
			}
			log.Printf("[#%d] ground pickup object=%d certified rejection: %v", p.Session.ID, context.ID, result.err)
			return nil
		})
	if err == nil {
		g.criticalPickup = operation
	}
	return err
}

// queueGroundDrop owns the unpublished allocation and proposed source removal.
// The caller unregisters that allocation only if admission fails.
func (w *World) queueGroundDrop(p *Player, g *GroundItem, req dropItemRequest, item model.Item) error {
	if p == nil || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
		g == nil || w.groundItems[g.ID] != g {
		return errCriticalPersistenceIdentity
	}
	if g.criticalReserved() || w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	source := playerSlotOf(p, req.srcType, req.srcPos)
	if source == nil || *source != item || item.Index == 0 || g.Permanent ||
		!p.InWorld || playerCurHP(p.Char) == 0 || !w.groundItemVisibleToPlayer(p, g) {
		return errors.New("critical drop: source or allocation is not eligible")
	}
	proposed := accountStateSnapshot(p.Account)
	switch req.srcType {
	case placeInv:
		if int(req.srcPos) >= model.PlayerCarrySlots {
			return errors.New("critical drop: invalid Carry source")
		}
		proposed.Chars[p.CharSlot].Inv[req.srcPos] = model.Item{}
	case placeStorage:
		if int(req.srcPos) >= model.PlayerCargoSlots || !w.nearCargoNPC(p) || p.ghostShopLocksCargoSlot(int(req.srcPos)) {
			return errors.New("critical drop: invalid or locked Cargo source")
		}
		proposed.Cargo[req.srcPos] = model.Item{}
	default:
		return errors.New("critical drop: invalid source place")
	}
	if _, filled := model.CelestialSealID(item); filled {
		return errors.New("critical drop: filled seal cannot be dropped")
	}
	context := *g
	var operation uint64
	var err error
	check := func() error {
		current := *g
		current.criticalDrop = 0
		if context.InstanceID != "" && current.InstanceID == "" {
			current.InstanceID = context.InstanceID
		}
		liveSource := playerSlotOf(p, req.srcType, req.srcPos)
		if w.groundItems[context.ID] != g || g.criticalDrop != operation || current != context ||
			liveSource == nil || *liveSource != item {
			return errors.New("critical drop: reserved allocation or economic source changed")
		}
		return nil
	}
	operation, err = w.submitCriticalSharedWriteWithRejection(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			*playerSlotOf(p, req.srcType, req.srcPos) = model.Item{}
			g.criticalDrop = 0
			w.publishItemSpawn(g)
			if !p.Session.IsClosed() {
				p.Session.Send(wire.CNFDropItem(uint32(req.srcType), uint32(req.srcPos), req.rotate, g.X, g.Y))
				p.Session.Send(wire.SendItem(p.ID, req.srcType, req.srcPos, model.Item{}))
			}
			log.Printf("[#%d] committed ground drop object=%d item=%d source=%d/%d", p.Session.ID, g.ID, item.Index, req.srcType, req.srcPos)
			return nil
		}, nil, func(result criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			// The allocation was never published and the source was never removed.
			g.criticalDrop = 0
			w.unregisterGroundItem(g)
			if !p.Session.IsClosed() {
				p.Session.Send(wire.SendItem(p.ID, req.srcType, req.srcPos, item))
				p.Session.Send(wire.MessagePanel("Save failed. The item was not dropped."))
			}
			log.Printf("[#%d] ground drop object=%d certified rejection: %v", p.Session.ID, context.ID, result.err)
			return nil
		})
	if err == nil {
		g.criticalDrop = operation
	}
	return err
}
