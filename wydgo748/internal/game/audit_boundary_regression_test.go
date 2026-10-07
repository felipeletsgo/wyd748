package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestSkillLearningRevalidatesMasterBoundary(t *testing.T) {
	for _, scenario := range []string{"hidden", "distant", "dead", "missing definition", "different gameplay space", "valid"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, st, master := skillMasterPacketTestWorld(t)
			p.show(master.ID)
			switch scenario {
			case "hidden":
				p.hide(master.ID)
			case "distant":
				p.X += 100
			case "dead":
				master.Dead = true
			case "missing definition":
				master.Def = nil
			case "different gameplay space":
				master.InstanceID = "other-runtime"
				w.itemInstances = map[string]*ItemInstance{"other-runtime": {RuntimeID: "other-runtime"}}
			}
			gold, learned := p.Char.Gold, p.Char.LearnedSkill
			w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID))
			if scenario == "valid" {
				if p.Char.LearnedSkill&1 == 0 || st.saves != 1 {
					t.Fatal("valid master interaction did not learn and persist")
				}
			} else if p.Char.Gold != gold || p.Char.LearnedSkill != learned || st.saves != 0 {
				t.Fatal("invalid master interaction mutated or persisted state")
			}
		})
	}
}

func TestPublicGhostShopRejectsPrivateRuntime(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.X, p.Y = armiaMinX+10, armiaMinY+10
	w.itemInstances = map[string]*ItemInstance{
		"private": {RuntimeID: "private", MemberIDs: []uint16{p.ID}},
	}
	w.setPlayerInstanceIndex(p.ID, "private")
	shop := &GhostShop{ID: 700, Name: "PublicShop", X: p.X, Y: p.Y}
	w.showGhostShop(p, shop)
	if p.hasVisible(shop.ID) {
		t.Fatal("public shop leaked into a private runtime")
	}
	p.Account.Cargo[7] = model.Item{Index: 4011}
	w.onAutoTrade(p.Session, autoTradePacket(p.ID, p.Account, "Shop", 7))
	if p.GhostShop != nil || st.saves != 0 {
		t.Fatal("private participant opened a public shop")
	}
	list := wire.Build(wire.OpReqTradeList, p.ID, 16)
	binary.LittleEndian.PutUint32(list[12:], uint32(shop.ID))
	w.ghostShops = map[uint16]*GhostShop{shop.ID: shop}
	w.onReqTradeList(p.Session, list)
	if p.BrowsingGhostShopID != 0 {
		t.Fatal("private participant bound a public shop window")
	}
	p.show(shop.ID) // Simulate visibility retained across a gameplay-space transition.
	w.refreshPlayerVisibility(p)
	if p.hasVisible(shop.ID) {
		t.Fatal("visibility reconciliation retained a public shop in a private runtime")
	}
	delete(w.itemInstances, "private")
	w.clearPlayerInstanceIndex(p.ID, "private")
	w.showGhostShop(p, shop)
	if !p.hasVisible(shop.ID) {
		t.Fatal("public participant could not see a public shop")
	}
}

func TestPublicGhostShopPurchaseRevalidatesGameplaySpace(t *testing.T) {
	seller, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	buyer, _ := networkedTestPlayer(2, "Buyer", 2102, 2100)
	w, st := economyWorld(seller, buyer)
	item := model.Item{Index: 4011}
	w.items[item.Index] = model.ItemDef{Index: item.Index}
	seller.Account.Cargo[7] = item
	buyer.Char.Gold = 5000
	w.onAutoTrade(seller.Session, autoTradePacket(seller.ID, seller.Account, "Public", 7))
	shop := seller.GhostShop
	if shop == nil {
		t.Fatal("public shop fixture failed to open")
	}
	list := wire.Build(wire.OpReqTradeList, buyer.ID, 16)
	binary.LittleEndian.PutUint32(list[12:], uint32(shop.ID))
	w.onReqTradeList(buyer.Session, list)
	if buyer.BrowsingGhostShopID != shop.ID {
		t.Fatal("public buyer fixture failed to bind the shop")
	}
	// Keep the valid listing and old window; change only the authoritative space.
	w.itemInstances = map[string]*ItemInstance{"private": {RuntimeID: "private", MemberIDs: []uint16{buyer.ID}}}
	w.setPlayerInstanceIndex(buyer.ID, "private")
	w.onReqBuyAutoTrade(buyer.Session, ghostBuyPacket(shop, 0))
	if buyer.Char.Gold != 5000 || buyer.Char.Inv[0].Index != 0 || seller.Account.Cargo[7] != item ||
		seller.Account.CargoGold != 0 || shop.Items[0] != item || st.batchSaves != 0 {
		t.Fatal("private buyer mutated or persisted a public shop purchase")
	}
	delete(w.itemInstances, "private")
	w.clearPlayerInstanceIndex(buyer.ID, "private")
	w.onReqBuyAutoTrade(buyer.Session, ghostBuyPacket(shop, 0))
	if buyer.Char.Gold != 4000 || buyer.Char.Inv[0] != item || seller.Account.Cargo[7].Index != 0 ||
		seller.Account.CargoGold != 1000 || st.batchSaves != 1 {
		t.Fatal("valid public purchase failed after leaving the private runtime")
	}
}

type splitStateRegressionStore struct {
	craftStore
	writes int
}

func (*splitStateRegressionStore) LoadCharState(string) (*model.CharState, error) {
	return nil, nil
}

func (s *splitStateRegressionStore) SaveCharState(string, *model.CharState) error {
	s.writes++
	return nil
}

func TestAggregatePersistenceRejectsNonAtomicAdapterBeforeWriting(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	st := &splitStateRegressionStore{}
	w.store = st
	for _, state := range []*model.CharState{nil, {SpecialCoins: map[string]uint32{"test": 1}}} {
		if err := w.saveAccountAndCharStateForUID(p.Account, "stable-uid", state); err == nil {
			t.Fatal("non-atomic aggregate was accepted")
		}
		if st.writes != 0 || st.saves != 0 {
			t.Fatal("rejection wrote one component of the aggregate")
		}
	}
}
