package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func configureCriticalGhostShop(t *testing.T, w *World, buyer, seller *Player, buyerSlot, sellerSlot int, stocked bool) *GhostShop {
	t.Helper()
	configureCriticalTrade(t, w, buyer, seller, buyerSlot, sellerSlot, "none")
	buyer.Trade, seller.Trade = nil, nil
	seller.Account.CargoGold = 900
	item := model.Item{Index: 4011, UID: "33333333333343338333333333333333",
		Eff: [6]byte{43, 9}, ActivatedUnix: 123, ExpiresUnix: 4102444800}
	seller.Account.Cargo[7] = item
	shop := &GhostShop{ID: ghostShopIDBase + seller.ID, OwnerID: seller.ID, X: 2101, Y: 2100,
		Title: "Staged Shop", Name: seller.Char.Name, CarryPos: emptyGhostShopPositions()}
	shop.Items[0], shop.CarryPos[0], shop.Prices[0] = item, 7, 250
	if stocked {
		shop.Items[1] = model.Item{Index: 4012, UID: "44444444444444448444444444444444"}
		shop.CarryPos[1], shop.Prices[1] = 8, 500
		seller.Account.Cargo[8] = shop.Items[1]
	}
	w.items[4011], w.items[4012] = model.ItemDef{Index: 4011}, model.ItemDef{Index: 4012}
	seller.GhostShop, buyer.BrowsingGhostShopID = shop, shop.ID
	w.registerGhostShop(shop)
	buyer.show(shop.ID)
	seller.show(shop.ID)
	return shop
}

func criticalGhostShopFixture(t *testing.T, buyerSlot, sellerSlot int, stocked bool) (*World, *Player, *Player, *GhostShop, *batchGameStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, buyer, seller, fallback, backend, queue, release := criticalTradeFixture(t, buyerSlot, sellerSlot, "none")
	shop := configureCriticalGhostShop(t, w, buyer, seller, buyerSlot, sellerSlot, stocked)
	return w, buyer, seller, shop, fallback, backend, queue, release
}

func criticalGhostShopRequest(buyer *Player, shop *GhostShop, pos int) []byte {
	packet := ghostBuyPacket(shop, pos)
	binary.LittleEndian.PutUint16(packet[0:2], uint16(len(packet)))
	binary.LittleEndian.PutUint16(packet[6:8], buyer.ID)
	return packet
}

func assertCriticalGhostPackets(t *testing.T, p *Player, expected ...[]byte) {
	t.Helper()
	if p.Session.IsClosed() {
		if p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("closed participant received purchase confirmation")
		}
		return
	}
	for _, want := range expected {
		got, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(got) || len(got) != len(want) {
			t.Fatal("missing or incorrectly sized ghost-shop packet")
		}
		header, wantHeader := wire.ParseHeader(got), wire.ParseHeader(want)
		if int(header.Size) != len(want) || header.ID != wantHeader.ID || header.Type != wantHeader.Type || !bytes.Equal(got[12:], want[12:]) {
			t.Fatalf("ghost-shop response order/recipient/body differs: got opcode %x, want %x", header.Type, wantHeader.Type)
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("ghost-shop purchase published extra responses")
	}
}

func assertCriticalGhostPublication(t *testing.T, buyer, seller *Player, shop *GhostShop, soldOut bool) {
	t.Helper()
	buyerPackets := [][]byte{wire.UpdateCarry(buyer.ID, buyer.Char.Inv[:], buyer.Char.Gold),
		wire.ItemSold(shop.ID, 0), wire.MessagePanel("Item 4011 purchased for 250 gold.")}
	sellerPackets := [][]byte{wire.SendItem(seller.ID, placeStorage, 7, model.Item{}),
		wire.UpdateCargoGold(wire.SceneField, seller.Account.CargoGold), wire.ItemSold(shop.ID, 0),
		wire.MessagePanel("Item 4011 sold for 250 gold.")}
	if soldOut {
		buyerPackets = append(buyerPackets, wire.CloseTrade(buyer.ID), wire.RemoveMob(shop.ID, 0))
		sellerPackets = append(sellerPackets, wire.RemoveMob(shop.ID, 0), wire.MessagePanel("Auto Trade closed: stock sold out."))
	}
	assertCriticalGhostPackets(t, buyer, buyerPackets...)
	assertCriticalGhostPackets(t, seller, sellerPackets...)
}

