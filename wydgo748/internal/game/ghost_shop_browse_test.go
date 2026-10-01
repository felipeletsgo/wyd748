package game

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func ghostShopBrowseWorld(t *testing.T, shopID uint16) (*World, *Player, *Player, *GhostShop, *batchGameStore) {
	t.Helper()
	seller, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	buyer, _ := networkedTestPlayer(2, "Buyer", 2102, 2100)
	w, st := economyWorld(seller, buyer)
	shop := &GhostShop{
		ID: shopID, OwnerID: seller.ID, X: 2101, Y: 2100,
		Title: "Seller Shop", CarryPos: emptyGhostShopPositions(),
	}
	shop.Items[0] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	shop.CarryPos[0], shop.Prices[0] = 7, 1000
	seller.Account.Cargo[7] = shop.Items[0]
	seller.GhostShop = shop
	w.ghostShops[shop.ID] = shop
	buyer.BrowsingGhostShopID = 24999
	return w, buyer, seller, shop, st
}

func ghostShopBrowsePacket(target uint32) []byte {
	pkt := inboundPacket(wire.OpReqTradeList, 16)
	binary.LittleEndian.PutUint32(pkt[12:16], target)
	return pkt
}

func TestGhostShopBrowseRejectsFullWidthTargetAliases(t *testing.T) {
	for _, tt := range []struct {
		name   string
		shopID uint16
		target uint32
	}{
		{"zero", 25001, 0},
		{"above_uint16", 25001, 0x10000},
		{"high_word_alias", 25001, 0x10000 | 25001},
		{"signed_high_word_alias", 25001, 0xFFFF0000 | 25001},
		{"all_bits_set_alias", 0xFFFF, 0xFFFFFFFF},
	} {
		t.Run(tt.name, func(t *testing.T) {
			w, buyer, seller, shop, st := ghostShopBrowseWorld(t, tt.shopID)
			beforeBuyer, beforeSeller, beforeShop := *buyer.Account, *seller.Account, *shop
			// Account snapshots must own their character slices to detect mutations.
			beforeBuyer.Chars = append([]model.Char(nil), buyer.Account.Chars...)
			beforeSeller.Chars = append([]model.Char(nil), seller.Account.Chars...)
			pkt := ghostShopBrowsePacket(tt.target)
			beforePacket := append([]byte(nil), pkt...)
			for attempt := 0; attempt < 2; attempt++ {
				w.handle(command{s: buyer.Session, pkt: pkt})
				if buyer.BrowsingGhostShopID != 24999 || buyer.Session.QueuedPacketsForTest() != 0 ||
					seller.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("invalid target changed the browse binding or published a shop list")
				}
				if !reflect.DeepEqual(*buyer.Account, beforeBuyer) || !reflect.DeepEqual(*seller.Account, beforeSeller) ||
					*shop != beforeShop || seller.GhostShop != shop || w.ghostShops[shop.ID] != shop ||
					st.saves != 0 || st.batchSaves != 0 {
					t.Fatal("invalid target changed account, shop, or persistence state")
				}
				if !bytes.Equal(pkt, beforePacket) {
					t.Fatal("browse handling modified borrowed request bytes")
				}
			}
		})
	}
}

func TestGhostShopBrowseCanonicalTargetPublishesCloneSnapshot(t *testing.T) {
	for _, shopID := range []uint16{25001, 0xFFFF} {
		t.Run(fmt.Sprintf("shop_%d", shopID), func(t *testing.T) {
			w, buyer, seller, shop, st := ghostShopBrowseWorld(t, shopID)
			pkt := ghostShopBrowsePacket(uint32(shop.ID))
			w.handle(command{s: buyer.Session, pkt: pkt})
			if buyer.BrowsingGhostShopID != shop.ID || buyer.Session.QueuedPacketsForTest() != 1 ||
				seller.Session.QueuedPacketsForTest() != 0 || st.saves != 0 || st.batchSaves != 0 {
				t.Fatal("canonical browse did not bind and publish exactly one read-only snapshot")
			}
			response, ok := buyer.Session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(response) || len(response) != 196 ||
				wire.ParseHeader(response).Type != wire.OpAutoTrade {
				t.Fatal("canonical browse did not publish the encrypted AutoTrade envelope")
			}
			if binary.LittleEndian.Uint16(response[194:196]) != shop.ID ||
				decodeTradeItem(response[36:44]) != shop.Items[0] ||
				response[132] != byte(shop.CarryPos[0]) || binary.LittleEndian.Uint32(response[144:148]) != shop.Prices[0] {
				t.Fatal("browse snapshot changed clone identity, item, Cargo mapping, or price")
			}
		})
	}
}

func TestGhostShopBrowseRetainsLifecycleRejections(t *testing.T) {
	for _, name := range []string{"dead_buyer", "out_of_world", "distant_shop", "missing_shop", "missing_owner", "stale_owner_binding"} {
		t.Run(name, func(t *testing.T) {
			w, buyer, seller, shop, st := ghostShopBrowseWorld(t, 25001)
			switch name {
			case "dead_buyer":
				setPlayerCurHP(buyer.Char, 0)
			case "out_of_world":
				buyer.InWorld = false
			case "distant_shop":
				shop.X = 2200
			case "missing_shop":
				delete(w.ghostShops, shop.ID)
			case "missing_owner":
				delete(w.playersByID, seller.ID)
				delete(w.players, seller.Session)
			case "stale_owner_binding":
				seller.GhostShop = &GhostShop{ID: shop.ID}
			}
			w.onReqTradeList(buyer.Session, ghostShopBrowsePacket(uint32(shop.ID)))
			if buyer.BrowsingGhostShopID != 24999 || buyer.Session.QueuedPacketsForTest() != 0 ||
				st.saves != 0 || st.batchSaves != 0 {
				t.Fatal("rejected lifecycle state changed browse binding, publication, or persistence")
			}
		})
	}
}
