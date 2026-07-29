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

// onDeleteItem trata o 0x2E4 nativo. O pacote apenas identifica um slot do
// Carry e repete o Index que o client acredita existir ali; o servidor exige
// que os dois coincidam antes de apagar e persiste antes de confirmar.
func (w *World) onDeleteItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil ||
		len(pkt) != deleteItemPacketSize || playerCurHP(p.Char) == 0 {
		return
	}
	slotRaw := binary.LittleEndian.Uint32(pkt[12:16])
	indexRaw := binary.LittleEndian.Uint32(pkt[16:20])
	if slotRaw >= model.PlayerCarrySlots || indexRaw == 0 || indexRaw > uint32(^uint16(0)) {
		w.recordSecurityViolation(s, wire.OpDeleteItem, "slot/index de inventario invalido")
		return
	}
	slot := int(slotRaw)
	current := p.Char.Inv[slot]
	if current.Index == 0 || current.Index != uint16(indexRaw) {
		w.recordSecurityViolation(s, wire.OpDeleteItem, "item informado diverge do inventario")
		s.Send(wire.SendItem(p.ID, placeInv, byte(slot), current))
		return
	}

	w.cancelTrade(p, "inventory changed")
	p.Char.Inv[slot] = model.Item{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = current
		s.Send(wire.SendItem(p.ID, placeInv, byte(slot), current))
		log.Printf("[#%d] apagar item slot=%d uid=%q: %v", s.ID, slot, current.UID, err)
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(slot), model.Item{}))
	log.Printf("[#%d] apagou item=%d slot=%d uid=%q", s.ID, current.Index, slot, current.UID)
}

// itemAmountEffectSlot localiza EF_AMOUNT no estado real da instancia. Apenas
// uma pilha que ja carrega esse efeito pode ser separada; o Index enviado pelo
// client nunca transforma um item comum em empilhavel.
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

// onSplitItem trata o 0x2E5 nativo. A nova pilha preserva Index e efeitos,
// recebe UID proprio e ocupa somente um slot visivel livre. A alteracao das
// duas pilhas e confirmada apenas depois do save anti-dupe.
func (w *World) onSplitItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil ||
		len(pkt) != splitItemPacketSize || playerCurHP(p.Char) == 0 {
		return
	}
	slotRaw := binary.LittleEndian.Uint32(pkt[12:16])
	indexRaw := binary.LittleEndian.Uint32(pkt[16:20])
	amount := binary.LittleEndian.Uint32(pkt[20:24])
	if slotRaw >= model.PlayerCarrySlots || indexRaw == 0 || indexRaw > uint32(^uint16(0)) ||
		amount == 0 || amount > 255 {
		w.recordSecurityViolation(s, wire.OpSplitItem, "parametros de pilha invalidos")
		return
	}
	sourceSlot := int(slotRaw)
	source := p.Char.Inv[sourceSlot]
	total := itemStackAmount(source)
	if source.Index != uint16(indexRaw) || itemAmountEffectSlot(source) < 0 ||
		total <= 1 || amount >= total {
		w.recordSecurityViolation(s, wire.OpSplitItem, "pilha informada diverge do inventario")
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
		log.Printf("[#%d] gerar UID ao separar item=%d: %v", s.ID, source.Index, err)
		return
	}
	remaining := total - amount
	if !setExactItemAmount(&source, remaining) {
		return
	}

	w.cancelTrade(p, "inventory changed")
	oldSource, oldDest := p.Char.Inv[sourceSlot], p.Char.Inv[destSlot]
	p.Char.Inv[sourceSlot], p.Char.Inv[destSlot] = source, split
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[sourceSlot], p.Char.Inv[destSlot] = oldSource, oldDest
		s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), oldSource))
		s.Send(wire.SendItem(p.ID, placeInv, byte(destSlot), oldDest))
		log.Printf("[#%d] separar item=%d slot=%d: %v", s.ID, source.Index, sourceSlot, err)
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(sourceSlot), source))
	s.Send(wire.SendItem(p.ID, placeInv, byte(destSlot), split))
	log.Printf("[#%d] separou item=%d %d+%d slots=%d/%d",
		s.ID, source.Index, remaining, amount, sourceSlot, destSlot)
}
