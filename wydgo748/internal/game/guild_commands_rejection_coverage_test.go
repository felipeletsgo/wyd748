package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func addGuildCoverageLeader(w *World, leader *Player, id uint16, name string, kingdom byte, rank byte) *model.Guild {
	guild := model.Guild{
		ID: id, Name: name, Kingdom: kingdom,
		Members: []model.GuildMember{{
			Character: leader.Char.Name,
			Account:   leader.Account.Name,
			Rank:      rank,
		}},
	}
	w.guilds.Guilds = append(w.guilds.Guilds, guild)
	leader.Char.GuildID, leader.Char.GuildRank = id, rank
	return &w.guilds.Guilds[len(w.guilds.Guilds)-1]
}

func TestGuildCreateRejectsInvalidDuplicateExistingMembershipAndFullRegistry(t *testing.T) {
	tests := []struct {
		name      string
		guildName string
		setup     func(*World, *Player)
	}{
		{name: "invalid name", guildName: "bad!"},
		{name: "already belongs to a guild", guildName: "NewGuild", setup: func(w *World, p *Player) {
			addGuildCoverageLeader(w, p, 1, "Existing", model.KingdomNeutral, model.GuildRankLeader)
		}},
		{name: "duplicate name", guildName: "Existing", setup: func(w *World, _ *Player) {
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 1, Name: "Existing"})
		}},
		{name: "all guild slots occupied", guildName: "NewGuild", setup: func(w *World, _ *Player) {
			for id := uint16(model.MinGuildID); id <= model.MaxGuildID; id++ {
				w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: id, Name: "Existing"})
			}
		}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
			w, st := guildFlowWorld(leader)
			if tc.setup != nil {
				tc.setup(w, leader)
			}
			beforeID, beforeRank, beforeGuilds := leader.Char.GuildID, leader.Char.GuildRank, len(w.guilds.Guilds)

			w.guildCommandCreate(leader.Session, leader, tc.guildName)

			if leader.Char.GuildID != beforeID || leader.Char.GuildRank != beforeRank ||
				len(w.guilds.Guilds) != beforeGuilds || st.gameSaves != 0 {
				t.Fatalf("rejected creation changed state: char=%d/%d guilds=%d saves=%d",
					leader.Char.GuildID, leader.Char.GuildRank, len(w.guilds.Guilds), st.gameSaves)
			}
		})
	}
}

func TestGuildInviteRejectsInvalidActorAndTarget(t *testing.T) {
	tests := []struct {
		name        string
		argument    string
		actorGuild  bool
		actorRank   byte
		target      bool
		targetCape  uint16
		targetGuild bool
	}{
		{name: "empty target", actorGuild: true, actorRank: model.GuildRankLeader},
		{name: "actor is unaffiliated", argument: "Member", target: true},
		{name: "actor is not leader", argument: "Member", actorGuild: true, actorRank: model.GuildRankMember, target: true},
		{name: "actor invites self", argument: "Leader", actorGuild: true, actorRank: model.GuildRankLeader},
		{name: "target is offline", argument: "Offline", actorGuild: true, actorRank: model.GuildRankLeader},
		{name: "target already belongs to a guild", argument: "Member", actorGuild: true, actorRank: model.GuildRankLeader, target: true, targetGuild: true},
		{name: "target belongs to another kingdom", argument: "Member", actorGuild: true, actorRank: model.GuildRankLeader, target: true, targetCape: 544},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
			players := []*Player{leader}
			var target *Player
			if tc.target {
				target, _ = networkedTestPlayer(2, "Member", 2101, 2100)
				if tc.targetCape != 0 {
					target.Char.Equip[model.CapeSlot].Index = tc.targetCape
				}
				players = append(players, target)
			}
			w, st := guildFlowWorld(players...)
			if tc.actorGuild {
				addGuildCoverageLeader(w, leader, 1, "Alpha", model.KingdomHekalotia, tc.actorRank)
			}
			if tc.targetGuild {
				guild := model.Guild{ID: 2, Name: "Beta", Kingdom: model.KingdomHekalotia, Members: []model.GuildMember{{
					Character: target.Char.Name, Account: target.Account.Name, Rank: model.GuildRankLeader,
				}}}
				w.guilds.Guilds = append(w.guilds.Guilds, guild)
				target.Char.GuildID, target.Char.GuildRank = guild.ID, model.GuildRankLeader
			}

			w.guildCommandInvite(leader.Session, leader, tc.argument)

			if target != nil && (target.GuildInviteFrom != 0 || !target.GuildInviteUntil.IsZero()) {
				t.Fatalf("rejected invite was recorded: %d until %v", target.GuildInviteFrom, target.GuildInviteUntil)
			}
			if st.gameSaves != 0 {
				t.Fatalf("invite unexpectedly persisted guild state %d times", st.gameSaves)
			}
		})
	}
}

