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

type criticalTreasuryTestStore struct {
	*criticalGameStateTestStore
	finalRelease <-chan struct{}
}

func (s *criticalTreasuryTestStore) SaveCriticalAccounts(accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts})
}

func (s *criticalTreasuryTestStore) SaveCriticalInstanceState(state *model.InstanceStateSnapshot,
	accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, instances: state})
}

func (s *criticalTreasuryTestStore) SavePlayerState(guilds *model.GuildRegistry, account *model.Account,
	uid string, state *model.CharState) error {
	return s.SavePlayerStateBatch(guilds, uid, state, account)
}

func (s *criticalTreasuryTestStore) SavePlayerStateBatch(guilds *model.GuildRegistry,
	uid string, state *model.CharState, accounts ...*model.Account) error {
	write := criticalWrite{accounts: accounts, guilds: guilds,
		character: &criticalCharacterState{uid: uid, state: state}}
	if s.finalRelease != nil {
		s.entered <- write
		<-s.finalRelease
		return nil
	}
	return s.record(write)
}

func criticalTreasuryRequest(p *Player, collector *Mob) []byte {
	packet := challengePacket(wire.OpChallenge, p.ID, collector.ID, 0)
	binary.LittleEndian.PutUint16(packet[0:2], uint16(len(packet)))
	return packet
}

func configureCriticalTreasury(w *World, p *Player, amount uint64) *Mob {
	w.warLocation = time.UTC
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{ID: 1, Name: "Treasury", Members: []model.GuildMember{{
			Character: p.Char.Name, Account: p.Account.Name, Rank: model.GuildRankLeader,
		}}}}}
	p.Char.GuildID, p.Char.GuildRank = 1, model.GuildRankLeader
	w.guilds.Wars.Cities.Territories[0].Owner = 1
	w.guilds.Wars.Cities.Territories[0].Treasury = amount
	p.X, p.Y, p.Char.X, p.Char.Y = 2107, 2143, 2107, 2143
	w.updatePlayerSpatial(p)
	collector := &Mob{ID: 3100, X: 2107, Y: 2144,
		Def: &model.NPCDef{Name: "Collector", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 6, Level: 0}}}
	w.registerMobSpatial(collector)
	p.show(collector.ID)
	return collector
}

func criticalTreasuryFixture(t *testing.T, amount uint64) (*World, *Player, *Mob, *guildFlowStore,
	*criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	w.clock = newFakeClock(time.Date(2026, 9, 13, 12, 0, 0, 0, time.UTC))
	collector := configureCriticalTreasury(w, p, amount)
	w.warConfig.Enabled = true
	// Admission precedes the due-tick probe; handle also runs the scheduler.
	w.warNextTick = w.now().Add(time.Minute)
	fallback := &guildFlowStore{}
	w.store = fallback
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("treasury fixture account admission failed")
	}
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 8), release: gate}
	q := newCriticalTestQueue(t, &criticalTreasuryTestStore{criticalGameStateTestStore: &criticalGameStateTestStore{base}}, 2, 4)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	return w, p, collector, fallback, base, q, release
}

