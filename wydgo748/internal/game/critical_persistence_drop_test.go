package game

import (
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

func criticalDropRequest(place byte, slot int) []byte {
	packet := inboundPacket(wire.OpDropItem, 32)
	binary.LittleEndian.PutUint32(packet[12:16], uint32(place))
	binary.LittleEndian.PutUint32(packet[16:20], uint32(slot))
	binary.LittleEndian.PutUint32(packet[20:24], 17)
	return packet
}

func criticalDropFixture(t *testing.T, place byte, slot int) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 0, 0)
	item := p.Char.Inv[0]
	p.Char.Inv[0] = model.Item{}
	*playerSlotOf(p, place, byte(slot)) = item
	w.items[item.Index] = model.ItemDef{Index: item.Index}
	w.clock = newFakeClock(time.Unix(100, 0))
	banker := &Mob{ID: 1102, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
		Name: "Warehouse", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 2},
	}}
	w.registerMobSpatial(banker)
	p.show(banker.ID)
	p.CargoNPC = banker.ID
	return w, p, fallback, backend, queue, release
}

func pendingCriticalDrop(t *testing.T, w *World) *GroundItem {
	t.Helper()
	for _, g := range w.groundItems {
		if g.criticalDrop != 0 {
			return g
		}
	}
	t.Fatal("drop did not retain an unpublished allocation")
	return nil
}

func criticalDropObserver(t *testing.T, w *World, p *Player, g *GroundItem) *Player {
	t.Helper()
	observer := criticalPickupCompetitor(t, w, g)
	observer.hide(g.ID)
	p.show(observer.ID)
	observer.show(p.ID)
	observer.show(1102)
	return observer
}

