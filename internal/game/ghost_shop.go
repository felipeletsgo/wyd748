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
	maxShopTitleBytes = 21 // o client 7.48 termina CreateMobTrade.Desc em @21

	// Limites CityLimit da Armia na BASE_GetVillage da W2PP.
	armiaMinX = 2052
	armiaMaxX = 2171
	armiaMinY = 2052
	armiaMaxY = 2163
)

// GhostShop e um clone estacionario independente do jogador real. Os itens
// continuam no Cargo da conta, mas os slots anunciados ficam bloqueados e cada
// compra confere novamente o snapshot antes da transacao.
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
	Extended model.ExtendedScore
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
		return "", errors.New("informe um titulo para a loja")
	}
	if len(raw) > maxShopTitleBytes {
		return "", fmt.Errorf("titulo excede %d caracteres", maxShopTitleBytes)
	}
	for _, c := range raw {
		// O client antigo usa o titulo como format string em alguns builds.
		if c == '%' || c < 0x20 || c == 0x7f {
			return "", errors.New("titulo contem caractere invalido")
		}
	}
	return string(raw), nil
}

func parseAutoTradeRequest(pkt []byte, acc *model.Account, playerID uint16) (autoTradeRequest, error) {
	var req autoTradeRequest
	req.CarryPos = emptyGhostShopPositions()
	if acc == nil {
		return req, errors.New("conta indisponivel")
	}
	if len(pkt) != 196 {
		return req, fmt.Errorf("tamanho %d, esperado 196", len(pkt))
	}
	if binary.LittleEndian.Uint16(pkt[194:196]) != playerID {
		return req, errors.New("TargetID divergente")
	}
	title, err := parseShopTitle(pkt[12:36])
	if err != nil {
		return req, err
	}
	req.Title = title

	used := make(map[int]struct{}, maxGhostShopItems)
	itemCount := 0
	for i := 0; i < maxGhostShopItems; i++ {
		item := decodeTradeItem(pkt[36+i*8 : 44+i*8])
		signedPrice := int32(binary.LittleEndian.Uint32(pkt[144+i*4 : 148+i*4]))
		if item.Index == 0 {
			if signedPrice != 0 {
				return req, fmt.Errorf("preco sem item no anuncio %d", i)
			}
			continue
		}
		if signedPrice <= 0 || uint32(signedPrice) > maxCharacterGold {
			return req, fmt.Errorf("preco invalido no anuncio %d", i)
		}
		pos := int(pkt[132+i])
		if pos < 0 || pos >= model.PlayerCargoSlots {
			return req, fmt.Errorf("slot de Cargo %d invalido", pos)
		}
		if _, duplicate := used[pos]; duplicate {
			return req, fmt.Errorf("slot de Cargo %d duplicado", pos)
		}
		used[pos] = struct{}{}
		if acc.Cargo[pos].Index == 0 || acc.Cargo[pos] != item {
			return req, fmt.Errorf("item do slot %d diverge do Cargo", pos)
		}
		req.Items[i] = item
		req.CarryPos[i] = int8(pos)
		req.Prices[i] = uint32(signedPrice)
		itemCount++
	}
	if itemCount == 0 {
		return req, errors.New("adicione ao menos um item")
	}
	return req, nil
}

func parseReqBuyAutoTrade(pkt []byte) (reqBuyAutoTrade, error) {
	var req reqBuyAutoTrade
	if len(pkt) != 36 {
		return req, fmt.Errorf("tamanho %d, esperado 36", len(pkt))
	}
	pos := int32(binary.LittleEndian.Uint32(pkt[12:16]))
	if pos < 0 || pos >= maxGhostShopItems {
		return req, fmt.Errorf("anuncio %d invalido", pos)
	}
	req.Pos = int(pos)
	req.TargetID = binary.LittleEndian.Uint16(pkt[16:18])
	req.Price = binary.LittleEndian.Uint32(pkt[20:24])
	req.Tax = binary.LittleEndian.Uint32(pkt[24:28])
	req.Item = decodeTradeItem(pkt[28:36])
	return req, nil
}

