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
	case 0: // TransKnight
		if learnedLocal(ch, 15) { // Armadura Critica
			e.Defense = extendedValue(int64(e.Defense)*110/100 + 50)
			e.Critical = clampScoreValue(e.Critical + 24)
		}
	case 2: // BeastMaster
		if learnedLocal(ch, 67-48) { // Escudo do Tormento
			if item, def, ok := w.rightHandShield(ch); ok {
				bonus := (itemAbility(item, def, "EF_AC") + 1) / 7
				e.Defense = extendedValue(int64(e.Defense) + int64(bonus))
			}
		}
	case 3: // Huntress
		if learnedLocal(ch, 90-72) { // Visao de Cacadora
			critical := (int(playerMastery(ch, 3))+1)/10 + int(e.Dex)/75
			e.Critical = clampScoreValue(e.Critical + uint32(maxInt(4, critical)))
		}
		if learnedLocal(ch, 92-72) { // Toxina de Serpente
			e.Range = uint32(maxInt(2, int(e.Range)))
		}
	}
}
