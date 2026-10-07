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

var criticalShopKinds = []string{"buy", "sale", "toto"}

func criticalShopRequest(kind string, id, merchant uint16) []byte {
	var packet []byte
	switch kind {
	case "buy":
		packet = inboundPacket(wire.OpBuyItem, 24)
		binary.LittleEndian.PutUint16(packet[12:14], merchant)
	case "sale":
		packet = saleIngressPacket(merchant, placeInv, 5)
	case "toto":
		packet = totoBuyPacket(merchant, 0, 5, 1, 80, 12, 127)
	}
	binary.LittleEndian.PutUint16(packet[6:8], id)
	return packet
}

func callCriticalShop(w *World, p *Player, kind string, packet []byte) {
	switch kind {
	case "buy":
		w.onBuyItem(p.Session, packet)
	case "sale":
		w.onSellItem(p.Session, packet)
	case "toto":
		w.onBuyToto(p.Session, packet)
	}
}

func configureCriticalShop(w *World, p *Player, shop *Mob, kind string) {
	w.items = map[uint16]model.ItemDef{400: {Index: 400, Price: 1000}, 4147: {Index: 4147, Price: 3000}}
	shop.Def.Score.Merchant = nativeShopMerchant
	index := uint16(400)
	if kind == "toto" {
		index = 4147
	}
	// The request addresses the compacted display, not raw Vende[0].
	shop.Def.Vende = []model.Item{{}, {Index: index, Eff: [6]byte{61, 7}}}
	p.ShopNPC, p.Char.Gold = shop.ID, 10000
	p.Char.Inv[0] = model.Item{Index: 200, UID: "unrelated-visible-item"}
	p.Char.Inv[model.PlayerCarrySlots] = model.Item{Index: 300, UID: "hidden-abi-item"}
	if kind == "sale" {
		p.Char.Inv[5] = model.Item{Index: 400, UID: "sold-item", Eff: [6]byte{61, 7}}
	}
}

func criticalShopFixture(t *testing.T, kind string) (*World, *Player, *craftStore, *Mob, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, fallback := saleIngressWorld(t)
	shop := w.mobsByID[p.ShopNPC]
	configureCriticalShop(w, p, shop, kind)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, shop, backend, queue, release
}

func criticalShopDestination(kind string) int {
	if kind == "buy" {
		return 1
	}
	return 5
}

func criticalShopGold(kind string) uint32 {
	switch kind {
	case "buy":
		return 9000
	case "sale":
		return 10250
	default:
		return 7000
	}
}

func assertCriticalShopPublication(t *testing.T, p *Player, kind string, merchant uint16) {
	t.Helper()
	var expected [][]byte
	switch kind {
	case "buy":
		expected = [][]byte{wire.BuyItem(p.ID, merchant, 0, 1, p.Char.Gold)}
	case "sale":
		expected = [][]byte{wire.SendItem(p.ID, placeInv, 5, p.Char.Inv[5]), wire.UpdateEtc(p.ID, *p.Char)}
	case "toto":
		expected = [][]byte{wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold), wire.UpdateEtc(p.ID, *p.Char)}
	}
	for _, want := range expected {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || len(packet) != len(want) {
			t.Fatal("missing or incorrectly sized merchant response")
		}
		header, expectedHeader := wire.ParseHeader(packet), wire.ParseHeader(want)
		if header.ID != expectedHeader.ID || header.Type != expectedHeader.Type ||
			int(header.Size) != len(want) || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatal("merchant response recipient, opcode, payload or ordering differs from committed state")
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("merchant transaction published an extra response")
	}
}

