package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Stack compatibility excludes identity but preserves all other instance data.
// EF_AMOUNT is quantity except on mounts, where effect 61 holds mount HP.
func stackSignature(item model.Item) ([256]int16, uint32, bool) {
	var signature [256]int16
	if item.Index == 0 || item.Index >= 2330 && item.Index <= 2389 {
		return signature, 0, false
	}
	var amount uint32
	for pair := 0; pair < 3; pair++ {
		id, value := item.Eff[pair*2], item.Eff[pair*2+1]
		if id == 0 {
			if value != 0 {
				return signature, 0, false
			}
			continue
		}
		if signature[id] != 0 {
			return signature, 0, false
		}
		signature[id] = int16(value) + 1
		if id == effectAmount {
			amount = uint32(value)
			signature[id] = 1
		}
	}
	return signature, amount, amount > 0
}

// Native 0x376 names the clicked target first and the dragged origin second.
// A full target is handled without swapping the stacks. Residual quantity and
// its UID stay in the origin; a consumed origin is cleared in the same write.
func mergeStackItems(target, origin model.Item) (model.Item, model.Item, bool) {
	targetSignature, targetAmount, targetOK := stackSignature(target)
	originSignature, originAmount, originOK := stackSignature(origin)
	if !targetOK || !originOK || target.Index != origin.Index || targetSignature != originSignature ||
		target.ActivatedUnix != origin.ActivatedUnix || target.ExpiresUnix != origin.ExpiresUnix ||
		target.UID != "" && target.UID == origin.UID {
		return target, origin, false
	}
	transfer := minU32(255-targetAmount, originAmount)
	if transfer == 0 {
		return target, origin, true
	}
	setExactItemAmount(&target, targetAmount+transfer)
	if transfer == originAmount {
		origin = model.Item{}
	} else {
		setExactItemAmount(&origin, originAmount-transfer)
	}
	return target, origin, true
}

func (w *World) tryStackMerge(p *Player, st, sp, dt, dp byte, targetID uint16) bool {
	if (st != placeInv && st != placeStorage) || (dt != placeInv && dt != placeStorage) || st == dt && sp == dp {
		return false
	}
	src, dst := playerSlotOf(p, st, sp), playerSlotOf(p, dt, dp)
	if src == nil || dst == nil {
		return false
	}
	target, origin, compatible := mergeStackItems(*src, *dst)
	if !compatible {
		return false
	}
	resync := func() {
		p.Session.Send(wire.SendItem(p.ID, st, sp, *src))
		p.Session.Send(wire.SendItem(p.ID, dt, dp, *dst))
	}
	publish := func() {
		// The native acknowledgement swaps local cells and finishes the cursor.
		// Authoritative snapshots must follow it, never precede that local swap.
		p.Session.Send(wire.SwapItem(p.ID, st, sp, dt, dp, targetID))
		resync()
	}
	if target == *src && origin == *dst {
		publish()
		return true
	}
	// Economic mutations require the S2 coordinator. Never fall back to an
	// inline save whose uncertain outcome could publish or undo a committed merge.
	if w.criticalWrites == nil {
		resync()
		p.Session.Send(wire.MessagePanel("Item stacking is unavailable. Please try again later."))
		return true
	}
	proposed := accountStateSnapshot(p.Account)
	planned := &Player{Account: proposed, Char: &proposed.Chars[p.CharSlot]}
	*playerSlotOf(planned, st, sp), *playerSlotOf(planned, dt, dp) = target, origin
	err := w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "stack merge",
		"Save failed. The items were not stacked.", func() {
			*src, *dst = target, origin
		}, publish, true, criticalAccountMutationHooks{reject: resync})
	if err != nil {
		resync()
		p.Session.Send(wire.MessagePanel("Item stacking is unavailable. Please try again later."))
		log.Printf("[#%d] stack merge admission rejected: %v", p.Session.ID, err)
	}
	return true
}
