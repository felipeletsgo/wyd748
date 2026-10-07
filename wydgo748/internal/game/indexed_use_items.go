package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func sancRaw(item model.Item) (byte, bool) {
	for i := 0; i < 3; i++ {
		effect := item.Eff[i*2]
		if effect == 43 || effect >= 116 && effect <= 125 {
			return item.Eff[i*2+1], true
		}
	}
	return 0, false
}

func gemTargetKind(pos int) (armor, weapon bool) {
	switch pos {
	case 2, 4, 8, 16, 32, 128:
		return true, false
	case 64, 192:
		return false, true
	default:
		return false, false
	}
}

func blockedGemTarget(index uint16) bool {
	return index >= 3500 && index <= 3507 || index >= 631 && index <= 633
}

// Diamond/Emerald/Coral/Garnet select variants 0..3 of the special +10..+15
// refinement value. Armor retains its index; weapons select an adjacent variant.
func (w *World) useEquipmentGem(s *net.Session, p *Player, source *model.Item,
	sourceSlot byte, rule model.VolatileRule, req useItemRequest) {
	if w.criticalTradeReserved(p) {
		return
	}
	resend := func(message string) {
		s.Send(wire.SendItem(p.ID, placeInv, sourceSlot, *source))
		if message != "" {
			s.Send(wire.MessagePanel(message))
		}
	}
	if req.dstType != placeEquip || req.dstPos >= 9 || req.dstPos == 0 {
		resend("Use the gem on equipped +10 gear or an Ancient weapon.")
		return
	}
	target := &p.Char.Equip[req.dstPos]
	def, exists := w.items[target.Index]
	if target.Index == 0 || !exists || blockedGemTarget(target.Index) {
		resend("This item cannot receive that gem.")
		return
	}
	armor, weapon := gemTargetKind(def.Pos)
	if !armor && !weapon {
		resend("This item cannot receive that gem.")
		return
	}
	raw, hasSanc := sancRaw(*target)
	specialRefine := hasSanc && raw >= 230 && raw <= 253
	// Ancient weapons (grades 5..8) also accept gems below +10, preserving
	// ordinary refinement. Armor requires the special +10..+15 interval.
	if armor && !specialRefine {
		resend("Armor must be refined to +10 or higher.")
		return
	}

	newIndex := target.Index
	if weapon {
		if def.Grade < 5 || def.Grade > 8 {
			resend("This weapon cannot receive that gem.")
			return
		}
		candidate := int(target.Index) + rule.Variant + 5 - def.Grade
		if candidate <= 0 || candidate > int(^uint16(0)) {
			resend("The weapon variant is missing from the server catalog.")
			return
		}
		variant, ok := w.items[uint16(candidate)]
		if !ok || variant.Pos != def.Pos || variant.Grade != 5+rule.Variant {
			resend("The weapon variant is missing from the server catalog.")
			return
		}
		newIndex = uint16(candidate)
	}

	if w.criticalWrites != nil {
		proposedTarget := *target
		proposedTarget.Index = newIndex
		if specialRefine && !setItemSancRaw(&proposedTarget, byte(230+((int(raw)-230)/4)*4+rule.Variant)) {
			resend("The equipment has no refinement effect.")
			return
		}
		if err := w.queueCriticalIndexedUse(p, sourceSlot, placeEquip, req.dstPos, proposedTarget, func() {
			s.Send(wire.SendItem(p.ID, placeInv, sourceSlot, p.Char.Inv[sourceSlot]))
			s.Send(wire.SendItem(p.ID, placeEquip, byte(req.dstPos), p.Char.Equip[req.dstPos]))
			s.Send(playerScorePacket(p))
			w.syncPlayerVitalsToObservers(p)
			w.refreshAppearance(p)
			w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
			s.Send(wire.MessagePanel("Gem applied successfully."))
		}); err != nil {
			resend("Item use is unavailable. Please try again later.")
			log.Printf("[#%d] equipment gem admission: %v", s.ID, err)
		}
		return
	}
	snapshot := cloneCharacterState(p.Char)
	oldSource, oldTarget := *source, *target
	target.Index = newIndex // UID and additional effects belong to the same instance.
	newRaw := raw
	if specialRefine {
		newRaw = byte(230 + ((int(raw)-230)/4)*4 + rule.Variant)
		if !setItemSancRaw(target, newRaw) {
			*target = oldTarget
			resend("The equipment has no refinement effect.")
			return
		}
	}
	consumeOne(source)
	// Persist the score derived from the new equipment in the same transaction.
	w.recalcPlayer(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		resend("")
		log.Printf("[#%d] save equipment gem item=%d: %v", s.ID, oldSource.Index, err)
		return
	}

	s.Send(wire.SendItem(p.ID, placeInv, sourceSlot, *source))
	s.Send(wire.SendItem(p.ID, placeEquip, byte(req.dstPos), *target))
	s.Send(playerScorePacket(p))
	w.syncPlayerVitalsToObservers(p)
	w.refreshAppearance(p)
	w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
	s.Send(wire.MessagePanel("Gem applied successfully."))
	log.Printf("[#%d] gem item=%d applied target=%d->%d refinement_raw=%d", s.ID,
		oldSource.Index, oldTarget.Index, target.Index, newRaw)
}