func TestCriticalCityTreasuryDetachedCommitAndSharedFence(t *testing.T) {
	for _, mode := range []string{"gold", "cheques", "partial", "gold-cap"} {
		for _, trading := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/trade-%t", mode, trading), func(t *testing.T) {
				amount := uint64(100)
				if mode == "cheques" || mode == "partial" {
					amount = 3_500_000_000
				}
				w, p, collector, fallback, base, q, release := criticalTreasuryFixture(t, amount)
				if mode == "partial" {
					for slot := 0; slot < model.PlayerCarrySlots-2; slot++ {
						p.Char.Inv[slot] = model.Item{Index: 400, UID: fmt.Sprintf("occupied-%d", slot)}
					}
				} else if mode == "gold-cap" {
					p.Char.Gold = maxCharacterGold - 100
				}
				var peer *Player
				if trading {
					peer = criticalTinyTradePeer(t, w, p)
				}
				before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
				nextWarTick := w.warNextTick
				trade := p.Trade
				request := criticalTreasuryRequest(p, collector)
				w.handle(command{s: p.Session, pkt: request})
				proposal := awaitCriticalWrite(t, base.entered)
				prepared := proposal.accounts[0].Chars[p.CharSlot]
				units, remaining, gold := 0, uint64(0), before.Chars[p.CharSlot].Gold+100
				if mode == "cheques" || mode == "partial" {
					units, remaining, gold = 3, 500_000_000, before.Chars[p.CharSlot].Gold
					if mode == "partial" {
						units, remaining = 2, 1_500_000_000
					}
				}
				participants := 1
				if trading {
					participants = 2
				}
				if proposal.guilds == nil || proposal.instances != nil || proposal.character != nil ||
					proposal.guilds.Wars.Cities.Territories[0].Treasury != remaining || prepared.Gold != gold ||
					len(proposal.accounts) != participants {
					t.Fatal("collector did not prepare one atomic account/guild proposal")
				}
				ids := make(map[string]bool)
				for slot, item := range prepared.Inv {
					if item == before.Chars[p.CharSlot].Inv[slot] {
						continue
					}
					if item.Index != cityWarChequeItem || item.UID == "" || ids[item.UID] {
						t.Fatal("cheque identity missing, repeated or wrong")
					}
					ids[item.UID] = true
				}
				if len(ids) != units {
					t.Fatal("private proposal paid the wrong number of cheques")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					w.handle(command{s: p.Session, pkt: request})
					if err := w.withdrawCityTreasury(p, 0); !errors.Is(err, errCriticalPersistenceBusy) {
						t.Fatalf("duplicate withdrawal was not fenced: %v", err)
					}
					called := false
					change := func() error { called = true; return nil }
					if !errors.Is(w.guildEconomyCommit(p, change), errCriticalPersistenceBusy) ||
						!errors.Is(w.warCommit(change), errCriticalPersistenceBusy) || called ||
						!errors.Is(w.creditCityTreasury(0, 1), errCriticalPersistenceBusy) ||
						!errors.Is(w.saveGuildState(p.Account), errCriticalPersistenceBusy) || !w.criticalKillParticipantsReserved(nil) {
						t.Fatal("shared guild consumer bypassed the pending aggregate reservation")
					}
					w.tickGuildWars(w.now().Add(time.Hour))
					if trading {
						w.cancelTrade(peer, "competing cancellation")
						w.tickTrades(w.now().Add(time.Hour))
						if !w.criticalPlayerReserved(peer) || peer.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("reciprocal trade participant was not retained")
						}
					}
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
						p.Trade != trade || !w.criticalGuildReserved() || w.warNextTick != nextWarTick || p.Session.QueuedPacketsForTest() != 0 ||
						fallback.gameSaves != 0 || fallback.saves != 0 || len(base.entered) != 0 {
						t.Fatalf("%s treasury changed or published before actor acknowledgement", phase)
					}
				}
				discardCriticalDeferredDuplicates(w)

				w.processCriticalCompletions(1)
				if w.shuttingDown || len(q.pending) != 0 || w.criticalGuildReserved() || p.Char.Gold != gold ||
					p.Char.Inv != prepared.Inv || w.guilds.Wars.Cities.Territories[0].Treasury != remaining || p.Trade != trade {
					t.Fatal("committed collection did not apply exactly the private payout")
				}
				assertCriticalPickupPacket(t, p, wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
				assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
				message := "Collected 100 gold from the city treasury."
				if units != 0 {
					message = fmt.Sprintf("Collected %d billion-gold cheque(s) from the city treasury.", units)
				}
				assertCriticalPickupPacket(t, p, wire.MessagePanel(message))
				discardCriticalDeferredDuplicates(w)

				w.processCriticalCompletions(1)
				if p.Session.QueuedPacketsForTest() != 0 || len(base.entered) != 0 {
					t.Fatal("completion was repeated")
				}
			})
		}
	}
}

