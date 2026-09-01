package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type batchGameStore struct {
	craftStore
	batchSaves int
}

func (s *batchGameStore) SaveAccounts(_ ...*model.Account) error {
	s.batchSaves++
	return s.err
}

func economyWorld(players ...*Player) (*World, *batchGameStore) {
	w := worldWithNetworkedPlayers(players...)
	st := &batchGameStore{}
	w.store = st
	w.items = make(map[uint16]model.ItemDef)
	w.groundItems = make(map[uint16]*GroundItem)
	w.ghostShops = make(map[uint16]*GhostShop)
	w.nextItemID = 10000
	return w, st
}

func ghostBuyPacket(shop *GhostShop, pos int) []byte {
	pkt := wire.Build(wire.OpReqBuyAutoTrade, 0, 36)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(pos))
	binary.LittleEndian.PutUint32(pkt[16:20], uint32(shop.ID))
	binary.LittleEndian.PutUint32(pkt[20:24], shop.Prices[pos])
	binary.LittleEndian.PutUint32(pkt[24:28], uint32(shop.Tax))
	wire.PutItem(pkt, 28, shop.Items[pos])
	return pkt
}

func TestGhostShopOpenBrowseBuyAndAutoClose(t *testing.T) {
	seller, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	buyer, _ := networkedTestPlayer(2, "Buyer", 2102, 2100)
	w, st := economyWorld(seller, buyer)
	item := model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	w.items[item.Index] = model.ItemDef{Index: item.Index}
	seller.Account.Cargo[7] = item
	buyer.Char.Gold = 5000

	w.onAutoTrade(seller.Session, autoTradePacket(seller.ID, seller.Account, "Seller Shop", 7))
	shop := seller.GhostShop
	if shop == nil || w.ghostShops[shop.ID] != shop || shopItemCount(shop) != 1 {
		t.Fatalf("loja nao abriu: shop=%+v registry=%d", shop, len(w.ghostShops))
	}
	if shop.X == seller.X && shop.Y == seller.Y {
		t.Fatal("clone ocupou a coordenada do dono")
	}

	list := make([]byte, 16)
	binary.LittleEndian.PutUint32(list[12:16], uint32(shop.ID))
	w.onReqTradeList(buyer.Session, list)
	if buyer.BrowsingGhostShopID != shop.ID {
		t.Fatal("comprador nao ficou vinculado ao clone")
	}

	buyerPacketsBefore := buyer.Session.QueuedPacketsForTest()
	w.onReqBuyAutoTrade(buyer.Session, ghostBuyPacket(shop, 0))
	if buyer.Char.Inv[0] != item || buyer.Char.Gold != 4000 ||
		seller.Account.Cargo[7].Index != 0 || seller.Account.CargoGold != 1000 {
		t.Fatalf("compra incorreta: buyerItem=%+v buyerGold=%d cargo=%+v cargoGold=%d",
			buyer.Char.Inv[0], buyer.Char.Gold, seller.Account.Cargo[7], seller.Account.CargoGold)
	}
	if st.batchSaves != 1 {
		t.Fatalf("compra deveria persistir as duas contas em lote: %d", st.batchSaves)
	}
	// UpdateCarry and the post-persistence success notice are both mandatory;
	// visibility/close packets may add more messages to this same flow.
	if got := buyer.Session.QueuedPacketsForTest(); got < buyerPacketsBefore+2 {
		t.Fatalf("comprador nao recebeu estado e confirmacao de sucesso: %d -> %d", buyerPacketsBefore, got)
	}
	if seller.GhostShop != nil || len(w.ghostShops) != 0 || buyer.BrowsingGhostShopID != 0 {
		t.Fatal("loja sem estoque nao foi encerrada para dono e comprador")
	}
}

func TestGhostShopPurchaseRejectsInsufficientGoldWithNotice(t *testing.T) {
	seller, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	buyer, _ := networkedTestPlayer(2, "Buyer", 2102, 2100)
	w, st := economyWorld(seller, buyer)
	item := model.Item{Index: 4013}
	w.items[item.Index] = model.ItemDef{Index: item.Index}
	seller.Account.Cargo[10] = item
	buyer.Char.Gold = 999
	w.onAutoTrade(seller.Session, autoTradePacket(seller.ID, seller.Account, "Expensive", 10))
	shop := seller.GhostShop
	buyer.BrowsingGhostShopID = shop.ID

	before := buyer.Session.QueuedPacketsForTest()
	w.onReqBuyAutoTrade(buyer.Session, ghostBuyPacket(shop, 0))

	if buyer.Char.Inv[0].Index != 0 || buyer.Char.Gold != 999 ||
		seller.Account.Cargo[10] != item || seller.Account.CargoGold != 0 ||
		shop.Items[0] != item {
		t.Fatal("rejeicao por gold insuficiente alterou a compra")
	}
	if st.batchSaves != 0 {
		t.Fatalf("rejeicao por gold insuficiente chegou a persistir: %d", st.batchSaves)
	}
	// This rejection exits before any state publication, so the sole new packet
	// is the localized indexed message explaining that gold is insufficient.
	if got := buyer.Session.QueuedPacketsForTest(); got != before+1 {
		t.Fatalf("aviso de gold insuficiente ausente ou duplicado: %d -> %d", before, got)
	}
}

