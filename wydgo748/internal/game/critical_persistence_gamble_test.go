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
	"wydgo/internal/wire"
)

type criticalGambleTestStore struct{ *criticalStateTestStore }

func (s *criticalGambleTestStore) SaveCriticalInstanceState(state *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, instances: state})
}

func criticalGambleRequest(p *Player, bet uint32) []byte {
	pkt := gambleBetPacket(1, bet)
	binary.LittleEndian.PutUint16(pkt[6:8], p.ID)
	return pkt
}

func configureCriticalGamble(w *World, p *Player, stops [3]byte) *gambleSequenceRNG {
	rng := &gambleSequenceRNG{values: []int{99, int(stops[0]), int(stops[1]), int(stops[2]), 99, int(stops[0]), int(stops[1]), int(stops[2])}}
	w.rng = rng
	p.Char.Gold, w.gambleJackpot, w.gamblePool = 10_000, 100, 200
	return rng
}

func criticalGambleFixture(t *testing.T, stops [3]byte) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, *gambleSequenceRNG, func()) {
	t.Helper()
	w, p, fallback := handlerTestWorld(t)
	w.clock = newFakeClock(time.Unix(100, 0))
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("Gamble fixture account admission failed")
	}
	rng := configureCriticalGamble(w, p, stops)
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 8), release: gate}
	q := newCriticalTestQueue(t, &criticalGambleTestStore{&criticalStateTestStore{&criticalBatchTestStore{base}}}, 2, 4)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	return w, p, fallback, base, q, rng, release
}

func TestCriticalGambleDetachedCommitAndSharedFence(t *testing.T) {
	for _, mode := range []string{"loss", "jackpot", "silver"} {
		for _, trading := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/trade-%t", mode, trading), func(t *testing.T) {
				stops := findGambleStops(t, func(r [5]byte) bool {
					if mode == "loss" {
						return gamblePrize(1000, r) == 0
					}
					return r[1] == 14
				})
				w, p, fallback, base, q, rng, release := criticalGambleFixture(t, stops)
				if mode == "silver" {
					p.Char.Gold, w.gambleJackpot, w.gamblePool = maxCharacterGold, gambleJackpotMax, gamblePoolMax
				}
				peer := criticalPickupCompetitor(t, w, &GroundItem{X: p.X, Y: p.Y})
				if trading {
					p.Trade = &TradeState{OpponentID: peer.ID, Gold: 12, ExpiresAt: w.now().Add(time.Minute)}
					peer.Trade = &TradeState{OpponentID: p.ID, Gold: 34, ExpiresAt: w.now().Add(time.Minute)}
				}
				before, peerBefore, trade, peerTrade := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account), p.Trade, peer.Trade
				jackpot, pool := w.gambleJackpot, w.gamblePool
				w.markInstanceStateDirty()
				w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
				proposal := awaitCriticalWrite(t, base.entered)
				wantJackpot := uint64(jackpot) + 1
				if wantJackpot > uint64(gambleJackpotMax) {
					wantJackpot = uint64(gambleJackpotMax)
				}
				result := gambleResults(stops)
				prize := gamblePrize(1000, result)
				if result[1] == 14 {
					prize += wantJackpot
					wantJackpot = 0
				}
				if prize > math.MaxInt32 {
					prize = math.MaxInt32
				}
				wantPool := min(pool+30, gamblePoolMax)
				if prize/1000 > wantPool {
					wantPool = 0
				} else {
					wantPool -= prize / 1000
				}
				wantGold, payout := gamblePayoutItems(uint64(before.Chars[0].Gold-1000) + prize)
				if proposal.instances == nil || proposal.instances.GambleJackpot != uint32(wantJackpot) || proposal.instances.GamblePool != wantPool ||
					proposal.accounts[0].Chars[0].Gold != wantGold || len(proposal.accounts) != 1+map[bool]int{false: 0, true: 1}[trading] {
					t.Fatal("detached bet lost its account, prize or atomic pools")
				}
				for i, index := range payout {
					item := proposal.accounts[0].Chars[0].Inv[i]
					if item.Index != index || item.UID == "" {
						t.Fatal("silver payout was not materialized before persistence")
					}
				}
				for _, completed := range []bool{false, true} {
					if completed {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					w.onDoJackpotBet(p.Session, criticalGambleRequest(p, 1000))
					w.onDoJackpotBet(peer.Session, criticalGambleRequest(peer, 1000))
					w.cancelTrade(p, "competing cancellation")
					w.tickTrades(w.now().Add(time.Hour))
					w.flushInstanceStateIfDirty()
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
						w.gambleJackpot != jackpot || w.gamblePool != pool || p.Trade != trade || peer.Trade != peerTrade || !w.criticalInstanceReserved() ||
						p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || rng.next != 4 || len(base.entered) != 0 || fallback.saves != 0 {
						t.Fatal("held or unacknowledged bet mutated, published or rerolled")
					}
				}
				w.processCriticalCompletions(1)
				if p.Char.Gold != wantGold || p.Char.Inv != proposal.accounts[0].Chars[0].Inv || w.gambleJackpot != uint32(wantJackpot) ||
					w.gamblePool != wantPool || w.instanceStateDirty || w.criticalInstanceReserved() || len(q.pending) != 0 || w.shuttingDown {
					t.Fatal("actor did not settle the committed bet exactly once")
				}
				if trading {
					assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
					assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
					if p.Trade != nil || peer.Trade != nil {
						t.Fatal("committed bet retained the trade")
					}
				}
				assertCriticalPickupPacket(t, p, wire.ResultGamble(p.ID, result, stops, int32(prize), uint32(wantJackpot)))
				assertCriticalPickupPacket(t, p, wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
				assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
			})
		}
	}
}