func TestCriticalCityTreasuryDisconnectWaitsForPayoutAndFinalSave(t *testing.T) {
	w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 2*cityWarChequeValue)
	finalGate, releaseFinal := criticalGate(t)
	t.Cleanup(releaseFinal)
	q.store.(*criticalTreasuryTestStore).finalRelease = finalGate
	before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
	session, uid, key := p.Session, p.Char.UID, accountSessionKey(p.Account.Name)
	reconnecting := gameNet.NewTestSession(3, 8)
	if err := w.withdrawCityTreasury(p, 0); err != nil {
		t.Fatal(err)
	}
	proposal := awaitCriticalWrite(t, base.entered)
	w.handle(command{s: session})
	w.handle(command{s: session})
	for _, phase := range []string{"payout-held", "payout-completed-unacknowledged"} {
		if phase == "payout-completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, q, 1)
		}
		if !session.IsClosed() || w.players[session] != p || w.accountSessions[key] != session ||
			!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
			len(base.entered) != 0 || len(w.criticalDisconnects) != 1 || w.claimAccountSession(reconnecting, p.Account.Name) {
			t.Fatalf("%s disconnect lost ownership or saved unpaid balances", phase)
		}
	}
	w.processCriticalCompletions(1)
	final := awaitCriticalWrite(t, base.entered)
	if final.character == nil || final.character.uid != uid || final.accounts[0] == p.Account ||
		final.accounts[0].Chars[p.CharSlot].Inv != proposal.accounts[0].Chars[p.CharSlot].Inv ||
		w.guilds.Wars.Cities.Territories[0].Treasury != 0 || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
		t.Fatal("final account/sidecar save omitted the acknowledged treasury payout")
	}
	for _, phase := range []string{"final-held", "final-completed-unacknowledged"} {
		if phase == "final-completed-unacknowledged" {
			releaseFinal()
			awaitReadyCriticalResults(t, q, 1)
		}
		w.handle(command{s: session})
		w.resumeCriticalDisconnects(8)
		if len(q.pending) != 1 || w.players[session] != p || w.accountSessions[key] != session || len(base.entered) != 0 ||
			session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || fallback.gameSaves != 0 || w.claimAccountSession(reconnecting, p.Account.Name) {
			t.Fatalf("%s final save was repeated or readmission happened before acknowledgement", phase)
		}
	}
	w.processCriticalCompletions(1)
	if w.players[session] != nil || w.playersByCharacterUID[uid] != nil || w.accountSessions[key] != nil ||
		len(q.pending) != 0 || len(w.criticalDisconnects) != 0 || session.QueuedPacketsForTest() != 0 ||
		w.shuttingDown || !w.claimAccountSession(reconnecting, p.Account.Name) {
		t.Fatal("acknowledged final save did not release ownership exactly once")
	}
}

func TestCriticalCityTreasuryActualWorldLoopProgress(t *testing.T) {
	var collector *Mob
	w, p, unrelatedUID, base, q, ticks, release, stop := criticalEconomyLoopFixture(t, nil, func(w *World, p *Player) {
		// Treasury admission must not depend on the machine's current weekday.
		w.clock = newFakeClock(time.Date(2026, 9, 13, 12, 0, 0, 0, time.UTC))
		w.playersByID[2].NextRegen = w.now().Add(-time.Second)
		collector = configureCriticalTreasury(w, p, 100)
		w.warConfig.Enabled = false
		base := w.criticalWrites.queue.store.(*criticalTestStore)
		w.criticalWrites.queue.store = &criticalTreasuryTestStore{criticalGameStateTestStore: &criticalGameStateTestStore{base}}
	})
	deliver := func() {
		t.Helper()
		select {
		case w.commands <- command{s: p.Session, pkt: criticalTreasuryRequest(p, collector)}:
		case <-time.After(time.Second):
			t.Fatal("World did not accept the treasury intention")
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
			t.Fatal("held treasury commit blocked unrelated ticks/commands or confirmed early")
		}
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() < 3 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	stop() // Join the actor before reading mutable state and reservations.
	if p.Char.Gold != 1100 || w.guilds.Wars.Cities.Territories[0].Treasury != 0 || w.shuttingDown ||
		len(q.pending) != 0 || len(base.entered) != 0 || w.mobTickCounter < 3 || p.Session.QueuedPacketsForTest() != 3 {
		t.Fatal("actual World did not settle one payout while advancing unrelated ticks")
	}
}

func TestCriticalCityTreasuryCertifiedRejectionAndExplicitRetry(t *testing.T) {
	for _, amount := range []uint64{100, 2 * cityWarChequeValue} {
		t.Run(fmt.Sprint(amount), func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, amount)
			peer := criticalTinyTradePeer(t, w, p)
			before, retained, trade := accountStateSnapshot(p.Account), w.criticalGuildSnapshot(), p.Trade
			base.failure = criticalCertifiedRejection(t)
			if err := w.withdrawCityTreasury(p, 0); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, base.entered)
			release()
			result := awaitCriticalResult(t, q)
			base.failure = nil
			w.applyCriticalCompletion(result)
			if w.shuttingDown || len(q.pending) != 0 || w.criticalGuildReserved() || p.Trade != trade ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
				len(base.entered) != 0 || fallback.gameSaves != 0 || peer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("certified rejection changed the payout, trade or guild, or replayed automatically")
			}
			assertCriticalPickupPacket(t, p, wire.MessagePanel("The city treasury collection could not be completed."))
			if err := w.withdrawCityTreasury(p, 0); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if p.Char.Gold != proposal.accounts[0].Chars[p.CharSlot].Gold || p.Char.Inv != proposal.accounts[0].Chars[p.CharSlot].Inv ||
				w.guilds.Wars.Cities.Territories[0].Treasury != 0 || len(q.pending) != 0 {
				t.Fatal("explicit new collection did not commit exactly once after rejection")
			}
			if err := w.withdrawCityTreasury(p, 0); err == nil || len(base.entered) != 0 {
				t.Fatal("empty treasury was collected again")
			}
		})
	}
}