func TestGuildAcceptRejectsInvalidInvitationAndRollsBackPersistenceFailure(t *testing.T) {
	now := time.Date(2026, 11, 11, 12, 0, 0, 0, time.UTC)
	tests := []struct {
		name         string
		inviteFrom   uint16
		inviteAge    time.Duration
		alreadyGuild bool
		guildExists  bool
		guildKingdom byte
		storeErr     error
		wantInvite   uint16
	}{
		{name: "no invitation"},
		{name: "expired invitation", inviteFrom: 1, inviteAge: -time.Second},
		{name: "already belongs to a guild", inviteFrom: 1, alreadyGuild: true},
		{name: "guild no longer exists", inviteFrom: 1},
		{name: "wrong kingdom", inviteFrom: 1, guildExists: true, guildKingdom: model.KingdomAkelonia},
		{name: "store failure restores membership", inviteFrom: 1, guildExists: true, guildKingdom: model.KingdomNeutral, storeErr: errors.New("database unavailable"), wantInvite: 1},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
			w, st := guildFlowWorld(member)
			w.clock = newFakeClock(now)
			st.err = tc.storeErr
			if tc.guildExists {
				w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{
					ID: 1, Name: "Alpha", Kingdom: tc.guildKingdom,
					Members: []model.GuildMember{{Character: "Leader", Account: "LeaderAccount", Rank: model.GuildRankLeader}},
				})
			}
			if tc.alreadyGuild {
				w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{
					ID: 2, Name: "Other", Kingdom: model.KingdomNeutral,
					Members: []model.GuildMember{{Character: member.Char.Name, Account: member.Account.Name, Rank: model.GuildRankMember}},
				})
				member.Char.GuildID, member.Char.GuildRank = 2, model.GuildRankMember
			}
			if tc.inviteFrom != 0 {
				member.GuildInviteFrom = tc.inviteFrom
				member.GuildInviteUntil = now.Add(tc.inviteAge)
			}
			beforeGuilds := len(w.guilds.Guilds)

			w.guildCommandAccept(member.Session, member, "")

			if member.GuildInviteFrom != tc.wantInvite {
				t.Fatalf("invitation after result = %d, want %d", member.GuildInviteFrom, tc.wantInvite)
			}
			wantGuildID := uint16(0)
			if tc.alreadyGuild {
				wantGuildID = 2
			}
			if member.Char.GuildID != wantGuildID || len(w.guilds.Guilds) != beforeGuilds {
				t.Fatalf("rejected accept changed membership: character=%d guilds=%d/%d",
					member.Char.GuildID, len(w.guilds.Guilds), beforeGuilds)
			}
			wantSaves := 0
			if tc.storeErr != nil {
				wantSaves = 1
			}
			if st.gameSaves != wantSaves {
				t.Fatalf("save calls = %d, want %d", st.gameSaves, wantSaves)
			}
			if guild := w.guilds.FindByID(1); guild != nil && len(guild.Members) != 1 {
				t.Fatalf("rejected accept changed target guild members: %+v", guild.Members)
			}
		})
	}
}

