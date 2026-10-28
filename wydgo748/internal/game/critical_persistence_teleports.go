package game

import (
	"errors"
	"log"
	"maps"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// criticalTeleportTile reserves one exact map tile in a gameplay space. It must
// not use spatialKey: a 16x16 spatial cell would let one reservation block the
// whole cell, and callers decode tile as exact coordinates.
type criticalTeleportTile struct {
	tile  uint32
	space string
}

func criticalTile(x, y uint16, space string) criticalTeleportTile {
	return criticalTeleportTile{uint32(x)<<16 | uint32(y), space}
}

func (t criticalTeleportTile) xy() (uint16, uint16) { return uint16(t.tile >> 16), uint16(t.tile) }

// Gold, consumption and hometown binding are one detached account proposal.
// Neither the source position nor destination ownership changes before commit.
func (w *World) queuePlayerTeleport(p *Player, proposed *model.Account, x, y uint16,
	operation string, publish func(), afterAck func(), reject func()) error {
	return w.queuePlayerTeleportState(p, proposed, x, y, operation, publish, afterAck, reject, false)
}

func (w *World) queuePlayerTeleportState(p *Player, proposed *model.Account, x, y uint16,
	operation string, publish func(), afterAck func(), reject func(), revive bool) error {
	return w.queuePlayerTeleportSnapshot(p, proposed, x, y, operation, publish, afterAck, reject, revive, nil, false)
}

// Counter-backed admission commits the balance and position together. A store
// that cannot persist character state must not silently grant a free entry.
func (w *World) queuePlayerTeleportWithCounters(p *Player, proposed *model.Account, counters map[string]uint32,
	x, y uint16, operation string, publish func()) error {
	if p == nil || p.Char == nil || p.Char.UID == "" || w.criticalWrites == nil {
		return errCriticalPersistenceIdentity
	}
	if !w.criticalStoreHasSidecars() {
		return errCriticalPersistenceUnsupported
	}
	state := buildCharState(p, w.now())
	state.SpecialCoins = maps.Clone(counters)
	return w.queuePlayerTeleportSnapshot(p, proposed, x, y, operation, publish, nil, nil, false,
		&criticalCharacterState{uid: p.Char.UID, state: state}, false)
}

func (w *World) queuePlayerTeleportSnapshot(p *Player, proposed *model.Account, x, y uint16,
	operation string, publish func(), afterAck func(), reject func(), revive bool, character *criticalCharacterState,
	preserveHomeCity bool) error {
	if p == nil || p.Char == nil || p.Account == nil || proposed == nil ||
		!bindCriticalPlayer(p).current(w) || w.criticalWrites == nil {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	departure, err := w.prepareCriticalInstanceDeparture(p)
	if err != nil {
		return err
	}
	x, y, ok := w.resolvePlayerTeleportDestination(p, x, y)
	if !ok || w.positionOccupiedExcept(x, y, nil, p) {
		return errors.New("critical teleport: no available destination")
	}
	ch := &proposed.Chars[p.CharSlot]
	ch.X, ch.Y = x, y
	if !preserveHomeCity {
		stagePlayerHomeCity(&Player{Char: ch, X: x, Y: y})
	}
	oldX, oldY, oldSpace := p.X, p.Y, w.playerRuntimeInstanceID(p.ID)
	// Public teleports also commit definitive departure from any retained room.
	// Entry or movement within a private room requires a separate proposal.
	tile, admitted := criticalTile(x, y, ""), false
	deadAt := p.DeadAt
	var corpseObservers []*Player
	if revive {
		for _, observer := range w.nearbyWorldPlayers(oldX, oldY, viewHalfX) {
			if observer != p && observer.Session != nil && observer.hasVisible(p.ID) {
				corpseObservers = append(corpseObservers, observer)
			}
		}
	}
	hooks := criticalAccountMutationHooks{
		check: func() error {
			owner := w.criticalTeleportTiles[tile]
			if p.X != oldX || p.Y != oldY || p.DeadAt != deadAt || w.playerRuntimeInstanceID(p.ID) != oldSpace ||
				(!admitted && owner != nil) || (admitted && owner != p) ||
				w.positionOccupiedExcept(x, y, nil, p) {
				return errCriticalPersistenceIdentity
			}
			return departure.check(w)
		},
		reject: func() {
			delete(w.criticalTeleportTiles, tile)
			if reject != nil {
				reject()
			}
			if !p.Session.IsClosed() {
				p.Session.Send(wire.ActionStop(p.ID, p.X, p.Y))
			}
		},
		afterAck: func() {
			delete(w.criticalTeleportTiles, tile)
			for _, observer := range corpseObservers {
				// Only retire a corpse still present in the observer's view.
				if w.players[observer.Session] == observer && observer.InWorld && observer.hasVisible(p.ID) {
					observer.Session.Send(wire.RemoveMob(p.ID, 3))
					observer.hide(p.ID)
				}
			}
			// Maintain indexes even if the socket closed during the write.
			w.publishPlayerTeleport(p)
			if departure != nil {
				clearItemInstanceHUD(p)
			}
			if afterAck != nil {
				afterAck()
			}
		},
	}
	returnErr := w.queueCriticalAccountWrite(p, proposed, criticalWrite{instances: departure.snapshot(), character: character}, operation,
		"The teleport could not be completed.", func() {
			departure.install(w)
			p.Char.Gold, p.Char.Inv = ch.Gold, ch.Inv
			if character != nil {
				p.SpecialCoins = maps.Clone(character.state.SpecialCoins)
			}
			if revive {
				setPlayerCurHP(p.Char, playerCurHP(ch))
				p.DeadAt = time.Time{}
			}
			p.X, p.Y, p.Char.X, p.Char.Y = x, y, x, y
			if p.Char.Score != nil && ch.Score != nil {
				p.Char.Score.Merchant = ch.Score.Merchant
			}
			if p.Char.RuntimeScore != nil && ch.RuntimeScore != nil {
				p.Char.RuntimeScore.Merchant = ch.RuntimeScore.Merchant
			}
			clearAirMove(p)
			clearPublishedPlayerMove(p)
		}, publish, true, hooks)
	if returnErr == nil {
		if w.criticalTeleportTiles == nil {
			w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
		}
		admitted, w.criticalTeleportTiles[tile] = true, p
	}
	return returnErr
}

func criticalTeleportConsumable(action string) bool {
	return action == "teleport" || action == "recall" || action == "warp_saved" ||
		action == "hunting_teleport" || action == "territory_pass"
}

func (w *World) admitTeleportConsumable(p *Player, slot byte, rule model.VolatileRule, req useItemRequest) {
	x, y := rule.X, rule.Y
	fail := func(message string) {
		resyncUsedItem(p.Session, p, slot)
		if message != "" {
			p.Session.Send(wire.MessagePanel(message))
		}
	}
	switch rule.Action {
	case "recall":
		_, x, y = w.recallSpawn(p.Char)
		x, y = w.findFreePlayerPosition(x, y, 3, p)
	case "warp_saved":
		x, y = p.Char.SavedX, p.Char.SavedY
		if x == 0 || y == 0 {
			fail("No position saved.")
			return
		}
	case "hunting_teleport":
		if req.warpID < 1 || req.warpID > 10 || len(rule.Destinations) != 10 {
			fail("")
			return
		}
		dest := rule.Destinations[int(req.warpID)-1]
		x, y = dest.X, dest.Y
	case "territory_pass":
		if !matchesEvolution(p.Char, rule.RequiredEvolution) {
			fail("This pass does not belong to your evolution.")
			return
		}
	case "ox_ticket":
		if message := w.oxEntryRejection(p, p.Char.Inv[slot]); message != "" {
			fail(message)
			return
		}
	}
	if p.Account == nil {
		fail("")
		return
	}
	proposed := accountStateSnapshot(p.Account)
	if rule.Consume {
		consumeOne(&proposed.Chars[p.CharSlot].Inv[slot])
	}
	var afterAck, reject func()
	if rule.Action == "ox_ticket" {
		o := w.ox
		o.admitting[p] = true
		afterAck = func() { delete(o.admitting, p); w.admitOXMember(p) }
		reject = func() {
			delete(o.admitting, p)
			fail("OX entry could not be saved. Your Entrance Ticket was not consumed.")
		}
	}
	if err := w.queuePlayerTeleportSnapshot(p, proposed, x, y, "teleport item",
		func() { resyncUsedItem(p.Session, p, slot) }, afterAck, reject, false, nil,
		rule.Action == "recall"); err != nil {
		if rule.Action == "ox_ticket" {
			delete(w.ox.admitting, p)
		}
		fail("")
		if !errors.Is(err, errCriticalPersistenceBusy) {
			log.Printf("[#%d] teleport item admission rejected: %v", p.Session.ID, err)
		}
	}
}

// requestPlayerTeleport reports admission, never durable success. Callers put
// dependent effects in afterAck; legacy fixtures complete that callback inline.
func (w *World) requestPlayerTeleport(p *Player, x, y uint16, afterAck func(), reject func()) bool {
	if w.criticalWrites == nil {
		if !w.teleportPlayer(p, x, y) {
			return false
		}
		if afterAck != nil {
			afterAck()
		}
		return true
	}
	if p == nil || p.Account == nil {
		return false
	}
	return w.queuePlayerTeleport(p, accountStateSnapshot(p.Account), x, y,
		"player teleport", nil, afterAck, reject) == nil
}