func TestCriticalGhostShopDetachedCommitAndSharedConsumers(t *testing.T) {
	for _, buyerSlot := range []int{0, 1} {
		for _, sellerSlot := range []int{0, 1} {
			for _, stocked := range []bool{false, true} {
				t.Run(fmt.Sprintf("buyer-%d-seller-%d-stocked-%t", buyerSlot, sellerSlot, stocked), func(t *testing.T) {
					w, buyer, seller, shop, fallback, backend, queue, release := criticalGhostShopFixture(t, buyerSlot, sellerSlot, stocked)
					third, _ := networkedTestPlayer(4, "ShopObserver", 2102, 2100)
					third.Char.Gold, third.BrowsingGhostShopID = 1000, shop.ID
					third.show(shop.ID)
					w.players[third.Session], w.playersByID[third.ID] = third, third
					w.updatePlayerSpatial(third)
					beforeBuyer, beforeSeller, beforeThird, beforeShop := accountStateSnapshot(buyer.Account), accountStateSnapshot(seller.Account), accountStateSnapshot(third.Account), *shop
					buyerAccount, buyerChar, sellerAccount, sellerChar := buyer.Account, buyer.Char, seller.Account, seller.Char
					request := criticalGhostShopRequest(buyer, shop, 0)
					w.onReqBuyAutoTrade(buyer.Session, request)
					proposal := awaitCriticalWrite(t, backend.entered)
					wantBuyer, wantSeller := accountStateSnapshot(beforeBuyer), accountStateSnapshot(beforeSeller)
					wantBuyer.Chars[buyerSlot].Inv[0], wantBuyer.Chars[buyerSlot].Gold = beforeShop.Items[0], 750
					wantSeller.Cargo[7], wantSeller.CargoGold = model.Item{}, 1150
					if len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts, accountPersistenceSnapshots(wantBuyer, wantSeller)) ||
						proposal.accounts[0] == buyer.Account || &proposal.accounts[0].Chars[buyerSlot] == buyer.Char || proposal.accounts[1] == seller.Account {
						t.Fatal("ghost-shop worker did not receive complete detached atomic account proposals")
					}
					assertHeld := func() {
						w.onReqBuyAutoTrade(buyer.Session, request)
						w.onReqBuyAutoTrade(third.Session, criticalGhostShopRequest(third, shop, 0))
						w.onAutoTrade(seller.Session, nil)
						w.closeGhostShop(seller, "competing close")
						w.onCharacterLogout(seller.Session, inboundPacket(wire.OpCharacterLogout, 12))
						// The logout rejection may only report that the save is held.
						for packet, ok := seller.Session.DequeuePacketForTest(); ok; packet, ok = seller.Session.DequeuePacketForTest() {
							if !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel {
								t.Fatal("reserved owner published a character or shop transition")
							}
						}
						if !reflect.DeepEqual(accountStateSnapshot(buyer.Account), beforeBuyer) || !reflect.DeepEqual(accountStateSnapshot(seller.Account), beforeSeller) ||
							!reflect.DeepEqual(accountStateSnapshot(third.Account), beforeThird) || *shop != beforeShop || seller.GhostShop != shop || w.ghostShops[shop.ID] != shop ||
							!w.criticalPlayerReserved(buyer) || !w.criticalPlayerReserved(seller) || buyer.Session.QueuedPacketsForTest() != 0 || third.Session.QueuedPacketsForTest() != 0 ||
							fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 || len(queue.pending) != 1 {
							t.Fatal("held shop transaction mutated accounts/listings, confirmed, repeated or saved inline")
						}
					}
					assertHeld()
					release()
					awaitReadyCriticalResults(t, queue, 1)
					assertHeld()
					w.processCriticalCompletions(1)
					if buyer.Account != buyerAccount || buyer.Char != buyerChar || seller.Account != sellerAccount || seller.Char != sellerChar ||
						!reflect.DeepEqual(accountStateSnapshot(buyer.Account), wantBuyer) || !reflect.DeepEqual(accountStateSnapshot(seller.Account), wantSeller) ||
						len(queue.pending) != 0 || shop.Items[0].Index != 0 || shop.CarryPos[0] != -1 || shop.Prices[0] != 0 || w.shuttingDown {
						t.Fatal("ghost-shop acknowledgement lost committed contents, identity or reservations")
					}
					if stocked && (seller.GhostShop != shop || w.ghostShops[shop.ID] != shop || shop.Items[1] != beforeShop.Items[1]) ||
						!stocked && (seller.GhostShop != nil || w.ghostShops[shop.ID] != nil || buyer.BrowsingGhostShopID != 0 || third.BrowsingGhostShopID != 0) {
						t.Fatal("post-ack shop cleanup changed remaining stock or retained a sold-out shop")
					}
					assertCriticalGhostPublication(t, buyer, seller, shop, !stocked)
					observerPackets := [][]byte{wire.ItemSold(shop.ID, 0)}
					if !stocked {
						observerPackets = append(observerPackets, wire.CloseTrade(third.ID), wire.RemoveMob(shop.ID, 0))
					}
					assertCriticalGhostPackets(t, third, observerPackets...)
					w.onReqBuyAutoTrade(buyer.Session, request)
					if len(queue.pending) != 0 || len(backend.entered) != 0 || buyer.Char.Gold != 750 {
						t.Fatal("stale purchase replayed the sold listing")
					}
				})
			}
		}
	}
}

