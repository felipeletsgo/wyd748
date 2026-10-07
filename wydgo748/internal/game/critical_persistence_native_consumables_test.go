package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

type criticalNativeCase struct {
	name string
	rule model.VolatileRule
	code int
}

func criticalNativeCases() []criticalNativeCase {
	return []criticalNativeCase{
		{"equipped-refine", model.VolatileRule{Action: "refine_equipped", Consume: true, TargetSlot: 13, TargetItems: []uint16{200}, RefineMax: 6}, 1},
		{"nightmare", model.VolatileRule{Action: "nightmare_ticket", Consume: true, Amount: 13, CooldownSeconds: 43200}, 2},
		{"opportunity", model.VolatileRule{Action: "mastery_reset", Consume: true, Amount: 50, MinLevel: 40, MaxLevelExclusive: 100}, 192},
		{"balance", model.VolatileRule{Action: "mastery_reset", Consume: true, Amount: 100, MinLevel: 100, MaxLevelExclusive: 200}, 193},
	}
}

func configureCriticalNative(w *World, p *Player, tc criticalNativeCase) {
	p.Char.Inv[62] = model.Item{Index: 100, UID: "native-consumable", Eff: [6]byte{effectAmount, 2}, ActivatedUnix: 12, ExpiresUnix: 4102444800}
	p.Char.Equip[13] = model.Item{Index: 200, UID: "equipped-target", Eff: [6]byte{43, 5}, ActivatedUnix: 20, ExpiresUnix: 4102444800}
	p.Char.Evolution = "mortal"
	p.Char.Score.Level = tc.rule.MinLevel
	p.Char.Score.Mastery = [4]uint32{11, 120, 30, 9}
	p.Char.LearnedSkill = 0x01000003
	for i := range p.Char.ShortSkill {
		p.Char.ShortSkill[i] = 0xFF
	}
	p.Char.ShortSkill[0], p.Char.ShortSkill[1], p.Char.ShortSkill[2] = 0, 1, 24
	p.Char.NightmareTickets = 7
	p.Char.LastNightmareUnix = 0
	p.Char.QuestsDone = []int32{12345}
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	w.items[100], w.items[200] = model.ItemDef{Index: 100}, model.ItemDef{Index: 200, Pos: 1}
	w.volatiles = model.VolatileCatalog{Items: map[uint16]model.VolatileRule{100: tc.rule}, ItemCodes: map[uint16]int{100: tc.code}}
	w.recalcPlayer(p.Char)
}

func criticalNativeRequest(p *Player) []byte {
	packet := useItemPacket(62, 0)
	copy(packet[:12], inboundPacket(wire.OpUseItem, 36)[:12])
	binary.LittleEndian.PutUint16(packet[6:8], p.ID)
	return packet
}

// Expected changes are independent of prepareNativeConsumable.
func criticalNativeExpected(w *World, before *model.Account, slot int, tc criticalNativeCase) *model.Account {
	want := accountStateSnapshot(before)
	ch := &want.Chars[slot]
	if tc.rule.Consume {
		ch.Inv[62].Eff[1] = 1
	}
	switch tc.rule.Action {
	case "refine_equipped":
		ch.Equip[13].Eff[1] = 6
	case "nightmare_ticket":
		ch.NightmareTickets = 20
		ch.LastNightmareUnix = w.now().Unix()
	case "mastery_reset":
		ch.Score.Mastery[1] = 120 - uint32(tc.rule.Amount)
		ch.Score.Mastery[2] = 0
		ch.LearnedSkill = 0x01000000
		ch.ShortSkill[0], ch.ShortSkill[1], ch.ShortSkill[2] = 0xFF, 0xFF, 0xFF
		ch.QuestsDone = append(ch.QuestsDone, int32(-tc.code))
	}
	if tc.rule.Action != "nightmare_ticket" {
		w.recalcPlayer(ch)
	}
	return want
}

