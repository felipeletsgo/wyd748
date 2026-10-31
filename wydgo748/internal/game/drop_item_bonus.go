package game

import "wydgo/internal/model"

// drop_item_bonus.go ports the native item option generator SetItemBonus
// (TMSRVIN FUN_005109B0, thunk 0x4B1AA0). MobKilled calls it for every carry
// drop with (mob level, 0, killer DropBonus, 0). Evidence and decisions:
// DOCS/server/drop-item-bonus.md.

// Native effect codes (data/catalog/ItemEffect.h).
const (
	itemBonusDamage     = 2   // EF_DAMAGE
	itemBonusAC         = 3   // EF_AC
	itemBonusHP         = 4   // EF_HP
	itemBonusAttSpeed   = 26  // EF_ATTSPEED
	itemBonusSanc       = 43  // EF_SANC
	itemBonusResistAll  = 54  // EF_RESISTALL
	itemBonusUnique     = 59  // EF_UNIQUE
	itemBonusMagic      = 60  // EF_MAGIC
	itemBonusAmount     = 61  // EF_AMOUNT
	itemBonusCritical2  = 71  // EF_CRITICAL2
	itemBonusACAdd2     = 72  // EF_ACADD2
	itemBonusDamage2    = 73  // EF_DAMAGE2
	itemBonusSpecialAll = 74  // EF_SPECIALALL
	itemBonusIncubate   = 78  // EF_INCUBATE
	itemBonusGrade0     = 100 // EF_GRADE0
	itemBonusGrade5     = 105 // EF_GRADE5
	itemBonusWDay       = 106 // EF_WDAY
	itemBonusHour       = 107 // EF_HOUR
	itemBonusMin        = 108 // EF_MIN
)

// itemBonusMaxIndex is the native drop guard: SetItemBonus runs only for
// carry indexes in (0, 6500).
const itemBonusMaxIndex = 6500

// itemBonusTypes is dword_63CC44[0..9]: the slot-0 option effects.
var itemBonusTypes = [10]byte{
	itemBonusHP, itemBonusResistAll, 11, 12, 13, 14, 7, 8, 9, 10,
}

// itemBonusRanges is the 100-int table at 0x63D120 (dword_63D120, dword_63D124,
// dword_63D2A8, dword_63D2AC): [level difference 0..4][option][min, max].
var itemBonusRanges = [5][10][2]int{
	{{10, 30}, {2, 5}, {3, 10}, {2, 8}, {2, 8}, {2, 8}, {2, 6}, {2, 6}, {2, 6}, {2, 6}},
	{{20, 45}, {4, 8}, {7, 20}, {5, 14}, {5, 14}, {5, 14}, {4, 12}, {4, 12}, {4, 12}, {4, 12}},
	{{30, 60}, {5, 10}, {10, 30}, {8, 20}, {8, 20}, {8, 20}, {6, 16}, {6, 16}, {6, 16}, {6, 16}},
	{{30, 60}, {5, 10}, {10, 30}, {8, 20}, {8, 20}, {8, 20}, {6, 16}, {6, 16}, {6, 16}, {6, 16}},
	{{30, 60}, {5, 10}, {10, 30}, {8, 20}, {8, 20}, {8, 20}, {6, 16}, {6, 16}, {6, 16}, {6, 16}},
}

// itemBonusOption is one option candidate: effect, value step per grade, and
// grade adjustment.
type itemBonusOption struct {
	effect byte
	step   int
	adjust int
}

func isWeaponPos(pos int) bool { return pos == 64 || pos == 192 }

// itemBonusFirstOption is the first switch on the item position (stEffect[1]).
func itemBonusFirstOption(pos, unique, choice int, armorMagic bool) itemBonusOption {
	switch pos {
	case 2:
		switch choice {
		case 0:
			return itemBonusOption{itemBonusAttSpeed, 3, 0}
		case 1:
			return itemBonusOption{itemBonusMagic, 2, 0}
		}
	case 4, 8:
		if armorMagic && choice == 1 {
			return itemBonusOption{itemBonusAC, 5, 0}
		}
		if !armorMagic || choice == 0 {
			return itemBonusOption{itemBonusCritical2, 10, 1}
		}
	case 16:
		return itemBonusOption{itemBonusACAdd2, 5, 0}
	case 32:
		return itemBonusOption{itemBonusDamage2, 6, -1}
	case 64, 192:
		if unique == 44 || unique == 47 {
			switch choice {
			case 0:
				return itemBonusOption{itemBonusMagic, 4, -1}
			case 1:
				return itemBonusOption{itemBonusSpecialAll, 3, 1}
			}
			break
		}
		switch choice {
		case 0:
			return itemBonusOption{itemBonusAttSpeed, 3, 1}
		case 1:
			return itemBonusOption{itemBonusDamage, 9, -1}
		case 2:
			return itemBonusOption{itemBonusSpecialAll, 3, 0}
		}
	}
	return itemBonusOption{itemBonusUnique, 0, 0}
}

