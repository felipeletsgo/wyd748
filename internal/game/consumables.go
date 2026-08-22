package game

import (
	"encoding/binary"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	effectAmount     = 61
	maxCharacterGold = uint32(2_000_000_000)
	potionCooldown   = 100 * time.Millisecond

	buffAlreadyActiveMessage = "This item cannot be used because the buff is already active."
	buffRejectedMessage      = "This buff cannot be applied right now."
)

// useItemRequest e o MSG_UseItem 0x373 nativo (36 bytes). Os campos sao DWORD;
// validar antes de converter evita que valores forjados sejam truncados para um
// slot valido.
type useItemRequest struct {
	srcType, srcPos uint32
	dstType, dstPos uint32
	gridX, gridY    uint16
	warpID          uint32
}

func parseUseItemRequest(pkt []byte) (useItemRequest, bool) {
	// MSG_UseItem/p373 is a fixed 36-byte layout. Accepting an oversized
	// packet here would let a forged trailer bypass the opcode size policy, and
	// reading only the low half of WarpID would truncate a malicious DWORD.
	if len(pkt) != 36 {
		return useItemRequest{}, false
	}
	r := useItemRequest{
		srcType: binary.LittleEndian.Uint32(pkt[12:16]),
		srcPos:  binary.LittleEndian.Uint32(pkt[16:20]),
		dstType: binary.LittleEndian.Uint32(pkt[20:24]),
		dstPos:  binary.LittleEndian.Uint32(pkt[24:28]),
		gridX:   binary.LittleEndian.Uint16(pkt[28:30]),
		gridY:   binary.LittleEndian.Uint16(pkt[30:32]),
		warpID:  binary.LittleEndian.Uint32(pkt[32:36]),
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
	case "equipment_gem":
		w.useEquipmentGem(s, p, item, slot, rule, req)

	case "ore_upgrade":
		w.useOreUpgrade(s, p, item, slot, rule, req)

	case "instance_ticket":
		if rule.Instance != nil && isDurablePrivateWaterConfig(*rule.Instance) {
			w.useWaterScroll(s, p, item, slot, rule, req, "")
		} else {
			w.useInstanceTicket(s, p, item, slot, rule, req)
		}

	case "loot_box":
		w.useLootBox(s, p, item, slot, rule)

	case "mount_revive":
		w.useMountRevive(s, p, item, slot, rule)

	case "timed_access":
		w.useTimedAccess(s, p, item, slot, rule)

	case "no_direct_use":
		w.useNoDirectItem(s, p, item, slot, "")

	case "celestial_capsule":
		w.useCelestialCapsule(s, p, item, slot)

	case "celestial_ideal":
		w.useCelestialIdeal(s, p, item, slot)

	case "celestial_fury":
		w.useFuryStone(s, p, item, slot)

	case "celestial_switch":
		w.useMysteriousStone(s, p, item, slot)

	case "nightmare_ticket":
		w.useNightmareTicket(s, p, item, slot, rule)

	case "refine_equipped":
		w.useEquippedRefine(s, p, item, slot, rule)

	case "territory_pass":
		w.useTerritoryPass(s, p, item, slot, rule)

	case "mastery_reset":
		w.useMasteryReset(s, p, item, slot, rule, code)

	case "firework":
		// O Premium FireCracker nao usa MSG_UseItem 0x373. A janela 10x10 do
		// client envia MSG_UseItem2 0x3C9; aceitar este caminho descartaria o
		// desenho e transformaria o item premium em um fogo comum.
		if rule.CustomPattern {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		parm := uint16(w.intn(6))
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 100, parm) })

	case "chaos_remission":
		// O nativo chama SetPKPoint(150): 150 e o byte bruto CP+75, portanto
		// a Carta do Perdao leva o CP autoritativo para o maximo +75.
		// Confirma o efeito somente depois de persistir, evitando carta duplicada
		// ou CP restaurado apenas em memoria numa queda.
		oldItem, oldCP := *item, p.Char.CP
		p.Char.CP = model.MaxCP
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.CP = oldItem, oldCP
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerChaos(p)

	case "grant_next_level":
		// Poeira de Fada nativa: posiciona a EXP exatamente no proximo marco,
		// portanto concede um unico nivel (e nao uma quantidade fixa arbitraria).
		if !matchesEvolution(p.Char, "mortal") ||
			p.Char.Score == nil || p.Char.Score.Level >= maxMortalLevel {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		next := mortalNextLevel[int(p.Char.Score.Level)+1]
		if p.Char.Exp >= next {
			syncProgression(p.Char)
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldIndex := item.Index
		res := w.grantItemExpReward(s, p, item, slot, next-p.Char.Exp, 0, rule.Consume)
		if !res.OK && res.Err != nil {
			log.Printf("[#%d] ERRO ao salvar poeira de fada item=%d: %v", s.ID, oldIndex, res.Err)
		}

	case "quest_reward":
		w.useQuestReward(s, p, item, slot, rule, code)

	case "arch_crystal":
		w.useArchCrystal(s, p, item, slot, rule, code)

	case "grant_counter":
		w.useCounterGrant(s, p, item, slot, rule, code)

	case "grant_counter_once":
		w.useCounterGrantOnce(s, p, item, slot, rule, code)

	case "gate_key":
		w.useGateKey(s, p, item, slot, rule, code)

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
		s.Send(playerScorePacket(p))
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
		s.Send(playerScorePacket(p))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "summon_contract":
		oldItem := *item
		if rule.Summon == nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		plan, ok := w.planContractSummon(p, rule.Summon)
		if !ok {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			return
		}
		w.commitContractSummon(plan)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "restore":
		now := w.now()
		if !p.LastPotion.IsZero() && now.Sub(p.LastPotion) < potionCooldown {
			// O cliente antecipa parte do uso visualmente; reenvia o slot real.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		snapshot := cloneCharacterState(p.Char)
		oldLastPotion := p.LastPotion
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
		// A potion changes both the stack and the authoritative current
		// resource. Persist before acknowledging the use; otherwise a forced
		// disconnect between SendItem and autosave can duplicate the unit or
		// roll the resource back independently of the inventory.
		if err := w.saveAccount(p.Account); err != nil {
			*p.Char = snapshot
			p.LastPotion = oldLastPotion
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			log.Printf("[#%d] ERRO ao salvar pocao item=%d: %v", s.ID, item.Index, err)
			return
		}
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
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		oldX, oldY := p.X, p.Y
		if !w.teleportPlayer(p, rule.X, rule.Y) {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
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
		s.Send(wire.MessagePanel("Position saved."))
		log.Printf("[#%d] gema estelar salvou (%d,%d) item=%d volatile=%d",
			s.ID, p.X, p.Y, oldItem.Index, code)

	case "warp_saved":
		// Warp (volatile 13): teleporta para a coordenada gravada pela Gema. Sem
		// coordenada salva, nao consome. Mesmo fluxo do teleporte fixo.
		if p.Char.SavedX == 0 || p.Char.SavedY == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("No position saved."))
			return
		}
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		if !w.teleportPlayer(p, p.Char.SavedX, p.Char.SavedY) {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] warp para coordenada salva (%d,%d) item=%d volatile=%d",
			s.ID, p.X, p.Y, oldItem.Index, code)

	case "buff":
		// Pocoes de dano, comidas e buffs de tempo. O affect e escolhido por
		// dado (AffectType) e sua formula vive em applyExtendedAffectStats. O
		// numero de balanceamento (% ou nivel) e a duracao vem do volatiles.json.
		snapshot := cloneCharacterState(p.Char)
		affects := rule.Affects
		if len(affects) == 0 && rule.AffectType > 0 {
			affects = []model.VolatileAffect{{
				Type: rule.AffectType, Value: rule.AffectValue,
				Level: rule.AffectLevel, DurationUnits: rule.DurationUnits,
			}}
		}
		if len(affects) == 0 {
			log.Printf("[#%d] volatile buff sem affect item=%d code=%d", s.ID, item.Index, code)
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		buffResult := w.applyVolatileBuff(p.Char, rule)
		if buffResult != volatileBuffApplied {
			// Buff igual/mais forte ja ativo, ou tempo ja no teto: o nativo nao deixa
			// "usar/comer mais". Reenvia o slot autoritativo, avisa o motivo e nao
			// consome nem persiste a unidade.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			if buffResult == volatileBuffAlreadyActive {
				s.Send(wire.MessagePanel(buffAlreadyActiveMessage))
			} else {
				s.Send(wire.MessagePanel(buffRejectedMessage))
				log.Printf("[#%d] buff item=%d recusado: configuracao invalida ou slots de affect cheios",
					s.ID, item.Index)
			}
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		// Affects temporizados tambem sao estado economico: confirma consumo e
		// duracao no mesmo save. Sem isto, desconectar antes do autosave devolvia
		// o item e permitia repetir Bigger Potion/comidas/bau de EXP.
		if err := w.saveAccountAndCharStateResult(p); err != nil {
			*p.Char = snapshot
			s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
			log.Printf("[#%d] ERRO ao salvar buff item=%d: %v", s.ID, item.Index, err)
			return
		}
		w.publishPlayerAffects(p)
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] usou buff item=%d volatile=%d affects=%d",
			s.ID, item.Index, code, len(affects))

	case "grant_exp":
		// Baus e poeiras de XP. Delegam a transacao comum de EXP (snapshot completo
		// + persist-before-confirm); aqui a EXP e bruta, sem rate/buff nem gold.
		if rule.Exp == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldIndex := item.Index
		res := w.grantItemExpReward(s, p, item, slot, rule.Exp, 0, rule.Consume)
		if !res.OK {
			if res.Err != nil {
				log.Printf("[#%d] ERRO ao salvar grant_exp item=%d: %v", s.ID, oldIndex, res.Err)
			}
			return
		}
		log.Printf("[#%d] usou grant_exp item=%d volatile=%d exp=+%d niveis=+%d",
			s.ID, oldIndex, code, res.Exp, res.Levels)

	case "disabled":
		// Item que NAO deveria ter volatile (ex.: code 9). Consome sem efeito, para
		// tirar de circulacao um item cujo volatile so causaria confusao. A decisao
		// de consumir e explicita: difere do generic, que preserva o item.
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			log.Printf("[#%d] ERRO ao salvar volatile desativado item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] volatile desativado item=%d code=%d (consumido=%v)", s.ID, item.Index, code, rule.Consume)

	case "face_transform":
		// Transforma o rosto em monstro (70-77), COSMETICO. O mesh vai no Value do
		// affect; bodyMesh o aplica e o visual e propagado aos observadores.
		snapshot := cloneCharacterState(p.Char)
		if !setFaceAffectAt(p.Char, rule.FaceMesh, rule.DurationUnits, w.now()) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccountAndCharStateResult(p); err != nil {
			*p.Char = snapshot
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			log.Printf("[#%d] ERRO ao salvar transformacao facial item=%d: %v", s.ID, item.Index, err)
			return
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.refreshAppearance(p) // forca a skin no proprio dono (SelfEquip) + observadores
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] transformou rosto item=%d volatile=%d mesh=%d dur=%d",
			s.ID, item.Index, code, rule.FaceMesh, rule.DurationUnits)

	case "face_restore":
		// Volta ao rosto real (volatile 89). So consome se havia transformacao.
		snapshot := cloneCharacterState(p.Char)
		restored := removeFaceAffect(p.Char)
		if rule.Consume && restored {
			consumeOne(item)
		}
		if restored {
			if err := w.saveAccountAndCharStateResult(p); err != nil {
				*p.Char = snapshot
				s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
				log.Printf("[#%d] ERRO ao salvar restauracao facial item=%d: %v", s.ID, item.Index, err)
				return
			}
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
			s.Send(wire.MessagePanel("Drag the dye onto an item."))
			return
		}
		oldDest, oldItem := *dest, *item
		if !tintItem(dest, rule.Color) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Dye a refined item (one with a + glow)."))
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
			w.refreshAppearance(p)
		}
		log.Printf("[#%d] tinturou item=%d alvo=%d cor=%d volatile=%d",
			s.ID, item.Index, dest.Index, rule.Color, code)

	case "untint":
		// Removedor de tintura (Remover_Enamel 3417): converte a cor de volta para
		// EF_SANC (43). So consome se removeu algo.
		dest, destType, destPos := w.destItemTarget(p, req)
		if dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Drag it onto a colored item."))
			return
		}
		oldDest, oldItem := *dest, *item
		if !untintItem(dest) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Use it on colored items."))
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
			w.refreshAppearance(p)
		}
		log.Printf("[#%d] removeu tintura item=%d alvo=%d volatile=%d",
			s.ID, item.Index, dest.Index, code)

	case "repliction":
		// Repliction (190, A-E normal/premium) porta SetItemBonus2: sorteia um
		// PAR completo conforme a parte da armadura e substitui os dois adds.
		// O terceiro par logico permanece reservado para refino/tintura.
		dest, destType, destPos := w.destItemTarget(p, req)
		if destType != placeEquip || destPos >= 16 || dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Repliction can only be used on equipped defensive equipment."))
			return
		}
		def, exists := w.items[dest.Index]
		if !exists {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}

		oldDest, oldItem := *dest, *item
		bonus, err := w.applyRepliction(dest, def, item.Index)
		if err != nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel(err.Error()))
			return
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
		s.Send(wire.SendItem(
			p.ID,
			byte(destType),
			byte(destPos),
			*dest,
		))

		if destType == placeEquip {
			w.recalcPlayer(p.Char)
			s.Send(wire.SelfEquip(p.ID, p.Char.Equip[:]))
			s.Send(playerScorePacket(p))
			w.syncPlayerVitalsToObservers(p)
		}

		s.Send(wire.MessagePanel("The item gained a new bonus!"))

		log.Printf(
			"[#%d] repliction item=%d alvo=%d adds=ef%d/v%d+ef%d/v%d volatile=%d",
			s.ID,
			item.Index,
			dest.Index,
			bonus.Effect1, bonus.Value1,
			bonus.Effect2, bonus.Value2,
			code,
		)

	case "mount":
		// Consumiveis de montaria (amago 16, racao 15, LP/catalisadores 90-92,
		// longevidade 93, crescimento 94, choco 196). rule.MountAction escolhe o efeito; a logica
		// fiel ao W2PP fica em mount.go.
		w.applyMountItem(p, s, item, slot, rule, code, req)

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