func assertCriticalNativePublication(t *testing.T, p *Player, tc criticalNativeCase) {
	t.Helper()
	ch := p.Char
	switch tc.rule.Action {
	case "refine_equipped":
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeEquip, 13, ch.Equip[13]))
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 62, ch.Inv[62]))
		assertCriticalPickupPacket(t, p, playerScorePacket(p))
		// Appearance is published through the existing observer projection.
		assertCriticalPickupPacket(t, p, playerAppearancePacket(p))
		assertCriticalPickupPacket(t, p, wire.MessagePanel("Refine successful!"))
	case "nightmare_ticket":
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 62, ch.Inv[62]))
		assertCriticalPickupPacket(t, p, wire.MessagePanel("13 Nightmare entries received."))
	case "mastery_reset":
		assertCriticalPickupPacket(t, p, wire.UpdateScore(p.ID, *ch))
		assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *ch))
		assertCriticalPickupPacket(t, p, wire.SetShortSkill(p.ID, ch.ShortSkill))
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 62, ch.Inv[62]))
		assertCriticalPickupPacket(t, p, wire.MessagePanel("Skills initialized."))
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("unexpected consumable response")
	}
}

func TestCriticalNativeConsumableDetachedCommit(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		for slot := range 4 {
			for _, consume := range []bool{false, true} {
				t.Run(fmt.Sprintf("%s/slot-%d/consume-%t", tc.name, slot, consume), func(t *testing.T) {
					tc := tc
					w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
					if slot != p.CharSlot {
						p.Account.Chars[slot] = cloneCharacterState(p.Char)
						p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
					}
					tc.rule.Consume = consume
					configureCriticalNative(w, p, tc)
					peer := criticalTinyTradePeer(t, w, p)
					before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
					account, character, score := p.Account, p.Char, p.Char.Score
					trade, peerTrade := p.Trade, peer.Trade
					want := criticalNativeExpected(w, before, slot, tc)
					request := criticalNativeRequest(p)
					w.handle(command{s: p.Session, pkt: request})
					proposal := awaitCriticalWrite(t, backend.entered)
					if len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) || !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peerBefore)) {
						if len(proposal.accounts) == 2 {
							got, expected := reflect.ValueOf(proposal.accounts[0].Chars[slot]), reflect.ValueOf(accountPersistenceSnapshot(want).Chars[slot])
							for i := range got.NumField() {
								if !reflect.DeepEqual(got.Field(i).Interface(), expected.Field(i).Interface()) {
									t.Errorf("proposal %s: got %+v, want %+v", got.Type().Field(i).Name, got.Field(i).Interface(), expected.Field(i).Interface())
								}
							}
						}
						t.Fatal("incorrect detached atomic proposal")
					}
					for _, phase := range []string{"held", "completed-unacknowledged"} {
						if phase == "completed-unacknowledged" {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						w.handle(command{s: p.Session, pkt: request})
						w.onUseItem(p.Session, request)
						w.cancelTrade(peer, "competing cancellation")
						w.tickTrades(w.now().Add(time.Hour))
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || p.Trade != trade || peer.Trade != peerTrade || !w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || len(backend.entered) != 0 {
							t.Fatalf("%s changed retained state or repeated persistence", phase)
						}
					}
					discardCriticalDeferredDuplicates(w)

					w.processCriticalCompletions(1)
					if p.Account != account || p.Char != character || p.Char.Score != score || !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || p.Trade != nil || peer.Trade != nil || len(queue.pending) != 0 || w.shuttingDown {
						t.Fatal("acknowledgement lost state, pointer identity or trade cancellation")
					}
					assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
					assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
					assertCriticalNativePublication(t, p, tc)
				})
			}
		}
	}
}

