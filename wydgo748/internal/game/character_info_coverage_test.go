package game

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestCharacterInfoLineCoversGuildRolesAllyAndKingdom(t *testing.T) {
	observer, _ := networkedTestPlayer(1, "Observer", 2100, 2100)
	w := worldWithNetworkedPlayers(observer)
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{
		{ID: 1, Name: "Alpha", Ally: 2, Members: []model.GuildMember{
			{Character: "GuildLeader", Rank: model.GuildRankLeader},
			{Character: "GuildSubLeader", Rank: model.GuildRankSubFirst},
			{Character: "GuildMember", Rank: model.GuildRankMember},
		}},
		{ID: 2, Name: "Beta"},
	}}

	for _, test := range []struct {
		name, character, role string
	}{
		{name: "leader", character: "GuildLeader", role: "Leader"},
		{name: "subleader", character: "GuildSubLeader", role: "Sub-leader"},
		{name: "member", character: "GuildMember", role: "Member"},
	} {
		t.Run(test.name, func(t *testing.T) {
			line := w.characterInfoLine(&model.Char{Name: test.character})
			if !strings.Contains(line, "Guild: Alpha") || !strings.Contains(line, test.role) ||
				!strings.Contains(line, "allied with Beta") {
				t.Fatalf("guild information omitted role or ally: %q", line)
			}
		})
	}

	cape, ok := model.KingdomCapeAtTier(model.CapeTierBasic, model.KingdomAkelonia)
	if !ok {
		t.Fatal("Akelonia common cape is unavailable")
	}
	character := &model.Char{Name: "KingdomMember", Citizenship: 2}
	character.Equip[model.CapeSlot].Index = cape
	line := w.characterInfoLine(character)
	if !strings.Contains(line, "Citizenship: Channel 2") || !strings.Contains(line, w.kingdomName(model.KingdomAkelonia)) {
		t.Fatalf("citizenship or kingdom omitted: %q", line)
	}
}
