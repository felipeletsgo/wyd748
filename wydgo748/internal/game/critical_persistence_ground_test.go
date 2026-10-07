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

func criticalPickupRequest(id uint16, slot int) []byte {
	packet := inboundPacket(wire.OpGetItem, 28)
	binary.LittleEndian.PutUint32(packet[12:16], uint32(placeInv))
	binary.LittleEndian.PutUint32(packet[16:20], uint32(slot))
	binary.LittleEndian.PutUint16(packet[20:22], id)
	return packet
}

func criticalPickupFixture(t *testing.T) (*World, *Player, *GroundItem, *craftStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 0, 0)
	g := &GroundItem{ID: 10001, Item: p.Char.Inv[0], X: p.X, Y: p.Y, Expire: time.Unix(100, 0)}
	p.Char.Inv[0] = model.Item{}
	w.registerGroundItem(g)
	return w, p, g, fallback, backend, queue, release
}

func criticalPickupCompetitor(t *testing.T, w *World, g *GroundItem) *Player {
	t.Helper()
	p, _ := networkedTestPlayer(2, "Competitor", g.X, g.Y)
	w.players[p.Session], w.playersByID[p.ID] = p, p
	w.indexPlayerCharacter(p)
	w.updatePlayerSpatial(p)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("competitor account admission failed")
	}
	p.show(g.ID)
	return p
}

func configureCriticalGroundCurrency(w *World, g *GroundItem, value uint32) {
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	w.items[g.Item.Index] = model.ItemDef{Price: value}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{g.Item.Index: {Action: "ground_gold", ValueSource: "price"}},
		ItemCodes: map[uint16]int{g.Item.Index: 1},
	}
}

func assertCriticalPickupPacket(t *testing.T, p *Player, want []byte) {
	t.Helper()
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || len(packet) != len(want) ||
		wire.ParseHeader(packet).Type != wire.ParseHeader(want).Type ||
		wire.ParseHeader(packet).ID != wire.ParseHeader(want).ID || !bytes.Equal(packet[12:], want[12:]) {
		t.Fatal("pickup confirmation size, opcode, recipient, order or body differs from committed state")
	}
}

func TestCriticalGroundPickupDetachedCommitAndSharedReservation(t *testing.T) {
	for _, kind := range []string{"preferred", "fallback", "currency"} {
		for charSlot := range 4 {
			t.Run(fmt.Sprintf("%s/character-%d", kind, charSlot), func(t *testing.T) {
				w, p, g, fallback, backend, queue, release := criticalPickupFixture(t)
				if charSlot != 0 {
					p.Account.Chars[0], p.Account.Chars[charSlot] = p.Account.Chars[charSlot], p.Account.Chars[0]
					p.CharSlot, p.Char = charSlot, &p.Account.Chars[charSlot]
				}
				competitor := criticalPickupCompetitor(t, w, g)
				const requested = 7
				slot := requested
				if kind == "fallback" {
					p.Char.Inv[requested] = model.Item{Index: 201, UID: "occupied-slot"}
					slot = 0
				}
				if kind == "currency" {
					configureCriticalGroundCurrency(w, g, 400)
					// Currency does not require an empty inventory slot.
					for i := range model.PlayerCarrySlots {
						p.Char.Inv[i] = model.Item{Index: 201}
					}
				}
				before := accountStateSnapshot(p.Account)
				account, character := p.Account, p.Char
				request := criticalPickupRequest(g.ID, requested)
				w.handle(command{s: p.Session, pkt: request})
				write := awaitCriticalWrite(t, backend.entered)
				want := accountStateSnapshot(before)
				if kind == "currency" {
					want.Chars[charSlot].Gold += 400
				} else {
					want.Chars[charSlot].Inv[slot] = g.Item
				}
				if write.accounts[0] == account || !reflect.DeepEqual(write.accounts[0], accountPersistenceSnapshot(want)) {
					t.Fatal("pickup proposal aliases live state or changes unrelated economic fields")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.handle(command{s: p.Session, pkt: request})
					w.onGetItem(p.Session, request)
					w.handle(command{s: competitor.Session, pkt: request})
					w.onGetItem(competitor.Session, request)
					w.expireGroundItems(time.Unix(200, 0))
					w.publishItemRemove(g)
					w.unregisterGroundItem(g)
					replacement := *g
					replacement.criticalPickup = 0
					w.registerGroundItem(&replacement)
					if !reflect.DeepEqual(accountStateSnapshot(account), before) || fallback.saves != 0 ||
						w.groundItems[g.ID] != g || g.criticalPickup == 0 || len(queue.pending) != 1 ||
						len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 ||
						competitor.Session.QueuedPacketsForTest() != 0 || !competitor.hasVisible(g.ID) {
						t.Fatalf("%s changed, removed, confirmed or repeated reserved pickup", phase)
					}
				}
				w.processCriticalCompletions(1)
				if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) ||
					w.groundItems[g.ID] != nil || g.criticalPickup != 0 || len(queue.pending) != 0 ||
					w.criticalPlayerReserved(p) || fallback.saves != 0 || competitor.hasVisible(g.ID) {
					t.Fatal("pickup failed to commit the exact proposal and release both ownerships")
				}
				assertCriticalPickupPacket(t, competitor, wire.RemoveItem(uint32(g.ID)))
				if kind == "currency" {
					assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
				} else {
					assertCriticalPickupPacket(t, p, wire.CNFGetItem(placeInv, uint32(slot)))
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, byte(slot), g.Item))
				}
				w.handle(command{s: p.Session, pkt: request})
				w.handle(command{s: competitor.Session, pkt: request})
				if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 ||
					competitor.Session.QueuedPacketsForTest() != 0 || competitor.Char.Inv[requested].Index != 0 {
					t.Fatal("removed ground object was picked up or confirmed twice")
				}
			})
		}
	}
}