// itemBonusSecondOption is the second switch on the item position (stEffect[2]).
func itemBonusSecondOption(pos, unique, choice int) itemBonusOption {
	switch pos {
	case 2:
		switch choice {
		case 0:
			return itemBonusOption{itemBonusHP, 10, 0}
		case 1:
			return itemBonusOption{itemBonusAC, 5, -1}
		}
	case 4, 8:
		switch choice {
		case 0:
			return itemBonusOption{itemBonusMagic, 2, -1}
		case 1:
			return itemBonusOption{itemBonusDamage, 6, -1}
		case 2:
			return itemBonusOption{itemBonusAC, 5, -1}
		}
	case 16:
		switch choice {
		case 0:
			return itemBonusOption{itemBonusMagic, 2, -1}
		case 1:
			return itemBonusOption{itemBonusDamage, 6, -1}
		case 2:
			return itemBonusOption{itemBonusSpecialAll, 3, 0}
		case 3:
			return itemBonusOption{itemBonusResistAll, 3, 0}
		}
	case 32:
		switch choice {
		case 0:
			return itemBonusOption{itemBonusMagic, 2, -1}
		case 1:
			return itemBonusOption{itemBonusSpecialAll, 3, 0}
		}
	case 64, 192:
		if unique == 44 || unique == 47 {
			switch choice {
			case 0:
				return itemBonusOption{itemBonusMagic, 4, -1}
			case 1:
				return itemBonusOption{itemBonusSpecialAll, 3, 1}
			}
			break
		}
		switch choice {
		case 0:
			return itemBonusOption{itemBonusAttSpeed, 3, 1}
		case 1:
			return itemBonusOption{itemBonusDamage, 9, -1}
		case 2:
			return itemBonusOption{itemBonusSpecialAll, 3, 1}
		}
	}
	return itemBonusOption{itemBonusUnique, 0, 0}
}

// itemBonusFirstGrade maps rand()%100 to the stEffect[1] grade.
func itemBonusFirstGrade(lv, roll int) int {
	switch lv {
	case 0:
		switch {
		case roll < 2:
			return 4
		case roll < 6:
			return 3
		case roll < 24:
			return 2
		case roll < 55:
			return 1
		}
		return 0
	case 1:
		switch {
		case roll < 4:
			return 4
		case roll < 24:
			return 3
		case roll < 65:
			return 2
		}
		return 1
	case 2:
		switch {
		case roll < 14:
			return 4
		case roll < 60:
			return 3
		}
		return 2
	case 3:
		switch {
		case roll < 9:
			return 6
		case roll < 50:
			return 5
		case roll < 90:
			return 4
		}
		return 3
	case 4:
		if roll < 25 {
			return 6
		}
		return 5
	}
	return 6
}

// itemBonusSecondGrade maps rand()%100 to the stEffect[2] grade. At level
// difference 0 the native code gates on the first grade roll (stack slot
// ebp-A4h), not on the second roll.
func itemBonusSecondGrade(lv, firstRoll, roll int) int {
	switch lv {
	case 0:
		if firstRoll < 2 {
			return 0
		}
		switch {
		case roll < 6:
			return 3
		case roll < 24:
			return 2
		case roll < 55:
			return 1
		}
		return 0
	case 1:
		switch {
		case roll < 1:
			return 5
		case roll < 5:
			return 4
		case roll < 24:
			return 3
		case roll < 65:
			return 2
		}
		return 1
	case 2:
		switch {
		case roll < 2:
			return 5
		case roll < 16:
			return 4
		case roll < 60:
			return 3
		}
		return 2
	case 3:
		switch {
		case roll < 9:
			return 6
		case roll < 45:
			return 5
		}
		return 4
	case 4:
		switch {
		case roll < 5:
			return 6
		case roll < 55:
			return 5
		}
		return 4
	}
	if roll < 65 {
		return 5
	}
	return 4
}

