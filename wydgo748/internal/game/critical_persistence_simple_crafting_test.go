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

var criticalSimpleArtisans = []string{"Compositor", "Agatha", "Aylin"}

func configureCriticalSimpleCraft(w *World, p *Player, artisan string, success bool) (combineRequest, *criticalTinyRNG) {
	_, rng := configureCriticalTiny(w, p, false, success)
	p.Char.Inv[1] = model.Item{}
	w.mobsByID[1500].Def.Name = artisan
	req := combineRequest{Pos: [combineSlots]int8{62, 0, 1, 2, 3, 4, 5, 6}}
	indices := []uint16{700, 701, 3140, 3140, 3140, 3140}
	w.items[700] = model.ItemDef{Index: 700, Grade: 5, Pos: 2,
		StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}}
	w.items[701] = model.ItemDef{Index: 701, Grade: 1, Pos: 2,
		StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 5}}}
	switch artisan {
	case "Compositor":
		indices = []uint16{700, 2441, 704, 705, 704, 705}
		w.items[700] = model.ItemDef{Index: 700, Unique: 45, Extra: 800, Pos: 64,
			StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 4}}}
		w.items[800] = model.ItemDef{Index: 800}
		for i, level := range []int{4, 5} {
			w.items[uint16(704+i)] = model.ItemDef{Index: uint16(704 + i), Pos: 2,
				StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: level}}}
		}
		// A synthetic configuration keeps both outcomes reachable and proves
		// the configured refinement policy, not native gameplay parity.
		w.gameplay.CompositorBaseChance = 10
		w.gameplay.CompositorRefineChance[2] = 8
	case "Aylin":
		indices = []uint16{700, 700, 1774, 2441, 2441, 2441, 2441}
		p.Char.Gold = aylinGoldCost + 321
	}
	for i, index := range indices {
		item := model.Item{Index: index, UID: fmt.Sprintf("%s-ingredient-%d", artisan, i),
			ActivatedUnix: int64(123 + i), ExpiresUnix: int64(4102444800 + i)}
		if i < 2 || artisan == "Compositor" && i >= 2 {
			item.Eff = [6]byte{43, 9, 7, byte(i + 1), 0, 0}
		}
		req.Items[i], p.Char.Inv[int(req.Pos[i])] = item, item
	}
	return req, rng
}

func criticalSimpleCraftFixture(t *testing.T, artisan string, success bool) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, combineRequest, *criticalTinyRNG, func()) {
	t.Helper()
	w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, false, success)
	req, rng := configureCriticalSimpleCraft(w, p, artisan, success)
	return w, p, fallback, backend, queue, req, rng, release
}

func criticalSimpleCraftRequest(id uint16, req combineRequest, artisan string) []byte {
	packet := criticalTinyRequest(id, req)
	opcode := map[string]uint16{"Compositor": wire.OpCombineCompositor, "Agatha": wire.OpCombineAgatha, "Aylin": wire.OpCombineAylin}[artisan]
	binary.LittleEndian.PutUint16(packet[4:6], opcode)
	return packet
}

func callCriticalSimpleCraft(w *World, p *Player, packet []byte, artisan string) {
	switch artisan {
	case "Compositor":
		w.onCombineCompositor(p.Session, packet)
	case "Agatha":
		w.onCombineAgatha(p.Session, packet)
	case "Aylin":
		w.onCombineAylin(p.Session, packet)
	}
}