func (w *World) onAutoTrade(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	// A W2PP usa uma segunda publicacao como liga/desliga da loja fantasma.
	if p.GhostShop != nil {
		w.closeGhostShop(p, "fechada pelo jogador")
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("Loja fantasma fechada."))
		return
	}
	if p.Trade != nil {
		s.Send(wire.MessagePanel("Finalize a troca atual antes de abrir a loja."))
		return
	}
	if !inArmiaCity(p.X, p.Y) {
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("A Loja Fantasma so pode ser aberta em Armia."))
		return
	}
	req, err := parseAutoTradeRequest(pkt, p.Account, p.ID)
	if err != nil {
		log.Printf("[#%d] LOJA FANTASMA rejeitada: %v", s.ID, err)
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("Nao foi possivel abrir a loja: " + err.Error()))
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
		s.Send(wire.MessagePanel("Nao ha espaco livre para a loja nesta area."))
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
	if extended := wireExtendedScore(p.Char); extended != nil {
		shop.Extended = *extended
	}
	copy(shop.Mesh[:], bodyMesh(p.Char))
	if _, exists := w.ghostShops[shop.ID]; exists {
		s.Send(wire.CloseTrade(p.ID))
		s.Send(wire.MessagePanel("ID da loja ja esta em uso. Tente novamente."))
		return
	}
	p.GhostShop = shop
	p.ShopNPC = 0
	w.ghostShops[shop.ID] = shop
	// O client coloca o proprio personagem em modo de auto-loja assim que envia
	// o 0x397. A ordem e importante: a W2PP executa RemoveTrade2 (0x384) ANTES
	// de criar o clone. Publicar o 0x363 primeiro fazia o 0x384 subsequente
	// apagar o titulo do clone e deixava o estado comercial no corpo real.
	for _, packet := range ghostShopOwnerResetPackets(p) {
		s.Send(packet)
	}
	w.publishGhostShopSpawn(shop)
	s.Send(wire.MessagePanel("Loja Fantasma aberta em Armia. Voce esta livre para jogar."))
	log.Printf("[#%d] LOJA FANTASMA aberta id=%d owner=%s @(%d,%d) itens=%d",
		s.ID, shop.ID, p.Char.Name, shop.X, shop.Y, shopItemCount(shop))
}

