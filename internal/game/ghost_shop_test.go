package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func autoTradePacket(playerID uint16, acc *model.Account, title string, slots ...int) []byte {
	var items [maxGhostShopItems]model.Item
	positions := emptyGhostShopPositions()
	var prices [maxGhostShopItems]uint32
	for i, slot := range slots {
		items[i] = acc.Cargo[slot]
		positions[i] = int8(slot)
		prices[i] = uint32(1000 + i)
	}
	return wire.AutoTrade(title, items, positions, prices, 0, playerID)
}

func TestArmiaCityLimits(t *testing.T) {
	for _, point := range [][2]uint16{{armiaMinX, armiaMinY}, {armiaMaxX, armiaMaxY}, {2112, 2088}} {
		if !inArmiaCity(point[0], point[1]) {
			t.Fatalf("coordenada de Armia rejeitada: %v", point)
		}
	}
	for _, point := range [][2]uint16{{armiaMinX - 1, armiaMinY}, {armiaMaxX + 1, armiaMaxY}, {2200, 2100}} {
		if inArmiaCity(point[0], point[1]) {
			t.Fatalf("coordenada externa aceita: %v", point)
		}
	}
}

func TestParseAutoTradeUsesAuthoritativeCargo(t *testing.T) {
	acc := &model.Account{}
	acc.Cargo[7] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	pkt := autoTradePacket(3, acc, "Loja Felipe", 7)
	req, err := parseAutoTradeRequest(pkt, acc, 3)
	if err != nil {
		t.Fatal(err)
	}
	if req.Title != "Loja Felipe" || req.Items[0] != acc.Cargo[7] ||
		req.CarryPos[0] != 7 || req.Prices[0] != 1000 {
		t.Fatalf("anuncio incorreto: %+v", req)
	}
}

func TestParseAutoTradeRejectsTamperingDuplicateAndFormatString(t *testing.T) {
	acc := &model.Account{}
	acc.Cargo[7] = model.Item{Index: 4011}

	tampered := autoTradePacket(3, acc, "Loja", 7)
	binary.LittleEndian.PutUint16(tampered[36:38], 4012)
	if _, err := parseAutoTradeRequest(tampered, acc, 3); err == nil {
		t.Fatal("item adulterado foi aceito")
	}
	duplicate := autoTradePacket(3, acc, "Loja", 7, 7)
	if _, err := parseAutoTradeRequest(duplicate, acc, 3); err == nil {
		t.Fatal("slot duplicado foi aceito")
	}
	badTitle := autoTradePacket(3, acc, "%s%s%s", 7)
	if _, err := parseAutoTradeRequest(badTitle, acc, 3); err == nil {
		t.Fatal("format string no titulo foi aceita")
	}
}

func TestParseReqBuyAutoTrade748Layout(t *testing.T) {
	pkt := wire.Build(wire.OpReqBuyAutoTrade, 3, 36)
	binary.LittleEndian.PutUint32(pkt[12:16], 4)
	binary.LittleEndian.PutUint32(pkt[16:20], 9)
	binary.LittleEndian.PutUint32(pkt[20:24], 123456)
	binary.LittleEndian.PutUint32(pkt[24:28], 0)
	wire.PutItem(pkt, 28, model.Item{Index: 4011, Eff: [6]byte{43, 9}})
	req, err := parseReqBuyAutoTrade(pkt)
	if err != nil {
		t.Fatal(err)
	}
	if req.Pos != 4 || req.TargetID != 9 || req.Price != 123456 ||
		req.Item != (model.Item{Index: 4011, Eff: [6]byte{43, 9}}) {
		t.Fatalf("ReqBuy incorreto: %+v", req)
	}
}

func TestGhostShopLocksOnlyAdvertisedCargoSlots(t *testing.T) {
	p := &Player{GhostShop: &GhostShop{CarryPos: emptyGhostShopPositions()}}
	p.GhostShop.Items[0] = model.Item{Index: 4011}
	p.GhostShop.CarryPos[0] = 7
	if !p.ghostShopLocksCargoSlot(7) || p.ghostShopLocksCargoSlot(8) {
		t.Fatal("bloqueio seletivo dos anuncios incorreto")
	}
}

