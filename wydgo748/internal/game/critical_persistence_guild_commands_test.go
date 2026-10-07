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

func criticalGuildTestPacket(op, actor, guild, target uint16) []byte {
	packet := inboundPacket(op, 20)
	binary.LittleEndian.PutUint16(packet[6:8], actor)
	binary.LittleEndian.PutUint32(packet[12:16], uint32(guild))
	binary.LittleEndian.PutUint32(packet[16:20], uint32(target))
	return packet
}

func criticalGuildCreateFixture(t *testing.T) (*World, *Player, *guildFlowStore, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	w.warConfig.Enabled = false
	p.Char.GuildID, p.Char.GuildRank = 0, 0
	return w, p, fallback, base, q, release
}

func TestCriticalGuildCreateDetachedCommit(t *testing.T) {
	for _, trading := range []bool{false, true} {
		t.Run(fmt.Sprint(trading), func(t *testing.T) {
			w, p, fallback, base, q, release := criticalGuildCreateFixture(t)
			var peer *Player
			accountCount := 1
			if trading {
				peer = criticalTinyTradePeer(t, w, p)
				accountCount++
			}
			// Legacy save pruning must also be detached, including invalid references.
			w.guilds.Wars.Kefra.Guild = 7
			registry, before, retained, trade := w.guilds, accountStateSnapshot(p.Account), w.criticalGuildSnapshot(), p.Trade
			w.guildCommandCreate(p.Session, p, "Created")
			proposal := awaitCriticalWrite(t, base.entered)
			guild := proposal.guilds.FindByName("Created")
			if guild == nil || guild.ID != 1 || guild.Members[0].Character != p.Char.Name ||
				guild.Members[0].Rank != model.GuildRankLeader || proposal.guilds.Wars.Kefra.Guild != 0 ||
				proposal.accounts[0] == p.Account || proposal.accounts[0].Chars[p.CharSlot].GuildID != 1 ||
				proposal.accounts[0].Chars[p.CharSlot].GuildRank != model.GuildRankLeader || len(proposal.accounts) != accountCount {
				t.Fatal("creation did not submit a detached canonical registry/account proposal")
			}
			if trading && (!w.criticalPlayerReserved(peer) || !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peer.Account))) {
				t.Fatal("creation omitted the reciprocal trade participant")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				w.guildCommandCreate(p.Session, p, "Repeated")
				w.cancelTrade(p, "competing cancellation")
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
					w.guilds != registry || p.Trade != trade || !w.criticalGuildReserved() || !w.criticalPlayerReserved(p) ||
					fallback.gameSaves != 0 || len(base.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatalf("%s creation mutated, published, replayed or released retained state", phase)
				}
			}
			w.processCriticalCompletions(1)
			want := accountStateSnapshot(before)
			want.Chars[p.CharSlot].GuildID, want.Chars[p.CharSlot].GuildRank = 1, model.GuildRankLeader
			if w.shuttingDown || w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), want) || w.criticalGuildReserved() ||
				len(q.pending) != 0 || fallback.gameSaves != 0 || p.Trade != trade || p.Session.QueuedPacketsForTest() != 4 {
				t.Fatalf("creation acknowledgement mismatch: maintenance=%t registry=%t guild=%t account=%t reserved=%t pending=%d saves=%d trade=%t packets=%d\nlive=%+v\nproposal=%+v",
					w.shuttingDown, w.guilds == registry, reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds),
					reflect.DeepEqual(accountStateSnapshot(p.Account), want), w.criticalGuildReserved(), len(q.pending), fallback.gameSaves,
					p.Trade == trade, p.Session.QueuedPacketsForTest(), w.guilds, proposal.guilds)
			}
			assertCriticalPickupPacket(t, p, wire.MessagePanel("Guild \"Created\" created."))
			assertCriticalPickupPacket(t, p, wire.CreateMobWithGuildRank(p.ID, p.Char.Name, p.X, p.Y,
				bodyMesh(p.Char), bodyAncient(p.Char), wireScoreState(p.Char), p.Char.Affects[:],
				2, p.Char.GuildID, p.Char.GuildRank, p.Char.CP))
			assertCriticalPickupPacket(t, p, playerScorePacket(p))
			assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
		})
	}
}