func TestCriticalGroundPickupExpiryDoesNotBlockOtherDueItems(t *testing.T) {
	w, p, g, _, backend, queue, release := criticalPickupFixture(t)
	w.onGetItem(p.Session, criticalPickupRequest(g.ID, 7))
	awaitCriticalWrite(t, backend.entered)
	other := &GroundItem{ID: 10002, Item: model.Item{Index: 201}, X: g.X, Y: g.Y, Expire: g.Expire.Add(time.Second)}
	w.registerGroundItem(other)
	for range 3 {
		w.expireGroundItems(g.Expire.Add(time.Hour))
		if w.groundItems[g.ID] != g || w.groundItems[other.ID] != nil || len(w.groundExpiry) != 1 ||
			!w.groundExpiryByID[g.ID].Equal(g.Expire) || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("reserved expiry blocked unrelated loot or changed its deadline/index")
		}
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	w.expireGroundItems(g.Expire.Add(time.Hour))
	if len(w.groundExpiry) != 0 || len(w.groundExpiryByID) != 0 || len(w.groundItems) != 0 {
		t.Fatal("pickup acknowledgement left a stale canonical or expiry entry")
	}
}

func TestCriticalGroundPickupAdmissionRejectionDoesNotSaveOrReserve(t *testing.T) {
	for _, scenario := range []string{"full", "busy", "stopping", "closed", "identity", "permanent", "distance", "instance", "full-inventory", "zero-currency", "overflow", "invalid-slot", "short"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, g, fallback, backend, queue, _ := criticalPickupFixture(t)
			request := criticalPickupRequest(g.ID, 7)
			switch scenario {
			case "full", "busy":
				for i := range 2 {
					account := &model.Account{Name: fmt.Sprintf("occupied-%d", i)}
					if scenario == "busy" {
						account = p.Account
					}
					if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{account}}, func(criticalPersistenceResult) error { return nil }); err != nil {
						t.Fatal(err)
					}
					if i == 0 {
						awaitCriticalWrite(t, backend.entered)
					}
					if scenario == "busy" {
						break
					}
				}
			case "stopping":
				queue.closeAdmission()
			case "closed":
				p.Session.Close()
			case "identity":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "permanent":
				g.Permanent = true
			case "distance":
				g.X += pickupRange + 1
			case "instance":
				g.InstanceID = "other-instance"
			case "full-inventory":
				for i := range model.PlayerCarrySlots {
					p.Char.Inv[i] = model.Item{Index: 201}
				}
			case "zero-currency":
				configureCriticalGroundCurrency(w, g, 0)
			case "overflow":
				configureCriticalGroundCurrency(w, g, 400)
				p.Char.Gold = maxCharacterGold - 399
			case "invalid-slot":
				request = criticalPickupRequest(g.ID, model.PlayerCarrySlots)
			case "short":
				request = request[:27]
			}
			before, pending := accountStateSnapshot(p.Account), len(queue.pending)
			w.onGetItem(p.Session, request)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || g.criticalPickup != 0 ||
				w.groundItems[g.ID] != g || fallback.saves != 0 || len(queue.pending) != pending ||
				len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 || p.criticalLifecycle != nil {
				t.Fatal("rejected pickup mutated, reserved, saved inline or confirmed")
			}
		})
	}
}

