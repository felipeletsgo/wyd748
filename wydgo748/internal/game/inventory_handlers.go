package game

import (
	"encoding/binary"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	deleteItemPacketSize = 20
	splitItemPacketSize  = 24
)

// onDeleteItem handles native 0x2E4. The packet identifies a Carry slot and
// repeats the client's expected Index. Both must match authoritative state;
// persist the deletion before confirming it.
func (w *World) onDeleteItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil ||
		len(pkt) != deleteItemPacketSize || playerCurHP(p.Char) == 0 ||
		w.shuttingDown || p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) {
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	slotRaw := binary.LittleEndian.Uint32(pkt[12:16])
	indexRaw := binary.LittleEndian.Uint32(pkt[16:20])
	if slotRaw >= model.PlayerCarrySlots || indexRaw == 0 || indexRaw > uint32(^uint16(0)) {
		w.recordSecurityViolation(s, wire.OpDeleteItem, "invalid inventory slot/index")
		return
	}
	slot := int(slotRaw)
	current := p.Char.Inv[slot]
	if current.Index == 0 || current.Index != uint16(indexRaw) {
		w.recordSecurityViolation(s, wire.OpDeleteItem, "requested item differs from inventory")
		s.Send(wire.SendItem(p.ID, placeInv, byte(slot), current))
		return
	}
	if _, filled := model.CelestialSealID(current); filled {
		s.Send(wire.MessagePanel("A filled Spirit's Seal cannot be deleted."))
		s.Send(wire.SendItem(p.ID, placeInv, byte(slot), current))
		return
	}

	w.cancelTrade(p, "inventory changed")
	if w.criticalWrites != nil {
		if err := w.queueInventorySlotChanges(p, "delete", criticalInventorySlotChange{slot, model.Item{}}); err != nil {
			s.Send(wire.SendItem(p.ID, placeInv, byte(slot), current))
			log.Printf("[#%d] submit item deletion slot=%d uid=%q: %v", s.ID, slot, current.UID, err)
		}
		return
	}
	p.Char.Inv[slot] = model.Item{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = current
		s.Send(wire.SendItem(p.ID, placeInv, byte(slot), current))
		log.Printf("[#%d] delete item slot=%d uid=%q: %v", s.ID, slot, current.UID, err)
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(slot), model.Item{}))
	log.Printf("[#%d] deleted item=%d slot=%d uid=%q", s.ID, current.Index, slot, current.UID)
}

// itemAmountEffectSlot locates EF_AMOUNT in the real item instance. Only an
// existing stack can be split; the client Index cannot make an item stackable.
func itemAmountEffectSlot(item model.Item) int {
	for effect := 0; effect < 3; effect++ {
		if item.Eff[effect*2] == effectAmount {
			return effect
		}
	}
	return -1
}

func setExactItemAmount(item *model.Item, amount uint32) bool {
	if item == nil || amount == 0 || amount > 255 {
		return false
	}
	effect := itemAmountEffectSlot(*item)
	if effect < 0 {
		return false
	}
	item.Eff[effect*2+1] = byte(amount)
	return true
}

// onSplitItem handles native 0x2E5. The new stack preserves Index and effects,
// receives its own UID and occupies a free visible slot. Confirm both changed
// stacks only after the anti-duplication save.
func (w *World) onSplitItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil ||
		len(pkt) != splitItemPacketSize || playerCurHP(p.Char) == 0 ||
		w.shuttingDown || p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) {
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	slotRaw := binary.LittleEndian.Uint32(pkt[12:16])
	indexRaw := binary.LittleEndian.Uint32(pkt[16:20])
	amount := binary.LittleEndian.Uint32(pkt[20:24])
	if slotRaw >= model.PlayerCarrySlots || indexRaw == 0 || indexRaw > uint32(^uint16(0)) ||
		amount == 0 || amount > 255 {
		w.recordSecurityViolation(s, wire.OpSplitItem, "invalid stack parameters")
		return
	}
	sourceSlot := int(slotRaw)
	source := p.Char.Inv[sourceSlot]
	total := itemStackAmount(source)
	if source.Index != uint16(indexRaw) || itemAmountEffectSlot(source) < 0 ||
		total <= 1 || amount >= total {
		w.recordSecurityViolation(s, wire.OpSplitItem, "requested stack differs from inventory")
		s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), source))
		return
	}
	destSlot := firstFreeVisibleInventorySlot(p.Char)
	if destSlot < 0 {
		s.Send(wire.MessagePanel("Your inventory is full."))
		s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), source))
		return
	}

	split := source
	split.UID = ""
	if !setExactItemAmount(&split, amount) {
		return
	}
	var err error
	split, err = materializeItem(split)
	if err != nil {
		log.Printf("[#%d] generate split item UID item=%d: %v", s.ID, source.Index, err)
		return
	}
	remaining := total - amount
	if !setExactItemAmount(&source, remaining) {
		return
	}

	w.cancelTrade(p, "inventory changed")
	oldSource, oldDest := p.Char.Inv[sourceSlot], p.Char.Inv[destSlot]
	if w.criticalWrites != nil {
		if err := w.queueInventorySlotChanges(p, "split", criticalInventorySlotChange{sourceSlot, source},
			criticalInventorySlotChange{destSlot, split}); err != nil {
			s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), oldSource))
			s.Send(wire.SendItem(p.ID, placeInv, byte(destSlot), oldDest))
			log.Printf("[#%d] submit item split item=%d slot=%d: %v", s.ID, source.Index, sourceSlot, err)
		}
		return
	}
	p.Char.Inv[sourceSlot], p.Char.Inv[destSlot] = source, split
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[sourceSlot], p.Char.Inv[destSlot] = oldSource, oldDest
		s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), oldSource))
		s.Send(wire.SendItem(p.ID, placeInv, byte(destSlot), oldDest))
		log.Printf("[#%d] split item=%d slot=%d: %v", s.ID, source.Index, sourceSlot, err)
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), source))
	s.Send(wire.SendItem(p.ID, placeInv, byte(destSlot), split))
	log.Printf("[#%d] split item=%d %d+%d slots=%d/%d",
		s.ID, source.Index, remaining, amount, sourceSlot, destSlot)
}
