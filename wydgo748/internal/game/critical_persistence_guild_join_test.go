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

type criticalGuildJoinFixtureState struct {
	w             *World
	actor, target *Player
	fallback      *guildFlowStore
	base          *criticalTestStore
	q             *criticalPersistence
	release       func()
	acceptance    bool
	inviteType    uint16
}

func criticalGuildJoinFixture(t *testing.T, acceptance bool) criticalGuildJoinFixtureState {
	t.Helper()
	w, actor, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
	w.clock = newFakeClock(time.Date(2026, 10, 5, 12, 0, 0, 0, time.UTC))
	w.warConfig.Enabled = false
	actor.Char.Gold = guildSubInviteCost + 100
	target := criticalPickupCompetitor(t, w, &GroundItem{X: actor.X, Y: actor.Y})
	target.GuildInviteFrom, target.GuildInviteUntil = 1, w.now().Add(guildInviteTTL)
	return criticalGuildJoinFixtureState{w, actor, target, fallback, base, q, release, acceptance, 0}
}

func (f criticalGuildJoinFixtureState) request() {
	if f.acceptance {
		f.w.guildCommandAccept(f.target.Session, f.target, "")
	} else {
		f.w.onInviteGuild(f.actor.Session, criticalGuildTestPacket(wire.OpInviteGuild, f.actor.ID, f.target.ID, f.inviteType))
	}
}

func (f criticalGuildJoinFixtureState) initiator() *Player {
	if f.acceptance {
		return f.target
	}
	return f.actor
}

func (f criticalGuildJoinFixtureState) cost() uint32 {
	if f.acceptance {
		return 0
	}
	if f.inviteType != 0 {
		return guildSubInviteCost
	}
	return guildInviteCost
}

func criticalGuildJoinRegister(t *testing.T, w *World, id uint16, name string, anchor *Player) *Player {
	t.Helper()
	p, _ := networkedTestPlayer(id, name, anchor.X, anchor.Y)
	w.players[p.Session], w.playersByID[p.ID] = p, p
	w.indexPlayerCharacter(p)
	w.updatePlayerSpatial(p)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("join fixture account admission failed")
	}
	return p
}

