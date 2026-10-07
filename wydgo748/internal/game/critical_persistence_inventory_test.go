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

func configureCriticalInventory(p *Player, sourceSlot, effectSlot int) {
	for len(p.Account.Chars) < 4 {
		p.Account.Chars = append(p.Account.Chars, model.Char{})
	}
	p.Char = &p.Account.Chars[p.CharSlot]
	p.Char.Inv = [model.MaxCarry]model.Item{}
	item := model.Item{Index: 412, UID: "11111111111141118111111111111111",
		Eff: [6]byte{7, 3, 8, 4, 9, 5}, ActivatedUnix: 123, ExpiresUnix: 4102444800}
	item.Eff[effectSlot*2], item.Eff[effectSlot*2+1] = effectAmount, 10
	p.Char.Inv[sourceSlot] = item
	p.Char.Inv[model.PlayerCarrySlots] = model.Item{Index: 200, UID: "hidden-inventory-item"}
	p.Account.Cargo[0] = model.Item{Index: 201, UID: "unrelated-cargo-item"}
	p.Account.Chars[3].Gold = 777
}

func criticalInventoryFixture(t *testing.T, sourceSlot, effectSlot int) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, fallback := handlerTestWorld(t)
	configureCriticalInventory(p, sourceSlot, effectSlot)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("inventory fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, backend, queue, release
}

func criticalInventoryRequest(kind string, id uint16, sourceSlot int) []byte {
	opcode, size := uint16(wire.OpDeleteItem), deleteItemPacketSize
	if kind == "split" {
		opcode, size = wire.OpSplitItem, splitItemPacketSize
	}
	packet := inboundPacket(opcode, size)
	binary.LittleEndian.PutUint16(packet[6:8], id)
	binary.LittleEndian.PutUint32(packet[12:16], uint32(sourceSlot))
	binary.LittleEndian.PutUint32(packet[16:20], 412)
	if kind == "split" {
		binary.LittleEndian.PutUint32(packet[20:24], 6)
	}
	return packet
}

func callCriticalInventory(w *World, p *Player, kind string, packet []byte) {
	if kind == "split" {
		w.onSplitItem(p.Session, packet)
	} else {
		w.onDeleteItem(p.Session, packet)
	}
}

func criticalInventoryExpected(t *testing.T, kind string, sourceSlot int, before *model.Account, charSlot int, proposal criticalWrite) (*model.Account, []int) {
	t.Helper()
	want := accountStateSnapshot(before)
	slots := []int{sourceSlot}
	if kind == "delete" {
		want.Chars[charSlot].Inv[sourceSlot] = model.Item{}
		return want, slots
	}
	destSlot := 0
	if sourceSlot == 0 {
		destSlot = 1
	}
	source := before.Chars[charSlot].Inv[sourceSlot]
	want.Chars[charSlot].Inv[sourceSlot] = source
	setExactItemAmount(&want.Chars[charSlot].Inv[sourceSlot], 4)
	split := source
	setExactItemAmount(&split, 6)
	split.UID = proposal.accounts[0].Chars[charSlot].Inv[destSlot].UID
	if split.UID == "" || split.UID == source.UID {
		t.Fatal("split proposal does not have an independent item identity")
	}
	want.Chars[charSlot].Inv[destSlot] = split
	return want, append(slots, destSlot)
}

func assertCriticalInventoryPublication(t *testing.T, p *Player, slots []int) {
	t.Helper()
	for _, slot := range slots {
		want := wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot])
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || len(packet) != len(want) {
			t.Fatal("missing or incorrectly sized inventory confirmation")
		}
		header := wire.ParseHeader(packet)
		if header.Type != wire.OpSendItem || header.ID != p.ID || int(header.Size) != len(want) || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatal("inventory confirmation recipient, destination, order or body differs from committed state")
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("inventory operation published extra responses")
	}
}

