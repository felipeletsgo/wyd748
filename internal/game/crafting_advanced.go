package game

import (
	"math/rand"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

var soulRecipes = [10][3]uint16{
	{2442, 2443, 2444}, {2442, 2441, 2443}, {2444, 2441, 2443},
	{2443, 2442, 2444}, {2441, 2441, 2441}, {2442, 2442, 2442},
	{2443, 2443, 2443}, {2444, 2444, 2444}, {2441, 2442, 2443},
	{2441, 2443, 2444},
}

var secretStoneRecipes = [4][8]uint16{
	{5126, 5122, 5121, 5114, 5125, 5111, 5118, 5334},
	{5131, 5113, 5115, 5116, 5125, 5112, 5114, 5335},
	{5110, 5124, 5117, 5129, 5114, 5125, 5128, 5336},
	{5122, 5119, 5132, 5120, 5130, 5133, 5123, 5337},
}

func advancedEvolution(ch *model.Char, values ...string) bool {
	if ch == nil {
		return false
	}
	for _, value := range values {
		if strings.EqualFold(ch.Evolution, value) {
			return true
		}
	}
	return false
}

func exactRecipe(req combineRequest, recipe []uint16) bool {
	for i := 0; i < combineSlots; i++ {
		expected := uint16(0)
		if i < len(recipe) {
			expected = recipe[i]
		}
		if req.Items[i].Index != expected {
			return false
		}
	}
	return true
}

func (w *World) putCraftResult(p *Player, preferred int, item model.Item, changed map[int]struct{}) bool {
	if _, exists := w.items[item.Index]; !exists {
		return false
	}
	item, err := materializeItem(item)
	if err != nil {
		return false
	}
	if preferred >= 0 && preferred < model.PlayerCarrySlots && p.Char.Inv[preferred].Index == 0 {
		p.Char.Inv[preferred] = item
		changed[preferred] = struct{}{}
		return true
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if p.Char.Inv[i].Index == 0 {
			p.Char.Inv[i] = item
			changed[i] = struct{}{}
			return true
		}
	}
	return false
}

func (w *World) onCombineEhre(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Ehre")
	if !ok {
		return
	}
	for index, recipe := range soulRecipes {
		if !exactRecipe(req, recipe[:]) {
			continue
		}
		oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
		oldSoul := p.Char.SoulInfo
		changed := make(map[int]struct{}, 3)
		consumeCombineItems(p.Char, req, 0, 2, changed)
		p.Char.SoulInfo = uint8(index + 1)
		w.commitCombineWithRollback(p, oldInv, oldEquip, oldGold, changed, nil, 1,
			func() { p.Char.SoulInfo = oldSoul })
		return
	}

	// Refinação Abençoada purificada, conforme Secrets 7.54.
	if req.Items[0].Index == 697 && req.Items[1].Index == 697 && req.Items[2].Index == 3338 &&
		req.Items[3].Index == 0 && req.Items[4].Index == 0 && req.Items[5].Index == 0 &&
		req.Items[6].Index == 0 && req.Items[7].Index == 0 && itemSanc(req.Items[2]) < 9 &&
		advancedEvolution(p.Char, "celestial", "subcelestial") && playerLevel(p.Char) >= 39 && p.Char.Exp >= 5_000_000 {
		level := int(playerLevel(p.Char))
		chance := 0
		switch {
		case level <= 149:
			chance = 30
		case level <= 159:
			chance = 35
		case level <= 169:
			chance = 40
		case level <= 179:
			chance = 50
		case level <= 189:
			chance = 70
		default:
			chance = 100
		}
		targetPos := int(req.Pos[2])
		success := rand.Intn(100) < chance
		refinedTarget := p.Char.Inv[targetPos]
		if success {
			if !setItemSanc(&refinedTarget, itemSanc(refinedTarget)+1) {
				w.sendCombineResult(p, 0)
				return
			}
		}
		oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
		oldExp := p.Char.Exp
		changed := make(map[int]struct{}, 3)
		consumeCombineItems(p.Char, req, 0, 1, changed)
		if success {
			p.Char.Inv[targetPos] = refinedTarget
		} else {
			p.Char.Inv[targetPos] = model.Item{}
		}
		changed[targetPos] = struct{}{}
		p.Char.Exp -= 5_000_000
		code := uint32(2)
		if success {
			code = 1
		}
		w.commitCombineWithRollback(p, oldInv, oldEquip, oldGold, changed, nil, code,
			func() { p.Char.Exp = oldExp })
		return
	}
	w.sendCombineResult(p, 0)
}

func odinTargetBlocked(index uint16) bool {
	if index == 540 || index == 541 || index == 633 || index == 1738 || index >= 551 && index <= 570 ||
		index >= 591 && index <= 595 || index >= 661 && index <= 663 {
		return true
	}
	return false
}

func odinRefineBonus(level int) int {
	switch level {
	case 11:
		return 5
	case 12:
		return 3
	case 13:
		return 2
	case 14:
		return 1
	default:
		return 0
	}
}

func (w *World) deterministicOdin(p *Player, req combineRequest, output uint16) {
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, 8)
	preferred := int(req.Pos[0])
	consumeCombineItems(p.Char, req, 0, 6, changed)
	if !w.putCraftResult(p, preferred, model.Item{Index: output}, changed) {
		p.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold
		w.sendCombineResult(p, 0)
		return
	}
	w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, 1)
}