func TestGuildLeaveDissolvesAndRestoresAfterPersistenceFailure(t *testing.T) {
	tests := []struct {
		name         string
		memberLeaves bool
		storeErr     error
		guild        bool
	}{
		{name: "unaffiliated character", guild: false},
		{name: "last member dissolves guild", guild: true},
		{name: "member leave restores on store failure", memberLeaves: true, storeErr: errors.New("database unavailable"), guild: true},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
			member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
			players := []*Player{leader}
			if tc.memberLeaves {
				players = append(players, member)
			}
			w, st := guildFlowWorld(players...)
			st.err = tc.storeErr
			if tc.guild {
				guild := model.Guild{ID: 1, Name: "Alpha", Members: []model.GuildMember{{
					Character: leader.Char.Name, Account: leader.Account.Name, Rank: model.GuildRankLeader,
				}}}
				leader.Char.GuildID, leader.Char.GuildRank = 1, model.GuildRankLeader
				if tc.memberLeaves {
					guild.Members = append(guild.Members, model.GuildMember{
						Character: member.Char.Name, Account: member.Account.Name, Rank: model.GuildRankMember,
					})
					member.Char.GuildID, member.Char.GuildRank = 1, model.GuildRankMember
				}
				w.guilds.Guilds = []model.Guild{guild}
			}
			actor := leader
			if tc.memberLeaves {
				actor = member
			}

			w.guildCommandLeave(actor.Session, actor, "")

			if !tc.guild {
				if actor.Char.GuildID != 0 || st.gameSaves != 0 {
					t.Fatalf("unaffiliated leave changed state: guild=%d saves=%d", actor.Char.GuildID, st.gameSaves)
				}
				return
			}
			if tc.storeErr != nil {
				current := w.guilds.FindByID(1)
				if current == nil || len(current.Members) != 2 || actor.Char.GuildID != 1 || st.gameSaves != 1 {
					t.Fatalf("failed leave did not restore membership: guild=%+v actor=%d saves=%d", current, actor.Char.GuildID, st.gameSaves)
				}
				return
			}
			if tc.memberLeaves {
				current := w.guilds.FindByID(1)
				if current == nil || current.Member(member.Char.Name) != nil || member.Char.GuildID != 0 ||
					leader.Char.GuildID != 1 || st.gameSaves != 1 {
					t.Fatalf("member leave changed wrong state: guild=%+v member=%d leader=%d saves=%d",
						current, member.Char.GuildID, leader.Char.GuildID, st.gameSaves)
				}
				return
			}
			if w.guilds.FindByID(1) != nil || actor.Char.GuildID != 0 || st.gameSaves != 1 {
				t.Fatalf("last leave did not dissolve guild: guild=%+v actor=%d saves=%d",
					w.guilds.FindByID(1), actor.Char.GuildID, st.gameSaves)
			}
		})
	}
}