func (w *World) applyRepliction(dest *model.Item, def model.ItemDef, sourceID uint16) (model.ReplictionBonus, error) {
	rule, ok := w.volatiles.Repliction.Items[sourceID]
	if !ok {
		return model.ReplictionBonus{}, clientError("This repliction is not configured.")
	}
	pool := w.volatiles.Repliction.Pools[def.Pos]
	if len(pool) == 0 {
		return model.ReplictionBonus{}, clientError("Repliction works only on defensive equipment.")
	}
	if dest.Index >= 3500 && dest.Index <= 3507 {
		return model.ReplictionBonus{}, clientError("Repliction cannot be used on Cythera.")
	}
	if sanc := itemSanc(*dest); sanc > rule.MaxSanc {
		return model.ReplictionBonus{}, clientError("The refinement level is too high for this repliction.")
	}
	mobType := itemAbility(*dest, def, "EF_MOBTYPE")
	if mobType != 0 && mobType != 2 {
		return model.ReplictionBonus{}, clientError("This item type cannot receive Repliction.")
	}
	if itemAbility(*dest, def, "EF_ITEMLEVEL") != rule.ItemLevel {
		return model.ReplictionBonus{}, clientError("The repliction grade does not match the item.")
	}

	// Preserva refino ou tintura independentemente do slot em que o item antigo
	// o armazenava. Item ainda sem refino recebe EF_SANC +0, como SetItemBonus2.
	sancEffect, sancValue := byte(0), byte(0)
	for id, name := range def.DynamicEffectNames {
		if name == "EF_SANC" {
			sancEffect = id
			break
		}
	}
	if sancEffect == 0 {
		return model.ReplictionBonus{}, clientError("EF_SANC is missing from ItemEffect.")
	}
	for i := 0; i < 3; i++ {
		effect := dest.Eff[i*2]
		if effect == sancEffect || (effect >= 116 && effect <= 125) {
			sancEffect, sancValue = effect, dest.Eff[i*2+1]
			break
		}
	}
	bonus := pool[w.intn(len(pool))]
	dest.Eff = [6]byte{
		sancEffect, sancValue,
		byte(bonus.Effect1), byte(bonus.Value1),
		byte(bonus.Effect2), byte(bonus.Value2),
	}
	return bonus, nil
}

