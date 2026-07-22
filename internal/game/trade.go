package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	maxTradeItems  = 15
	tradeInviteTTL = 30 * time.Second
	tradeActiveTTL = 5 * time.Minute
)

// TradeState existe somente em RAM. Os itens continuam no inventario ate os
// dois jogadores confirmarem; CarryPos + Items formam um snapshot anti-dupe.
type TradeState struct {
	OpponentID uint16
	Items      [maxTradeItems]model.Item
	CarryPos   [maxTradeItems]int8
	Gold       uint32
	Checked    bool
	ExpiresAt  time.Time
}

type tradeRequest struct {
	OpponentID uint16
	Items      [maxTradeItems]model.Item
	CarryPos   [maxTradeItems]int8
	Gold       uint32
	Checked    bool
}

func emptyTradePositions() [maxTradeItems]int8 {
	var positions [maxTradeItems]int8
	for i := range positions {
		positions[i] = -1
	}
	return positions
}

func parseTradeRequest(pkt []byte, ch *model.Char) (tradeRequest, error) {
	var req tradeRequest
	req.CarryPos = emptyTradePositions()
	if len(pkt) != 156 {
		return req, fmt.Errorf("tamanho %d, esperado 156", len(pkt))
	}
	req.OpponentID = binary.LittleEndian.Uint16(pkt[154:156])
	if req.OpponentID == 0 {
		return req, fmt.Errorf("oponente vazio")
	}
	signedGold := int32(binary.LittleEndian.Uint32(pkt[148:152]))
	if signedGold < 0 || uint32(signedGold) > maxCharacterGold || uint32(signedGold) > ch.Gold {
		return req, fmt.Errorf("gold invalido %d", signedGold)
	}
	req.Gold = uint32(signedGold)
	if pkt[152] > 1 {
		return req, fmt.Errorf("confirmacao invalida %d", pkt[152])
	}
	req.Checked = pkt[152] == 1

	used := make(map[int]struct{}, maxTradeItems)
	for i := 0; i < maxTradeItems; i++ {
		rawPos := pkt[132+i]
		packetItem := decodeTradeItem(pkt[12+i*8 : 20+i*8])
		if rawPos == 0xFF {
			if packetItem.Index != 0 {
				return req, fmt.Errorf("item sem posicao no slot %d", i)
			}
			continue
		}
		pos := int(rawPos)
		if pos < 0 || pos >= model.PlayerCarrySlots {
			return req, fmt.Errorf("posicao de inventario %d invalida", pos)
		}
		if _, duplicate := used[pos]; duplicate {
			return req, fmt.Errorf("posicao de inventario %d duplicada", pos)
		}
		used[pos] = struct{}{}
		item := ch.Inv[pos]
		if item.Index == 0 || item != packetItem {
			return req, fmt.Errorf("item do slot %d diverge do inventario", pos)
		}
		req.CarryPos[i] = int8(pos)
		req.Items[i] = item
	}
	return req, nil
}

func decodeTradeItem(b []byte) model.Item {
	var item model.Item
	item.Index = binary.LittleEndian.Uint16(b[:2])
	copy(item.Eff[:], b[2:8])
	return item
}

func tradeOfferEqual(state *TradeState, req tradeRequest) bool {
	return state != nil && state.Items == req.Items && state.CarryPos == req.CarryPos && state.Gold == req.Gold
}

func updateTradeOffer(state, opponent *TradeState, req tradeRequest) error {
	changed := !tradeOfferEqual(state, req)
	if req.Checked && changed {
		return errors.New("oferta alterada durante confirmacao")
	}
	wasChecked := state.Checked
	state.Items = req.Items
	state.CarryPos = req.CarryPos
	state.Gold = req.Gold
	if changed || (!req.Checked && wasChecked) {
		state.Checked = false
		opponent.Checked = false
	}
	if !req.Checked {
		state.Checked = false
	}
	return nil
}

func (w *World) tradeRequestValid(p, opponent *Player) bool {
	return p != nil && opponent != nil && p != opponent && p.InWorld && opponent.InWorld &&
		p.Char != nil && opponent.Char != nil && playerCurHP(p.Char) > 0 && playerCurHP(opponent.Char) > 0 &&
		inView(p.X, p.Y, opponent.X, opponent.Y)
}

