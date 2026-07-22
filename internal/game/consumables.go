package game

import (
	"encoding/binary"
	"log"
	"math/rand"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	effectAmount     = 61
	maxCharacterGold = uint32(2_000_000_000)
	potionCooldown   = 100 * time.Millisecond
)

// useItemRequest e o MSG_UseItem 0x373 nativo (36 bytes). Os campos sao DWORD;
// validar antes de converter evita que valores forjados sejam truncados para um
// slot valido.
type useItemRequest struct {
	srcType, srcPos uint32
	dstType, dstPos uint32
	gridX, gridY    uint16
	warpID          uint16
}

func parseUseItemRequest(pkt []byte) (useItemRequest, bool) {
	if len(pkt) < 36 {
		return useItemRequest{}, false
	}
	r := useItemRequest{
		srcType: binary.LittleEndian.Uint32(pkt[12:16]),
		srcPos:  binary.LittleEndian.Uint32(pkt[16:20]),
		dstType: binary.LittleEndian.Uint32(pkt[20:24]),
		dstPos:  binary.LittleEndian.Uint32(pkt[24:28]),
		gridX:   binary.LittleEndian.Uint16(pkt[28:30]),
		gridY:   binary.LittleEndian.Uint16(pkt[30:32]),
		warpID:  binary.LittleEndian.Uint16(pkt[32:34]),
	}
	// O plugin 7.54 aceita uso somente a partir do Carry/Inv. Destino ainda e
	// validado, mesmo que pocao e barra nao o utilizem.
	if r.srcType != placeInv || r.srcPos >= model.PlayerCarrySlots ||
		r.dstType > placeStorage || r.dstPos >= model.PlayerCarrySlots {
		return useItemRequest{}, false
	}
	return r, true
}

// consumeOne porta AmountMinus: EF_AMOUNT (61) representa a pilha; sem esse
// efeito o STRUCT_ITEM e uma unidade e o slot deve ser zerado.
func consumeOne(item *model.Item) {
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] != effectAmount {
			continue
		}
		if item.Eff[i*2+1] > 1 {
			item.Eff[i*2+1]--
		} else {
			*item = model.Item{}
		}
		return
	}
	*item = model.Item{}
}

func volatileGoldValue(rule model.VolatileRule, item model.Item, def model.ItemDef) uint32 {
	switch rule.ValueSource {
	case "price":
		return def.Price
	case "coin_effects":
		return uint32(itemAbility(item, def, "EF_HWORDCOIN")*256 + itemAbility(item, def, "EF_LWORDCOIN"))
	default:
		return rule.Gold
	}
}

func applyRestore(ch *model.Char, item model.Item, def model.ItemDef, rule model.VolatileRule) (hp, mp uint32) {
	var requestedHP, requestedMP int
	if rule.ValueSource == "item_effects" {
		requestedHP = itemAbility(item, def, "EF_HP") + itemAbility(item, def, "EF_HPADD") +
			itemAbility(item, def, "EF_HPADD2")
		requestedMP = itemAbility(item, def, "EF_MP") + itemAbility(item, def, "EF_MPADD") +
			itemAbility(item, def, "EF_MPADD2")
	} else {
		requestedHP, requestedMP = rule.HP, rule.MP
	}
	if requestedHP > 0 {
		hp = restorePlayerHP(ch, uint32(requestedHP))
	}
	if requestedMP > 0 {
		mp = restorePlayerMP(ch, uint32(requestedMP))
	}
	return hp, mp
}

