package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
)

// W9: a guild that another process dissolved leaves the connected members of
// this process without the guild, and their clients receive the change.
func TestGuildReloadRepairsConnectedMembers(t *testing.T) {
	w := newZoneTestWorld()
	w.guildVersioning = true
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{ID: 7, Name: "Gone",
		Members: []model.GuildMember{{Character: "Member", Account: "member", Rank: model.GuildRankLeader}}}}}
	w.guildVersions = map[uint16]int64{7: 3}
	p := addZonePlayer(w, 1, 2100, 2100, 1000)
	p.Char.Name, p.Char.GuildID, p.Char.GuildRank = "Member", 7, model.GuildRankLeader
	w.onGuildSync(&guildSyncEvent{ids: []uint16{7}, loaded: true, states: []store.GuildRowState{{GuildID: 7}}})
	if w.guilds.FindByID(7) != nil || w.guildVersions[7] != 0 {
		t.Fatal("the removed guild stayed")
	}
	if p.Char.GuildID != 0 || p.Char.GuildRank != 0 || p.Session.QueuedPacketsForTest() == 0 {
		t.Fatalf("connected member was not repaired: guild=%d rank=%d packets=%d", p.Char.GuildID, p.Char.GuildRank,
			p.Session.QueuedPacketsForTest())
	}
	// An older row never replaces a newer local copy.
	w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 8, Name: "Kept", Fame: 5,
		Members: []model.GuildMember{{Character: "Other", Account: "other", Rank: model.GuildRankLeader}}})
	w.guildVersions[8] = 4
	w.onGuildSync(&guildSyncEvent{ids: []uint16{8}, loaded: true, states: []store.GuildRowState{{GuildID: 8, Present: true,
		Version: 3, Guild: model.Guild{ID: 8, Name: "Kept", Fame: 1}}}})
	if w.guilds.FindByID(8).Fame != 5 {
		t.Fatal("an older row replaced the local guild")
	}
}

func TestRequestGuildRefreshLaterCoalescesRowsAndFullRefresh(t *testing.T) {
	w := &World{}
	w.requestGuildRefreshLater([]uint16{7, 8, 7}, false)
	w.requestGuildRefreshLater([]uint16{9}, true)
	w.requestGuildRefreshLater(nil, false)
	if !w.guildRefreshAll || len(w.guildRefreshPending) != 3 ||
		!w.guildRefreshPending[7] || !w.guildRefreshPending[8] || !w.guildRefreshPending[9] {
		t.Fatalf("pending refresh state = all:%v rows:%v", w.guildRefreshAll, w.guildRefreshPending)
	}
}

func TestGuildReloadFailureKeepsRowsPendingForRetry(t *testing.T) {
	w := newZoneTestWorld()
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	w.guildRefreshInFlight = true
	w.guildRefreshPending = map[uint16]bool{3: true}

	w.onGuildSync(&guildSyncEvent{ids: []uint16{7, 8, 7}, all: true, loaded: true,
		err: errors.New("injected guild read failure")})

	if w.guildRefreshInFlight || !w.guildRefreshAll || len(w.guildRefreshPending) != 3 ||
		!w.guildRefreshPending[3] || !w.guildRefreshPending[7] || !w.guildRefreshPending[8] {
		t.Fatalf("failed reload lost retry state: inFlight=%v all=%v rows=%v",
			w.guildRefreshInFlight, w.guildRefreshAll, w.guildRefreshPending)
	}
}

func TestInstallGuildRowAddsNewGuildAndPreservesLocalRowsOnHeaderRefresh(t *testing.T) {
	w := newZoneTestWorld()
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, ActiveWarChannel: 2,
		Guilds: []model.Guild{{ID: 1, Name: "Local"}}}
	w.guildVersions = make(map[uint16]int64)

	newGuild := model.Guild{ID: 7, Name: "Reloaded", Fame: 40}
	if !w.installGuildRow(store.GuildRowState{GuildID: 7, Present: true, Version: 3, Guild: newGuild}) {
		t.Fatal("an unreserved new guild row was deferred")
	}
	if got := w.guilds.FindByID(7); got == nil || got.Name != newGuild.Name || got.Fame != newGuild.Fame || w.guildVersions[7] != 3 {
		t.Fatalf("new guild row was not installed: guild=%+v version=%d", got, w.guildVersions[7])
	}

	header := &model.GuildRegistry{Version: model.GuildRegistryVersion, Wars: model.GuildWarState{}}
	if !w.installGuildRow(store.GuildRowState{GuildID: 0, Present: true, Version: 5, Header: header}) {
		t.Fatal("an unreserved registry header was deferred")
	}
	if w.guilds.FindByID(1) == nil || w.guilds.FindByID(7) == nil || w.guilds.ActiveWarChannel != 2 ||
		w.guildVersions[0] != 5 {
		t.Fatalf("header refresh replaced local guild rows or channel: guilds=%+v channel=%d version=%d",
			w.guilds.Guilds, w.guilds.ActiveWarChannel, w.guildVersions[0])
	}
}

