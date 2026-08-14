package game

import "wydgo/internal/model"

const fairySlot = 13

type fairyBonus struct {
	expPercent  int
	dropPercent int
}

// fairyBonusByIndex contains only the functional fairy progression. Lifetime
// is owned by the UID-bound timed-item subsystem in timed_items.go.
func fairyBonusByIndex(index uint16) fairyBonus {
	switch index {
	case 3900, 3903, 3906, 3911, 3912, 3913: // Green Angel
		return fairyBonus{expPercent: 16}
	case 3901, 3904, 3907: // Blue Angel
		return fairyBonus{dropPercent: 32}
	case 3902, 3905, 3908: // Red Angel
		return fairyBonus{expPercent: 8, dropPercent: 16}
	case 3914: // Silver Angel
		return fairyBonus{expPercent: 16, dropPercent: 32}
	case 3915: // Gold Angel
		return fairyBonus{expPercent: 24, dropPercent: 48}
	default:
		return fairyBonus{}
	}
}

func isFairyIndex(index uint16) bool {
	return fairyBonusByIndex(index) != (fairyBonus{})
}

func (w *World) activeFairyBonus(ch *model.Char) fairyBonus {
	if w == nil || ch == nil {
		return fairyBonus{}
	}
	item := ch.Equip[fairySlot]
	bonus := fairyBonusByIndex(item.Index)
	if bonus == (fairyBonus{}) {
		return fairyBonus{}
	}
	def, ok := w.items[item.Index]
	if !ok || timedItemDuration(def, item.Index, fairySlot) <= 0 {
		return fairyBonus{}
	}
	// The normal equip/enter path activates the UID before the first score is
	// published. Accepting zero here only covers the transient plan phase.
	if item.ExpiresUnix == 0 || item.ExpiresUnix > w.now().Unix() {
		return bonus
	}
	return fairyBonus{}
}

func (w *World) hasActiveSilverFairy(ch *model.Char) bool {
	if ch == nil || ch.Equip[fairySlot].Index != 3914 {
		return false
	}
	bonus := w.activeFairyBonus(ch)
	return bonus.expPercent == 16 && bonus.dropPercent == 32
}