func TestCriticalGroundPickupUnknownOutcomeRetainsSharedClaim(t *testing.T) {
	for _, scenario := range []string{"error", "panic", "identity", "replaced-object", "item", "position", "deadline", "inventory", "gold", "instance"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, g, fallback, backend, queue, release := criticalPickupFixture(t)
			competitor := criticalPickupCompetitor(t, w, g)
			backend.failure = nil
			if scenario == "error" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			backend.panicAfterWrite = scenario == "panic"
			request := criticalPickupRequest(g.ID, 7)
			if scenario == "gold" {
				configureCriticalGroundCurrency(w, g, 400)
			}
			w.onGetItem(p.Session, request)
			awaitCriticalWrite(t, backend.entered)
			switch scenario {
			case "identity":
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			case "replaced-object":
				replacement := *g
				w.groundItems[g.ID] = &replacement // Bypass the guarded registry deliberately.
			case "item":
				g.Item.Eff[0]++
			case "position":
				g.X++
			case "deadline":
				g.Expire = g.Expire.Add(time.Second)
			case "inventory":
				p.Char.Inv[7] = model.Item{Index: 201}
			case "gold":
				p.Char.Gold++
			case "instance":
				g.InstanceID = "different-runtime"
			}
			before := accountStateSnapshot(p.Account)
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.onGetItem(p.Session, request)
			w.onGetItem(competitor.Session, request)
			if !w.shuttingDown || w.shutdownErr == nil || !p.PersistencePoisoned || !p.Session.IsClosed() ||
				g.criticalPickup == 0 || len(queue.pending) != 1 || !w.criticalPlayerReserved(p) ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
				p.Session.QueuedPacketsForTest() != 0 || competitor.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
				t.Fatal("uncertain pickup applied, published, rolled back, repeated or released ownership")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 {
				t.Fatal("shutdown overwrote an uncertain ground pickup")
			}
		})
	}
}

func TestCriticalGroundPickupDisconnectWaitsForCommittedInventory(t *testing.T) {
	w, p, g, fallback, backend, queue, release := criticalPickupFixture(t)
	competitor := criticalPickupCompetitor(t, w, g)
	w.onGetItem(p.Session, criticalPickupRequest(g.ID, 7))
	awaitCriticalWrite(t, backend.entered)
	w.handle(command{s: p.Session})
	if !p.Session.IsClosed() || w.players[p.Session] != p || len(backend.entered) != 0 ||
		w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) {
		t.Fatal("disconnect overtook pending pickup or allowed stale readmission")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	final := awaitCriticalWrite(t, backend.entered)
	if final.accounts[0].Chars[p.CharSlot].Inv[7] != g.Item || p.Char.Inv[7] != g.Item ||
		w.groundItems[g.ID] != nil || p.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 {
		t.Fatal("final disconnect save missed committed pickup or sent a closed-session reply")
	}
	assertCriticalPickupPacket(t, competitor, wire.RemoveItem(uint32(g.ID)))
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.players[p.Session] != nil || len(queue.pending) != 0 ||
		!w.claimAccountSession(gameNet.NewTestSession(4, 8), p.Account.Name) {
		t.Fatal("final disconnect acknowledgement did not release readmission")
	}
}

