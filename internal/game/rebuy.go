package game

import (
	"encoding/binary"
	"log"
	"math"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	maxRebuyEntries      = 10
	repurchasePacketSize = 176 // header + target + 10 * STRUCT_REPURCHASEITEM
)

// RebuyEntry e a entrada efemera da lixeira do mercador. Price e guardado
// para a apresentacao; o custo efetivo da recompra e recalculado pelo itemlist
// server-side, como no _MSG_Buy nativo.
type RebuyEntry struct {
	Order uint32
	Item  model.Item
	Price uint32
}

func rebuyEntriesForWire(entries [maxRebuyEntries]RebuyEntry) [10]wire.RepurchaseEntry {
	var out [10]wire.RepurchaseEntry
	for i, entry := range entries {
		out[i] = wire.RepurchaseEntry{Order: entry.Order, Item: entry.Item, Price: entry.Price}
	}
	return out
}

// addRebuy insere o item no primeiro espaco livre; quando a lixeira esta cheia
// a entrada mais antiga sai, exatamente como GetRecycleBin do W2PP.
func (p *Player) addRebuy(item model.Item, price uint32) {
	if p == nil || item.Index == 0 {
		return
	}
	entry := RebuyEntry{Item: item, Price: price}
	for i := range p.Rebuy {
		if p.Rebuy[i].Item.Index == 0 {
			entry.Order = uint32(i)
			p.Rebuy[i] = entry
			return
		}
	}
	copy(p.Rebuy[:maxRebuyEntries-1], p.Rebuy[1:])
	entry.Order = maxRebuyEntries - 1
	p.Rebuy[maxRebuyEntries-1] = entry
}

func (w *World) sendRebuyList(p *Player) {
	if p == nil || p.Session == nil {
		return
	}
	p.Session.Send(wire.RepurchaseList(p.ID, uint32(p.ShopNPC), rebuyEntriesForWire(p.Rebuy)))
}

// onRebuyRequest responde ao clique da janela de recompra. O pacote do
// client 7.48 pode vir completo ou apenas com o header; em ambos os casos o
// target informado nunca autoriza uma loja remota.
func (w *World) onRebuyRequest(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || playerCurHP(p.Char) == 0 ||
		(len(pkt) != wire.HeaderSize && len(pkt) != repurchasePacketSize) {
		return
	}
	w.cancelTrade(p, "recompra")
	if p.ShopNPC == 0 {
		s.Send(wire.MessagePanel("Open a merchant before viewing repurchases."))
		return
	}
	if _, err := w.resolveNPCInteraction(p, p.ShopNPC); err != nil {
		s.Send(wire.MessagePanel("The merchant is no longer available."))
		return
	}
	header := wire.ParseHeader(pkt)
	if header.ID != p.ID {
		w.recordSecurityViolation(s, wire.OpRebuy, "ID do pedido de recompra diverge da sessao")
		return
	}
	if len(pkt) >= 16 {
		target := binary.LittleEndian.Uint32(pkt[12:16])
		if target != 0 && target != uint32(p.ShopNPC) && target != uint32(p.ID) {
			w.recordSecurityViolation(s, wire.OpRebuy, "alvo de recompra fora da loja aberta")
			return
		}
	}
	w.sendRebuyList(p)
	log.Printf("[#%d] recompra aberta (%d itens)", s.ID, countRebuy(p))
}

func countRebuy(p *Player) int {
	if p == nil {
		return 0
	}
	count := 0
	for _, entry := range p.Rebuy {
		if entry.Item.Index != 0 {
			count++
		}
	}
	return count
}

func rebuyPrice(w *World, item model.Item) (uint32, bool) {
	if w == nil || item.Index == 0 {
		return 0, false
	}
	def, ok := w.items[item.Index]
	if !ok || def.Price == 0 {
		return 0, false
	}
	amount := itemStackAmount(item)
	if amount == 0 || uint64(def.Price) > math.MaxUint32/uint64(amount) {
		return 0, false
	}
	return def.Price * amount, true
}

// onRebuyPurchase compra uma entrada da lixeira. O client reutiliza o mesmo
// 0x379 da loja; TargetID==o proprio jogador distingue o fluxo sem confiar no
// indice que o pacote tenta vender.
func (w *World) onRebuyPurchase(s *net.Session, p *Player, pkt []byte, targetPos uint16) {
	if p == nil || p.Char == nil || len(pkt) != 24 {
		return
	}
	index, ok := shopSlotFromClient(targetPos)
	if !ok || index < 0 || index >= maxRebuyEntries || p.Rebuy[index].Item.Index == 0 {
		log.Printf("[#%d] recompra invalida slot=%d indice=%d", s.ID, targetPos, index)
		return
	}
	entry := p.Rebuy[index]
	price, ok := rebuyPrice(w, entry.Item)
	if !ok {
		s.Send(wire.MessagePanel("This item is no longer available."))
		return
	}
	if p.Char.Gold < price {
		s.Send(wire.MessagePanel("You do not have enough gold."))
		return
	}
	dst := firstFreeVisibleInventorySlot(p.Char)
	if dst < 0 {
		s.Send(wire.MessagePanel("Your inventory is full."))
		return
	}
	oldItem, oldGold, oldRebuy := p.Char.Inv[dst], p.Char.Gold, p.Rebuy
	p.Char.Inv[dst] = entry.Item
	p.Char.Gold -= price
	p.Rebuy[index] = RebuyEntry{}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[dst], p.Char.Gold, p.Rebuy = oldItem, oldGold, oldRebuy
		log.Printf("[#%d] ERRO ao salvar recompra item=%d: %v", s.ID, entry.Item.Index, err)
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(dst), p.Char.Inv[dst]))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.sendRebuyList(p)
	log.Printf("[#%d] recomprou item %d por %d gold -> inv[%d]", s.ID, entry.Item.Index, price, dst)
}
