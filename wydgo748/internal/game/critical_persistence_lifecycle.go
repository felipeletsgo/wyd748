package game

import (
	"errors"
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

var errCriticalPersistenceIdentity = errors.New("critical persistence: participant identity changed")

// Nonzero size ensures distinct allocations have distinct pointer identities.
// A reset drops the token; no numeric generation can wrap back to an old one.
type criticalPlayerLifecycle struct{ marker byte }

// These bindings never leave the World goroutine. Workers receive only the
// coordinator's detached snapshots, not sessions, players or these tokens.
type criticalPlayerBinding struct {
	player     *Player
	session    *net.Session
	account    *model.Account
	accountKey string
	character  *model.Char
	uid        string
	slot       int
	id         uint16
	inWorld    bool
	lifecycle  *criticalPlayerLifecycle
}

// submitCriticalWrite is the identity-bound admission path for gameplay
// producers. The low-level owner.submit is not a lifecycle-safe producer API.
// Proposed account copies may differ from live state but must match exactly one
// authenticated participant per account. No rejection installs a lifecycle token.
func (w *World) submitCriticalWrite(write criticalWrite, participants []*Player, apply func(criticalPersistenceResult) error) (uint64, error) {
	return w.submitCriticalWriteBound(write, participants, false, apply)
}

// Only a retained physical disconnect may admit a closed transport. Gameplay
// producers must continue to reject closed sessions through the public wrapper.
func (w *World) submitCriticalWriteBound(write criticalWrite, participants []*Player, disconnect bool, apply func(criticalPersistenceResult) error) (uint64, error) {
	return w.submitCriticalWritePhased(write, participants, disconnect, apply, nil)
}

// Both phases are actor-owned and bound to the same authenticated identities.
// afterAck runs before completion delivery yields to any lifecycle consumer.
func (w *World) submitCriticalWritePhased(write criticalWrite, participants []*Player, disconnect bool, apply, afterAck func(criticalPersistenceResult) error) (uint64, error) {
	return w.submitCriticalWriteWithRejection(write, participants, disconnect, apply, afterAck, nil)
}

// Shared gameplay objects need global containment if settlement fails, even
// when the database transaction only writes accounts. Record this on the actor,
// before any completion can be delivered; workers receive no live-object data.
func (w *World) submitCriticalSharedWriteWithRejection(write criticalWrite, participants []*Player, apply, afterAck, reject func(criticalPersistenceResult) error) (uint64, error) {
	id, err := w.submitCriticalWriteWithRejection(write, participants, false, apply, afterAck, reject)
	if err == nil {
		w.criticalWrites.continuations[id].sharedWorld = true
	}
	return id, err
}

// Rejection recovery shares the success path's identity/lifecycle bindings.
// Each opting-in producer must additionally validate its retained economic and
// shared contexts before settling failure effects under the reservation.
func (w *World) submitCriticalWriteWithRejection(write criticalWrite, participants []*Player, disconnect bool, apply, afterAck, reject func(criticalPersistenceResult) error) (uint64, error) {
	if w.shuttingDown {
		return 0, errCriticalPersistenceStopping
	}
	if w.criticalWrites == nil || apply == nil || len(participants) == 0 ||
		len(participants) != len(write.accounts) || len(participants) > criticalPersistenceMaxAccounts {
		return 0, errors.New("critical persistence: missing owner, continuation or participant set")
	}
	bindings := make([]criticalPlayerBinding, len(participants))
	seen := make(map[*Player]bool, len(participants))
	for i, player := range participants {
		if player == nil || player.Session == nil ||
			player.Account == nil || player.PersistencePoisoned || seen[player] || write.accounts[i] == nil {
			return 0, errCriticalPersistenceIdentity
		}
		if disconnect {
			retained := w.criticalDisconnects[player.Session]
			if !player.Session.IsClosed() || retained == nil || retained.player != player || retained.saved {
				return 0, errCriticalPersistenceIdentity
			}
		} else if player.Session.IsClosed() {
			return 0, errCriticalPersistenceIdentity
		}
		seen[player] = true
		binding := bindCriticalPlayer(player)
		if binding.accountKey == "" || binding.accountKey != accountSessionKey(write.accounts[i].Name) || !binding.current(w) {
			return 0, errCriticalPersistenceIdentity
		}
		if binding.lifecycle == nil {
			binding.lifecycle = &criticalPlayerLifecycle{}
		}
		bindings[i] = binding
	}
	checkIdentity := func() error {
		// No accepted job may settle after World entered maintenance.
		if w.shutdownErr != nil {
			return errors.New("critical persistence: maintenance prevents publication")
		}
		for _, binding := range bindings {
			if !binding.current(w) {
				return fmt.Errorf("%w: account %q", errCriticalPersistenceIdentity, binding.accountKey)
			}
		}
		return nil
	}
	checkSuccess := func(result criticalPersistenceResult) error {
		// An unknown durable outcome is not permission to publish or roll back.
		if result.err != nil {
			return result.err
		}
		return checkIdentity()
	}
	var publish func(criticalPersistenceResult) error
	if afterAck != nil {
		publish = func(result criticalPersistenceResult) error {
			if err := checkSuccess(result); err != nil {
				return err
			}
			return afterAck(result)
		}
	}
	var rejectBound func(criticalPersistenceResult) error
	if reject != nil {
		rejectBound = func(result criticalPersistenceResult) error {
			if err := checkIdentity(); err != nil {
				return err
			}
			return reject(result)
		}
	}
	id, err := w.criticalWrites.submitWithRejection(write, func(result criticalPersistenceResult) error {
		if err := checkSuccess(result); err != nil {
			return err
		}
		return apply(result)
	}, publish, rejectBound)
	if err != nil {
		return 0, err
	}
	for _, binding := range bindings {
		binding.player.criticalLifecycle = binding.lifecycle
	}
	return id, nil
}

func bindCriticalPlayer(player *Player) criticalPlayerBinding {
	binding := criticalPlayerBinding{
		player: player, session: player.Session, account: player.Account,
		accountKey: accountSessionKey(player.Account.Name), character: player.Char,
		slot: player.CharSlot, id: player.ID, inWorld: player.InWorld,
		lifecycle: player.criticalLifecycle,
	}
	if player.Char != nil {
		binding.uid = player.Char.UID
	}
	return binding
}

func (binding criticalPlayerBinding) current(w *World) bool {
	p := binding.player
	if w.players[binding.session] != p || p.Session != binding.session ||
		p.Account != binding.account || accountSessionKey(p.Account.Name) != binding.accountKey ||
		w.accountSessions[binding.accountKey] != binding.session || p.PersistencePoisoned ||
		p.Char != binding.character || p.CharSlot != binding.slot || p.ID != binding.id ||
		p.InWorld != binding.inWorld || p.criticalLifecycle != binding.lifecycle {
		return false
	}
	if p.Char == nil {
		return !p.InWorld && p.ID == 0 && p.CharSlot == -1
	}
	return p.Char.UID == binding.uid && p.CharSlot >= 0 && p.CharSlot < len(p.Account.Chars) &&
		p.Char == &p.Account.Chars[p.CharSlot] && (!p.InWorld || (p.ID != 0 &&
		w.playersByID[p.ID] == p && (binding.uid == "" || w.playersByCharacterUID[binding.uid] == p)))
}

type criticalDisconnect struct {
	player            *Player
	saved             bool
	instancesDetached bool
	instancesSaved    bool
	admissionErr      error
}

// Transport/authentication accounting can be released immediately; account,
// character and spatial ownership cannot. Repeated disconnects retain one entry.
func (w *World) deferCriticalDisconnect(s *net.Session) bool {
	p := w.players[s]
	if p == nil || (w.criticalWrites == nil || p.Account == nil || p.PersistencePoisoned) && !w.criticalPlayerReserved(p) {
		return false
	}
	s.Close()
	if w.criticalDisconnects == nil {
		w.criticalDisconnects = make(map[*net.Session]*criticalDisconnect)
	}
	if w.criticalDisconnects[s] == nil {
		w.criticalDisconnects[s] = &criticalDisconnect{player: p}
	}
	return true
}

// The final account/sidecar save is asynchronous under the staged coordinator.
// Membership detachment runs once on World; final index/ownership cleanup waits
// for a serialized instance snapshot including that detachment. Shared gameplay
// operation migration must still finish before production activation.
func (w *World) resumeCriticalDisconnects(limit int) {
	if w.shuttingDown {
		return
	}
	for session, retained := range w.criticalDisconnects {
		if limit <= 0 {
			return
		}
		player := retained.player
		if w.players[session] != player {
			// Never clean up a replacement using an old disconnect's identity.
			delete(w.criticalDisconnects, session)
			continue
		}
		if w.criticalPlayerWriteReserved(player) || retained.admissionErr != nil {
			continue
		}
		if player.PersistencePoisoned && !retained.saved {
			// Isolated RAM is discarded: the database is authoritative and no
			// final snapshot may overwrite what actually committed.
			retained.saved = true
		}
		limit--
		if retained.saved {
			if !retained.instancesDetached {
				if w.criticalInstanceReserved() {
					// A committed account does not grant permission to change
					// membership retained by another in-flight aggregate.
					continue
				}
				w.detachPlayerFromItemInstances(player.ID, w.now())
				retained.instancesDetached = true
				if !w.instanceStateDirty && !w.criticalInstanceReserved() {
					retained.instancesSaved = true
				} else {
					// Even a no-op detachment arriving behind an older snapshot
					// needs its own covering acknowledgement before readmission.
					w.markInstanceStateDirty()
				}
			}
			if !retained.instancesSaved {
				w.flushInstanceStateIfDirty()
				continue
			}
			if player.Trade != nil {
				opponent := w.playerByID(player.Trade.OpponentID)
				if opponent != nil && opponent.Trade != nil && opponent.Trade.OpponentID == player.ID &&
					w.criticalPlayerWriteReserved(opponent) {
					// A pending reciprocal operation retains both identities and
					// trade pointers. Disconnect intent alone is not an in-flight
					// write and must not deadlock simultaneous final cleanup.
					continue
				}
			}
			w.closeTradeContext(player, "disconnect")
			w.finishDisconnectAfterInstances(session, player)
			delete(w.criticalDisconnects, session)
			continue
		}
		write := criticalWrite{accounts: []*model.Account{accountPersistenceSnapshot(player.Account)}}
		if player.InWorld && player.Char != nil {
			if _, atomic := w.criticalWrites.queue.store.(playerStateStore); atomic {
				write.character = &criticalCharacterState{uid: player.Char.UID, state: buildCharState(player, w.now())}
			} else if _, sidecar := w.criticalWrites.queue.store.(charStateStore); sidecar {
				// Never replace an atomic aggregate with two independent writes.
				retained.admissionErr = errCriticalPersistenceUnsupported
				log.Printf("[#%d] final disconnect admission failed; retaining ownership: %v", session.ID, retained.admissionErr)
				continue
			}
		}
		_, err := w.submitCriticalWriteBound(write, []*Player{player}, true, func(criticalPersistenceResult) error {
			if w.criticalDisconnects[session] != retained {
				return errCriticalPersistenceIdentity
			}
			retained.saved = true
			return nil
		})
		if err != nil && !errors.Is(err, errCriticalPersistenceFull) && !errors.Is(err, errCriticalPersistenceBusy) {
			// A rejected request has no unknown commit to reconcile. Keep it
			// fenced, log once, and allow explicit shutdown's terminal snapshot.
			retained.admissionErr = err
			log.Printf("[#%d] final disconnect admission failed; retaining ownership: %v", session.ID, err)
		}
	}
}
