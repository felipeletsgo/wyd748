package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"slices"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

type criticalTinyRNG struct {
	first, calls int
}

func (r *criticalTinyRNG) Intn(n int) int {
	r.calls++
	if r.calls == 1 {
		return r.first % n
	}
	return (99 - r.first) % n // A second roll would change the prepared outcome.
}

func configureCriticalTiny(w *World, p *Player, extras, success bool) (combineRequest, *criticalTinyRNG) {
	configureCriticalInventory(p, 7, 0)
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	w.items[2723] = model.ItemDef{Index: 2723, Grade: 5, Pos: 64,
		StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}}
	w.items[2727] = model.ItemDef{Index: 2727, Grade: 5, Pos: 64}
	w.items[702] = model.ItemDef{Index: 702}
	req := combineRequest{Pos: [combineSlots]int8{62, 0, 1, 2, 3, 4, 5, 6}}
	for i := range combineSlots {
		if i >= 3 && !extras {
			continue
		}
		index := uint16(700 + i)
		if i == 0 {
			index = 2723
		} else if i == 1 {
			index = 2727
		}
		req.Items[i] = model.Item{Index: index, UID: fmt.Sprintf("tiny-ingredient-%d", i),
			ActivatedUnix: int64(123 + i), ExpiresUnix: int64(4102444800 + i)}
		if i < 3 {
			req.Items[i].Eff = [6]byte{43, 9, 7, byte(i + 1), 0, 0}
		}
		p.Char.Inv[int(req.Pos[i])] = req.Items[i]
	}
	p.Char.Gold = tinyGoldCost + 321
	mob := &Mob{ID: 1500, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
		Name: "Tiny", Tipo: model.TipoNPC, Score: &model.Score{Merchant: craftingMerchant},
	}}
	w.registerMobSpatial(mob)
	p.show(mob.ID)
	p.CraftNPC = 0 // Exercise client-side window opening without a prior click.
	rng := &criticalTinyRNG{}
	if !success {
		rng.first = 99
	}
	w.rng = rng
	return req, rng
}

func criticalTinyFixture(t *testing.T, extras, success bool) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, combineRequest, *criticalTinyRNG, func()) {
	t.Helper()
	w, p, fallback := handlerTestWorld(t)
	w.clock = newFakeClock(time.Unix(100, 0))
	req, rng := configureCriticalTiny(w, p, extras, success)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("Tiny fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 8), release: gate}
	queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, backend, queue, req, rng, release
}

func criticalTinyRequest(id uint16, req combineRequest) []byte {
	packet := buildCombinePacket(req.Items, req.Pos)
	copy(packet[:12], inboundPacket(wire.OpCombineTiny, combinePacketSize)[:12])
	binary.LittleEndian.PutUint16(packet[6:8], id)
	return packet
}

// Construct the expected economic delta independently of queueTinyCraft.
func criticalTinyExpected(before *model.Account, slot int, req combineRequest, success bool) (*model.Account, []int, percentRoll, uint32) {
	want := accountStateSnapshot(before)
	ch := &want.Chars[slot]
	var positions []int
	for i := 2; i < combineSlots; i++ {
		if req.Items[i].Index != 0 {
			ch.Inv[int(req.Pos[i])] = model.Item{}
			positions = append(positions, int(req.Pos[i]))
		}
	}
	roll, code := percentRoll{Chance: 30, Roll: 100}, uint32(2)
	if success {
		result := req.Items[0]
		result.Eff = req.Items[1].Eff
		result.Eff[1] = 7 // The fixture donor's refinement occupies effect pair zero.
		ch.Inv[int(req.Pos[0])], ch.Inv[int(req.Pos[1])] = result, model.Item{}
		positions = append(positions, int(req.Pos[0]), int(req.Pos[1]))
		roll.Roll, roll.Success, code = 1, true, 1
	}
	ch.Gold -= tinyGoldCost
	slices.Sort(positions)
	return want, positions, roll, code
}

func assertCriticalTinyPublication(t *testing.T, p *Player, positions []int, roll percentRoll, code uint32, trade bool) {
	t.Helper()
	if trade {
		assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
	}
	for _, pos := range positions {
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
	}
	assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
	assertCriticalPickupPacket(t, p, wire.MessagePanel(roll.message()))
	assertCriticalPickupPacket(t, p, wire.StandardParm(wire.OpCombineComplete, wire.SceneField, code))
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("Tiny composition published extra responses")
	}
}

