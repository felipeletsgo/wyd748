package game

import "wydgo/internal/model"

// Secondary-source adaptation, not a claim of native 7.48 lottery parity.
// TMSRVIN 7.59: sub_4E3D50/4160/46C0/4AA0 and HellGateRate_*.txt.
// Keep the rate files' zero outcomes. Normalizing partial weight tables would
// change the historical lottery; each caller applies the recovered clamps.
var hell759Weapons = [2][10]uint16{
	{869, 910, 809, 935, 839, 824, 884, 854, 901, 899},
	{870, 911, 810, 936, 840, 825, 885, 855, 902, 900},
}

var hell759Armor = [2][20]uint16{
	{1192, 1327, 1477, 1627, 1195, 1330, 1480, 1630, 1198, 1333, 1483, 1633,
		1201, 1336, 1486, 1636, 1204, 1339, 1489, 1639},
	{1207, 1342, 1492, 1642, 1210, 1345, 1495, 1645, 1213, 1348, 1498, 1648,
		1216, 1351, 1501, 1651, 1219, 1354, 1504, 1654},
}

var hell759ArmorFirst = [15][3]int{
	{3000, 2000, 500}, {3000, 2000, 500}, {3000, 2000, 200},
	{3000, 2000, 100}, {}, {}, {3000, 2000, 100}, {}, {},
	{3000, 2000, 200}, {}, {}, {4000, 1500, 200}, {}, {},
}

var hell759ArmorSecond = [15][3]int{
	{2500, 1000, 400}, {2500, 1000, 400}, {2500, 1000, 200},
	{}, {3000, 1000, 300}, {3000, 1000, 300},
	{}, {3000, 1000, 300}, {3000, 1000, 300},
	{}, {3000, 1000, 300}, {3000, 1000, 300},
	{3000, 2000, 200}, {3000, 1500, 200}, {3000, 1500, 400},
}

var hell759AccessoryFirst = [12][4]int{
	{3000, 2000, 1000, 300}, {3000, 2000, 1000, 300}, {3000, 2000, 1000, 300},
	{3500, 2000, 1000, 0}, {3500, 2000, 1000, 200}, {3500, 2000, 1000, 200},
	{3500, 2000, 1000, 0}, {3500, 2000, 1000, 200}, {3500, 2000, 1000, 200},
	{3000, 2000, 1000, 300}, {3000, 2000, 1000, 300}, {3000, 2000, 1000, 300},
}

var hell759AccessorySecond = [12][4]int{
	{2000, 1000, 500, 0}, {2000, 1000, 500, 200}, {2000, 1000, 500, 200},
	{2000, 1000, 500, 0}, {2000, 1000, 500, 0}, {2000, 1000, 500, 200},
	{2000, 1000, 500, 0}, {2000, 1000, 500, 0}, {2000, 1000, 500, 200},
	{2500, 1000, 500, 0}, {2500, 1000, 500, 200}, {2500, 1000, 500, 200},
}

// sub_4E4B60 places a possibly partial table at a random offset within 10000
// slots. A miss returns zero; each caller applies its own fallback/clamp.
func (w *World) hell759Pick(weights ...int) int {
	total := 0
	for _, weight := range weights {
		total += weight
	}
	offset := 0
	if total < 10000 {
		offset = w.intn(10000 - total)
	}
	roll := w.intn(10000)
	for index, weight := range weights {
		if roll >= offset && roll < offset+weight {
			return index + 1
		}
		offset += weight
	}
	return 0
}

func (w *World) rollDionysStoreItem(category int) (model.Item, bool) {
	if category == hellStoreRandom {
		category = max(1, w.hell759Pick(3300, 3300, 3400))
	}
	var item model.Item
	switch category {
	case hellStoreWeapon:
		item = w.hell759Weapon()
	case hellStoreArmor:
		item = w.hell759ArmorItem()
	case hellStoreAmulet:
		item = w.hell759Accessory()
	default:
		return item, false
	}
	_, exists := w.items[item.Index]
	return item, exists
}

