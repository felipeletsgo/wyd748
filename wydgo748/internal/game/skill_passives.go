package game

import "wydgo/internal/model"

func learnedLocal(ch *model.Char, local int) bool {
	return ch != nil && local >= 0 && local < 24 && ch.LearnedSkill&(uint32(1)<<local) != 0
}

// W2PP _MSG_Sell: o bit local 9 da HT melhora a venda ao mercador.
// Nao confundir LearnedSkill com Rsv (flags de affects usadas na precisao).
// Aplica somente o bonus da passiva; a politica de preco-base e imposto
// continua pertencendo ao handler de venda.
func passiveMerchantSalePrice(ch *model.Char, item uint16, price uint64) uint64 {
	if ch == nil || ch.Class != 3 || !learnedLocal(ch, 81-72) || price <= 1000 ||
		item == 693 || item == 694 || item == 695 {
		return price
	}
	bonus := minInt(int(playerMastery(ch, 2))/10+6, 26) / 2
	return price * uint64(50+bonus) / 50
}

func (w *World) rightHandShield(ch *model.Char) (model.Item, model.ItemDef, bool) {
	if ch == nil {
		return model.Item{}, model.ItemDef{}, false
	}
	item := ch.Equip[7]
	def, ok := w.items[item.Index]
	if item.Index == 0 || !ok || def.Pos != 128 {
		return model.Item{}, model.ItemDef{}, false
	}
	return item, def, true
}

// applyPassiveSkills concentra efeitos permanentes que o WYD 7.48 aplica durante
// BASE_GetCurrentScore/BASE_GetMobAbility. Eles nunca dependem de pacote do
// client e sao reconstruidos do zero em todo recalc.
func (w *World) applyPassiveSkills(ch *model.Char) {
	if ch == nil {
		return
	}
	e := effectiveScore(ch)
	if e == nil {
		return
	}
	switch ch.Class {
	case 0: // TransKnight; official TMSRVIN sub_4CE1C0, same bits in 7.48.
		// Destiny (7) adds 20 holy resistance.
		if learnedLocal(ch, 7) {
			e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+20, 0, 100))
		}
		// Critical Armor (15): defense 120%, attack + defense/10 and
		// critical + 100, in tenths of a percent.
		if learnedLocal(ch, 15) {
			e.Defense = extendedValue(int64(e.Defense) * 120 / 100)
			e.Attack = extendedValue(int64(e.Attack) + int64(e.Defense)/10)
			e.Critical = addCritical(e.Critical, 100)
		}
	case 2: // BeastMaster
		// Official TMSRVIN sub_4CE1C0: Bear's Roar (67) with a shield in slot 7
		// raises defense to 105%.
		if learnedLocal(ch, 67-48) {
			if _, _, ok := w.rightHandShield(ch); ok {
				e.Defense = extendedValue(int64(e.Defense) * 105 / 100)
			}
		}
	case 3: // Huntress; official TMSRVIN bits mapped to 7.48 skills by name.
		// Two Weapon Fighting (server bit 2, sub_4BB4C0): a slot-6 weapon of
		// Unique 42 or 43 adds Special[1]/5 + 10, or + 320 with Rapid Hit.
		if learnedLocal(ch, 82-72) && ch.Equip[6].Index != 0 {
			if def, ok := w.items[ch.Equip[6].Index]; ok && (def.Unique == 42 || def.Unique == 43) {
				bonus := int(playerMastery(ch, 1))/5 + 10
				if learnedLocal(ch, 95-72) {
					bonus = int(playerMastery(ch, 1))/5 + 320
				}
				e.Attack = extendedValue(int64(e.Attack) + int64(bonus))
			}
		}
		// Hunter's Vision (server bit 18, sub_4CE1C0): base DEX/20, in tenths
		// of a percent.
		if learnedLocal(ch, 91-72) && ch.Score != nil {
			e.Critical = addCritical(e.Critical, int(ch.Score.Dex)/20)
		}
		// Improved Range (server bit 19, sub_4BA690): range at least 2.
		if learnedLocal(ch, 92-72) {
			e.Range = uint32(maxInt(2, int(e.Range)))
		}
	}
}