// Build the delta independently of the extracted recipe and queue implementation.
func criticalSimpleCraftExpected(before *model.Account, slot int, req combineRequest, artisan string, success bool) (*model.Account, []int, percentRoll, uint32) {
	want := accountStateSnapshot(before)
	ch := &want.Chars[slot]
	from, to, chance := 2, 5, 41
	switch artisan {
	case "Compositor":
		from, chance = 0, 42
	case "Aylin":
		to, chance = 6, 40
		ch.Gold -= aylinGoldCost
	}
	changed := make(map[int]bool)
	for i := from; i <= to; i++ {
		pos := int(req.Pos[i])
		ch.Inv[pos] = model.Item{}
		changed[pos] = true
	}
	main, donor := int(req.Pos[0]), int(req.Pos[1])
	if artisan == "Agatha" {
		ch.Inv[main], changed[main] = model.Item{}, true
	}
	roll, code := percentRoll{Chance: chance, Roll: 100}, uint32(2)
	if success {
		result := req.Items[0]
		switch artisan {
		case "Compositor":
			result.Index, result.Eff[1] = 800, 7
		case "Agatha":
			result.Eff = req.Items[1].Eff
			result.Eff[1] = 7
		case "Aylin":
			result.Eff[1] = 230 // Grade 5 (Diamond Ancient) requires four 2441 gems.
		}
		ch.Inv[main], changed[main] = result, true
		if artisan != "Compositor" {
			ch.Inv[donor], changed[donor] = model.Item{}, true
		}
		roll.Roll, roll.Success, code = 1, true, 1
	}
	positions := make([]int, 0, len(changed))
	for pos := range changed {
		positions = append(positions, pos)
	}
	slices.Sort(positions)
	return want, positions, roll, code
}

func TestCriticalSimpleCraftDetachedCommitAndReplay(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, success := range []bool{false, true} {
			for slot := range 4 {
				t.Run(fmt.Sprintf("%s/success-%t/character-%d", artisan, success, slot), func(t *testing.T) {
					w, p, fallback, backend, queue, req, rng, release := criticalSimpleCraftFixture(t, artisan, success)
					if slot != 0 {
						p.Account.Chars[0], p.Account.Chars[slot] = p.Account.Chars[slot], p.Account.Chars[0]
						p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
					}
					account, character := p.Account, p.Char
					before := accountStateSnapshot(account)
					want, positions, roll, code := criticalSimpleCraftExpected(before, slot, req, artisan, success)
					request := criticalSimpleCraftRequest(p.ID, req, artisan)
					w.handle(command{s: p.Session, pkt: request})
					proposal := awaitCriticalWrite(t, backend.entered)
					if len(proposal.accounts) != 1 || proposal.accounts[0] == account || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
						t.Fatal("detached composition proposal lost identities, unrelated fields or recipe semantics")
					}
					lastCraft, npc := p.LastCraft, p.CraftNPC
					for _, phase := range []string{"held", "completed-unacknowledged"} {
						if phase == "completed-unacknowledged" {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						w.handle(command{s: p.Session, pkt: request})
						callCriticalSimpleCraft(w, p, request, artisan)
						if !reflect.DeepEqual(accountStateSnapshot(account), before) || fallback.saves != 0 || len(backend.entered) != 0 ||
							p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || !w.criticalPlayerReserved(p) ||
							rng.calls != 1 || p.LastCraft != lastCraft || p.CraftNPC != npc {
							t.Fatalf("%s composition changed, rerolled, confirmed or released pending state", phase)
						}
					}
					discardCriticalDeferredDuplicates(w)

					w.processCriticalCompletions(1)
					if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) ||
						len(queue.pending) != 0 || w.criticalPlayerReserved(p) || w.shuttingDown || rng.calls != 1 || fallback.saves != 0 {
						t.Fatal("composition failed to install its committed delta once")
					}
					assertCriticalTinyPublication(t, p, positions, roll, code, false)
					p.LastCraft = time.Time{}
					w.handle(command{s: p.Session, pkt: request})
					if len(queue.pending) != 0 || len(backend.entered) != 0 || rng.calls != 1 || !reflect.DeepEqual(accountStateSnapshot(account), want) {
						t.Fatal("stale composition request repeated consumption or charging")
					}
				})
			}
		}
	}
}

