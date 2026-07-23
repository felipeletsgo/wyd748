package game

import (
	"log"
	"math/rand"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const aylinGoldCost = 50_000_000

// agathaBaseChance porta g_pAgathaBase (Basedef.cpp:156 na fonte W2PP, default
// 20, configuravel em arquivo texto no W2PP -- aqui fixo, como os demais
// custos de craft do arquivo). A chance real e base + grade*5 + bonus de
// nivel (GetMatchCombineAgatha, GetFunc.cpp:564-628), tipicamente 30-50%,
// nunca garantida.
const agathaBaseChance = 20

func (w *World) beginCombine(s *net.Session, pkt []byte, npc string) (*Player, combineRequest, bool) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
		return nil, combineRequest{}, false
	}
	now := time.Now()
	if !p.LastCraft.IsZero() && now.Sub(p.LastCraft) < 800*time.Millisecond {
		s.Send(wire.MessagePanel("Aguarde um segundo para tentar novamente."))
		return p, combineRequest{}, false
	}
	p.LastCraft = now
	if _, err := w.combineNPC(p, npc); err != nil {
		w.sendCombineResult(p, 0)
		return p, combineRequest{}, false
	}
	req, err := parseCombineRequest(pkt, p.Char)
	if err != nil {
		w.sendCombineResult(p, 0)
		return p, combineRequest{}, false
	}
	return p, req, true
}

func setItemSancRaw(item *model.Item, raw byte) bool {
	if item == nil {
		return false
	}
	for i := 0; i < 3; i++ {
		if effect := item.Eff[i*2]; effect == 43 || effect >= 116 && effect <= 125 {
			item.Eff[i*2+1] = raw
			return true
		}
	}
	return false
}

func (w *World) commitCombine(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{}, equipSlots map[int]struct{}, result uint32) bool {
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold
		for pos := range invSlots {
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
		}
		for pos := range equipSlots {
			p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.sendCombineResult(p, 0)
		return false
	}
	for pos := range invSlots {
		p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
	}
	for pos := range equipSlots {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
	}
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.sendCombineResult(p, result)
	return true
}

func consumeCombineItems(ch *model.Char, req combineRequest, from, to int, changed map[int]struct{}) {
	for i := from; i <= to && i < combineSlots; i++ {
		if req.Items[i].Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		ch.Inv[pos] = model.Item{}
		changed[pos] = struct{}{}
	}
}

func (w *World) onCombineCompositor(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Compositor")
	if !ok {
		return
	}
	target, jewel := req.Items[0], req.Items[1]
	targetDef, targetOK := w.items[target.Index]
	if !targetOK || targetDef.Unique < 41 || targetDef.Unique > 49 || targetDef.Extra <= 0 ||
		itemAbility(target, targetDef, "EF_MOBTYPE") == 3 || itemAbility(target, targetDef, "EF_ITEMLEVEL") <= 3 ||
		jewel.Index < 2441 || jewel.Index > 2444 || itemStackAmount(jewel) > 1 {
		w.sendCombineResult(p, 0)
		return
	}
	chance, materials := 0, 0
	for i := 2; i < combineSlots; i++ {
		item := req.Items[i]
		if item.Index == 0 {
			continue
		}
		def, exists := w.items[item.Index]
		level, sanc := itemAbility(item, def, "EF_ITEMLEVEL"), itemSanc(item)
		if !exists || def.Pos == 0 || level < 4 || level > 5 || def.Grade < 1 || def.Grade > 4 || sanc < 7 || sanc > 9 {
			w.sendCombineResult(p, 0)
			return
		}
		base := [2][4][3]int{
			{{4, 5, 6}, {5, 6, 7}, {6, 7, 8}, {8, 9, 10}},
			{{6, 7, 8}, {7, 8, 9}, {8, 9, 10}, {10, 11, 12}},
		}
		chance += base[level-4][def.Grade-1][sanc-7]
		materials++
	}
	resultIndex := uint16(targetDef.Extra + int(jewel.Index-2441))
	_, resultExists := w.items[resultIndex]
	if materials == 0 || !resultExists {
		w.sendCombineResult(p, 0)
		return
	}
	// O compositor transforma a arma-base: os efeitos adicionais pertencem ao
	// item do jogador e precisam sobreviver Ã troca do Ã­ndice.
	resultItem := target
	resultItem.Index = resultIndex
	if !setItemSanc(&resultItem, 7) {
		w.sendCombineResult(p, 0)
		return
	}
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 0, 7, changed)
	success := combineRoll() <= clampInt(chance, 0, 100)
	if success {
		pos := int(req.Pos[0])
		p.Char.Inv[pos] = resultItem
		changed[pos] = struct{}{}
	}
	code := uint32(2)
	if success {
		code = 1
	}
	if w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code) {
		log.Printf("[#%d] CRAFT Compositor sucesso=%t chance=%d", s.ID, success, chance)
	}
}

