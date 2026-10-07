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

func configureCriticalTrade(t *testing.T, w *World, a, b *Player, aSlot, bSlot int, seals string) {
	t.Helper()
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	for i, p := range []*Player{a, b} {
		active := *p.Char
		p.Account.Chars = make([]model.Char, 4)
		slot := aSlot
		if i == 1 {
			slot = bSlot
		}
		p.Account.Chars[slot] = active
		p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
		p.Char.Inv = [model.MaxCarry]model.Item{}
		p.Char.Inv[model.PlayerCarrySlots] = model.Item{Index: 200, UID: fmt.Sprintf("hidden-%d", p.ID)}
		p.Account.Cargo[0] = model.Item{Index: 201, UID: fmt.Sprintf("cargo-%d", p.ID)}
		p.Account.Chars[3].Gold = 777
	}
	a.Char.Gold, b.Char.Gold = 1000, 2000
	a.Char.Inv[5] = model.Item{Index: 412, UID: "11111111111141118111111111111111",
		Eff: [6]byte{7, 3, 8, 4, 9, 5}, ActivatedUnix: 123, ExpiresUnix: 4102444800}
	b.Char.Inv[62] = model.Item{Index: 413, UID: "22222222222242228222222222222222",
		Eff: [6]byte{9, 5, 8, 4, 7, 3}, ActivatedUnix: 456, ExpiresUnix: 4102444801}
	if seals == "a" || seals == "both" {
		tradeCapsuleFixture(t, a, 1, 5)
	}
	if seals == "b" || seals == "both" {
		tradeCapsuleFixture(t, b, 1, 62)
	}
	w.items[412], w.items[413] = model.ItemDef{Index: 412}, model.ItemDef{Index: 413}
	w.items[model.CelestialSealItem] = model.ItemDef{Index: model.CelestialSealItem}
	aPos, bPos := emptyTradePositions(), emptyTradePositions()
	aPos[0], bPos[0] = 5, 62
	a.Trade = &TradeState{OpponentID: b.ID, Checked: true, CarryPos: aPos, Gold: 300,
		Items: [maxTradeItems]model.Item{a.Char.Inv[5]}, ExpiresAt: w.now().Add(time.Minute)}
	b.Trade = &TradeState{OpponentID: a.ID, Checked: true, CarryPos: bPos, Gold: 500,
		Items: [maxTradeItems]model.Item{b.Char.Inv[62]}, ExpiresAt: w.now().Add(time.Minute)}
}

func criticalTradeFixture(t *testing.T, aSlot, bSlot int, seals string) (*World, *Player, *Player, *batchGameStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	a, _ := networkedTestPlayer(1, "TradeOwnerA", 2100, 2100)
	b, _ := networkedTestPlayer(3, "TradeOwnerB", 2101, 2100)
	w, fallback := economyWorld(a, b)
	configureCriticalTrade(t, w, a, b, aSlot, bSlot, seals)
	for _, p := range []*Player{a, b} {
		if !w.claimAccountSession(p.Session, p.Account.Name) {
			t.Fatal("trade fixture account admission failed")
		}
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 8), release: gate}
	queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, a, b, fallback, backend, queue, release
}

func criticalTradeRequest(p, opponent *Player, gold uint32, checked bool, slots ...int) []byte {
	packet := tradeOfferPacket(opponent.ID, p.Char, gold, checked, slots...)
	binary.LittleEndian.PutUint16(packet[0:2], uint16(len(packet)))
	binary.LittleEndian.PutUint16(packet[6:8], p.ID)
	return packet
}