func TestCriticalGuildJoinDetachedCommit(t *testing.T) {
	for _, acceptance := range []bool{true, false} {
		for _, trading := range []bool{false, true} {
			for inviteType := uint16(0); inviteType < 4; inviteType++ {
				if acceptance && inviteType != 0 {
					continue
				}
				t.Run(fmt.Sprintf("accept-%t/trade-%t/type-%d", acceptance, trading, inviteType), func(t *testing.T) {
					f := criticalGuildJoinFixture(t, acceptance)
					f.inviteType = inviteType
					if trading {
						f.actor.Trade = &TradeState{OpponentID: f.target.ID, Gold: 12, Checked: true, ExpiresAt: f.w.now().Add(time.Minute)}
						f.target.Trade = &TradeState{OpponentID: f.actor.ID, Gold: 34, Checked: true, ExpiresAt: f.w.now().Add(time.Minute)}
					}
					actorBefore, targetBefore, retained := accountStateSnapshot(f.actor.Account), accountStateSnapshot(f.target.Account), f.w.criticalGuildSnapshot()
					registry, trade, targetTrade := f.w.guilds, f.actor.Trade, f.target.Trade
					f.request()
					proposal := awaitCriticalWrite(t, f.base.entered)
					wantCount := 2
					if acceptance && !trading {
						wantCount = 1
					}
					if len(proposal.accounts) != wantCount || proposal.guilds == f.w.guilds || len(proposal.guilds.Guilds[0].Members) != 2 {
						t.Fatal("join did not detach and deduplicate the complete guild/account proposal")
					}
					for _, proposed := range proposal.accounts {
						want := accountStateSnapshot(actorBefore)
						if proposed.Name == f.target.Account.Name {
							want = accountStateSnapshot(targetBefore)
							want.Chars[f.target.CharSlot].GuildID, want.Chars[f.target.CharSlot].GuildRank = 1, model.GuildRankMember
						} else {
							want.Chars[f.actor.CharSlot].Gold -= f.cost()
						}
						if proposed == f.actor.Account || proposed == f.target.Account || !reflect.DeepEqual(proposed, accountPersistenceSnapshot(want)) {
							t.Fatal("join proposal changed unrelated fields or omitted payment/membership")
						}
					}
					for _, phase := range []string{"held", "completed-unacknowledged"} {
						if phase == "completed-unacknowledged" {
							f.release()
							awaitReadyCriticalResults(t, f.q, 1)
						}
						f.request()
						f.w.guildCommandInvite(f.actor.Session, f.actor, f.target.Char.Name)
						f.w.guildCommandLeave(f.actor.Session, f.actor, "")
						f.w.cancelTrade(f.target, "competing cancellation")
						if !reflect.DeepEqual(accountStateSnapshot(f.actor.Account), actorBefore) || !reflect.DeepEqual(accountStateSnapshot(f.target.Account), targetBefore) ||
							!reflect.DeepEqual(f.w.criticalGuildSnapshot(), retained) || f.w.guilds != registry || f.actor.Trade != trade || f.target.Trade != targetTrade ||
							f.target.GuildInviteFrom != 1 || !f.w.criticalGuildReserved() || !f.w.criticalPlayerReserved(f.target) ||
							f.fallback.gameSaves != 0 || len(f.base.entered) != 0 || f.actor.Session.QueuedPacketsForTest() != 0 || f.target.Session.QueuedPacketsForTest() != 0 {
							t.Fatalf("%s join mutated, published, repeated or released retained state", phase)
						}
						if (!acceptance || trading) && !f.w.criticalPlayerReserved(f.actor) {
							t.Fatal("join omitted the actor/trade account reservation")
						}
					}
					f.w.processCriticalCompletions(1)
					wantTarget := accountStateSnapshot(targetBefore)
					wantTarget.Chars[f.target.CharSlot].GuildID, wantTarget.Chars[f.target.CharSlot].GuildRank = 1, model.GuildRankMember
					wantActor := accountStateSnapshot(actorBefore)
					wantActor.Chars[f.actor.CharSlot].Gold -= f.cost()
					if f.w.shuttingDown || f.w.guilds != registry || !reflect.DeepEqual(f.w.criticalGuildSnapshot(), proposal.guilds) ||
						!reflect.DeepEqual(accountStateSnapshot(f.actor.Account), wantActor) || !reflect.DeepEqual(accountStateSnapshot(f.target.Account), wantTarget) ||
						f.w.criticalGuildReserved() || len(f.q.pending) != 0 || f.target.Session.QueuedPacketsForTest() != 4 ||
						(acceptance && f.target.GuildInviteFrom != 0) || (!acceptance && f.target.GuildInviteFrom != 1) {
						t.Fatal("acknowledged join did not apply exactly the private payment/membership delta")
					}
					assertCriticalPickupPacket(t, f.target, wire.CreateMobWithGuildRank(f.target.ID, f.target.Char.Name, f.target.X, f.target.Y,
						bodyMesh(f.target.Char), bodyAncient(f.target.Char), wireScoreState(f.target.Char), f.target.Char.Affects[:], 2, 1, model.GuildRankMember, f.target.Char.CP, f.target.Char.Citizenship))
					assertCriticalPickupPacket(t, f.target, playerScorePacket(f.target))
					assertCriticalPickupPacket(t, f.target, wire.UpdateEtc(f.target.ID, *f.target.Char))
					assertCriticalPickupPacket(t, f.target, wire.MessagePanel("You joined the guild Treasury."))
					if !acceptance {
						assertCriticalPickupPacket(t, f.actor, wire.UpdateEtc(f.actor.ID, *f.actor.Char))
						assertCriticalPickupPacket(t, f.actor, wire.MessagePanel("Competitor joined the guild."))
					}
					assertCriticalPickupPacket(t, f.actor, wire.MessagePanel("Competitor joined the guild."))
				})
			}
		}
	}
}

