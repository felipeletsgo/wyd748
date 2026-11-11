package game

import (
	"math"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestKefraDefeatProposalCoversEligibilityAndDetachedFame(t *testing.T) {
	const guildID = uint16(7)
	defeatedAt := time.Unix(1_800_000_000, 0)
	tests := []struct {
		name         string
		registry     *model.GuildRegistry
		plans        []*killRewardPlan
		wantPlan     bool
		wantGuild    uint16
		wantFame     uint32
		wantDefeated bool
	}{
		{name: "missing guild registry"},
		{name: "already defeated", registry: &model.GuildRegistry{Wars: model.GuildWarState{
			Kefra: model.KefraState{Defeated: true},
		}}},
		{name: "no world Kefra", registry: &model.GuildRegistry{}, plans: []*killRewardPlan{
			{mob: &Mob{Def: &model.NPCDef{Name: "Other_Boss"}}},
		}},
		{name: "nil plans skipped", registry: &model.GuildRegistry{}, plans: []*killRewardPlan{nil}},
		{name: "Kefra death without killer", registry: &model.GuildRegistry{}, plans: []*killRewardPlan{
			{mob: &Mob{Def: &model.NPCDef{Name: kefraBossName}}},
		}, wantPlan: true, wantDefeated: true},
		{name: "Kefra death with empty killer character", registry: &model.GuildRegistry{}, plans: []*killRewardPlan{
			{mob: &Mob{Def: &model.NPCDef{Name: kefraBossName}}, killer: &Player{}},
		}, wantPlan: true, wantDefeated: true},
		{name: "killer outside a guild", registry: &model.GuildRegistry{}, plans: []*killRewardPlan{
			{mob: &Mob{Def: &model.NPCDef{Name: kefraBossName}}, killer: &Player{Char: &model.Char{Name: "Outsider"}}},
		}, wantPlan: true, wantDefeated: true},
		{name: "guild fame credited to detached proposal", registry: &model.GuildRegistry{
			Version: model.GuildRegistryVersion,
			Guilds:  []model.Guild{guildWithMembers(guildID, "Alpha", member("Killer", "account", model.GuildRankLeader))},
		}, plans: []*killRewardPlan{
			{mob: &Mob{Def: &model.NPCDef{Name: kefraBossName}}, killer: &Player{Char: &model.Char{Name: "Killer"}}},
		}, wantPlan: true, wantGuild: guildID, wantFame: kefraGuildFame, wantDefeated: true},
		{name: "fame overflow keeps the defeat", registry: &model.GuildRegistry{
			Version: model.GuildRegistryVersion,
			Guilds: []model.Guild{{ID: guildID, Name: "Alpha", Fame: math.MaxUint32 - 1,
				Members: []model.GuildMember{member("Killer", "account", model.GuildRankLeader)}}},
		}, plans: []*killRewardPlan{
			{mob: &Mob{Def: &model.NPCDef{Name: kefraBossName}}, killer: &Player{Char: &model.Char{Name: "Killer"}}},
		}, wantPlan: true, wantGuild: guildID, wantFame: math.MaxUint32 - 1, wantDefeated: true},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			world := &World{guilds: test.registry, clock: newFakeClock(defeatedAt)}
			var originalFame uint32
			if test.registry != nil {
				if guild := test.registry.FindByID(guildID); guild != nil {
					originalFame = guild.Fame
				}
			}

			plan, proposal, guild := world.kefraDefeatProposal(test.plans)
			if !test.wantPlan {
				if plan != nil || proposal != nil || guild != 0 {
					t.Fatalf("unexpected defeat proposal: plan=%p registry=%p guild=%d", plan, proposal, guild)
				}
				return
			}
			if plan == nil || proposal == nil || guild != test.wantGuild || proposal == test.registry {
				t.Fatalf("proposal identity or guild = plan %p registry %p guild %d", plan, proposal, guild)
			}
			if proposal.Wars.Kefra.Defeated != test.wantDefeated ||
				proposal.Wars.Kefra.Guild != test.wantGuild ||
				!proposal.Wars.Kefra.DefeatedAt.Equal(defeatedAt) {
				t.Fatalf("proposed Kefra state = %+v", proposal.Wars.Kefra)
			}
			if test.registry.Wars.Kefra.Defeated || test.registry.FindByID(guildID) != nil &&
				test.registry.FindByID(guildID).Fame != originalFame {
				t.Fatal("proposal mutated the live guild registry before persistence")
			}
			if proposed := proposal.FindByID(guildID); proposed != nil && proposed.Fame != test.wantFame {
				t.Fatalf("proposed guild fame = %d, want %d", proposed.Fame, test.wantFame)
			}
		})
	}
}

func TestKefraLocationUsesExplicitConfiguredAndLocalFallback(t *testing.T) {
	explicit := time.FixedZone("war-test", 90*60)
	world := &World{warLocation: explicit}
	world.warConfig.Timezone = "UTC"
	if got := world.kefraLocation(); got != explicit {
		t.Fatalf("explicit war location = %v, want the configured location", got)
	}

	world.warLocation = nil
	if got := world.kefraLocation(); got.String() != "UTC" {
		t.Fatalf("configured timezone = %q, want UTC", got)
	}

	world.warConfig.Timezone = "Invalid/Unknown"
	if got := world.kefraLocation(); got != time.Local {
		t.Fatalf("invalid timezone fallback = %v, want local %v", got, time.Local)
	}
	world.warConfig.Timezone = ""
	if got := world.kefraLocation(); got != time.Local {
		t.Fatalf("empty timezone fallback = %v, want local %v", got, time.Local)
	}
}
