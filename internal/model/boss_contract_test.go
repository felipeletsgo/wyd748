package model

import (
	"strings"
	"testing"
	"time"
)

func validBossContract() BossConfig {
	return BossConfig{
		ID: "contract-boss", NPC: "Gremlin", Type: BossTypeChaser,
		Spawn: BossSpawn{X: 2100, Y: 2100},
	}
}

func TestBossTypeAndRespawnDelayContracts(t *testing.T) {
	for _, typ := range []BossType{BossTypeChaser, BossTypeCaster, BossTypeSummoner, BossTypePhased} {
		if !typ.Valid() {
			t.Fatalf("tipo nativo %q foi recusado", typ)
		}
	}
	if BossType("scripted").Valid() {
		t.Fatal("tipo de boss arbitrario foi aceito")
	}

	cfg := validBossContract()
	if cfg.RespawnDelay() != 0 {
		t.Fatal("respawn zero deveria desabilitar renascimento")
	}
	cfg.Spawn.RespawnSeconds = 17
	if got := cfg.RespawnDelay(); got != 17*time.Second {
		t.Fatalf("RespawnDelay=%v, esperado 17s", got)
	}
}

func TestBossConfigRejectsEveryInvalidContractFamily(t *testing.T) {
	tooLong := strings.Repeat("x", MaxBossMessageBytes+1)
	tests := []struct {
		name   string
		mutate func(*BossConfig)
	}{
		{"empty id", func(c *BossConfig) { c.ID = "  " }},
		{"empty npc", func(c *BossConfig) { c.NPC = "  " }},
		{"invalid type", func(c *BossConfig) { c.Type = BossType("scripted") }},
		{"zero spawn", func(c *BossConfig) { c.Spawn.X = 0 }},
		{"outside map y", func(c *BossConfig) { c.Spawn.Y = MaxBossCoordinate + 1 }},
		{"negative respawn", func(c *BossConfig) { c.Spawn.RespawnSeconds = -1 }},
		{"caster without skill", func(c *BossConfig) { c.Type = BossTypeCaster }},
		{"summoner without summons", func(c *BossConfig) { c.Type = BossTypeSummoner }},
		{"phased without phases", func(c *BossConfig) { c.Type = BossTypePhased }},
		{"name too long", func(c *BossConfig) { c.Name = tooLong }},
		{"death message too long", func(c *BossConfig) { c.DeathMessage = tooLong }},

		{"skill negative id", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: -1, CooldownSeconds: 1, Range: 1}}
		}},
		{"skill zero cooldown", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 0, Range: 1}}
		}},
		{"skill negative range", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Range: -1}}
		}},
		{"skill hp below zero", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Range: 1, MaxHPPercent: -1}}
		}},
		{"skill hp above hundred", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Range: 1, MaxHPPercent: 101}}
		}},
		{"skill message too long", func(c *BossConfig) {
			c.Skills = []BossSkill{{ID: 1, CooldownSeconds: 1, Range: 1, Message: tooLong}}
		}},

		{"summon empty npc", func(c *BossConfig) {
			c.Summons = []BossSummon{{NPC: " ", Count: 1, CooldownSeconds: 1}}
		}},
		{"summon zero count", func(c *BossConfig) {
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 0, CooldownSeconds: 1}}
		}},
		{"summon negative max alive", func(c *BossConfig) {
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 1, MaxAlive: -1, CooldownSeconds: 1}}
		}},
		{"summon max alive below count", func(c *BossConfig) {
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 2, MaxAlive: 1, CooldownSeconds: 1}}
		}},
		{"summon message too long", func(c *BossConfig) {
			c.Summons = []BossSummon{{NPC: "Gremlin", Count: 1, CooldownSeconds: 1, Message: tooLong}}
		}},

		{"phase hp zero", func(c *BossConfig) { c.Phases = []BossPhase{{HPPercent: 0}} }},
		{"phase hp hundred", func(c *BossConfig) { c.Phases = []BossPhase{{HPPercent: 100}} }},
		{"duplicate phase", func(c *BossConfig) {
			c.Phases = []BossPhase{{HPPercent: 75}, {HPPercent: 75}}
		}},
		{"phase invalid type", func(c *BossConfig) {
			c.Phases = []BossPhase{{HPPercent: 75, Type: BossType("scripted")}}
		}},
		{"phase shield below zero", func(c *BossConfig) {
			c.Phases = []BossPhase{{HPPercent: 75, ShieldPercent: -1}}
		}},
		{"phase shield above hundred", func(c *BossConfig) {
			c.Phases = []BossPhase{{HPPercent: 75, ShieldPercent: 101}}
		}},
		{"phase shield without adds", func(c *BossConfig) {
			c.Phases = []BossPhase{{HPPercent: 75, ShieldUntilAddsDead: true}}
		}},
		{"phase message too long", func(c *BossConfig) {
			c.Phases = []BossPhase{{HPPercent: 75, Message: tooLong}}
		}},

		{"drop missing item", func(c *BossConfig) {
			c.Drops = []BossDrop{{ChancePercent: 100}}
		}},
		{"drop zero chance", func(c *BossConfig) {
			c.Drops = []BossDrop{{Item: 4011, ChancePercent: 0}}
		}},
		{"drop chance above hundred", func(c *BossConfig) {
			c.Drops = []BossDrop{{Item: 4011, ChancePercent: 101}}
		}},
		{"drop negative amount", func(c *BossConfig) {
			c.Drops = []BossDrop{{Item: 4011, ChancePercent: 100, Amount: -1}}
		}},
		{"area item without amount", func(c *BossConfig) {
			c.AreaReward = BossAreaReward{Item: 4011}
		}},
		{"area amount without item", func(c *BossConfig) {
			c.AreaReward = BossAreaReward{Amount: 1}
		}},
		{"area negative amount", func(c *BossConfig) {
			c.AreaReward = BossAreaReward{Item: 4011, Amount: -1}
		}},
		{"area excessive amount", func(c *BossConfig) {
			c.AreaReward = BossAreaReward{Item: 4011, Amount: MaxBossAreaReward + 1}
		}},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			cfg := validBossContract()
			tc.mutate(&cfg)
			if err := cfg.Validate(); err == nil {
				t.Fatal("configuracao invalida foi aceita")
			}
		})
	}
}