func TestCriticalInventoryDetachedCommitAndRepeatedRequests(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		for _, sourceSlot := range []int{0, 5, model.PlayerCarrySlots - 1} {
			for effectSlot := 0; effectSlot < 3; effectSlot++ {
				for _, charSlot := range []int{0, 1} {
					t.Run(fmt.Sprintf("%s/source-%d/effect-%d/character-%d", kind, sourceSlot, effectSlot, charSlot), func(t *testing.T) {
						w, p, fallback, backend, queue, release := criticalInventoryFixture(t, sourceSlot, effectSlot)
						if charSlot != 0 {
							p.Account.Chars[charSlot], p.Account.Chars[0] = p.Account.Chars[0], model.Char{}
							p.CharSlot, p.Char = charSlot, &p.Account.Chars[charSlot]
						}
						account, character := p.Account, p.Char
						before := accountStateSnapshot(account)
						request := criticalInventoryRequest(kind, p.ID, sourceSlot)
						w.handle(command{s: p.Session, pkt: request})
						proposal := awaitCriticalWrite(t, backend.entered)
						want, slots := criticalInventoryExpected(t, kind, sourceSlot, before, charSlot, proposal)
						if proposal.accounts[0] == account || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
							t.Fatal("inventory proposal is incomplete, incorrect or aliases live state")
						}
						checkHeld := func() {
							w.handle(command{s: p.Session, pkt: request})
							callCriticalInventory(w, p, kind, request)
							if !reflect.DeepEqual(accountStateSnapshot(account), before) || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 || len(queue.pending) != 1 || !w.criticalPlayerReserved(p) {
								t.Fatal("held inventory write mutated, saved inline, confirmed, repeated or released ownership")
							}
						}
						checkHeld()
						release()
						awaitReadyCriticalResults(t, queue, 1)
						checkHeld()
						discardCriticalDeferredDuplicates(w)

						w.processCriticalCompletions(1)
						if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) || fallback.saves != 0 || len(queue.pending) != 0 || w.criticalPlayerReserved(p) {
							t.Fatal("inventory commit replaced identity, lost prepared fields or wrote inline")
						}
						assertCriticalInventoryPublication(t, p, slots)
						// The remaining stack is smaller than the requested split. This
						// is stale-state rejection, not global packet-ID deduplication.
						w.handle(command{s: p.Session, pkt: request})
						if len(queue.pending) != 0 || len(backend.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(account), want) {
							t.Fatal("stale request deleted or split the committed inventory twice")
						}
					})
				}
			}
		}
	}
}

func TestCriticalInventoryRejectionsDoNotPersistOrMutate(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		for _, reason := range []string{"short", "slot", "wide-index", "wrong-index", "empty", "dead", "outside", "closed", "poisoned", "shutdown", "wrong-character", "unclaimed", "stopping", "full", "zero-amount", "whole-stack", "wide-amount", "nonstack", "inventory-full"} {
			if kind == "delete" && (reason == "zero-amount" || reason == "whole-stack" || reason == "wide-amount" || reason == "nonstack" || reason == "inventory-full") {
				continue
			}
			t.Run(kind+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, _ := criticalInventoryFixture(t, 5, 0)
				request := criticalInventoryRequest(kind, p.ID, 5)
				switch reason {
				case "short":
					request = request[:len(request)-1]
				case "slot":
					binary.LittleEndian.PutUint32(request[12:16], model.PlayerCarrySlots)
				case "wide-index":
					binary.LittleEndian.PutUint32(request[16:20], 65536+412)
				case "wrong-index":
					binary.LittleEndian.PutUint32(request[16:20], 413)
				case "empty":
					p.Char.Inv[5] = model.Item{}
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "outside":
					p.InWorld = false
				case "closed":
					p.Session.Close()
				case "poisoned":
					p.PersistencePoisoned = true
				case "shutdown":
					w.shuttingDown = true
				case "wrong-character":
					ch := cloneCharacterState(p.Char)
					p.Char = &ch
				case "unclaimed":
					delete(w.accountSessions, accountSessionKey(p.Account.Name))
				case "stopping":
					queue.closeAdmission()
				case "full":
					for _, name := range []string{"foreign-one", "foreign-two"} {
						if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
							t.Fatal(err)
						}
					}
					awaitCriticalWrite(t, backend.entered)
				case "zero-amount":
					binary.LittleEndian.PutUint32(request[20:24], 0)
				case "whole-stack":
					binary.LittleEndian.PutUint32(request[20:24], 10)
				case "wide-amount":
					binary.LittleEndian.PutUint32(request[20:24], 256)
				case "nonstack":
					p.Char.Inv[5].Eff[0] = 7
				case "inventory-full":
					for slot := 0; slot < model.PlayerCarrySlots; slot++ {
						if slot != 5 {
							p.Char.Inv[slot] = model.Item{Index: 200}
						}
					}
				}
				before, charBefore := accountStateSnapshot(p.Account), cloneCharacterState(p.Char)
				callCriticalInventory(w, p, kind, request)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(*p.Char, charBefore) || fallback.saves != 0 || len(backend.entered) != 0 || p.criticalLifecycle != nil || (reason != "full" && len(queue.pending) != 0) {
					t.Fatal("rejected inventory operation changed live data, wrote inline or retained a proposal")
				}
				// Corrective snapshots are permitted, but may only describe the old
				// authoritative inventory; no deletion/split success may escape.
				for p.Session.QueuedPacketsForTest() > 0 {
					packet, _ := p.Session.DequeuePacketForTest()
					if !wire.Decrypt(packet) {
						t.Fatal("invalid rejection response")
					}
					switch wire.ParseHeader(packet).Type {
					case wire.OpMessagePanel:
					case wire.OpSendItem:
						slot := int(binary.LittleEndian.Uint16(packet[14:16]))
						if slot >= model.PlayerCarrySlots {
							t.Fatal("corrective response addressed an invisible slot")
						}
						want := wire.SendItem(p.ID, placeInv, byte(slot), charBefore.Inv[slot])
						if !bytes.Equal(packet[12:], want[12:]) {
							t.Fatal("rejection confirmed an uncommitted inventory change")
						}
					default:
						t.Fatal("rejection published an unexpected opcode")
					}
				}
			})
		}
	}
}

