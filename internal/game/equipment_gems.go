package game

import "wydgo/internal/model"

// equipmentGemBonus e o estado derivado das Gemas. Nao e persistido nem vem
// do client: a cada uso ele e recomposto dos itens equipados e do itemlist.
type equipmentGemBonus struct {
	dropPercent  int
	expPercent   int
	forceDamage  int
	absorbDamage int
}

// equipmentGemBonuses porta GetCurrentScore/GetCurScore das fontes 7.54/7.59.
// Armas Ancient codificam a gema em Grade 5..8 mesmo abaixo de +10. No refino
// especial 230..253, a variante fica nos dois bits inferiores e +10..+15
// define a escala de perfuracao/absorcao.
func (w *World) equipmentGemBonuses(ch *model.Char) equipmentGemBonus {
	var bonus equipmentGemBonus
	if ch == nil {
		return bonus
	}
	for slot := 1; slot < len(ch.Equip); slot++ {
		item := ch.Equip[slot]
		def, ok := w.items[item.Index]
		if item.Index == 0 || !ok {
			continue
		}
		switch def.Grade {
		case 5: // Diamond Ancient
			bonus.dropPercent += 8
		case 6: // Emerald Ancient
			bonus.forceDamage += 20
		case 7: // Coral Ancient
			bonus.expPercent += 2
		case 8: // Garnet Ancient
			bonus.absorbDamage += 20
		}

		raw, exists := sancRaw(item)
		if !exists || raw < 230 || raw > 253 {
			continue
		}
		variant := (int(raw) - 230) % 4
		scale := (int(raw)-230)/4 + 1 // +10=1 ... +15=6
		switch variant {
		case 0: // Diamond
			bonus.dropPercent += 8
		case 1: // Emerald
			perLevel := 40
			if def.Grade == 6 {
				perLevel = 80
			}
			bonus.forceDamage += perLevel * scale
		case 2: // Coral
			bonus.expPercent += 2
		case 3: // Garnet
			perLevel := 40
			if def.Grade == 8 {
				perLevel = 80
			}
			bonus.absorbDamage += perLevel * scale
		}
	}
	return bonus
}

func addFlatDamage(damage uint32, bonus int) uint32 {
	if damage == 0 || bonus <= 0 {
		return damage
	}
	if uint64(damage)+uint64(bonus) > uint64(maxExtendedStat) {
		return maxExtendedStat
	}
	return damage + uint32(bonus)
}

func absorbFlatDamage(damage uint32, bonus int) uint32 {
	if damage == 0 || bonus <= 0 {
		return damage
	}
	if uint32(bonus) >= damage {
		return 1 // o nativo conserva dano minimo depois da absorcao.
	}
	return damage - uint32(bonus)
}

func applyPercentReward(reward uint32, percent int) uint32 {
	if reward == 0 || percent <= 0 {
		return reward
	}
	result := uint64(reward) + uint64(reward)*uint64(percent)/100
	if result > uint64(^uint32(0)) {
		return ^uint32(0)
	}
	return uint32(result)
}