// itemBonusSancUpgrade raises the +2 refinement by the carry grade value.
var itemBonusSancUpgrade = map[int][]struct{ below, sanc int }{
	3: {{30, 3}},
	4: {{10, 4}, {40, 3}},
	5: {{10, 5}, {30, 4}, {60, 3}},
	6: {{10, 6}, {20, 5}, {40, 4}, {60, 3}},
	7: {{4, 7}, {10, 6}, {20, 5}, {35, 4}, {60, 3}},
}

// setItemBonus rolls the options of a new item instance. reduced is the
// native third argument (quest rewards); armorMagic is the fifth argument.
// The item definition must exist; the caller applies the drop index guard.
func (w *World) setItemBonus(item *model.Item, def model.ItemDef, level int, reduced bool, dropBonus int, armorMagic bool) {
	forcedGrade := 0
	if dropBonus >= 1000 {
		forcedGrade = dropBonus % 1000
		dropBonus = 0
	}
	extra := dropBonus / 8
	if extra < 0 {
		extra = 0
	}
	if extra > 2 {
		extra = 2
	}
	gradeLevel, gradeValue := -1, -1
	if item.Eff[0] >= itemBonusGrade0 && item.Eff[0] <= itemBonusGrade5 {
		gradeLevel = int(item.Eff[0]) - itemBonusGrade0
		gradeValue = int(item.Eff[1])
		item.Eff[0], item.Eff[1] = 0, 0
	}
	if forcedGrade > 100 {
		gradeLevel = forcedGrade - 100
	}
	if !reduced && level >= 210 {
		level -= 47
	}
	pos := def.Pos
	lv := (level - def.ReqLevel) / 25
	if gradeLevel != -1 {
		lv = gradeLevel
	}
	forced := lv >= 4
	if lv < 0 {
		lv = 0
	}
	if gradeLevel <= 4 {
		lv = min(lv, 4)
	} else {
		lv = min(lv, 5)
	}
	if reduced && lv >= 3 {
		lv = 2
	}

	if pos&0xFE != 0 && item.Eff[0] == 0 && pos != 128 {
		w.rollItemBonusOptions(item, def, lv, forced, reduced, extra, gradeValue, armorMagic)
	}

	for _, effect := range def.StaticEffects {
		switch effect.Name {
		case "EF_SANC":
			item.Eff[0], item.Eff[1] = itemBonusSanc, byte(effect.Value)
		case "EF_AMOUNT":
			item.Eff[0], item.Eff[1] = itemBonusAmount, byte(effect.Value)
		case "EF_INCUBATE":
			item.Eff[0], item.Eff[1] = itemBonusIncubate, byte(min(9, effect.Value+w.intn(4)))
		case "EF_WDAY":
			item.Eff[0], item.Eff[1] = itemBonusWDay, byte(effect.Value)
		case "EF_HOUR":
			item.Eff[2], item.Eff[3] = itemBonusHour, byte(effect.Value)
		case "EF_MIN":
			item.Eff[4], item.Eff[5] = itemBonusMin, byte(effect.Value)
		}
	}

	if idx := item.Index; idx >= 447 && idx <= 450 || idx >= 692 && idx <= 695 || idx == 753 {
		for slot := 0; slot < 6; slot += 2 {
			if item.Eff[slot] == 0 {
				item.Eff[slot], item.Eff[slot+1] = itemBonusUnique, byte(w.intn(256))
			}
		}
	}
}