func TestCriticalTinyDetachedCommitAndRepeatedRequests(t *testing.T) {
	for _, extras := range []bool{false, true} {
		for _, success := range []bool{false, true} {
			for slot := range 4 {
				t.Run(fmt.Sprintf("extras-%t/success-%t/character-%d", extras, success, slot), func(t *testing.T) {
					w, p, fallback, backend, queue, req, rng, release := criticalTinyFixture(t, extras, success)
					if slot != 0 {
						p.Account.Chars[0], p.Account.Chars[slot] = p.Account.Chars[slot], p.Account.Chars[0]
						p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
					}
					account, character := p.Account, p.Char
					before := accountStateSnapshot(account)
					want, positions, roll, code := criticalTinyExpected(before, slot, req, success)
					request := criticalTinyRequest(p.ID, req)
					w.handle(command{s: p.Session, pkt: request})
					proposal := awaitCriticalWrite(t, backend.entered)
					if len(proposal.accounts) != 1 || proposal.accounts[0] == account ||
						!reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
						t.Fatal("Tiny proposal aliases live state or loses ingredient identities, gold or unrelated fields")
					}
					lastCraft, npc := p.LastCraft, p.CraftNPC
					for _, phase := range []string{"held", "completed-unacknowledged"} {
						if phase == "completed-unacknowledged" {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						w.handle(command{s: p.Session, pkt: request})
						w.onCombineTiny(p.Session, request)
						if !reflect.DeepEqual(accountStateSnapshot(account), before) || fallback.saves != 0 ||
							p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 || len(queue.pending) != 1 ||
							!w.criticalPlayerReserved(p) || rng.calls != 1 || p.LastCraft != lastCraft || p.CraftNPC != npc {
							t.Fatalf("%s Tiny composition mutated, confirmed, rerolled or released ownership", phase)
						}
					}
					discardCriticalDeferredDuplicates(w)

					w.processCriticalCompletions(1)
					if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) ||
						fallback.saves != 0 || len(queue.pending) != 0 || w.criticalPlayerReserved(p) || rng.calls != 1 || w.shuttingDown {
						t.Fatal("Tiny commit replaced identity, lost fields, rerolled or failed to release ownership")
					}
					assertCriticalTinyPublication(t, p, positions, roll, code, false)
					p.LastCraft = time.Time{}
					w.handle(command{s: p.Session, pkt: request})
					if len(queue.pending) != 0 || len(backend.entered) != 0 || rng.calls != 1 ||
						!reflect.DeepEqual(accountStateSnapshot(account), want) {
						t.Fatal("stale Tiny ingredients were consumed or charged twice")
					}
				})
			}
		}
	}
}

func criticalTinyTradePeer(t *testing.T, w *World, p *Player) *Player {
	t.Helper()
	peer := criticalPickupCompetitor(t, w, &GroundItem{X: p.X, Y: p.Y})
	p.Trade = &TradeState{OpponentID: peer.ID, Gold: 12, Checked: true, ExpiresAt: w.now().Add(time.Minute)}
	peer.Trade = &TradeState{OpponentID: p.ID, Gold: 34, Checked: true, ExpiresAt: w.now().Add(time.Minute)}
	return peer
}

func TestCriticalTinyRetainsReciprocalTradeUntilAtomicCommit(t *testing.T) {
	for _, success := range []bool{false, true} {
		t.Run(fmt.Sprint(success), func(t *testing.T) {
			w, p, fallback, backend, queue, req, rng, release := criticalTinyFixture(t, true, success)
			peer := criticalTinyTradePeer(t, w, p)
			before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
			trade, peerTrade, offer, peerOffer := p.Trade, peer.Trade, *p.Trade, *peer.Trade
			want, positions, roll, code := criticalTinyExpected(before, p.CharSlot, req, success)
			request := criticalTinyRequest(p.ID, req)
			w.handle(command{s: p.Session, pkt: request})
			proposal := awaitCriticalWrite(t, backend.entered)
			if len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) ||
				!reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peerBefore)) {
				t.Fatal("Tiny trade cancellation did not submit one complete atomic participant batch")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.cancelTrade(p, "test competing cancellation")
				w.tickTrades(w.now().Add(time.Hour))
				w.handle(command{s: peer.Session, pkt: criticalTradeRequest(peer, p, 0, true)})
				w.onCombineTiny(p.Session, request)
				if p.Trade != trade || peer.Trade != peerTrade || *trade != offer || *peerTrade != peerOffer ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
					!w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || p.Session.QueuedPacketsForTest() != 0 ||
					peer.Session.QueuedPacketsForTest() != 0 || rng.calls != 1 || fallback.saves != 0 || len(backend.entered) != 0 {
					t.Fatalf("%s Tiny write allowed a competing trade consumer", phase)
				}
			}
			w.processCriticalCompletions(1)
			if p.Trade != nil || peer.Trade != nil || w.shuttingDown || len(queue.pending) != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), want) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) {
				t.Fatal("Tiny acknowledgement lost economic state or failed to cancel the unchanged reciprocal trade")
			}
			assertCriticalTinyPublication(t, p, positions, roll, code, true)
			assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
			if peer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("Tiny composition sent crafting results to its trade counterpart")
			}
		})
	}
}