func TestGhostShopPurchaseRollsBackOnSaveFailure(t *testing.T) {
	seller, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	buyer, _ := networkedTestPlayer(2, "Buyer", 2102, 2100)
	w, st := economyWorld(seller, buyer)
	item := model.Item{Index: 4012}
	w.items[item.Index] = model.ItemDef{Index: item.Index}
	seller.Account.Cargo[9] = item
	buyer.Char.Gold = 5000
	w.onAutoTrade(seller.Session, autoTradePacket(seller.ID, seller.Account, "Rollback", 9))
	shop := seller.GhostShop
	buyer.BrowsingGhostShopID = shop.ID
	st.err = errors.New("disk full")

	w.onReqBuyAutoTrade(buyer.Session, ghostBuyPacket(shop, 0))
	if buyer.Char.Inv[0].Index != 0 || buyer.Char.Gold != 5000 ||
		seller.Account.Cargo[9] != item || seller.Account.CargoGold != 0 ||
		shop.Items[0] != item {
		t.Fatal("falha de save nao restaurou a compra da loja fantasma")
	}
}

func tradeOfferPacket(opponent uint16, ch *model.Char, gold uint32, checked bool, slots ...int) []byte {
	pkt := tradePacket(opponent, ch, slots...)
	binary.LittleEndian.PutUint32(pkt[148:152], gold)
	if checked {
		pkt[152] = 1
	}
	return pkt
}

func TestPlayerTradeInviteOffersAndAtomicCommit(t *testing.T) {
	a, _ := networkedTestPlayer(1, "TraderA", 2100, 2100)
	b, _ := networkedTestPlayer(2, "TraderB", 2101, 2100)
	w, st := economyWorld(a, b)
	a.Char.Inv[0], b.Char.Inv[0] = model.Item{Index: 100}, model.Item{Index: 200}
	a.Char.Gold, b.Char.Gold = 1000, 2000
	w.items[100] = model.ItemDef{Index: 100}
	w.items[200] = model.ItemDef{Index: 200}

	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 0, false))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 0, false))
	if a.Trade == nil || b.Trade == nil || a.Trade.OpponentID != b.ID || b.Trade.OpponentID != a.ID {
		t.Fatal("convite/aceite nao criou a sessao bilateral")
	}

	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 100, false, 0))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 200, false, 0))
	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 100, true, 0))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 200, true, 0))
	if a.Trade != nil || b.Trade != nil {
		t.Fatal("trade confirmado permaneceu aberto")
	}
	if a.Char.Inv[0].Index != 200 || b.Char.Inv[0].Index != 100 ||
		a.Char.Gold != 1100 || b.Char.Gold != 1900 {
		t.Fatalf("troca incorreta: A item/gold=%d/%d B=%d/%d",
			a.Char.Inv[0].Index, a.Char.Gold, b.Char.Inv[0].Index, b.Char.Gold)
	}
	if st.batchSaves != 1 {
		t.Fatalf("trade nao persistiu em lote: %d", st.batchSaves)
	}
}

func TestPlayerTradeRollbackCancelsBothSides(t *testing.T) {
	a, _ := networkedTestPlayer(1, "TraderA", 2100, 2100)
	b, _ := networkedTestPlayer(2, "TraderB", 2101, 2100)
	w, st := economyWorld(a, b)
	a.Char.Inv[0], b.Char.Inv[0] = model.Item{Index: 100}, model.Item{Index: 200}
	a.Char.Gold, b.Char.Gold = 1000, 2000
	w.items[100] = model.ItemDef{Index: 100}
	w.items[200] = model.ItemDef{Index: 200}

	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 0, false))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 0, false))
	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 100, false, 0))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 200, false, 0))
	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 100, true, 0))
	st.err = errors.New("disk full")
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 200, true, 0))

	if a.Char.Inv[0].Index != 100 || b.Char.Inv[0].Index != 200 ||
		a.Char.Gold != 1000 || b.Char.Gold != 2000 {
		t.Fatal("falha de save alterou itens ou gold do trade")
	}
	if a.Trade != nil || b.Trade != nil {
		t.Fatal("trade com falha de persistencia permaneceu aberto")
	}
}

func TestCloseTradeClearsGhostBrowsingAndBothTradeSides(t *testing.T) {
	a, _ := networkedTestPlayer(1, "TraderA", 2100, 2100)
	b, _ := networkedTestPlayer(2, "TraderB", 2101, 2100)
	w, _ := economyWorld(a, b)
	a.Trade = &TradeState{OpponentID: b.ID}
	b.Trade = &TradeState{OpponentID: a.ID}
	a.BrowsingGhostShopID = 25001

	w.onCloseTrade(a.Session)
	if a.Trade != nil || b.Trade != nil || a.BrowsingGhostShopID != 0 {
		t.Fatal("fechamento nao limpou trade bilateral/navegacao de loja")
	}
	w.onCloseTrade(nil)
}