func (w *World) onCombineAgatha(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Agatha")
	if !ok {
		return
	}
	a, donor := req.Items[0], req.Items[1]
	defA, okA := w.items[a.Index]
	defB, okB := w.items[donor.Index]
	donorLevel := itemAbility(donor, defB, "EF_ITEMLEVEL")
	validPos := defA.Pos >= 2 && defA.Pos <= 32 && defA.Pos&(defA.Pos-1) == 0
	if !okA || !okB || itemAbility(a, defA, "EF_MOBTYPE") != 1 ||
		itemAbility(donor, defB, "EF_ITEMTYPE") != 0 || defA.Pos != defB.Pos || !validPos ||
		itemSanc(a) < 9 || itemSanc(donor) < 9 || donorLevel < 4 {
		w.sendCombineResult(p, 0)
		return
	}
	for i := 2; i <= 5; i++ {
		if req.Items[i].Index != 3140 || itemStackAmount(req.Items[i]) > 1 {
			w.sendCombineResult(p, 0)
			return
		}
	}
	if req.Items[6].Index != 0 || req.Items[7].Index != 0 {
		w.sendCombineResult(p, 0)
		return
	}
	result := a
	result.Eff = donor.Eff
	if !setItemSanc(&result, 7) {
		w.sendCombineResult(p, 0)
		return
	}
	// GetMatchCombineAgatha (GetFunc.cpp:620-627): grade do item DOADOR, com o
	// caso especial nivel 5 + grade 1 -> grade 3; bonus +6 no nivel 5, senao +1.
	// Sem clamp -- a fonte compara "combine" cru contra o roll 0..99.
	grade := defB.Grade
	if donorLevel == 5 && grade == 1 {
		grade = 3
	}
	bonus := 1
	if donorLevel == 5 {
		bonus = 6
	}
	chance := agathaBaseChance + grade*5 + bonus
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 2, 5, changed)
	mainPos := int(req.Pos[0])
	p.Char.Inv[mainPos] = model.Item{}
	changed[mainPos] = struct{}{}
	success := combineRoll() <= chance
	if success {
		donorPos := int(req.Pos[1])
		p.Char.Inv[mainPos] = result
		p.Char.Inv[donorPos] = model.Item{}
		changed[donorPos] = struct{}{}
	}
	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
}

func (w *World) onCombineAylin(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Aylin")
	if !ok {
		return
	}
	a, b := req.Items[0], req.Items[1]
	defA, okA := w.items[a.Index]
	defB, okB := w.items[b.Index]
	validPos := defA.Pos == 2 || defA.Pos == 4 || defA.Pos == 8 || defA.Pos == 16 ||
		defA.Pos == 32 || defA.Pos == 64 || defA.Pos == 128 || defA.Pos == 192
	if !okA || !okB || a.Index != b.Index || defA.Grade != defB.Grade || !validPos ||
		itemSanc(a) != 9 || itemSanc(b) != 9 || req.Items[2].Index != 1774 || itemStackAmount(req.Items[2]) > 1 ||
		p.Char.Gold < aylinGoldCost || req.Items[7].Index != 0 {
		w.sendCombineResult(p, 0)
		return
	}
	for i := 3; i <= 6; i++ {
		if req.Items[i].Index < 2441 || req.Items[i].Index > 2444 || itemStackAmount(req.Items[i]) > 1 {
			w.sendCombineResult(p, 0)
			return
		}
	}
	result := a
	if !setItemSancRaw(&result, byte(230+req.Items[3].Index-2441)) {
		w.sendCombineResult(p, 0)
		return
	}
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 2, 6, changed)
	success := rand.Intn(100) < 40 // fluxo coerente da source 7.54 Secrets.
	if success {
		mainPos, donorPos := int(req.Pos[0]), int(req.Pos[1])
		p.Char.Inv[mainPos] = result
		p.Char.Inv[donorPos] = model.Item{}
		changed[mainPos], changed[donorPos] = struct{}{}, struct{}{}
	}
	p.Char.Gold -= aylinGoldCost
	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
}

func (w *World) onCombineLindy(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Lindy")
	if !ok {
		return
	}
	if req.Items[0].Index != 413 || itemStackAmount(req.Items[0]) != 10 ||
		req.Items[1].Index != 413 || itemStackAmount(req.Items[1]) != 10 ||
		req.Items[2].Index != 4127 || req.Items[7].Index != 0 {
		w.sendCombineResult(p, 0)
		return
	}
	for i := 3; i <= 6; i++ {
		if req.Items[i].Index != 413 || itemStackAmount(req.Items[i]) > 1 {
			w.sendCombineResult(p, 0)
			return
		}
	}
	// _MSG_CombineItemLindy escolhe a capa Elite pelo Clan atual: 3191
	// Hekalotia, 3192 Akelonia e 3193 neutra. O reino continua derivado da capa
	// antiga ate este ponto, portanto capture-o antes de substituir Equip[15].
	capeIndex := lindyCapeIndex(p.Char)
	cape := model.Item{Index: capeIndex, Eff: [6]byte{54, 16}}
	if _, exists := w.items[cape.Index]; !exists {
		w.sendCombineResult(p, 0)
		return
	}
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changedInv, changedEquip := make(map[int]struct{}, 7), map[int]struct{}{15: {}}
	consumeCombineItems(p.Char, req, 0, 6, changedInv)
	p.Char.Equip[15] = cape
	w.recalcPlayer(p.Char)
	w.commitCombine(p, oldInv, oldEquip, oldGold, changedInv, changedEquip, 1)
}

func lindyCapeIndex(ch *model.Char) uint16 {
	cape, _ := model.KingdomCapeAtTier(model.CapeTierElite, characterKingdom(ch))
	return cape
}
