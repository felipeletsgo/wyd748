package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	jeffiMerchant    = 12
	shamanMerchant   = 13
	circleSlot       = 13
	circleCarrySlots = 60
	circleStackLimit = uint32(120)
)

// Circle services use the recovered 7.59 server rules through the existing
// 7.48 item, score and shortcut packets. Server Merchant is not a UI nibble.
func (w *World) handleCircleServiceNPC(s *net.Session, p *Player, m *Mob) bool {
	if m == nil || m.Def == nil || m.Def.Score == nil {
		return false
	}
	merchant := m.Def.Score.Merchant
	if merchant != jeffiMerchant && merchant != shamanMerchant {
		return false
	}
	if s == nil || s.IsClosed() || p == nil || p.Session != s || !p.InWorld ||
		p.Account == nil || p.Char == nil || p.Char.Score == nil ||
		p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) ||
		p.Char != &p.Account.Chars[p.CharSlot] || playerCurHP(p.Char) == 0 ||
		p.PersistencePoisoned || w.shuttingDown || w.criticalTradeReserved(p) {
		return true
	}
	if merchant == shamanMerchant {
		// The client confirms with localized message 131, then sends ClickOk=0.
		// The recovered server branch likewise does not test the third argument.
		w.useShamanCircle(p)
	} else {
		w.useJeffi(p)
	}
	return true
}

// Keep errors in the client's existing EN/PT message table. The native 7.59
// server's dialogue numbers are a different table and cannot be sent as indices.
func circleServiceMessage(p *Player, index int16) {
	if p.Session != nil && !p.Session.IsClosed() {
		p.Session.Send(wire.MessageIndexed(index - 1000))
	}
}

func (w *World) commitCircleService(p *Player, images []criticalPreimage, operation string, reset bool) {
	// Persist the same canonical point budgets that will be published. Runtime
	// score copies are excluded by the persistence representation.
	w.recalcPlayer(p.Char)
	rejected := func() {
		circleServiceMessage(p, 188) // Failed to save.
		w.resendInventory(p)
	}
	publish := func() {
		w.recalcPlayer(p.Char)
		if !p.Session.IsClosed() {
			w.resendInventory(p)
			p.Session.Send(wire.SendItem(p.ID, placeEquip, circleSlot, p.Char.Equip[circleSlot]))
			p.Session.Send(playerScorePacket(p))
			if reset {
				p.Session.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
			}
		}
		w.refreshAppearance(p)
		w.syncPlayerVitalsToObservers(p)
		w.updatePartyMember(p)
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation(operation, images, publish,
			criticalLiveMutationOptions{reject: rejected}); err != nil {
			log.Printf("[#%d] %s admission: %v", p.ID, operation, err)
			rejected()
		}
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		w.restoreCriticalPreimages(images)
		log.Printf("[#%d] %s save: %v", p.ID, operation, err)
		rejected()
		return
	}
	publish()
}

func (w *World) useJeffi(p *Player) {
	circle := p.Char.Equip[circleSlot]
	var price uint32
	var first uint16
	switch circle.Index {
	case 447:
		price, first = 1_000_000, 448
	case 692:
		price, first = 5_000_000, 693
	}
	if first != 0 {
		if p.Char.Gold < price {
			circleServiceMessage(p, 155)
			return
		}
		result := first + uint16(w.intn(3))
		if _, ok := w.items[result]; !ok {
			circleServiceMessage(p, 190)
			return
		}
		images := criticalPreimages(p)
		p.Char.Equip[circleSlot].Index = result // Preserve effects and identity.
		p.Char.Gold -= price
		w.commitCircleService(p, images, "Jeffi circle creation", false)
		return
	}
	var scraps, powder, stones uint32
	for _, item := range p.Char.Inv[:circleCarrySlots] {
		switch item.Index {
		case 419:
			scraps += itemStackAmount(item)
		case 420:
			powder += itemStackAmount(item)
		case 3327:
			stones++ // Native admission counts occupied slots, not units.
		}
	}
	if scraps < 10 && powder < 10 && stones < 3 {
		circleServiceMessage(p, 274)
		return
	}
	if p.Char.Gold < 1_000_000 {
		circleServiceMessage(p, 155)
		return
	}
	proposed := p.Char.Inv
	conversions := [][3]uint16{{419, 412, 10}, {420, 413, 10}}
	if stones >= 3 {
		variant := uint16(w.intn(3))
		base := uint16(3390)
		if w.intn(2) != 0 {
			base = 3324
		}
		conversions = append(conversions, [3]uint16{3327, base + variant, 3})
	}
	for _, conversion := range conversions {
		if _, ok := w.items[conversion[1]]; !ok {
			circleServiceMessage(p, 190)
			return
		}
		if !convertJeffiIngredients(&proposed, conversion[0], conversion[1], uint32(conversion[2])) {
			circleServiceMessage(p, 1)
			return
		}
	}
	images := criticalPreimages(p)
	p.Char.Inv = proposed
	p.Char.Gold -= 1_000_000
	// Guild-fame wars explicitly disable gold treasury funding. The native
	// auxiliary city counters are not converted into guild or personal fame.
	w.commitCircleService(p, images, "Jeffi ingredient conversion", false)
}