func TestCriticalGuildCreateRejectionAndExplicitRetry(t *testing.T) {
	w, p, fallback, base, q, release := criticalGuildCreateFixture(t)
	peer := criticalTinyTradePeer(t, w, p)
	before, retained, trade := accountStateSnapshot(p.Account), w.criticalGuildSnapshot(), p.Trade
	base.failure = criticalCertifiedRejection(t)
	w.guildCommandCreate(p.Session, p, "Created")
	awaitCriticalWrite(t, base.entered)
	release()
	result := awaitCriticalResult(t, q)
	base.failure = nil
	w.applyCriticalCompletion(result)
	if w.shuttingDown || len(q.pending) != 0 || w.criticalGuildReserved() || w.criticalPlayerReserved(peer) ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
		fallback.gameSaves != 0 || len(base.entered) != 0 || p.Trade != trade {
		t.Fatal("certified creation rejection mutated, replayed or retained settled fences")
	}
	assertCriticalPickupPacket(t, p, wire.MessagePanel("Save failed. The guild was not created."))
	w.guildCommandCreate(p.Session, p, "Created")
	awaitCriticalWrite(t, base.entered)
	w.applyCriticalCompletion(awaitCriticalResult(t, q))
	if p.Char.GuildID != 1 || w.guilds.FindByName("Created") == nil || len(q.pending) != 0 || w.shuttingDown {
		t.Fatal("explicit retry did not create one guild after definite rejection")
	}
}

func TestCriticalGuildCreateUnknownAndStaleRetainFences(t *testing.T) {
	for _, mode := range []string{"unknown", "panic", "account", "registry", "guild", "trade", "peer", "lifecycle"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, base, q, release := criticalGuildCreateFixture(t)
			peer := criticalTinyTradePeer(t, w, p)
			if mode == "unknown" {
				base.failure = errors.New("lost commit acknowledgement")
			} else if mode == "panic" {
				base.panicAfterWrite = true
			}
			w.guildCommandCreate(p.Session, p, "Created")
			awaitCriticalWrite(t, base.entered)
			switch mode {
			case "account":
				p.Account.CargoGold++
			case "registry":
				w.guilds = w.criticalGuildSnapshot()
			case "guild":
				w.guilds.Wars.Kefra.Guild = 7
			case "trade":
				peer.Trade.Gold++
			case "peer":
				peer.Account.CargoGold++
			case "lifecycle":
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned || !w.criticalGuildReserved() ||
				len(q.pending) != 1 || len(base.entered) != 0 || fallback.gameSaves != 0 || p.Session.QueuedPacketsForTest() != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) {
				t.Fatal("unknown/stale creation applied, published, replayed or released a fence")
			}
		})
	}
}

func TestCriticalGuildCreateAdmissionDoesNotWrite(t *testing.T) {
	for _, mode := range []string{"full", "stopping", "maintenance", "unsupported", "closed", "identity", "invalid-name", "name-conflict", "member", "closed-peer", "nonreciprocal"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, base, q, _ := criticalGuildCreateFixture(t)
			name := "Created"
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
				delete(w.playersByID, p.ID)
			case "invalid-name":
				name = ""
			case "name-conflict":
				w.guilds.Guilds = []model.Guild{{ID: 1, Name: name}}
			case "member":
				w.guilds.Guilds = []model.Guild{{ID: 1, Name: "Existing", Members: []model.GuildMember{{Character: p.Char.Name}}}}
			case "closed-peer":
				criticalTinyTradePeer(t, w, p).Session.Close()
			case "nonreciprocal":
				criticalTinyTradePeer(t, w, p).Trade.OpponentID = 0
			}
			before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
			if err := w.queueGuildCreate(p, name); err == nil || len(q.pending) != 0 || len(base.entered) != 0 ||
				fallback.gameSaves != 0 || p.Session.QueuedPacketsForTest() != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) {
				t.Fatal("rejected creation admission changed state, wrote or published")
			}
		})
	}
}