func oreAcceptsUnique(variant, unique int) bool {
	switch variant {
	case 0:
		return unique == 5 || unique == 14 || unique == 24 || unique == 34
	case 1:
		return unique == 6 || unique == 15 || unique == 25 || unique == 35
	case 2:
		return unique == 7 || unique == 16 || unique == 26 || unique == 36
	case 3:
		return unique == 8 || unique == 17 || unique == 27 || unique == 37 ||
			unique == 10 || unique == 20 || unique == 30 || unique == 40
	default:
		return false
	}
}

// The 575..578 ore family uses catalog Extra to select Legendary equipment.
// A failed roll also consumes the ore; an invalid target consumes nothing.
func (w *World) useOreUpgrade(s *net.Session, p *Player, source *model.Item,
	sourceSlot byte, rule model.VolatileRule, req useItemRequest) {
	if w.criticalTradeReserved(p) {
		return
	}
	resend := func(target *model.Item, targetType, targetPos uint32, message string) {
		s.Send(wire.SendItem(p.ID, placeInv, sourceSlot, *source))
		if target != nil {
			s.Send(wire.SendItem(p.ID, byte(targetType), byte(targetPos), *target))
		}
		if message != "" {
			s.Send(wire.MessagePanel(message))
		}
	}
	target, targetType, targetPos := w.destItemTarget(p, req)
	if target == nil || target.Index == 0 || target == source {
		resend(nil, 0, 0, "Use Adamantite on compatible equipment.")
		return
	}
	def, exists := w.items[target.Index]
	if !exists || def.Pos == 0 || def.Grade < 1 || def.Grade > 3 ||
		!oreAcceptsUnique(rule.Variant, def.Unique) || def.Extra <= 0 || def.Extra > int(^uint16(0)) {
		resend(target, targetType, targetPos, "This equipment cannot become Legendary.")
		return
	}
	result, exists := w.items[uint16(def.Extra)]
	// Extra identifies a Legendary variant, not a numeric bonus. Validate the
	// relationship so a tampered catalog cannot select unrelated equipment.
	if !exists || result.Pos != def.Pos || result.Grade != 4 || result.Unique != def.Unique {
		resend(target, targetType, targetPos, "The Legendary result is missing from the server catalog.")
		return
	}

	snapshot := cloneCharacterState(p.Char)
	oldSource, oldTarget := *source, *target
	roll := w.rollPercent(rule.SuccessPercent)
	success := roll.Success
	if w.criticalWrites != nil {
		proposedTarget := oldTarget
		if success {
			proposedTarget.Index = uint16(def.Extra)
		}
		if err := w.queueCriticalIndexedUse(p, sourceSlot, targetType, targetPos, proposedTarget, func() {
			resend(target, targetType, targetPos, "")
			if success {
				if targetType == placeEquip {
					s.Send(playerScorePacket(p))
					w.syncPlayerVitalsToObservers(p)
					w.refreshAppearance(p)
				}
				w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
			} else {
				w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 0) })
			}
			s.Send(wire.MessagePanel(roll.message()))
			if success {
				s.Send(wire.MessagePanel("The equipment became Legendary!"))
			}
		}); err != nil {
			resend(target, targetType, targetPos, "Item use is unavailable. Please try again later.")
			log.Printf("[#%d] ore upgrade admission: %v", s.ID, err)
		}
		return
	}
	consumeOne(source)
	if success {
		target.Index = uint16(def.Extra) // Preserve UID, refinement and additional effects.
	}
	if targetType == placeEquip {
		w.recalcPlayer(p.Char)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		resend(target, targetType, targetPos, "")
		log.Printf("[#%d] save Adamantite item=%d: %v", s.ID, oldSource.Index, err)
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "adamantite upgrade", err)
		return
	}

	resend(target, targetType, targetPos, "")
	if success {
		if targetType == placeEquip {
			s.Send(playerScorePacket(p))
			w.syncPlayerVitalsToObservers(p)
			w.refreshAppearance(p)
		}
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
	} else {
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 0) })
	}
	s.Send(wire.MessagePanel(roll.message()))
	if success {
		s.Send(wire.MessagePanel("The equipment became Legendary!"))
	}
	log.Printf("[#%d] Adamantite target=%d result=%d success=%v roll=%d/%d",
		s.ID, oldTarget.Index, target.Index, success, roll.Roll, roll.Chance)
}
