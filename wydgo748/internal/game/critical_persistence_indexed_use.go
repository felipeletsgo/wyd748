package game

import "wydgo/internal/model"

// The roll and catalog validation have already run on World. Freeze that one
// result with its consumed source; completion never rerolls or retries it.
func (w *World) queueCriticalIndexedUse(p *Player, source byte, targetType, targetPos uint32,
	result model.Item, publish func()) error {
	if p == nil || !bindCriticalPlayer(p).current(w) || int(source) >= model.PlayerCarrySlots {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	if ch.Inv[source].Index == 0 {
		return errCriticalPersistenceIdentity
	}
	switch targetType {
	case placeEquip:
		if int(targetPos) >= len(ch.Equip) {
			return errCriticalPersistenceIdentity
		}
		ch.Equip[targetPos] = result
	case placeInv:
		if int(targetPos) >= len(ch.Inv) || targetPos == uint32(source) {
			return errCriticalPersistenceIdentity
		}
		ch.Inv[targetPos] = result
	default:
		return errCriticalPersistenceIdentity
	}
	consumeOne(&ch.Inv[source])
	if targetType == placeEquip {
		w.recalcPlayer(ch)
	}
	inv, equip := ch.Inv, ch.Equip
	return w.queueCriticalAccountMutation(p, proposed, "equipment item use", "Item use could not be saved.", func() {
		p.Char.Inv, p.Char.Equip = inv, equip
		if targetType == placeEquip {
			w.recalcPlayer(p.Char)
		}
	}, publish, true)
}