func TestCriticalTinyRejectedAdmissionNeverFallsBack(t *testing.T) {
	for _, reason := range []string{"short", "wrong-item", "duplicate", "hidden-slot", "negative-slot", "blocked", "gold", "NPC", "outside", "closed", "unclaimed", "character", "stopping", "full", "missing-atomic", "invalid-peer"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, req, _, _ := criticalTinyFixture(t, true, true)
			var peer *Player
			request := criticalTinyRequest(p.ID, req)
			switch reason {
			case "short":
				request = request[:83]
			case "wrong-item":
				request[12]++
			case "duplicate":
				request[77] = request[76]
			case "hidden-slot":
				request[76] = model.PlayerCarrySlots
			case "negative-slot":
				request[76] = 255
			case "blocked":
				p.Char.Inv[int(req.Pos[7])].Index, req.Items[7].Index = 747, 747
				request = criticalTinyRequest(p.ID, req)
			case "gold":
				p.Char.Gold = tinyGoldCost - 1
			case "NPC":
				w.mobsByID[1500].X += npcInteractionRange + 2
			case "outside":
				p.InWorld = false
			case "closed":
				p.Session.Close()
			case "unclaimed":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "character":
				ch := cloneCharacterState(p.Char)
				p.Char = &ch
			case "stopping":
				queue.closeAdmission()
			case "full":
				for _, name := range []string{"foreign-one", "foreign-two"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			case "missing-atomic":
				peer = criticalTinyTradePeer(t, w, p)
				queue.store = backend
			case "invalid-peer":
				peer = criticalTinyTradePeer(t, w, p)
				peer.Trade.OpponentID++
			}
			before, charBefore, trade := accountStateSnapshot(p.Account), cloneCharacterState(p.Char), p.Trade
			pending := len(queue.pending)
			w.onCombineTiny(p.Session, request)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(*p.Char, charBefore) ||
				p.Trade != trade || fallback.saves != 0 || len(backend.entered) != 0 || len(queue.pending) != pending || p.criticalLifecycle != nil ||
				(peer != nil && (peer.Trade == nil || peer.Session.QueuedPacketsForTest() != 0)) {
				t.Fatal("rejected Tiny request mutated economics/trade, wrote inline or retained a proposal")
			}
			for p.Session.QueuedPacketsForTest() != 0 {
				packet, _ := p.Session.DequeuePacketForTest()
				if !wire.Decrypt(packet) {
					t.Fatal("invalid rejection response")
				}
				switch wire.ParseHeader(packet).Type {
				case wire.OpMessagePanel:
				case wire.OpCombineComplete:
					if binary.LittleEndian.Uint32(packet[12:16]) != 0 {
						t.Fatal("rejection confirmed a rolled economic outcome")
					}
				default:
					t.Fatal("rejection published economic or trade changes")
				}
			}
		})
	}
}