func TestCriticalGambleCertifiedRejectionAndClosedTransport(t *testing.T) {
	stops := findGambleStops(t, func(r [5]byte) bool { return gamblePrize(1000, r) == 0 })
	for _, mode := range []string{"solo", "trade", "closed-owner", "closed-peer", "closed-both"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, base, q, rng, release := criticalGambleFixture(t, stops)
			var peer *Player
			if mode != "solo" {
				peer = criticalTinyTradePeer(t, w, p)
			}
			before, trade := accountStateSnapshot(p.Account), p.Trade
			base.failure = criticalCertifiedRejection(t)
			w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
			awaitCriticalWrite(t, base.entered)
			w.onDoJackpotBet(p.Session, criticalGambleRequest(p, 1000))
			var closed []*Player
			if mode == "closed-owner" || mode == "closed-both" {
				closed = append(closed, p)
			}
			if mode == "closed-peer" || mode == "closed-both" {
				closed = append(closed, peer)
			}
			for _, participant := range closed {
				w.handle(command{s: participant.Session})
			}
			release()
			result := awaitCriticalResult(t, q)
			base.failure = nil
			w.applyCriticalCompletion(result)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || w.gambleJackpot != 100 || w.gamblePool != 200 || p.Trade != trade ||
				w.shuttingDown || w.criticalInstanceReserved() || rng.next != 4 || fallback.saves != 0 {
				t.Fatal("certified rejection paid out, cancelled the trade, rerolled or entered maintenance")
			}
			if !p.Session.IsClosed() {
				assertCriticalPickupPacket(t, p, wire.MessagePanel("The Gamble bet could not be completed."))
			}
			if len(closed) == 0 {
				if len(q.pending) != 0 || len(base.entered) != 0 {
					t.Fatal("rejection automatically replayed the bet")
				}
				w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
				awaitCriticalWrite(t, base.entered)
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
				if p.Char.Gold != 9000 || w.gamblePool != 230 || rng.next != 8 || len(q.pending) != 0 {
					t.Fatal("explicit new intention failed after certified rejection")
				}
			} else {
				for range closed {
					final := awaitCriticalWrite(t, base.entered)
					if len(final.accounts) != 1 || final.character == nil || final.instances != nil {
						t.Fatal("disconnect lost its final atomic account/sidecar form")
					}
					if final.accounts[0].Name == p.Account.Name && !reflect.DeepEqual(final.accounts[0], accountPersistenceSnapshot(before)) {
						t.Fatal("disconnect saved the rejected bet")
					}
				}
				for range closed {
					w.applyCriticalCompletion(awaitCriticalResult(t, q))
				}
				for _, participant := range closed {
					if w.players[participant.Session] != nil || participant.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("disconnect cleanup retained identity or replied to closed transport")
					}
				}
			}
		})
	}
}