func TestCriticalCityTreasuryUnknownAndStaleContextsRetainFences(t *testing.T) {
	for _, mode := range []string{"unknown", "panic", "account", "guild-member", "treasury", "registry", "peer", "trade", "uid", "lifecycle"} {
		t.Run(mode, func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
			peer := criticalTinyTradePeer(t, w, p)
			if mode == "unknown" {
				base.failure = errors.New("commit acknowledgement lost")
			} else if mode == "panic" {
				base.panicAfterWrite = true
			}
			if err := w.withdrawCityTreasury(p, 0); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, base.entered)
			switch mode {
			case "account":
				p.Account.CargoGold++
			case "guild-member":
				w.guilds.Guilds[0].Members[0].Rank = model.GuildRankMember
			case "treasury":
				w.guilds.Wars.Cities.Territories[0].Treasury++
			case "registry":
				w.guilds = w.criticalGuildSnapshot()
			case "peer":
				peer.Account.CargoGold++
			case "trade":
				peer.Trade.Gold++
			case "uid":
				p.Char.UID = "replacement"
			case "lifecycle":
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned || len(q.pending) != 1 || !w.criticalGuildReserved() ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
				p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 || fallback.gameSaves != 0 {
				t.Fatal("unverified completion published, replayed or released guild/account fences")
			}
		})
	}
}

func TestCriticalCityTreasuryAdmissionDoesNotWrite(t *testing.T) {
	for _, mode := range []string{"full", "stopping", "maintenance", "unsupported", "closed", "identity", "saturday", "foreign-owner",
		"not-leader", "empty", "treasury-cap", "gold-cap", "full-inventory", "closed-peer", "nonreciprocal-trade"} {
		t.Run(mode, func(t *testing.T) {
			w, p, _, fallback, base, q, _ := criticalTreasuryFixture(t, 100)
			switch mode {
			case "full":
				q.capacity = 0
			case "stopping":
				q.closeAdmission()
			case "maintenance":
				w.shuttingDown = true
			case "unsupported":
				q.store = base
			case "closed":
				p.Session.Close()
			case "identity":
				p.Char = &model.Char{}
			case "saturday":
				w.clock = newFakeClock(time.Date(2026, 9, 12, 12, 0, 0, 0, time.UTC))
			case "foreign-owner":
				w.guilds.Wars.Cities.Territories[0].Owner = 2
			case "not-leader":
				w.guilds.Guilds[0].Members[0].Rank = model.GuildRankMember
			case "empty":
				w.guilds.Wars.Cities.Territories[0].Treasury = 0
			case "treasury-cap":
				w.guilds.Wars.Cities.Territories[0].Treasury = model.GuildWarTreasuryCap + 1
			case "gold-cap":
				p.Char.Gold = maxCharacterGold - 99
			case "full-inventory":
				w.guilds.Wars.Cities.Territories[0].Treasury = cityWarChequeValue
				for slot := 0; slot < model.PlayerCarrySlots; slot++ {
					p.Char.Inv[slot] = model.Item{Index: 400}
				}
			case "closed-peer":
				criticalTinyTradePeer(t, w, p).Session.Close()
			case "nonreciprocal-trade":
				criticalTinyTradePeer(t, w, p).Trade.OpponentID++
			}
			before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
			if err := w.withdrawCityTreasury(p, 0); err == nil || len(q.pending) != 0 || q.guilds != 0 || len(q.accounts) != 0 ||
				len(base.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
				!reflect.DeepEqual(w.criticalGuildSnapshot(), retained) || p.Session.QueuedPacketsForTest() != 0 || fallback.gameSaves != 0 {
				t.Fatal("rejected collection wrote, mutated, reserved or confirmed")
			}
		})
	}
}
