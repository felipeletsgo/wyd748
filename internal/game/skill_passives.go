package game

import "wydgo/internal/model"

func learnedLocal(ch *model.Char, local int) bool {
	return ch != nil && local >= 0 && local < 24 && ch.LearnedSkill&(uint32(1)<<local) != 0
}

// applyPassiveSkills concentra efeitos permanentes que a W2PP aplica durante
// BASE_GetCurrentScore/BASE_GetMobAbility. Eles nunca dependem de pacote do
// client e sao reconstruidos do zero em todo recalc.
func (w *World) applyPassiveSkills(ch *model.Char) {
	if ch == nil {
		return
	}
	e := effectiveExtended(ch)
	if e == nil {
		return
	}
	switch ch.Class {
	case 0: // TransKnight
		if learnedLocal(ch, 15) { // Armadura Critica
			e.Defense = extendedValue(int64(e.Defense)*110/100 + 50)
			e.Critical = clampExtended(e.Critical + 24)
		}
	case 2: // BeastMaster
		if learnedLocal(ch, 65-48) { // Armadura Elemental
			bonus := int(playerMastery(ch, 2)) / 6
			e.ResistFire = uint32(clampInt(int(e.ResistFire)+bonus, 0, 100))
			e.ResistIce = uint32(clampInt(int(e.ResistIce)+bonus, 0, 100))
			e.ResistHoly = uint32(clampInt(int(e.ResistHoly)+bonus, 0, 100))
			e.ResistThunder = uint32(clampInt(int(e.ResistThunder)+bonus, 0, 100))
		}
		if learnedLocal(ch, 67-48) { // Escudo do Tormento
			e.Defense = extendedValue(int64(e.Defense) * 105 / 100)
		}
	case 3: // Huntress
		if learnedLocal(ch, 82-72) { // Agressividade
			e.Attack = clampExtended(e.Attack + playerMastery(ch, 1) + 10)
		}
		if learnedLocal(ch, 91-72) { // Visao de Cacadora
			critical := (int(playerMastery(ch, 3))+1)/10 + int(e.Dex)/75
			e.Critical = clampExtended(e.Critical + uint32(maxInt(4, critical)))
		}
		if learnedLocal(ch, 92-72) { // Olhos de Aguia
			e.Range = uint32(maxInt(2, int(e.Range)))
		}
	}
}
