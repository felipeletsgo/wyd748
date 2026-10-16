package game

import (
	"errors"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

var errEquipmentSwapEligibility = errors.New("equipment move rejected")

// queueEquipmentSwap plans every persisted item value on World. The worker
// cannot activate deadlines, choose mount state or reconcile live followers.
func (w *World) queueEquipmentSwap(p *Player, st, sp, dt, dp byte, targetID uint16) error {
	if p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] ||
		(st != placeEquip && dt != placeEquip) || st == placeStorage || dt == placeStorage {
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
	if (st == placeEquip && !w.canEquip(planned.Char, *plannedSrc, sp)) ||
		(dt == placeEquip && !w.canEquip(planned.Char, *plannedDst, dp)) {
		return errEquipmentSwapEligibility
	}
	now := w.now()
	for _, target := range []struct {
		typ, pos byte
		item     *model.Item
	}{{st, sp, plannedSrc}, {dt, dp, plannedDst}} {
		if target.typ == placeEquip {
			if _, err := w.activateTimedItem(target.item, target.pos, now); err != nil {
				return err
			}
		}
	}
	// A fairy/armor move can also initialize a legacy zero-state equipped mount.
	// Preserve that existing effect, but include its exact random state in the
	// same detached save rather than changing it only after the commit.
	mountInitialized := false
	if mount, _ := equippedMount(planned.Char); mount != nil {
		mountInitialized = w.initFreshMount(mount)
	}
	sourceItem, destinationItem := *plannedSrc, *plannedDst
	promoted := w.promoteOffhandWeapon(planned.Char)
	mainWeapon, offhandWeapon := planned.Char.Equip[6], planned.Char.Equip[7]
	mountItem := planned.Char.Equip[mountSlot]
	_, err := w.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p}, false,
		func(criticalPersistenceResult) error {
			*src, *dst = sourceItem, destinationItem
			if promoted {
				p.Char.Equip[6], p.Char.Equip[7] = mainWeapon, offhandWeapon
			}
			if mountInitialized {
				p.Char.Equip[mountSlot] = mountItem
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if (st == placeEquip && sp == mountSlot) || (dt == placeEquip && dp == mountSlot) {
				// A closed transport may retain a disconnect-intent fence. Clear
				// this committed clock directly; ordinary clock helpers stay guarded.
				p.EggIncubationUID = ""
				p.NextEggIncubationTick = time.Time{}
			}
			if p.Session.IsClosed() {
				w.recalcPlayer(p.Char) // Final lifecycle saves see committed score.
				return nil
			}
			p.Session.Send(wire.SwapItem(p.ID, st, sp, dt, dp, targetID))
			if promoted {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, 6, p.Char.Equip[6]))
				p.Session.Send(wire.SendItem(p.ID, placeEquip, 7, p.Char.Equip[7]))
			}
			w.recalcPlayer(p.Char)
			p.Session.Send(playerScorePacket(p))
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			w.syncPlayerVitalsToObservers(p)
			w.syncCriaPet(p)
			w.refreshAppearance(p)
			log.Printf("[#%d] SWAP ok %d/%d <-> %d/%d", p.Session.ID, st, sp, dt, dp)
			return nil
		})
	return err
}
