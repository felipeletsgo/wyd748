package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	// Merchant 42 projeta o tipo 10 no score legado. O client 7.48 abre sua
	// confirmação nativa e envia 0x28B/ClickOk=1 somente depois do "Yes".
	abilityResetMasterMerchant = 42
	abilityResetSapphireCost   = uint32(30)
	abilityResetPremiumItem    = uint16(3336) // Return Of Ability
	abilityResetPerAttribute   = uint32(100)
)

func isAbilityResetMasterNPC(def *model.NPCDef) bool {
	return def != nil && def.Extended != nil && def.Extended.Merchant == abilityResetMasterMerchant
}

func abilityResetEquipmentEmpty(ch *model.Char) bool {
	if ch == nil {
		return false
	}
	// Regra nativa: rosto e acessórios podem permanecer; elmo até a segunda
	// mão (Equip[1..7]) precisam estar vazios para o recálculo ser inequívoco.
	for slot := 1; slot < 8; slot++ {
		if ch.Equip[slot].Index != 0 {
			return false
		}
	}
	return true
}

func resetDistributedAttributes(ch *model.Char) (uint32, bool) {
	base, ok := naturalStats(ch)
	if !ok || ch == nil || ch.Extended == nil {
		return 0, false
	}
	stats := [4]*uint32{
		&ch.Extended.Str, &ch.Extended.Int, &ch.Extended.Dex, &ch.Extended.Con,
	}
	var recovered uint32
	for index, stat := range stats {
		natural := uint32(base[index])
		if *stat <= natural {
			*stat = natural
			continue
		}
		amount := *stat - natural
		if amount > abilityResetPerAttribute {
			amount = abilityResetPerAttribute
		}
		*stat -= amount
		recovered += amount
	}
	return recovered, true
}

// handleAbilityResetMasterNPC porta o MESTREHAB do W2PP para o estado
// ExtendedScore autoritativo. Return Of Ability tem prioridade; sem ele, o
// custo é 30 Safiras (avulsas e/ou pacotes de dez).
func (w *World) handleAbilityResetMasterNPC(s *net.Session, p *Player, m *Mob, clickOk int32) bool {
	if m == nil || !isAbilityResetMasterNPC(m.Def) {
		return false
	}
	if s == nil || p == nil || p.Char == nil || !p.InWorld {
		return true
	}
	if clickOk != 1 {
		s.Send(wire.MessagePanel("Resetting attributes requires 30 Sapphires or Return Of Ability."))
		return true
	}
	if !abilityResetEquipmentEmpty(p.Char) {
		s.Send(wire.MessagePanel("Unequip your armor and weapons first."))
		return true
	}
	if _, ok := naturalStats(p.Char); !ok {
		s.Send(wire.MessagePanel("This character class cannot be reset."))
		return true
	}

	usePremium := countInventoryItem(p.Char, abilityResetPremiumItem) != 0
	if !usePremium && sapphireCount(p.Char) < abilityResetSapphireCost {
		s.Send(wire.MessagePanel("You need 30 Sapphires or Return Of Ability."))
		return true
	}

	snapshot := cloneCharacterState(p.Char)
	consumed := false
	if usePremium {
		consumed = consumeInventoryItem(p.Char, abilityResetPremiumItem, 1)
	} else {
		consumed = consumeSapphires(p.Char, abilityResetSapphireCost)
	}
	if !consumed {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("The reset material could not be consumed."))
		return true
	}
	recovered, ok := resetDistributedAttributes(p.Char)
	if !ok {
		*p.Char = snapshot
		s.Send(wire.MessagePanel("This character class cannot be reset."))
		return true
	}
	w.recalcPlayer(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		w.recalcPlayer(p.Char)
		log.Printf("[#%d] salvar reset no Skill_Master: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		return true
	}

	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitalsToObservers(p)
	w.updatePartyMember(p)
	s.Send(wire.MessagePanel("Attribute points reset successfully."))
	log.Printf("[#%d] Skill_Master resetou %d ponto(s) de %q (premium=%t)",
		s.ID, recovered, p.Char.Name, usePremium)
	return true
}
