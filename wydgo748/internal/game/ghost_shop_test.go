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

func TestParseShopTitleUsesEnglishValidationMessages(t *testing.T) {
	for _, test := range []struct {
		title string
		want  string
	}{
		{"", "enter a shop title"},
		{"1234567890123456789012", "title exceeds 21 bytes"},
		{"%s", "title contains an invalid character"},
	} {
		_, err := parseShopTitle([]byte(test.title))
		if err == nil || err.Error() != test.want {
			t.Errorf("title %q: got %v, want %q", test.title, err, test.want)
		}
	}
}

func TestArmiaCityLimits(t *testing.T) {
	for _, point := range [][2]uint16{{armiaMinX, armiaMinY}, {armiaMaxX, armiaMaxY}, {2112, 2088}} {
		if !inArmiaCity(point[0], point[1]) {
			t.Fatalf("Armia coordinate rejected: %v", point)
		}
	}
	for _, point := range [][2]uint16{{armiaMinX - 1, armiaMinY}, {armiaMaxX + 1, armiaMaxY}, {2200, 2100}} {
		if inArmiaCity(point[0], point[1]) {
			t.Fatalf("outside coordinate accepted: %v", point)
		}
	}
}

func TestParseAutoTradeUsesAuthoritativeCargo(t *testing.T) {
	acc := &model.Account{}
	acc.Cargo[7] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	pkt := autoTradePacket(3, acc, "Felipe's Shop", 7)
	req, err := parseAutoTradeRequest(pkt, acc, 3)
	if err != nil {
		t.Fatal(err)
	}
	if req.Title != "Felipe's Shop" || req.Items[0] != acc.Cargo[7] ||
		req.CarryPos[0] != 7 || req.Prices[0] != 1000 {
		t.Fatalf("incorrect listing: %+v", req)
	}
}

func TestParseAutoTradeRejectsTamperingDuplicateAndFormatString(t *testing.T) {
	acc := &model.Account{}
	acc.Cargo[7] = model.Item{Index: 4011}

	tampered := autoTradePacket(3, acc, "Shop", 7)
	binary.LittleEndian.PutUint16(tampered[36:38], 4012)
	if _, err := parseAutoTradeRequest(tampered, acc, 3); err == nil {
		t.Fatal("tampered item was accepted")
	}
	duplicate := autoTradePacket(3, acc, "Shop", 7, 7)
	if _, err := parseAutoTradeRequest(duplicate, acc, 3); err == nil {
		t.Fatal("duplicate slot was accepted")
	}
	badTitle := autoTradePacket(3, acc, "%s%s%s", 7)
	if _, err := parseAutoTradeRequest(badTitle, acc, 3); err == nil {
		t.Fatal("format string in title was accepted")
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
		t.Fatalf("incorrect ReqBuy: %+v", req)
	}
}

