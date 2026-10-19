package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestDionys759SalePriceBoundaries(t *testing.T) {
	for _, tc := range [][2]uint64{{0, 0}, {5000, 5000}, {5001, 3334}, {10000, 6666}, {10001, 5000}, {50000, 25000}} {
		if got := dionysSalePrice(tc[0]); got != tc[1] {
			t.Fatalf("quarter price %d: got %d, want %d", tc[0], got, tc[1])
		}
	}
}

func TestDionys759SaleWithoutOrdinaryShopContext(t *testing.T) {
	for _, reason := range []string{"success", "admission", "deadline", "dead", "nonmember", "unowned", "other-runtime", "out-of-range", "hidden", "gold-cap", "save-failure", "wide-type", "wide-position", "reserved-slot"} {
		t.Run(reason, func(t *testing.T) {
			w, p, st, merchant := hellStoreWorld(t)
			inst := attachDionysStore(w, p, merchant)
			w.items[400] = model.ItemDef{Index: 400, Price: 200000}
			p.Char.Inv[5] = model.Item{Index: 400, UID: "dionys-sale-item"}
			request := saleIngressPacket(merchant.ID, placeInv, 5)
			switch reason {
			case "admission":
				inst.State = "admission"
			case "deadline":
				inst.HardDeadline = w.now()
			case "dead":
				merchant.HP, merchant.Dead = 0, true
			case "nonmember":
				inst.MemberIDs = nil
				w.rebuildPlayerInstanceIndex()
			case "unowned":
				delete(inst.NPCIDs, merchant.ID)
			case "other-runtime":
				merchant.InstanceID = "other"
			case "out-of-range":
				merchant.X += 50
			case "hidden":
				p.hide(merchant.ID)
			case "gold-cap":
				p.Char.Gold = maxCharacterGold
			case "save-failure":
				st.err = errors.New("disk full")
			case "wide-type":
				request = saleIngressPacket(merchant.ID, 0x0101, 5)
			case "wide-position":
				request = saleIngressPacket(merchant.ID, placeInv, 0x0105)
			case "reserved-slot":
				request = saleIngressPacket(merchant.ID, placeInv, model.PlayerCarrySlots)
			}
			gold, inv := p.Char.Gold, p.Char.Inv
			if p.ShopNPC != 0 {
				t.Fatal("fixture unexpectedly opened an ordinary shop")
			}
			w.onSellItem(p.Session, request)
			if reason == "success" {
				if p.Char.Gold != gold+25000 || p.Char.Inv[5] != (model.Item{}) || st.saves != 1 {
					t.Fatal("native-panel sale did not commit the historical price")
				}
				assertDionysSalePublication(t, p)
			} else if p.Char.Gold != gold || p.Char.Inv != inv || reason != "save-failure" && st.saves != 0 {
				t.Fatal("rejected sale changed inventory/gold or persisted")
			}
		})
	}
}

func assertDionysSalePublication(t *testing.T, p *Player) {
	t.Helper()
	for _, expected := range []uint16{wire.OpSendItem, wire.OpUpdateEtc} {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != expected {
			t.Fatal("sale did not publish inventory then gold after commit")
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("unexpected extra sale response")
	}
}

func TestCriticalDionys759SalePublishesOnlyAfterCommit(t *testing.T) {
	w, p, fallback, merchant, backend, queue, _, release := criticalHellStoreFixture(t)
	attachDionysStore(w, p, merchant)
	w.items[400] = model.ItemDef{Index: 400, Price: 200000}
	p.Char.Inv[5] = model.Item{Index: 400, UID: "dionys-sale-item"}
	gold, inv := p.Char.Gold, p.Char.Inv
	request := saleIngressPacket(merchant.ID, placeInv, 5)
	w.onSellItem(p.Session, request)
	write := awaitCriticalWrite(t, backend.entered)
	proposal := write.accounts[0].Chars[p.CharSlot]
	if proposal.Inv[5] != (model.Item{}) || proposal.Gold != gold+25000 {
		t.Fatal("incorrect detached sale proposal")
	}
	w.onSellItem(p.Session, request)
	if p.Char.Gold != gold || p.Char.Inv != inv || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("sale changed live state or published before commit")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Gold != gold+25000 || p.Char.Inv[5] != (model.Item{}) {
		t.Fatal("committed sale missing")
	}
	assertDionysSalePublication(t, p)
}

func TestDionys759SaleFailureRetainsInventoryAndGold(t *testing.T) {
	w, p, fallback, merchant, backend, queue, _, release := criticalHellStoreFixture(t)
	attachDionysStore(w, p, merchant)
	backend.failure = errors.New("sale persistence rejected")
	w.items[400] = model.ItemDef{Index: 400, Price: 200000}
	p.Char.Inv[5] = model.Item{Index: 400, UID: "failed-sale"}
	gold, inv := p.Char.Gold, p.Char.Inv
	w.onSellItem(p.Session, saleIngressPacket(merchant.ID, placeInv, 5))
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Gold != gold || p.Char.Inv != inv || fallback.saves != 0 {
		t.Fatal("failed sale changed live inventory/gold or used inline persistence")
	}
	for p.Session.QueuedPacketsForTest() > 0 {
		packet, _ := p.Session.DequeuePacketForTest()
		if !wire.Decrypt(packet) {
			t.Fatal("invalid failure response")
		}
		if wire.ParseHeader(packet).Type == wire.OpUpdateEtc {
			t.Fatal("failed sale published success gold")
		}
	}
}

func TestHellStorePublicSaleRetainsQuarterPrice(t *testing.T) {
	w, p, st, merchant := hellStoreWorld(t)
	w.items[400] = model.ItemDef{Index: 400, Price: 200000}
	p.Char.Inv[5] = model.Item{Index: 400}
	gold := p.Char.Gold
	w.onSellItem(p.Session, saleIngressPacket(merchant.ID, placeInv, 5))
	if p.Char.Gold != gold+50000 || p.Char.Inv[5] != (model.Item{}) || st.saves != 1 {
		t.Fatal("public merchant changed its quarter-price economics")
	}
	assertDionysSalePublication(t, p)
}
