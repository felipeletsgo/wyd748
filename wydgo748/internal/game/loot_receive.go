package game

import "wydgo/internal/model"

func lootStackUnit(item model.Item) model.Item {
	if itemAmountEffectSlot(item) >= 0 {
		return item
	}
	for pair := 0; pair < 3; pair++ {
		if item.Eff[pair*2] == 0 && item.Eff[pair*2+1] == 0 {
			item.Eff[pair*2], item.Eff[pair*2+1] = effectAmount, 1
			break
		}
	}
	return item
}

// receiveLoot stages all inventory changes. A residual stack without a free
// slot leaves the inventory unchanged, so ground fallback cannot duplicate it.
func (w *World) receiveLoot(ch *model.Char, item model.Item, requested int) ([]int, bool) {
	before := ch.Inv
	staged := *ch
	if ch.Equip[fairySlot].Index == 3915 && w.activeFairyBonus(ch) != (fairyBonus{}) {
		for i := 0; i < model.PlayerCarrySlots && item.Index != 0; i++ {
			candidate, incoming := staged.Inv[i], item
			// Single powders have no EF_AMOUNT. An existing stack also proves
			// that the matching item can carry quantities without changing type.
			if candidate.Index != item.Index {
				continue
			}
			stackable := item.Index == 412 || item.Index == 413 ||
				itemAmountEffectSlot(item) >= 0 || itemAmountEffectSlot(candidate) >= 0 ||
				itemAbility(item, w.items[item.Index], "EF_AMOUNT") > 0
			if !stackable {
				continue
			}
			candidate, incoming = lootStackUnit(candidate), lootStackUnit(incoming)
			target, rest, ok := mergeStackItems(candidate, incoming)
			if ok {
				staged.Inv[i], item = target, rest
			}
		}
	}
	if item.Index != 0 {
		if requested >= 0 && requested < model.PlayerCarrySlots && staged.Inv[requested].Index == 0 {
			staged.Inv[requested] = item
		} else if addToInv(&staged, item) < 0 {
			return nil, false
		}
	}
	var changed []int
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if before[i] != staged.Inv[i] {
			changed = append(changed, i)
		}
	}
	ch.Inv = staged.Inv
	return changed, true
}