// Build the expected exchange independently of the production trade planner.
func criticalTradeExpected(a, b *Player, beforeA, beforeB *model.Account, seals string) (*model.Account, *model.Account) {
	wantA, wantB := accountStateSnapshot(beforeA), accountStateSnapshot(beforeB)
	ac, bc := &wantA.Chars[a.CharSlot], &wantB.Chars[b.CharSlot]
	aItem, bItem := ac.Inv[5], bc.Inv[62]
	ac.Inv[5], bc.Inv[62] = model.Item{}, model.Item{}
	ac.Inv[0], bc.Inv[0] = bItem, aItem
	ac.Gold, bc.Gold = 1200, 1800
	wantA.CelestialCapsules = make([]model.CelestialCapsule, 0)
	wantB.CelestialCapsules = make([]model.CelestialCapsule, 0)
	if seals == "a" || seals == "both" {
		capsule := beforeA.CelestialCapsules[0]
		if seals == "both" {
			// The recipient's outgoing capsule still owns ID 1 in the first leg.
			capsule.ID = 2
			bc.Inv[0] = model.NewCelestialSeal(capsule.ItemUID, 2)
		}
		wantB.CelestialCapsules = append(wantB.CelestialCapsules, capsule)
	}
	if seals == "b" || seals == "both" {
		wantA.CelestialCapsules = append(wantA.CelestialCapsules, beforeB.CelestialCapsules[0])
	}
	return wantA, wantB
}

func assertCriticalTradePublication(t *testing.T, p *Player) {
	t.Helper()
	for _, want := range [][]byte{wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold), wire.CloseTrade(p.ID)} {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || len(packet) != len(want) {
			t.Fatal("missing or incorrectly sized trade confirmation")
		}
		header := wire.ParseHeader(packet)
		if header.Type != wire.ParseHeader(want).Type || header.ID != p.ID || int(header.Size) != len(want) || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatal("trade confirmation recipient, order or body differs from committed state")
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("trade published extra responses")
	}
}

func TestCriticalTradeDetachedAtomicCommitAndHeldConsumers(t *testing.T) {
	for _, aSlot := range []int{0, 1} {
		for _, bSlot := range []int{0, 1} {
			for _, seals := range []string{"none", "a", "b", "both"} {
				t.Run(fmt.Sprintf("a-%d/b-%d/seals-%s", aSlot, bSlot, seals), func(t *testing.T) {
					w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, aSlot, bSlot, seals)
					beforeA, beforeB := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
					wantA, wantB := criticalTradeExpected(a, b, beforeA, beforeB, seals)
					aAccount, bAccount, aChar, bChar := a.Account, b.Account, a.Char, b.Char
					aTrade, bTrade, aOffer, bOffer := a.Trade, b.Trade, *a.Trade, *b.Trade
					third, _ := networkedTestPlayer(4, "ThirdTrader", 2102, 2100)
					w.players[third.Session], w.playersByID[third.ID] = third, third
					w.commitTrade(a, b)
					proposal := awaitCriticalWrite(t, backend.entered)
					if len(proposal.accounts) != 2 || proposal.accounts[0] == a.Account || proposal.accounts[1] == b.Account ||
						!reflect.DeepEqual(proposal.accounts, accountPersistenceSnapshots(wantA, wantB)) {
						t.Fatal("trade proposal aliases live state or fails to capture both complete accounts")
					}
					checkHeld := func() {
						for _, p := range []*Player{a, b} {
							w.onTrade(p.Session, []byte{1})
							w.onCloseTrade(p.Session)
							w.cancelTrade(p, "held test cancellation")
						}
						w.tickTrades(w.now().Add(time.Hour))
						w.onTrade(third.Session, criticalTradeRequest(third, b, 0, false))
						w.commitTrade(a, b)
						if !reflect.DeepEqual(accountStateSnapshot(a.Account), beforeA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), beforeB) ||
							a.Trade != aTrade || b.Trade != bTrade || *aTrade != aOffer || *bTrade != bOffer || third.Trade != nil ||
							fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 || len(queue.pending) != 1 ||
							!w.criticalPlayerReserved(a) || !w.criticalPlayerReserved(b) || a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("held trade mutated context, repeated, saved inline, published or released either participant")
						}
					}
					checkHeld()
					release()
					awaitReadyCriticalResults(t, queue, 1)
					checkHeld()
					w.processCriticalCompletions(1)
					if a.Account != aAccount || b.Account != bAccount || a.Char != aChar || b.Char != bChar ||
						!reflect.DeepEqual(accountStateSnapshot(a.Account), wantA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), wantB) ||
						a.Trade != nil || b.Trade != nil || len(queue.pending) != 0 || w.criticalPlayerReserved(a) || w.criticalPlayerReserved(b) {
						t.Fatal("trade delivery lost fields, replaced live identity or retained successful reservations")
					}
					assertCriticalTradePublication(t, a)
					assertCriticalTradePublication(t, b)
				})
			}
		}
	}
}