func TestCriticalGroundPickupShutdownWaitsForCommittedSnapshot(t *testing.T) {
	for _, currency := range []bool{false, true} {
		t.Run(fmt.Sprintf("currency-%t", currency), func(t *testing.T) {
			w, p, g, fallback, backend, queue, release := criticalPickupFixture(t)
			terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
			w.store = terminal
			if currency {
				configureCriticalGroundCurrency(w, g, 400)
			}
			before := accountStateSnapshot(p.Account)
			w.onGetItem(p.Session, criticalPickupRequest(g.ID, 7))
			proposal := awaitCriticalWrite(t, backend.entered)
			first, second := make(chan error, 1), make(chan error, 1)
			w.runShutdown(first)
			w.runShutdown(second)
			w.handle(command{s: p.Session})
			if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping ||
				w.shutdownFinished || !p.Session.IsClosed() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
				w.groundItems[g.ID] != g || g.criticalPickup == 0 {
				t.Fatal("shutdown saved or removed uncommitted pickup before acknowledgement")
			}
			for _, done := range []chan error{first, second} {
				select {
				case <-done:
					t.Fatal("shutdown finished while pickup was held")
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
					t.Fatal("shutdown did not finish after pickup acknowledgement")
				}
			}
			if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
				fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 ||
				p.Session.QueuedPacketsForTest() != 0 || w.groundItems[g.ID] != nil || g.criticalPickup != 0 {
				t.Fatal("terminal save lost pickup, repeated persistence or confirmed to a closed session")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed pickup persistence")
			}
		})
	}
}

func TestCriticalGroundPickupAllowsMetadataOnlyInstanceRelease(t *testing.T) {
	w, p, g, _, backend, queue, release := criticalPickupFixture(t)
	const runtimeID = "pickup-private-runtime"
	w.itemInstances = map[string]*ItemInstance{runtimeID: {RuntimeID: runtimeID, MemberIDs: []uint16{p.ID}}}
	g.InstanceID = runtimeID
	if !w.groundItemVisibleToPlayer(p, g) || w.gameplaySpaceForPlayer(p) != runtimeID {
		t.Fatal("fixture did not establish private pickup eligibility")
	}
	w.onGetItem(p.Session, criticalPickupRequest(g.ID, 7))
	awaitCriticalWrite(t, backend.entered)
	// Exercise the same metadata-only release performed by instance cleanup.
	// The object and economic contents must remain exactly those admitted.
	g.InstanceID = ""
	delete(w.itemInstances, runtimeID)
	competitor := criticalPickupCompetitor(t, w, g)
	w.onGetItem(competitor.Session, criticalPickupRequest(g.ID, 7))
	if g.criticalPickup == 0 || competitor.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 {
		t.Fatal("public release granted a second claim before acknowledgement")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.groundItems[g.ID] != nil || p.Char.Inv[7] != g.Item || w.shuttingDown {
		t.Fatal("metadata-only public release prevented pickup completion")
	}
	assertCriticalPickupPacket(t, competitor, wire.RemoveItem(uint32(g.ID)))
}

func TestCriticalGroundPickupFallbackMaterializesIdentityOnce(t *testing.T) {
	w, p, g, fallback, backend, queue, release := criticalPickupFixture(t)
	g.Item.UID = ""
	p.Char.Inv[7] = model.Item{Index: 201}
	request := criticalPickupRequest(g.ID, 7)
	w.onGetItem(p.Session, request)
	write := awaitCriticalWrite(t, backend.entered)
	prepared := write.accounts[0].Chars[p.CharSlot].Inv[0]
	if prepared.UID == "" || prepared.Index != g.Item.Index || g.Item.UID != "" || p.Char.Inv[0].Index != 0 {
		t.Fatal("fallback identity was not isolated in the detached proposal")
	}
	w.onGetItem(p.Session, request)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.onGetItem(p.Session, request)
	w.processCriticalCompletions(1)
	if p.Char.Inv[0] != prepared || len(backend.entered) != 0 || len(queue.pending) != 0 || fallback.saves != 0 {
		t.Fatal("completion regenerated item identity or repeated persistence")
	}
	assertCriticalPickupPacket(t, p, wire.CNFGetItem(placeInv, 0))
	assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 0, prepared))
}