func TestCriticalTinyUnknownOutcomeRetainsEconomicsAndTrade(t *testing.T) {
	for _, reason := range []string{"error", "panic", "identity", "source", "gold", "trade", "offer", "peer-identity", "peer-account"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, req, rng, release := criticalTinyFixture(t, true, true)
			peer := criticalTinyTradePeer(t, w, p)
			if reason == "error" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			backend.panicAfterWrite = reason == "panic"
			request := criticalTinyRequest(p.ID, req)
			w.handle(command{s: p.Session, pkt: request})
			awaitCriticalWrite(t, backend.entered)
			switch reason {
			case "identity":
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			case "source":
				p.Char.Inv[62].UID = "replacement-instance"
			case "gold":
				p.Char.Gold++
			case "trade":
				replaced := *p.Trade
				p.Trade = &replaced
			case "offer":
				peer.Trade.Gold++
			case "peer-identity":
				peer.criticalLifecycle = &criticalPlayerLifecycle{}
			case "peer-account":
				peer.Account.CargoGold++
			}
			before, peerBefore, trade, peerTrade := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account), p.Trade, peer.Trade
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.onCombineTiny(p.Session, request)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
				p.Trade != trade || peer.Trade != peerTrade || !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned ||
				!w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || !p.Session.IsClosed() || !peer.Session.IsClosed() ||
				p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || rng.calls != 1 ||
				len(queue.pending) != 1 || len(backend.entered) != 0 || w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
				t.Fatal("uncertain Tiny result applied, published, rerolled or released ownership")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 {
				t.Fatal("shutdown overwrote an uncertain Tiny result")
			}
		})
	}
}

func TestCriticalTinyPreparedResultAndReservedPeerRejections(t *testing.T) {
	for _, reason := range []string{"result", "roll", "chance", "success", "instance", "reserved-peer"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, backend, queue, req, rng, release := criticalTinyFixture(t, true, true)
			peer := criticalTinyTradePeer(t, w, p)
			result, chance, err := w.tinyRecipe(req)
			if err != nil {
				t.Fatal(err)
			}
			roll := percentRoll{Chance: chance, Roll: 1, Success: true}
			switch reason {
			case "result":
				result.UID = "forged-result"
			case "roll":
				roll.Roll = 0
			case "chance":
				roll.Chance++
			case "success":
				roll.Success = false
			case "instance":
				req.Items[0].UID = "stale-instance-with-identical-wire-bytes"
			case "reserved-peer":
				if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{peer.Account}}, []*Player{peer},
					func(criticalPersistenceResult) error { return nil }); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, backend.entered)
			}
			before, peerBefore, trade, peerTrade := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account), p.Trade, peer.Trade
			if err := w.queueTinyCraft(p, req, result, roll); err == nil {
				t.Fatal("invalid prepared Tiny operation was admitted")
			}
			if reason == "reserved-peer" {
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.onCombineTiny(p.Session, criticalTinyRequest(p.ID, req))
					if !p.LastCraft.IsZero() || p.CraftNPC != 0 || rng.calls != 0 {
						t.Fatal("reserved peer allowed recipe context changes or a new roll")
					}
				}
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
				p.Trade != trade || peer.Trade != peerTrade || fallback.saves != 0 || len(backend.entered) != 0 ||
				p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("rejected prepared Tiny operation changed state, wrote inline or published")
			}
			if reason == "reserved-peer" {
				w.processCriticalCompletions(1)
			}
		})
	}
}

func TestCriticalTinyDisconnectPersistsCommittedAccountsAndSidecars(t *testing.T) {
	for _, closing := range []string{"owner", "peer", "both"} {
		t.Run(closing, func(t *testing.T) {
			w, p, fallback, backend, queue, req, _, release := criticalTinyFixture(t, true, true)
			peer := criticalTinyTradePeer(t, w, p)
			before := accountStateSnapshot(p.Account)
			_, positions, roll, code := criticalTinyExpected(before, p.CharSlot, req, true)
			w.handle(command{s: p.Session, pkt: criticalTinyRequest(p.ID, req)})
			proposal := awaitCriticalWrite(t, backend.entered)
			closed := []*Player{p}
			if closing == "peer" {
				closed = []*Player{peer}
			}
			if closing == "both" {
				closed = append(closed, peer)
			}
			for _, player := range closed {
				w.handle(command{s: player.Session})
			}
			if fallback.saves != 0 || len(backend.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
				p.Trade == nil || peer.Trade == nil {
				t.Fatal("disconnect saved or exposed uncommitted Tiny state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			seen := make(map[string]bool)
			for range closed {
				final := awaitCriticalWrite(t, backend.entered)
				var participant *Player
				var want *model.Account
				for i, player := range []*Player{p, peer} {
					if final.accounts[0].Name == player.Account.Name {
						participant, want = player, proposal.accounts[i]
					}
				}
				if participant == nil || seen[participant.Account.Name] || !reflect.DeepEqual(final.accounts[0], want) || final.character == nil ||
					final.character.uid != participant.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(participant, w.now())) {
					t.Fatal("final account/sidecar write lost the committed Tiny result or repeated a participant")
				}
				seen[participant.Account.Name] = true
			}
			awaitReadyCriticalResults(t, queue, len(closed))
			for _, player := range closed {
				w.handle(command{s: player.Session})
				if player.Session.QueuedPacketsForTest() != 0 || !w.criticalPlayerReserved(player) ||
					w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
					t.Fatal("final unacknowledged Tiny disconnect replied or released account ownership")
				}
			}
			w.processCriticalCompletions(len(closed))
			for i, player := range closed {
				if w.players[player.Session] != nil || !w.claimAccountSession(gameNet.NewTestSession(int64(90+i), 8), player.Account.Name) {
					t.Fatal("final Tiny disconnect acknowledgement did not release ownership")
				}
			}
			if p.Trade != nil || peer.Trade != nil || w.shuttingDown || len(queue.pending) != 0 || fallback.saves != 0 || len(backend.entered) != 0 {
				t.Fatal("Tiny disconnect failed to settle its trade or persisted inline")
			}
			if closing == "peer" {
				assertCriticalTinyPublication(t, p, positions, roll, code, true)
			}
			if closing == "owner" {
				assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
				if peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("open trade peer received Tiny results")
				}
			}
		})
	}
}

