package game

import (
	"errors"
	"log"
	"time"

	"wydgo/internal/model"
)

// charStateStore optionally persists session buffs and currencies. Like
// guildStore, gameplay uses a minimal interface rather than a concrete adapter.
type charStateStore interface {
	LoadCharState(characterUID string) (*model.CharState, error)
	SaveCharState(characterUID string, state *model.CharState) error
}

type playerStateStore interface {
	SavePlayerState(guilds *model.GuildRegistry, account *model.Account,
		characterUID string, state *model.CharState) error
}

// A trade peer may be reserved by a sidecar-changing operation. Its unchanged
// snapshot and the owner's account/sidecar must use one durable transaction.
type playerStateBatchStore interface {
	SavePlayerStateBatch(guilds *model.GuildRegistry, characterUID string,
		state *model.CharState, accounts ...*model.Account) error
}

// buildCharState snapshots unexpired affects and nonzero player currencies.
func buildCharState(p *Player, now time.Time) *model.CharState {
	state := &model.CharState{Version: model.CharStateVersion}
	if p.Char != nil {
		for _, a := range p.Char.Affects {
			if a.Type == 0 || !a.ExpiresAt.After(now) {
				continue
			}
			nextTickUnix := int64(0)
			if a.FinalTickAtExpiry {
				nextTickUnix = a.NextTick.Unix()
			}
			state.Affects = append(state.Affects, model.PersistedAffect{
				Type: a.Type, ClientType: a.ClientType, Value: a.Value, Level: a.Level,
				OwnerID: a.OwnerID, OwnerCharacterUID: a.OwnerCharacterUID,
				// The sidecar links absolute expiry to the consumed item's UID.
				SourceItemUID: a.SourceItemUID, SourceItemIndex: a.SourceItemIndex,
				ExpiresUnix:       a.ExpiresAt.Unix(),
				FinalTickAtExpiry: a.FinalTickAtExpiry,
				NextTickUnix:      nextTickUnix,
			})
		}
	}
	for name, value := range p.SpecialCoins {
		if value == 0 {
			continue
		}
		if state.SpecialCoins == nil {
			state.SpecialCoins = make(map[string]uint32, len(p.SpecialCoins))
		}
		state.SpecialCoins[name] = value
	}
	return state
}

// applyCharState restores currencies and unexpired affects from absolute expiry.
func (w *World) applyCharState(p *Player, state *model.CharState, now time.Time) {
	if p == nil || p.Char == nil || state == nil {
		return
	}
	slot := 0
	for _, pa := range state.Affects {
		if pa.Type == 0 || pa.ExpiresUnix <= now.Unix() {
			continue
		}
		// A player-owned affect without a stable character identity belongs to
		// an older sidecar format. Restoring it by recycled OwnerID could bind a
		// debuff to another character, so discard it rather than resurrecting a
		// stale effect. OwnerID-only affects with no source are still valid
		// server buffs (OwnerID=0) and remain loadable.
		if pa.OwnerID != 0 && pa.OwnerCharacterUID == "" {
			continue
		}
		for slot < len(p.Char.Affects) && p.Char.Affects[slot].Type != 0 {
			slot++
		}
		if slot >= len(p.Char.Affects) {
			break
		}
		// Older sidecars may contain raw Critical Armor type 50. Restore its
		// semantics and visual together: score type 31 and native visual slot 24.
		if pa.Type == 50 {
			pa.Type = 31
			if pa.ClientType == 0 || pa.ClientType == 50 {
				pa.ClientType = 24
			}
		}
		clientType := pa.ClientType
		if clientType == 0 {
			clientType = pa.Type
		}
		// Older saves published the face transformation in the client's stun
		// slot 40; restore it in the Transformation slot so a relogin does not
		// freeze the character.
		if pa.Type == affectFaceTransform {
			clientType = affectFaceTransformClientType
		}
		p.Char.Affects[slot] = model.Affect{
			Type: pa.Type, ClientType: clientType, Value: pa.Value, Level: pa.Level,
			OwnerID: pa.OwnerID, OwnerCharacterUID: pa.OwnerCharacterUID,
			// Reconnect preserves the economic origin; optional empty fields
			// keep older sidecars compatible.
			SourceItemUID: pa.SourceItemUID, SourceItemIndex: pa.SourceItemIndex,
			ExpiresAt: time.Unix(pa.ExpiresUnix, 0),
			NextTick:  now.Add(8 * time.Second),
		}
		if pa.Type == 20 && pa.FinalTickAtExpiry && pa.NextTickUnix > 0 && pa.NextTickUnix <= pa.ExpiresUnix {
			p.Char.Affects[slot].FinalTickAtExpiry = true
			p.Char.Affects[slot].NextTick = time.Unix(pa.NextTickUnix, 0)
		}
		slot++
	}
	// Replace, never merge: selection reuses Player. Merging could duplicate
	// the previous character's currencies into the next character's autosave.
	p.SpecialCoins = nil
	if len(state.SpecialCoins) > 0 {
		p.SpecialCoins = make(map[string]uint32, len(state.SpecialCoins))
		for name, value := range state.SpecialCoins {
			p.SpecialCoins[name] = value
		}
	}
}