func TestCriticalGhostShopRejectionsNeverMutateOrSaveInline(t *testing.T) {
	for _, reason := range []string{"unsupported", "full", "stopping", "unclaimed", "wrong-character", "buyer-gold", "seller-gold-cap", "inventory-full", "changed-cargo", "hidden-cargo", "unknown-item", "nontradable", "filled-seal", "distance", "browse", "price", "tax", "item", "malformed", "seller-closed", "seller-poisoned", "seller-outside", "buyer-closed", "buyer-poisoned", "shutdown"} {
		t.Run(reason, func(t *testing.T) {
			w, buyer, seller, shop, fallback, backend, queue, _ := criticalGhostShopFixture(t, 0, 1, true)
			switch reason {
			case "unsupported":
				queue.store = backend
			case "full":
				for _, name := range []string{"foreign-one", "foreign-two"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
					if name == "foreign-one" {
						awaitCriticalWrite(t, backend.entered)
					}
				}
			case "stopping":
				queue.closeAdmission()
			case "unclaimed":
				w.releaseAccountSession(seller.Session, seller.Account)
			case "wrong-character":
				copy := *seller.Char
				seller.Char = &copy
			case "buyer-gold":
				buyer.Char.Gold = 249
			case "seller-gold-cap":
				seller.Account.CargoGold = maxCharacterGold
			case "inventory-full":
				for i := 0; i < model.PlayerCarrySlots; i++ {
					buyer.Char.Inv[i] = model.Item{Index: 200}
				}
			case "changed-cargo":
				seller.Account.Cargo[7].UID = "different-item"
			case "hidden-cargo":
				shop.CarryPos[0] = model.PlayerCargoSlots
			case "unknown-item":
				delete(w.items, 4011)
			case "nontradable":
				w.items[4011] = model.ItemDef{Index: 4011, StaticEffects: []model.StaticEffect{{Name: "EF_NOTRADE", Value: 1}}}
			case "filled-seal":
				shop.Items[0] = model.NewCelestialSeal("55555555555545558555555555555555", 1)
				seller.Account.Cargo[7] = shop.Items[0]
				w.items[model.CelestialSealItem] = model.ItemDef{Index: model.CelestialSealItem}
			case "distance":
				buyer.X = 3000
			case "browse":
				buyer.BrowsingGhostShopID = 0
			case "seller-closed":
				seller.Session.Close()
			case "seller-poisoned":
				seller.PersistencePoisoned = true
			case "seller-outside":
				seller.InWorld = false
			case "buyer-closed":
				buyer.Session.Close()
			case "buyer-poisoned":
				buyer.PersistencePoisoned = true
			case "shutdown":
				w.shuttingDown = true
			}
			request := criticalGhostShopRequest(buyer, shop, 0)
			switch reason {
			case "price":
				binary.LittleEndian.PutUint32(request[20:24], 251)
			case "tax":
				binary.LittleEndian.PutUint32(request[24:28], 1)
			case "item":
				request[28]++
			case "malformed":
				request = request[:35]
			}
			beforeBuyer, beforeSeller, beforeShop, pending := accountStateSnapshot(buyer.Account), accountStateSnapshot(seller.Account), *shop, len(queue.pending)
			w.onReqBuyAutoTrade(buyer.Session, request)
			if !reflect.DeepEqual(accountStateSnapshot(buyer.Account), beforeBuyer) || !reflect.DeepEqual(accountStateSnapshot(seller.Account), beforeSeller) || *shop != beforeShop ||
				seller.GhostShop != shop || w.ghostShops[shop.ID] != shop || len(queue.pending) != pending || len(backend.entered) != 0 ||
				fallback.saves != 0 || fallback.batchSaves != 0 || len(w.criticalWrites.continuations) != 0 || seller.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("rejected shop purchase changed state, queued or synchronously saved")
			}
			if reason == "buyer-gold" {
				assertCriticalGhostPackets(t, buyer, wire.MessageIndexed(-845))
			}
			for packet, ok := buyer.Session.DequeuePacketForTest(); ok; packet, ok = buyer.Session.DequeuePacketForTest() {
				if !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel {
					t.Fatal("rejected purchase published economic success")
				}
			}
		})
	}
}

