package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func handlerTestWorld(t *testing.T) (*World, *Player, *craftStore) {
	t.Helper()
	p, _ := networkedTestPlayer(1, "Handler", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	st := &craftStore{}
	w.store = st
	w.items = make(map[uint16]model.ItemDef)
	w.groundItems = make(map[uint16]*GroundItem)
	w.ghostShops = make(map[uint16]*GhostShop)
	w.nextItemID = 10000
	return w, p, st
}

func useNPCPacket(id uint16) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint16(pkt[0:2], uint16(len(pkt)))
	binary.LittleEndian.PutUint16(pkt[4:6], 0x28B)
	binary.LittleEndian.PutUint16(pkt[12:14], id)
	return pkt
}

func TestInventorySwapAndShortSkillHandlers(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Inv[0] = model.Item{Index: 100}
	p.Char.Inv[1] = model.Item{Index: 200}

	swap := make([]byte, 20)
	swap[12], swap[13], swap[14], swap[15] = placeInv, 0, placeInv, 1
	w.onSwapItem(p.Session, swap)
	if p.Char.Inv[0].Index != 200 || p.Char.Inv[1].Index != 100 {
		t.Fatalf("swap nao aplicado: inv[0]=%d inv[1]=%d", p.Char.Inv[0].Index, p.Char.Inv[1].Index)
	}

	before := p.Char.Inv
	swap[13] = model.PlayerCarrySlots
	w.onSwapItem(p.Session, swap)
	if p.Char.Inv != before {
		t.Fatal("swap com slot invisivel alterou o inventario")
	}

	p.Char.Class = 0
	p.Char.LearnedSkill = 1 << 2
	short := make([]byte, 32)
	for i := 12; i < 32; i++ {
		short[i] = 0xFF
	}
	short[12], short[13] = 2, 3
	w.onSetShortSkill(p.Session, short)
	if p.Char.ShortSkill[0] != 2 || p.Char.ShortSkill[1] != 0xFF {
		t.Fatalf("filtro de atalhos incorreto: %v", p.Char.ShortSkill[:2])
	}
}

func TestShopOpenBuyAndSellLifecycle(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.items[400] = model.ItemDef{Index: 400, Price: 1000}
	shop := &Mob{
		ID: 1100, X: 2101, Y: 2100,
		Def: &model.NPCDef{
			Name: "Merchant", Tipo: model.TipoNPC,
			Score: &model.Score{Merchant: nativeShopMerchant},
			Vende: []model.Item{{Index: 400}},
		},
	}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.Char.Gold = 5000

	w.onUseNPC(p.Session, useNPCPacket(shop.ID))
	if p.ShopNPC != shop.ID {
		t.Fatalf("loja aberta nao ficou autoritativa: %d", p.ShopNPC)
	}

	buy := make([]byte, 24)
	// The 7.48 request binds the clicked list to the visible merchant at @12.
	binary.LittleEndian.PutUint16(buy[12:14], shop.ID)
	binary.LittleEndian.PutUint16(buy[14:16], 0)
	w.onBuyItem(p.Session, buy)
	if p.Char.Inv[0].Index != 400 || p.Char.Gold != 4000 || st.saves != 1 {
		t.Fatalf("compra incorreta: item=%d gold=%d saves=%d", p.Char.Inv[0].Index, p.Char.Gold, st.saves)
	}

	sell := make([]byte, 20)
	sell[14], sell[16] = placeInv, 0
	w.onSellItem(p.Session, sell)
	if p.Char.Inv[0].Index != 0 || p.Char.Gold != 4250 || st.saves != 2 {
		t.Fatalf("venda incorreta: item=%d gold=%d saves=%d", p.Char.Inv[0].Index, p.Char.Gold, st.saves)
	}
}

func TestShopAndCargoRejectInvalidOperations(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.items[401] = model.ItemDef{Index: 401, Price: 1000}
	shop := &Mob{ID: 1101, X: 2101, Y: 2100, Def: &model.NPCDef{
		Name: "Merchant", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: nativeShopMerchant},
		Vende: []model.Item{{Index: 401}},
	}}
	cargo := &Mob{ID: 1102, X: 2102, Y: 2100, Def: &model.NPCDef{
		Name: "Cargo", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: 2},
	}}
	w.registerMobSpatial(shop)
	w.registerMobSpatial(cargo)
	w.mobs = append(w.mobs, shop, cargo)
	p.show(shop.ID)
	p.show(cargo.ID)
	p.CargoNPC = cargo.ID

	p.ShopNPC = shop.ID
	p.Char.Gold = 999
	// O 0x379 nativo tem 24 bytes; um buffer de 16 bytes exercitava apenas a
	// rejeicao de packet malformado e nunca alcançava a regra de gold.
	buy := make([]byte, 24)
	// Exercise the expected insufficient-gold path with a valid merchant ID.
	binary.LittleEndian.PutUint16(buy[12:14], shop.ID)
	w.onBuyItem(p.Session, buy)
	if p.Char.Inv[0].Index != 0 || st.saves != 0 {
		t.Fatal("compra sem gold foi aplicada")
	}
	// Rejeicoes normais de compra precisam gerar o painel 0x101 consumido pelo
	// client 7.48, em vez de terminar silenciosamente no servidor.
	if got := p.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("compra sem gold enfileirou %d packets, esperado painel de erro", got)
	}

	transfer := make([]byte, 16)
	binary.LittleEndian.PutUint32(transfer[12:16], 500)
	p.Char.Gold = 1000
	w.onCargoGold(p.Session, transfer, true)
	if p.Char.Gold != 500 || p.Account.CargoGold != 500 {
		t.Fatalf("deposito incorreto: char=%d cargo=%d", p.Char.Gold, p.Account.CargoGold)
	}
	w.onCargoGold(p.Session, transfer, false)
	if p.Char.Gold != 1000 || p.Account.CargoGold != 0 {
		t.Fatalf("saque incorreto: char=%d cargo=%d", p.Char.Gold, p.Account.CargoGold)
	}

	binary.LittleEndian.PutUint32(transfer[12:16], 1001)
	w.onCargoGold(p.Session, transfer, true)
	if p.Char.Gold != 1000 || p.Account.CargoGold != 0 {
		t.Fatal("deposito sem saldo alterou o estado")
	}
}