func TestCriticalGuildCommandsPreserveHeldRegistry(t *testing.T) {
	w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
	other := criticalPickupCompetitor(t, w, &GroundItem{X: p.X, Y: p.Y})
	w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Other", WarTarget: 1,
		Members: []model.GuildMember{{Character: other.Char.Name, Account: other.Account.Name, Rank: model.GuildRankLeader}}})
	other.Char.GuildID, other.Char.GuildRank = 2, model.GuildRankLeader
	other.GuildInviteFrom, other.GuildInviteUntil = 1, w.now().Add(time.Minute)
	registry := w.guilds
	wantGold := p.Char.Gold + 100
	before, retained := accountStateSnapshot(other.Account), w.criticalGuildSnapshot()
	if err := w.withdrawCityTreasury(p, 0); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, base.entered)
	calls := []struct {
		name string
		run  func()
	}{
		{"create", func() { w.guildCommandCreate(other.Session, other, "Blocked") }},
		{"invite", func() { w.guildCommandInvite(other.Session, other, p.Char.Name) }},
		{"accept", func() { w.guildCommandAccept(other.Session, other, "") }},
		{"leave", func() { w.guildCommandLeave(other.Session, other, "") }},
		{"expel", func() { w.guildCommandExpel(other.Session, other, p.Char.Name) }},
		{"promote", func() { w.guildCommandSubLeader(other.Session, other, p.Char.Name) }},
		{"native-expel", func() {
			w.onGuildDeprivate(other.Session, criticalGuildTestPacket(wire.OpGuildDeprivate, other.ID, p.ID, 0))
		}},
		{"native-recruit", func() { w.onInviteGuild(other.Session, criticalGuildTestPacket(wire.OpInviteGuild, other.ID, p.ID, 0)) }},
		{"ally", func() { w.onGuildAlly(other.Session, criticalGuildTestPacket(wire.OpGuildAlly, other.ID, 2, 1)) }},
		{"war", func() { w.onGuildWar(other.Session, criticalGuildTestPacket(wire.OpGuildWar, other.ID, 2, 0)) }},
		{"cancel-direct", func() { w.cancelGuildWar(other.Session, &w.guilds.Guilds[1]) }},
		{"expel-direct", func() { w.expelGuildMember(other.Session, other, &w.guilds.Guilds[0], p) }},
	}
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, q, 1)
		}
		for _, call := range calls {
			t.Run(phase+"/"+call.name, func(t *testing.T) {
				for range 2 {
					call.run()
				}
				if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
					!reflect.DeepEqual(accountStateSnapshot(other.Account), before) || other.GuildInviteFrom != 1 ||
					!other.NextGuildInvite.IsZero() || fallback.gameSaves != 0 || len(base.entered) != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || other.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("guild command mutated retained context, saved or published before acknowledgement")
				}
			})
		}
	}
	w.processCriticalCompletions(1)
	if w.shuttingDown || w.criticalGuildReserved() || p.Char.Gold != wantGold {
		t.Fatal("competing guild commands invalidated the accepted payout")
	}
}

func TestCriticalGuildCreateDisconnectWaitsForFinalSave(t *testing.T) {
	w, p, fallback, base, q, release := criticalGuildCreateFixture(t)
	finalGate, releaseFinal := criticalGate(t)
	t.Cleanup(releaseFinal)
	q.store.(*criticalTreasuryTestStore).finalRelease = finalGate
	session, uid, key := p.Session, p.Char.UID, accountSessionKey(p.Account.Name)
	reconnecting := gameNet.NewTestSession(3, 8)
	before, retained := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
	w.guildCommandCreate(session, p, "Created")
	proposal := awaitCriticalWrite(t, base.entered)
	w.handle(command{s: session})
	w.handle(command{s: session})
	for _, phase := range []string{"held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, q, 1)
		}
		if !session.IsClosed() || w.players[session] != p || w.accountSessions[key] != session ||
			!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
			len(base.entered) != 0 || len(w.criticalDisconnects) != 1 || w.claimAccountSession(reconnecting, p.Account.Name) {
			t.Fatalf("%s creation disconnect lost ownership or saved uncommitted membership", phase)
		}
	}
	w.processCriticalCompletions(1)
	final := awaitCriticalWrite(t, base.entered)
	if final.character == nil || final.character.uid != uid || final.accounts[0] == p.Account ||
		final.accounts[0].Chars[p.CharSlot].GuildID != proposal.accounts[0].Chars[p.CharSlot].GuildID ||
		final.accounts[0].Chars[p.CharSlot].GuildRank != model.GuildRankLeader ||
		!reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) ||
		!reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
		t.Fatal("final account/sidecar save omitted acknowledged guild membership")
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
			t.Fatalf("%s creation final save repeated or released account ownership early", phase)
		}
	}
	w.processCriticalCompletions(1)
	if w.players[session] != nil || w.playersByCharacterUID[uid] != nil || w.accountSessions[key] != nil ||
		len(q.pending) != 0 || len(w.criticalDisconnects) != 0 || session.QueuedPacketsForTest() != 0 ||
		w.shuttingDown || !w.claimAccountSession(reconnecting, p.Account.Name) {
		t.Fatal("acknowledged creation final save did not release ownership exactly once")
	}
}

