package model

import (
	"encoding/json"
	"testing"
	"time"
)

func TestChannelWarPersistenceAndSingleWeeklyReset(t *testing.T) {
	r := &GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{{ID: 1, Fame: 50}}, Wars: GuildWarState{FameResetDay: "2026-10-18"}}
	r.SelectWarChannel(1)
	r.Wars.Tower.Owner = 1
	r.SelectWarChannel(2)
	if r.Wars.Tower.Owner != 0 {
		t.Fatal("channel 2 inherited channel 1 tower")
	}
	r.Wars.Tower.Day = "2026-10-25"
	now := time.Date(2026, 10, 25, 0, 0, 0, 0, time.FixedZone("BRT", -3*3600))
	if !r.ResetWeeklyGuildFame(now) || r.Guilds[0].Fame != 0 {
		t.Fatal("weekly reset missing")
	}
	r.Guilds[0].Fame = 7
	r.SelectWarChannel(1)
	if r.ResetWeeklyGuildFame(now) || r.Guilds[0].Fame != 7 || r.Wars.Tower.Owner != 1 {
		t.Fatal("reset repeated or channel ownership lost")
	}
	r.SelectWarChannel(2)
	b, err := json.Marshal(r)
	if err != nil {
		t.Fatal(err)
	}
	var restored GuildRegistry
	if err := json.Unmarshal(b, &restored); err != nil {
		t.Fatal(err)
	}
	if restored.Wars.Tower.Owner != 1 || restored.ActiveWarChannel != 0 {
		t.Fatal("legacy root does not represent channel 1")
	}
	restored.SelectWarChannel(2)
	if restored.Wars.Tower.Owner != 0 || restored.Wars.Tower.Day != "2026-10-25" || restored.ResetWeeklyGuildFame(now) {
		t.Fatal("channel 2 state or reset marker lost on restart")
	}
	clone := restored.Clone()
	state := clone.ChannelWars[1]
	state.Tower.Owner = 0
	clone.ChannelWars[1] = state
	if restored.ChannelWars[1].Tower.Owner != 1 {
		t.Fatal("snapshot shares mutable channel registry")
	}
}

func TestChannelGuildCitizenshipUpgradeRunsOnce(t *testing.T) {
	r := &GuildRegistry{Guilds: []Guild{{ID: 1}, {ID: 2, Citizenship: 2}}}
	r.UpgradeChannelCitizenship()
	if r.Guilds[0].Citizenship != 1 || r.Guilds[1].Citizenship != 2 {
		t.Fatal("legacy migration lost explicit citizenship")
	}
	r.Guilds[0].Citizenship = 0
	r.UpgradeChannelCitizenship()
	if r.Guilds[0].Citizenship != 0 {
		t.Fatal("neutral citizenship was reassigned")
	}
}