func TestCriticalShopDetachedCommitAndRepeatedRequests(t *testing.T) {
	for _, kind := range criticalShopKinds {
		for _, slot := range []int{0, 1} {
			t.Run(fmt.Sprintf("%s/character-%d", kind, slot), func(t *testing.T) {
				w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, kind)
				if slot == 1 {
					p.Account.Chars = append(p.Account.Chars, cloneCharacterState(p.Char))
					p.CharSlot, p.Char = 1, &p.Account.Chars[1]
					p.Char.UID += "-second"
					w.indexPlayerCharacter(p)
				}
				before, account, character, score := accountStateSnapshot(p.Account), p.Account, p.Char, p.Char.Score
				request := criticalShopRequest(kind, p.ID, shop.ID)
				w.handle(command{s: p.Session, pkt: request})
				write := awaitCriticalWrite(t, backend.entered)
				dst, gold := criticalShopDestination(kind), criticalShopGold(kind)
				item := write.accounts[0].Chars[slot].Inv[dst]
				want := accountPersistenceSnapshot(before)
				want.Chars[slot].Inv[dst], want.Chars[slot].Gold = item, gold
				if write.accounts[0] == account || write.character != nil || write.instances != nil || !reflect.DeepEqual(write.accounts[0], want) {
					t.Fatal("merchant transaction did not submit only the detached intended item and gold change")
				}
				if kind == "sale" {
					if item != (model.Item{}) {
						t.Fatal("sale did not remove the item in its proposal")
					}
				} else if item.UID == "" || (kind == "buy" && (item.Index != 400 || item.Eff != [6]byte{61, 7})) ||
					(kind == "toto" && (item.Index != 4147 || item.Eff != [6]byte{64, 80, 65, 12, 66, 127})) {
					t.Fatal("purchase lost catalog bundle data or authoritative persistent ticket identity")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					callCriticalShop(w, p, kind, request)
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
						p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 || !w.criticalPlayerReserved(p) {
						t.Fatalf("%s merchant transaction mutated, confirmed or repeated before acknowledgement", phase)
					}
				}
				w.processCriticalCompletions(1)
				wantLive := accountStateSnapshot(before)
				wantLive.Chars[slot].Inv[dst], wantLive.Chars[slot].Gold = item, gold
				if p.Account != account || p.Char != character || p.Char.Score != score ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), wantLive) || fallback.saves != 0 || len(queue.pending) != 0 {
					t.Fatal("merchant acknowledgement replaced identity or unrelated fields, wrote inline or lost committed state")
				}
				assertCriticalShopPublication(t, p, kind, shop.ID)
				callCriticalShop(w, p, kind, request)
				if kind == "buy" {
					next := awaitCriticalWrite(t, backend.entered)
					if next.accounts[0].Chars[slot].Inv[2].UID == item.UID || next.accounts[0].Chars[slot].Inv[2].Index != 400 ||
						next.accounts[0].Chars[slot].Gold != gold-1000 || p.Char.Inv[2].Index != 0 || p.Char.Gold != gold {
						t.Fatal("post-acknowledgement buy did not represent a new independent purchase")
					}
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
				} else if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Char.Gold != gold || p.Char.Inv[dst] != item {
					t.Fatal("repeated sale/occupied-slot ticket duplicated the economic operation")
				}
			})
		}
	}
}

func TestCriticalShopAdmissionAndDomainRejections(t *testing.T) {
	for _, kind := range criticalShopKinds {
		for _, reason := range []string{"full", "stopping", "world-stopping", "unclaimed", "slot-pointer", "closed", "poisoned",
			"dead", "out-of-world", "hidden", "distant", "other-merchant", "no-shop", "truncated", "oversized", "balance", "slot"} {
			t.Run(kind+"/"+reason, func(t *testing.T) {
				w, p, fallback, shop, backend, queue, _ := criticalShopFixture(t, kind)
				request := criticalShopRequest(kind, p.ID, shop.ID)
				switch reason {
				case "full":
					for _, name := range []string{"occupied-a", "occupied-b"} {
						if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
							t.Fatal(err)
						}
					}
					awaitCriticalWrite(t, backend.entered)
				case "stopping":
					queue.closeAdmission()
				case "world-stopping":
					w.shuttingDown = true
				case "unclaimed":
					delete(w.accountSessions, accountSessionKey(p.Account.Name))
				case "slot-pointer":
					copy := cloneCharacterState(p.Char)
					p.Char = &copy
				case "closed":
					p.Session.Close()
				case "poisoned":
					p.PersistencePoisoned = true
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "out-of-world":
					p.InWorld = false
				case "hidden":
					delete(p.Visible, shop.ID)
				case "distant":
					p.X = shop.X + npcInteractionRange + 1
				case "other-merchant":
					binary.LittleEndian.PutUint16(request[12:14], shop.ID+1)
				case "no-shop":
					p.ShopNPC = 0
				case "truncated":
					request = request[:len(request)-1]
				case "oversized":
					request = append(request, 0)
				case "balance":
					p.Char.Gold = 0
					if kind == "sale" {
						p.Char.Gold = maxCharacterGold
					}
				case "slot":
					if kind == "buy" {
						for i := 0; i < model.PlayerCarrySlots; i++ {
							p.Char.Inv[i] = model.Item{Index: 200}
						}
					} else if kind == "toto" {
						p.Char.Inv[5] = model.Item{Index: 200}
					} else {
						request[16] = model.PlayerCarrySlots
					}
				}
				before := accountStateSnapshot(p.Account)
				callCriticalShop(w, p, kind, request)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
					len(backend.entered) != 0 || p.criticalLifecycle != nil || w.criticalPlayerReserved(p) ||
					(reason != "full" && len(queue.pending) != 0) {
					t.Fatal("rejected merchant transaction mutated state, saved inline or retained work for replay")
				}
				for p.Session.QueuedPacketsForTest() > 0 {
					packet, _ := p.Session.DequeuePacketForTest()
					if !wire.Decrypt(packet) {
						t.Fatal("invalid rejection response")
					}
					if opcode := wire.ParseHeader(packet).Type; opcode != wire.OpMessagePanel && opcode != wire.OpSendItem {
						t.Fatal("rejected merchant transaction published a success response")
					}
				}
			})
		}
	}
}