func TestCriticalGroundDropDetachedCommitAndHiddenReservation(t *testing.T) {
	for _, source := range []struct {
		place byte
		slot  int
	}{{placeInv, 0}, {placeInv, model.PlayerCarrySlots - 1}, {placeStorage, 0}, {placeStorage, model.PlayerCargoSlots - 1}} {
		for charSlot := range 4 {
			t.Run(fmt.Sprintf("place-%d/slot-%d/character-%d", source.place, source.slot, charSlot), func(t *testing.T) {
				w, p, fallback, backend, queue, release := criticalDropFixture(t, source.place, source.slot)
				if charSlot != 0 {
					p.Account.Chars[0], p.Account.Chars[charSlot] = p.Account.Chars[charSlot], p.Account.Chars[0]
					p.CharSlot, p.Char = charSlot, &p.Account.Chars[charSlot]
				}
				account, character := p.Account, p.Char
				before := accountStateSnapshot(account)
				item := *playerSlotOf(p, source.place, byte(source.slot))
				expected := accountPersistenceSnapshot(account)
				if source.place == placeInv {
					expected.Chars[charSlot].Inv[source.slot] = model.Item{}
				} else {
					expected.Cargo[source.slot] = model.Item{}
				}
				request := criticalDropRequest(source.place, source.slot)
				w.handle(command{s: p.Session, pkt: request})
				write := awaitCriticalWrite(t, backend.entered)
				g := pendingCriticalDrop(t, w)
				observer := criticalDropObserver(t, w, p, g)
				if len(write.accounts) != 1 || !reflect.DeepEqual(write.accounts[0], expected) || write.accounts[0] == account || g.Item != item {
					t.Fatal("drop proposal lost full account contents, identity or item effects")
				}
				other := &GroundItem{ID: 10002, Item: item, X: p.X, Y: p.Y, Expire: time.Unix(221, 0)}
				w.registerGroundItem(other)
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.handle(command{s: p.Session, pkt: request})
					w.onDropItem(p.Session, request)
					w.onGetItem(observer.Session, criticalPickupRequest(g.ID, 7))
					w.publishItemSpawn(g)
					w.publishItemRemove(g)
					w.refreshPlayerVisibility(observer)
					w.unregisterGroundItem(g)
					w.registerGroundItem(&GroundItem{ID: g.ID, Item: model.Item{Index: 201}, X: p.X, Y: p.Y})
					w.expireGroundItems(time.Unix(300, 0))
					if w.groundItems[g.ID] != g || g.criticalDrop == 0 || w.groundItems[other.ID] != nil ||
						w.groundItemVisibleToPlayer(observer, g) || observer.hasVisible(g.ID) || p.hasVisible(g.ID) ||
						!reflect.DeepEqual(accountStateSnapshot(account), before) || fallback.saves != 0 ||
						len(queue.pending) != 1 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatalf("%s drop leaked, expired, replaced, saved inline or changed its source", phase)
					}
					// View refresh may introduce the independent due item, but never
					// the pending drop. Its visibility is checked above and after ack.
					for observer.Session.QueuedPacketsForTest() != 0 {
						packet, _ := observer.Session.DequeuePacketForTest()
						if !wire.Decrypt(packet) {
							t.Fatal("view refresh returned an invalid frame")
						}
						header := wire.ParseHeader(packet)
						if (header.Type == wire.OpCreateItem && binary.LittleEndian.Uint16(packet[16:18]) == g.ID) ||
							(header.Type == wire.OpRemoveItem && binary.LittleEndian.Uint32(packet[12:16]) == uint32(g.ID)) {
							t.Fatal("view refresh exposed pending ground identity")
						}
					}
				}
				discardCriticalDeferredDuplicates(w)

				w.processCriticalCompletions(1)
				if p.Account != account || p.Char != character || *playerSlotOf(p, source.place, byte(source.slot)) != (model.Item{}) ||
					g.criticalDrop != 0 || w.groundItems[g.ID] != g || len(queue.pending) != 0 || fallback.saves != 0 {
					t.Fatal("drop acknowledgement did not apply only the durable source removal")
				}
				assertCriticalPickupPacket(t, observer, w.groundItemCreatePacket(g))
				assertCriticalPickupPacket(t, p, w.groundItemCreatePacket(g))
				assertCriticalPickupPacket(t, p, wire.CNFDropItem(uint32(source.place), uint32(source.slot), 17, g.X, g.Y))
				assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, source.place, byte(source.slot), model.Item{}))
				if p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("drop confirmation was duplicated")
				}
				// Another player is refused while the dropper's protection lasts.
				w.onGetItem(observer.Session, criticalPickupRequest(g.ID, 7))
				if len(backend.entered) != 0 || g.criticalPickup != 0 {
					t.Fatal("a protected drop was admitted for another player")
				}
				assertCriticalPickupPacket(t, observer, wire.MessagePanel(groundProtectedMessage))
				w.clock.(*fakeClock).Advance(groundItemProtection)
				w.onGetItem(observer.Session, criticalPickupRequest(g.ID, 7))
				pickup := awaitCriticalWrite(t, backend.entered)
				if pickup.accounts[0].Chars[0].Inv[7] != item || observer.Char.Inv[7].Index != 0 {
					t.Fatal("acknowledged drop did not admit a fresh durable pickup")
				}
				awaitReadyCriticalResults(t, queue, 1)
				discardCriticalDeferredDuplicates(w)

				w.processCriticalCompletions(1)
				if observer.Char.Inv[7] != item || w.groundItems[g.ID] != nil {
					t.Fatal("fresh pickup failed to transfer the committed drop")
				}
			})
		}
	}
}