func (w *World) onTrade(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	p.BrowsingGhostShopID = 0
	req, err := parseTradeRequest(pkt, p.Char)
	if err != nil {
		log.Printf("[#%d] TRADE pacote rejeitado: %v", s.ID, err)
		w.cancelTrade(p, "pacote invalido")
		return
	}
	opponent := w.playerByID(req.OpponentID)
	if !w.tradeRequestValid(p, opponent) {
		s.Send(wire.MessagePanel("Jogador indisponivel ou fora de alcance."))
		w.cancelTrade(p, "oponente indisponivel")
		return
	}

	if p.Trade == nil {
		// Aceite: o outro jogador possui um convite pendente apontando para p.
		if opponent.Trade != nil && opponent.Trade.OpponentID == p.ID {
			p.ShopNPC, opponent.ShopNPC = 0, 0
			p.Trade = &TradeState{
				OpponentID: opponent.ID,
				CarryPos:   emptyTradePositions(),
				ExpiresAt:  time.Now().Add(tradeActiveTTL),
			}
			opponent.Trade.ExpiresAt = p.Trade.ExpiresAt
			opponent.Session.Send(wire.Trade(opponent.ID, p.Trade.Items, p.Trade.CarryPos, 0, false, p.ID))
			log.Printf("[#%d] TRADE aceito: %s(%d) <-> %s(%d)", s.ID,
				p.Char.Name, p.ID, opponent.Char.Name, opponent.ID)
			return
		}
		if opponent.Trade != nil {
			s.Send(wire.MessagePanel("Esse jogador ja esta negociando."))
			return
		}
		// Convite inicial nunca aceita itens/gold/check embutidos.
		if req.Gold != 0 || req.Checked || req.Items != [maxTradeItems]model.Item{} {
			log.Printf("[#%d] TRADE convite com oferta embutida rejeitado", s.ID)
			return
		}
		p.Trade = &TradeState{
			OpponentID: opponent.ID,
			CarryPos:   emptyTradePositions(),
			ExpiresAt:  time.Now().Add(tradeInviteTTL),
		}
		p.ShopNPC = 0
		opponent.Session.Send(wire.Trade(opponent.ID, p.Trade.Items, p.Trade.CarryPos, 0, false, p.ID))
		log.Printf("[#%d] TRADE convite: %s(%d) -> %s(%d)", s.ID,
			p.Char.Name, p.ID, opponent.Char.Name, opponent.ID)
		return
	}

	if p.Trade.OpponentID != opponent.ID || opponent.Trade == nil || opponent.Trade.OpponentID != p.ID {
		w.cancelTrade(p, "sessao inconsistente")
		return
	}
	if err := w.validateTradableItems(req.Items); err != nil {
		s.Send(wire.MessagePanel(err.Error()))
		w.cancelTrade(p, "item nao negociavel")
		return
	}

	if err := updateTradeOffer(p.Trade, opponent.Trade, req); err != nil {
		w.cancelTrade(p, "oferta alterada durante confirmacao")
		return
	}
	p.Trade.ExpiresAt = time.Now().Add(tradeActiveTTL)
	opponent.Trade.ExpiresAt = p.Trade.ExpiresAt

	if !req.Checked {
		opponent.Session.Send(wire.Trade(opponent.ID, p.Trade.Items, p.Trade.CarryPos,
			p.Trade.Gold, false, p.ID))
		return
	}

	p.Trade.Checked = true
	s.Send(wire.CNFTradeCheck(p.ID))
	opponent.Session.Send(wire.Trade(opponent.ID, p.Trade.Items, p.Trade.CarryPos,
		p.Trade.Gold, true, p.ID))
	if opponent.Trade.Checked {
		w.commitTrade(p, opponent)
	}
}

func (w *World) validateTradableItems(items [maxTradeItems]model.Item) error {
	for _, item := range items {
		if item.Index == 0 {
			continue
		}
		if err := w.validateTradableItem(item); err != nil {
			return err
		}
	}
	return nil
}

func (w *World) validateTradableItem(item model.Item) error {
	def, ok := w.items[item.Index]
	if !ok {
		return fmt.Errorf("Item inexistente nao pode ser negociado.")
	}
	if itemAbility(item, def, "EF_NOTRADE") != 0 {
		return fmt.Errorf("Esse item nao pode ser negociado.")
	}
	return nil
}

func buildTradeInventory(ch *model.Char, outgoing, incoming *TradeState) ([64]model.Item, bool) {
	result := ch.Inv
	for i, pos := range outgoing.CarryPos {
		if pos < 0 {
			if outgoing.Items[i].Index != 0 {
				return result, false
			}
			continue
		}
		slot := int(pos)
		if slot >= model.PlayerCarrySlots || result[slot] != outgoing.Items[i] || result[slot].Index == 0 {
			return result, false
		}
		result[slot] = model.Item{}
	}
	for _, item := range incoming.Items {
		if item.Index == 0 {
			continue
		}
		slot := -1
		for i := 0; i < model.PlayerCarrySlots; i++ {
			if result[i].Index == 0 {
				slot = i
				break
			}
		}
		if slot < 0 {
			return result, false
		}
		result[slot] = item
	}
	return result, true
}