func TestDropAndPickupLifecycle(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[412] = model.ItemDef{Index: 412, Price: 100}
	p.Char.Inv[4] = model.Item{Index: 412, Eff: [6]byte{1, 2, 3, 4, 5, 6}}

	drop := make([]byte, 32)
	binary.LittleEndian.PutUint32(drop[12:16], placeInv)
	binary.LittleEndian.PutUint32(drop[16:20], 4)
	binary.LittleEndian.PutUint32(drop[20:24], 3)
	w.onDropItem(p.Session, drop)
	if p.Char.Inv[4].Index != 0 || len(w.groundItems) != 1 {
		t.Fatalf("drop nao materializado: inv=%d ground=%d", p.Char.Inv[4].Index, len(w.groundItems))
	}

	var ground *GroundItem
	for _, item := range w.groundItems {
		ground = item
	}
	get := make([]byte, 28)
	binary.LittleEndian.PutUint32(get[12:16], placeInv)
	binary.LittleEndian.PutUint32(get[16:20], 7)
	binary.LittleEndian.PutUint16(get[20:22], ground.ID)
	w.onGetItem(p.Session, get)
	if len(w.groundItems) != 0 || p.Char.Inv[7].Index != 412 || p.Char.Inv[7].Eff[5] != 6 {
		t.Fatalf("coleta perdeu estado: ground=%d item=%+v", len(w.groundItems), p.Char.Inv[7])
	}
}

func TestDropAndPickupValidation(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[413] = model.ItemDef{Index: 413}
	p.Char.Inv[0] = model.Item{Index: 413}

	invalidDrop := make([]byte, 32)
	binary.LittleEndian.PutUint32(invalidDrop[12:16], placeEquip)
	w.onDropItem(p.Session, invalidDrop)
	if p.Char.Inv[0].Index != 413 || len(w.groundItems) != 0 {
		t.Fatal("drop de origem invalida alterou o estado")
	}

	w.groundItems[10001] = &GroundItem{ID: 10001, Item: model.Item{Index: 413}, X: 2200, Y: 2200}
	get := make([]byte, 28)
	binary.LittleEndian.PutUint32(get[12:16], placeInv)
	binary.LittleEndian.PutUint16(get[20:22], 10001)
	w.onGetItem(p.Session, get)
	if _, ok := w.groundItems[10001]; !ok {
		t.Fatal("item fora de alcance foi coletado")
	}
}

func TestMovementStopAndChangeCityHandlers(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[24:26], 2105)
	binary.LittleEndian.PutUint16(move[26:28], 2106)
	w.onMove(p.Session, move)
	if p.X != 2100 || p.Y != 2100 || !p.MovePublished {
		t.Fatalf("destino futuro aplicado imediatamente: player=(%d,%d)", p.X, p.Y)
	}
	clock.Advance(1500 * time.Millisecond) // seis passos a RunSpeed 4.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2105 || p.Y != 2106 || p.Char.X != 2105 || p.Char.Y != 2106 {
		t.Fatalf("movimento nao chegou: player=(%d,%d) char=(%d,%d)", p.X, p.Y, p.Char.X, p.Char.Y)
	}

	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], p.X)
	binary.LittleEndian.PutUint16(stop[14:16], p.Y)
	w.onActionStop(p.Session, stop)
	if p.X != 2105 || p.Y != 2106 || p.MovePublished {
		t.Fatalf("action stop incorreto: (%d,%d) published=%v", p.X, p.Y, p.MovePublished)
	}

	moveStop := make([]byte, 36)
	binary.LittleEndian.PutUint32(moveStop[20:24], uint32(p.X))
	binary.LittleEndian.PutUint32(moveStop[24:28], uint32(p.Y))
	w.onMoveStop(p.Session, moveStop)
	if p.X != 2105 || p.Y != 2106 || p.MovePublished {
		t.Fatalf("move stop incorreto: (%d,%d) published=%v", p.X, p.Y, p.MovePublished)
	}

	changeCity := make([]byte, 16)
	binary.LittleEndian.PutUint32(changeCity[12:16], 4)
	w.onChangeCity(p.Session, changeCity)
	if p.Char.X != p.X || p.Char.Y != p.Y {
		t.Fatal("ChangeCity nao registrou a posicao corrente")
	}
}

func TestClearInventoryRollbackAndDisconnect(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Inv[0] = model.Item{Index: 100}
	p.Char.Inv[62] = model.Item{Index: 200}
	p.Char.Inv[63] = model.Item{Index: 300}

	st.err = errors.New("save falhou")
	w.executeClearInventory(p.Session, p)
	if p.Char.Inv[0].Index != 100 || p.Char.Inv[63].Index != 300 {
		t.Fatal("falha de save nao restaurou o inventario")
	}

	st.err = nil
	w.executeClearInventory(p.Session, p)
	for i, item := range p.Char.Inv {
		if item.Index != 0 {
			t.Fatalf("slot %d sobreviveu ao /limparinv: %d", i, item.Index)
		}
	}

	w.onDisconnect(p.Session)
	if w.players[p.Session] != nil || w.playersByID[p.ID] != nil {
		t.Fatal("desconexao deixou o jogador registrado no mundo")
	}
}