func TestBuildGhostShopPurchaseIsAtomicAndServerAuthoritative(t *testing.T) {
	buyer := &model.Char{Gold: 5000}
	seller := &model.Account{CargoGold: 100}
	item := model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	seller.Cargo[7] = item
	shop := &GhostShop{Items: [maxGhostShopItems]model.Item{item},
		CarryPos: emptyGhostShopPositions(), Prices: [maxGhostShopItems]uint32{1000}}
	shop.CarryPos[0] = 7

	buyerInv, sellerCargo, buyerGold, sellerCargoGold, slot, err :=
		buildGhostShopPurchase(buyer, seller, shop, 0)
	if err != nil {
		t.Fatal(err)
	}
	if slot != 0 || buyerInv[0] != item || sellerCargo[7].Index != 0 ||
		buyerGold != 4000 || sellerCargoGold != 1100 {
		t.Fatalf("resultado incorreto slot=%d buyerGold=%d sellerCargoGold=%d", slot, buyerGold, sellerCargoGold)
	}
	// A funcao prepara cópias: antes do commit/persistencia, o estado real nao muda.
	if buyer.Inv[0].Index != 0 || seller.Cargo[7] != item || buyer.Gold != 5000 || seller.CargoGold != 100 {
		t.Fatal("estado foi alterado antes do commit")
	}
}

func TestBuildGhostShopPurchaseRejectsChangedItemFullInventoryAndGoldOverflow(t *testing.T) {
	buyer := &model.Char{Gold: 5000}
	seller := &model.Account{CargoGold: 100}
	item := model.Item{Index: 4011}
	seller.Cargo[7] = model.Item{Index: 4012}
	shop := &GhostShop{Items: [maxGhostShopItems]model.Item{item},
		CarryPos: emptyGhostShopPositions(), Prices: [maxGhostShopItems]uint32{1000}}
	shop.CarryPos[0] = 7
	if _, _, _, _, _, err := buildGhostShopPurchase(buyer, seller, shop, 0); err == nil {
		t.Fatal("snapshot alterado foi aceito")
	}

	seller.Cargo[7] = item
	for i := 0; i < model.PlayerCarrySlots; i++ {
		buyer.Inv[i] = model.Item{Index: uint16(100 + i)}
	}
	if _, _, _, _, _, err := buildGhostShopPurchase(buyer, seller, shop, 0); err == nil {
		t.Fatal("inventario cheio foi aceito")
	}
	buyer.Inv = [64]model.Item{}
	seller.CargoGold = maxCharacterGold
	if _, _, _, _, _, err := buildGhostShopPurchase(buyer, seller, shop, 0); err == nil {
		t.Fatal("overflow de gold do vendedor foi aceito")
	}
}

func TestGhostShopResetsOwnerBeforePublishingClone(t *testing.T) {
	ch := &model.Char{
		Name: "Felipe",
		Extended: testExtended(model.ExtendedScore{
			MaxHP: 321,
			CurHP: 321,
			MaxMP: 123,
			CurMP: 123,
		}),
	}
	p := &Player{ID: 7, X: 2135, Y: 2099, Char: ch}
	packets := ghostShopOwnerResetPackets(p)
	if len(packets) != 3 {
		t.Fatalf("reset gerou %d pacotes, esperado 3", len(packets))
	}
	wantTypes := []uint16{wire.OpCloseTrade, wire.OpUpdateEquip, wire.OpSetHpMp}
	for i, want := range wantTypes {
		if got := wire.ParseHeader(packets[i]).Type; got != want {
			t.Fatalf("pacote %d Type=0x%X, esperado 0x%X", i, got, want)
		}
	}
	for i, packet := range packets {
		typeID := wire.ParseHeader(packet).Type
		if typeID == wire.OpCreateMob || typeID == wire.OpAction {
			t.Fatalf("reset pacote %d reinicia entidade/movimento: Type=0x%X", i, typeID)
		}
	}
}

