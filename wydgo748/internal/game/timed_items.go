package game

import (
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Effect 111 is the native EF_NOTRADE; premium costumes and mounts carry it with
// their EF_WDAY/EF_HOUR/EF_MIN lifetime, and the catalog now loads it by name.
const premiumTimedMarker = "EF_NOTRADE"

var errTimedItemExpired = errors.New("timed item expired")

// timedItemDuration returns the data-driven lifetime for an equipped item.
// Fairies use their native WDAY/HOUR/MIN duration. Premium costumes and mounts
// are explicitly marked by effect 111 and use the same catalog fields; costumes
// live in model.CostumeSlot under the WYD-Go costume-slot extension.
func timedItemDuration(def model.ItemDef, index uint16, slot byte) time.Duration {
	if slot != fairySlot && slot != mountSlot && slot != model.CostumeSlot {
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
	if duration := timedItemDuration(def, item.Index, slot); duration > 0 {
		return duration
	}
	// Timed horses 3980..3989 encode their lifetime only in the name; the
	// mount catalog carries it for marked (EF_NOTRADE) rows.
	if slot == mountSlot && staticAbility(def, premiumTimedMarker) == 1 {
		if stats, ok := w.mounts.PremiumStats(item.Index); ok {
			return time.Duration(stats.DurationDays) * 24 * time.Hour
		}
	}
	return 0
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
		if isFairyIndex(item.Index) && item.PausedUnix != 0 {
			if item.PausedUnix < item.ActivatedUnix || item.PausedUnix >= item.ExpiresUnix || now.Unix() < item.PausedUnix {
				return false, errors.New("invalid paused fairy metadata")
			}
			item.ExpiresUnix += now.Unix() - item.PausedUnix
			item.PausedUnix = 0
			return true, nil
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

// Fairies spend lifetime only while equipped. This server-only timestamp
// follows the UID through inventory, Cargo, trade and relogin; premium mounts
// and costumes retain their absolute-deadline contract.
func pauseUnequippedFairy(item *model.Item, now time.Time) bool {
	if item == nil || !isFairyIndex(item.Index) || item.PausedUnix != 0 ||
		item.ActivatedUnix <= 0 || item.ExpiresUnix <= now.Unix() || now.Unix() < item.ActivatedUnix {
		return false
	}
	item.PausedUnix = now.Unix()
	return true
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
// except paused fairies carried or stored in Cargo. The caller
// owns the save/rollback boundary and must not publish these removals first.
func expireTimedAccountItems(account *model.Account, now time.Time) []timedItemRemoval {
	if account == nil {
		return nil
	}
	expired := func(item model.Item) bool {
		return item.Index != 0 && item.PausedUnix == 0 && item.ExpiresUnix > 0 && item.ExpiresUnix <= now.Unix()
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
	if w.criticalPlayerReserved(p) {
		return errCriticalPersistenceBusy
	}
	snapshot := snapshotTimedAccountItems(p.Account)
	changed, err := w.planTimedEquipmentForEnter(p, now)
	if err != nil {
		restoreTimedAccountItems(p.Account, snapshot)
		return err
	}
	if !changed {
		return nil
	}
	// This synchronous API cannot confirm a queued save. Staged entry uses the
	// detached planner and its identity-bound continuation instead.
	if w.criticalWrites != nil {
		restoreTimedAccountItems(p.Account, snapshot)
		return errCriticalPersistenceUnsupported
	}
	if err := w.saveAccount(p.Account); err != nil {
		restoreTimedAccountItems(p.Account, snapshot)
		return fmt.Errorf("persist timed equipment: %w", err)
	}
	return nil
}

// planTimedEquipmentForEnter operates on the caller's supplied private state.
// The synchronous legacy wrapper restores it on error; staged entry never
// exposes the proposal until its durable write is acknowledged.
func (w *World) planTimedEquipmentForEnter(p *Player, now time.Time) (bool, error) {
	changed := len(expireTimedAccountItems(p.Account, now)) != 0
	// Migrate still-live carried fairies from the former wall-clock policy.
	// Already expired items are not resurrected and no unknown past usage is refunded.
	for i := range p.Account.Chars {
		for slot := range p.Account.Chars[i].Inv {
			changed = pauseUnequippedFairy(&p.Account.Chars[i].Inv[slot], now) || changed
		}
	}
	for slot := range p.Account.Cargo {
		changed = pauseUnequippedFairy(&p.Account.Cargo[slot], now) || changed
	}
	// Costumes equipped in slot 13 by older builds move to the costume slot
	// before activation, persisted with the same entry write.
	if model.MigrateCostumeSlot(p.Char, w.items) {
		changed = true
	}
	for _, slot := range []byte{model.CostumeSlot, fairySlot, mountSlot} {
		item := &p.Char.Equip[slot]
		if w.timedItemDuration(*item, slot) <= 0 {
			continue
		}
		activated, err := w.activateTimedItem(item, slot, now)
		if err != nil {
			return false, err
		}
		changed = changed || activated
	}
	return changed, nil
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
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil || w.criticalPlayerReserved(p) {
			continue
		}
		if _, done := seenAccounts[p.Account]; done {
			continue
		}
		seenAccounts[p.Account] = struct{}{}
		if w.criticalWrites != nil {
			if err := w.queueTimedItemExpiry(p, now); err != nil {
				log.Printf("[#%d] failed to submit timed item expiry: %v", p.ID, err)
			}
			continue
		}
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
		w.publishTimedItemExpiry(p, removals)
	}
}

// publishTimedItemExpiry runs only after durable removal, on World. The staged
// caller invokes it after acknowledgement so guarded follower helpers can run.
func (w *World) publishTimedItemExpiry(p *Player, removals []timedItemRemoval) {
	activeEquipChanged := false
	for _, removal := range removals {
		activeEquipChanged = activeEquipChanged ||
			removal.charIndex == p.CharSlot && removal.place == placeEquip
	}
	if p.Session != nil && !p.Session.IsClosed() {
		for _, removal := range removals {
			if removal.charIndex == p.CharSlot || removal.place == placeStorage {
				p.Session.Send(wire.SendItem(p.ID, removal.place, removal.slot, model.Item{}))
			}
		}
		p.Session.Send(wire.MessagePanel("One or more timed items have expired."))
	}
	if !activeEquipChanged {
		return
	}
	w.recalcPlayer(p.Char)
	if p.Session != nil && p.Session.IsClosed() {
		return // Final lifecycle persistence sees committed items and score.
	}
	if p.Session != nil {
		p.Session.Send(playerScorePacket(p))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	}
	w.syncPlayerVitalsToObservers(p)
	w.syncCriaPet(p)
	w.refreshAppearance(p)
}