// onUseItem trata o MSG_UseItem 0x373. Toda decisao usa o item e o catalogo
// presentes no servidor; o ID/efeito alegado pelo cliente nunca e confiado.
func (w *World) onUseItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	req, ok := parseUseItemRequest(pkt)
	if !ok {
		log.Printf("[#%d] pacote de uso de item invalido", s.ID)
		return
	}
	w.cancelTrade(p, "item consumido")
	slot := byte(req.srcPos)
	item := &p.Char.Inv[slot]
	if item.Index == 0 {
		return
	}
	def, ok := w.items[item.Index]
	if !ok {
		log.Printf("[#%d] uso rejeitado: item %d ausente do catalogo server-side", s.ID, item.Index)
		return
	}

	rule, code, registered := w.volatiles.Rule(item.Index)
	if !registered {
		log.Printf("[#%d] uso rejeitado: item %d nao possui EF_VOLATILE server-side", s.ID, item.Index)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	switch rule.Action {
	case "magical_pill":
		if p.Char.MagicalPillUsed {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem, oldBonus := *item, p.Char.SkillPointBonus
		p.Char.MagicalPillUsed, p.Char.SkillPointBonus = true, oldBonus+9
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.SkillPointBonus, p.Char.MagicalPillUsed = oldItem, oldBonus, false
			w.recalcPlayer(p.Char)
			return
		}
		s.Send(wire.UpdateScore(p.ID, *p.Char))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "hunting_teleport":
		if req.warpID < 1 || req.warpID > 10 || len(rule.Destinations) != 10 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		dest, oldItem := rule.Destinations[int(req.warpID)-1], *item
		if rule.Consume {
			consumeOne(item)
		}
		if !w.teleportPlayer(p, dest.X, dest.Y) {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "learn_special_skill":
		bit := uint(rule.LearnedBit)
		if bit < 25 || bit > 29 || p.Char.LearnedSkill&(uint32(1)<<bit) != 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem, oldLearned := *item, p.Char.LearnedSkill
		p.Char.LearnedSkill |= uint32(1) << bit
		if rule.Consume {
			consumeOne(item)
		}
		filterShortSkills(p.Char)
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.LearnedSkill = oldItem, oldLearned
			return
		}
		s.Send(wire.UpdateScore(p.ID, *p.Char))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "summon_contract":
		oldItem := *item
		if rule.Summon == nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		// O summon anterior permanece intacto ate o consumo estar persistido.
		// Assim uma falha de disco nao apaga o contrato ativo nem o item.
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			return
		}
		if !w.replaceContractSummon(p, rule.Summon) {
			*item = oldItem
			if err := w.saveAccount(p.Account); err != nil {
				log.Printf("[#%d] ERRO ao restaurar contrato item=%d: %v", s.ID, oldItem.Index, err)
			}
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "restore":
		now := time.Now()
		if !p.LastPotion.IsZero() && now.Sub(p.LastPotion) < potionCooldown {
			// O cliente antecipa parte do uso visualmente; reenvia o slot real.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		hp, mp := applyRestore(p.Char, *item, def, rule)
		if hp == 0 && mp == 0 {
			// O cliente pode repetir automaticamente o uso de uma pilha. Se
			// ambos os recursos ja estao cheios, restaura o slot autoritativo
			// sem consumir uma unidade nem registrar uma cura inexistente.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		p.LastPotion = now
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "gold":
		value := volatileGoldValue(rule, *item, def)
		if value == 0 || value > maxCharacterGold {
			log.Printf("[#%d] volatile gold invalido item=%d valor=%d", s.ID, item.Index, value)
			return
		}
		if p.Char.Gold > maxCharacterGold || value > maxCharacterGold-p.Char.Gold {
			// Nativo nao consome a barra quando o total ultrapassaria 2 bilhoes.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			log.Printf("[#%d] barra item=%d rejeitada: gold=%d valor=%d teto=%d",
				s.ID, item.Index, p.Char.Gold, value, maxCharacterGold)
			return
		}
		oldItem, oldGold := *item, p.Char.Gold
		if rule.Consume {
			consumeOne(item)
		}
		p.Char.Gold += value
		// Barra vale ate 2 bilhoes: persiste antes de confirmar ao cliente para
		// impedir duplicacao por desconexao entre o uso e o autosave.
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.Gold = oldItem, oldGold
			log.Printf("[#%d] ERRO ao salvar uso da barra item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] usou barra item=%d gold=+%d total=%d",
			s.ID, oldItem.Index, value, p.Char.Gold)

	case "teleport":
		oldX, oldY := p.X, p.Y
		oldItem := *item
		p.X, p.Y = rule.X, rule.Y
		p.Char.X, p.Char.Y = p.X, p.Y
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			p.X, p.Y, p.Char.X, p.Char.Y = oldX, oldY, oldX, oldY
			*item = oldItem
			log.Printf("[#%d] ERRO ao salvar teleporte item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
		w.refreshPlayerVisibility(p)
		log.Printf("[#%d] usou teleporte item=%d volatile=%d @(%d,%d)->(%d,%d)",
			s.ID, def.Index, code, oldX, oldY, p.X, p.Y)

	case "refine":
		// Refino Ori/Lac (item-alvo no DestPos do 0x373). Toda a validacao e o
		// consumo ficam em refineItem: e o unico ponto destas fases onde um bug
		// vira exploit economico (refino infinito).
		w.refineItem(p, s, item, slot, req, rule, code)

	case "refine_set":
		// Molar do Gargula: refino FIXO no set de armadura (slots 1-5), sem
		// rolagem. Nao usa item-alvo -- age nas pecas equipadas.
		w.refineSet(p, s, item, slot, rule, code)

	case "save_position":
		// Gema Estelar (volatile 12): grava a coordenada ATUAL na ficha para o
		// Warp usar depois. Persiste antes de confirmar para nao duplicar o item.
		oldSX, oldSY, oldItem := p.Char.SavedX, p.Char.SavedY, *item
		p.Char.SavedX, p.Char.SavedY = p.X, p.Y
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			p.Char.SavedX, p.Char.SavedY, *item = oldSX, oldSY, oldItem
			log.Printf("[#%d] ERRO ao salvar save_position item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Coordenada salva."))
		log.Printf("[#%d] gema estelar salvou (%d,%d) item=%d volatile=%d",
			s.ID, p.X, p.Y, oldItem.Index, code)

	case "warp_saved":
		// Warp (volatile 13): teleporta para a coordenada gravada pela Gema. Sem
		// coordenada salva, nao consome. Mesmo fluxo do teleporte fixo.
		if p.Char.SavedX == 0 || p.Char.SavedY == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Nenhuma coordenada salva."))
			return
		}
		oldX, oldY, oldItem := p.X, p.Y, *item
		p.X, p.Y = p.Char.SavedX, p.Char.SavedY
		p.Char.X, p.Char.Y = p.X, p.Y
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			p.X, p.Y, p.Char.X, p.Char.Y, *item = oldX, oldY, oldX, oldY, oldItem
			log.Printf("[#%d] ERRO ao salvar warp_saved item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
		w.refreshPlayerVisibility(p)
		log.Printf("[#%d] warp para coordenada salva (%d,%d) item=%d volatile=%d",
			s.ID, p.X, p.Y, oldItem.Index, code)

	case "buff":
		// Pocoes de dano, comidas e buffs de tempo. O affect e escolhido por
		// dado (AffectType) e sua formula vive em applyExtendedAffectStats. O
		// numero de balanceamento (% ou nivel) e a duracao vem do volatiles.json.
		affectType := byte(rule.AffectType)
		if affectType == 0 {
			log.Printf("[#%d] volatile buff sem affectType item=%d code=%d", s.ID, item.Index, code)
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		var applied bool
		if rule.Accumulate {
			// Bau de EXP e comidas: SOMA tempo ate o teto; recusa quando saturado.
			applied = accumulateAffect(p.Char, affectType, rule.AffectValue, rule.AffectLevel,
				rule.DurationUnits, rule.MaxDurationUnits)
		} else {
			applied = setAffect(p.Char, affectType, rule.AffectValue, rule.AffectLevel, rule.DurationUnits)
		}
		if !applied {
			// Buff igual/mais forte ja ativo, ou tempo ja no teto: o nativo nao deixa
			// "usar/comer mais". Reenvia o slot autoritativo e nao consome a unidade.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		// O 0x336 do buff faz o client re-renderizar o avatar a partir do m_Equip
		// cru, perdendo a cor da tintura (que so viaja no mesh/anct do CreateMob).
		// Reasseverar a aparencia DEPOIS do 0x336 mantem a tintura visivel, como ja
		// fazem transform/refino/amago. Ultimo pacote visual do fluxo.
		w.refreshAppearance(p)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] usou buff item=%d volatile=%d affect=%d value=%d level=%d dur=%d",
			s.ID, item.Index, code, affectType, rule.AffectValue, rule.AffectLevel, rule.DurationUnits)

	case "grant_exp":
		// Baus e poeiras de XP. Reusa grantExp (progressao Mortal); persiste antes
		// de confirmar ao client, como qualquer ganho de EXP relevante.
		if rule.Exp == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem := *item
		// O recalc CRU dentro de grantExp clampa HP/MP no max sem buffs; preserva
		// os valores atuais para um jogador buffado nao perder vida ao ganhar EXP.
		oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
		gained, applied := grantExp(p.Char, rule.Exp)
		if applied == 0 {
			// EXP no teto Mortal: nao consome nem promete um ganho inexistente.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			log.Printf("[#%d] ERRO ao salvar grant_exp item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		w.recalcPlayer(p.Char)
		if oldHP > 0 {
			setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
		}
		if oldMP > 0 {
			setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
		}
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		// Igual ao kill: UpdateScore (0x336) so e preciso ao subir de nivel; mandado a
		// toa ele reconstroi o avatar e apaga a tintura. A exp vai no UpdateEtc.
		if gained > 0 {
			s.Send(wire.UpdateScore(p.ID, *p.Char))
			w.refreshAppearance(p)
		}
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] usou grant_exp item=%d volatile=%d exp=+%d niveis=+%d",
			s.ID, oldItem.Index, code, applied, gained)

	case "disabled":
		// Item que NAO deveria ter volatile (ex.: code 9). Consome sem efeito, para
		// tirar de circulacao um item cujo volatile so causaria confusao. A decisao
		// de consumir e explicita: difere do generic, que preserva o item.
		if rule.Consume {
			consumeOne(item)
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] volatile desativado item=%d code=%d (consumido=%v)", s.ID, item.Index, code, rule.Consume)

	case "face_transform":
		// Transforma o rosto em monstro (70-77), COSMETICO. O mesh vai no Value do
		// affect; bodyMesh o aplica e o visual e propagado aos observadores.
		if !setFaceAffect(p.Char, rule.FaceMesh, rule.DurationUnits) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.refreshAppearance(p) // forca a skin no proprio dono (SelfEquip) + observadores
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] transformou rosto item=%d volatile=%d mesh=%d dur=%d",
			s.ID, item.Index, code, rule.FaceMesh, rule.DurationUnits)

	case "face_restore":
		// Volta ao rosto real (volatile 89). So consome se havia transformacao.
		restored := removeFaceAffect(p.Char)
		if rule.Consume && restored {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.refreshAppearance(p) // forca a volta da skin no dono + observadores
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] restaurou rosto item=%d volatile=%d (havia=%v)", s.ID, item.Index, code, restored)

	case "tint":
		// Tintura (186): pinta o item-alvo trocando o codigo do slot de sanc para
		// a cor (116-125). COSMETICO -- o nivel de refino (valor) nao muda, entao
		// nao ha recalculo de stats. Persiste antes de confirmar e propaga o visual
		// aos observadores quando o alvo esta equipado.
		dest, destType, destPos := w.destItemTarget(p, req)
		if dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Arraste a tintura sobre um item."))
			return
		}
		oldDest, oldItem := *dest, *item
		if !tintItem(dest, rule.Color) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Tinja um item refinado (com brilho de +)."))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*dest, *item = oldDest, oldItem
			log.Printf("[#%d] ERRO ao salvar tint: %v", s.ID, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		if destType == placeEquip {
			s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
			w.refreshAppearance(p) // re-materializa o avatar do dono NO MUNDO + observadores
		}
		log.Printf("[#%d] tinturou item=%d alvo=%d cor=%d volatile=%d",
			s.ID, item.Index, dest.Index, rule.Color, code)

	case "untint":
		// Removedor de tintura (Remover_Enamel 3417): converte a cor de volta para
		// EF_SANC (43). So consome se removeu algo.
		dest, destType, destPos := w.destItemTarget(p, req)
		if dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Arraste sobre um item colorido."))
			return
		}
		oldDest, oldItem := *dest, *item
		if !untintItem(dest) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Use em itens coloridos."))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*dest, *item = oldDest, oldItem
			log.Printf("[#%d] ERRO ao salvar untint: %v", s.ID, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		if destType == placeEquip {
			s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
			w.refreshAppearance(p) // re-materializa o avatar do dono NO MUNDO + observadores
		}
		log.Printf("[#%d] removeu tintura item=%d alvo=%d volatile=%d",
			s.ID, item.Index, dest.Index, code)

	case "repliction":
		// Repliction (190, Grade A-E): sorteia um adicional do pool e o grava num
		// slot de efeito livre do item-alvo ARRASTADO (equip ou inv) -- nao ha
		// restricao de manto. Muda stats -> persist-before-confirm com rollback.
		dest, destType, destPos := w.destItemTarget(p, req)
		if dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Arraste a repliction sobre um item."))
			return
		}
		add, ok := pickVolatileAdd(rule.AddPool)
		if !ok {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldDest, oldItem := *dest, *item
		// Grava no 1o slot de efeito LIVRE; sem slot livre, sobrescreve o 2o (o
		// "adicional", como o stEffect[1] do W2PP).
		wrote := false
		for i := 0; i < 3; i++ {
			if dest.Eff[i*2] == 0 {
				dest.Eff[i*2], dest.Eff[i*2+1] = byte(add.Effect), byte(add.Value)
				wrote = true
				break
			}
		}
		if !wrote {
			dest.Eff[2], dest.Eff[3] = byte(add.Effect), byte(add.Value)
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*dest, *item = oldDest, oldItem
			log.Printf("[#%d] ERRO ao salvar repliction: %v", s.ID, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		if destType == placeEquip {
			w.recalcPlayer(p.Char)
			s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
			s.Send(wire.UpdateScore(p.ID, *p.Char))
			w.syncPlayerVitals(p)
			w.refreshAppearance(p)
		}
		s.Send(wire.MessagePanel("O item recebeu um novo adicional!"))
		log.Printf("[#%d] repliction item=%d alvo=%d add=ef%d/v%d volatile=%d",
			s.ID, item.Index, dest.Index, add.Effect, add.Value, code)

	case "mount":
		// Consumiveis de montaria (amago 16, racao 15, longevidade 93, crescimento
		// 94, invuln 90-92, choco 196). rule.MountAction escolhe o efeito; a logica
		// fiel ao W2PP fica em mount.go.
		w.applyMountItem(p, s, item, slot, rule, code)

	case "generic":
		// Ponto unico para os volatiles ainda sem regra definitiva. Nao altera
		// estado nem consome o item: o log fornece todos os campos necessarios
		// para identificar o comportamento quando ele for configurado.
		log.Printf("[#%d] VOLATILE GENERICO code=%d item=%d src=%d/%d dst=%d/%d grid=(%d,%d) packetItem=%d",
			s.ID, code, item.Index, req.srcType, req.srcPos, req.dstType, req.dstPos,
			req.gridX, req.gridY, req.warpID)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	default:
		log.Printf("[#%d] item %d volatile=%d action=%s ainda nao possui handler ativo",
			s.ID, item.Index, code, rule.Action)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	}
}

// pickVolatileAdd sorteia UM adicional do pool da repliction, respeitando o peso
// de cada entrada (Weight ausente ou <=0 conta como 1). Devolve ok=false para
// pool vazio, para o handler recusar sem consumir.
func pickVolatileAdd(pool []model.VolatileAdd) (model.VolatileAdd, bool) {
	total := 0
	for _, a := range pool {
		w := a.Weight
		if w <= 0 {
			w = 1
		}
		total += w
	}
	if total == 0 {
		return model.VolatileAdd{}, false
	}
	r := rand.Intn(total)
	for _, a := range pool {
		w := a.Weight
		if w <= 0 {
			w = 1
		}
		if r < w {
			return a, true
		}
		r -= w
	}
	return pool[len(pool)-1], true
}

// refineRoll porta a rolagem de refino do W2PP (_MSG_UseItem.cpp:424): sorteia
// 0..114, corrige a faixa alta e compara com g_pCelestialRate[sanc]. Sanc fora
// da tabela nunca refina.
func refineRoll(sanc int) bool {
	if sanc < 0 || sanc >= len(celestialRate) {
		return false
	}
	rd := rand.Intn(115)
	if rd > 100 {
		rd -= 15
	}
	return rd <= celestialRate[sanc]
}

// refineSet porta o Molar do Gargula (Micronics MolarGargula): refino FIXO no
// set de armadura equipado (slots 1-5), sem rolagem. Sobe cada peca ate o teto
// (RefineMax=6), nunca rebaixa, e so consome se ALGO mudou. Persiste antes de
// confirmar, como o refino Ori/Lac.
func (w *World) refineSet(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	rule model.VolatileRule, code int) {
	level := rule.RefineMax
	if level <= 0 || level > 15 {
		level = 6
	}
	oldEquip, oldPowder := p.Char.Equip, *powder
	changed := 0
	for slot := 1; slot <= 5; slot++ {
		it := &p.Char.Equip[slot]
		if it.Index == 0 || itemSanc(*it) >= level {
			continue
		}
		if setItemSanc(it, level) {
			changed++
		}
	}
	if changed == 0 {
		s.Send(wire.SendItem(p.ID, placeInv, powderSlot, *powder))
		s.Send(wire.MessagePanel("Nenhuma peca de armadura para refinar."))
		return
	}
	if rule.Consume {
		consumeOne(powder)
	}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Equip, *powder = oldEquip, oldPowder
		log.Printf("[#%d] ERRO ao salvar refine_set: %v", s.ID, err)
		return
	}
	w.recalcPlayer(p.Char)
	s.Send(wire.SendItem(p.ID, placeInv, powderSlot, *powder))
	// Reenvia CADA peca de armadura equipada individualmente: e o SendItem por
	// slot (nao o SelfEquip do array inteiro) que faz o client redesenhar o brilho
	// do refino sem precisar desequipar/equipar. Mesmo padrao do refino individual.
	for slot := 1; slot <= 5; slot++ {
		if p.Char.Equip[slot].Index != 0 {
			s.Send(wire.SendItem(p.ID, placeEquip, byte(slot), p.Char.Equip[slot]))
		}
	}
	s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	w.syncPlayerVitals(p)
	// O brilho/mesh do refino do set viaja no CreateMob: propaga aos observadores.
	w.refreshAppearance(p)
	s.Send(wire.MessagePanel("Set refinado!"))
	log.Printf("[#%d] refine_set code=%d po=%d pecas=%d nivel=+%d",
		s.ID, code, oldPowder.Index, changed, level)
}