func TestCriticalTinyShutdownDrainsBeforeFinalSnapshot(t *testing.T) {
	w, p, fallback, backend, queue, req, rng, release := criticalTinyFixture(t, true, true)
	terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
	w.store = terminal
	before := accountStateSnapshot(p.Account)
	w.handle(command{s: p.Session, pkt: criticalTinyRequest(p.ID, req)})
	proposal := awaitCriticalWrite(t, backend.entered)
	first, second := make(chan error, 1), make(chan error, 1)
	w.runShutdown(first)
	w.runShutdown(second)
	w.handle(command{s: p.Session})
	if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished ||
		!p.Session.IsClosed() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
		t.Fatal("shutdown saved or applied uncommitted Tiny state")
	}
	for _, done := range []chan error{first, second} {
		select {
		case <-done:
			t.Fatal("shutdown finished before Tiny acknowledgement")
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
			t.Fatal("shutdown failed to finish after Tiny acknowledgement")
		}
	}
	if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
		fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 || rng.calls != 1 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("final snapshot lost committed Tiny state, repeated persistence or replied to a closed transport")
	}
	third := make(chan error, 1)
	w.runShutdown(third)
	if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
		t.Fatal("repeated shutdown replayed Tiny state")
	}
}

func TestCriticalTinyActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, outcome := range []string{"success", "failure", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			var failure error
			if outcome == "unknown" {
				failure = errors.New("Tiny acknowledgement lost")
			}
			var req combineRequest
			var rng *criticalTinyRNG
			var before, want *model.Account
			var positions []int
			var roll percentRoll
			var code uint32
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, p *Player) {
				req, rng = configureCriticalTiny(w, p, true, outcome != "failure")
				before = accountStateSnapshot(p.Account)
				want, positions, roll, code = criticalTinyExpected(before, p.CharSlot, req, outcome != "failure")
			})
			request := criticalTinyRequest(p.ID, req)
			deliver := func() {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("actor did not accept Tiny intention")
				}
			}
			deliver()
			proposal := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
				t.Fatal("actual Tiny dispatcher submitted the wrong result")
			}
			for range 3 {
				deliverCriticalLoopPing(t, w, p)
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held Tiny write blocked unrelated ticks/commands or published before commit")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for {
				overview := readCriticalLoopOverview(t, w)
				if (outcome == "unknown" && overview.State == "maintenance") ||
					(outcome != "unknown" && p.Session.QueuedPacketsForTest() >= len(positions)+3) {
					break
				}
				if time.Now().After(deadline) {
					t.Fatal("actor did not deliver Tiny outcome")
				}
			}
			stop() // Join World before inspecting its mutable state and RNG.
			if w.mobTickCounter < 3 || len(backend.entered) != 0 || rng.calls != 1 {
				t.Fatal("Tiny blocked unrelated ticks or rerolled/repeated persistence")
			}
			if outcome == "unknown" {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 1 ||
					!p.PersistencePoisoned || !p.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 ||
					!errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
					t.Fatal("unknown actor Tiny result applied, published or released ownership")
				}
			} else {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || len(queue.pending) != 0 || w.shuttingDown {
					t.Fatal("actor did not settle Tiny once")
				}
				assertCriticalTinyPublication(t, p, positions, roll, code, false)
			}
		})
	}
}
