package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func guildAllyCoverageFixture(t *testing.T) (*World, *Player, *guildFlowStore) {
	t.Helper()
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	w, store := guildFlowWorld(leader)
	w.guilds.Guilds = []model.Guild{
		{ID: 1, Name: "Alpha", Members: []model.GuildMember{{
			Character: leader.Char.Name, Account: leader.Account.Name, Rank: model.GuildRankLeader,
		}}},
		{ID: 2, Name: "Beta"},
	}
	leader.Char.GuildID, leader.Char.GuildRank = 1, model.GuildRankLeader
	return w, leader, store
}

func guildAllyCoveragePacket(claimedGuild, allyID uint32) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint32(pkt[12:16], claimedGuild)
	binary.LittleEndian.PutUint32(pkt[16:20], allyID)
	return pkt
}

func TestGuildAllyHandlerRejectsMalformedUnauthorizedSelfAndMissingGuild(t *testing.T) {
	w, leader, store := guildAllyCoverageFixture(t)
	w.onGuildAlly(leader.Session, make([]byte, 19))
	if got := leader.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("malformed packet queued %d responses", got)
	}

	// A leader rank does not grant authority when the character is absent from
	// the canonical guild registry.
	leaderName := leader.Char.Name
	leader.Char.Name = "Orphan"
	w.onGuildAlly(leader.Session, guildAllyCoveragePacket(1, 2))
	if got := leader.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("unauthorized request queued %d responses, want 1", got)
	}
	leader.Char.Name = leaderName

	w.onGuildAlly(leader.Session, guildAllyCoveragePacket(0, 1))
	if got := leader.Session.QueuedPacketsForTest(); got != 2 {
		t.Fatalf("self alliance queued %d responses, want 2 total", got)
	}
	w.onGuildAlly(leader.Session, guildAllyCoveragePacket(0, 99))
	if got := leader.Session.QueuedPacketsForTest(); got != 3 {
		t.Fatalf("unknown guild queued %d responses, want 3 total", got)
	}
	if w.guilds.FindByID(1).Ally != 0 || store.gameSaves != 0 {
		t.Fatal("rejected alliance request changed or saved guild state")
	}
}

func TestGuildAllyHandlerAcceptsClaimMismatchAndPersistsAllianceBreak(t *testing.T) {
	w, leader, store := guildAllyCoverageFixture(t)
	// The first packet field is untrusted metadata; the authenticated character
	// and registry membership select the guild to mutate.
	w.onGuildAlly(leader.Session, guildAllyCoveragePacket(99, 2))
	if got := w.guilds.FindByID(1).Ally; got != 2 || store.gameSaves != 1 {
		t.Fatalf("mismatched claim applied ally=%d with saves=%d", got, store.gameSaves)
	}

	w.onGuildAlly(leader.Session, guildAllyCoveragePacket(1, 0))
	if got := w.guilds.FindByID(1).Ally; got != 0 || store.gameSaves != 2 {
		t.Fatalf("alliance break left ally=%d with saves=%d", got, store.gameSaves)
	}
}

func TestGuildAllyHandlerRejectsCriticalSelfAndMissingGuild(t *testing.T) {
	for _, tc := range []struct {
		name   string
		allyID uint16
	}{
		{name: "self", allyID: 1},
		{name: "missing", allyID: 99},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, leader, _, _, base, _, release := criticalTreasuryFixture(t, 100)
			defer release()
			w.onGuildAlly(leader.Session,
				criticalGuildTestPacket(wire.OpGuildAlly, leader.ID, 0, tc.allyID))
			if got := leader.Session.QueuedPacketsForTest(); got != 1 {
				t.Fatalf("rejected critical request queued %d responses, want 1", got)
			}
			if len(base.entered) != 0 || w.guilds.FindByID(1).Ally != 0 {
				t.Fatal("rejected critical request submitted or changed guild state")
			}
		})
	}
}

func TestGuildAllyHandlerCommitsCriticalAllianceAndBreakAfterAcknowledgement(t *testing.T) {
	for _, tc := range []struct {
		name      string
		initial   uint16
		requested uint16
	}{
		{name: "form", requested: 2},
		{name: "break", initial: 2},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, leader, _, _, base, q, release := criticalTreasuryFixture(t, 100)
			defer release()
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Beta"})
			w.guilds.FindByID(1).Ally = tc.initial
			w.onGuildAlly(leader.Session,
				criticalGuildTestPacket(wire.OpGuildAlly, leader.ID, 1, tc.requested))

			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.guilds == nil || proposal.guilds.FindByID(1).Ally != tc.requested ||
				w.guilds.FindByID(1).Ally != tc.initial || leader.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("critical alliance was not kept detached until persistence acknowledgement")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			if w.shuttingDown || w.guilds.FindByID(1).Ally != tc.requested ||
				leader.Session.QueuedPacketsForTest() != 1 || w.criticalGuildReserved() {
				t.Fatal("acknowledged critical alliance did not settle exactly once")
			}
		})
	}
}