func TestCriticalTradeRejectsWithoutEconomicMutationOrInlineFallback(t *testing.T) {
	for _, reason := range []string{"unsupported", "full", "stopping", "unclaimed", "wrong-character", "unchecked", "nonreciprocal", "outside", "dead", "distance", "closed", "poisoned", "shutdown", "gold", "gold-cap", "inventory-full", "changed-item", "unknown-item", "nontradable-item", "foreign-capsule", "duplicate-capsule"} {
		t.Run(reason, func(t *testing.T) {
			w, a, b, fallback, backend, queue, _ := criticalTradeFixture(t, 0, 1, "both")
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
				w.releaseAccountSession(b.Session, b.Account)
			case "wrong-character":
				copy := *b.Char
				b.Char = &copy
			case "unchecked":
				b.Trade.Checked = false
			case "nonreciprocal":
				b.Trade.OpponentID = 100
			case "outside":
				b.InWorld = false
			case "dead":
				setPlayerCurHP(b.Char, 0)
			case "distance":
				b.X = 3000
			case "closed":
				b.Session.Close()
			case "poisoned":
				b.PersistencePoisoned = true
			case "shutdown":
				w.shuttingDown = true
			case "gold":
				b.Trade.Gold = b.Char.Gold + 1
			case "gold-cap":
				a.Char.Gold = maxCharacterGold
			case "inventory-full":
				for i := 0; i < model.PlayerCarrySlots; i++ {
					b.Char.Inv[i] = model.Item{Index: 200}
				}
				b.Trade.Items = [maxTradeItems]model.Item{}
				b.Trade.CarryPos = emptyTradePositions()
			case "changed-item":
				b.Char.Inv[62].UID = "replaced-item"
			case "unknown-item":
				delete(w.items, model.CelestialSealItem)
			case "nontradable-item":
				w.items[model.CelestialSealItem] = model.ItemDef{Index: model.CelestialSealItem,
					StaticEffects: []model.StaticEffect{{Name: "EF_NOTRADE", Value: 1}}}
			case "foreign-capsule":
				b.Account.CelestialCapsules[0].ItemUID = "foreign-seal"
			case "duplicate-capsule":
				b.Account.CelestialCapsules[0].SourceUID = a.Account.CelestialCapsules[0].SourceUID
			}
			beforeA, beforeB := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
			pending := len(queue.pending)
			w.commitTrade(a, b)
			if !reflect.DeepEqual(accountStateSnapshot(a.Account), beforeA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), beforeB) ||
				fallback.saves != 0 || fallback.batchSaves != 0 || len(queue.pending) != pending || len(backend.entered) != 0 || len(w.criticalWrites.continuations) != 0 {
				t.Fatal("rejected trade mutated economics, submitted or fell back to synchronous persistence")
			}
			for _, p := range []*Player{a, b} {
				for packet, ok := p.Session.DequeuePacketForTest(); ok; packet, ok = p.Session.DequeuePacketForTest() {
					if !wire.Decrypt(packet) {
						t.Fatal("rejection packet is invalid")
					}
					typ := wire.ParseHeader(packet).Type
					if typ != wire.OpMessagePanel && typ != wire.OpCloseTrade {
						t.Fatalf("rejected trade published economic success: opcode %x", typ)
					}
				}
			}
		})
	}
}