func TestCriticalShopUnknownOutcomeNeverPublishesOrOverwrites(t *testing.T) {
	for _, kind := range criticalShopKinds {
		for _, reason := range []string{"error", "panic", "identity"} {
			t.Run(kind+"/"+reason, func(t *testing.T) {
				w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, kind)
				if reason == "error" {
					backend.failure = errors.New("commit acknowledgement lost")
				}
				backend.panicAfterWrite = reason == "panic"
				before := accountStateSnapshot(p.Account)
				request := criticalShopRequest(kind, p.ID, shop.ID)
				callCriticalShop(w, p, kind, request)
				awaitCriticalWrite(t, backend.entered)
				w.handle(command{s: p.Session})
				if reason == "identity" {
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				callCriticalShop(w, p, kind, request)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
					!w.shuttingDown || !p.PersistencePoisoned || !w.criticalPlayerReserved(p) || !p.Session.IsClosed() ||
					p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 ||
					w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
					t.Fatal("uncertain merchant transaction published, released ownership, applied a proposal or replayed")
				}
				done := make(chan error, 1)
				w.runShutdown(done)
				if err := <-done; err == nil || fallback.saves != 0 {
					t.Fatal("terminal snapshot overwrote an uncertain merchant transaction")
				}
			})
		}
	}
}

func TestCriticalShopDisconnectFinalSnapshotContainsCommittedEconomy(t *testing.T) {
	for _, kind := range criticalShopKinds {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, kind)
			queue.store = &criticalStateTestStore{&criticalBatchTestStore{backend}}
			callCriticalShop(w, p, kind, criticalShopRequest(kind, p.ID, shop.ID))
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if len(backend.entered) != 0 || fallback.saves != 0 || p.Char.Gold != 10000 {
				t.Fatal("disconnect persisted stale/proposed merchant balances")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(final.accounts, proposal.accounts) || final.character == nil ||
				final.character.uid != p.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) ||
				p.Session.QueuedPacketsForTest() != 0 || p.Char.Gold != criticalShopGold(kind) || fallback.saves != 0 ||
				w.claimAccountSession(gameNet.NewTestSession(98, 8), p.Account.Name) {
				t.Fatal("final disconnect lost committed merchant state or allowed early readmission")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.handle(command{s: p.Session})
			if len(backend.entered) != 0 || !w.criticalPlayerReserved(p) {
				t.Fatal("unacknowledged final merchant snapshot replayed or released its fence")
			}
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || len(queue.pending) != 0 || fallback.saves != 0 ||
				!w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
				t.Fatal("final acknowledgement did not release merchant account ownership")
			}
		})
	}
}

func TestCriticalShopActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, kind := range criticalShopKinds {
		t.Run(kind, func(t *testing.T) {
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil,
				func(w *World, p *Player) { configureCriticalShop(w, p, w.mobsByID[1102], kind) })
			request := criticalShopRequest(kind, 1, 1102)
			deliver := func() {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept the merchant intention")
				}
			}
			deliver()
			awaitCriticalWrite(t, backend.entered)
			for range 3 {
				deliverCriticalLoopPing(t, w, p)
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, other := range overview.Players {
					if other.UID == unrelatedUID && other.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("held merchant transaction stalled unrelated commands/ticks, repeated or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for p.Session.QueuedPacketsForTest() == 0 && time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
			}
			stop()
			if p.Char.Gold != criticalShopGold(kind) || len(queue.pending) != 0 || w.shuttingDown || w.mobTickCounter < 3 {
				t.Fatal("actual dispatcher failed to deliver the merchant commit while advancing the world")
			}
			assertCriticalShopPublication(t, p, kind, 1102)
		})
	}
}

