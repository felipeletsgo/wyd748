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

// TradeState exists only in RAM. Items remain in inventory until both players
// confirm; CarryPos + Items form an anti-duplication snapshot.
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
		return req, fmt.Errorf("packet size %d, expected 156", len(pkt))
	}
	req.OpponentID = binary.LittleEndian.Uint16(pkt[154:156])
	if req.OpponentID == 0 {
		return req, fmt.Errorf("missing opponent")
	}
	signedGold := int32(binary.LittleEndian.Uint32(pkt[148:152]))
	if signedGold < 0 || uint32(signedGold) > maxCharacterGold || uint32(signedGold) > ch.Gold {
		return req, fmt.Errorf("invalid gold amount %d", signedGold)
	}
	req.Gold = uint32(signedGold)
	if pkt[152] > 1 {
		return req, fmt.Errorf("invalid confirmation value %d", pkt[152])
	}
	req.Checked = pkt[152] == 1

	used := make(map[int]struct{}, maxTradeItems)
	for i := 0; i < maxTradeItems; i++ {
		rawPos := pkt[132+i]
		packetItem := decodeTradeItem(pkt[12+i*8 : 20+i*8])
		// Legacy invitations may leave CarryPos at zero even when the matching
		// slot has no item. Empty wire content decides this; a leftover position
		// does not turn the slot into an offer.
		if packetItem.Index == 0 && packetItem.Eff == [6]byte{} {
			continue
		}
		if rawPos == 0xFF {
			return req, fmt.Errorf("item without a position in slot %d", i)
		}
		pos := int(rawPos)
		if pos < 0 || pos >= model.PlayerCarrySlots {
			return req, fmt.Errorf("invalid inventory position %d", pos)
		}
		if _, duplicate := used[pos]; duplicate {
			return req, fmt.Errorf("duplicate inventory position %d", pos)
		}
		used[pos] = struct{}{}
		item := ch.Inv[pos]
		if item.Index == 0 || !item.WireEqual(packetItem) {
			return req, fmt.Errorf("item in slot %d differs from inventory", pos)
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
		return errors.New("offer changed during confirmation")
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
		w.playersShareGameplaySpace(p, opponent) &&
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
		log.Printf("[#%d] TRADE packet rejected: %v", s.ID, err)
		w.cancelTrade(p, "invalid packet")
		return
	}
	opponent := w.playerByID(req.OpponentID)
	if !w.tradeRequestValid(p, opponent) {
		s.Send(wire.MessagePanel("Player unavailable or out of range."))
		w.cancelTrade(p, "opponent unavailable")
		return
	}

	if p.Trade == nil {
		// Acceptance: the other player has a pending invitation to p.
		if opponent.Trade != nil && opponent.Trade.OpponentID == p.ID {
			p.ShopNPC, opponent.ShopNPC = 0, 0
			p.Trade = &TradeState{
				OpponentID: opponent.ID,
				CarryPos:   emptyTradePositions(),
				ExpiresAt:  w.now().Add(tradeActiveTTL),
			}
			opponent.Trade.ExpiresAt = p.Trade.ExpiresAt
			opponent.Session.Send(wire.Trade(opponent.ID, p.Trade.Items, p.Trade.CarryPos, 0, false, p.ID))
			log.Printf("[#%d] TRADE accepted: %s(%d) <-> %s(%d)", s.ID,
				p.Char.Name, p.ID, opponent.Char.Name, opponent.ID)
			return
		}
		if opponent.Trade != nil {
			s.Send(wire.MessagePanel("That player is already trading."))
			return
		}
		// An initial invitation cannot include items, gold, or confirmation.
		if req.Gold != 0 || req.Checked || req.Items != [maxTradeItems]model.Item{} {
			log.Printf("[#%d] TRADE invitation with embedded offer rejected", s.ID)
			return
		}
		p.Trade = &TradeState{
			OpponentID: opponent.ID,
			CarryPos:   emptyTradePositions(),
			ExpiresAt:  w.now().Add(tradeInviteTTL),
		}
		p.ShopNPC = 0
		opponent.Session.Send(wire.Trade(opponent.ID, p.Trade.Items, p.Trade.CarryPos, 0, false, p.ID))
		log.Printf("[#%d] TRADE invitation: %s(%d) -> %s(%d)", s.ID,
			p.Char.Name, p.ID, opponent.Char.Name, opponent.ID)
		return
	}

	if p.Trade.OpponentID != opponent.ID || opponent.Trade == nil || opponent.Trade.OpponentID != p.ID {
		w.cancelTrade(p, "inconsistent session")
		return
	}
	if err := w.validateTradableItems(req.Items); err != nil {
		s.Send(wire.MessagePanel(err.Error()))
		w.cancelTrade(p, "item cannot be traded")
		return
	}
	if err := w.validateTradeCapsules(p.Account, req.Items); err != nil {
		s.Send(wire.MessagePanel(err.Error()))
		w.cancelTrade(p, "invalid Celestial capsule")
		return
	}

	if err := updateTradeOffer(p.Trade, opponent.Trade, req); err != nil {
		w.cancelTrade(p, "offer changed during confirmation")
		return
	}
	p.Trade.ExpiresAt = w.now().Add(tradeActiveTTL)
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
		return clientError("An unknown item cannot be traded.")
	}
	if itemAbility(item, def, "EF_NOTRADE") != 0 {
		return clientError("This item cannot be traded.")
	}
	return nil
}

