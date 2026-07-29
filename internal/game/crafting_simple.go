package game

import (
	"fmt"
	"log"
	"math/rand"
	"strings"
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

// blockedCombineItem e Atila's_Crown. O nativo recusa a composicao se ele
// ocupar qualquer um dos 8 slots (GetFunc.cpp:55-59 e os equivalentes nas
// linhas 218, 263, 337, 400, 461, 505, 634, 669 e 740).
const blockedCombineItem = 747

func (w *World) beginCombine(s *net.Session, pkt []byte, npc string) (*Player, combineRequest, bool) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
		return nil, combineRequest{}, false
	}
	now := time.Now()
	if !p.LastCraft.IsZero() && now.Sub(p.LastCraft) < 800*time.Millisecond {
		s.Send(wire.MessagePanel("Wait a second before trying again."))
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
	for _, item := range req.Items {
		if item.Index == blockedCombineItem {
			w.sendCombineResult(p, 0)
			return p, combineRequest{}, false
		}
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
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, nil)
}

func (w *World) commitCombineWithPlayerState(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccountAndCharStateResult(p) }, nil)
}

func (w *World) commitCombineWithRollback(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, rollback func()) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, rollback)
}

func (w *World) commitCombineWithSave(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, persist func() error, rollback func()) bool {
	if err := persist(); err != nil {
		p.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold
		if rollback != nil {
			rollback()
		}
		// Composicoes que alteram equipamento recalculam o runtime antes de
		// persistir. Restaurar apenas o array deixava os stats autoritativos
		// apontando para o item que falhou no banco.
		if len(equipSlots) != 0 && p.Char.Extended != nil {
			w.recalcPlayer(p.Char)
		}
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
	if len(equipSlots) != 0 {
		// Equipamento muda score e precisa ser publicado incrementalmente; o
		// personagem ja visivel nunca deve ser recriado com CreateMob.
		w.syncPlayerScoreAndVitals(p)
	}
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

// compositorMaterials e o numero EXATO de materiais aceitos: nem 3, nem 5.
const compositorMaterials = 4

// compositorChance soma a base configurada ao bonus de cada material. Cada um
// dos quatro materiais precisa ser equipavel, do set D (item level 4) ou E (5),
// e refinado dentro de +7..+9 -- o bonus vem do refino.
//
// O Grade do item NAO entra na conta. A W2PP tem duas rotinas concorrentes: a
// tabela por (nivel, grade, refino) de GetMatchCombine, que e o que o binario
// executa, e g_pAnctChance[3] indexado por refino (Basedef.cpp:158), lido do
// CompRate.txt. A segunda e a que vale aqui -- na W2PP ela esta morta duas
// vezes (nenhuma funcao consulta o array, e o parser so faz _strupr na primeira
// coluna, entao "Item_+7" nunca casa com "ITEM_+7"). Os valores sao os do
// servidor, em data/server.txt, nao os do array de la.
//
// Devolve a chance, a quebra por material para o log e se a composicao vale.
func (w *World) compositorChance(req combineRequest) (int, string, bool) {
	chance := int(w.gameplay.CompositorBaseChance)
	detail := make([]string, 0, compositorMaterials)
	// A janela 7.48 possui quatro materiais reais nos slots 2..5. Aceitar
	// quatro itens espalhados por 2..7 permitia forjar uma disposicao que o
	// client legitimo nunca produz.
	for i := 2; i < 2+compositorMaterials; i++ {
		item := req.Items[i]
		if item.Index == 0 {
			return 0, "", false
		}
		def, exists := w.items[item.Index]
		level, sanc := itemAbility(item, def, "EF_ITEMLEVEL"), itemSanc(item)
		refine := sanc - model.CompositorMinRefine
		if !exists || def.Pos == 0 || level < 4 || level > 5 ||
			refine < 0 || refine >= model.CompositorRefineLevels {
			return 0, "", false
		}
		bonus := int(w.gameplay.CompositorRefineChance[refine])
		chance += bonus
		detail = append(detail, fmt.Sprintf("%d(set%c+%d:%d)",
			item.Index, "DE"[level-4], sanc, bonus))
	}
	if req.Items[6].Index != 0 || req.Items[7].Index != 0 ||
		len(detail) != compositorMaterials {
		return 0, "", false
	}
	return chance, strings.Join(detail, " "), true
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
	chance, breakdown, chanceOK := w.compositorChance(req)
	if !chanceOK {
		w.sendCombineResult(p, 0)
		return
	}
	resultIndex := uint16(targetDef.Extra + int(jewel.Index-2441))
	if _, resultExists := w.items[resultIndex]; !resultExists {
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
		log.Printf("[#%d] CRAFT Compositor sucesso=%t chance=%d (base %d + %s)",
			s.ID, success, chance, w.gameplay.CompositorBaseChance, breakdown)
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
	// Secrets 7.54 usa `rand()%100 <= 40`: sao 41 resultados (0..40).
	success := aylinRollSucceeds(rand.Intn(100))
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

func aylinRollSucceeds(roll int) bool {
	return roll >= 0 && roll <= 40
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
	// A receita existe exclusivamente para um Arch parado em uma das duas
	// travas. O handler W2PP retorna antes de consumir para qualquer outra
	// evolucao, nivel ou trava ja liberada.
	trava, destrava := lindyLevelUnlock(p.Char)
	if !destrava {
		w.sendCombineResult(p, 0)
		return
	}
	if trava == archLockLevel370 && counterBalance(p, fameCounter) < 1 {
		s.Send(wire.MessagePanel("You need 1 fame point."))
		w.sendCombineResult(p, 0)
		return
	}

	var cape model.Item
	if trava == archLockLevel355 {
		// _MSG_CombineItemLindy escolhe a capa Elite pelo Clan atual: 3191
		// Hekalotia, 3192 Akelonia e 3193 neutra. O reino continua derivado da
		// capa antiga ate este ponto, portanto capture-o antes de substituir.
		capeIndex := lindyCapeIndex(p.Char)
		cape = model.Item{Index: capeIndex, UID: p.Char.Equip[15].UID, Eff: [6]byte{54, 16}}
		var err error
		cape, err = materializeItem(cape)
		if err != nil {
			w.sendCombineResult(p, 0)
			return
		}
		if _, exists := w.items[cape.Index]; !exists {
			w.sendCombineResult(p, 0)
			return
		}
	}

	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	oldFame := copyCounters(p)
	old355, old370 := p.Char.ArchLevel355, p.Char.ArchLevel370
	changedInv := make(map[int]struct{}, 7)
	changedEquip := make(map[int]struct{}, 1)
	consumeCombineItems(p.Char, req, 0, 6, changedInv)
	// A capa Elite nasce somente no destrave 355. No 370 o nativo altera a
	// flag/fama e mantem a capa ja equipada intacta.
	if trava == archLockLevel355 {
		p.Char.Equip[15] = cape
		changedEquip[15] = struct{}{}
	}
	if trava == archLockLevel355 {
		p.Char.ArchLevel355 = true
	} else {
		p.Char.ArchLevel370 = true
		spendCounters(p, map[string]uint32{fameCounter: 1})
	}
	if len(changedEquip) != 0 {
		w.recalcPlayer(p.Char)
	}
	persisted := false
	if trava == archLockLevel370 {
		persisted = w.commitCombineWithPlayerState(
			p, oldInv, oldEquip, oldGold, changedInv, changedEquip, 1)
	} else {
		persisted = w.commitCombine(
			p, oldInv, oldEquip, oldGold, changedInv, changedEquip, 1)
	}
	if !persisted {
		// commitCombine ja restaurou inventario/equip; o resto e nosso.
		p.SpecialCoins = oldFame
		p.Char.ArchLevel355, p.Char.ArchLevel370 = old355, old370
		w.recalcPlayer(p.Char)
		return
	}
	s.Send(wire.MessagePanel("Your level limit has been lifted."))
	log.Printf("[#%d] ARCH destravou o nivel %d (fama=%d)", s.ID, trava+1, counterBalance(p, fameCounter))
}

// lindyLevelUnlock diz qual trava este personagem esta destravando, se alguma.
// So vale para Arch parado EXATAMENTE numa das travas e ainda nao liberada --
// gastar a receita fora disso nao pode marcar nada.
func lindyLevelUnlock(ch *model.Char) (uint32, bool) {
	if ch == nil || ch.Extended == nil || !isArch(ch) {
		return 0, false
	}
	switch ch.Extended.Level {
	case archLockLevel355:
		return archLockLevel355, !ch.ArchLevel355
	case archLockLevel370:
		return archLockLevel370, !ch.ArchLevel370
	}
	return 0, false
}

func lindyCapeIndex(ch *model.Char) uint16 {
	cape, _ := model.KingdomCapeAtTier(model.CapeTierElite, characterKingdom(ch))
	return cape
}