func TestCriticalShopReservedTradeCounterpartRetainsContext(t *testing.T) {
	for _, kind := range criticalShopKinds {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, kind)
			opponent, _ := networkedTestPlayer(2, "TradePeer", 2101, 2100)
			w.players[opponent.Session], w.playersByID[opponent.ID] = opponent, opponent
			w.indexPlayerCharacter(opponent)
			if !w.claimAccountSession(opponent.Session, opponent.Account.Name) {
				t.Fatal("trade counterpart account admission failed")
			}
			p.Trade, opponent.Trade = &TradeState{OpponentID: opponent.ID}, &TradeState{OpponentID: p.ID}
			trade, peerTrade := p.Trade, opponent.Trade
			if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{opponent.Account}}, []*Player{opponent},
				func(criticalPersistenceResult) error { return nil }); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			before := accountStateSnapshot(p.Account)
			request := criticalShopRequest(kind, p.ID, shop.ID)
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				callCriticalShop(w, p, kind, request)
				if p.Trade != trade || opponent.Trade != peerTrade || p.ShopNPC != shop.ID ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || opponent.Session.QueuedPacketsForTest() != 0 ||
					len(queue.pending) != 1 || len(backend.entered) != 0 {
					t.Fatalf("%s reserved trade counterpart was canceled or admitted a merchant transaction", phase)
				}
			}
			w.processCriticalCompletions(1)
			callCriticalShop(w, p, kind, request)
			awaitCriticalWrite(t, backend.entered)
			if p.Trade != nil || opponent.Trade != nil || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("eligible merchant transaction did not preserve trade cancellation and detached planning")
			}
			for _, player := range []*Player{p, opponent} {
				packet, ok := player.Session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpCloseTrade ||
					player.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("eligible trade cancellation must publish only the existing close packet before commit")
				}
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			assertCriticalShopPublication(t, p, kind, shop.ID)
		})
	}
}

func TestCriticalShopShutdownPersistsCommittedEconomyAfterDrain(t *testing.T) {
	for _, kind := range criticalShopKinds {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, kind)
			terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
			w.store = terminal
			before := accountStateSnapshot(p.Account)
			callCriticalShop(w, p, kind, criticalShopRequest(kind, p.ID, shop.ID))
			proposal := awaitCriticalWrite(t, backend.entered)
			first, second := make(chan error, 1), make(chan error, 1)
			w.runShutdown(first)
			w.runShutdown(second)
			w.handle(command{s: p.Session})
			if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 ||
				!queue.stopping || w.shutdownFinished || !p.Session.IsClosed() ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("shutdown persisted stale/proposed merchant state before acknowledgement")
			}
			for _, done := range []chan error{first, second} {
				select {
				case <-done:
					t.Fatal("shutdown finished with an accepted merchant write still held")
				default:
				}
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
					t.Fatal("shutdown did not finish after merchant acknowledgement")
				}
			}
			if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
				p.Char.Gold != criticalShopGold(kind) || fallback.saves != 1 || terminal.flushes != 1 ||
				!w.shutdownFinished || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("terminal snapshot lost committed merchant state, repeated a save or published to a closed session")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed the terminal merchant snapshot")
			}
		})
	}
}

func TestCriticalShopTaxedPathRejectsWithoutInlineFallback(t *testing.T) {
	for _, kind := range criticalShopKinds {
		t.Run(kind, func(t *testing.T) {
			index := uint16(400)
			if kind == "toto" {
				index = 4147
			}
			w, fallback, p, shop := cityEconomyFixture(t, model.ItemDef{Index: index, Price: 1000})
			configureCriticalShop(w, p, shop, kind)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("fixture account admission failed")
			}
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
			queue := newCriticalTestQueue(t, backend, 1, 1)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			before, guilds := accountStateSnapshot(p.Account), cloneGuildRegistry(w.guilds)
			if _, treasury, _, _ := w.cityPriceForNPC(shop, 1000); treasury == 0 {
				t.Fatal("fixture must exercise a nonzero city treasury transaction")
			}
			callCriticalShop(w, p, kind, criticalShopRequest(kind, p.ID, shop.ID))
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.guilds, guilds) ||
				fallback.saves != 0 || fallback.gameSaves != 0 || len(queue.pending) != 0 || len(backend.entered) != 0 ||
				w.criticalPlayerReserved(p) || p.criticalLifecycle != nil {
				t.Fatal("unmigrated taxed merchant transaction changed an aggregate or fell back to inline persistence")
			}
			responses := 0
			for p.Session.QueuedPacketsForTest() > 0 {
				packet, _ := p.Session.DequeuePacketForTest()
				if !wire.Decrypt(packet) {
					t.Fatal("invalid taxed-path rejection response")
				}
				opcode := wire.ParseHeader(packet).Type
				if opcode != wire.OpMessagePanel && !(kind == "sale" && opcode == wire.OpSendItem) {
					t.Fatal("unmigrated taxed merchant transaction published success")
				}
				if opcode == wire.OpSendItem && !bytes.Equal(packet[12:], wire.SendItem(p.ID, placeInv, 5, p.Char.Inv[5])[12:]) {
					t.Fatal("rejected sale removed the live item in its resynchronization response")
				}
				responses++
			}
			want := 1
			if kind == "sale" {
				want = 2
			}
			if responses != want {
				t.Fatal("unmigrated taxed merchant transaction lacked its terminal rejection response")
			}
		})
	}
}