func TestCriticalGroundDropUnknownRetainsHiddenAllocation(t *testing.T) {
	for _, kind := range []string{"error", "panic", "identity", "source", "object", "position", "deadline", "instance"} {
		for _, place := range []byte{placeInv, placeStorage} {
			t.Run(fmt.Sprintf("%s/place-%d", kind, place), func(t *testing.T) {
				w, p, fallback, backend, queue, release := criticalDropFixture(t, place, 5)
				if kind == "error" {
					backend.failure = errors.New("drop acknowledgement lost")
				} else if kind == "panic" {
					backend.panicAfterWrite = true
				}
				w.onDropItem(p.Session, criticalDropRequest(place, 5))
				awaitCriticalWrite(t, backend.entered)
				g := pendingCriticalDrop(t, w)
				observer := criticalDropObserver(t, w, p, g)
				switch kind {
				case "identity":
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				case "source":
					playerSlotOf(p, place, 5).Index++
				case "object":
					g.Item.Eff[0]++
				case "position":
					g.X++
				case "deadline":
					g.Expire = g.Expire.Add(time.Second)
				case "instance":
					g.InstanceID = "foreign-runtime"
				}
				before := accountStateSnapshot(p.Account)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || w.groundItems[g.ID] != g || g.criticalDrop == 0 ||
					len(queue.pending) != 1 || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 ||
					observer.Session.QueuedPacketsForTest() != 0 || w.groundItemVisibleToPlayer(observer, g) {
					t.Fatal("unknown drop applied, published, rolled back or released ownership")
				}
				w.processCriticalCompletions(8)
				w.onGetItem(observer.Session, criticalPickupRequest(g.ID, 7))
				done := make(chan error, 1)
				w.runShutdown(done)
				if err := <-done; err == nil || fallback.saves != 0 || len(backend.entered) != 0 {
					t.Fatal("unknown drop replayed or overwrote its possibly committed account")
				}
			})
		}
	}
}

func TestCriticalGroundDropDisconnectWaitsForSourceRemoval(t *testing.T) {
	for _, place := range []byte{placeInv, placeStorage} {
		t.Run(fmt.Sprint(place), func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalDropFixture(t, place, 5)
			w.onDropItem(p.Session, criticalDropRequest(place, 5))
			proposal := awaitCriticalWrite(t, backend.entered)
			g := pendingCriticalDrop(t, w)
			observer := criticalDropObserver(t, w, p, g)
			w.handle(command{s: p.Session})
			if !p.Session.IsClosed() || w.players[p.Session] != p ||
				w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) {
				t.Fatal("disconnect released pending drop ownership")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(final.accounts[0], proposal.accounts[0]) || fallback.saves != 0 ||
				g.criticalDrop != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("final disconnect save lost committed drop or replied to closed transport")
			}
			assertCriticalPickupPacket(t, observer, w.groundItemCreatePacket(g))
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || len(queue.pending) != 0 ||
				!w.claimAccountSession(gameNet.NewTestSession(4, 8), p.Account.Name) {
				t.Fatal("final drop acknowledgement did not release readmission")
			}
		})
	}
}

func TestCriticalGroundDropAdmissionRejectionPreservesSource(t *testing.T) {
	for _, scenario := range []string{"full", "busy", "stopping", "closed", "identity", "dead", "outside-world", "invalid-Carry", "invalid-Cargo", "Equip", "short", "unknown-item", "warehouse", "listed", "filled-seal", "ID-exhaustion"} {
		t.Run(scenario, func(t *testing.T) {
			place := byte(placeInv)
			if scenario == "warehouse" || scenario == "listed" || scenario == "invalid-Cargo" {
				place = placeStorage
			}
			w, p, fallback, backend, queue, _ := criticalDropFixture(t, place, 5)
			request := criticalDropRequest(place, 5)
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
			case "dead":
				setPlayerCurHP(p.Char, 0)
			case "outside-world":
				p.InWorld = false
			case "invalid-Carry":
				request = criticalDropRequest(placeInv, model.PlayerCarrySlots)
			case "invalid-Cargo":
				request = criticalDropRequest(placeStorage, model.PlayerCargoSlots)
			case "Equip":
				request = criticalDropRequest(placeEquip, 0)
			case "short":
				request = request[:31]
			case "unknown-item":
				delete(w.items, p.Char.Inv[5].Index)
			case "warehouse":
				w.mobsByID[1102].X += npcInteractionRange + 1
			case "listed":
				p.GhostShop = &GhostShop{}
				p.GhostShop.CarryPos[0], p.GhostShop.Items[0] = 5, p.Account.Cargo[5]
			case "filled-seal":
				p.Char.Inv[5] = model.NewCelestialSeal(p.Char.Inv[5].UID, 1)
			case "ID-exhaustion":
				p.Char.Inv[5].Index = 746
				w.items[746] = model.ItemDef{Index: 746}
				for id := uint16(15001); id <= 15100; id++ {
					w.registerGroundItem(&GroundItem{ID: id, Item: model.Item{Index: 746}, X: p.X, Y: p.Y, Permanent: true})
				}
			}
			before, pending, objects := accountStateSnapshot(p.Account), len(queue.pending), len(w.groundItems)
			w.onDropItem(p.Session, request)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != pending ||
				len(w.groundItems) != objects || fallback.saves != 0 || len(backend.entered) != 0 || p.criticalLifecycle != nil {
				t.Fatal("rejected drop changed its source, leaked an allocation or saved inline")
			}
			for p.Session.QueuedPacketsForTest() != 0 {
				packet, _ := p.Session.DequeuePacketForTest()
				if !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel {
					t.Fatal("rejected drop published economic success")
				}
			}
		})
	}
}

