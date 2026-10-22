package model

// DropRateForSlot applies the historical MobKilled.cpp level-band adjustments
// (MobKilled.cpp:2359+) to a monster carry slot's base rate. A kill drops the
// slot when guaranteed is true, when rate is zero, or when rand()%rate == 0;
// the killer's drop bonus is applied afterwards by the World.
func DropRateForSlot(base [MaxCarry]int, slot int, mobLevel int) (rate int, guaranteed bool) {
	rate = base[slot]
	if slot < 60 {
		if pos := slot / 8; pos <= 2 {
			switch {
			case mobLevel < 10:
				rate = 4 * rate / 100
			case mobLevel < 20:
				rate = 5 * rate / 100
			case mobLevel < 30:
				rate = 6 * rate / 100
			case mobLevel < 40:
				rate = 7 * rate / 100
			case mobLevel < 60:
				rate = 8 * rate / 100
			default:
				rate = 99 * rate / 100
			}
		}
	} else {
		switch {
		case mobLevel < 170:
			rate = 90 * rate / 100
		case mobLevel < 200:
			rate = 60 * rate / 100
		case mobLevel < 230:
			rate = 50 * rate / 100
		case mobLevel < 255:
			rate = 43 * rate / 100
		case mobLevel < 320:
			rate = 38 * rate / 100
		default:
			rate = 50 * rate / 100
		}
	}
	// Special slots: 8-10 force rate 4; slot 11 guarantees the drop.
	if slot >= 8 && slot <= 10 {
		rate = 4
	}
	if slot == 11 {
		return 0, true
	}
	if rate > 32000 {
		rate = 32000
	}
	return rate, false
}