func (w *World) rollItemBonusOptions(item *model.Item, def model.ItemDef, lv int, forced, reduced bool, extra, gradeValue int, armorMagic bool) {
	pos := def.Pos
	choiceRoll := w.intn(101)
	firstChoice := 100
	if reduced {
		firstChoice = choiceRoll % 3
	} else {
		switch lv {
		case 0:
			firstChoice = choiceRoll % (8 - extra)
		case 1, 2:
			firstChoice = choiceRoll % (6 - extra)
		case 3:
			firstChoice = choiceRoll % 4
		}
		if lv >= 4 {
			firstChoice = choiceRoll % 2
		}
	}
	first := itemBonusFirstOption(pos, def.Unique, firstChoice, armorMagic)

	secondRoll := w.intn(100)
	if reduced {
		secondRoll = 2 * secondRoll / 3
	}
	var secondChoice int
	switch {
	case reduced:
		secondChoice = secondRoll % 4
	case lv == 0:
		secondChoice = secondRoll % 8
	case lv == 1 || lv == 2:
		secondChoice = secondRoll % 6
	case lv == 3:
		secondChoice = secondRoll % 4
	default:
		secondChoice = secondRoll % 3
	}
	second := itemBonusSecondOption(pos, def.Unique, secondChoice)

	firstRoll := w.intn(100)
	grade := itemBonusFirstGrade(lv, firstRoll)
	if forced && grade < 4 {
		grade = 4
	}
	if lv >= 3 && isWeaponPos(pos) && (first.effect == itemBonusDamage || first.effect == itemBonusMagic) {
		first.adjust = -2
	}
	value := first.adjust + grade
	if reduced && value == 0 {
		value = 1
	}
	switch {
	case item.Eff[2] == 0 && value > 0:
		item.Eff[2], item.Eff[3] = first.effect, byte(first.step*value)
	case item.Eff[2] == 0 && pos == 32:
		item.Eff[2], item.Eff[3] = first.effect, 0
	case item.Eff[2] == 0:
		item.Eff[2], item.Eff[3] = itemBonusUnique, byte(w.intn(128))
	}

	grade = itemBonusSecondGrade(lv, firstRoll, w.intn(100))
	if forced && grade < 3 {
		grade = 3
	}
	if lv >= 3 && isWeaponPos(pos) && (second.effect == itemBonusDamage || second.effect == itemBonusMagic) {
		second.adjust = -2
	}
	if reduced && grade >= 5 {
		grade = 4
	}
	value = second.adjust + grade
	if extra != 0 && value == 0 {
		value = extra
	}
	if reduced && value == 0 {
		value = 1
	}
	if item.Eff[4] == 0 {
		if value > 0 {
			item.Eff[4], item.Eff[5] = second.effect, byte(second.step*value)
		} else {
			item.Eff[4], item.Eff[5] = itemBonusUnique, byte(w.intn(128))
		}
	}

	if item.Eff[0] != 0 {
		return
	}
	roll := w.intn(100)
	if reduced {
		roll /= 2
	}
	plus2, plus1, plus0, option := 6, 22, 75, 90
	if lv >= 2 {
		plus2, plus1, plus0, option = 6, 35, 85, 100
	}
	switch {
	case roll < plus2:
		item.Eff[0], item.Eff[1] = itemBonusSanc, 2
		if gradeValue > 2 {
			upgrade := w.intn(100)
			for _, step := range itemBonusSancUpgrade[gradeValue] {
				if upgrade < step.below {
					item.Eff[1] = byte(step.sanc)
					break
				}
			}
		}
	case roll < plus1:
		item.Eff[0], item.Eff[1] = itemBonusSanc, 1
	case roll < plus0:
		item.Eff[0], item.Eff[1] = itemBonusSanc, 0
	case roll < option:
		// Level difference 5 reads past the native 0x63D120 table into
		// unrelated data; no option is written (see the research record).
		if lv < len(itemBonusRanges) {
			pick := w.intn(10)
			bounds := itemBonusRanges[lv][pick]
			item.Eff[0] = itemBonusTypes[pick]
			item.Eff[1] = byte(bounds[0] + w.intn(bounds[1]+1-bounds[0]))
		}
	default:
		item.Eff[0], item.Eff[1] = itemBonusUnique, byte(w.intn(128))
	}
}

// applyDropItemBonus applies SetItemBonus to a carry drop copy, as MobKilled
// does before PutItem.
func (w *World) applyDropItemBonus(item model.Item, mobLevel, dropBonus int) model.Item {
	if item.Index == 0 || item.Index >= itemBonusMaxIndex {
		return item
	}
	def, ok := w.items[item.Index]
	if !ok {
		return item
	}
	w.setItemBonus(&item, def, mobLevel, false, dropBonus, false)
	return item
}
