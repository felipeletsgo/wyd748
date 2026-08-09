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

// useEquipmentGem porta o bloco Gemas do UseItem 7.54. Diamond/Emerald/Coral/
// Garnet sao variantes 0..3 do valor especial de refino +10..+15. Armaduras
// preservam o Index; armas mudam para o item contiguo da mesma familia.
func (w *World) useEquipmentGem(s *net.Session, p *Player, source *model.Item,
	sourceSlot byte, rule model.VolatileRule, req useItemRequest) {
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
	// O W2PP permite a familia de armas Ancient (Grade 5..8) mesmo abaixo
	// de +10: nesse caso somente o Index da variante muda e o refino comum e
	// preservado. Armaduras exigem o intervalo especial +10..+15.
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

	snapshot := cloneCharacterState(p.Char)
	oldSource, oldTarget := *source, *target
	target.Index = newIndex // UID e os adicionais pertencem a mesma instancia.
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
	// A conta persistida precisa conter o score derivado do novo equipamento;
	// recalcular depois do commit criaria uma janela de estado dividido.
	w.recalcPlayer(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		resend("")
		log.Printf("[#%d] ERRO ao salvar gema item=%d: %v", s.ID, oldSource.Index, err)
		return
	}

	s.Send(wire.SendItem(p.ID, placeInv, sourceSlot, *source))
	s.Send(wire.SendItem(p.ID, placeEquip, byte(req.dstPos), *target))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	w.syncPlayerVitalsToObservers(p)
	w.refreshAppearance(p)
	w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
	s.Send(wire.MessagePanel("Gem applied successfully."))
	log.Printf("[#%d] gema item=%d aplicada alvo=%d->%d refino_raw=%d", s.ID,
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

// useOreUpgrade porta a familia 575..578. Adamantita e a variante 3 e usa o
// Extra do itemlist para chegar ao equipamento Lendario. Falha tambem consome
// o minerio; alvo invalido nao consome nada.
func (w *World) useOreUpgrade(s *net.Session, p *Player, source *model.Item,
	sourceSlot byte, rule model.VolatileRule, req useItemRequest) {
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
	// Extra nao e um bonus numerico: ele aponta para OUTRO item do catalogo,
	// a variante Legend/Le da mesma familia. Trave a relacao para um itemlist
	// adulterado nao transformar uma armadura em outro equipamento arbitrario.
	if !exists || result.Pos != def.Pos || result.Grade != 4 || result.Unique != def.Unique {
		resend(target, targetType, targetPos, "The Legendary result is missing from the server catalog.")
		return
	}

	snapshot := cloneCharacterState(p.Char)
	oldSource, oldTarget := *source, *target
	roll := w.rollPercent(rule.SuccessPercent)
	success := roll.Success
	consumeOne(source)
	if success {
		target.Index = uint16(def.Extra) // preserva UID, refino e adicionais.
	}
	if targetType == placeEquip {
		w.recalcPlayer(p.Char)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		resend(target, targetType, targetPos, "")
		log.Printf("[#%d] ERRO ao salvar Adamantita item=%d: %v", s.ID, oldSource.Index, err)
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "adamantite upgrade", err)
		return
	}

	resend(target, targetType, targetPos, "")
	if success {
		if targetType == placeEquip {
			s.Send(wire.UpdateScore(p.ID, *p.Char))
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
	log.Printf("[#%d] Adamantita alvo=%d resultado=%d sucesso=%v roll=%d/%d",
		s.ID, oldTarget.Index, target.Index, success, roll.Roll, roll.Chance)
}