func tradeGold(current uint32, outgoing, incoming uint32) (uint32, bool) {
	if current > maxCharacterGold || outgoing > current {
		return current, false
	}
	remaining := current - outgoing
	if incoming > maxCharacterGold-remaining {
		return current, false
	}
	return remaining + incoming, true
}

func (w *World) commitTrade(a, b *Player) {
	if a == nil || b == nil || a.Trade == nil || b.Trade == nil ||
		!a.Trade.Checked || !b.Trade.Checked || a.Trade.OpponentID != b.ID || b.Trade.OpponentID != a.ID {
		return
	}
	aInv, aOK := buildTradeInventory(a.Char, a.Trade, b.Trade)
	bInv, bOK := buildTradeInventory(b.Char, b.Trade, a.Trade)
	aGold, aGoldOK := tradeGold(a.Char.Gold, a.Trade.Gold, b.Trade.Gold)
	bGold, bGoldOK := tradeGold(b.Char.Gold, b.Trade.Gold, a.Trade.Gold)
	if !aOK || !bOK || !aGoldOK || !bGoldOK {
		a.Session.Send(wire.MessagePanel("Sem espaco, gold invalido ou oferta alterada."))
		b.Session.Send(wire.MessagePanel("Sem espaco, gold invalido ou oferta alterada."))
		w.cancelTrade(a, "validacao final falhou")
		return
	}

	oldAInv, oldBInv := a.Char.Inv, b.Char.Inv
	oldAGold, oldBGold := a.Char.Gold, b.Char.Gold
	a.Char.Inv, b.Char.Inv = aInv, bInv
	a.Char.Gold, b.Char.Gold = aGold, bGold
	if err := w.saveTradeAccounts(a.Account, b.Account); err != nil {
		a.Char.Inv, b.Char.Inv = oldAInv, oldBInv
		a.Char.Gold, b.Char.Gold = oldAGold, oldBGold
		log.Printf("TRADE salvar contas %q/%q: %v", a.Account.Name, b.Account.Name, err)
		a.Session.Send(wire.MessagePanel("Falha ao salvar a troca. Nada foi alterado."))
		b.Session.Send(wire.MessagePanel("Falha ao salvar a troca. Nada foi alterado."))
		w.cancelTrade(a, "falha de persistencia")
		return
	}

	aOfferGold, bOfferGold := a.Trade.Gold, b.Trade.Gold
	a.Trade, b.Trade = nil, nil
	a.Session.Send(wire.UpdateCarry(a.ID, a.Char.Inv[:], a.Char.Gold))
	b.Session.Send(wire.UpdateCarry(b.ID, b.Char.Inv[:], b.Char.Gold))
	log.Printf("TRADE concluido: %s(%d gold) <-> %s(%d gold)",
		a.Char.Name, aOfferGold, b.Char.Name, bOfferGold)
}

type tradeBatchStore interface {
	SaveAccounts(accounts ...*model.Account) error
}

func (w *World) saveTradeAccounts(accounts ...*model.Account) error {
	for _, account := range accounts {
		pinAccountEntryPositions(account)
	}
	if batch, ok := w.store.(tradeBatchStore); ok {
		return batch.SaveAccounts(accounts...)
	}
	for _, account := range accounts {
		if err := w.saveAccount(account); err != nil {
			return err
		}
	}
	return nil
}

func (w *World) onCloseTrade(s *net.Session) {
	if p := w.players[s]; p != nil {
		p.BrowsingGhostShopID = 0
		w.cancelTrade(p, "cancelado pelo jogador")
	}
}

func (w *World) cancelTrade(p *Player, reason string) {
	if p == nil || p.Trade == nil {
		return
	}
	opponentID := p.Trade.OpponentID
	p.Trade = nil
	if p.InWorld {
		p.Session.Send(wire.CloseTrade(p.ID))
	}
	if opponent := w.playerByID(opponentID); opponent != nil && opponent.Trade != nil &&
		opponent.Trade.OpponentID == p.ID {
		opponent.Trade = nil
		if opponent.InWorld {
			opponent.Session.Send(wire.CloseTrade(opponent.ID))
		}
	}
	log.Printf("TRADE cancelado para player=%d: %s", p.ID, reason)
}

func (w *World) tickTrades(now time.Time) {
	for _, p := range w.players {
		if p.Trade != nil && !p.Trade.ExpiresAt.IsZero() && !now.Before(p.Trade.ExpiresAt) {
			w.cancelTrade(p, "tempo esgotado")
		}
	}
}
