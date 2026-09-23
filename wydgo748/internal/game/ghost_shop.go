package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	maxGhostShopItems = 12
	ghostShopIDBase   = 25000
	maxShopTitleBytes = 21 // The 7.48 client terminates CreateMobTrade.Desc at @21.

	// Armia CityLimit bounds in WYD 7.48 BASE_GetVillage.
	armiaMinX = 2052
	armiaMaxX = 2171
	armiaMinY = 2052
	armiaMaxY = 2163
)

// GhostShop is a stationary clone independent of the real player. Items remain
// in the account's Cargo, but listed slots are locked and every purchase
// rechecks the snapshot before the transaction.
type GhostShop struct {
	ID       uint16
	OwnerID  uint16
	X, Y     uint16
	Title    string
	Name     string
	Items    [maxGhostShopItems]model.Item
	CarryPos [maxGhostShopItems]int8
	Prices   [maxGhostShopItems]uint32
	Tax      uint16
	Mesh     [16]uint16
	Score    model.Score
}

type autoTradeRequest struct {
	Title    string
	Items    [maxGhostShopItems]model.Item
	CarryPos [maxGhostShopItems]int8
	Prices   [maxGhostShopItems]uint32
}

type reqBuyAutoTrade struct {
	Pos      int
	TargetID uint16
	Price    uint32
	Tax      uint32
	Item     model.Item
}

func inArmiaCity(x, y uint16) bool {
	return x >= armiaMinX && x <= armiaMaxX && y >= armiaMinY && y <= armiaMaxY
}

func emptyGhostShopPositions() [maxGhostShopItems]int8 {
	var positions [maxGhostShopItems]int8
	for i := range positions {
		positions[i] = -1
	}
	return positions
}

func parseShopTitle(raw []byte) (string, error) {
	if end := bytes.IndexByte(raw, 0); end >= 0 {
		raw = raw[:end]
	}
	raw = bytes.TrimSpace(raw)
	if len(raw) == 0 {
		return "", errors.New("enter a shop title")
	}
	if len(raw) > maxShopTitleBytes {
		return "", fmt.Errorf("title exceeds %d bytes", maxShopTitleBytes)
	}
	for _, c := range raw {
		// Some legacy client builds use the title as a format string.
		if c == '%' || c < 0x20 || c == 0x7f {
			return "", errors.New("title contains an invalid character")
		}
	}
	return string(raw), nil
}

func parseAutoTradeRequest(pkt []byte, acc *model.Account, playerID uint16) (autoTradeRequest, error) {
	var req autoTradeRequest
	req.CarryPos = emptyGhostShopPositions()
	if acc == nil {
		return req, errors.New("account unavailable")
	}
	if len(pkt) != 196 {
		return req, fmt.Errorf("packet size %d, expected 196", len(pkt))
	}
	if binary.LittleEndian.Uint16(pkt[194:196]) != playerID {
		return req, errors.New("TargetID mismatch")
	}
	title, err := parseShopTitle(pkt[12:36])
	if err != nil {
		return req, err
	}
	req.Title = title

	used := make(map[int]struct{}, maxGhostShopItems)
	itemCount := 0
	for i := 0; i < maxGhostShopItems; i++ {
		packetItem := decodeTradeItem(pkt[36+i*8 : 44+i*8])
		signedPrice := int32(binary.LittleEndian.Uint32(pkt[144+i*4 : 148+i*4]))
		if packetItem.Index == 0 {
			if signedPrice != 0 {
				return req, fmt.Errorf("price without an item in listing %d", i)
			}
			continue
		}
		if signedPrice <= 0 || uint32(signedPrice) > maxCharacterGold {
			return req, fmt.Errorf("invalid price in listing %d", i)
		}
		pos := int(pkt[132+i])
		if pos < 0 || pos >= model.PlayerCargoSlots {
			return req, fmt.Errorf("invalid Cargo slot %d", pos)
		}
		if _, duplicate := used[pos]; duplicate {
			return req, fmt.Errorf("duplicate Cargo slot %d", pos)
		}
		used[pos] = struct{}{}
		if acc.Cargo[pos].Index == 0 || !acc.Cargo[pos].WireEqual(packetItem) {
			return req, fmt.Errorf("item in slot %d differs from Cargo", pos)
		}
		if _, filled := model.CelestialSealID(acc.Cargo[pos]); filled {
			return req, errors.New("a filled Spirit's Seal cannot be listed")
		}
		req.Items[i] = acc.Cargo[pos]
		req.CarryPos[i] = int8(pos)
		req.Prices[i] = uint32(signedPrice)
		itemCount++
	}
	if itemCount == 0 {
		return req, errors.New("add at least one item")
	}
	return req, nil
}