func TestCriticalSimpleCraftRetainsTradeAcrossAtomicCommit(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, success := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/%t", artisan, success), func(t *testing.T) {
				w, p, fallback, backend, queue, req, rng, release := criticalSimpleCraftFixture(t, artisan, success)
				peer := criticalTinyTradePeer(t, w, p)
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				trade, peerTrade, offer, peerOffer := p.Trade, peer.Trade, *p.Trade, *peer.Trade
				want, positions, roll, code := criticalSimpleCraftExpected(before, p.CharSlot, req, artisan, success)
				request := criticalSimpleCraftRequest(p.ID, req, artisan)
				w.handle(command{s: p.Session, pkt: request})
				proposal := awaitCriticalWrite(t, backend.entered)
				if len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) ||
					!reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peerBefore)) {
					t.Fatal("composition did not persist one atomic reciprocal participant batch")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.cancelTrade(p, "competing cancellation")
					w.tickTrades(w.now().Add(time.Hour))
					w.handle(command{s: peer.Session, pkt: criticalTradeRequest(peer, p, 0, true)})
					callCriticalSimpleCraft(w, p, request, artisan)
					if p.Trade != trade || peer.Trade != peerTrade || *trade != offer || *peerTrade != peerOffer ||
						!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
						!w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || p.Session.QueuedPacketsForTest() != 0 ||
						peer.Session.QueuedPacketsForTest() != 0 || rng.calls != 1 || fallback.saves != 0 || len(backend.entered) != 0 {
						t.Fatalf("%s composition admitted a conflicting trade consumer", phase)
					}
				}
				discardCriticalDeferredDuplicates(w)

				w.processCriticalCompletions(1)
				if p.Trade != trade || peer.Trade != peerTrade || *trade != offer || *peerTrade != peerOffer ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), want) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
					len(queue.pending) != 0 || w.shuttingDown || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("composition cancelled the retained trade or lost committed state")
				}
				assertCriticalTinyPublication(t, p, positions, roll, code, false)
				w.cancelTrade(p, "normal cancellation after composition")
				if p.Trade != nil || peer.Trade != nil {
					t.Fatal("acknowledged composition failed to release trade reservations")
				}
				assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
				assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
			})
		}
	}
}

func TestCriticalSimpleCraftRetainedTradeRejectsConsumedOffer(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, success := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/%t", artisan, success), func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalSimpleCraftFixture(t, artisan, success)
				peer := criticalTinyTradePeer(t, w, p)
				p.Trade.CarryPos, peer.Trade.CarryPos = emptyTradePositions(), emptyTradePositions()
				p.Trade.CarryPos[0], p.Trade.Items[0] = req.Pos[2], req.Items[2]
				p.Trade.Gold, peer.Trade.Gold = 0, 0
				// Admit a genuinely tradable offer; the rejection must come from
				// the changed inventory, not a missing catalog or capsule entry.
				if _, exists := w.items[req.Items[2].Index]; !exists {
					w.items[req.Items[2].Index] = model.ItemDef{Index: req.Items[2].Index}
				}
				if !w.tradeRequestValid(p, peer) || w.validateTradableItems(p.Trade.Items) != nil ||
					w.validateTradeCapsules(p.Account, p.Trade.Items) != nil {
					t.Fatal("fixture did not reach valid final trade validation")
				}
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				want, positions, roll, code := criticalSimpleCraftExpected(before, p.CharSlot, req, artisan, success)
				w.handle(command{s: p.Session, pkt: criticalSimpleCraftRequest(p.ID, req, artisan)})
				awaitCriticalWrite(t, backend.entered)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				assertCriticalTinyPublication(t, p, positions, roll, code, false)
				if p.Trade == nil || peer.Trade == nil || p.Char.Inv[int(req.Pos[2])].Index != 0 {
					t.Fatal("composition did not retain the trade while consuming its offered material")
				}
				w.commitTrade(p, peer)
				if p.Trade != nil || peer.Trade != nil || len(queue.pending) != 0 || len(backend.entered) != 0 ||
					fallback.saves != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), want) ||
					!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || w.shuttingDown {
					t.Fatal("stale trade transferred a consumed item, overwrote composition or submitted persistence")
				}
				for _, player := range []*Player{p, peer} {
					assertCriticalPickupPacket(t, player, wire.MessagePanel("No space, invalid gold, or the offer changed."))
					assertCriticalPickupPacket(t, player, wire.CloseTrade(player.ID))
					if player.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("rejected stale trade published an economic result")
					}
				}
			})
		}
	}
}

