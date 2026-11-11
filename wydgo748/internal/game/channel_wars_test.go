package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestChannelWarEligibilityRequiresMatchingGuildCitizenshipInCluster(t *testing.T) {
	p := &Player{Char: &model.Char{Name: "Warrior", GuildID: 7, Citizenship: 2}}
	w := &World{channel: 2}
	if !w.channelWarEligible(p) {
		t.Fatal("standalone legacy world changed its admission rule")
	}
	w.cluster = &ChannelCluster{}
	if w.channelWarEligible(nil) || w.channelWarEligible(&Player{}) || w.channelWarEligible(p) {
		t.Fatal("cluster admitted a missing or unregistered guild member")
	}
	w.guilds = &model.GuildRegistry{Guilds: []model.Guild{{ID: 7, Citizenship: 2,
		Members: []model.GuildMember{{Character: "Warrior", Account: "warrior"}}}}}
	if !w.channelWarEligible(p) {
		t.Fatal("matching channel, character and guild citizenship was rejected")
	}
	p.Char.Citizenship = 1
	if w.channelWarEligible(p) {
		t.Fatal("character citizenship from another channel was accepted")
	}
	p.Char.Citizenship = 2
	w.guilds.Guilds[0].Citizenship = 1
	if w.channelWarEligible(p) {
		t.Fatal("guild citizenship from another channel was accepted")
	}
	w.guilds.Guilds[0].Citizenship = 2
	w.channel = 0
	if w.channelWarEligible(p) {
		t.Fatal("channel zero admitted a clustered war participant")
	}
}
