package game

import (
	"errors"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type criticalInventorySlotChange struct {
	slot int
	item model.Item
}

// Deletion and splitting change at most two visible inventory slots. The actor
// validates and materializes items once before admission; persistence and live
// delivery use those exact values, never regenerating a split UID on completion.
func (w *World) queueInventorySlotChanges(p *Player, operation string, changes ...criticalInventorySlotChange) error {
	if p == nil || p.Account == nil || p.Char == nil || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		return errCriticalPersistenceIdentity
	}
	if len(changes) < 1 || len(changes) > 2 {
		return errors.New("critical inventory: invalid slot change count")
	}
	var committed [2]criticalInventorySlotChange
	copy(committed[:], changes) // Retain values, not a caller-owned slice.
	count := len(changes)
	proposed := accountStateSnapshot(p.Account)
	for i, change := range committed[:count] {
		if change.slot < 0 || change.slot >= model.PlayerCarrySlots ||
			(i != 0 && change.slot == committed[0].slot) {
			return errors.New("critical inventory: invalid or duplicate slot")
		}
		proposed.Chars[p.CharSlot].Inv[change.slot] = change.item
	}
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			// Install all economic fields before any client confirmation. Preserve
			// live account/character identity, score, vitals and unrelated items.
			for _, change := range committed[:count] {
				p.Char.Inv[change.slot] = change.item
			}
			if !p.Session.IsClosed() {
				for _, change := range committed[:count] {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(change.slot), change.item))
				}
			}
			log.Printf("[#%d] committed inventory %s slots=%d", p.Session.ID, operation, count)
			return nil
		})
	return err
}