func TestCriticalInventoryUnknownOutcomeNeverPublishesOrOverwrites(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		for _, reason := range []string{"error", "panic", "identity"} {
			t.Run(kind+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 5, 0)
				if reason == "error" {
					backend.failure = errors.New("commit acknowledgement lost")
				}
				backend.panicAfterWrite = reason == "panic"
				before := accountStateSnapshot(p.Account)
				request := criticalInventoryRequest(kind, p.ID, 5)
				w.handle(command{s: p.Session, pkt: request})
				awaitCriticalWrite(t, backend.entered)
				w.handle(command{s: p.Session})
				if reason == "identity" {
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				callCriticalInventory(w, p, kind, request)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || !w.shuttingDown || !p.PersistencePoisoned || !w.criticalPlayerReserved(p) || !p.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 || w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
					t.Fatal("uncertain inventory write applied, confirmed, replayed or released ownership")
				}
				done := make(chan error, 1)
				w.runShutdown(done)
				if err := <-done; err == nil || fallback.saves != 0 {
					t.Fatal("shutdown overwrote an uncertain inventory transaction")
				}
			})
		}
	}
}

func TestCriticalInventoryReservedTradeCounterpartRetainsContext(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 5, 0)
			opponent, _ := networkedTestPlayer(2, "InventoryTradePeer", 2101, 2100)
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
			request := criticalInventoryRequest(kind, p.ID, 5)
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				callCriticalInventory(w, p, kind, request)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || p.Trade != trade || opponent.Trade != peerTrade || p.Session.QueuedPacketsForTest() != 0 || opponent.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 {
					t.Fatalf("%s reserved counterpart lost trade context or admitted inventory changes", phase)
				}
			}
			w.processCriticalCompletions(1)
			callCriticalInventory(w, p, kind, request)
			proposal := awaitCriticalWrite(t, backend.entered)
			if p.Trade != nil || opponent.Trade != nil {
				t.Fatal("eligible inventory operation did not cancel the reciprocal trade")
			}
			for _, player := range []*Player{p, opponent} {
				packet, ok := player.Session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpCloseTrade || player.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("trade cancellation must precede inventory confirmation")
				}
			}
			want, slots := criticalInventoryExpected(t, kind, 5, before, p.CharSlot, proposal)
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || fallback.saves != 0 {
				t.Fatal("eligible operation lost its proposal or wrote inline")
			}
			assertCriticalInventoryPublication(t, p, slots)
		})
	}
}