func TestGuildFullReloadRemovesRowsMissingFromTheDatabase(t *testing.T) {
	w := newZoneTestWorld()
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{
		{ID: 7, Name: "Removed"}, {ID: 8, Name: "Updated"},
	}}
	w.guildVersions = map[uint16]int64{7: 2, 8: 3}

	w.onGuildSync(&guildSyncEvent{all: true, loaded: true, states: []store.GuildRowState{{
		GuildID: 8, Present: true, Version: 4, Guild: model.Guild{ID: 8, Name: "Current"},
	}}})

	if w.guilds.FindByID(7) != nil {
		t.Fatal("whole refresh retained a guild missing from the database")
	}
	if got := w.guilds.FindByID(8); got == nil || got.Name != "Current" || w.guildVersions[8] != 4 {
		t.Fatalf("whole refresh failed to install the current row: guild=%+v version=%d", got, w.guildVersions[8])
	}
}

func TestGuildReloadDefersRowsOwnedByCriticalWrites(t *testing.T) {
	w := newZoneTestWorld()
	w.guildVersioning = true
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{ID: 7, Name: "Local"}}}
	w.guildVersions = map[uint16]int64{7: 2}
	key := guildRowKey(store.GuildParticipant{GuildID: 7})
	w.criticalWrites = &criticalPersistenceOwner{queue: &criticalPersistence{
		guildKeys: map[string]uint64{key: 1},
	}}

	w.onGuildSync(&guildSyncEvent{loaded: true, states: []store.GuildRowState{{
		GuildID: 7, Present: true, Version: 3, Guild: model.Guild{ID: 7, Name: "External"},
	}}})

	if got := w.guilds.FindByID(7); got == nil || got.Name != "Local" || w.guildVersions[7] != 2 {
		t.Fatalf("an in-flight guild row was published: guild=%+v version=%d", got, w.guildVersions[7])
	}
	if !w.guildRefreshPending[7] || w.guildRefreshAll {
		t.Fatalf("blocked row was not scheduled for retry: all=%v rows=%v", w.guildRefreshAll, w.guildRefreshPending)
	}
}

func TestGuildFullReloadDefersMissingRowOwnedByCriticalWrite(t *testing.T) {
	w := newZoneTestWorld()
	w.guildVersioning = true
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{ID: 7, Name: "Local"}}}
	w.guildVersions = map[uint16]int64{7: 2}
	key := guildRowKey(store.GuildParticipant{GuildID: 7})
	w.criticalWrites = &criticalPersistenceOwner{queue: &criticalPersistence{
		guildKeys: map[string]uint64{key: 1},
	}}

	w.onGuildSync(&guildSyncEvent{all: true, loaded: true})

	if guild := w.guilds.FindByID(7); guild == nil || guild.Name != "Local" || w.guildVersions[7] != 2 {
		t.Fatalf("whole refresh removed a guild while its write was pending: guild=%+v version=%d",
			guild, w.guildVersions[7])
	}
	if w.guildRefreshAll || !w.guildRefreshPending[7] {
		t.Fatalf("deferred missing row was not scheduled for a scoped retry: all=%v rows=%v",
			w.guildRefreshAll, w.guildRefreshPending)
	}
}

func TestGuildRefreshSkipsPlayerOwnedByCriticalDisconnect(t *testing.T) {
	w := newZoneTestWorld()
	w.guildVersioning = true
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{ID: 7, Name: "Gone",
		Members: []model.GuildMember{{Character: "Member", Account: "p", Rank: model.GuildRankLeader}},
	}}}
	w.guildVersions = map[uint16]int64{7: 2}
	p := addZonePlayer(w, 1, 2100, 2100, 1000)
	p.Char.Name, p.Char.GuildID, p.Char.GuildRank = "Member", 7, model.GuildRankLeader
	w.criticalDisconnects = map[*gameNet.Session]*criticalDisconnect{
		p.Session: {player: p},
	}

	w.onGuildSync(&guildSyncEvent{ids: []uint16{7}, loaded: true,
		states: []store.GuildRowState{{GuildID: 7}},
	})

	if w.guilds.FindByID(7) != nil || p.Char.GuildID != 7 || p.Char.GuildRank != model.GuildRankLeader ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("guild refresh modified a character while critical disconnect persistence owned it")
	}
}