func TestCriticalGambleUnknownAndStaleContextsRetainFences(t *testing.T) {
	stops := findGambleStops(t, func(r [5]byte) bool { return gamblePrize(1000, r) == 0 })
	for _, mode := range []string{"unknown", "account", "peer", "trade", "generation", "pool", "party-runs", "lifecycle"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, base, q, _, release := criticalGambleFixture(t, stops)
			peer := criticalTinyTradePeer(t, w, p)
			base.failure = criticalCertifiedRejection(t)
			if mode == "unknown" {
				base.failure = errors.New("commit response lost")
			}
			w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
			awaitCriticalWrite(t, base.entered)
			switch mode {
			case "account":
				p.Account.CargoGold++
			case "peer":
				peer.Account.CargoGold++
			case "trade":
				peer.Trade.Gold++
			case "generation":
				w.markInstanceStateDirty()
			case "pool":
				w.gamblePool++
			case "party-runs":
				w.nightmarePartyRuns = map[string]int{"other": 1}
			case "lifecycle":
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			before, pool := accountStateSnapshot(p.Account), w.gamblePool
			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned || len(q.pending) != 1 || !w.criticalInstanceReserved() ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || w.gamblePool != pool || w.gambleJackpot != 100 ||
				p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || len(base.entered) != 0 {
				t.Fatal("unverified result released fences, replayed or published")
			}
		})
	}
}

func TestCriticalGambleActualWorldLoopProgress(t *testing.T) {
	stops := findGambleStops(t, func(r [5]byte) bool { return gamblePrize(1000, r) == 0 })
	w, p, unrelatedUID, base, q, ticks, release, stop := criticalEconomyLoopFixture(t, nil, func(w *World, p *Player) {
		configureCriticalGamble(w, p, stops)
		base := w.criticalWrites.queue.store.(*criticalTestStore)
		w.criticalWrites.queue.store = &criticalCertifiedInstanceTestStore{base}
	})
	deliver := func() {
		t.Helper()
		select {
		case w.commands <- command{s: p.Session, pkt: criticalGambleRequest(p, 1000)}:
		case <-time.After(time.Second):
			t.Fatal("World did not accept the Gamble intention")
		}
	}
	deliver()
	awaitCriticalWrite(t, base.entered)
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
			t.Fatal("held Gamble blocked unrelated commands/ticks or confirmed early")
		}
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() < 3 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	stop() // Join the actor before inspecting mutable pools and queue maps.
	if p.Char.Gold != 9000 || w.gambleJackpot != 101 || w.gamblePool != 230 || w.shuttingDown || len(q.pending) != 0 ||
		len(base.entered) != 0 || w.mobTickCounter < 3 || w.rng.(*gambleSequenceRNG).next != 4 {
		t.Fatal("actual World did not settle one bet while advancing unrelated ticks")
	}
}

