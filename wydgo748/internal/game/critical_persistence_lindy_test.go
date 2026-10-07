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

// Deliberately separate from the single-account fake: tests must explicitly
// opt into the new atomic capability rather than silently gaining a fallback.
type criticalPlayerBatchTestStore struct{ *criticalStateTestStore }

func (s *criticalPlayerBatchTestStore) SavePlayerStateBatch(_ *model.GuildRegistry,
	uid string, state *model.CharState, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, character: &criticalCharacterState{uid: uid, state: state}})
}

func configureCriticalLindy(w *World, p *Player, level uint32) (combineRequest, model.Item) {
	req, _ := configureCriticalTiny(w, p, true, true)
	w.mobsByID[1500].Def.Name = "Lindy"
	for i := range req.Items {
		index := uint16(413)
		if i < 2 {
			index = 3448
		} else if i == 2 {
			index = 4127
		} else if i == 7 {
			index = 4200 // The optional eighth slot is validated, not consumed.
		}
		req.Items[i] = model.Item{Index: index, UID: fmt.Sprintf("lindy-ingredient-%d", i)}
		if i < 2 {
			setItemAmount(&req.Items[i], 10)
		}
		p.Char.Inv[int(req.Pos[i])] = req.Items[i]
	}
	p.Char.Evolution, p.Char.ArchLevel355, p.Char.ArchLevel370 = archEvolution, level == archLockLevel370, false
	p.Char.Score = testScore(model.Score{Level: level, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100})
	p.Char.Equip[15] = model.Item{Index: 3197, UID: "11111111111141118111111111111111", Eff: [6]byte{43, 7}}
	p.SpecialCoins = map[string]uint32{fameCounter: 2, "unrelated": 9}
	if level == archLockLevel370 {
		return req, model.Item{}
	}
	index := lindyCapeIndex(p.Char)
	w.items[index] = model.ItemDef{Index: index}
	return req, model.Item{Index: index, UID: p.Char.Equip[15].UID, Eff: [6]byte{54, 16}}
}

func criticalLindyFixture(t *testing.T, level uint32) (*World, *Player, *craftStore, *criticalTestStore,
	*criticalPersistence, combineRequest, model.Item, func()) {
	t.Helper()
	w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
	req, cape := configureCriticalLindy(w, p, level)
	queue.store = &criticalPlayerBatchTestStore{&criticalStateTestStore{&criticalBatchTestStore{backend}}}
	return w, p, fallback, backend, queue, req, cape, release
}

func criticalLindyRequest(p *Player, req combineRequest) []byte {
	packet := criticalTinyRequest(p.ID, req)
	binary.LittleEndian.PutUint16(packet[4:6], wire.OpCombineLindy)
	return packet
}

func criticalLindyExpected(w *World, p *Player, req combineRequest, cape model.Item) (*model.Account, map[string]uint32, []int) {
	want, coins := accountStateSnapshot(p.Account), copyCounters(p)
	ch := &want.Chars[p.CharSlot]
	positions := make([]int, 0, 7)
	for i := 0; i <= 6; i++ {
		pos := int(req.Pos[i])
		ch.Inv[pos] = model.Item{}
		positions = append(positions, pos)
	}
	if ch.Score.Level == archLockLevel355 {
		ch.ArchLevel355, ch.Equip[15] = true, cape
		w.recalcPlayer(ch)
	} else {
		ch.ArchLevel370 = true
		coins[fameCounter]--
		if coins[fameCounter] == 0 {
			delete(coins, fameCounter)
		}
	}
	slices.Sort(positions)
	return want, coins, positions
}

func assertCriticalLindyPublication(t *testing.T, p *Player, positions []int, level uint32) {
	t.Helper()
	for _, pos := range positions {
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
	}
	if level == archLockLevel355 {
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeEquip, 15, p.Char.Equip[15]))
	}
	assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
	if level == archLockLevel355 {
		assertCriticalPickupPacket(t, p, observedPlayerScorePacket(p, p))
	}
	assertCriticalPickupPacket(t, p, wire.MessagePanel("Processing complete."))
	assertCriticalPickupPacket(t, p, wire.StandardParm(wire.OpCombineComplete, wire.SceneField, 1))
	assertCriticalPickupPacket(t, p, wire.MessagePanel("Your level limit has been lifted."))
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("Lindy published extra responses")
	}
}