func TestCriticalGroundDropShutdownWaitsForCommittedRemoval(t *testing.T) {
	for _, place := range []byte{placeInv, placeStorage} {
		t.Run(fmt.Sprint(place), func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalDropFixture(t, place, 5)
			terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
			w.store = terminal
			before := accountStateSnapshot(p.Account)
			w.onDropItem(p.Session, criticalDropRequest(place, 5))
			proposal := awaitCriticalWrite(t, backend.entered)
			g := pendingCriticalDrop(t, w)
			first, second := make(chan error, 1), make(chan error, 1)
			w.runShutdown(first)
			w.runShutdown(second)
			w.handle(command{s: p.Session})
			if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping ||
				w.shutdownFinished || !p.Session.IsClosed() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || g.criticalDrop == 0 {
				t.Fatal("shutdown persisted or published an uncommitted drop")
			}
			for _, done := range []chan error{first, second} {
				select {
				case <-done:
					t.Fatal("shutdown returned before drop acknowledgement")
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
					t.Fatal("shutdown did not finish after committed removal")
				}
			}
			if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
				fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 ||
				p.Session.QueuedPacketsForTest() != 0 || w.groundItems[g.ID] != g || g.criticalDrop != 0 {
				t.Fatal("terminal save lost the removal, repeated persistence or replied to a closed session")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed drop persistence")
			}
		})
	}
}

