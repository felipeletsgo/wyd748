package game

import (
	"errors"
	"log"

	"wydgo/internal/model"
)

// Key consumption is durable; the permanent gate is runtime state. Retain the
// gate until the account settles so another player cannot spend a second key.
func (w *World) queueGateKey(p *Player, gate *GroundItem, slot byte, consume bool, code int) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) ||
		int(slot) >= model.PlayerCarrySlots || gate == nil || w.groundItems[gate.ID] != gate ||
		!gate.Permanent || gate.State != gateClosed || !p.InWorld || playerCurHP(p.Char) == 0 ||
		chebyshev(p.X, p.Y, gate.X, gate.Y) > gateReach {
		return errCriticalPersistenceIdentity
	}
	if gate.criticalReserved() || w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	source := p.Char.Inv[slot]
	keyDef, keyOK := w.items[source.Index]
	gateDef, gateOK := w.items[gate.Item.Index]
	keyID := staticEffect(keyDef, "EF_KEYID")
	gateKeyID := staticEffect(gateDef, "EF_KEYID")
	if !keyOK || !gateOK || !playerGateKeyID(keyID) || !playerGateKeyID(gateKeyID) || gateKeyID != keyID {
		return errors.New("critical gate: key does not match the gate")
	}
	proposed := accountStateSnapshot(p.Account)
	if consume {
		consumeOne(&proposed.Chars[p.CharSlot].Inv[slot])
	}
	context, oldX, oldY, admitted := *gate, p.X, p.Y, false
	hooks := criticalAccountMutationHooks{
		check: func() error {
			current := *gate
			current.criticalGate = false
			if w.groundItems[context.ID] != gate || current != context || gate.criticalGate != admitted ||
				p.X != oldX || p.Y != oldY {
				return errCriticalPersistenceIdentity
			}
			return nil
		},
		reject: func() {
			gate.criticalGate = false
			if !p.Session.IsClosed() {
				p.Session.Send(groundItemUpdatePacket(gate))
			}
		},
		afterAck: func() {
			gate.criticalGate = false
			for _, observer := range w.nearbyWorldPlayers(gate.X, gate.Y, viewHalfX) {
				if observer.hasVisible(gate.ID) && !observer.Session.IsClosed() {
					observer.Session.Send(groundItemUpdatePacket(gate))
				}
			}
			log.Printf("[#%d] committed gate opening object=%d key=%d keyid=%d volatile=%d", p.Session.ID, gate.ID, source.Index, keyID, code)
		},
	}
	err := w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "gate key", "The door could not be opened.",
		func() {
			p.Char.Inv[slot] = proposed.Chars[p.CharSlot].Inv[slot]
			gate.State = gateOpen
			w.applyGroundItemHeight(gate)
			w.scheduleGateRelock(gate)
		}, func() { resyncUsedItem(p.Session, p, slot) }, true, hooks)
	if err == nil {
		admitted, gate.criticalGate = true, true
	}
	return err
}

// The detached entity owns an ID but is absent from all gameplay/spatial maps
// until acknowledgement. Rejection releases it; unknown outcomes retain it.
func (w *World) queueContractSummon(p *Player, slot byte, rule model.VolatileRule) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) ||
		int(slot) >= model.PlayerCarrySlots || !p.InWorld || playerCurHP(p.Char) == 0 {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	plan, ok := w.planContractSummon(p, rule.Summon)
	if !ok {
		return errors.New("critical contract summon: no eligible summon allocation")
	}
	proposed := accountStateSnapshot(p.Account)
	if rule.Consume {
		consumeOne(&proposed.Chars[p.CharSlot].Inv[slot])
	}
	m, oldX, oldY, admitted := plan.summon, p.X, p.Y, false
	hooks := criticalAccountMutationHooks{
		check: func() error {
			reserved := w.criticalSummonIDs[m.ID]
			if w.mobsByID[m.ID] != nil || w.groundItems[m.ID] != nil ||
				(!admitted && reserved != nil) || (admitted && reserved != m) ||
				p.X != oldX || p.Y != oldY || w.playerRuntimeInstanceID(p.ID) != m.InstanceID {
				return errCriticalPersistenceIdentity
			}
			return nil
		},
		reject: func() { delete(w.criticalSummonIDs, m.ID) },
		afterAck: func() {
			// Runtime coordinates are not in the account write. Resolve occupancy
			// again after the wait without changing the materialized entity ID.
			m.X, m.Y = w.findFreeGameplayPosition(p, nil, oldX, oldY, 3)
			delete(w.criticalSummonIDs, m.ID)
			w.commitContractSummon(plan)
		},
	}
	err := w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "contract summon", "The summon could not be created.",
		func() { p.Char.Inv[slot] = proposed.Chars[p.CharSlot].Inv[slot] },
		func() { resyncUsedItem(p.Session, p, slot) }, true, hooks)
	if err == nil {
		if w.criticalSummonIDs == nil {
			w.criticalSummonIDs = make(map[uint16]*Mob)
		}
		admitted, w.criticalSummonIDs[m.ID] = true, m
	}
	return err
}

func (w *World) admitContractSummon(p *Player, slot byte, rule model.VolatileRule) {
	if err := w.queueContractSummon(p, slot, rule); err != nil {
		resyncUsedItem(p.Session, p, slot)
		if !errors.Is(err, errCriticalPersistenceBusy) {
			log.Printf("[#%d] contract summon admission rejected: %v", p.Session.ID, err)
		}
	}
}