func TestCriticalGhostShopUnknownOutcomeRetainsOwnership(t *testing.T) {
	for _, reason := range []string{"error", "panic", "lifecycle", "shop-pointer", "registration", "listing", "browse"} {
		t.Run(reason, func(t *testing.T) {
			w, buyer, seller, shop, fallback, backend, queue, release := criticalGhostShopFixture(t, 1, 0, true)
			beforeBuyer, beforeSeller := accountStateSnapshot(buyer.Account), accountStateSnapshot(seller.Account)
			if reason == "error" {
				backend.failure = errors.New("shop commit outcome unknown")
			}
			backend.panicAfterWrite = reason == "panic"
			w.onReqBuyAutoTrade(buyer.Session, criticalGhostShopRequest(buyer, shop, 0))
			awaitCriticalWrite(t, backend.entered)
			switch reason {
			case "lifecycle":
				buyer.criticalLifecycle = nil
			case "shop-pointer":
				copy := *shop
				seller.GhostShop = &copy
			case "registration":
				copy := *shop
				w.ghostShops[shop.ID] = &copy
			case "listing":
				shop.Prices[1]++
			case "browse":
				buyer.BrowsingGhostShopID = 0
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if !w.shuttingDown || w.shutdownErr == nil || !queue.stopping || len(queue.pending) != 1 ||
				!buyer.PersistencePoisoned || !seller.PersistencePoisoned || !buyer.Session.IsClosed() || !seller.Session.IsClosed() ||
				!reflect.DeepEqual(accountStateSnapshot(buyer.Account), beforeBuyer) || !reflect.DeepEqual(accountStateSnapshot(seller.Account), beforeSeller) ||
				!w.criticalPlayerReserved(buyer) || !w.criticalPlayerReserved(seller) || buyer.Session.QueuedPacketsForTest() != 0 || seller.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("unknown shop outcome applied economics, confirmed or released ownership")
			}
			if w.claimAccountSession(gameNet.NewTestSession(90, 8), buyer.Account.Name) || w.claimAccountSession(gameNet.NewTestSession(91, 8), seller.Account.Name) {
				t.Fatal("uncertain shop result permitted account readmission")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 {
				t.Fatal("maintenance shutdown overwrote or replayed an uncertain shop result")
			}
		})
	}
}

func TestCriticalGhostShopAcceptsNewPurchaseAfterAcknowledgement(t *testing.T) {
	w, buyer, seller, shop, fallback, backend, queue, release := criticalGhostShopFixture(t, 0, 1, true)
	w.onReqBuyAutoTrade(buyer.Session, criticalGhostShopRequest(buyer, shop, 0))
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCriticalGhostPublication(t, buyer, seller, shop, false)
	secondItem := shop.Items[1]
	w.onReqBuyAutoTrade(buyer.Session, criticalGhostShopRequest(buyer, shop, 1))
	proposal := awaitCriticalWrite(t, backend.entered)
	if len(queue.pending) != 1 || buyer.Char.Gold != 750 || seller.Account.CargoGold != 1150 || shop.Items[1] != secondItem {
		t.Fatal("new purchase was suppressed after acknowledgement or applied before commit")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !reflect.DeepEqual(accountPersistenceSnapshots(buyer.Account, seller.Account), proposal.accounts) ||
		buyer.Char.Gold != 250 || seller.Account.CargoGold != 1650 || buyer.Char.Inv[1] != secondItem ||
		seller.Account.Cargo[8].Index != 0 || seller.GhostShop != nil || w.ghostShops[shop.ID] != nil ||
		len(queue.pending) != 0 || fallback.saves != 0 || fallback.batchSaves != 0 {
		t.Fatal("second committed purchase lost stock, account economics or ownership")
	}
	assertCriticalGhostPackets(t, buyer, wire.UpdateCarry(buyer.ID, buyer.Char.Inv[:], buyer.Char.Gold),
		wire.ItemSold(shop.ID, 1), wire.MessagePanel("Item 4012 purchased for 500 gold."),
		wire.CloseTrade(buyer.ID), wire.RemoveMob(shop.ID, 0))
	assertCriticalGhostPackets(t, seller, wire.SendItem(seller.ID, placeStorage, 8, model.Item{}),
		wire.UpdateCargoGold(wire.SceneField, 1650), wire.ItemSold(shop.ID, 1),
		wire.MessagePanel("Item 4012 sold for 500 gold."), wire.RemoveMob(shop.ID, 0),
		wire.MessagePanel("Auto Trade closed: stock sold out."))
}

func TestCriticalGhostShopRejectsReservedParticipantsAndReciprocalTradePeers(t *testing.T) {
	for _, role := range []string{"buyer", "seller"} {
		for _, peerReserved := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s-peer-%t", role, peerReserved), func(t *testing.T) {
				w, buyer, seller, shop, fallback, backend, queue, _ := criticalGhostShopFixture(t, 1, 0, true)
				participant := buyer
				if role == "seller" {
					participant = seller
				}
				reserved := participant
				if peerReserved {
					reserved, _ = networkedTestPlayer(4, "ReservedShopPeer", 2102, 2100)
					w.players[reserved.Session], w.playersByID[reserved.ID] = reserved, reserved
					participant.Trade = &TradeState{OpponentID: reserved.ID}
					reserved.Trade = &TradeState{OpponentID: participant.ID}
				}
				if _, err := queue.submit(criticalWrite{accounts: []*model.Account{reserved.Account}}); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, backend.entered)
				beforeBuyer, beforeSeller, beforeShop := accountStateSnapshot(buyer.Account), accountStateSnapshot(seller.Account), *shop
				var beforeTrade TradeState
				if participant.Trade != nil {
					beforeTrade = *participant.Trade
				}
				w.onReqBuyAutoTrade(buyer.Session, criticalGhostShopRequest(buyer, shop, 0))
				if !reflect.DeepEqual(accountStateSnapshot(buyer.Account), beforeBuyer) || !reflect.DeepEqual(accountStateSnapshot(seller.Account), beforeSeller) ||
					*shop != beforeShop || len(queue.pending) != 1 || len(backend.entered) != 0 || len(w.criticalWrites.continuations) != 0 ||
					fallback.saves != 0 || fallback.batchSaves != 0 || buyer.Session.QueuedPacketsForTest() != 0 || seller.Session.QueuedPacketsForTest() != 0 ||
					peerReserved && (participant.Trade == nil || *participant.Trade != beforeTrade || reserved.Trade == nil || reserved.Trade.OpponentID != participant.ID) {
					t.Fatal("shop purchase changed a reserved participant, reciprocal trade or shared listing")
				}
			})
		}
	}
}

