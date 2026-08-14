package game

import (
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const premiumTimedMarker = "111"

var errTimedItemExpired = errors.New("timed item expired")

// timedItemDuration returns the data-driven lifetime for an equipped item.
// Fairies use their native WDAY/HOUR/MIN duration. Premium costumes and mounts
// are explicitly marked by effect 111 and use the same catalog fields.
func timedItemDuration(def model.ItemDef, index uint16, slot byte) time.Duration {
	if slot != fairySlot && slot != mountSlot {
		return 0
	}
	if isFairyIndex(index) {
		if slot != fairySlot {
			return 0
		}
	} else if staticAbility(def, premiumTimedMarker) != 1 {
		return 0
	}
	days := staticAbility(def, "EF_WDAY")
	hours := staticAbility(def, "EF_HOUR")
	minutes := staticAbility(def, "EF_MIN")
	if days < 0 || hours < 0 || hours > 23 || minutes < 0 || minutes > 59 {
		return 0
	}
	return time.Duration(days*24*60+hours*60+minutes) * time.Minute
}

func (w *World) timedItemDuration(item model.Item, slot byte) time.Duration {
	if w == nil || item.Index == 0 {
		return 0
	}
	def, ok := w.items[item.Index]
	if !ok {
		return 0
	}
	return timedItemDuration(def, item.Index, slot)
}

// activateTimedItem binds the first-equip timestamp to the server-side item
// instance. ActivatedUnix/ExpiresUnix follow the UID through inventory, Cargo,
// trade and relogin and are never read from the 7.48 wire.
func (w *World) activateTimedItem(item *model.Item, slot byte, now time.Time) (bool, error) {
	if item == nil || w.timedItemDuration(*item, slot) <= 0 {
		return false, nil
	}
	if item.ExpiresUnix != 0 {
		if item.ActivatedUnix <= 0 || item.ExpiresUnix < item.ActivatedUnix {
			return false, errors.New("invalid timed item metadata")
		}
		if item.ExpiresUnix <= now.Unix() {
			return false, errTimedItemExpired
		}
		return false, nil
	}
	if item.UID == "" {
		return false, errors.New("timed item has no server UID")
	}
	duration := w.timedItemDuration(*item, slot)
	activated := now.UTC().Unix()
	item.ActivatedUnix = activated
	item.ExpiresUnix = time.Unix(activated, 0).UTC().Add(duration).Unix()
	return true, nil
}

type timedItemRemoval struct {
	charIndex int
	place     byte
	slot      byte
}

type timedAccountItemsSnapshot struct {
	equip [][model.MaxEquipSlots]model.Item
	inv   [][model.MaxCarry]model.Item
	cargo [model.MaxCargo]model.Item
}

func snapshotTimedAccountItems(account *model.Account) timedAccountItemsSnapshot {
	snapshot := timedAccountItemsSnapshot{}
	if account == nil {
		return snapshot
	}
	snapshot.equip = make([][model.MaxEquipSlots]model.Item, len(account.Chars))
	snapshot.inv = make([][model.MaxCarry]model.Item, len(account.Chars))
	for i := range account.Chars {
		snapshot.equip[i] = account.Chars[i].Equip
		snapshot.inv[i] = account.Chars[i].Inv
	}
	snapshot.cargo = account.Cargo
	return snapshot
}

func restoreTimedAccountItems(account *model.Account, snapshot timedAccountItemsSnapshot) {
	if account == nil || len(snapshot.equip) != len(account.Chars) || len(snapshot.inv) != len(account.Chars) {
		return
	}
	for i := range account.Chars {
		account.Chars[i].Equip = snapshot.equip[i]
		account.Chars[i].Inv = snapshot.inv[i]
	}
	account.Cargo = snapshot.cargo
}

// expireTimedAccountItems consumes every expired UID in the live account,
// regardless of whether it is equipped, carried or stored in Cargo. The caller
// owns the save/rollback boundary and must not publish these removals first.
func expireTimedAccountItems(account *model.Account, now time.Time) []timedItemRemoval {
	if account == nil {
		return nil
	}
	expired := func(item model.Item) bool {
		return item.Index != 0 && item.ExpiresUnix > 0 && item.ExpiresUnix <= now.Unix()
	}
	removals := make([]timedItemRemoval, 0, 2)
	for charIndex := range account.Chars {
		ch := &account.Chars[charIndex]
		for slot := range ch.Equip {
			if expired(ch.Equip[slot]) {
				ch.Equip[slot] = model.Item{}
				removals = append(removals, timedItemRemoval{charIndex: charIndex, place: placeEquip, slot: byte(slot)})
			}
		}
		for slot := range ch.Inv {
			if expired(ch.Inv[slot]) {
				ch.Inv[slot] = model.Item{}
				removals = append(removals, timedItemRemoval{charIndex: charIndex, place: placeInv, slot: byte(slot)})
			}
		}
	}
	for slot := range account.Cargo {
		if expired(account.Cargo[slot]) {
			account.Cargo[slot] = model.Item{}
			removals = append(removals, timedItemRemoval{charIndex: -1, place: placeStorage, slot: byte(slot)})
		}
	}
	return removals
}

func (w *World) prepareTimedEquipmentForEnter(p *Player, now time.Time) error {
	if w == nil || p == nil || p.Char == nil || p.Account == nil {
		return nil
	}
	snapshot := snapshotTimedAccountItems(p.Account)
	changed := len(expireTimedAccountItems(p.Account, now)) != 0
	for _, slot := range []byte{fairySlot, mountSlot} {
		item := &p.Char.Equip[slot]
		if w.timedItemDuration(*item, slot) <= 0 {
			continue
		}
		activated, err := w.activateTimedItem(item, slot, now)
		if err != nil {
			restoreTimedAccountItems(p.Account, snapshot)
			return err
		}
		changed = changed || activated
	}
	if !changed {
		return nil
	}
	if err := w.saveAccount(p.Account); err != nil {
		restoreTimedAccountItems(p.Account, snapshot)
		return fmt.Errorf("persist timed equipment: %w", err)
	}
	return nil
}

const timedItemSweepInterval = time.Minute

// tickTimedItems removes expired UID instances with persist-before-publish.
// A failed save restores every location, so an item can never disappear only
// from RAM while its UID remains alive in PostgreSQL.
func (w *World) tickTimedItems(now time.Time) {
	if !w.nextTimedItemSweep.IsZero() && now.Before(w.nextTimedItemSweep) {
		return
	}
	w.nextTimedItemSweep = now.Add(timedItemSweepInterval)
	seenAccounts := make(map[*model.Account]struct{}, len(w.players))
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		if _, done := seenAccounts[p.Account]; done {
			continue
		}
		seenAccounts[p.Account] = struct{}{}
		snapshot := snapshotTimedAccountItems(p.Account)
		removals := expireTimedAccountItems(p.Account, now)
		if len(removals) == 0 {
			continue
		}
		if err := w.saveAccount(p.Account); err != nil {
			restoreTimedAccountItems(p.Account, snapshot)
			log.Printf("[#%d] failed to expire timed equipment: %v", p.ID, err)
			continue
		}
		activeEquipChanged := false
		for _, removal := range removals {
			activeEquipChanged = activeEquipChanged ||
				removal.charIndex == p.CharSlot && removal.place == placeEquip
		}
		if p.Session != nil {
			for _, removal := range removals {
				if removal.charIndex == p.CharSlot {
					p.Session.Send(wire.SendItem(p.ID, removal.place, removal.slot, model.Item{}))
				} else if removal.place == placeStorage {
					p.Session.Send(wire.SendItem(p.ID, placeStorage, removal.slot, model.Item{}))
				}
			}
			p.Session.Send(wire.MessagePanel("One or more timed items have expired."))
		}
		if !activeEquipChanged {
			continue
		}
		w.recalcPlayer(p.Char)
		if p.Session != nil {
			p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		}
		w.syncPlayerVitalsToObservers(p)
		w.syncCriaPet(p)
		w.refreshAppearance(p)
	}
}
