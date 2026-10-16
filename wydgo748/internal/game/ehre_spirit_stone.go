package game

import "wydgo/internal/model"

// W2PP GetMatchCombineEhre recipe 3 and Exec_MSG_CombineItemEhre define
// the ingredients and additions. The configured server chance is 50%.
const ehreSpiritStoneChance = 50

func ehreSpiritStone(req combineRequest) (model.Item, bool) {
	stone := req.Items[2]
	if stone.Index != 633 || itemSanc(stone) < 9 || !exactRecipeTailEmpty(req, 3) {
		return model.Item{}, false
	}
	var values [3]int
	for i := 0; i < len(stone.Eff); i += 2 {
		if effect := stone.Eff[i]; effect >= 69 && effect <= 71 {
			values[effect-69] += int(stone.Eff[i+1])
		}
	}
	hp, mp, critical := values[0], values[1], values[2]
	if hp >= 20 || mp >= 20 || critical >= 100 ||
		hp >= 10 && mp >= 10 || hp >= 10 && critical >= 50 || mp >= 10 && critical >= 50 {
		return model.Item{}, false
	}
	for _, ankh := range req.Items[:2] {
		var effect, amount, limit byte
		switch ankh.Index {
		case 661:
			effect, amount, limit = 70, 2, 20
		case 662:
			effect, amount, limit = 69, 2, 20
		case 663:
			effect, amount, limit = 71, 10, 100
		default:
			return model.Item{}, false
		}
		pos := -1
		for _, i := range []int{2, 4} {
			if stone.Eff[i] == effect {
				pos = i
				break
			}
		}
		if pos < 0 {
			for _, i := range []int{2, 4} {
				if stone.Eff[i] == 0 {
					pos = i
					break
				}
			}
		}
		// Reject an unrepresentable third addition before charging ingredients.
		if pos < 0 {
			return model.Item{}, false
		}
		value := int(stone.Eff[pos+1]) + int(amount)
		if stone.Eff[pos] == 0 {
			value = int(amount)
		}
		stone.Eff[pos], stone.Eff[pos+1] = effect, byte(min(value, int(limit)))
	}
	if !setItemSanc(&stone, 7) {
		return model.Item{}, false
	}
	return stone, true
}