func TestCriticalSimpleCraftTradeDisconnectRetainsBothParticipants(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, closing := range []string{"owner", "peer", "both"} {
			t.Run(artisan+"/"+closing, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalSimpleCraftFixture(t, artisan, true)
				peer := criticalTinyTradePeer(t, w, p)
				before := accountStateSnapshot(p.Account)
				_, positions, roll, code := criticalSimpleCraftExpected(before, p.CharSlot, req, artisan, true)
				w.handle(command{s: p.Session, pkt: criticalSimpleCraftRequest(p.ID, req, artisan)})
				proposal := awaitCriticalWrite(t, backend.entered)
				closed := []*Player{p}
				if closing == "peer" {
					closed = []*Player{peer}
				} else if closing == "both" {
					closed = append(closed, peer)
				}
				for _, player := range closed {
					w.handle(command{s: player.Session})
				}
				if fallback.saves != 0 || len(backend.entered) != 0 || p.Trade == nil || peer.Trade == nil ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("disconnect changed a trade or saved an uncommitted composition")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				// Composition preserves the trade; only final disconnect cleanup
				// may cancel it, after the account/sidecar acknowledgement.
				if p.Trade == nil || peer.Trade == nil {
					t.Fatal("composition cancelled the retained trade before disconnect persistence")
				}
				if closing == "peer" {
					assertCriticalTinyPublication(t, p, positions, roll, code, false)
				}
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
					if participant == nil || seen[participant.Account.Name] || !reflect.DeepEqual(final.accounts[0], want) ||
						final.character == nil || final.character.uid != participant.Char.UID ||
						!reflect.DeepEqual(final.character.state, buildCharState(participant, w.now())) {
						t.Fatal("trade disconnect lost the committed participant account or sidecar")
					}
					seen[participant.Account.Name] = true
				}
				awaitReadyCriticalResults(t, queue, len(closed))
				for _, player := range closed {
					w.handle(command{s: player.Session})
					if !w.criticalPlayerReserved(player) || player.Session.QueuedPacketsForTest() != 0 ||
						w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
						t.Fatal("unacknowledged disconnect released ownership or replied to a closed session")
					}
				}
				w.processCriticalCompletions(1)
				if closing == "both" {
					if w.players[p.Session] != p || w.players[peer.Session] != peer || p.Trade == nil || peer.Trade == nil {
						t.Fatal("first disconnect acknowledgement removed an identity or trade needed by the peer write")
					}
					w.processCriticalCompletions(1)
				}
				for i, player := range closed {
					if w.players[player.Session] != nil || !w.claimAccountSession(gameNet.NewTestSession(int64(90+i), 8), player.Account.Name) {
						t.Fatal("disconnect acknowledgement did not release session ownership")
					}
				}
				if p.Trade != nil || peer.Trade != nil || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 || len(backend.entered) != 0 {
					t.Fatal("trade disconnect failed to settle or persisted inline")
				}
				for _, player := range []*Player{p, peer} {
					if !player.Session.IsClosed() {
						assertCriticalPickupPacket(t, player, wire.CloseTrade(player.ID))
					}
					if player.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("trade disconnect published extra packets")
					}
				}
			})
		}
	}
}