func TestGhostShopTradeListKeepsVirtualCloneIdentity(t *testing.T) {
	shop := &GhostShop{
		ID:      25007,
		OwnerID: 7,
		Title:   "Loja Fantasma",
	}
	shop.Items[0] = model.Item{Index: 4011}
	shop.CarryPos[0] = 3
	shop.Prices[0] = 1000

	pkt := ghostShopTradeListPacket(shop)
	if got := wire.ParseHeader(pkt).Type; got != wire.OpAutoTrade {
		t.Fatalf("Type=0x%X, esperado 0x%X", got, wire.OpAutoTrade)
	}
	if got := binary.LittleEndian.Uint16(pkt[194:196]); got != shop.ID {
		t.Fatalf("TargetID=%d, esperado clone %d (owner real=%d)", got, shop.ID, shop.OwnerID)
	}
}

// TestClonePerdeOCorpoDoDono: o clone da loja veste o rosto do Carbunkle e
// NENHUMA peca. Herdar o corpo do dono causava dois problemas -- mesh de
// monstro no rosto conflita com peca humana, e o dono expunha o proprio
// equipamento enquanto vendia.
func TestClonePerdeOCorpoDoDono(t *testing.T) {
	w := &World{npcs: []model.NPCDef{{
		Name:  "Carbunkle",
		Equip: model.Equip{Rosto: model.Item{Index: 230}, Armadura: model.Item{Index: 999}},
	}}}
	shop := &GhostShop{}
	// Suja todos os slots como se tivessem vindo do dono.
	for i := range shop.Mesh {
		shop.Mesh[i] = uint16(500 + i)
	}

	w.applyGhostShopLook(shop)

	if shop.Mesh[0] != 230 {
		t.Errorf("rosto=%d, quer 230 (Carbunkle)", shop.Mesh[0])
	}
	for i := 1; i < len(shop.Mesh); i++ {
		if shop.Mesh[i] != 0 {
			t.Errorf("slot %d ficou com %d; o clone nao pode vestir peca nenhuma",
				i, shop.Mesh[i])
		}
	}
}

// TestCloneSemCatalogoNaoHerdaODono: se o NPC sumir do data/npcs, o clone fica
// sem rosto -- mas nao pode voltar a copiar o dono em silencio.
func TestCloneSemCatalogoNaoHerdaODono(t *testing.T) {
	w := &World{}
	shop := &GhostShop{}
	for i := range shop.Mesh {
		shop.Mesh[i] = uint16(500 + i)
	}

	w.applyGhostShopLook(shop)

	if shop.Mesh != ([16]uint16{}) {
		t.Errorf("clone manteve aparencia do dono sem o catalogo: %v", shop.Mesh)
	}
}

// TestTrocaDeEquipamentoNaoMandaVitalsDuplicado: o wrapper wide do client copia
// a cauda uint32 para o sidecar e DEPOIS chama o handler nativo, sempre. Cada
// pacote de vitals custa um redesenho; 0x336 e 0x181 em sequencia ao mesmo
// jogador custavam dois, e a barra piscava.
//
// O dono recebe so o 0x336 (que ja leva HP/MP). Quem observa continua
// recebendo o 0x181, porque para ele o 0x336 privado nunca chegou.
func TestVitalsDeObservadorNaoVaoParaODono(t *testing.T) {
	w := newZoneTestWorld()
	w.players = map[*net.Session]*Player{}

	novo := func(id uint16, x uint16) (*Player, *net.Session) {
		s := net.NewTestSession(int64(id), 64)
		acc := &model.Account{Name: "c", Chars: []model.Char{{Name: "n",
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100}}}}
		p := &Player{ID: id, Session: s, Account: acc, Char: &acc.Chars[0],
			InWorld: true, X: x, Y: 2100, Visible: map[uint16]struct{}{}}
		w.players[s] = p
		w.updatePlayerSpatial(p)
		return p, s
	}
	dono, sessaoDono := novo(1, 2100)
	observador, sessaoObs := novo(2, 2101)
	observador.show(dono.ID)

	w.syncPlayerVitalsToObservers(dono)

	if sessaoDono.QueuedPacketsForTest() != 0 {
		t.Error("o dono recebeu o 0x181 redundante; e ele que faz a barra piscar")
	}
	if sessaoObs.QueuedPacketsForTest() == 0 {
		t.Error("o observador ficou sem a atualizacao de vitals")
	}
}