func TestCriticalGroundPickupReservedTradeCounterpartRetainsContext(t *testing.T) {
	w, p, g, fallback, backend, queue, release := criticalPickupFixture(t)
	opponent := criticalPickupCompetitor(t, w, g)
	p.Trade, opponent.Trade = &TradeState{OpponentID: opponent.ID}, &TradeState{OpponentID: p.ID}
	trade, peerTrade := p.Trade, opponent.Trade
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{opponent.Account}}, []*Player{opponent},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	before := accountStateSnapshot(p.Account)
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.handle(command{s: p.Session, pkt: criticalPickupRequest(g.ID, 7)})
		w.onGetItem(p.Session, criticalPickupRequest(g.ID, 7))
		if err := w.queueGroundPickup(p, g, 7, 0); !errors.Is(err, errCriticalPersistenceBusy) {
			t.Fatalf("%s accepted a conflicting trade participant: %v", phase, err)
		}
		if p.Trade != trade || opponent.Trade != peerTrade || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
			g.criticalPickup != 0 || len(queue.pending) != 1 || fallback.saves != 0 || len(backend.entered) != 0 ||
			p.Session.QueuedPacketsForTest() != 0 || opponent.Session.QueuedPacketsForTest() != 0 {
			t.Fatalf("%s pickup changed a reserved reciprocal trade", phase)
		}
	}
	w.processCriticalCompletions(1)
}

func TestCriticalGroundPickupActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, kind := range []string{"item", "currency", "unknown-item", "unknown-currency"} {
		t.Run(kind, func(t *testing.T) {
			failed := kind == "unknown-item" || kind == "unknown-currency"
			currency := kind == "currency" || kind == "unknown-currency"
			var failure error
			if failed {
				failure = errors.New("ground pickup acknowledgement lost")
			}
			var g *GroundItem
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure,
				func(w *World, p *Player) {
					g = &GroundItem{ID: 10001, Item: model.Item{Index: 201, UID: "pickup-loop-item"}, X: p.X, Y: p.Y, Expire: time.Unix(100, 0)}
					w.registerGroundItem(g)
					w.registerGroundItem(&GroundItem{ID: 10002, Item: model.Item{Index: 201}, X: p.X, Y: p.Y, Expire: time.Unix(101, 0)})
					if currency {
						configureCriticalGroundCurrency(w, g, 400)
					}
				})
			request := criticalPickupRequest(g.ID, 7)
			deliver := func() {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("actor did not accept pickup intention")
				}
			}
			deliver()
			write := awaitCriticalWrite(t, backend.entered)
			proposed := write.accounts[0].Chars[0]
			if (currency && proposed.Gold != 1400) || (!currency && proposed.Inv[7] != g.Item) {
				t.Fatal("actual dispatcher submitted the wrong pickup proposal")
			}
			for range 3 {
				deliver()
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held pickup blocked unrelated ticks/commands or confirmed before commit")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for {
				overview := readCriticalLoopOverview(t, w)
				if (failed && overview.State == "maintenance") || (!failed && p.Session.QueuedPacketsForTest() >= 1) {
					break
				}
				if time.Now().After(deadline) {
					t.Fatal("actor did not deliver pickup outcome")
				}
			}
			stop() // Join the actor before inspecting mutable gameplay state.
			if w.groundItems[10002] != nil || w.mobTickCounter < 3 || len(backend.entered) != 0 {
				t.Fatal("reserved pickup blocked other expiry/ticks or repeated persistence")
			}
			if failed {
				if w.groundItems[g.ID] != g || g.criticalPickup == 0 || len(queue.pending) != 1 ||
					!p.PersistencePoisoned || !p.Session.IsClosed() || p.Char.Gold != 1000 || p.Char.Inv[7].Index != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
					t.Fatal("unknown actor pickup applied, published or released ownership")
				}
			} else {
				if w.groundItems[g.ID] != nil || g.criticalPickup != 0 || len(queue.pending) != 0 || w.shuttingDown ||
					p.Char.Gold != proposed.Gold || p.Char.Inv[7] != proposed.Inv[7] {
					t.Fatal("actor pickup did not apply the durable proposal exactly once")
				}
				if currency {
					assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
				} else {
					assertCriticalPickupPacket(t, p, wire.CNFGetItem(placeInv, 7))
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 7, g.Item))
				}
			}
		})
	}
}
