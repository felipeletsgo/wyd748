package game

import (
	"testing"

	"wydgo/internal/model"
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
