package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	// Merchant 42 projects type 10 in the legacy score. The 7.48 client opens
	// native confirmation and sends 0x28B/ClickOk=1 only after "Yes".
	abilityResetMasterMerchant = 42
	abilityResetSapphireCost   = uint32(30)
	abilityResetPremiumItem    = uint16(3336) // Return Of Ability
	abilityResetPerAttribute   = uint32(100)
)

func isAbilityResetMasterNPC(def *model.NPCDef) bool {
	return def != nil && def.Score != nil && def.Score.Merchant == abilityResetMasterMerchant
}

func abilityResetEquipmentEmpty(ch *model.Char) bool {
	if ch == nil {
		return false
	}
	// The face and accessories may remain; helmet through off-hand slots
	// (Equip[1..7]) must be empty for an unambiguous recalculation.
	for slot := 1; slot < 8; slot++ {
		if ch.Equip[slot].Index != 0 {
			return false
		}
	}
	return true
}

func resetDistributedAttributes(ch *model.Char) (uint32, bool) {
	base, ok := naturalStats(ch)
	if !ok || ch == nil || ch.Score == nil {
		return 0, false
	}
	stats := [4]*uint32{
		&ch.Score.Str, &ch.Score.Int, &ch.Score.Dex, &ch.Score.Con,
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

// handleAbilityResetMasterNPC applies MESTREHAB to the authoritative Score.
// Return Of Ability takes priority; otherwise the cost is 30 Sapphires
// supplied individually, in ten-unit packs, or both.
func (w *World) handleAbilityResetMasterNPC(s *net.Session, p *Player, m *Mob, clickOk int32) bool {
	if m == nil || !isAbilityResetMasterNPC(m.Def) {
		return false
	}
	if s == nil || p == nil || p.Char == nil || !p.InWorld {
		return true
	}
	if w.criticalTradeReserved(p) {
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
	if w.criticalWrites != nil {
		if err := w.queueAbilityReset(p, usePremium); err != nil {
			log.Printf("[#%d] attribute reset admission: %v", s.ID, err)
			s.Send(wire.MessagePanel("Attribute reset is unavailable. Please try again later."))
		}
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
		log.Printf("[#%d] save attribute reset at Skill_Master: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. Nothing was consumed."))
		return true
	}

	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(playerScorePacket(p))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitalsToObservers(p)
	w.updatePartyMember(p)
	s.Send(wire.MessagePanel("Attribute points reset successfully."))
	log.Printf("[#%d] Skill_Master reset %d point(s) for %q (premium=%t)",
		s.ID, recovered, p.Char.Name, usePremium)
	return true
}

func (w *World) queueAbilityReset(p *Player, usePremium bool) error {
	if p == nil || p.Account == nil || p.Char == nil || !abilityResetEquipmentEmpty(p.Char) ||
		!bindCriticalPlayer(p).current(w) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	if usePremium != (countInventoryItem(ch, abilityResetPremiumItem) != 0) {
		return errCriticalPersistenceIdentity
	}
	consumed := false
	if usePremium {
		consumed = consumeInventoryItem(ch, abilityResetPremiumItem, 1)
	} else {
		consumed = consumeSapphires(ch, abilityResetSapphireCost)
	}
	if !consumed {
		return errCriticalPersistenceIdentity
	}
	recovered, ok := resetDistributedAttributes(ch)
	if !ok {
		return errCriticalPersistenceIdentity
	}
	w.recalcPlayer(ch)
	inv, str, intelligence, dex, con := ch.Inv, ch.Score.Str, ch.Score.Int, ch.Score.Dex, ch.Score.Con
	return w.queueCriticalAccountMutation(p, proposed, "attribute reset", "Save failed. Nothing was consumed.", func() {
		p.Char.Inv = inv
		p.Char.Score.Str, p.Char.Score.Int, p.Char.Score.Dex, p.Char.Score.Con = str, intelligence, dex, con
		w.recalcPlayer(p.Char)
	}, func() {
		p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
		p.Session.Send(playerScorePacket(p))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerVitalsToObservers(p)
		w.updatePartyMember(p)
		p.Session.Send(wire.MessagePanel("Attribute points reset successfully."))
		log.Printf("[#%d] Skill_Master reset %d point(s) for %q (premium=%t)",
			p.Session.ID, recovered, p.Char.Name, usePremium)
	})
}