func TestCriticalGuildCreateActualWorldLoopProgress(t *testing.T) {
	w, p, unrelatedUID, base, q, ticks, release, stop := criticalEconomyLoopFixture(t, nil, func(w *World, p *Player) {
		w.warConfig.Enabled = false
		base := w.criticalWrites.queue.store.(*criticalTestStore)
		w.criticalWrites.queue.store = &criticalTreasuryTestStore{criticalGameStateTestStore: &criticalGameStateTestStore{base}}
	})
	packet := wire.MessageChat(p.ID, "/create Created")
	// Build leaves Size for transport encryption; direct actor delivery needs it.
	binary.LittleEndian.PutUint16(packet[0:2], uint16(len(packet)))
	deliver := func() {
		t.Helper()
		select {
		case w.commands <- command{s: p.Session, pkt: packet}:
		case <-time.After(time.Second):
			t.Fatal("World did not accept the guild creation intention")
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
			t.Fatal("held creation blocked unrelated ticks/commands or confirmed early")
		}
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() < 4 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	stop() // Join the actor before inspecting mutable registry and reservations.
	if p.Char.GuildID != 1 || p.Char.GuildRank != model.GuildRankLeader || len(w.guilds.Guilds) != 1 ||
		w.guilds.FindByName("Created") == nil || w.shuttingDown || len(q.pending) != 0 ||
		len(base.entered) != 0 || w.mobTickCounter < 3 || p.Session.QueuedPacketsForTest() != 4 {
		t.Fatal("actual World did not settle one creation while advancing unrelated ticks")
	}
}

func TestCriticalGuildCommandsPreserveReservedTargetAccount(t *testing.T) {
	for _, mode := range []string{"invite", "native-recruit", "expel", "native-expel", "promote", "succession"} {
		t.Run(mode, func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
			w.clock = newFakeClock(time.Date(2026, 10, 5, 12, 0, 0, 0, time.UTC))
			p.Char.Gold = guildSubInviteCost
			target := criticalPickupCompetitor(t, w, &GroundItem{X: p.X, Y: p.Y})
			if mode != "invite" && mode != "native-recruit" {
				w.guilds.Guilds[0].Members = append(w.guilds.Guilds[0].Members, model.GuildMember{
					Character: target.Char.Name, Account: target.Account.Name, Rank: model.GuildRankMember,
				})
				target.Char.GuildID, target.Char.GuildRank = 1, model.GuildRankMember
			}
			before, targetBefore, retained := accountStateSnapshot(p.Account), accountStateSnapshot(target.Account), w.criticalGuildSnapshot()
			registry := w.guilds
			if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{targetBefore}}, []*Player{target},
				func(criticalPersistenceResult) error { return nil }); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, base.entered)
			if w.criticalGuildReserved() || w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(target) {
				t.Fatal("target-only reservation fixture also reserved the actor or guild")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				for range 2 {
					switch mode {
					case "invite":
						w.guildCommandInvite(p.Session, p, target.Char.Name)
					case "native-recruit":
						w.onInviteGuild(p.Session, criticalGuildTestPacket(wire.OpInviteGuild, p.ID, target.ID, 0))
					case "expel":
						w.guildCommandExpel(p.Session, p, target.Char.Name)
					case "native-expel":
						w.onGuildDeprivate(p.Session, criticalGuildTestPacket(wire.OpGuildDeprivate, p.ID, target.ID, 0))
					case "promote":
						w.guildCommandSubLeader(p.Session, p, target.Char.Name)
					case "succession":
						w.guildCommandLeave(p.Session, p, "")
					}
				}
				if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(target.Account), targetBefore) ||
					target.GuildInviteFrom != 0 || !p.NextGuildInvite.IsZero() || fallback.gameSaves != 0 || len(base.entered) != 0 ||
					p.Session.QueuedPacketsForTest() != 0 || target.Session.QueuedPacketsForTest() != 0 {
					t.Fatalf("%s command mutated, saved or published a reserved target account", phase)
				}
			}
			w.processCriticalCompletions(1)
			if w.shuttingDown || len(q.pending) != 0 || w.criticalPlayerReserved(target) {
				t.Fatal("target command invalidated the accepted account write")
			}
		})
	}
}