func TestCriticalLindyDetachedCommitReplayAndTrade(t *testing.T) {
	for _, level := range []uint32{archLockLevel355, archLockLevel370} {
		for _, trading := range []bool{false, true} {
			for slot := range 4 {
				t.Run(fmt.Sprintf("level-%d/trade-%t/slot-%d", level+1, trading, slot), func(t *testing.T) {
					w, p, fallback, backend, queue, req, cape, release := criticalLindyFixture(t, level)
					if slot != 0 {
						p.Account.Chars[0], p.Account.Chars[slot] = p.Account.Chars[slot], p.Account.Chars[0]
						p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
					}
					var peer *Player
					if trading {
						peer = criticalTinyTradePeer(t, w, p)
					}
					before, beforeCoins := accountStateSnapshot(p.Account), copyCounters(p)
					account, character, trade := p.Account, p.Char, p.Trade
					want, coins, positions := criticalLindyExpected(w, p, req, cape)
					request := criticalLindyRequest(p, req)
					w.handle(command{s: p.Session, pkt: request})
					proposal := awaitCriticalWrite(t, backend.entered)
					if proposal.accounts[0] == account || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
						t.Fatal("Lindy lost detached account, ingredient, cape or flag state")
					}
					if level == archLockLevel370 {
						if proposal.character == nil || proposal.character.uid != p.Char.UID ||
							!reflect.DeepEqual(proposal.character.state, buildCharState(&Player{Char: character, SpecialCoins: coins}, w.now())) {
							t.Fatal("level-370 transaction omitted or corrupted the Fame sidecar")
						}
					} else if proposal.character != nil {
						t.Fatal("level-355 transaction changed the sidecar")
					}
					if trading && (len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peer.Account))) {
						t.Fatal("trade participant was not included in the atomic transaction")
					}
					lastCraft := p.LastCraft
					for _, completed := range []bool{false, true} {
						if completed {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						w.handle(command{s: p.Session, pkt: request})
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(p.SpecialCoins, beforeCoins) ||
							p.LastCraft != lastCraft || p.Trade != trade || !w.criticalPlayerReserved(p) ||
							(trading && !w.criticalPlayerReserved(peer)) || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
							t.Fatal("held/unacknowledged Lindy request changed state, replayed or published")
						}
					}
					discardCriticalDeferredDuplicates(w)

					w.processCriticalCompletions(1)
					if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) ||
						!reflect.DeepEqual(p.SpecialCoins, coins) || p.Trade != trade || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 {
						t.Fatal("Lindy failed to settle one durable operation without replacing identity or closing trade")
					}
					assertCriticalLindyPublication(t, p, positions, level)
					w.handle(command{s: p.Session, pkt: request})
					if len(backend.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(account), want) || !reflect.DeepEqual(p.SpecialCoins, coins) {
						t.Fatal("settled request consumed the recipe twice")
					}
				})
			}
		}
	}
}

func TestCriticalLindyRejectsWithoutFallback(t *testing.T) {
	for _, level := range []uint32{archLockLevel355, archLockLevel370} {
		for _, reason := range []string{"identity", "ingredient", "duplicate", "hidden", "package", "powder", "limit", "cape", "missing-atomic", "stopping", "peer"} {
			t.Run(fmt.Sprintf("%d/%s", level+1, reason), func(t *testing.T) {
				w, p, fallback, backend, queue, req, cape, _ := criticalLindyFixture(t, level)
				criticalTinyTradePeer(t, w, p)
				switch reason {
				case "identity":
					delete(w.accountSessions, accountSessionKey(p.Account.Name))
				case "ingredient":
					req.Items[0].UID = "forged"
				case "duplicate":
					req.Pos[1] = req.Pos[0]
				case "hidden":
					req.Pos[0] = model.PlayerCarrySlots
				case "package":
					setItemAmount(&req.Items[0], 9)
					p.Char.Inv[int(req.Pos[0])] = req.Items[0]
				case "powder":
					setItemAmount(&req.Items[3], 2)
					p.Char.Inv[int(req.Pos[3])] = req.Items[3]
				case "limit":
					p.Char.Score.Level++
				case "cape":
					cape.UID = "forged"
				case "missing-atomic":
					if level == archLockLevel355 {
						queue.store = backend
					} else {
						queue.store = &criticalStateTestStore{&criticalBatchTestStore{backend}}
					}
				case "stopping":
					queue.closeAdmission()
				case "peer":
					p.Trade.OpponentID = 9999
				}
				before, coins := accountStateSnapshot(p.Account), copyCounters(p)
				if err := w.queueLindyCraft(p, req, cape); err == nil {
					t.Fatal("invalid Lindy operation was admitted")
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(p.SpecialCoins, coins) ||
					fallback.saves != 0 || len(backend.entered) != 0 || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejected Lindy operation mutated, persisted or published")
				}
				if reason == "missing-atomic" {
					w.onCombineLindy(p.Session, criticalLindyRequest(p, req))
					if fallback.saves != 0 || len(backend.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
						t.Fatal("handler fell back to sequential persistence")
					}
				}
			})
		}
	}
}

