package game

import (
	"maps"

	"wydgo/internal/model"
)

// prepareCriticalCharacterEntry leaves the authenticated selection state intact
// while reading the sidecar and planning item changes on a detached account.
// The sidecar read is unchanged; this migration removes preparation writes from
// World, not all entry I/O. No worker owns a session or selects a character.
func (w *World) prepareCriticalCharacterEntry(p *Player, slot int) error {
	if p == nil || p.Session == nil || p.Account == nil || slot < 0 ||
		slot >= len(p.Account.Chars) || slot >= maxCharactersPerAccount || p.Account.Chars[slot].Name == "" ||
		p.Char != nil || p.CharSlot != -1 || p.ID != 0 || p.InWorld {
		return errCriticalPersistenceIdentity
	}
	if w.players[p.Session] != p || w.accountSessions[accountSessionKey(p.Account.Name)] != p.Session {
		return errCriticalPersistenceIdentity
	}
	proposed := accountStateSnapshot(p.Account)
	planned := &Player{Account: proposed, Char: &proposed.Chars[slot], CharSlot: slot}
	if err := w.loadCharStateInto(planned); err != nil {
		return err
	}
	changed := w.planUnsupportedEquipment(planned)
	if w.ensureCapeEmblem(planned.Char) {
		changed = true
	}
	return w.continueCriticalCharacterEntry(p, planned, changed)
}

func (w *World) continueCriticalCharacterEntry(p, planned *Player, changed bool) error {
	if p.Session.IsClosed() || w.shuttingDown {
		return nil // Committed item changes still feed the final lifecycle save.
	}
	timedChanged, err := w.planTimedEquipmentForEnter(planned, w.now())
	if err != nil {
		return err
	}
	if !changed && !timedChanged {
		p.CharSlot = planned.CharSlot
		p.Char = &p.Account.Chars[p.CharSlot]
		// The held write may outlive a buff. Drop expired affects before building
		// entry packets without repeating a blocking sidecar read in delivery.
		p.Char.Affects = [16]model.Affect{}
		p.SpecialCoins = maps.Clone(planned.SpecialCoins)
		for slot, affect := range planned.Char.Affects {
			if affect.Type != 0 && affect.ExpiresAt.After(w.now()) {
				p.Char.Affects[slot] = affect
			}
		}
		w.publishCharacterEntry(p, p.CharSlot)
		return nil
	}
	_, err = w.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{planned.Account}}, []*Player{p}, false,
		func(criticalPersistenceResult) error {
			// The participant binding intentionally retains selection (Char=nil).
			// Bind the prepared character list too: the same account/session alone
			// must not authorize applying an old proposal to replaced/reordered slots.
			if len(p.Account.Chars) != len(planned.Account.Chars) {
				return errCriticalPersistenceIdentity
			}
			for slot := range planned.Account.Chars {
				live, proposed := &p.Account.Chars[slot], &planned.Account.Chars[slot]
				if live.UID != proposed.UID || live.Name != proposed.Name {
					return errCriticalPersistenceIdentity
				}
			}
			// Preserve live identities, scores, positions and selected lifecycle.
			for slot := range planned.Account.Chars {
				p.Account.Chars[slot].Equip = planned.Account.Chars[slot].Equip
				p.Account.Chars[slot].Inv = planned.Account.Chars[slot].Inv
				p.Account.Chars[slot].CapeEmblem = planned.Account.Chars[slot].CapeEmblem
			}
			p.Account.Cargo = planned.Account.Cargo
			return nil
		}, func(criticalPersistenceResult) error {
			// Recheck deadlines at delivery. If an item expired during the write,
			// its removal needs a new durable transaction before entry, not a RAM
			// deletion or publication of already-expired equipment. This is not
			// replay of the accepted operation and never reloads the sidecar.
			return w.continueCriticalCharacterEntry(p, planned, false)
		})
	return err
}
