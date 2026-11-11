package model

import (
	"strings"
	"testing"
	"time"
)

func validBossForCoverage() BossConfig {
	return BossConfig{ID: "encounter", NPC: "Gremlin", Type: BossTypeChaser, Spawn: BossSpawn{X: 2100, Y: 2100}}
}

func TestBossTypeValidity(t *testing.T) {
	for _, test := range []struct {
		typeName BossType
		want     bool
	}{
		{BossTypeChaser, true}, {BossTypeCaster, true}, {BossTypeSummoner, true}, {BossTypePhased, true}, {BossType("other"), false},
	} {
		if got := test.typeName.Valid(); got != test.want {
			t.Errorf("BossType(%q).Valid() = %t, want %t", test.typeName, got, test.want)
		}
	}
}

func TestBossValidationRejectsInvalidEncounterFields(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*BossConfig)
	}{
		{"blank ID", func(c *BossConfig) { c.ID = " \t" }},
		{"blank NPC", func(c *BossConfig) { c.NPC = " " }},
		{"unknown type", func(c *BossConfig) { c.Type = BossType("unknown") }},
		{"zero x", func(c *BossConfig) { c.Spawn.X = 0 }},
		{"zero y", func(c *BossConfig) { c.Spawn.Y = 0 }},
		{"negative respawn", func(c *BossConfig) { c.Spawn.RespawnSeconds = -1 }},
		{"caster without skill", func(c *BossConfig) { c.Type = BossTypeCaster }},
		{"summoner without adds", func(c *BossConfig) { c.Type = BossTypeSummoner }},
		{"phased without phases", func(c *BossConfig) { c.Type = BossTypePhased }},
		{"name too long", func(c *BossConfig) { c.Name = strings.Repeat("x", MaxBossMessageBytes+1) }},
		{"death message too long", func(c *BossConfig) { c.DeathMessage = strings.Repeat("x", MaxBossMessageBytes+1) }},
		{"skill negative ID", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: -1, CooldownSeconds: 1}}
		}},
		{"skill nonpositive cooldown", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 0}}
		}},
		{"skill negative range", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Range: -1}}
		}},
		{"skill negative threshold", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, MaxHPPercent: -1}}
		}},
		{"skill high threshold", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, MaxHPPercent: 101}}
		}},
		{"skill message too long", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Message: strings.Repeat("x", MaxBossMessageBytes+1)}}
		}},
		{"summon blank NPC", func(c *BossConfig) {
			c.Type = BossTypeSummoner
			c.Summons = []BossSummon{{Count: 1, CooldownSeconds: 1}}
		}},
		{"summon nonpositive count", func(c *BossConfig) {
			c.Type = BossTypeSummoner
			c.Summons = []BossSummon{{NPC: "Gremlin", CooldownSeconds: 1}}
		}},
		{"negative max alive", func(c *BossConfig) {
			c.Type = BossTypeSummoner
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 1, MaxAlive: -1, CooldownSeconds: 1}}
		}},
		{"max alive below count", func(c *BossConfig) {
			c.Type = BossTypeSummoner
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 2, MaxAlive: 1, CooldownSeconds: 1}}
		}},
		{"summon message too long", func(c *BossConfig) {
			c.Type = BossTypeSummoner
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 1, CooldownSeconds: 1, Message: strings.Repeat("x", MaxBossMessageBytes+1)}}
		}},
		{"phase threshold low", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Phases = []BossPhase{{HPPercent: 0}}
		}},
		{"phase threshold high", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Phases = []BossPhase{{HPPercent: 100}}
		}},
		{"invalid phase type", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Phases = []BossPhase{{HPPercent: 50, Type: "unknown"}}
		}},
		{"negative phase shield", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Phases = []BossPhase{{HPPercent: 50, ShieldPercent: -1}}
		}},
		{"phase shield over one hundred", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Phases = []BossPhase{{HPPercent: 50, ShieldPercent: 101}}
		}},
		{"shield waits without summons", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Phases = []BossPhase{{HPPercent: 50, ShieldUntilAddsDead: true}}
		}},
		{"drop without item", func(c *BossConfig) { c.Drops = []BossDrop{{ChancePercent: 100}} }},
		{"drop zero chance", func(c *BossConfig) { c.Drops = []BossDrop{{Item: 1}} }},
		{"drop chance above one hundred", func(c *BossConfig) { c.Drops = []BossDrop{{Item: 1, ChancePercent: 101}} }},
		{"drop negative amount", func(c *BossConfig) { c.Drops = []BossDrop{{Item: 1, ChancePercent: 100, Amount: -1}} }},
		{"incomplete area reward item", func(c *BossConfig) { c.AreaReward.Item = 1 }},
		{"incomplete area reward amount", func(c *BossConfig) { c.AreaReward.Amount = 1 }},
		{"negative area reward amount", func(c *BossConfig) { c.AreaReward.Item, c.AreaReward.Amount = 1, -1 }},
	} {
		t.Run(test.name, func(t *testing.T) {
			config := validBossForCoverage()
			test.mutate(&config)
			if err := config.Validate(); err == nil {
				t.Fatal("invalid boss configuration was accepted")
			}
		})
	}
}

func TestBossValidationAcceptsEveryBehaviorAndBoundary(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*BossConfig)
	}{
		{"chaser", func(*BossConfig) {}},
		{"caster", func(c *BossConfig) {
			c.Type = BossTypeCaster
			c.Skills = []BossSkill{{ID: 0, CooldownSeconds: 1, Range: MaxBossSkillRange, MaxHPPercent: 100}}
		}},
		{"summoner", func(c *BossConfig) {
			c.Type = BossTypeSummoner
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 1, CooldownSeconds: 1}}
		}},
		{"phased with adds", func(c *BossConfig) {
			c.Type = BossTypePhased
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 1, CooldownSeconds: 1}}
			c.Phases = []BossPhase{{HPPercent: 50, ShieldPercent: 100, ShieldUntilAddsDead: true}}
		}},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			config := validBossForCoverage()
			test.mutate(&config)
			if err := config.Validate(); err != nil {
				t.Fatalf("valid behavior was rejected: %v", err)
			}
		})
	}

	config := validBossForCoverage()
	config.Spawn.RespawnSeconds = 0
	if got := config.RespawnDelay(); got != 0 {
		t.Fatalf("disabled respawn delay = %s, want zero", got)
	}
	config.Spawn.RespawnSeconds = 3
	if got := config.RespawnDelay(); got != 3*time.Second {
		t.Fatalf("respawn delay = %s, want %s", got, 3*time.Second)
	}
}

func TestBossValidationRejectsDuplicatePhasesAndCatalogIDs(t *testing.T) {
	config := validBossForCoverage()
	config.Type = BossTypePhased
	config.Phases = []BossPhase{{HPPercent: 50}, {HPPercent: 50}}
	if err := config.Validate(); err == nil {
		t.Fatal("duplicate phase threshold was accepted")
	}

	first := validBossForCoverage()
	first.SourceFile = "first.json"
	second := validBossForCoverage()
	second.SourceFile = "second.json"
	catalog := BossCatalog{Bosses: []BossConfig{first, second}}
	if err := catalog.Validate(); err == nil {
		t.Fatal("duplicate boss encounter ID was accepted")
	}
	catalog.Bosses[1].ID = "other"
	if err := catalog.Validate(); err != nil {
		t.Fatalf("distinct valid boss IDs were rejected: %v", err)
	}
	catalog.Bosses[1].NPC = " "
	if err := catalog.Validate(); err == nil {
		t.Fatal("catalog accepted an invalid boss member")
	}
}