func TestCriticalLindyUnknownCommitRetainsOwnership(t *testing.T) {
	for _, level := range []uint32{archLockLevel355, archLockLevel370} {
		for _, outcome := range []string{"error", "panic", "stale-currency", "stale-cape"} {
			t.Run(fmt.Sprintf("%d/%s", level+1, outcome), func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalLindyFixture(t, level)
				peer := criticalTinyTradePeer(t, w, p)
				if outcome == "error" {
					backend.failure = errors.New("commit acknowledgement lost")
				} else if outcome == "panic" {
					backend.panicAfterWrite = true
				}
				w.handle(command{s: p.Session, pkt: criticalLindyRequest(p, req)})
				awaitCriticalWrite(t, backend.entered)
				if outcome == "stale-currency" {
					p.SpecialCoins[fameCounter]++
				} else if outcome == "stale-cape" {
					p.Char.Equip[15].Eff[1]++
				}
				before, coins := accountStateSnapshot(p.Account), copyCounters(p)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned || len(queue.pending) != 1 ||
					!w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					!reflect.DeepEqual(p.SpecialCoins, coins) || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 {
					t.Fatal("unknown or stale completion applied, published or released a participant")
				}
				w.runShutdown(make(chan error, 1))
				if fallback.saves != 0 || len(backend.entered) != 0 {
					t.Fatal("shutdown overwrote an uncertain commit")
				}
			})
		}
	}
}

func TestCriticalLindyDisconnectPersistsCommittedState(t *testing.T) {
	for _, level := range []uint32{archLockLevel355, archLockLevel370} {
		t.Run(fmt.Sprint(level+1), func(t *testing.T) {
			w, p, fallback, backend, queue, req, _, release := criticalLindyFixture(t, level)
			w.handle(command{s: p.Session, pkt: criticalLindyRequest(p, req)})
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if w.players[p.Session] != p || len(backend.entered) != 0 || fallback.saves != 0 {
				t.Fatal("disconnect removed ownership or saved a precommit snapshot")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(final.accounts[0], proposal.accounts[0]) || final.character == nil ||
				!reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
				t.Fatal("disconnect lost the committed unlock, cape or Fame")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("disconnect failed to acknowledge and release committed Lindy state")
			}
		})
	}
}

func TestCriticalLindyActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, failure := range []bool{false, true} {
		t.Run(fmt.Sprint(failure), func(t *testing.T) {
			var err error
			if failure {
				err = errors.New("commit acknowledgement lost")
			}
			var req combineRequest
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, err, func(w *World, p *Player) {
				req, _ = configureCriticalLindy(w, p, archLockLevel370)
				w.criticalWrites.queue.store = &criticalPlayerBatchTestStore{&criticalStateTestStore{&criticalBatchTestStore{w.store.(*criticalTestStore)}}}
			})
			request := criticalLindyRequest(p, req)
			for range 3 {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept Lindy intention")
				}
				if len(queue.results) == 0 {
					deliverCriticalLoopTick(t, ticks)
				}
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held Lindy operation blocked unrelated commands or published early")
				}
			}
			awaitCriticalWrite(t, backend.entered)
			release()
			deadline := time.Now().Add(2 * time.Second)
			for {
				overview := readCriticalLoopOverview(t, w)
				if failure && overview.State == "maintenance" || !failure && p.Session.QueuedPacketsForTest() >= 11 {
					break
				}
				if time.Now().After(deadline) {
					t.Fatal("World did not deliver Lindy completion")
				}
			}
			stop()
			if len(backend.entered) != 0 || w.mobTickCounter < 3 || failure && len(queue.pending) != 1 ||
				!failure && (!p.Char.ArchLevel370 || counterBalance(p, fameCounter) != 1 || len(queue.pending) != 0) {
				t.Fatal("World replayed, blocked or failed to settle Lindy")
			}
		})
	}
}