func TestCriticalSimpleCraftRejectsInvalidPreparedOperation(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, reason := range []string{"instance", "duplicate", "hidden", "blocked", "result", "roll", "chance", "outcome", "recipe", "unclaimed", "stopping", "full", "missing-atomic", "reserved-peer"} {
			t.Run(artisan+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, req, rng, release := criticalSimpleCraftFixture(t, artisan, true)
				peer := criticalTinyTradePeer(t, w, p)
				want, _, roll, _ := criticalSimpleCraftExpected(p.Account, p.CharSlot, req, artisan, true)
				result := want.Chars[p.CharSlot].Inv[int(req.Pos[0])]
				switch reason {
				case "instance":
					req.Items[0].UID = "forged-instance"
				case "duplicate":
					req.Pos[1] = req.Pos[0]
				case "hidden":
					req.Pos[0] = model.PlayerCarrySlots
				case "blocked":
					req.Items[0].Index = blockedCombineItem
					p.Char.Inv[int(req.Pos[0])] = req.Items[0]
				case "result":
					result.UID = "forged-result"
				case "roll":
					roll.Roll = 0
				case "chance":
					roll.Chance++
				case "outcome":
					roll.Success = false
				case "recipe":
					req.Items[6].Index = 413
					p.Char.Inv[int(req.Pos[6])] = req.Items[6]
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
				case "missing-atomic":
					queue.store = backend
				case "reserved-peer":
					if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{peer.Account}}, []*Player{peer},
						func(criticalPersistenceResult) error { return nil }); err != nil {
						t.Fatal(err)
					}
					awaitCriticalWrite(t, backend.entered)
				}
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				if err := w.queueInventoryComposition(p, req, result, roll, artisan); err == nil {
					t.Fatal("invalid prepared operation was admitted")
				}
				if reason == "reserved-peer" {
					for _, phase := range []string{"held", "completed-unacknowledged"} {
						if phase == "completed-unacknowledged" {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						callCriticalSimpleCraft(w, p, criticalSimpleCraftRequest(p.ID, req, artisan), artisan)
						if !p.LastCraft.IsZero() || p.CraftNPC != 0 || rng.calls != 0 {
							t.Fatal("reserved trade peer allowed context mutation or a roll")
						}
					}
					w.processCriticalCompletions(1)
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
					fallback.saves != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejected operation mutated, saved inline or published a result")
				}
			})
		}
	}
}

func TestCriticalSimpleCraftUnknownOutcomeRetainsOwnership(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, failure := range []string{"error", "panic", "identity", "peer-state"} {
			t.Run(artisan+"/"+failure, func(t *testing.T) {
				w, p, fallback, backend, queue, req, rng, release := criticalSimpleCraftFixture(t, artisan, true)
				peer := criticalTinyTradePeer(t, w, p)
				backend.failure = errors.New("composition outcome unknown")
				if failure != "error" {
					backend.failure = nil
				}
				backend.panicAfterWrite = failure == "panic"
				w.handle(command{s: p.Session, pkt: criticalSimpleCraftRequest(p.ID, req, artisan)})
				awaitCriticalWrite(t, backend.entered)
				if failure == "identity" {
					p.Char.UID = "replacement-character"
				}
				if failure == "peer-state" {
					peer.Account.Cargo[0].UID = "conflicting-peer-state"
				}
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				w.processCriticalCompletions(8)
				if !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned || !p.Session.IsClosed() || !peer.Session.IsClosed() ||
					!w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || len(queue.pending) != 1 ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
					p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || rng.calls != 1 {
					t.Fatal("unknown composition applied, published, replayed or released ownership")
				}
				done := make(chan error, 1)
				w.runShutdown(done)
				select {
				case err := <-done:
					if err == nil || fallback.saves != 0 || len(backend.entered) != 0 {
						t.Fatal("unknown composition was overwritten by a final snapshot")
					}
				default:
					t.Fatal("unknown composition shutdown did not report its terminal failure")
				}
			})
		}
	}
}

