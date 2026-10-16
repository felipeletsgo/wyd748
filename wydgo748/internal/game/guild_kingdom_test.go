package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func configureGuildKingdom(w *World, leader, follower *Player) {
	w.clock = newFakeClock(time.Date(2026, 10, 15, 12, 0, 0, 0, time.UTC))
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{
		ID: 1, Name: "Realm", Kingdom: model.KingdomAkelonia, Members: []model.GuildMember{
			member(leader.Char.Name, leader.Account.Name, model.GuildRankLeader),
			member(follower.Char.Name, follower.Account.Name, model.GuildRankMember),
		},
	}}}
	for _, p := range []*Player{leader, follower} {
		p.Char.Score.Level = 354
		p.Char.GuildID = 1
		p.Char.GuildRank = model.GuildRankMember
		p.Char.Equip[model.CapeSlot] = model.Item{Index: 3192, UID: p.Char.Name + "-cape", Eff: [6]byte{43, 9}}
		p.Char.Inv[0] = model.Item{Index: model.SapphireItem, UID: p.Char.Name + "-sapphires", Eff: [6]byte{effectAmount, 40}}
		w.recalcPlayer(p.Char)
	}
	leader.Char.GuildRank = model.GuildRankLeader
}

func TestGuildKingdomMembersCannotChangeRealm(t *testing.T) {
	for _, rank := range []byte{model.GuildRankMember, model.GuildRankSubFirst} {
		for _, operation := range []string{"broker", "opposite", "white-to-king"} {
			t.Run(fmt.Sprintf("%d/%s", rank, operation), func(t *testing.T) {
				leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
				follower, _ := networkedTestPlayer(2, "Follower", 2101, 2100)
				w, st := guildFlowWorld(leader, follower)
				configureGuildKingdom(w, leader, follower)
				w.guilds.Guilds[0].Members[1].Rank = rank
				if operation == "white-to-king" {
					follower.Char.Equip[model.CapeSlot].Index = 3193
				}
				before := cloneCharacterState(follower.Char)
				if operation == "broker" {
					w.leaveKingdom(follower.Session, follower, w.now())
				} else {
					w.joinKingdom(follower.Session, follower, model.KingdomHekalotia)
				}
				if !reflect.DeepEqual(*follower.Char, before) || st.gameSaves != 0 || st.saves != 0 {
					t.Fatal("guild member changed kingdom or paid for a rejected change")
				}
			})
		}
	}
}

func TestGuildKingdomLoginRepairPreservesCape(t *testing.T) {
	for _, kingdom := range []byte{model.KingdomNeutral, model.KingdomHekalotia, model.KingdomAkelonia} {
		for tier := model.CapeTierBasic; tier <= model.CapeTierMaster; tier++ {
			t.Run(fmt.Sprintf("%d/%d", kingdom, tier), func(t *testing.T) {
				guild := guildWithMembers(1, "Realm", member("Offline", "Account", model.GuildRankMember))
				guild.Kingdom = kingdom
				w := guildTestWorld(guild)
				index, _ := model.KingdomCapeAtTier(tier, model.KingdomAkelonia)
				ch := model.Char{Name: "Offline", GuildID: 1, GuildRank: model.GuildRankMember}
				ch.Equip[model.CapeSlot] = model.Item{Index: index, UID: "offline-cape", Eff: [6]byte{43, 9, 116, 3}}
				want := ch.Equip[model.CapeSlot]
				want.Index, _ = model.KingdomCapeAtTier(tier, kingdom)
				if changed := w.repairGuildState(&ch); changed != (index != want.Index) || ch.Equip[model.CapeSlot] != want {
					t.Fatal("login did not restore guild color while preserving cape identity/effects/tier")
				}
				if w.repairGuildState(&ch) {
					t.Fatal("guild color repair is not idempotent")
				}
			})
		}
	}
}

func TestGuildKingdomRecruitmentRequiresCanonicalRealm(t *testing.T) {
	for _, route := range []string{"native", "invite", "accept"} {
		for _, kingdom := range []byte{model.KingdomNeutral, model.KingdomHekalotia, model.KingdomAkelonia} {
			t.Run(fmt.Sprintf("%s/%d", route, kingdom), func(t *testing.T) {
				leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
				target, _ := networkedTestPlayer(2, "Recruit", 2101, 2100)
				w, st := guildFlowWorld(leader, target)
				configureGuildKingdom(w, leader, target)
				w.guilds.Guilds[0].Members = w.guilds.Guilds[0].Members[:1]
				w.guilds.Guilds[0].Kingdom = kingdom
				target.Char.GuildID, target.Char.GuildRank = 0, 0
				// A stale recruiter with the same wrong color must not authorize a recruit.
				wrong := model.KingdomHekalotia
				if kingdom == wrong {
					wrong = model.KingdomAkelonia
				}
				index, _ := model.KingdomCapeAtTier(model.CapeTierElite, wrong)
				leader.Char.Equip[model.CapeSlot].Index, target.Char.Equip[model.CapeSlot].Index = index, index
				leader.Char.Gold = guildInviteCost
				switch route {
				case "native":
					w.onInviteGuild(leader.Session, criticalGuildTestPacket(wire.OpInviteGuild, leader.ID, target.ID, 0))
				case "invite":
					w.guildCommandInvite(leader.Session, leader, target.Char.Name)
				case "accept":
					target.GuildInviteFrom, target.GuildInviteUntil = 1, w.now().Add(time.Minute)
					w.guildCommandAccept(target.Session, target, "")
				}
				if len(w.guilds.Guilds[0].Members) != 1 || target.Char.GuildID != 0 || target.GuildInviteFrom != 0 || st.gameSaves != 0 || leader.Char.Gold != guildInviteCost {
					t.Fatal("mismatched kingdom recruited or charged a player")
				}
			})
		}
	}
}