func TestCriticalGuildJoinFourAccountTradeClosure(t *testing.T) {
	f := criticalGuildJoinFixture(t, false)
	peer1 := criticalGuildJoinRegister(t, f.w, 3, "RecruiterPeer", f.actor)
	peer2 := criticalGuildJoinRegister(t, f.w, 4, "RecruitPeer", f.target)
	for _, pair := range [][2]*Player{{f.actor, peer1}, {f.target, peer2}} {
		pair[0].Trade = &TradeState{OpponentID: pair[1].ID, Checked: true}
		pair[1].Trade = &TradeState{OpponentID: pair[0].ID, Checked: true}
	}
	f.request()
	proposal := awaitCriticalWrite(t, f.base.entered)
	if len(proposal.accounts) != 4 {
		t.Fatal("recruitment did not reserve both reciprocal trade closures")
	}
	for i, p := range []*Player{f.actor, f.target, peer1, peer2} {
		if !f.w.criticalPlayerReserved(p) || proposal.accounts[i].Name != p.Account.Name {
			t.Fatal("trade closure participant ordering/reservation mismatch")
		}
		if i >= 2 && !reflect.DeepEqual(proposal.accounts[i], accountPersistenceSnapshot(p.Account)) {
			t.Fatal("unchanged trade peer was mutated")
		}
	}
	f.release()
	f.w.applyCriticalCompletion(awaitCriticalResult(t, f.q))
	if f.w.shuttingDown || f.target.Char.GuildID != 1 || f.w.criticalPlayerReserved(peer1) || f.w.criticalPlayerReserved(peer2) {
		t.Fatal("trade closure did not settle together")
	}
}

func TestCriticalGuildJoinCertifiedRejectionAndRetry(t *testing.T) {
	for _, acceptance := range []bool{true, false} {
		t.Run(fmt.Sprint(acceptance), func(t *testing.T) {
			f := criticalGuildJoinFixture(t, acceptance)
			actorBefore, targetBefore, retained := accountStateSnapshot(f.actor.Account), accountStateSnapshot(f.target.Account), f.w.criticalGuildSnapshot()
			f.base.failure = criticalCertifiedRejection(t)
			f.request()
			awaitCriticalWrite(t, f.base.entered)
			f.release()
			result := awaitCriticalResult(t, f.q)
			f.base.failure = nil
			f.w.applyCriticalCompletion(result)
			message := "Save failed. The recruitment was not applied."
			if acceptance {
				message = "Save failed. You did not join the guild."
			}
			assertCriticalPickupPacket(t, f.initiator(), wire.MessagePanel(message))
			if f.w.shuttingDown || len(f.q.pending) != 0 || f.w.criticalGuildReserved() || f.target.GuildInviteFrom != 1 ||
				!reflect.DeepEqual(accountStateSnapshot(f.actor.Account), actorBefore) || !reflect.DeepEqual(accountStateSnapshot(f.target.Account), targetBefore) ||
				!reflect.DeepEqual(f.w.criticalGuildSnapshot(), retained) || len(f.base.entered) != 0 || f.fallback.gameSaves != 0 {
				t.Fatal("certified join rejection mutated or replayed")
			}
			f.w.clock = newFakeClock(f.w.now().Add(guildInviteCooldown))
			f.request()
			awaitCriticalWrite(t, f.base.entered)
			f.w.applyCriticalCompletion(awaitCriticalResult(t, f.q))
			if f.w.shuttingDown || len(f.q.pending) != 0 || len(f.w.guilds.Guilds[0].Members) != 2 || f.target.Char.GuildID != 1 || f.actor.Char.Gold != actorBefore.Chars[0].Gold-f.cost() {
				t.Fatal("explicit join retry charged or enrolled more than once")
			}
		})
	}
}

func TestCriticalGuildJoinUnknownAndStaleRetainFences(t *testing.T) {
	for _, acceptance := range []bool{true, false} {
		for _, mode := range []string{"unknown", "panic", "account", "registry", "invitation", "cooldown", "trade", "lifecycle"} {
			t.Run(fmt.Sprintf("accept-%t/%s", acceptance, mode), func(t *testing.T) {
				f := criticalGuildJoinFixture(t, acceptance)
				f.actor.Trade, f.target.Trade = &TradeState{OpponentID: f.target.ID}, &TradeState{OpponentID: f.actor.ID}
				if mode == "unknown" {
					f.base.failure = errors.New("lost join commit acknowledgement")
				}
				if mode == "panic" {
					f.base.panicAfterWrite = true
				}
				f.request()
				awaitCriticalWrite(t, f.base.entered)
				switch mode {
				case "account":
					f.target.Account.CargoGold++
				case "registry":
					f.w.guilds = f.w.criticalGuildSnapshot()
				case "invitation":
					f.target.GuildInviteUntil = f.target.GuildInviteUntil.Add(time.Second)
				case "cooldown":
					f.initiator().NextGuildInvite = f.initiator().NextGuildInvite.Add(time.Second)
				case "trade":
					f.actor.Trade.Gold++
				case "lifecycle":
					f.target.criticalLifecycle = &criticalPlayerLifecycle{}
				}
				before, retained := accountStateSnapshot(f.target.Account), f.w.criticalGuildSnapshot()
				f.release()
				f.w.applyCriticalCompletion(awaitCriticalResult(t, f.q))
				if !f.w.shuttingDown || !f.actor.PersistencePoisoned || !f.target.PersistencePoisoned || !f.w.criticalGuildReserved() ||
					len(f.q.pending) != 1 || len(f.base.entered) != 0 || f.actor.Session.QueuedPacketsForTest() != 0 || f.target.Session.QueuedPacketsForTest() != 0 ||
					!reflect.DeepEqual(accountStateSnapshot(f.target.Account), before) || !reflect.DeepEqual(f.w.criticalGuildSnapshot(), retained) {
					t.Fatal("unknown/stale join published, replayed or dropped retained fences")
				}
			})
		}
	}
}