func TestCriticalGhostShopDisconnectWaitsForCommittedFinalSnapshots(t *testing.T) {
	for _, closedSide := range []string{"buyer", "seller", "both"} {
		for _, stocked := range []bool{false, true} {
			t.Run(fmt.Sprintf("closed-%s-stocked-%t", closedSide, stocked), func(t *testing.T) {
				w, buyer, seller, shop, fallback, backend, queue, release := criticalGhostShopFixture(t, 0, 1, stocked)
				w.onReqBuyAutoTrade(buyer.Session, criticalGhostShopRequest(buyer, shop, 0))
				proposal := awaitCriticalWrite(t, backend.entered)
				var closed []*Player
				if closedSide != "seller" {
					closed = append(closed, buyer)
				}
				if closedSide != "buyer" {
					closed = append(closed, seller)
				}
				for _, p := range closed {
					w.handle(command{s: p.Session})
				}
				if seller.GhostShop != shop || shop.Items[0].Index == 0 || fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 {
					t.Fatal("disconnect removed pending shop context or saved old economics")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				assertCriticalGhostPublication(t, buyer, seller, shop, !stocked)
				for range closed {
					final := awaitCriticalWrite(t, backend.entered)
					var p *Player
					var want *model.Account
					for i, candidate := range []*Player{buyer, seller} {
						if candidate.Account.Name == final.accounts[0].Name {
							p, want = candidate, proposal.accounts[i]
						}
					}
					if p == nil || !reflect.DeepEqual(final.accounts[0], want) || final.character == nil ||
						final.character.uid != p.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
						t.Fatal("final disconnect save lost committed shop economics or sidecar")
					}
				}
				awaitReadyCriticalResults(t, queue, len(closed))
				for _, p := range closed {
					if !w.criticalPlayerReserved(p) || w.claimAccountSession(gameNet.NewTestSession(90, 8), p.Account.Name) {
						t.Fatal("disconnect released ownership before final acknowledgement")
					}
				}
				w.processCriticalCompletions(len(closed))
				for i, p := range closed {
					if w.players[p.Session] != nil || p.Session.QueuedPacketsForTest() != 0 || !w.claimAccountSession(gameNet.NewTestSession(int64(90+i), 8), p.Account.Name) {
						t.Fatal("disconnect did not clean up and release committed ownership")
					}
				}
				if closedSide != "buyer" && (seller.GhostShop != nil || w.ghostShops[shop.ID] != nil) ||
					len(queue.pending) != 0 || fallback.saves != 0 || fallback.batchSaves != 0 {
					t.Fatal("seller disconnect retained an orphan shop or saved inline")
				}
			})
		}
	}
}