// ghostShopOwnerResetPackets desfaz no client o modo comercial aplicado
// localmente ao avatar que enviou 0x397. CloseTrade e a operacao nativa que
// limpa esse estado; UpdateEquip e SetHpMp reforcam o avatar autoritativo sem
// enviar coordenadas nem reiniciar seu movimento. O CreateMobTrade com titulo
// sera enviado separadamente, apenas para o ID virtual da loja.
func ghostShopOwnerResetPackets(p *Player) [][]byte {
	if p == nil || p.Char == nil {
		return nil
	}
	return [][]byte{
		wire.CloseTrade(p.ID),
		playerAppearancePacket(p),
		wire.SetHpMpExtended(p.ID, wireExtendedScore(p.Char)),
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
	// O client deve manter a janela vinculada ao clone que foi clicado. Enviar
	// OwnerID aqui faz o 7.48 aplicar o estado visual/comercial ao personagem
	// real quando o proprio dono consulta a loja fantasma.
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
		log.Printf("[#%d] compra LOJA FANTASMA rejeitada: %v", s.ID, err)
		return
	}
	// TargetID e o ID virtual anunciado ao client (25000 + owner), nunca o ID
	// do personagem real. A traducao para o dono acontece somente no servidor.
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
		req.Item != shop.Items[req.Pos] || req.Item.Index == 0 {
		log.Printf("[#%d] compra LOJA FANTASMA divergiu do anuncio owner=%d pos=%d", s.ID, seller.ID, req.Pos)
		return
	}
	buyerInv, sellerCargo, buyerGold, sellerCargoGold, buyerSlot, err :=
		buildGhostShopPurchase(buyer.Char, seller.Account, shop, req.Pos)
	if err != nil {
		s.Send(wire.MessagePanel(err.Error()))
		return
	}

	oldBuyerInv, oldSellerCargo := buyer.Char.Inv, seller.Account.Cargo
	oldBuyerGold, oldSellerCargoGold := buyer.Char.Gold, seller.Account.CargoGold
	buyer.Char.Inv, seller.Account.Cargo = buyerInv, sellerCargo
	buyer.Char.Gold, seller.Account.CargoGold = buyerGold, sellerCargoGold
	if err := w.saveTradeAccounts(buyer.Account, seller.Account); err != nil {
		buyer.Char.Inv, seller.Account.Cargo = oldBuyerInv, oldSellerCargo
		buyer.Char.Gold, seller.Account.CargoGold = oldBuyerGold, oldSellerCargoGold
		log.Printf("LOJA FANTASMA salvar contas %q/%q: %v", buyer.Account.Name, seller.Account.Name, err)
		s.Send(wire.MessagePanel("Falha ao salvar a compra. Nada foi alterado."))
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
	seller.Session.Send(wire.MessagePanel(fmt.Sprintf("Item %d vendido por %d gold.", item.Index, price)))
	log.Printf("[#%d] LOJA FANTASMA compra owner=%s buyer=%s item=%d price=%d inv[%d]",
		s.ID, seller.Char.Name, buyer.Char.Name, item.Index, price, buyerSlot)
	if shopItemCount(shop) == 0 {
		w.closeGhostShop(seller, "estoque esgotado")
		seller.Session.Send(wire.MessagePanel("Loja Fantasma fechada: estoque esgotado."))
	}
}

func buildGhostShopPurchase(buyer *model.Char, seller *model.Account, shop *GhostShop, pos int) (
	buyerInv [64]model.Item, sellerCargo [model.MaxCargo]model.Item,
	buyerGold, sellerCargoGold uint32, buyerSlot int, err error,
) {
	if buyer == nil || seller == nil {
		err = errors.New("Conta indisponivel.")
		return
	}
	buyerInv, sellerCargo = buyer.Inv, seller.Cargo
	buyerGold, sellerCargoGold = buyer.Gold, seller.CargoGold
	buyerSlot = -1
	if shop == nil || pos < 0 || pos >= maxGhostShopItems || shop.Items[pos].Index == 0 {
		err = errors.New("Este item nao esta mais a venda.")
		return
	}
	storageSlot := int(shop.CarryPos[pos])
	if storageSlot < 0 || storageSlot >= model.PlayerCargoSlots ||
		sellerCargo[storageSlot] != shop.Items[pos] {
		err = errors.New("O item anunciado foi alterado. Compra cancelada.")
		return
	}
	price := shop.Prices[pos]
	if buyerGold > maxCharacterGold || price == 0 || buyerGold < price {
		err = errors.New("Gold insuficiente.")
		return
	}
	if sellerCargoGold > maxCharacterGold || price > maxCharacterGold-sellerCargoGold {
		err = errors.New("O Cargo do vendedor atingiu o limite de gold.")
		return
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if buyerInv[i].Index == 0 {
			buyerSlot = i
			break
		}
	}
	if buyerSlot < 0 {
		err = errors.New("Inventario cheio.")
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
	delete(w.ghostShops, shop.ID)
	for _, viewer := range w.players {
		if viewer.BrowsingGhostShopID == shop.ID {
			viewer.BrowsingGhostShopID = 0
			if viewer.InWorld {
				viewer.Session.Send(wire.CloseTrade(viewer.ID))
			}
		}
	}
	w.publishGhostShopRemove(shop)
	log.Printf("LOJA FANTASMA fechada id=%d owner=%d: %s", shop.ID, owner.ID, reason)
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