func TestCriticalTradeUnknownOutcomeRetainsBothFences(t *testing.T) {
	for _, reason := range []string{"error", "panic", "identity", "trade-pointer", "trade-offer"} {
		t.Run(reason, func(t *testing.T) {
			w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, 1, 0, "both")
			backend.panicAfterWrite = reason == "panic"
			if reason == "error" {
				backend.failure = errors.New("trade commit acknowledgement lost")
			}
			beforeA, beforeB := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
			w.commitTrade(a, b)
			awaitCriticalWrite(t, backend.entered)
			switch reason {
			case "identity":
				b.criticalLifecycle = &criticalPlayerLifecycle{}
			case "trade-pointer":
				copy := *b.Trade
				b.Trade = &copy
			case "trade-offer":
				b.Trade.Gold++
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.commitTrade(a, b)
			if !reflect.DeepEqual(accountStateSnapshot(a.Account), beforeA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), beforeB) ||
				!w.shuttingDown || len(queue.pending) != 1 || fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 {
				t.Fatal("unknown trade applied, retried or overwrote either account")
			}
			for _, p := range []*Player{a, b} {
				if !p.PersistencePoisoned || !p.Session.IsClosed() || !w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() != 0 ||
					w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
					t.Fatal("unknown trade confirmed or released participant ownership")
				}
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 || fallback.batchSaves != 0 {
				t.Fatal("terminal cleanup overwrote an uncertain two-account transaction")
			}
		})
	}
}

func TestCriticalTradeReciprocalCancellationWaitsForOtherOperation(t *testing.T) {
	w, a, b, _, backend, queue, release := criticalTradeFixture(t, 0, 0, "none")
	aTrade, bTrade, aOffer, bOffer := a.Trade, b.Trade, *a.Trade, *b.Trade
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{b.Account}}, []*Player{b}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.cancelTrade(a, "counterpart operation held")
		w.onCloseTrade(a.Session)
		w.onTrade(a.Session, []byte{1})
		w.tickTrades(w.now().Add(time.Hour))
		if a.Trade != aTrade || b.Trade != bTrade || *aTrade != aOffer || *bTrade != bOffer || a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 {
			t.Fatalf("%s reciprocal cancellation invalidated the other participant's operation", phase)
		}
	}
	w.processCriticalCompletions(1)
	w.cancelTrade(a, "counterpart acknowledged")
	if a.Trade != nil || b.Trade != nil || a.Session.QueuedPacketsForTest() != 1 || b.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("acknowledged participant remained locked against ordinary cancellation")
	}
}

func TestCriticalTradeAllowsNewOperationAfterAcknowledgement(t *testing.T) {
	w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, 0, 1, "none")
	w.commitTrade(a, b)
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	assertCriticalTradePublication(t, a)
	assertCriticalTradePublication(t, b)
	w.commitTrade(a, b)
	if len(queue.pending) != 0 || len(backend.entered) != 0 {
		t.Fatal("completed trade could be submitted again without a new context")
	}
	positions := emptyTradePositions()
	positions[0] = 0
	a.Trade = &TradeState{OpponentID: b.ID, Checked: true, Gold: 100,
		CarryPos: positions, Items: [maxTradeItems]model.Item{a.Char.Inv[0]}}
	b.Trade = &TradeState{OpponentID: a.ID, Checked: true, Gold: 200,
		CarryPos: positions, Items: [maxTradeItems]model.Item{b.Char.Inv[0]}}
	w.commitTrade(a, b)
	second := awaitCriticalWrite(t, backend.entered)
	if second.accounts[0].Chars[a.CharSlot].Gold != 1300 || second.accounts[1].Chars[b.CharSlot].Gold != 1700 {
		t.Fatal("new trade did not use the acknowledged balances")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if a.Char.Inv[0].Index != 412 || b.Char.Inv[0].Index != 413 || a.Char.Gold != 1300 || b.Char.Gold != 1700 ||
		fallback.saves != 0 || fallback.batchSaves != 0 || len(queue.pending) != 0 {
		t.Fatal("new valid trade was suppressed or lost its economic result")
	}
	assertCriticalTradePublication(t, a)
	assertCriticalTradePublication(t, b)
}