func TestCriticalLindyTradeDisconnectRetainsCommittedAccountAndFame(t *testing.T) {
	for _, closing := range []string{"owner", "peer", "both"} {
		t.Run(closing, func(t *testing.T) {
			w, p, fallback, backend, queue, req, cape, release := criticalLindyFixture(t, archLockLevel370)
			p.SpecialCoins[fameCounter] = 1 // Spending the last Fame removes the sidecar counter.
			peer := criticalTinyTradePeer(t, w, p)
			_, _, positions := criticalLindyExpected(w, p, req, cape)
			w.handle(command{s: p.Session, pkt: criticalLindyRequest(p, req)})
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
			if len(backend.entered) != 0 || fallback.saves != 0 || p.Trade == nil || peer.Trade == nil {
				t.Fatal("disconnect saved or cancelled a trade before Lindy committed")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if p.Trade == nil || peer.Trade == nil || counterBalance(p, fameCounter) != 0 {
				t.Fatal("Lindy lost the last-Fame debit or cancelled trade before final persistence")
			}
			if closing == "peer" {
				assertCriticalLindyPublication(t, p, positions, archLockLevel370)
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
					t.Fatal("final trade disconnect lost or repeated committed account/sidecar state")
				}
				seen[participant.Account.Name] = true
			}
			awaitReadyCriticalResults(t, queue, len(closed))
			for _, player := range closed {
				if !w.criticalPlayerReserved(player) || w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
					t.Fatal("unacknowledged final save released account ownership")
				}
			}
			w.processCriticalCompletions(1)
			if closing == "both" {
				if w.players[p.Session] != p || w.players[peer.Session] != peer || p.Trade == nil || peer.Trade == nil {
					t.Fatal("first final acknowledgement destroyed the peer write's retained identity")
				}
				w.processCriticalCompletions(1)
			}
			for _, player := range closed {
				if w.players[player.Session] != nil || !w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
					t.Fatal("final acknowledgement failed to release disconnected ownership")
				}
			}
			if p.Trade != nil || peer.Trade != nil || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 {
				t.Fatal("Lindy trade disconnect did not settle without inline persistence")
			}
			for _, player := range []*Player{p, peer} {
				if !player.Session.IsClosed() {
					assertCriticalPickupPacket(t, player, wire.CloseTrade(player.ID))
				}
				if player.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("disconnect sent extra responses or replied to a closed session")
				}
			}
		})
	}
}

type criticalLindyTerminalStore struct {
	*craftStore
	final   []criticalWrite
	flushes int
}

func (s *criticalLindyTerminalStore) SavePlayerState(_ *model.GuildRegistry, account *model.Account,
	uid string, state *model.CharState) error {
	s.final = append(s.final, criticalWrite{accounts: []*model.Account{accountPersistenceSnapshot(account)},
		character: &criticalCharacterState{uid: uid, state: state}})
	return nil
}

func (s *criticalLindyTerminalStore) Flush() { s.flushes++ }

func TestCriticalLindyShutdownDrainsBeforeFinalAccountAndSidecar(t *testing.T) {
	for _, level := range []uint32{archLockLevel355, archLockLevel370} {
		t.Run(fmt.Sprint(level+1), func(t *testing.T) {
			w, p, fallback, backend, queue, req, _, release := criticalLindyFixture(t, level)
			terminal := &criticalLindyTerminalStore{craftStore: fallback}
			w.store = terminal
			w.handle(command{s: p.Session, pkt: criticalLindyRequest(p, req)})
			proposal := awaitCriticalWrite(t, backend.entered)
			waiters := []chan error{make(chan error, 1), make(chan error, 1)}
			for _, done := range waiters {
				w.runShutdown(done)
				select {
				case <-done:
					t.Fatal("shutdown finished before Lindy acknowledgement")
				default:
				}
			}
			w.handle(command{s: p.Session})
			if len(terminal.final) != 0 || terminal.flushes != 0 || fallback.saves != 0 || !queue.stopping || w.shutdownFinished {
				t.Fatal("shutdown saved uncommitted Lindy state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			for _, done := range waiters {
				select {
				case err := <-done:
					if err != nil {
						t.Fatal(err)
					}
				default:
					t.Fatal("shutdown did not finish after Lindy acknowledgement")
				}
			}
			if len(terminal.final) != 1 || !reflect.DeepEqual(terminal.final[0].accounts, proposal.accounts) ||
				terminal.final[0].character.uid != p.Char.UID || !reflect.DeepEqual(terminal.final[0].character.state, buildCharState(p, w.now())) ||
				terminal.flushes != 1 || fallback.saves != 0 || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("shutdown lost committed account/Fame state, repeated a save or published to a closed session")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || len(terminal.final) != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed Lindy persistence")
			}
		})
	}
}