func (w *World) hell759Weapon() model.Item {
	tier := 0
	if w.hell759Pick(5500, 4500) > 1 {
		tier = 1
	}
	kind := w.hell759Pick(800, 1000, 900, 1000, 900)
	if kind != 0 {
		kind--
	} else if next := w.hell759Pick(900, 900, 1200, 1200, 1200); next != 0 {
		kind = next + 4
	}
	first := max(1, w.hell759Pick(7000, 1500, 1500)) - 1
	weights := [3]int{3000, 1500, 500}
	if first == 1 {
		weights = [3]int{3000, 2000, 500}
	} else if first == 2 {
		weights = [3]int{3500, 2000, 1000}
	} else if kind == 0 {
		weights = [3]int{1500, 3000, 500}
	}
	grade1 := max(2, w.hell759Pick(weights[:]...))
	second := max(1, w.hell759Pick(3500, 3300, 3200)) - 1
	weights = [3]int{2500, 1000, 500}
	if first == 1 {
		weights = [3]int{2500, 1500, 500}
	} else if first == 2 {
		weights = [3]int{3000, 1500, 500}
	}
	// Historical behavior: the second grade uses the FIRST option's row.
	grade2 := max(1, w.hell759Pick(weights[:]...))
	option := func(choice, grade int) (byte, byte) {
		switch choice {
		case 1:
			return 4, byte(10*grade + 20)
		case 2:
			return 74, byte(3*grade + 9)
		default:
			if kind >= 7 {
				return 60, byte(4*grade + 4)
			}
			return 2, byte(9*grade + 9)
		}
	}
	e1, v1 := option(first, grade1)
	e2, v2 := option(second, grade2)
	return model.Item{Index: hell759Weapons[tier][kind], Eff: [6]byte{43, 0, e1, v1, e2, v2}}
}

func (w *World) hell759ArmorItem() model.Item {
	tier := 0
	if w.hell759Pick(4500, 5500) > 1 {
		tier = 1
	}
	part := max(1, w.hell759Pick(1800, 1800, 1800, 2400, 2200)) - 1
	class := max(1, w.hell759Pick(2800, 2800, 2200, 2200)) - 1
	kind1, effect1 := 0, byte(71)
	switch part {
	case 0:
		kind1 = max(1, w.hell759Pick(3000, 3500, 3500)) - 1
		effect1 = [3]byte{4, 3, 60}[kind1]
	case 3:
		effect1 = 72
	case 4:
		effect1 = 73
	}
	row := hell759ArmorFirst[part*3+kind1]
	grade1 := w.hell759Pick(row[:]...)
	choice := w.hell759Pick(3000, 3500, 3500)
	kind2, effect2 := 2, byte(60)
	switch part {
	case 0:
		switch choice {
		case 0:
			kind2, effect2 = 0, 4
		case 1:
			kind2, effect2 = 1, 3
		}
	case 4:
		kind2 = 2
		if choice == 1 {
			kind2 = 0
		} else if choice == 2 {
			kind2 = 1
		}
		effect2 = [3]byte{73, 60, 74}[kind2]
	default:
		if choice == 2 {
			kind2, effect2 = 1, 2
		}
	}
	row = hell759ArmorSecond[part*3+kind2]
	grade2 := max(2, w.hell759Pick(row[:]...))
	value := func(effect byte, grade int) byte {
		switch effect {
		case 2:
			return byte(6*grade + 12)
		case 60:
			return byte(2*grade + 4)
		case 4:
			return byte(10*grade + 20)
		case 72:
			return byte(10*grade + 10)
		case 71:
			return byte(10*grade + 40)
		case 3:
			return byte(5*grade + 10)
		case 74:
			return byte(3*grade + 9)
		default:
			return byte(6*grade + 12)
		}
	}
	return model.Item{Index: hell759Armor[tier][part*4+class],
		Eff: [6]byte{43, 0, effect1, value(effect1, grade1), effect2, value(effect2, grade2)}}
}

func (w *World) hell759Accessory() model.Item {
	choice := w.hell759Pick(100, 100, 4000, 3000, 2800)
	if choice == 1 || choice == 2 {
		return model.Item{Index: uint16(539 + choice)}
	}
	tier := max(0, choice-3)
	mastery := max(1, w.hell759Pick(2000, 2500, 2500, 3000)) - 1
	option := func(weights []int, rates [12][4]int) (byte, byte) {
		kind := max(1, w.hell759Pick(weights...)) - 1
		row := rates[kind*3+tier]
		grade := w.hell759Pick(row[:]...)
		effect := [4]byte{4, 2, 60, 26}[kind]
		value := [4]int{10*grade + 10, 5*grade + 5, grade + 4, 2*grade + 7}[kind]
		return effect, byte(value)
	}
	e1, v1 := option([]int{2200, 2800, 2800, 2200}, hell759AccessoryFirst)
	e2, v2 := option([]int{2400, 2600, 2600, 2400}, hell759AccessorySecond)
	return model.Item{Index: uint16(551 + tier*4 + mastery), Eff: [6]byte{43, 0, e1, v1, e2, v2}}
}