func TestCriticalTradeDisconnectFinalSnapshotsContainBothCommitResults(t *testing.T) {
	for _, both := range []bool{false, true} {
		t.Run(fmt.Sprintf("both-%t", both), func(t *testing.T) {
			w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, 1, 0, "both")
			w.commitTrade(a, b)
			proposal := awaitCriticalWrite(t, backend.entered)
			closed := []*Player{a}
			if both {
				closed = append(closed, b)
			}
			for _, p := range closed {
				w.handle(command{s: p.Session})
			}
			if fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 || a.Char.Gold != 1000 || b.Char.Gold != 2000 {
				t.Fatal("disconnect saved or exposed uncommitted trade state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			for range closed {
				final := awaitCriticalWrite(t, backend.entered)
				var participant *Player
				var want *model.Account
				for i, p := range []*Player{a, b} {
					if final.accounts[0].Name == p.Account.Name {
						participant, want = p, proposal.accounts[i]
					}
				}
				if participant == nil || !reflect.DeepEqual(final.accounts[0], want) || final.character == nil ||
					final.character.uid != participant.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(participant, w.now())) {
					t.Fatal("final account/sidecar write lost the committed trade result")
				}
			}
			awaitReadyCriticalResults(t, queue, len(closed))
			for _, p := range closed {
				w.handle(command{s: p.Session})
				if p.Session.QueuedPacketsForTest() != 0 || !w.criticalPlayerReserved(p) || w.claimAccountSession(gameNet.NewTestSession(98, 8), p.Account.Name) {
					t.Fatal("unacknowledged disconnect confirmed, repeated or released ownership")
				}
			}
			w.processCriticalCompletions(len(closed))
			for i, p := range closed {
				if w.players[p.Session] != nil || !w.claimAccountSession(gameNet.NewTestSession(int64(90+i), 8), p.Account.Name) {
					t.Fatal("final disconnect acknowledgement did not release account ownership")
				}
			}
			if len(queue.pending) != 0 || fallback.saves != 0 || fallback.batchSaves != 0 || len(backend.entered) != 0 {
				t.Fatal("disconnect repeated or synchronously overwrote committed trade")
			}
			if !both {
				assertCriticalTradePublication(t, b)
			}
		})
	}
}

type criticalTradeTerminalStore struct {
	*batchGameStore
	accounts map[string]*model.Account
	flushes  int
}

func (s *criticalTradeTerminalStore) SaveAccount(account *model.Account) error {
	s.saves++
	s.accounts[account.Name] = accountStateSnapshot(account)
	return nil
}

func (s *criticalTradeTerminalStore) Flush() { s.flushes++ }

func TestCriticalTradeRepeatedShutdownWaitsForBothAccountResults(t *testing.T) {
	w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, 0, 1, "both")
	terminal := &criticalTradeTerminalStore{batchGameStore: fallback, accounts: make(map[string]*model.Account)}
	w.store = terminal
	w.commitTrade(a, b)
	proposal := awaitCriticalWrite(t, backend.entered)
	first, second := make(chan error, 1), make(chan error, 1)
	w.runShutdown(first)
	w.runShutdown(second)
	for _, p := range []*Player{a, b} {
		w.handle(command{s: p.Session})
	}
	if len(terminal.accounts) != 0 || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished {
		t.Fatal("shutdown saved old state before the two-account transaction settled")
	}
	for _, done := range []chan error{first, second} {
		select {
		case <-done:
			t.Fatal("shutdown completed while trade was held")
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
			t.Fatal("shutdown did not complete after trade acknowledgement")
		}
	}
	for _, want := range proposal.accounts {
		if !reflect.DeepEqual(accountPersistenceSnapshot(terminal.accounts[want.Name]), want) {
			t.Fatal("shutdown lost items, gold or capsule ownership from the committed trade")
		}
	}
	third := make(chan error, 1)
	w.runShutdown(third)
	if err := <-third; err != nil || fallback.saves != 2 || terminal.flushes != 1 || len(queue.pending) != 0 ||
		a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("repeated shutdown replayed persistence or confirmed to closed sessions")
	}
}

func TestCriticalTradeActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, failed := range []bool{false, true} {
		t.Run(fmt.Sprintf("unknown-%t", failed), func(t *testing.T) {
			var b *Player
			var failure error
			if failed {
				failure = errors.New("trade durable outcome unknown")
			}
			w, a, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, a *Player) {
				b, _ = networkedTestPlayer(3, "TradeLoopPeer", 2101, 2100)
				w.players[b.Session], w.playersByID[b.ID] = b, b
				w.indexPlayerCharacter(b)
				w.updatePlayerSpatial(b)
				if !w.claimAccountSession(b.Session, b.Account.Name) {
					t.Fatal("loop trade peer admission failed")
				}
				configureCriticalTrade(t, w, a, b, 0, 1, "both")
				b.NextCPRecovery, b.NextRegen = w.now().Add(time.Hour), w.now().Add(time.Hour)
				b.Trade.Checked = false
				w.criticalWrites.queue.store = &criticalStateTestStore{&criticalBatchTestStore{w.criticalWrites.queue.store.(*criticalTestStore)}}
			})
			beforeA, beforeB := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
			request := criticalTradeRequest(b, a, 500, true, 62)
			deliver := func(packet []byte) {
				select {
				case w.commands <- command{s: b.Session, pkt: packet}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept trade intention")
				}
			}
			deliver(request)
			proposal := awaitCriticalWrite(t, backend.entered)
			// These acknowledge the offered intention, not a successful transfer.
			for _, p := range []*Player{a, b} {
				packet, ok := p.Session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).ID != p.ID {
					t.Fatal("missing trade offer acknowledgement")
				}
				want := uint16(wire.OpTrade)
				if p == b {
					want = wire.OpCNFTradeCheck
				}
				if wire.ParseHeader(packet).Type != want || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("trade published economic success before persistence")
				}
			}
			for range 3 {
				deliver(request)
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, other := range overview.Players {
					if other.UID == unrelatedUID && other.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("held trade stalled unrelated World work, repeated or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
				if failed && a.Session.IsClosed() || !failed && a.Session.QueuedPacketsForTest() >= 2 && b.Session.QueuedPacketsForTest() >= 2 {
					break
				}
			}
			stop() // Join the actor before inspecting mutable account state/fences.
			if w.mobTickCounter < 3 {
				t.Fatal("World did not advance during held trade persistence")
			}
			if failed {
				if !w.shuttingDown || len(queue.pending) != 1 || !a.PersistencePoisoned || !b.PersistencePoisoned ||
					!reflect.DeepEqual(accountStateSnapshot(a.Account), beforeA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), beforeB) ||
					a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("actual World loop failed to contain an unknown trade without publication")
				}
			} else {
				// The final confirmation was from B, so its proposal is first.
				if w.shuttingDown || len(queue.pending) != 0 || !reflect.DeepEqual(accountPersistenceSnapshots(b.Account, a.Account), proposal.accounts) {
					t.Fatal("actual dispatcher lost either account or did not acknowledge the trade")
				}
				assertCriticalTradePublication(t, a)
				assertCriticalTradePublication(t, b)
			}
		})
	}
}