func parseReqBuyAutoTrade(pkt []byte) (reqBuyAutoTrade, error) {
	var req reqBuyAutoTrade
	if len(pkt) != 36 {
		return req, fmt.Errorf("packet size %d, expected 36", len(pkt))
	}
	pos := int32(binary.LittleEndian.Uint32(pkt[12:16]))
	if pos < 0 || pos >= maxGhostShopItems {
		return req, fmt.Errorf("invalid listing %d", pos)
	}
	req.Pos = int(pos)
	req.TargetID = binary.LittleEndian.Uint16(pkt[16:18])
	req.Price = binary.LittleEndian.Uint32(pkt[20:24])
	req.Tax = binary.LittleEndian.Uint32(pkt[24:28])
	req.Item = decodeTradeItem(pkt[28:36])
	return req, nil
}

// ghostShopFaceNPC names the mob whose face the shop clone uses. Resolve it
// through the catalog instead of hardcoding an index, so NPC data may change.
const ghostShopFaceNPC = "Carbunkle"

// applyGhostShopLook gives the clone its shop appearance: the Carbunkle face
// and NO equipment pieces.
//
// Clearing the other fifteen slots prevents a monster-face mesh from
// conflicting with human armor, pants, gloves, and boots. Without those
// pieces, the clone is simply a Carbunkle, and the owner's own equipment is
// not exposed while selling.
func (w *World) applyGhostShopLook(shop *GhostShop) {
	shop.Mesh = [16]uint16{}
	if def := w.npcDefByName(ghostShopFaceNPC); def != nil {
		if mesh := def.Mesh(); len(mesh) > 0 {
			shop.Mesh[0] = mesh[0]
			return
		}
	}
	// If the NPC is missing, keep the shop open with a faceless clone rather
	// than silently inheriting the owner's body and old appearance.
	log.Printf("GHOST SHOP: NPC %q missing from catalog; clone has no face", ghostShopFaceNPC)
}

func (w *World) onAutoTrade(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	// WYD 7.48 uses a second listing request to toggle the ghost shop off.
	if p.GhostShop != nil {
		w.closeGhostShop(p, "closed by player")
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("Auto Trade closed."))
		return
	}
	if p.Trade != nil {
		s.Send(wire.MessagePanel("Finish the current trade before opening the shop."))
		return
	}
	if !inArmiaCity(p.X, p.Y) {
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("Auto Trade can only be opened in Armia."))
		return
	}
	req, err := parseAutoTradeRequest(pkt, p.Account, p.ID)
	if err != nil {
		log.Printf("[#%d] GHOST SHOP rejected: %v", s.ID, err)
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("Could not open the shop: " + err.Error()))
		return
	}
	for _, item := range req.Items {
		if item.Index == 0 {
			continue
		}
		if err := w.validateTradableItem(item); err != nil {
			s.Send(wire.CloseTrade(p.ID))
			s.Send(wire.MessagePanel(err.Error()))
			return
		}
	}
	x, y, ok := w.findGhostShopPosition(p.X, p.Y)
	if !ok {
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("There is no free space for the shop in this area."))
		return
	}
	shop := &GhostShop{
		ID:       ghostShopIDBase + p.ID,
		OwnerID:  p.ID,
		X:        x,
		Y:        y,
		Title:    req.Title,
		Name:     p.Char.Name,
		Items:    req.Items,
		CarryPos: req.CarryPos,
		Prices:   req.Prices,
	}
	if extended := wireScoreState(p.Char); extended != nil {
		shop.Score = *extended
	}
	w.applyGhostShopLook(shop)
	if _, exists := w.ghostShops[shop.ID]; exists {
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("That shop ID is already in use. Try again."))
		return
	}
	p.GhostShop = shop
	p.ShopNPC = 0
	w.registerGhostShop(shop)
	// The client puts its own character in auto-shop mode when it sends 0x397.
	// Order matters: WYD 7.48 must execute RemoveTrade2 (0x384) BEFORE creating
	// the clone. Sending 0x363 first let the later 0x384 erase the clone's title
	// while leaving trade appearance on the real character.
	for _, packet := range ghostShopOwnerResetPackets(p) {
		s.Send(packet)
	}
	w.publishGhostShopSpawn(shop)
	s.Send(wire.MessagePanel("Auto Trade opened in Armia. You are free to play."))
	log.Printf("[#%d] GHOST SHOP opened id=%d owner=%s @(%d,%d) items=%d",
		s.ID, shop.ID, p.Char.Name, shop.X, shop.Y, shopItemCount(shop))
}