func TestCriticalSimpleCraftHandlerAdmissionDoesNotFallBack(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, reason := range []string{"short", "wire-item", "duplicate", "hidden", "negative", "blocked", "NPC", "outside", "closed", "character", "unclaimed", "stopping", "full", "missing-atomic", "invalid-peer", "gold"} {
			if reason == "gold" && artisan != "Aylin" {
				continue
			}
			t.Run(artisan+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, _ := criticalSimpleCraftFixture(t, artisan, true)
				request := criticalSimpleCraftRequest(p.ID, req, artisan)
				switch reason {
				case "short":
					request = request[:83]
				case "wire-item":
					request[12]++
				case "duplicate":
					request[77] = request[76]
				case "hidden":
					request[76] = model.PlayerCarrySlots
				case "negative":
					request[76] = 255
				case "blocked":
					req.Items[7].Index = blockedCombineItem
					p.Char.Inv[int(req.Pos[7])] = req.Items[7]
					request = criticalSimpleCraftRequest(p.ID, req, artisan)
				case "NPC":
					w.mobsByID[1500].X += npcInteractionRange + 2
				case "outside":
					p.InWorld = false
				case "closed":
					p.Session.Close()
				case "character":
					copy := cloneCharacterState(p.Char)
					p.Char = &copy
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
				case "missing-atomic":
					criticalTinyTradePeer(t, w, p)
					queue.store = backend
				case "invalid-peer":
					p.Trade = &TradeState{OpponentID: 999}
				case "gold":
					p.Char.Gold = aylinGoldCost - 1
				}
				before, live := accountStateSnapshot(p.Account), cloneCharacterState(p.Char)
				callCriticalSimpleCraft(w, p, request, artisan)
				if fallback.saves != 0 || len(backend.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					!reflect.DeepEqual(cloneCharacterState(p.Char), live) || reason != "full" && len(queue.pending) != 0 {
					t.Fatal("rejected handler admission changed economic state or used inline persistence")
				}
				// Rejection may send a refusal, but never successful/failed recipe
				// results or inventory snapshots belonging to an uncommitted roll.
				for p.Session.QueuedPacketsForTest() != 0 {
					packet, ok := p.Session.DequeuePacketForTest()
					if !ok || !wire.Decrypt(packet) {
						t.Fatal("queued refusal disappeared or was invalid")
					}
					switch wire.ParseHeader(packet).Type {
					case wire.OpMessagePanel:
					case wire.OpCombineComplete:
						if binary.LittleEndian.Uint32(packet[12:16]) != 0 {
							t.Fatal("rejected admission published a rolled recipe result")
						}
					default:
						t.Fatal("rejected admission published economic or trade changes")
					}
				}
			})
		}
	}
}

func TestCriticalSimpleCraftDisconnectPersistsCommittedResult(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, success := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/%t", artisan, success), func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalSimpleCraftFixture(t, artisan, success)
				before := accountStateSnapshot(p.Account)
				w.handle(command{s: p.Session, pkt: criticalSimpleCraftRequest(p.ID, req, artisan)})
				proposal := awaitCriticalWrite(t, backend.entered)
				w.handle(command{s: p.Session})
				if !p.Session.IsClosed() || w.players[p.Session] != p || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					fallback.saves != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("disconnect exposed, overwrote or released pending composition")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				final := awaitCriticalWrite(t, backend.entered)
				if !reflect.DeepEqual(final.accounts[0], proposal.accounts[0]) || final.character == nil ||
					final.character.uid != p.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
					t.Fatal("final disconnect write lost the committed composition or character sidecar")
				}
				awaitReadyCriticalResults(t, queue, 1)
				if w.players[p.Session] != p || !w.criticalPlayerReserved(p) {
					t.Fatal("disconnect released ownership before final save acknowledgement")
				}
				w.processCriticalCompletions(1)
				if w.players[p.Session] != nil || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("disconnect failed to settle composition and cleanup without inline save or reply")
				}
			})
		}
	}
}