func TestBossConfigAcceptsCompleteCrossFamilyProfile(t *testing.T) {
	cfg := validBossContract()
	cfg.Type = BossTypePhased
	cfg.Skills = []BossSkill{{ID: 7, CooldownSeconds: 2, Range: MaxBossSkillRange, MaxHPPercent: 100}}
	cfg.Summons = []BossSummon{{NPC: "Gremlin", Count: 2, MaxAlive: 4, CooldownSeconds: 3}}
	cfg.Phases = []BossPhase{{HPPercent: 75, Type: BossTypeCaster, ShieldPercent: 25, ShieldUntilAddsDead: true}}
	cfg.Drops = []BossDrop{{Item: 4011, ChancePercent: 100, Amount: 1}}
	cfg.AreaReward = BossAreaReward{Item: 4012, Amount: MaxBossAreaReward}
	if err := cfg.Validate(); err != nil {
		t.Fatalf("perfil completo valido foi recusado: %v", err)
	}
}

func TestBossCatalogRejectsDuplicateIDsAndAcceptsUniqueBosses(t *testing.T) {
	a := validBossContract()
	a.SourceFile = "a.lua"
	b := validBossContract()
	b.SourceFile = "b.lua"
	duplicate := BossCatalog{Bosses: []BossConfig{a, b}}
	if err := duplicate.Validate(); err == nil || !strings.Contains(err.Error(), "definido em") {
		t.Fatalf("catalogo duplicado nao foi rejeitado corretamente: %v", err)
	}

	b.ID = "other-boss"
	if err := (&BossCatalog{Bosses: []BossConfig{a, b}}).Validate(); err != nil {
		t.Fatalf("catalogo unico foi recusado: %v", err)
	}
}