func TestCriticalGambleAdmissionDoesNotRollOrWrite(t *testing.T) {
	stops := findGambleStops(t, func(r [5]byte) bool { return gamblePrize(1000, r) == 0 })
	for _, mode := range []string{"full", "stopping", "maintenance", "unsupported", "unstable-instance", "owner-identity", "peer-identity", "closed-peer", "nonreciprocal-trade", "invalid-kind", "invalid-bet", "insufficient-gold"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, base, q, rng, _ := criticalGambleFixture(t, stops)
			peer := criticalTinyTradePeer(t, w, p)
			kind, bet := uint32(1), uint32(1000)
			switch mode {
			case "full":
				q.capacity = 0
			case "stopping":
				q.closeAdmission()
			case "maintenance":
				w.shuttingDown = true
			case "unsupported":
				q.store = base
			case "unstable-instance":
				w.itemInstances = map[string]*ItemInstance{"water-missing": {
					Config: model.VolatileInstance{ID: "water-missing"}, MemberIDs: []uint16{999},
				}}
			case "owner-identity":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "peer-identity":
				delete(w.accountSessions, accountSessionKey(peer.Account.Name))
			case "closed-peer":
				peer.Session.Close()
			case "nonreciprocal-trade":
				peer.Trade.OpponentID = 999
			case "invalid-kind":
				kind = 3
			case "invalid-bet":
				bet = 1001
			case "insufficient-gold":
				p.Char.Gold = 999
			}
			before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
			trade, peerTrade := *p.Trade, *peer.Trade
			if err := w.queueGamble(p, kind, bet); err == nil {
				t.Fatal("invalid Gamble admission succeeded")
			}
			if rng.next != 0 || len(base.entered) != 0 || len(q.pending) != 0 || w.criticalInstanceReserved() || fallback.saves != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
				*p.Trade != trade || *peer.Trade != peerTrade || w.gambleJackpot != 100 || w.gamblePool != 200 ||
				p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("invalid admission rolled, saved, changed retained state or published")
			}
		})
	}
}

func TestCriticalGambleFullInventoryRejectsPrivatePayout(t *testing.T) {
	stops := findGambleStops(t, func(r [5]byte) bool { return r[1] == 14 })
	w, p, fallback, base, q, rng, _ := criticalGambleFixture(t, stops)
	peer := criticalTinyTradePeer(t, w, p)
	p.Char.Gold, w.gambleJackpot, w.gamblePool = maxCharacterGold, gambleJackpotMax, gamblePoolMax
	for slot := range model.PlayerCarrySlots {
		p.Char.Inv[slot] = model.Item{Index: 4011}
	}
	before, trade, peerTrade := accountStateSnapshot(p.Account), p.Trade, peer.Trade
	w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade || peer.Trade != peerTrade ||
		w.gambleJackpot != gambleJackpotMax || w.gamblePool != gamblePoolMax || rng.next != 4 || len(base.entered) != 0 ||
		len(q.pending) != 0 || w.criticalInstanceReserved() || fallback.saves != 0 || peer.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("inventory rejection lost gold/pools, cancelled trade or admitted a write")
	}
	assertCriticalPickupPacket(t, p, wire.MessagePanel("Free more inventory slots before playing Gamble."))
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("inventory rejection published a Gamble result")
	}
}

func TestCriticalGambleClockPassageIsNotAnActorMutation(t *testing.T) {
	stops := findGambleStops(t, func(r [5]byte) bool { return gamblePrize(1000, r) == 0 })
	w, p, _, base, q, _, release := criticalGambleFixture(t, stops)
	w.itemInstances = map[string]*ItemInstance{"water-clock": {
		Config: model.VolatileInstance{ID: "water-clock"}, MemberIDs: []uint16{p.ID},
		HardDeadline: w.now().Add(time.Second),
	}}
	w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
	proposal := awaitCriticalWrite(t, base.entered)
	if len(proposal.instances.Instances) != 1 {
		t.Fatal("private proposal did not retain the resumable instance")
	}
	w.clock.(*fakeClock).Advance(2 * time.Second)
	release()
	w.applyCriticalCompletion(awaitCriticalResult(t, q))
	if w.shuttingDown || w.criticalInstanceReserved() || len(q.pending) != 0 || p.Char.Gold != 9000 || w.gamblePool != 230 {
		t.Fatal("wall-clock passage falsely invalidated unchanged retained actor state")
	}
}