func TestCriticalGuildJoinAdmissionDoesNotWrite(t *testing.T) {
	for _, acceptance := range []bool{true, false} {
		for _, mode := range []string{"full", "stopping", "maintenance", "unsupported", "closed-target", "identity", "closed-peer", "nonreciprocal"} {
			t.Run(fmt.Sprintf("accept-%t/%s", acceptance, mode), func(t *testing.T) {
				f := criticalGuildJoinFixture(t, acceptance)
				switch mode {
				case "full":
					f.q.capacity = 0
				case "stopping":
					f.q.closeAdmission()
				case "maintenance":
					f.w.shuttingDown = true
				case "unsupported":
					f.q.store = f.base
				case "closed-target":
					f.target.Session.Close()
				case "identity":
					delete(f.w.playersByID, f.target.ID)
				case "closed-peer", "nonreciprocal":
					peer := criticalGuildJoinRegister(t, f.w, 3, "InvalidPeer", f.target)
					f.target.Trade = &TradeState{OpponentID: peer.ID}
					peer.Trade = &TradeState{OpponentID: f.target.ID}
					if mode == "closed-peer" {
						peer.Session.Close()
					} else {
						peer.Trade.OpponentID = 99
					}
				}
				actorBefore, targetBefore, retained := accountStateSnapshot(f.actor.Account), accountStateSnapshot(f.target.Account), f.w.criticalGuildSnapshot()
				actor, cost := f.actor, f.cost()
				if acceptance {
					actor = f.target
				}
				if err := f.w.queueGuildJoin(actor, f.target, &f.w.guilds.Guilds[0], cost, 0, acceptance); err == nil {
					t.Fatal("invalid join admission succeeded")
				}
				if len(f.q.pending) != 0 || len(f.base.entered) != 0 || f.w.criticalGuildReserved() || f.target.GuildInviteFrom != 1 ||
					!reflect.DeepEqual(accountStateSnapshot(f.actor.Account), actorBefore) || !reflect.DeepEqual(accountStateSnapshot(f.target.Account), targetBefore) ||
					!reflect.DeepEqual(f.w.criticalGuildSnapshot(), retained) || f.fallback.gameSaves != 0 {
					t.Fatal("failed join admission wrote or changed retained state")
				}
			})
		}
	}
}