// Convert complete groups, retain the remainder and never use slots 60..63.
// Unlike the recovered helper, insufficient output space rejects the entire
// detached proposal instead of risking loss after ingredients were cleared.
func convertJeffiIngredients(inv *[64]model.Item, source, target uint16, ratio uint32) bool {
	var total uint32
	for _, item := range inv[:circleCarrySlots] {
		if item.Index == source {
			total += itemStackAmount(item)
		}
	}
	if total < ratio {
		return true
	}
	for slot, item := range inv[:circleCarrySlots] {
		if item.Index == source {
			inv[slot] = model.Item{}
		}
	}
	outputs, remainder := total/ratio, total%ratio
	// Only plain matching stacks may receive new units; effects, expiry and
	// activation state on an existing economic instance must remain meaningful.
	for slot := 0; slot < circleCarrySlots && outputs > 0; slot++ {
		item := &inv[slot]
		if item.Index != target || item.ActivatedUnix != 0 || item.ExpiresUnix != 0 || item.PausedUnix != 0 {
			continue
		}
		plain := *item
		for pair := 0; pair < 3; pair++ {
			if plain.Eff[pair*2] == effectAmount {
				plain.Eff[pair*2], plain.Eff[pair*2+1] = 0, 0
			}
		}
		amount := itemStackAmount(*item)
		if plain.Eff != [6]byte{} || amount >= circleStackLimit {
			continue
		}
		added := min(outputs, circleStackLimit-amount)
		setItemStackAmount(item, amount+added)
		outputs -= added
	}
	for _, output := range []struct {
		index  uint16
		amount uint32
	}{{target, outputs}, {source, remainder}} {
		for output.amount > 0 {
			slot := -1
			for i := 0; i < circleCarrySlots; i++ {
				if inv[i].Index == 0 {
					slot = i
					break
				}
			}
			if slot < 0 {
				return false
			}
			amount := min(output.amount, circleStackLimit)
			item := model.Item{Index: output.index}
			setItemAmount(&item, int(amount))
			var err error
			inv[slot], err = materializeItem(item)
			if err != nil {
				return false
			}
			output.amount -= amount
		}
	}
	return true
}

func (w *World) useShamanCircle(p *Player) {
	index := p.Char.Equip[circleSlot].Index
	var variant int
	var recovery uint32
	switch {
	case index >= 448 && index <= 450 && !p.Char.DivineCircleUsed:
		variant, recovery = int(index-448), 50
	case index >= 693 && index <= 695:
		variant, recovery = int(index-693), 100
	default:
		circleServiceMessage(p, 274)
		return
	}
	images := criticalPreimages(p)
	for i, old := range p.Char.Score.Mastery {
		remaining := old - min(old, recovery)
		remaining = min(remaining, uint32(200))
		p.Char.Score.Mastery[i] = remaining
	}
	// The current server derives the unspent pool from level and remaining
	// mastery. Recalculation refunds these points without trusting a saved pool
	// or overflowing it on malformed legacy values.
	p.Char.LearnedSkill &= [3]uint32{0xffffff00, 0xffff00ff, 0xff00ffff}[variant]
	p.Char.SecondaryLearnedSkill &= [3]uint32{0x00000ff0, 0x00000f0f, 0x000000ff}[variant]
	p.Char.Equip[circleSlot] = model.Item{}
	if recovery == 50 {
		p.Char.DivineCircleUsed = true
	} else {
		p.Char.FullDivineCircleUsed = true
	}
	for i := range p.Char.ShortSkill {
		p.Char.ShortSkill[i] = 0xff
	}
	w.commitCircleService(p, images, "Shaman skill reset", true)
}