func TestGuildKingdomLeaderLegacyPropagationAndRollback(t *testing.T) {
	for _, fail := range []bool{false, true} {
		t.Run(fmt.Sprint(fail), func(t *testing.T) {
			leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
			follower, _ := networkedTestPlayer(2, "Follower", 2101, 2100)
			w, st := guildFlowWorld(leader, follower)
			configureGuildKingdom(w, leader, follower)
			beforeLeader, beforeFollower := cloneCharacterState(leader.Char), cloneCharacterState(follower.Char)
			if fail {
				st.err = errors.New("injected guild write rejection")
			}
			w.leaveKingdom(leader.Session, leader, w.now())
			if fail {
				if !reflect.DeepEqual(*leader.Char, beforeLeader) || !reflect.DeepEqual(*follower.Char, beforeFollower) || w.guilds.Guilds[0].Kingdom != model.KingdomAkelonia || follower.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("failed guild change mutated or published a member's cape")
				}
				return
			}
			if w.guilds.Guilds[0].Kingdom != model.KingdomNeutral || leader.Char.Equip[15].Index != 3193 || follower.Char.Equip[15].Index != 3193 || sapphireCount(leader.Char) != 24 || sapphireCount(follower.Char) != 40 {
				t.Fatal("leader's white cape was not propagated with one payment")
			}
			w.joinKingdom(leader.Session, leader, model.KingdomHekalotia)
			want := beforeFollower.Equip[15]
			want.Index = 3191
			if w.guilds.Guilds[0].Kingdom != model.KingdomHekalotia || follower.Char.Equip[15] != want || leader.Char.Equip[15].Index != 3191 || st.gameSaves != 2 || st.durable.Guilds[0].Kingdom != model.KingdomHekalotia {
				t.Fatal("leader's new kingdom was not durably propagated")
			}
		})
	}
}

func TestCriticalGuildKingdomAtomicPropagation(t *testing.T) {
	for _, outcome := range []string{"commit", "reject", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, leader, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
			follower := criticalGuildJoinRegister(t, w, 2, "Follower", leader)
			configureGuildKingdom(w, leader, follower)
			peer := criticalGuildJoinRegister(t, w, 3, "TradePeer", follower)
			follower.Trade = &TradeState{OpponentID: peer.ID, Checked: true}
			peer.Trade = &TradeState{OpponentID: follower.ID, Checked: true}
			beforeLeader, beforeFollower := accountStateSnapshot(leader.Account), accountStateSnapshot(follower.Account)
			if outcome == "reject" {
				base.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				base.failure = errors.New("lost guild kingdom commit acknowledgement")
			}
			w.leaveKingdom(leader.Session, leader, w.now())
			proposal := awaitCriticalWrite(t, base.entered)
			if len(proposal.accounts) != 3 || proposal.guilds.Guilds[0].Kingdom != model.KingdomNeutral || !w.criticalGuildReserved() || !w.criticalPlayerReserved(follower) || !w.criticalPlayerReserved(peer) {
				t.Fatal("guild, changed accounts and reciprocal trade peer are not reserved atomically")
			}
			for _, account := range proposal.accounts {
				if account.Name == follower.Account.Name && account.Chars[0].Equip[15].Index != 3193 {
					t.Fatal("detached transaction omitted the follower's cape")
				}
			}
			w.leaveKingdom(leader.Session, leader, w.now())
			w.leaveKingdom(follower.Session, follower, w.now())
			if !reflect.DeepEqual(accountStateSnapshot(leader.Account), beforeLeader) || !reflect.DeepEqual(accountStateSnapshot(follower.Account), beforeFollower) || leader.Session.QueuedPacketsForTest() != 0 || follower.Session.QueuedPacketsForTest() != 0 || len(base.entered) != 0 {
				t.Fatal("held guild change mutated, published or replayed before acknowledgement")
			}
			release()
			result := awaitCriticalResult(t, q)
			w.applyCriticalCompletion(result)
			if outcome != "commit" {
				if !reflect.DeepEqual(accountStateSnapshot(leader.Account), beforeLeader) || !reflect.DeepEqual(accountStateSnapshot(follower.Account), beforeFollower) || w.guilds.Guilds[0].Kingdom != model.KingdomAkelonia || follower.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejection/unknown outcome installed or published a cape")
				}
				if w.shuttingDown != (outcome == "unknown") || w.criticalGuildReserved() != (outcome == "unknown") {
					t.Fatal("guild change did not distinguish certified rejection from unknown commit")
				}
			} else {
				if w.shuttingDown || w.criticalGuildReserved() || w.criticalPlayerReserved(peer) || follower.Char.Equip[15].Index != 3193 || sapphireCount(leader.Char) != 24 || sapphireCount(follower.Char) != 40 {
					t.Fatal("committed guild change was not installed once for all members")
				}
				want := beforeFollower.Chars[0].Equip[15]
				want.Index = 3193
				if follower.Char.Equip[15] != want || follower.Session.QueuedPacketsForTest() == 0 {
					t.Fatal("committed cape lost identity/effects or was not published")
				}
				w.applyCriticalCompletion(result)
				if sapphireCount(leader.Char) != 24 || len(base.entered) != 0 {
					t.Fatal("duplicate completion charged the kingdom payment twice")
				}
			}
			if fallback.gameSaves != 0 || fallback.saves != 0 {
				t.Fatal("critical kingdom change fell back to synchronous persistence")
			}
		})
	}
}