// ghostShopOwnerResetPackets reverses the trade mode applied locally by the
// client to the avatar that sent 0x397. CloseTrade clears that state;
// UpdateEquip and SetHpMp reinforce the authoritative avatar without sending
// coordinates or restarting movement. CreateMobTrade with the title is sent
// separately for the virtual shop ID only.
func ghostShopOwnerResetPackets(p *Player) [][]byte {
	if p == nil || p.Char == nil {
		return nil
	}
	return [][]byte{
		wire.CloseTrade(p.ID),
		playerAppearancePacket(p),
		wire.HpMp(p.ID, wireScoreState(p.Char)),
	}
}

func (w *World) onReqTradeList(s *net.Session, pkt []byte) {
	buyer := w.players[s]
	if buyer == nil || buyer.Char == nil || !buyer.InWorld || playerCurHP(buyer.Char) == 0 || len(pkt) != 16 {
		return
	}
	target := uint16(binary.LittleEndian.Uint32(pkt[12:16]))
	shop := w.ghostShops[target]
	if shop == nil || !inView(buyer.X, buyer.Y, shop.X, shop.Y) {
		return
	}
	owner := w.playerByID(shop.OwnerID)
	if owner == nil || owner.GhostShop != shop {
		return
	}
	buyer.BrowsingGhostShopID = shop.ID
	// Keep the client window bound to the clicked clone. Sending OwnerID here
	// makes 7.48 apply the shop appearance to the real character when the owner
	// browses their own ghost shop.
	s.Send(ghostShopTradeListPacket(shop))
}

func ghostShopTradeListPacket(shop *GhostShop) []byte {
	if shop == nil {
		return nil
	}
	return wire.AutoTrade(shop.Title, shop.Items, shop.CarryPos, shop.Prices, shop.Tax, shop.ID)
}

func (w *World) onReqBuyAutoTrade(s *net.Session, pkt []byte) {
	buyer := w.players[s]
	if buyer == nil || buyer.Char == nil || !buyer.InWorld || playerCurHP(buyer.Char) == 0 {
		return
	}
	req, err := parseReqBuyAutoTrade(pkt)
	if err != nil {
		log.Printf("[#%d] GHOST SHOP purchase rejected: %v", s.ID, err)
		return
	}
	// TargetID is the virtual ID advertised to the client (25000 + owner),
	// never the real character ID. Only the server maps it back to the owner.
	shop := w.ghostShops[req.TargetID]
	if shop == nil || buyer.BrowsingGhostShopID != shop.ID {
		return
	}
	seller := w.playerByID(shop.OwnerID)
	if seller == nil || seller == buyer || seller.Char == nil || seller.GhostShop != shop ||
		seller.Account == buyer.Account {
		return
	}
	if !inView(buyer.X, buyer.Y, shop.X, shop.Y) {
		return
	}
	if req.Tax != uint32(shop.Tax) || req.Price != shop.Prices[req.Pos] ||
		!req.Item.WireEqual(shop.Items[req.Pos]) || req.Item.Index == 0 {
		log.Printf("[#%d] GHOST SHOP purchase differs from listing owner=%d pos=%d", s.ID, seller.ID, req.Pos)
		return
	}
	buyerInv, sellerCargo, buyerGold, sellerCargoGold, buyerSlot, err :=
		buildGhostShopPurchase(buyer.Char, seller.Account, shop, req.Pos)
	if err != nil {
		if err.Error() == "Not enough gold." {
			s.Send(wire.MessageIndexed(-845))
		} else {
			s.Send(wire.MessagePanel(err.Error()))
		}
		return
	}

	oldBuyerInv, oldSellerCargo := buyer.Char.Inv, seller.Account.Cargo
	oldBuyerGold, oldSellerCargoGold := buyer.Char.Gold, seller.Account.CargoGold
	buyer.Char.Inv, seller.Account.Cargo = buyerInv, sellerCargo
	buyer.Char.Gold, seller.Account.CargoGold = buyerGold, sellerCargoGold
	if err := w.saveTradeAccounts(buyer.Account, seller.Account); err != nil {
		buyer.Char.Inv, seller.Account.Cargo = oldBuyerInv, oldSellerCargo
		buyer.Char.Gold, seller.Account.CargoGold = oldBuyerGold, oldSellerCargoGold
		log.Printf("GHOST SHOP save accounts %q/%q: %v", buyer.Account.Name, seller.Account.Name, err)
		s.Send(wire.MessagePanel("Save failed. The purchase was not applied."))
		return
	}

	item, price := shop.Items[req.Pos], shop.Prices[req.Pos]
	storageSlot := int(shop.CarryPos[req.Pos])
	shop.Items[req.Pos] = model.Item{}
	shop.CarryPos[req.Pos] = -1
	shop.Prices[req.Pos] = 0
	buyer.Session.Send(wire.UpdateCarry(buyer.ID, buyer.Char.Inv[:], buyer.Char.Gold))
	seller.Session.Send(wire.SendItem(seller.ID, placeStorage, byte(storageSlot), model.Item{}))
	seller.Session.Send(wire.UpdateCargoGold(wire.SceneField, seller.Account.CargoGold))
	w.publishGhostShopItemSold(shop, uint32(req.Pos))
	// Only acknowledge the buyer after the atomic account save succeeded; a
	// rejected or rolled-back purchase must never display a false success.
	buyer.Session.Send(wire.MessagePanel(fmt.Sprintf("Item %d purchased for %d gold.", item.Index, price)))
	seller.Session.Send(wire.MessagePanel(fmt.Sprintf("Item %d sold for %d gold.", item.Index, price)))
	log.Printf("[#%d] GHOST SHOP purchase owner=%s buyer=%s item=%d price=%d inv[%d]",
		s.ID, seller.Char.Name, buyer.Char.Name, item.Index, price, buyerSlot)
	if shopItemCount(shop) == 0 {
		w.closeGhostShop(seller, "stock sold out")
		seller.Session.Send(wire.MessagePanel("Auto Trade closed: stock sold out."))
	}
}