func TestCriticalNativeConsumableRejection(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		for _, reason := range []string{"domain", "invalid-amount", "stopping", "missing-batch", "invalid-peer", "detached-character"} {
			t.Run(tc.name+"/"+reason, func(t *testing.T) {
				tc := tc
				w, p, fallback, backend, queue, _, _, _ := criticalTinyFixture(t, true, true)
				configureCriticalNative(w, p, tc)
				peer := criticalTinyTradePeer(t, w, p)
				switch reason {
				case "domain":
					if tc.rule.Action == "refine_equipped" {
						p.Char.Equip[13].Eff[1] = 6
					} else if tc.rule.Action == "nightmare_ticket" {
						p.Char.LastNightmareUnix = w.now().Unix()
					} else {
						p.Char.QuestsDone = append(p.Char.QuestsDone, int32(-tc.code))
					}
				case "invalid-amount":
					if tc.rule.Action == "refine_equipped" {
						tc.rule.TargetSlot = -1
					} else if tc.rule.Action == "nightmare_ticket" {
						p.Char.NightmareTickets = math.MaxUint32
					} else {
						tc.rule.Amount = -1
					}
					w.volatiles.Items[100] = tc.rule
				case "stopping":
					queue.closeAdmission()
				case "missing-batch":
					queue.store = backend
				case "invalid-peer":
					peer.Trade.OpponentID++
				case "detached-character":
					ch := cloneCharacterState(p.Char)
					p.Char = &ch
				}
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				trade, peerTrade := p.Trade, peer.Trade
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || p.Trade != trade || peer.Trade != peerTrade || fallback.saves != 0 || len(backend.entered) != 0 || len(queue.pending) != 0 || w.shuttingDown {
					t.Fatal("rejection mutated state, cancelled trade or fell back to inline saves")
				}
				assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 62, p.Char.Inv[62]))
			})
		}
	}
}

func TestCriticalNativeConsumableUnknownAndStale(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		for _, mode := range []string{"unknown", "panic", "stale-owner", "stale-peer"} {
			t.Run(tc.name+"/"+mode, func(t *testing.T) {
				w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
				configureCriticalNative(w, p, tc)
				peer := criticalTinyTradePeer(t, w, p)
				if mode == "unknown" {
					backend.failure = errors.New("commit acknowledgement lost")
				}
				backend.panicAfterWrite = mode == "panic"
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				awaitCriticalWrite(t, backend.entered)
				if mode == "stale-owner" {
					p.Account.CargoGold++
				} else if mode == "stale-peer" {
					peer.Char.Inv[0].Index++
				}
				before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || !p.PersistencePoisoned || !peer.PersistencePoisoned || !w.shuttingDown || len(queue.pending) != 1 || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("unknown or stale outcome applied unverified state or released ownership")
				}
			})
		}
	}
}

func TestCriticalNativeConsumableDisconnectAndShutdown(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		for _, shutdown := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/shutdown-%t", tc.name, shutdown), func(t *testing.T) {
				w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
				configureCriticalNative(w, p, tc)
				terminal := &criticalLindyTerminalStore{craftStore: fallback}
				if shutdown {
					w.store = terminal
				}
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				proposal := awaitCriticalWrite(t, backend.entered)
				waiters := []chan error{make(chan error, 1), make(chan error, 1)}
				if shutdown {
					for _, done := range waiters {
						w.runShutdown(done)
						select {
						case <-done:
							t.Fatal("shutdown completed before durable acknowledgement")
						default:
						}
					}
				}
				p.Session.Close()
				w.handle(command{s: p.Session})
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if shutdown {
					for _, done := range waiters {
						select {
						case err := <-done:
							if err != nil {
								t.Fatal(err)
							}
						default:
							t.Fatal("shutdown failed to finish after acknowledgement")
						}
					}
					if len(terminal.final) != 1 || !reflect.DeepEqual(terminal.final[0].accounts, proposal.accounts) || terminal.flushes != 1 {
						t.Fatal("shutdown did not retain committed consumable state")
					}
				} else {
					final := awaitCriticalWrite(t, backend.entered)
					if !reflect.DeepEqual(final.accounts, proposal.accounts) || final.character == nil || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
						t.Fatal("disconnect did not retain committed account/sidecar state")
					}
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
					if w.players[p.Session] != nil {
						t.Fatal("disconnect did not release acknowledged identity")
					}
				}
				if fallback.saves != 0 || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("lifecycle repeated persistence or replied to a closed session")
				}
			})
		}
	}
}