func TestCriticalInventorySplitAllowsNewIdenticalRequestAfterAcknowledgement(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 5, 2)
	request := criticalInventoryRequest("split", p.ID, 5)
	binary.LittleEndian.PutUint32(request[20:24], 3)
	w.handle(command{s: p.Session, pkt: request})
	first := awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCriticalInventoryPublication(t, p, []int{5, 0})
	w.handle(command{s: p.Session, pkt: request})
	second := awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCriticalInventoryPublication(t, p, []int{5, 1})
	if itemStackAmount(p.Char.Inv[5]) != 4 || itemStackAmount(p.Char.Inv[0]) != 3 || itemStackAmount(p.Char.Inv[1]) != 3 || p.Char.Inv[0].UID != first.accounts[0].Chars[0].Inv[0].UID || p.Char.Inv[1].UID != second.accounts[0].Chars[0].Inv[1].UID || p.Char.Inv[0].UID == p.Char.Inv[1].UID || fallback.saves != 0 || len(queue.pending) != 0 {
		t.Fatal("post-acknowledgement split was deduplicated, lost quantity or reused an item identity")
	}
}

func TestCriticalInventoryDisconnectFinalSnapshotContainsCommit(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 5, 0)
			queue.store = &criticalStateTestStore{&criticalBatchTestStore{backend}}
			w.handle(command{s: p.Session, pkt: criticalInventoryRequest(kind, p.ID, 5)})
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if fallback.saves != 0 || len(backend.entered) != 0 || p.Char.Inv[5].Index != 412 || itemStackAmount(p.Char.Inv[5]) != 10 {
				t.Fatal("disconnect saved or applied uncommitted inventory state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(final.accounts, proposal.accounts) || final.character == nil || final.character.uid != p.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) || p.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || w.claimAccountSession(gameNet.NewTestSession(98, 8), p.Account.Name) {
				t.Fatal("final disconnect snapshot lost inventory changes, sidecar or account ownership")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.handle(command{s: p.Session})
			if len(backend.entered) != 0 || !w.criticalPlayerReserved(p) {
				t.Fatal("final unacknowledged save repeated or released ownership")
			}
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || len(queue.pending) != 0 || fallback.saves != 0 || !w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
				t.Fatal("disconnect acknowledgement did not release account ownership")
			}
		})
	}
}

func TestCriticalInventoryShutdownWaitsForCommittedSnapshot(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 5, 0)
			terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
			w.store = terminal
			before := accountStateSnapshot(p.Account)
			w.handle(command{s: p.Session, pkt: criticalInventoryRequest(kind, p.ID, 5)})
			proposal := awaitCriticalWrite(t, backend.entered)
			first, second := make(chan error, 1), make(chan error, 1)
			w.runShutdown(first)
			w.runShutdown(second)
			w.handle(command{s: p.Session})
			if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished || !p.Session.IsClosed() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("shutdown saved uncommitted inventory state before acknowledgement")
			}
			for _, done := range []chan error{first, second} {
				select {
				case <-done:
					t.Fatal("shutdown finished while inventory write was held")
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
					t.Fatal("shutdown did not complete after inventory acknowledgement")
				}
			}
			if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) || fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("terminal save lost inventory changes, repeated persistence or published to closed transport")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed terminal inventory state")
			}
		})
	}
}

func TestCriticalInventoryActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		t.Run(kind, func(t *testing.T) {
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil,
				func(_ *World, p *Player) { configureCriticalInventory(p, 5, 1) })
			request := criticalInventoryRequest(kind, 1, 5)
			deliver := func() {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept inventory intention")
				}
			}
			deliver()
			proposal := awaitCriticalWrite(t, backend.entered)
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
					t.Fatal("held inventory save stalled other players, repeated or confirmed early")
				}
			}
			release()
			count := 1
			if kind == "split" {
				count = 2
			}
			deadline := time.Now().Add(2 * time.Second)
			for p.Session.QueuedPacketsForTest() < count && time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
			}
			stop() // Join the actor before reading mutable inventory or fences.
			if !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[0]) || len(queue.pending) != 0 || w.criticalPlayerReserved(p) || w.mobTickCounter < 3 || w.shuttingDown {
				t.Fatal("actual World loop did not commit inventory once and continue ticking")
			}
			slots := []int{5}
			if kind == "split" {
				slots = append(slots, 0)
			}
			assertCriticalInventoryPublication(t, p, slots)
		})
	}
}