func TestCriticalGroundDropAllowsMetadataOnlyInstanceRelease(t *testing.T) {
	w, p, _, backend, queue, release := criticalDropFixture(t, placeInv, 5)
	const runtimeID = "drop-private-runtime"
	w.itemInstances = map[string]*ItemInstance{runtimeID: {RuntimeID: runtimeID, MemberIDs: []uint16{p.ID}}}
	w.onDropItem(p.Session, criticalDropRequest(placeInv, 5))
	awaitCriticalWrite(t, backend.entered)
	g := pendingCriticalDrop(t, w)
	if g.InstanceID != runtimeID {
		t.Fatal("drop allocation lost its private gameplay space")
	}
	// Instance cleanup releases metadata and calls this publication boundary.
	g.InstanceID = ""
	delete(w.itemInstances, runtimeID)
	observer := criticalDropObserver(t, w, p, g)
	w.publishItemSpawn(g)
	w.refreshPlayerVisibility(observer)
	w.onGetItem(observer.Session, criticalPickupRequest(g.ID, 7))
	if observer.hasVisible(g.ID) || observer.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 {
		t.Fatal("private-to-public cleanup exposed an uncommitted drop")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if g.criticalDrop != 0 || g.InstanceID != "" || p.Char.Inv[5].Index != 0 || w.shuttingDown {
		t.Fatal("metadata-only release prevented committed publication")
	}
	assertCriticalPickupPacket(t, observer, w.groundItemCreatePacket(g))
}

func TestCriticalGroundDropMaterializesUIDOnce(t *testing.T) {
	w, p, _, backend, queue, release := criticalDropFixture(t, placeInv, 5)
	p.Char.Inv[5].UID = ""
	request := criticalDropRequest(placeInv, 5)
	w.onDropItem(p.Session, request)
	awaitCriticalWrite(t, backend.entered)
	g := pendingCriticalDrop(t, w)
	prepared := g.Item
	if prepared.UID == "" || p.Char.Inv[5].UID != "" {
		t.Fatal("drop materialization changed the live source or omitted identity")
	}
	w.onDropItem(p.Session, request)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.onDropItem(p.Session, request)
	w.processCriticalCompletions(1)
	if g.Item != prepared || p.Char.Inv[5].Index != 0 || len(backend.entered) != 0 || len(w.groundItems) != 1 || len(queue.pending) != 0 {
		t.Fatal("drop completion regenerated UID or repeated source removal")
	}
}

func TestCriticalGroundCannonCannotUseReservedLoot(t *testing.T) {
	w, p, _, backend, queue, release := criticalDropFixture(t, placeInv, 5)
	p.Char.Inv[5].Index = 746
	w.items[746] = model.ItemDef{Index: 746}
	w.onDropItem(p.Session, criticalDropRequest(placeInv, 5))
	awaitCriticalWrite(t, backend.entered)
	g := pendingCriticalDrop(t, w)
	if g.ID != 15001 || w.groundCannonAt(p.X, p.Y) != nil {
		t.Fatal("pending cannon was usable before durable inventory removal")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.groundCannonAt(p.X, p.Y) != g {
		t.Fatal("committed cannon did not become usable")
	}
	w.onGetItem(p.Session, criticalPickupRequest(g.ID, 7))
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	if w.groundCannonAt(p.X, p.Y) != nil {
		t.Fatal("completed but unacknowledged pickup still exposed a usable cannon")
	}
	w.processCriticalCompletions(1)
	if w.groundCannonAt(p.X, p.Y) != nil || w.groundItems[g.ID] != nil {
		t.Fatal("picked-up cannon remained usable")
	}
}

func TestCriticalGroundDropActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, place := range []byte{placeInv, placeStorage} {
		for _, failed := range []bool{false, true} {
			t.Run(fmt.Sprintf("place-%d/unknown-%t", place, failed), func(t *testing.T) {
				var failure error
				if failed {
					failure = errors.New("drop acknowledgement lost")
				}
				item := model.Item{Index: 201, UID: "11111111111141118111111111111111"}
				w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure,
					func(w *World, p *Player) {
						w.items = map[uint16]model.ItemDef{201: {Index: 201}}
						*playerSlotOf(p, place, 5) = item
						w.registerGroundItem(&GroundItem{ID: 10002, Item: item, X: p.X, Y: p.Y, Expire: time.Unix(101, 0)})
					})
				request := criticalDropRequest(place, 5)
				deliver := func() {
					select {
					case w.commands <- command{s: p.Session, pkt: request}:
					case <-time.After(time.Second):
						t.Fatal("actor did not accept drop intention")
					}
				}
				deliver()
				write := awaitCriticalWrite(t, backend.entered)
				if (place == placeInv && write.accounts[0].Chars[0].Inv[5].Index != 0) ||
					(place == placeStorage && write.accounts[0].Cargo[5].Index != 0) {
					t.Fatal("actual dispatcher submitted the wrong source removal")
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
						t.Fatal("held drop blocked unrelated ticks or confirmed before commit")
					}
				}
				release()
				deadline := time.Now().Add(2 * time.Second)
				for {
					overview := readCriticalLoopOverview(t, w)
					if (failed && overview.State == "maintenance") || (!failed && p.Session.QueuedPacketsForTest() >= 3) {
						break
					}
					if time.Now().After(deadline) {
						t.Fatal("actor did not deliver drop outcome")
					}
				}
				stop() // Join the actor before inspecting mutable fields.
				if w.groundItems[10002] != nil || w.mobTickCounter < 3 || len(backend.entered) != 0 {
					t.Fatal("drop blocked unrelated expiry/ticks or repeated persistence")
				}
				var g *GroundItem
				for _, object := range w.groundItems {
					g = object
				}
				if g == nil || g.Item != item {
					t.Fatal("drop lost its exact allocated object")
				}
				if failed {
					if g.criticalDrop == 0 || len(queue.pending) != 1 || !p.PersistencePoisoned || !p.Session.IsClosed() ||
						*playerSlotOf(p, place, 5) != item || p.Session.QueuedPacketsForTest() != 0 ||
						!errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
						t.Fatal("unknown actor drop applied, published or released ownership")
					}
				} else {
					if g.criticalDrop != 0 || len(queue.pending) != 0 || w.shuttingDown || playerSlotOf(p, place, 5).Index != 0 {
						t.Fatal("actor drop did not apply the durable removal exactly once")
					}
					assertCriticalPickupPacket(t, p, w.groundItemCreatePacket(g))
					assertCriticalPickupPacket(t, p, wire.CNFDropItem(uint32(place), 5, 17, g.X, g.Y))
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, place, 5, model.Item{}))
				}
			})
		}
	}
}

