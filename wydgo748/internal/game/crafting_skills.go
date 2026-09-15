package game

import (
	"encoding/binary"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	extracaoCatalyst = 1774
	effectItemLevel  = 87 // ItemEffect.h: EF_ITEMLEVEL
)

// beginSkillCraft aplica a mesma janela anti-spam dos compositores, mas sem
// exigir NPC: Extração e Alquimia são abertas pelas próprias skills.
func (w *World) beginSkillCraft(s *net.Session, skillIndex int) (*Player, bool) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil || p.Char.Score == nil {
		return nil, false
	}
	// A janela abre localmente; receber o opcode nao prova aprendizado.
	local := skillIndex - int(p.Char.Class)*24
	if p.Char.Class != 3 || local < 0 || local >= 24 ||
		p.Char.LearnedSkill&(uint32(1)<<local) == 0 || playerCurHP(p.Char) == 0 ||
		p.PersistencePoisoned || p.GhostShop != nil {
		return p, false
	}
	now := w.now()
	if !p.LastCraft.IsZero() && now.Sub(p.LastCraft) < 800*time.Millisecond {
		s.Send(wire.MessagePanel("Wait a second before trying again."))
		return p, false
	}
	p.LastCraft = now
	return p, true
}

func (w *World) beginSkillCombine(s *net.Session, pkt []byte) (*Player, combineRequest, bool) {
	p, ok := w.beginSkillCraft(s, 84)
	if !ok {
		return p, combineRequest{}, false
	}
	req, err := parseCombineRequest(pkt, p.Char)
	if err != nil {
		w.sendCombineResult(p, 0)
		return p, combineRequest{}, false
	}
	return p, req, true
}

// rollPercentStrict preserva a comparação usada por Extração no W2PP:
// o dado visível é 1..100 e só há sucesso quando roll < chance.
func (w *World) rollPercentStrict(chance int) percentRoll {
	chance = clampInt(chance, 0, 100)
	roll := w.intn(100) + 1
	return percentRoll{Roll: roll, Chance: chance, Success: roll < chance}
}

func (w *World) onCombineExtracao(s *net.Session, pkt []byte) {
	p, ok := w.beginSkillCraft(s, 83)
	if !ok || len(pkt) != 20 || p.Char.Score == nil {
		return
	}
	pos := int(binary.LittleEndian.Uint32(pkt[16:20]))
	if pos < 0 || pos >= model.PlayerCarrySlots {
		return
	}
	target := p.Char.Inv[pos]
	def, exists := w.items[target.Index]
	if target.Index == 0 || !exists || itemAbility(target, def, "EF_ITEMLEVEL") >= 5 ||
		itemAbility(target, def, "EF_MOBTYPE") != 0 {
		return
	}
	resultIndex, validPos := map[int]uint16{2: 3021, 4: 3022, 8: 3023, 16: 3024, 32: 3025}[def.Pos]
	if !validPos {
		return
	}

	catalystPos := -1
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if p.Char.Inv[i].Index == extracaoCatalyst {
			catalystPos = i
			break
		}
	}
	if catalystPos < 0 {
		return
	}

	w.cancelTrade(p, "skill Extracao")
	oldInv := p.Char.Inv
	// O W2PP usa BASE_ClearItem, portanto o slot 1774 inteiro é consumido,
	// mesmo se uma instância tiver sido indevidamente empilhada.
	p.Char.Inv[catalystPos] = model.Item{}
	changed := map[int]struct{}{catalystPos: {}, pos: {}}

	chance := (int(playerMastery(p.Char, 2)) - 20) / 5
	roll := w.rollPercentStrict(chance)
	if roll.Success {
		result := target
		itemLevel := itemAbility(target, def, "EF_ITEMLEVEL")
		result.Index = resultIndex
		result.Eff[0], result.Eff[1] = effectItemLevel, byte(clampInt(itemLevel, 0, 255))
		staticDamage := staticAbility(def, "EF_DAMAGE")
		for slot := 1; slot < 3; slot++ {
			effectPos := slot * 2
			if def.DynamicEffectNames[result.Eff[effectPos]] == "EF_DAMAGE" {
				result.Eff[effectPos+1] = byte(clampInt(int(result.Eff[effectPos+1])+staticDamage, 0, 255))
			}
		}
		p.Char.Inv[pos] = result
	} else {
		p.Char.Inv[pos] = model.Item{}
	}

	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv = oldInv
		for changedPos := range changed {
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(changedPos), p.Char.Inv[changedPos]))
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		p.Session.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "skill Extracao", err)
		return
	}
	for changedPos := range changed {
		p.Session.Send(wire.SendItem(p.ID, placeInv, byte(changedPos), p.Char.Inv[changedPos]))
	}
}

func alquimiaRecipe(req combineRequest) (int, bool) {
	for _, item := range req.Items {
		if item.Index == blockedCombineItem || item.Eff[0] == effectAmount && item.Eff[1] > 1 {
			return 0, false
		}
	}
	idx := func(i int) uint16 { return req.Items[i].Index }
	switch {
	case idx(0) == 413 && idx(1) == 2441 && idx(2) == 2442:
		return 0, true
	case idx(0) == 413 && idx(1) == 2443 && idx(2) == 2442:
		return 1, true
	case idx(0) == 4127 && idx(1) == 4127 && idx(2) == 4127:
		return 2, true
	case idx(0) == 4127 && idx(1) == 4127 && idx(2) == 697:
		return 3, true
	case idx(0) == 412 && idx(1) == 2441 && idx(2) == 2444:
		return 4, true
	case idx(0) == 412 && idx(1) == 2444 && idx(2) == 2443:
		return 5, true
	case idx(0) == 612 && itemSanc(req.Items[0]) >= 9 && idx(1) == 2441 && idx(2) == 2442:
		return 6, true
	case idx(0) == 612 && idx(1) == 613 && idx(2) == 614 && idx(3) == 615:
		return 7, true
	case idx(0) == 614 && itemSanc(req.Items[0]) >= 9 && idx(1) == 2443 && idx(2) == 2444:
		return 8, true
	case idx(0) == 615 && itemSanc(req.Items[0]) >= 9 && idx(1) == 697 && idx(2) == 697 && idx(3) == 697:
		return 9, true
	default:
		return 0, false
	}
}

func (w *World) onCombineAlquimia(s *net.Session, pkt []byte) {
	p, req, ok := w.beginSkillCombine(s, pkt)
	if !ok {
		return
	}
	combine, valid := alquimiaRecipe(req)
	if !valid || p.Char.Class != 3 || p.Char.Score == nil {
		w.sendCombineResult(p, 0)
		return
	}

	w.cancelTrade(p, "skill Alquimia")
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	firstPos := int(req.Pos[0])
	firstUID := req.Items[0].UID
	for i, item := range req.Items {
		if item.Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		p.Char.Inv[pos] = model.Item{}
		changed[pos] = struct{}{}
	}

	roll := w.rollPercent((int(playerMastery(p.Char, 2)) - 20) / 5)
	code := uint32(2)
	if roll.Success {
		result := model.Item{Index: uint16(3200 + combine), UID: firstUID}
		if p.Char.SecondaryLearnedSkill&0x10 != 0 {
			result.Eff[0], result.Eff[1] = effectAmount, 5
		}
		p.Char.Inv[firstPos] = result
		changed[firstPos] = struct{}{}
		code = 1
	}
	w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
}
