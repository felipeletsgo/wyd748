package model

import (
	"bytes"
	"testing"
)

func TestGuildLeaderAndRegistryNameLookup(t *testing.T) {
	guild := Guild{Name: "Silver Guard", Members: []GuildMember{
		{Character: "Member", Rank: GuildRankMember},
		{Character: "Leader", Rank: GuildRankLeader},
	}}
	if got := guild.Leader(); got == nil || got.Character != "Leader" {
		t.Fatalf("Leader() = %+v, want the leader member", got)
	}
	guild.Leader().Character = "Updated"
	if guild.Members[1].Character != "Updated" {
		t.Fatal("Leader returned a copy instead of the canonical member")
	}
	noLeader := Guild{}
	if noLeader.Leader() != nil {
		t.Fatal("guild without a leader returned a member")
	}

	registry := GuildRegistry{Guilds: []Guild{guild}}
	if got := registry.FindByName(" silver GUARD "); got == nil || got.ID != guild.ID {
		t.Fatalf("case and whitespace insensitive lookup = %+v", got)
	}
	if registry.FindByName("missing") != nil {
		t.Fatal("unknown guild name returned a registry entry")
	}
}

func TestGuildsTxtUsesRequestedGroupAndChannel(t *testing.T) {
	registry := GuildRegistry{Guilds: []Guild{
		{ID: 7, Name: "Silver Guard"},
		{ID: 19, Name: "Akelonia"},
	}}
	want := []byte("2 3 7 Silver Guard\n2 3 19 Akelonia\n")
	if got := registry.GuildsTxt(2, 3); !bytes.Equal(got, want) {
		t.Fatalf("GuildsTxt = %q, want %q", got, want)
	}
	emptyRegistry := GuildRegistry{}
	if got := emptyRegistry.GuildsTxt(0, 1); len(got) != 0 {
		t.Fatalf("empty registry serialized as %q", got)
	}
}
