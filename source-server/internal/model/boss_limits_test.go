package model

import (
	"strings"
	"testing"
)

func validLimitedBoss() BossConfig {
	return BossConfig{ID: "safe", NPC: "Gremlin", Type: BossTypeSummoner,
		Spawn:   BossSpawn{X: 2100, Y: 2100},
		Summons: []BossSummon{{NPC: "Gremlin", Count: 1, MaxAlive: 1, CooldownSeconds: 1}}}
}

func TestBossRejectsUnsafeContentLimits(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*BossConfig)
	}{
		{"summon count", func(c *BossConfig) {
			c.Summons[0].Count = MaxBossSummonPerCast + 1
			c.Summons[0].MaxAlive = c.Summons[0].Count
		}},
		{"summon alive", func(c *BossConfig) { c.Summons[0].MaxAlive = MaxBossSummonedAlive + 1 }},
		{"zero summon cooldown", func(c *BossConfig) { c.Summons[0].CooldownSeconds = 0 }},
		{"skill range", func(c *BossConfig) { c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Range: MaxBossSkillRange + 1}} }},
		{"too many skills", func(c *BossConfig) { c.Skills = make([]BossSkill, MaxBossSkills+1) }},
		{"too many summons", func(c *BossConfig) { c.Summons = make([]BossSummon, MaxBossSummonGroups+1) }},
		{"too many phases", func(c *BossConfig) { c.Phases = make([]BossPhase, MaxBossPhases+1) }},
		{"too many drops", func(c *BossConfig) { c.Drops = make([]BossDrop, MaxBossDrops+1) }},
		{"message", func(c *BossConfig) { c.SpawnMessage = strings.Repeat("x", MaxBossMessageBytes+1) }},
		{"coordinate", func(c *BossConfig) { c.Spawn.X = MaxBossCoordinate + 1 }},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			config := validLimitedBoss()
			tc.mutate(&config)
			if err := config.Validate(); err == nil {
				t.Fatal("configuracao capaz de amplificar carga foi aceita")
			}
		})
	}
}

func TestBossAcceptsLimitsAtBoundary(t *testing.T) {
	config := validLimitedBoss()
	config.Summons[0].Count = MaxBossSummonPerCast
	config.Summons[0].MaxAlive = MaxBossSummonedAlive
	config.SpawnMessage = strings.Repeat("x", MaxBossMessageBytes)
	if err := config.Validate(); err != nil {
		t.Fatalf("limites validos foram recusados: %v", err)
	}
}