func TestCriticalNativeConsumableTradeDisconnect(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		for _, closing := range []string{"owner", "peer", "both"} {
			t.Run(tc.name+"/"+closing, func(t *testing.T) {
				w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
				configureCriticalNative(w, p, tc)
				peer := criticalTinyTradePeer(t, w, p)
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				proposal := awaitCriticalWrite(t, backend.entered)
				closed := []*Player{p}
				if closing == "peer" {
					closed = []*Player{peer}
				} else if closing == "both" {
					closed = append(closed, peer)
				}
				for _, player := range closed {
					w.handle(command{s: player.Session})
					w.handle(command{s: player.Session}) // Repeated transport cleanup.
					if !w.criticalPlayerReserved(player) || w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
						t.Fatal("disconnect released accepted consumable ownership")
					}
				}
				if p.Trade == nil || peer.Trade == nil || len(backend.entered) != 0 || fallback.saves != 0 ||
					p.Char.Inv[62].Eff[1] != 2 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("disconnect mutated, cancelled, published or persisted before commit")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if p.Trade != nil || peer.Trade != nil || p.Char.Inv[62].Eff[1] != 1 || w.shuttingDown {
					t.Fatal("acknowledgement lost consumable state or trade cancellation")
				}
				for _, player := range []*Player{p, peer} {
					if !player.Session.IsClosed() {
						assertCriticalPickupPacket(t, player, wire.CloseTrade(player.ID))
					}
				}
				if closing == "peer" {
					assertCriticalNativePublication(t, p, tc)
				}
				seen := make(map[string]bool)
				for range closed {
					final := awaitCriticalWrite(t, backend.entered)
					var participant *Player
					var expected *model.Account
					for i, player := range []*Player{p, peer} {
						if final.accounts[0].Name == player.Account.Name {
							participant, expected = player, proposal.accounts[i]
						}
					}
					if participant == nil || seen[participant.Account.Name] || !reflect.DeepEqual(final.accounts[0], expected) ||
						final.character == nil || final.character.uid != participant.Char.UID ||
						!reflect.DeepEqual(final.character.state, buildCharState(participant, w.now())) {
						t.Fatal("final save lost or repeated committed account/sidecar state")
					}
					seen[participant.Account.Name] = true
				}
				awaitReadyCriticalResults(t, queue, len(closed))
				for _, player := range closed {
					if !w.criticalPlayerReserved(player) || w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
						t.Fatal("completed final save released ownership before acknowledgement")
					}
				}
				w.processCriticalCompletions(len(closed))
				for _, player := range closed {
					if w.players[player.Session] != nil || !w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
						t.Fatal("final acknowledgement failed to release disconnected ownership")
					}
				}
				if len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("trade disconnect repeated persistence or replied to a closed session")
				}
			})
		}
	}
}

func TestCriticalNativeConsumableActualWorldProgress(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		t.Run(tc.name, func(t *testing.T) {
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil, func(w *World, p *Player) {
				configureCriticalNative(w, p, tc)
			})
			request := criticalNativeRequest(p)
			select {
			case w.commands <- command{s: p.Session, pkt: request}:
			case <-time.After(time.Second):
				t.Fatal("World did not accept consumable intention")
			}
			awaitCriticalWrite(t, backend.entered)
			for range 3 {
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held consumable write blocked unrelated World progress")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for p.Session.QueuedPacketsForTest() == 0 && time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
			}
			stop()
			if len(queue.pending) != 0 || w.shuttingDown || w.mobTickCounter < 3 {
				t.Fatal("actual dispatcher failed to settle consumable once")
			}
			assertCriticalNativePublication(t, p, tc)
		})
	}
}