// destRefineTarget resolve o item-alvo do refino pelo DestType/DestPos do 0x373.
// Refino e SOMENTE em item EQUIPADO (regra do felipe e do client 7.48, que so
// deixa arrastar Ori/Lac sobre o equip -- SGrid:2324). Alvo no inventario/cargo
// devolve nil, e refineItem responde "equipe o item para refinar".
func (w *World) destRefineTarget(p *Player, req useItemRequest) (*model.Item, uint32, uint32) {
	if req.dstType == placeEquip && req.dstPos < 16 {
		return &p.Char.Equip[req.dstPos], req.dstType, req.dstPos
	}
	return nil, 0, 0
}

// destItemTarget resolve o item-alvo aceitando equip OU inventario (usado pela
// tintura, que pode pintar item guardado ou vestido). Cargo nao entra: o client
// nao arrasta consumivel sobre o bau.
func (w *World) destItemTarget(p *Player, req useItemRequest) (*model.Item, uint32, uint32) {
	switch req.dstType {
	case placeEquip:
		if req.dstPos < 16 {
			return &p.Char.Equip[req.dstPos], req.dstType, req.dstPos
		}
	case placeInv:
		if int(req.dstPos) < len(p.Char.Inv) {
			return &p.Char.Inv[req.dstPos], req.dstType, req.dstPos
		}
	}
	return nil, 0, 0
}