// loadCharStateInto restores buffs and currencies before publishing world entry.
func (w *World) loadCharStateInto(p *Player) error {
	if p == nil || p.Char == nil {
		return nil
	}
	if w.criticalPlayerReserved(p) {
		return errCriticalPersistenceBusy
	}
	var state *model.CharState
	if stateStore, ok := w.store.(charStateStore); ok {
		loaded, err := stateStore.LoadCharState(p.Char.UID)
		if err != nil {
			return err
		}
		state = loaded
	}
	// Clear only after a successful read. Otherwise autosave could persist
	// the loss of authoritative buffs and counters after a failed load.
	p.Char.Affects = [16]model.Affect{}
	p.SpecialCoins = nil
	if state != nil {
		w.applyCharState(p, state, w.now())
	}
	return nil
}

// saveCharState synchronously persists buffs and currencies on logout or
// disconnection, before releasing Player. Empty state removes its sidecar.
func (w *World) saveCharState(p *Player) {
	if err := w.saveCharStateResult(p); err != nil {
		log.Printf("[#%d] save charstate %q failed: %v", p.Session.ID, p.Char.Name, err)
	}
}

// saveCharStateResult returns a standalone write error. Operations changing
// account and charstate together must use the atomic aggregate helper instead.
func (w *World) saveCharStateResult(p *Player) error {
	if w.criticalPlayerReserved(p) {
		return errCriticalPersistenceBusy
	}
	if err := w.syncPersistenceAllowed("character state"); err != nil {
		return err
	}
	store, ok := w.store.(charStateStore)
	if !ok || p == nil || p.Char == nil {
		return nil
	}
	return store.SaveCharState(p.Char.UID, buildCharState(p, w.now()))
}

// saveAccountAndCharStateResult requires an atomic transaction when both
// account and character sidecar persistence are supported.
func (w *World) saveAccountAndCharStateResult(p *Player) error {
	if p == nil || p.Account == nil || p.Char == nil {
		return nil
	}
	state := buildCharState(p, w.now())
	return w.saveAccountAndCharStateForUID(p.Account, p.Char.UID, state)
}

// saveAccountAndCharStateForUID keeps identity explicit when an operation
// replaces or removes Player.Char, such as Celestial sealing or extraction.
// PostgreSQL can delete the sidecar in the same transaction as the cleared slot.
func (w *World) saveAccountAndCharStateForUID(account *model.Account, uid string, state *model.CharState) error {
	if account == nil {
		return nil
	}
	if w.criticalAccountStateReserved(account) || w.criticalCharacterReserved(uid) {
		return errCriticalPersistenceBusy
	}
	if err := w.syncPersistenceAllowed("account and character state"); err != nil {
		return err
	}
	snapshot := accountPersistenceSnapshot(account)
	if transactional, ok := w.store.(playerStateStore); ok {
		return transactional.SavePlayerState(nil, snapshot, uid, state)
	}
	if _, ok := w.store.(charStateStore); ok && uid != "" {
		// Separate writes cannot be made atomic by ordering or compensating:
		// a crash can occur between them. Reject before writing either state.
		return errors.New("store does not support atomic account and character state persistence")
	}
	return w.store.SaveAccount(snapshot)
}

// asyncCharStateStore exposes asynchronous character-state autosave.
type asyncCharStateStore interface {
	SaveCharStateAsync(characterUID string, state *model.CharState) error
}

// saveCharStateAsync writes outside the game loop when supported, otherwise
// it uses the synchronous standalone save.
func (w *World) saveCharStateAsync(p *Player) {
	if p == nil || p.Char == nil || p.PersistencePoisoned {
		return
	}
	// Under the critical coordinator the sidecar is written atomically with the
	// account snapshot; a standalone write could race a newer committed state.
	// A busy or full queue is fine: the next autosave carries the same state.
	if w.criticalWrites != nil {
		if err := w.queueAccountAutoSave(p, w.now()); err != nil && !errors.Is(err, errCriticalPersistenceBusy) &&
			!errors.Is(err, errCriticalPersistenceFull) && !errors.Is(err, errCriticalPersistenceIdentity) {
			log.Printf("[#%d] queue character state save for %q failed: %v", p.Session.ID, p.Char.Name, err)
		}
		return
	}
	if w.criticalPlayerReserved(p) || w.syncPersistenceAllowed("legacy character state autosave") != nil {
		return
	}
	state := buildCharState(p, w.now())
	if as, ok := w.store.(asyncCharStateStore); ok {
		if err := as.SaveCharStateAsync(p.Char.UID, state); err != nil {
			log.Printf("[#%d] autosave charstate %q failed: %v", p.Session.ID, p.Char.Name, err)
		}
		return
	}
	w.saveCharState(p)
}