func TestGuildSubLeaderRejectsInvalidPromotionRequests(t *testing.T) {
	tests := []struct {
		name       string
		argument   string
		actorGuild bool
		actorRank  byte
		target     bool
		membership bool
		targetRank byte
		fullRanks  bool
		storeErr   error
	}{
		{name: "empty target", actorGuild: true, actorRank: model.GuildRankLeader},
		{name: "actor is unaffiliated", argument: "Member", target: true},
		{name: "actor is not leader", argument: "Member", actorGuild: true, actorRank: model.GuildRankMember, target: true},
		{name: "target is offline", argument: "Offline", actorGuild: true, actorRank: model.GuildRankLeader},
		{name: "target is not a guild member", argument: "Member", actorGuild: true, actorRank: model.GuildRankLeader, target: true},
		{name: "target already has a rank", argument: "Member", actorGuild: true, actorRank: model.GuildRankLeader, target: true, membership: true, targetRank: model.GuildRankSubFirst},
		{name: "all sub-leader ranks are occupied", argument: "Member", actorGuild: true, actorRank: model.GuildRankLeader, target: true, membership: true, fullRanks: true},
		{name: "persistence failure restores rank", argument: "Member", actorGuild: true, actorRank: model.GuildRankLeader, target: true, membership: true, storeErr: errors.New("database unavailable")},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
			var target *Player
			players := []*Player{leader}
			if tc.target {
				target, _ = networkedTestPlayer(2, "Member", 2101, 2100)
				players = append(players, target)
			}
			w, st := guildFlowWorld(players...)
			st.err = tc.storeErr
			if tc.actorGuild {
				addGuildCoverageLeader(w, leader, 1, "Alpha", model.KingdomNeutral, tc.actorRank)
			}
			if tc.membership {
				guild := w.guilds.FindByID(1)
				guild.Members = append(guild.Members, model.GuildMember{
					Character: target.Char.Name, Account: target.Account.Name, Rank: tc.targetRank,
				})
				target.Char.GuildID, target.Char.GuildRank = 1, tc.targetRank
			}
			if tc.fullRanks {
				guild := w.guilds.FindByID(1)
				for rank := model.GuildRankSubFirst; rank <= model.GuildRankSubLast; rank++ {
					guild.Members = append(guild.Members, model.GuildMember{Character: string(rune('A' + rank)), Rank: byte(rank)})
				}
			}
			beforeRank := byte(0)
			if target != nil {
				beforeRank = target.Char.GuildRank
			}

			w.guildCommandSubLeader(leader.Session, leader, tc.argument)

			if target != nil && target.Char.GuildRank != beforeRank {
				t.Fatalf("rejected promotion changed character rank: got %d, want %d", target.Char.GuildRank, beforeRank)
			}
			if tc.membership {
				member := w.guilds.FindByID(1).Member(target.Char.Name)
				if member == nil || member.Rank != beforeRank {
					t.Fatalf("rejected promotion changed registry rank: %+v", member)
				}
			}
			wantSaves := 0
			if tc.storeErr != nil {
				wantSaves = 1
			}
			if st.gameSaves != wantSaves {
				t.Fatalf("save calls = %d, want %d", st.gameSaves, wantSaves)
			}
		})
	}
}

func TestGuildDeprivateRejectsInvalidPacketContextAndTargets(t *testing.T) {
	tests := []struct {
		name       string
		actorGuild bool
		target     bool
		targetID   uint16
		member     bool
		targetRank byte
	}{
		{name: "actor has no guild", target: true, targetID: 2},
		{name: "target is not connected", actorGuild: true, targetID: 99},
		{name: "actor targets self", actorGuild: true, targetID: 1},
		{name: "target is not a guild member", actorGuild: true, target: true, targetID: 2},
		{name: "peer rank cannot be expelled", actorGuild: true, target: true, targetID: 2, member: true, targetRank: model.GuildRankLeader},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
			players := []*Player{leader}
			var target *Player
			if tc.target {
				target, _ = networkedTestPlayer(2, "Member", 2101, 2100)
				players = append(players, target)
			}
			w, st := guildFlowWorld(players...)
			var guild *model.Guild
			if tc.actorGuild {
				guild = addGuildCoverageLeader(w, leader, 1, "Alpha", model.KingdomNeutral, model.GuildRankLeader)
			}
			if tc.member {
				guild.Members = append(guild.Members, model.GuildMember{
					Character: target.Char.Name, Account: target.Account.Name, Rank: tc.targetRank,
				})
				target.Char.GuildID, target.Char.GuildRank = 1, tc.targetRank
			}
			beforeMembers := 0
			if guild != nil {
				beforeMembers = len(guild.Members)
			}
			packet := make([]byte, 16)
			binary.LittleEndian.PutUint32(packet[12:16], uint32(tc.targetID))

			w.onGuildDeprivate(leader.Session, packet)

			if guild != nil && len(w.guilds.FindByID(1).Members) != beforeMembers {
				t.Fatalf("rejected deprivate changed guild members: %+v", w.guilds.FindByID(1).Members)
			}
			if target != nil && tc.member && target.Char.GuildID != 1 {
				t.Fatalf("rejected deprivate changed target membership: %d", target.Char.GuildID)
			}
			if st.gameSaves != 0 {
				t.Fatalf("rejected deprivate saved %d times", st.gameSaves)
			}
		})
	}
	world := newZoneTestWorld()
	unmapped, _ := networkedTestPlayer(77, "NotMapped", 2100, 2100)
	world.onGuildDeprivate(unmapped.Session, nil)
	world.onGuildDeprivate(unmapped.Session, make([]byte, 15))
}