func TestCriticalGroundDropReservedTradeCounterpartRetainsContext(t *testing.T) {
	for _, place := range []byte{placeInv, placeStorage} {
		t.Run(fmt.Sprintf("place-%d", place), func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalDropFixture(t, place, 5)
			opponent := criticalPickupCompetitor(t, w, &GroundItem{ID: 10001, X: p.X, Y: p.Y})
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
				request := criticalDropRequest(place, 5)
				w.handle(command{s: p.Session, pkt: request})
				w.onDropItem(p.Session, request)
				if p.Trade != trade || opponent.Trade != peerTrade || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					len(w.groundItems) != 0 || len(queue.pending) != 1 || fallback.saves != 0 || len(backend.entered) != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || opponent.Session.QueuedPacketsForTest() != 0 {
					t.Fatalf("%s drop changed a reserved reciprocal trade or allocated loot", phase)
				}
			}
			w.processCriticalCompletions(1)
		})
	}
}

func TestCriticalGroundDropPreservesOriginalExpiryAfterCommit(t *testing.T) {
	w, p, _, backend, queue, release := criticalDropFixture(t, placeInv, 5)
	w.onDropItem(p.Session, criticalDropRequest(placeInv, 5))
	awaitCriticalWrite(t, backend.entered)
	g := pendingCriticalDrop(t, w)
	deadline := g.Expire
	w.expireGroundItems(deadline.Add(time.Second))
	if w.groundItems[g.ID] != g || g.Expire != deadline || p.Char.Inv[5].Index == 0 {
		t.Fatal("pending drop expired or extended its original lifetime")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if g.Expire != deadline || w.groundItems[g.ID] != g || p.Char.Inv[5].Index != 0 {
		t.Fatal("committed drop lost its original expiry or source removal")
	}
	w.expireGroundItems(deadline.Add(time.Second))
	if w.groundItems[g.ID] != nil {
		t.Fatal("commit left an expired drop permanently reserved")
	}
}

// Native FUN_0046247E sends the character's own cell at GridX@24/GridY@26; the
// item lands exactly there, never on a random neighbor.
func TestPlayerDropLandsOnTheReportedCharacterCell(t *testing.T) {
	for _, tc := range []struct {
		name   string
		dx, dy int
		wantDX int
		wantDY int
	}{
		{"own cell", 0, 0, 0, 0},
		{"server position trails the client", 1, -1, 1, -1},
		{"too far falls back to the server position", dropCellTolerance + 1, 0, 0, 0},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _, _, _, _ := criticalDropFixture(t, placeInv, 0)
			packet := criticalDropRequest(placeInv, 0)
			binary.LittleEndian.PutUint16(packet[24:26], uint16(int(p.X)+tc.dx))
			binary.LittleEndian.PutUint16(packet[26:28], uint16(int(p.Y)+tc.dy))
			for range 8 {
				g := w.createGroundDropCell(p.X, p.Y, model.Item{Index: p.Char.Inv[0].Index}, false, "", groundOwner{})
				if g == nil || g.X != p.X || g.Y != p.Y {
					t.Fatal("an unscattered drop must keep its exact cell")
				}
				w.unregisterGroundItem(g)
			}
			req, ok := parseDropItemRequest(packet)
			if !ok {
				t.Fatal("drop request did not parse")
			}
			x, y := w.playerDropCell(p, req)
			if int(x) != int(p.X)+tc.wantDX || int(y) != int(p.Y)+tc.wantDY {
				t.Fatalf("drop cell (%d,%d), want (%d,%d)", x, y, int(p.X)+tc.wantDX, int(p.Y)+tc.wantDY)
			}
		})
	}
}