func (w *World) onCombineOdin(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Odin")
	if !ok {
		return
	}
	for _, recipe := range secretStoneRecipes {
		if exactRecipe(req, recipe[:7]) {
			w.deterministicOdin(p, req, recipe[7])
			return
		}
	}
	fury := []uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}
	if exactRecipe(req, fury) {
		w.deterministicOdin(p, req, 3020)
		return
	}
	clue := []uint16{413, 413, 413, 413, 413, 413, 413}
	if exactRecipe(req, clue) {
		w.deterministicOdin(p, req, 5134)
		return
	}
	crystal := []uint16{421, 422, 423, 424, 425, 426, 427}
	if exactRecipe(req, crystal) {
		w.deterministicOdin(p, req, 4032)
		return
	}

	capeRecipe := []uint16{4127, 4127, 5135, 413, 413, 413, 413}
	if exactRecipe(req, capeRecipe) && p.Char.Equip[15].Index >= 3197 && p.Char.Equip[15].Index <= 3199 &&
		advancedEvolution(p.Char, "celestial", "subcelestial") && itemSanc(p.Char.Equip[15]) <= 9 {
		refinedCape := p.Char.Equip[15]
		if !setItemSanc(&refinedCape, itemSanc(refinedCape)+1) {
			w.sendCombineResult(p, 0)
			return
		}
		oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
		changedInv, changedEquip := make(map[int]struct{}, 7), map[int]struct{}{15: {}}
		consumeCombineItems(p.Char, req, 0, 6, changedInv)
		p.Char.Equip[15] = refinedCape
		w.recalcPlayer(p.Char)
		w.commitCombine(p, oldInv, oldEquip, oldGold, changedInv, changedEquip, 1)
		return
	}

	unlock := []uint16{5113, 5129, 5112, 5110, 4127, 4127, 5135}
	if exactRecipe(req, unlock) && advancedEvolution(p.Char, "celestial") && playerLevel(p.Char) == 39 &&
		!p.Char.CelestialLevel40Unlocked && counterBalance(p, fameCounter) >= 200 {
		oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
		oldFame := copyCounters(p)
		changed := make(map[int]struct{}, 7)
		consumeCombineItems(p.Char, req, 0, 6, changed)
		spendCounters(p, map[string]uint32{fameCounter: 200})
		p.Char.CelestialLevel40Unlocked = true
		if !w.commitCombineWithPlayerState(p, oldInv, oldEquip, oldGold, changed, nil, 1) {
			p.SpecialCoins = oldFame
			p.Char.CelestialLevel40Unlocked = false
			w.recalcPlayer(p.Char)
		}
		return
	}

	if req.Items[0].Index == 4043 && req.Items[1].Index == 4043 && req.Items[2].Index != 0 &&
		req.Items[7].Index == 0 && !odinTargetBlocked(req.Items[2].Index) {
		validCatalysts, chance := true, 0
		for i := 3; i <= 6; i++ {
			if req.Items[i].Index != 0 && req.Items[i].Index != 3338 {
				validCatalysts = false
				break
			}
			// Slot vazio e permitido, mas nao e catalisador e portanto nao
			// concede os +2 pontos base da Refinacao AbenÃ§oada.
			if req.Items[i].Index == 3338 {
				chance += itemSanc(req.Items[i]) + 2
			}
		}
		level := itemSanc(req.Items[2])
		chance += odinRefineBonus(level)
		if validCatalysts && level >= 11 && level <= 14 {
			targetPos := int(req.Pos[2])
			success := rand.Intn(100) < clampInt(chance, 0, 100)
			newLevel := level - 1
			if success {
				newLevel = level + 1
			}
			refinedTarget := p.Char.Inv[targetPos]
			if !setItemSanc(&refinedTarget, newLevel) {
				w.sendCombineResult(p, 0)
				return
			}
			oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
			changed := make(map[int]struct{}, 7)
			consumeCombineItems(p.Char, req, 0, 1, changed)
			consumeCombineItems(p.Char, req, 3, 6, changed)
			p.Char.Inv[targetPos] = refinedTarget
			changed[targetPos] = struct{}{}
			code := uint32(2)
			if success {
				code = 1
			}
			w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
			return
		}
	}
	w.sendCombineResult(p, 0)
}