func TestCriticalSimpleCraftShutdownDrainsBeforeFinalSnapshot(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, success := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/%t", artisan, success), func(t *testing.T) {
				w, p, fallback, backend, queue, req, rng, release := criticalSimpleCraftFixture(t, artisan, success)
				terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
				w.store = terminal
				before := accountStateSnapshot(p.Account)
				w.handle(command{s: p.Session, pkt: criticalSimpleCraftRequest(p.ID, req, artisan)})
				proposal := awaitCriticalWrite(t, backend.entered)
				first, second := make(chan error, 1), make(chan error, 1)
				w.runShutdown(first)
				w.runShutdown(second)
				w.handle(command{s: p.Session})
				if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished ||
					!p.Session.IsClosed() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("shutdown persisted uncommitted composition or discarded pending work")
				}
				for _, done := range []chan error{first, second} {
					select {
					case <-done:
						t.Fatal("shutdown finished before composition acknowledgement")
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
						t.Fatal("shutdown did not finish after composition acknowledgement")
					}
				}
				if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) ||
					fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 || rng.calls != 1 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("shutdown lost committed composition, repeated saves or replied to a closed session")
				}
				third := make(chan error, 1)
				w.runShutdown(third)
				if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
					t.Fatal("repeated shutdown replayed the composition snapshot")
				}
			})
		}
	}
}

func TestCriticalSimpleCraftActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, artisan := range criticalSimpleArtisans {
		for _, outcome := range []string{"success", "failure", "unknown"} {
			t.Run(artisan+"/"+outcome, func(t *testing.T) {
				var failure error
				if outcome == "unknown" {
					failure = errors.New("composition acknowledgement lost")
				}
				var req combineRequest
				var rng *criticalTinyRNG
				var before, want *model.Account
				var positions []int
				var roll percentRoll
				var code uint32
				w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, p *Player) {
					req, rng = configureCriticalSimpleCraft(w, p, artisan, outcome != "failure")
					before = accountStateSnapshot(p.Account)
					want, positions, roll, code = criticalSimpleCraftExpected(before, p.CharSlot, req, artisan, outcome != "failure")
				})
				request := criticalSimpleCraftRequest(p.ID, req, artisan)
				deliver := func() {
					select {
					case w.commands <- command{s: p.Session, pkt: request}:
					case <-time.After(time.Second):
						t.Fatal("World did not accept composition intention")
					}
				}
				deliver()
				proposal := awaitCriticalWrite(t, backend.entered)
				if !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
					t.Fatal("actual dispatcher submitted the wrong composition delta")
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
						t.Fatal("held composition blocked unrelated ticks/commands or published early")
					}
				}
				release()
				deadline := time.Now().Add(2 * time.Second)
				for {
					overview := readCriticalLoopOverview(t, w)
					if outcome == "unknown" && overview.State == "maintenance" || outcome != "unknown" && p.Session.QueuedPacketsForTest() >= len(positions)+3 {
						break
					}
					if time.Now().After(deadline) {
						t.Fatal("World did not deliver composition outcome")
					}
				}
				stop() // Join the actor before reading mutable state and RNG.
				if w.mobTickCounter < 3 || len(backend.entered) != 0 || rng.calls != 1 {
					t.Fatal("composition blocked ticks or repeated persistence/randomness")
				}
				if outcome == "unknown" {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 1 || !p.PersistencePoisoned ||
						!p.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
						t.Fatal("unknown actor result changed state, published or released ownership")
					}
				} else {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || len(queue.pending) != 0 || w.shuttingDown {
						t.Fatal("actual World failed to settle composition")
					}
					assertCriticalTinyPublication(t, p, positions, roll, code, false)
				}
			})
		}
	}
}