func TestCriticalGhostShopShutdownWaitsForBothAccountResults(t *testing.T) {
	w, buyer, seller, shop, fallback, backend, queue, release := criticalGhostShopFixture(t, 1, 0, false)
	terminal := &criticalTradeTerminalStore{batchGameStore: fallback, accounts: make(map[string]*model.Account)}
	w.store = terminal
	w.onReqBuyAutoTrade(buyer.Session, criticalGhostShopRequest(buyer, shop, 0))
	proposal := awaitCriticalWrite(t, backend.entered)
	first, second := make(chan error, 1), make(chan error, 1)
	w.runShutdown(first)
	w.runShutdown(second)
	// Session transports close through disconnect commands, not runShutdown.
	for _, p := range []*Player{buyer, seller} {
		w.handle(command{s: p.Session})
		if !p.Session.IsClosed() {
			t.Fatal("shutdown disconnect retained an open session transport")
		}
	}
	if len(terminal.accounts) != 0 || terminal.flushes != 0 || w.shutdownFinished {
		t.Fatal("shutdown saved old shop economics or finished before pending transaction")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	for _, done := range []chan error{first, second} {
		select {
		case err := <-done:
			if err != nil {
				t.Fatal(err)
			}
		default:
			t.Fatal("shutdown did not settle the accepted shop transaction")
		}
	}
	for _, want := range proposal.accounts {
		if !reflect.DeepEqual(accountPersistenceSnapshot(terminal.accounts[want.Name]), want) {
			t.Fatal("shutdown snapshot lost committed shop items or gold")
		}
	}
	third := make(chan error, 1)
	w.runShutdown(third)
	if err := <-third; err != nil || fallback.saves != 2 || terminal.flushes != 1 || len(queue.pending) != 0 ||
		buyer.Session.QueuedPacketsForTest() != 0 || seller.Session.QueuedPacketsForTest() != 0 || seller.GhostShop != nil || w.ghostShops[shop.ID] != nil {
		t.Fatal("repeated shutdown saved again, confirmed to closed sessions or retained a sold-out shop")
	}
}

func TestCriticalGhostShopActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, failed := range []bool{false, true} {
		t.Run(fmt.Sprintf("unknown-%t", failed), func(t *testing.T) {
			var seller *Player
			var shop *GhostShop
			var failure error
			if failed {
				failure = errors.New("shop durable outcome unknown")
			}
			w, buyer, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, buyer *Player) {
				seller, _ = networkedTestPlayer(3, "GhostLoopSeller", 2101, 2100)
				w.players[seller.Session], w.playersByID[seller.ID] = seller, seller
				w.indexPlayerCharacter(seller)
				w.updatePlayerSpatial(seller)
				if !w.claimAccountSession(seller.Session, seller.Account.Name) {
					t.Fatal("loop shop seller admission failed")
				}
				shop = configureCriticalGhostShop(t, w, buyer, seller, 1, 0, true)
				seller.NextCPRecovery, seller.NextRegen = w.now().Add(time.Hour), w.now().Add(time.Hour)
				w.criticalWrites.queue.store = &criticalStateTestStore{&criticalBatchTestStore{w.criticalWrites.queue.store.(*criticalTestStore)}}
			})
			beforeBuyer, beforeSeller, beforeShop := accountStateSnapshot(buyer.Account), accountStateSnapshot(seller.Account), *shop
			request := criticalGhostShopRequest(buyer, shop, 0)
			deliver := func() {
				select {
				case w.commands <- command{s: buyer.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept ghost-shop intention")
				}
			}
			deliver()
			proposal := awaitCriticalWrite(t, backend.entered)
			for range 3 {
				deliver()
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, p := range overview.Players {
					if p.UID == unrelatedUID && p.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || buyer.Session.QueuedPacketsForTest() != 0 || seller.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("held shop purchase stalled World, repeated persistence or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
				if failed && buyer.Session.IsClosed() || !failed && buyer.Session.QueuedPacketsForTest() >= 3 && seller.Session.QueuedPacketsForTest() >= 4 {
					break
				}
			}
			stop() // Join World before reading mutable accounts and reservations.
			if w.mobTickCounter < 3 {
				t.Fatal("World did not advance ticks during held shop persistence")
			}
			if failed {
				if !w.shuttingDown || len(queue.pending) != 1 || !buyer.PersistencePoisoned || !seller.PersistencePoisoned || *shop != beforeShop ||
					!reflect.DeepEqual(accountStateSnapshot(buyer.Account), beforeBuyer) || !reflect.DeepEqual(accountStateSnapshot(seller.Account), beforeSeller) ||
					buyer.Session.QueuedPacketsForTest() != 0 || seller.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("actual World failed to contain uncertain shop persistence")
				}
			} else {
				if w.shuttingDown || len(queue.pending) != 0 || !reflect.DeepEqual(accountPersistenceSnapshots(buyer.Account, seller.Account), proposal.accounts) || seller.GhostShop != shop {
					t.Fatal("actual dispatcher failed to commit and acknowledge both shop accounts")
				}
				assertCriticalGhostPublication(t, buyer, seller, shop, false)
			}
		})
	}
}