func TestGhostShopLocksOnlyAdvertisedCargoSlots(t *testing.T) {
	p := &Player{GhostShop: &GhostShop{CarryPos: emptyGhostShopPositions()}}
	p.GhostShop.Items[0] = model.Item{Index: 4011}
	p.GhostShop.CarryPos[0] = 7
	if !p.ghostShopLocksCargoSlot(7) || p.ghostShopLocksCargoSlot(8) {
		t.Fatal("selective locking of listed slots is incorrect")
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
		t.Fatalf("incorrect result slot=%d buyerGold=%d sellerCargoGold=%d", slot, buyerGold, sellerCargoGold)
	}
	// The function prepares copies: real state is unchanged before commit and persistence.
	if buyer.Inv[0].Index != 0 || seller.Cargo[7] != item || buyer.Gold != 5000 || seller.CargoGold != 100 {
		t.Fatal("state changed before commit")
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
	if _, _, _, _, _, err := buildGhostShopPurchase(buyer, seller, shop, 0); err == nil || err.Error() != "The listed item changed. Purchase canceled." {
		t.Fatalf("expected English changed-item message, got %v", err)
	}

	seller.Cargo[7] = item
	for i := 0; i < model.PlayerCarrySlots; i++ {
		buyer.Inv[i] = model.Item{Index: uint16(100 + i)}
	}
	if _, _, _, _, _, err := buildGhostShopPurchase(buyer, seller, shop, 0); err == nil || err.Error() != "Inventory is full." {
		t.Fatalf("expected English full-inventory message, got %v", err)
	}
	buyer.Inv = [64]model.Item{}
	seller.CargoGold = maxCharacterGold
	if _, _, _, _, _, err := buildGhostShopPurchase(buyer, seller, shop, 0); err == nil || err.Error() != "The seller's Cargo has reached its gold limit." {
		t.Fatalf("expected English seller-gold message, got %v", err)
	}
}

func TestGhostShopResetsOwnerBeforePublishingClone(t *testing.T) {
	ch := &model.Char{
		Name: "Felipe",
		Score: testScore(model.Score{
			MaxHP: 321,
			CurHP: 321,
			MaxMP: 123,
			CurMP: 123,
		}),
	}
	p := &Player{ID: 7, X: 2135, Y: 2099, Char: ch}
	packets := ghostShopOwnerResetPackets(p)
	if len(packets) != 3 {
		t.Fatalf("reset produced %d packets, expected 3", len(packets))
	}
	wantTypes := []uint16{wire.OpCloseTrade, wire.OpUpdateEquip, wire.OpSetHpMp}
	for i, want := range wantTypes {
		if got := wire.ParseHeader(packets[i]).Type; got != want {
			t.Fatalf("packet %d Type=0x%X, expected 0x%X", i, got, want)
		}
	}
	for i, packet := range packets {
		typeID := wire.ParseHeader(packet).Type
		if typeID == wire.OpCreateMob || typeID == wire.OpAction {
			t.Fatalf("reset packet %d recreates entity/movement: Type=0x%X", i, typeID)
		}
	}
}

func TestGhostShopTradeListKeepsVirtualCloneIdentity(t *testing.T) {
	shop := &GhostShop{
		ID:      25007,
		OwnerID: 7,
		Title:   "Ghost Shop",
	}
	shop.Items[0] = model.Item{Index: 4011}
	shop.CarryPos[0] = 3
	shop.Prices[0] = 1000

	pkt := ghostShopTradeListPacket(shop)
	if got := wire.ParseHeader(pkt).Type; got != wire.OpAutoTrade {
		t.Fatalf("Type=0x%X, expected 0x%X", got, wire.OpAutoTrade)
	}
	if got := binary.LittleEndian.Uint16(pkt[194:196]); got != shop.ID {
		t.Fatalf("TargetID=%d, expected clone %d (real owner=%d)", got, shop.ID, shop.OwnerID)
	}
}

// TestCloneDoesNotInheritOwnerBody checks that the shop clone uses Carbunkle's
// face and no other equipment. Inheriting the owner's body caused the monster
// face mesh to conflict with human parts and exposed the owner's equipment.
func TestCloneDoesNotInheritOwnerBody(t *testing.T) {
	w := &World{npcs: []model.NPCDef{{
		Name:  "Carbunkle",
		Equip: model.Equip{Rosto: model.Item{Index: 230}, Armadura: model.Item{Index: 999}},
	}}}
	shop := &GhostShop{}
	// Populate every slot as if it came from the owner.
	for i := range shop.Mesh {
		shop.Mesh[i] = uint16(500 + i)
	}

	w.applyGhostShopLook(shop)

	if shop.Mesh[0] != 230 {
		t.Errorf("face=%d, want 230 (Carbunkle)", shop.Mesh[0])
	}
	for i := 1; i < len(shop.Mesh); i++ {
		if shop.Mesh[i] != 0 {
			t.Errorf("slot %d contains %d; the clone must not wear other equipment",
				i, shop.Mesh[i])
		}
	}
}

// TestCloneWithoutCatalogDoesNotInheritOwner checks that a missing NPC leaves
// the clone without a face instead of silently copying the owner.
func TestCloneWithoutCatalogDoesNotInheritOwner(t *testing.T) {
	w := &World{}
	shop := &GhostShop{}
	for i := range shop.Mesh {
		shop.Mesh[i] = uint16(500 + i)
	}

	w.applyGhostShopLook(shop)

	if shop.Mesh != ([16]uint16{}) {
		t.Errorf("clone kept the owner's appearance without the catalog: %v", shop.Mesh)
	}
}

// TestObserverVitalsAreNotSentToOwner checks that the client's wide wrapper
// copies the uint32 tail to the sidecar and then always calls the native
// handler. Each vitals packet causes a redraw; sending 0x336 and 0x181 in
// sequence to the same player made the bar flicker.
//
// The owner receives only 0x336, which already carries HP/MP. Observers still
// receive 0x181 because they never receive the owner's private 0x336 packet.
func TestObserverVitalsAreNotSentToOwner(t *testing.T) {
	w := newZoneTestWorld()
	w.players = map[*net.Session]*Player{}

	newPlayer := func(id uint16, x uint16) (*Player, *net.Session) {
		s := net.NewTestSession(int64(id), 64)
		acc := &model.Account{Name: "c", Chars: []model.Char{{Name: "n",
			Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100}}}}
		p := &Player{ID: id, Session: s, Account: acc, Char: &acc.Chars[0],
			InWorld: true, X: x, Y: 2100, Visible: map[uint16]struct{}{}}
		w.players[s] = p
		w.updatePlayerSpatial(p)
		return p, s
	}
	owner, ownerSession := newPlayer(1, 2100)
	observer, observerSession := newPlayer(2, 2101)
	observer.show(owner.ID)

	w.syncPlayerVitalsToObservers(owner)

	if ownerSession.QueuedPacketsForTest() != 0 {
		t.Error("owner received redundant 0x181, which causes the bar to flicker")
	}
	if observerSession.QueuedPacketsForTest() == 0 {
		t.Error("observer did not receive the vitals update")
	}
}