// validateTradeCapsules ensures that a filled seal offered in trade still
// belongs to the authoritative account aggregate. The client carries only
// STRUCT_ITEM; the server transfers the Celestial snapshot when both accounts
// commit atomically.
func (w *World) validateTradeCapsules(account *model.Account, items [maxTradeItems]model.Item) error {
	seen := make(map[string]struct{})
	for _, item := range items {
		id, filled := model.CelestialSealID(item)
		if !filled {
			continue
		}
		if item.UID == "" {
			return clientError("The Spirit's Seal has no valid identity.")
		}
		if _, duplicate := seen[item.UID]; duplicate {
			return clientError("The same Spirit's Seal was offered more than once.")
		}
		seen[item.UID] = struct{}{}
		capsule, _ := w.capsuleByID(account, id)
		if capsule == nil || capsule.ItemUID != item.UID || !accountOwnsCapsuleSeal(account, capsule) {
			return clientError("The Spirit's Seal data is unavailable.")
		}
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
	if err := w.validateTradeCapsules(a.Account, a.Trade.Items); err != nil {
		a.Session.Send(wire.MessagePanel(err.Error()))
		b.Session.Send(wire.MessagePanel("The other player's Spirit's Seal is invalid."))
		w.cancelTrade(a, "invalid Celestial capsule at final validation")
		return
	}
	if err := w.validateTradeCapsules(b.Account, b.Trade.Items); err != nil {
		b.Session.Send(wire.MessagePanel(err.Error()))
		a.Session.Send(wire.MessagePanel("The other player's Spirit's Seal is invalid."))
		w.cancelTrade(a, "invalid Celestial capsule at final validation")
		return
	}
	aInv, aOK := buildTradeInventory(a.Char, a.Trade, b.Trade)
	bInv, bOK := buildTradeInventory(b.Char, b.Trade, a.Trade)
	aGold, aGoldOK := tradeGold(a.Char.Gold, a.Trade.Gold, b.Trade.Gold)
	bGold, bGoldOK := tradeGold(b.Char.Gold, b.Trade.Gold, a.Trade.Gold)
	if !aOK || !bOK || !aGoldOK || !bGoldOK {
		a.Session.Send(wire.MessagePanel("No space, invalid gold, or the offer changed."))
		b.Session.Send(wire.MessagePanel("No space, invalid gold, or the offer changed."))
		w.cancelTrade(a, "final validation failed")
		return
	}

	oldAInv, oldBInv := a.Char.Inv, b.Char.Inv
	oldAGold, oldBGold := a.Char.Gold, b.Char.Gold
	oldACapsules := append([]model.CelestialCapsule(nil), a.Account.CelestialCapsules...)
	oldBCapsules := append([]model.CelestialCapsule(nil), b.Account.CelestialCapsules...)
	a.Char.Inv, b.Char.Inv = aInv, bInv
	a.Char.Gold, b.Char.Gold = aGold, bGold
	if err := w.transferTradeCapsules(a, b, a.Trade.Items); err != nil {
		a.Char.Inv, b.Char.Inv = oldAInv, oldBInv
		a.Char.Gold, b.Char.Gold = oldAGold, oldBGold
		a.Account.CelestialCapsules = oldACapsules
		b.Account.CelestialCapsules = oldBCapsules
		log.Printf("TRADE transfer capsules %q/%q: %v", a.Account.Name, b.Account.Name, err)
		a.Session.Send(wire.MessagePanel("The Spirit's Seal could not be transferred."))
		b.Session.Send(wire.MessagePanel("The Spirit's Seal could not be transferred."))
		w.cancelTrade(a, "Celestial capsule transfer failed")
		return
	}
	if err := w.transferTradeCapsules(b, a, b.Trade.Items); err != nil {
		a.Char.Inv, b.Char.Inv = oldAInv, oldBInv
		a.Char.Gold, b.Char.Gold = oldAGold, oldBGold
		a.Account.CelestialCapsules = oldACapsules
		b.Account.CelestialCapsules = oldBCapsules
		log.Printf("TRADE transfer capsules %q/%q: %v", b.Account.Name, a.Account.Name, err)
		a.Session.Send(wire.MessagePanel("The Spirit's Seal could not be transferred."))
		b.Session.Send(wire.MessagePanel("The Spirit's Seal could not be transferred."))
		w.cancelTrade(a, "Celestial capsule transfer failed")
		return
	}
	if err := w.saveTradeAccounts(a.Account, b.Account); err != nil {
		a.Char.Inv, b.Char.Inv = oldAInv, oldBInv
		a.Char.Gold, b.Char.Gold = oldAGold, oldBGold
		a.Account.CelestialCapsules = oldACapsules
		b.Account.CelestialCapsules = oldBCapsules
		log.Printf("TRADE save accounts %q/%q: %v", a.Account.Name, b.Account.Name, err)
		a.Session.Send(wire.MessagePanel("Save failed. The trade was not applied."))
		b.Session.Send(wire.MessagePanel("Save failed. The trade was not applied."))
		w.cancelTrade(a, "persistence failed")
		return
	}

	aOfferGold, bOfferGold := a.Trade.Gold, b.Trade.Gold
	a.Trade, b.Trade = nil, nil
	a.Session.Send(wire.UpdateCarry(a.ID, a.Char.Inv[:], a.Char.Gold))
	b.Session.Send(wire.UpdateCarry(b.ID, b.Char.Inv[:], b.Char.Gold))
	a.Session.Send(wire.CloseTrade(a.ID))
	b.Session.Send(wire.CloseTrade(b.ID))
	log.Printf("TRADE completed: %s(%d gold) <-> %s(%d gold)",
		a.Char.Name, aOfferGold, b.Char.Name, bOfferGold)
}

// transferTradeCapsules moves snapshots for offered seals to the account that
// receives the items. Capsule IDs are local to each account; on collision,
// the ID is remapped and the received STRUCT_ITEM is updated before saving.
func (w *World) transferTradeCapsules(sender, recipient *Player, offered [maxTradeItems]model.Item) error {
	if sender == nil || recipient == nil || sender.Account == nil || recipient.Account == nil || recipient.Char == nil {
		return errors.New("player unavailable")
	}
	for _, offeredItem := range offered {
		oldID, filled := model.CelestialSealID(offeredItem)
		if !filled {
			continue
		}

		capsule, capsuleIndex := w.capsuleByID(sender.Account, oldID)
		if capsule == nil || capsule.ItemUID != offeredItem.UID {
			return fmt.Errorf("capsule %d does not belong to seal %q", oldID, offeredItem.UID)
		}
		moved := *capsule
		sender.Account.CelestialCapsules = append(sender.Account.CelestialCapsules[:capsuleIndex],
			sender.Account.CelestialCapsules[capsuleIndex+1:]...)

		for _, existing := range recipient.Account.CelestialCapsules {
			if existing.ItemUID == moved.ItemUID || existing.SourceUID == moved.SourceUID {
				return fmt.Errorf("capsule %d conflicts with an existing identity", oldID)
			}
		}
		for i := range recipient.Account.Chars {
			if recipient.Account.Chars[i].UID == moved.SourceUID {
				return fmt.Errorf("sealed character %q is already active in the destination", moved.SourceUID)
			}
		}

		if existing, _ := w.capsuleByID(recipient.Account, moved.ID); existing != nil {
			newID, ok := nextCelestialCapsuleID(recipient.Account)
			if !ok {
				return errors.New("destination account has no free capsule ID")
			}
			moved.ID = newID
		}

		received := false
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			item := &recipient.Char.Inv[slot]
			if item.UID != moved.ItemUID {
				continue
			}
			id, ok := model.CelestialSealID(*item)
			if !ok || id != oldID || received {
				return fmt.Errorf("received seal %q is inconsistent", moved.ItemUID)
			}
			item.Eff[0] = model.CelestialSealEffect
			item.Eff[1] = byte(moved.ID >> 8)
			item.Eff[2] = model.CelestialSealEffect
			item.Eff[3] = byte(moved.ID)
			received = true
		}
		if !received {
			return fmt.Errorf("received seal %q not found in Carry", moved.ItemUID)
		}
		recipient.Account.CelestialCapsules = append(recipient.Account.CelestialCapsules, moved)
	}
	return nil
}

type tradeBatchStore interface {
	SaveAccounts(accounts ...*model.Account) error
}

func (w *World) saveTradeAccounts(accounts ...*model.Account) error {
	snapshots := accountPersistenceSnapshots(accounts...)
	if batch, ok := w.store.(tradeBatchStore); ok {
		return batch.SaveAccounts(snapshots...)
	}
	if len(snapshots) > 1 {
		return errors.New("store lacks multi-account transactions; trade rejected")
	}
	for _, account := range snapshots {
		if err := w.store.SaveAccount(account); err != nil {
			return err
		}
	}
	return nil
}

func (w *World) onCloseTrade(s *net.Session) {
	if p := w.players[s]; p != nil {
		p.BrowsingGhostShopID = 0
		w.cancelTrade(p, "canceled by player")
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
	log.Printf("TRADE canceled for player=%d: %s", p.ID, reason)
}

func (w *World) tickTrades(now time.Time) {
	for _, p := range w.players {
		if p.Trade != nil && !p.Trade.ExpiresAt.IsZero() && !now.Before(p.Trade.ExpiresAt) {
			w.cancelTrade(p, "timed out")
		}
	}
}