// refineItem porta o handler de refino Ori/Lac do W2PP. Regras server-side:
// so equipamento, recusa consumivel/EF_NOSANC, respeita o teto por tipo de po,
// consome a poeira em sucesso E falha, e PERSISTE antes de confirmar ao client
// (refino e o vetor classico de dupe/refino infinito).
func (w *World) refineItem(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	req useItemRequest, rule model.VolatileRule, code int) {
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, powderSlot, *powder)) }

	// Ori/Lac sobre um OVO no inventario inicia/avanca a incubacao (choco) em vez
	// de refinar. Fiel ao W2PP, que trata o ovo no mesmo handler de refino.
	if egg, eggSlot := w.destEggTarget(p, req); egg != nil {
		w.incubateEgg(p, s, powder, powderSlot, egg, eggSlot, code)
		return
	}

	dest, destType, destPos := w.destRefineTarget(p, req)
	if dest == nil {
		// Alvo nao e um slot de equip: refino e somente em item equipado.
		resend()
		s.Send(wire.MessagePanel("Equipe o item para refinar."))
		return
	}
	if dest.Index == 0 {
		resend()
		return
	}
	def, ok := w.items[dest.Index]
	if !ok {
		resend()
		return
	}
	// So equipamento (tem EF_POS), nao consumivel (EF_VOLATILE), nao selado
	// (EF_NOSANC). itemAbility soma efeitos estaticos do def e da instancia.
	if def.Pos == 0 || itemAbility(*dest, def, "EF_VOLATILE") != 0 ||
		itemAbility(*dest, def, "EF_NOSANC") != 0 {
		resend()
		s.Send(wire.MessagePanel("Esse item nao pode ser refinado."))
		return
	}
	sanc := itemSanc(*dest)
	limit := rule.RefineMax
	if limit <= 0 || limit > 15 {
		limit = 15
	}
	if sanc >= limit {
		resend()
		s.Send(wire.MessagePanel("Esse item nao pode ser refinado mais."))
		return
	}

	oldPowder, oldDest := *powder, *dest
	success := refineRoll(sanc)
	if success {
		if !setItemSanc(dest, sanc+1) {
			// Sem par EF_SANC e sem slot livre (item cheio de adds): nao refina e
			// NAO consome a poeira, para nao punir o jogador por um item invalido.
			*dest = oldDest
			resend()
			s.Send(wire.MessagePanel("Esse item nao pode ser refinado."))
			return
		}
	}
	if rule.Consume {
		consumeOne(powder)
	}
	// Persiste ANTES de confirmar. Se a gravacao falhar, reverte tudo: nunca
	// confirmar ao client um refino que o disco nao guardou.
	if err := w.saveAccount(p.Account); err != nil {
		*powder, *dest = oldPowder, oldDest
		log.Printf("[#%d] ERRO ao salvar refino alvo=%d: %v", s.ID, oldDest.Index, err)
		return
	}
	resend()
	s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
	if destType == placeEquip {
		w.recalcPlayer(p.Char)
		s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
		s.Send(wire.UpdateScore(p.ID, *p.Char))
		w.syncPlayerVitals(p)
		// O brilho/mesh do refino viaja no CreateMob: propaga aos observadores.
		w.refreshAppearance(p)
	}
	if success {
		s.Send(wire.MessagePanel("Refino bem-sucedido!"))
	} else {
		s.Send(wire.MessagePanel("O refino falhou."))
	}
	result := "FALHOU"
	if success {
		result = "OK"
	}
	log.Printf("[#%d] refino code=%d po=%d alvo=%d +%d->%d %s",
		s.ID, code, oldPowder.Index, oldDest.Index, sanc, sanc+1, result)
}
