package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func rebuyBuyPacket(actor, targetPos uint16) []byte {
	pkt := make([]byte, 24)
	binary.LittleEndian.PutUint16(pkt[0:2], uint16(len(pkt)))
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpBuyItem)
	binary.LittleEndian.PutUint16(pkt[6:8], actor)
	binary.LittleEndian.PutUint16(pkt[12:14], actor) // TargetID=self => rebuy
	binary.LittleEndian.PutUint16(pkt[14:16], targetPos)
	return pkt
}

func TestRebuyLifecycleUsesUIDAndAuthoritativePrice(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.items[400] = model.ItemDef{Index: 400, Price: 1000}
	shop := &Mob{ID: 1100, X: 2101, Y: 2100, Def: &model.NPCDef{
		Name: "Merchant", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: nativeShopMerchant},
		Vende: []model.Item{{Index: 400}},
	}}
	w.registerMobSpatial(shop)
	w.mobs = append(w.mobs, shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	p.Char.Inv[0] = model.Item{Index: 400}
	p.Char.Gold = 0
	sell := make([]byte, 20)
	sell[14], sell[16] = placeInv, 0
	w.onSellItem(p.Session, sell)
	if p.Rebuy[0].Item.Index != 400 || p.Rebuy[0].Item.UID == "" || p.Char.Inv[0].Index != 0 {
		t.Fatalf("venda nao alimentou a recompra: rebuy=%+v inv=%+v", p.Rebuy[0], p.Char.Inv[0])
	}
	before := p.Session.QueuedPacketsForTest()
	request := make([]byte, repurchasePacketSize)
	binary.LittleEndian.PutUint16(request[0:2], uint16(len(request)))
	binary.LittleEndian.PutUint16(request[4:6], wire.OpRebuy)
	binary.LittleEndian.PutUint16(request[6:8], p.ID)
	binary.LittleEndian.PutUint32(request[12:16], uint32(shop.ID))
	w.onRebuyRequest(p.Session, request)
	if p.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("pedido de recompra nao retornou a lista")
	}
	p.Char.Gold = 1000
	w.onBuyItem(p.Session, rebuyBuyPacket(p.ID, 0))
	if p.Char.Inv[0].Index != 400 || p.Char.Gold != 0 || p.Rebuy[0].Item.Index != 0 || st.saves != 2 {
		t.Fatalf("recompra incorreta: inv=%+v gold=%d rebuy=%+v saves=%d", p.Char.Inv[0], p.Char.Gold, p.Rebuy[0], st.saves)
	}
}

func TestRebuyRollbackAndFullListShift(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Rebuy", 2100, 2100)
	for i := 0; i < maxRebuyEntries+1; i++ {
		p.addRebuy(model.Item{Index: uint16(500 + i)}, uint32(i))
	}
	if p.Rebuy[0].Item.Index != 501 || p.Rebuy[9].Item.Index != 510 {
		t.Fatalf("lixeira cheia nao deslocou a entrada mais antiga: %+v", p.Rebuy)
	}
	w := worldWithNetworkedPlayers(p)
	w.store = &craftStore{err: errors.New("falha")}
	w.items = map[uint16]model.ItemDef{501: {Index: 501, Price: 10}}
	p.ShopNPC = 0
	p.Char.Gold = 100
	w.onRebuyPurchase(p.Session, p, rebuyBuyPacket(p.ID, 0), 0)
	if p.Char.Inv[0].Index != 0 || p.Char.Gold != 100 || p.Rebuy[0].Item.Index != 501 {
		t.Fatalf("rollback da recompra deixou estado parcial: inv=%d gold=%d rebuy=%d",
			p.Char.Inv[0].Index, p.Char.Gold, p.Rebuy[0].Item.Index)
	}
}