func TestCriticalGuildJoinDisconnectWaitsForFinalSave(t *testing.T) {
	for _, acceptance := range []bool{true, false} {
		for _, disconnectActor := range []bool{false, true} {
			if acceptance && disconnectActor {
				continue
			}
			t.Run(fmt.Sprintf("accept-%t/actor-%t", acceptance, disconnectActor), func(t *testing.T) {
				f := criticalGuildJoinFixture(t, acceptance)
				p := f.target
				if disconnectActor {
					p = f.actor
				}
				finalGate, releaseFinal := criticalGate(t)
				t.Cleanup(releaseFinal)
				f.q.store.(*criticalTreasuryTestStore).finalRelease = finalGate
				before, retained := accountStateSnapshot(p.Account), f.w.criticalGuildSnapshot()
				session, uid, key := p.Session, p.Char.UID, accountSessionKey(p.Account.Name)
				reconnecting := gameNet.NewTestSession(33, 8)
				f.request()
				awaitCriticalWrite(t, f.base.entered)
				f.w.handle(command{s: session})
				f.w.handle(command{s: session})
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						f.release()
						awaitReadyCriticalResults(t, f.q, 1)
					}
					if !session.IsClosed() || f.w.players[session] != p || f.w.accountSessions[key] != session || f.w.claimAccountSession(reconnecting, p.Account.Name) ||
						!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(f.w.criticalGuildSnapshot(), retained) || len(f.base.entered) != 0 {
						t.Fatalf("%s join disconnect saved or released state early", phase)
					}
				}
				f.w.processCriticalCompletions(1)
				final := awaitCriticalWrite(t, f.base.entered)
				if final.character == nil || final.character.uid != uid || !reflect.DeepEqual(final.accounts[0], accountPersistenceSnapshot(p.Account)) ||
					!reflect.DeepEqual(final.character.state, buildCharState(p, f.w.now())) || f.target.Char.GuildID != 1 || f.actor.Char.Gold != guildSubInviteCost+100-f.cost() {
					t.Fatal("disconnect final save omitted acknowledged join/payment")
				}
				f.w.handle(command{s: session})
				if f.w.accountSessions[key] != session || f.w.claimAccountSession(reconnecting, p.Account.Name) || len(f.base.entered) != 0 {
					t.Fatal("final held join save released ownership or repeated")
				}
				releaseFinal()
				awaitReadyCriticalResults(t, f.q, 1)
				if f.w.accountSessions[key] != session {
					t.Fatal("final worker completion released ownership before acknowledgement")
				}
				f.w.processCriticalCompletions(1)
				if f.w.shuttingDown || f.w.players[session] != nil || f.w.playersByCharacterUID[uid] != nil || f.w.accountSessions[key] != nil ||
					len(f.q.pending) != 0 || len(f.w.criticalDisconnects) != 0 || session.QueuedPacketsForTest() != 0 || f.fallback.gameSaves != 0 ||
					!f.w.claimAccountSession(reconnecting, p.Account.Name) {
					t.Fatal("join disconnect did not settle and release exactly once")
				}
			})
		}
	}
}

func TestCriticalGuildJoinActualWorldLoopProgress(t *testing.T) {
	for _, acceptance := range []bool{true, false} {
		t.Run(fmt.Sprint(acceptance), func(t *testing.T) {
			var target *Player
			w, p, unrelatedUID, base, q, ticks, release, stop := criticalEconomyLoopFixture(t, nil, func(w *World, p *Player) {
				w.clock = newFakeClock(time.Date(2026, 10, 5, 12, 0, 0, 0, time.UTC))
				// Reset the due regeneration time after replacing the fixture clock.
				w.playerByID(2).NextRegen = w.now().Add(-time.Second)
				w.warConfig.Enabled = false
				base := w.criticalWrites.queue.store.(*criticalTestStore)
				w.criticalWrites.queue.store = &criticalTreasuryTestStore{criticalGameStateTestStore: &criticalGameStateTestStore{base}}
				configureCriticalTreasury(w, p, 100)
				p.Char.Gold = guildInviteCost + 100
				target = criticalGuildJoinRegister(t, w, 3, "LoopRecruit", p)
				target.NextRegen, target.NextCPRecovery = w.now().Add(time.Hour), w.now().Add(time.Hour)
				target.GuildInviteFrom, target.GuildInviteUntil = 1, w.now().Add(guildInviteTTL)
			})
			packet, session := criticalGuildTestPacket(wire.OpInviteGuild, p.ID, target.ID, 0), p.Session
			if acceptance {
				packet, session = wire.MessageChat(target.ID, "/accept"), target.Session
				binary.LittleEndian.PutUint16(packet[0:2], uint16(len(packet)))
			}
			deliver := func() {
				t.Helper()
				select {
				case w.commands <- command{s: session, pkt: packet}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept join intention")
				}
			}
			deliver()
			awaitCriticalWrite(t, base.entered)
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
				if overview.State != "running" || !regenerated || target.Session.QueuedPacketsForTest() != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held join blocked unrelated ticks/commands or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for target.Session.QueuedPacketsForTest() < 4 && time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
			}
			stop()
			cost := guildInviteCost
			if acceptance {
				cost = 0
			}
			if w.shuttingDown || target.Char.GuildID != 1 || p.Char.Gold != guildInviteCost+100-cost || len(w.guilds.Guilds[0].Members) != 2 ||
				len(q.pending) != 0 || len(base.entered) != 0 || w.mobTickCounter < 3 {
				t.Fatal("actual World did not settle one join while advancing unrelated ticks")
			}
		})
	}
}