func buildGhostShopPurchase(buyer *model.Char, seller *model.Account, shop *GhostShop, pos int) (
	buyerInv [64]model.Item, sellerCargo [model.MaxCargo]model.Item,
	buyerGold, sellerCargoGold uint32, buyerSlot int, err error,
) {
	if buyer == nil || seller == nil {
		err = clientError("Account unavailable.")
		return
	}
	buyerInv, sellerCargo = buyer.Inv, seller.Cargo
	buyerGold, sellerCargoGold = buyer.Gold, seller.CargoGold
	buyerSlot = -1
	if shop == nil || pos < 0 || pos >= maxGhostShopItems || shop.Items[pos].Index == 0 {
		err = clientError("This item is no longer for sale.")
		return
	}
	storageSlot := int(shop.CarryPos[pos])
	if storageSlot < 0 || storageSlot >= model.PlayerCargoSlots ||
		sellerCargo[storageSlot] != shop.Items[pos] {
		err = clientError("The listed item changed. Purchase canceled.")
		return
	}
	price := shop.Prices[pos]
	if buyerGold > maxCharacterGold || price == 0 || buyerGold < price {
		err = clientError("Not enough gold.")
		return
	}
	if sellerCargoGold > maxCharacterGold || price > maxCharacterGold-sellerCargoGold {
		err = clientError("The seller's Cargo has reached its gold limit.")
		return
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if buyerInv[i].Index == 0 {
			buyerSlot = i
			break
		}
	}
	if buyerSlot < 0 {
		err = clientError("Inventory is full.")
		return
	}
	buyerInv[buyerSlot] = shop.Items[pos]
	sellerCargo[storageSlot] = model.Item{}
	buyerGold -= price
	sellerCargoGold += price
	return
}

func (w *World) findGhostShopPosition(x, y uint16) (uint16, uint16, bool) {
	for radius := 0; radius <= 8; radius++ {
		for dy := -radius; dy <= radius; dy++ {
			for dx := -radius; dx <= radius; dx++ {
				if radius > 0 && absInt(dx) != radius && absInt(dy) != radius {
					continue
				}
				cx, cy := int(x)+dx, int(y)+dy
				if cx < armiaMinX || cx > armiaMaxX || cy < armiaMinY || cy > armiaMaxY {
					continue
				}
				ux, uy := uint16(cx), uint16(cy)
				if !w.positionOccupiedExcept(ux, uy, nil, nil) {
					return ux, uy, true
				}
			}
		}
	}
	return 0, 0, false
}

func (w *World) closeGhostShop(owner *Player, reason string) {
	if owner == nil || owner.GhostShop == nil {
		return
	}
	shop := owner.GhostShop
	owner.GhostShop = nil
	w.unregisterGhostShop(shop)
	for _, viewer := range w.players {
		if viewer.BrowsingGhostShopID == shop.ID {
			viewer.BrowsingGhostShopID = 0
			if viewer.InWorld {
				viewer.Session.Send(wire.CloseTrade(viewer.ID))
			}
		}
	}
	w.publishGhostShopRemove(shop)
	log.Printf("GHOST SHOP closed id=%d owner=%d: %s", shop.ID, owner.ID, reason)
}

func shopItemCount(shop *GhostShop) int {
	if shop == nil {
		return 0
	}
	count := 0
	for _, item := range shop.Items {
		if item.Index != 0 {
			count++
		}
	}
	return count
}

func (p *Player) ghostShopLocksCargoSlot(slot int) bool {
	if p == nil || p.GhostShop == nil || slot < 0 {
		return false
	}
	for i, pos := range p.GhostShop.CarryPos {
		if int(pos) == slot && p.GhostShop.Items[i].Index != 0 {
			return true
		}
	}
	return false
}