// refineChance devolve a chance configurada para o nivel atual. A decisao
// percentual e centralizada em World.rollPercent: dominio uniforme 1..100 e
// sucesso quando roll <= chance.
func refineChance(sanc int) int {
	if sanc < 0 || sanc >= len(celestialRate) {
		return 0
	}
	return clampInt(celestialRate[sanc], 0, 100)
}

// refineSet porta o Molar do Gargula (Micronics MolarGargula): refino FIXO no
// set de armadura equipado (slots 1-5), sem rolagem. Sobe cada peca ate o teto
// (RefineMax=6), nunca rebaixa, e so consome se ALGO mudou. Persiste antes de
// confirmar, como o refino Ori/Lac.
func (w *World) refineSet(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	rule model.VolatileRule, code int) {
	if p == nil || p.Char == nil || p.Char.Score == nil {
		return
	}
	resend := func() {
		if int(powderSlot) < len(p.Char.Inv) {
			s.Send(wire.SendItem(p.ID, placeInv, powderSlot, p.Char.Inv[powderSlot]))
		}
	}
	// O Molar e uma quest do Mortal, nao um consumivel de refino generico.
	// Os limites sao internos: 200..255 correspondem aos niveis exibidos
	// 201..256 no client 7.48. A flag negativa e deliberadamente separada das
	// quests de data para impedir que uma quest configurada reutilize o premio.
	if rule.MortalOnly && !matchesEvolution(p.Char, "mortal") {
		resend()
		s.Send(wire.MessagePanel("This item is exclusive to Mortals."))
		return
	}
	if rule.RequiredEvolution != "" && !matchesEvolution(p.Char, rule.RequiredEvolution) {
		resend()
		s.Send(wire.MessagePanel("This item cannot be used by this evolution."))
		return
	}
	playerLevel := p.Char.Score.Level
	if (rule.MinLevel != 0 && playerLevel < rule.MinLevel) ||
		(rule.MaxLevelExclusive != 0 && playerLevel >= rule.MaxLevelExclusive) {
		resend()
		s.Send(wire.MessagePanel("This item is only available between levels 201 and 256."))
		return
	}
	if rule.OnceQuestID != 0 && questCompleted(p.Char, int(rule.OnceQuestID)) {
		resend()
		s.Send(wire.MessagePanel("You have already completed this quest."))
		return
	}
	// A Cythera occupies the first armor slot but is not a piece of the mortal
	// armor set. The native 7.54 handler rejects the whole use when it is
	// equipped, rather than silently refining another slot.
	if cythera := p.Char.Equip[1].Index; cythera >= 3500 && cythera <= 3507 {
		resend()
		s.Send(wire.MessagePanel("Cythera cannot be refined by this quest."))
		return
	}

	level := rule.RefineMax
	if level <= 0 || level > 15 {
		level = 6
	}
	snapshot := cloneCharacterState(p.Char)
	oldPowder := snapshot.Inv[powderSlot]
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
		resend()
		s.Send(wire.MessagePanel("No armor piece to refine."))
		return
	}
	if rule.OnceQuestID != 0 {
		markQuestCompleted(p.Char, int(rule.OnceQuestID))
	}
	if rule.Consume {
		consumeOne(powder)
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*p.Char = snapshot
		log.Printf("[#%d] ERRO ao salvar refine_set: %v", s.ID, err)
		resend()
		return
	}
	w.recalcPlayer(p.Char)
	resend()
	// Reenvia CADA peca de armadura equipada individualmente: e o SendItem por
	// slot (nao o SelfEquip do array inteiro) que faz o client redesenhar o brilho
	// do refino sem precisar desequipar/equipar. Mesmo padrao do refino individual.
	for slot := 1; slot <= 5; slot++ {
		if p.Char.Equip[slot].Index != 0 {
			s.Send(wire.SendItem(p.ID, placeEquip, byte(slot), p.Char.Equip[slot]))
		}
	}
	s.Send(playerScorePacket(p))
	w.syncPlayerVitalsToObservers(p)
	// O brilho/mesh do refino viaja no UpdateEquip incremental.
	w.refreshAppearance(p)
	s.Send(wire.MessagePanel("Set refined!"))
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

	// Ori/Lac sobre o OVO equipado inicia/avanca a incubacao em vez de refinar.
	// O slot 14 e obrigatorio e tambem governa o contador de horas online.
	if egg, eggType, eggSlot := w.destEggTarget(p, req); egg != nil {
		w.incubateEgg(p, s, powder, powderSlot, egg, eggType, eggSlot, code)
		return
	}

	dest, destType, destPos := w.destRefineTarget(p, req)
	if dest == nil {
		// Alvo nao e um slot de equip: refino e somente em item equipado.
		resend()
		s.Send(wire.MessagePanel("Equip the item to refine it."))
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
		s.Send(wire.MessagePanel("That item cannot be refined."))
		return
	}
	sanc := itemSanc(*dest)
	limit := rule.RefineMax
	if limit <= 0 || limit > 15 {
		limit = 15
	}
	if sanc >= limit {
		resend()
		s.Send(wire.MessagePanel("That item cannot be refined any further."))
		return
	}

	oldPowder, oldDest := *powder, *dest
	roll := w.rollPercent(refineChance(sanc))
	success := roll.Success
	if success {
		if !setItemSanc(dest, sanc+1) {
			// Sem par EF_SANC e sem slot livre (item cheio de adds): nao refina e
			// NAO consome a poeira, para nao punir o jogador por um item invalido.
			*dest = oldDest
			resend()
			s.Send(wire.MessagePanel("That item cannot be refined."))
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
		resend()
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "refinement", err)
		return
	}
	resend()
	s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
	if destType == placeEquip {
		w.recalcPlayer(p.Char)
		s.Send(playerScorePacket(p))
		w.syncPlayerVitals(p)
		// O brilho/mesh do refino viaja no UpdateEquip incremental.
		w.refreshAppearance(p)
	}
	s.Send(wire.MessagePanel(roll.message()))
	result := "FALHOU"
	if success {
		result = "OK"
	}
	log.Printf("[#%d] refino code=%d po=%d alvo=%d +%d->%d %s roll=%d/%d",
		s.ID, code, oldPowder.Index, oldDest.Index, sanc, sanc+1, result, roll.Roll, roll.Chance)
}
