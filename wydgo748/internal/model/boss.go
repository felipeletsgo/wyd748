package model

import (
	"fmt"
	"strings"
	"time"
)

// boss.go defines declarative boss configuration loaded from data/content/boss/*.lua.
// Boss behavior lives in internal/game/boss_types.go; Lua selects a behavior
// and supplies values, but cannot define new server rules or mutate world state.
// Bosses have their own spawn and respawn rules, separate from NPCGener. They
// inherit the catalog NPC's appearance, equipment, and base attributes.

// BossType selects a built-in behavior. Unsupported types fail at startup.
type BossType string

const (
	// BossTypeChaser pursues and attacks in melee; the other types build on it.
	BossTypeChaser BossType = "chaser"
	// BossTypeCaster keeps its distance and casts configured skills.
	BossTypeCaster BossType = "caster"
	// BossTypeSummoner calls groups of adds during combat.
	BossTypeSummoner BossType = "summoner"
	// BossTypePhased changes behavior at HP thresholds.
	BossTypePhased BossType = "phased"
)

func (t BossType) Valid() bool {
	switch t {
	case BossTypeChaser, BossTypeCaster, BossTypeSummoner, BossTypePhased:
		return true
	}
	return false
}

// BossSpawn specifies where and how often the boss appears.
type BossSpawn struct {
	X, Y uint16
	// RespawnSeconds is the delay after death. Zero disables automatic respawn.
	RespawnSeconds int
}

// BossStats overrides inherited NPC attributes. Nil retains the base value;
// pointers allow explicit zero values, including zero defense.
type BossStats struct {
	Level     *uint32
	MaxHP     *uint32
	Con       *uint32
	Attack    *uint32
	Defense   *uint32
	AttackRun *uint32
	ExpReward *uint32
	Gold      *uint32
}

// BossSkill is a spell in the boss's repertoire.
type BossSkill struct {
	// ID indexes data/catalog/SkillData.csv.
	ID int
	// CooldownSeconds is the minimum interval between uses of this skill.
	CooldownSeconds int
	// Range is the maximum casting distance.
	Range int
	// MaxHPPercent enables this skill at or below the given HP percentage.
	// Zero and 100 both mean always available.
	MaxHPPercent int
	// Message announces the cast to nearby players.
	Message string
}

// BossSummon defines an invocable group of adds.
type BossSummon struct {
	// NPC is the name in the catalog (data/npcs/*.json).
	NPC string
	// Count is the number created per summon.
	Count int
	// MaxAlive limits concurrent members of this group. Zero has no extra cap.
	MaxAlive int
	// CooldownSeconds is the interval between summons.
	CooldownSeconds int
	Message         string
}

// BossPhase changes behavior at an HP threshold.
type BossPhase struct {
	// HPPercent activates the phase when HP crosses it from above.
	HPPercent int
	// Type is the new behavior. Empty retains the boss's current type.
	Type BossType
	// ShieldPercent absorbs this percentage of incoming damage.
	ShieldPercent int
	// ShieldUntilAddsDead removes the shield after the encounter adds die.
	ShieldUntilAddsDead bool
	Message             string
}

// BossDrop is a special reward independent of native carry/drop rates.
// MaxBossAreaReward limits the number of ground items and visibility packets.
const MaxBossAreaReward = 200

// Content safety limits prevent Lua typos from creating excessive entities,
// packets, or work on the World goroutine; they do not balance encounters.
const (
	MaxBossSkills        = 32
	MaxBossSummonGroups  = 16
	MaxBossPhases        = 16
	MaxBossDrops         = 64
	MaxBossDropAmount    = 255
	MaxBossSummonPerCast = 64
	MaxBossSummonedAlive = 256
	MaxBossSkillRange    = 64
	MaxBossMessageBytes  = 256
	MaxBossCoordinate    = 4095
)

// MaxBossDurationSeconds is the largest whole-second interval representable
// by time.Duration. Rejecting larger values prevents a configuration integer
// from wrapping into a negative cooldown or respawn deadline.
const MaxBossDurationSeconds int64 = (1<<63 - 1) / int64(time.Second)

// BossAreaReward distributes item units on the ground when the boss dies.
// Zero Item and Amount disable the reward.
type BossAreaReward struct {
	// Item is the index in data/catalog/itemlist.csv.
	Item uint16
	// Amount is the number of ground units, one per cell.
	Amount int
}

type BossDrop struct {
	// Item is the index in data/catalog/itemlist.csv.
	Item uint16
	// ChancePercent is the probability (1-100); 100 is guaranteed.
	ChancePercent int
	// Amount uses native EF_AMOUNT for stackable items. Zero means one unit.
	Amount int
}

// BossConfig describes one complete boss encounter.
type BossConfig struct {
	// ID identifies the encounter in logs and must be unique.
	ID string
	// Name is shown in the client. Empty uses the base NPC's name.
	Name string
	// NPC names the catalog NPC that supplies face, equipment, and base stats.
	NPC   string
	Type  BossType
	Spawn BossSpawn
	Stats BossStats

	Skills  []BossSkill
	Summons []BossSummon
	Phases  []BossPhase
	Drops   []BossDrop

	// SpawnMessage and DeathMessage reach only nearby encounter participants.
	SpawnMessage string
	DeathMessage string

	// AreaReward is dropped for nearby players; Drops go to the final attacker.
	AreaReward BossAreaReward
	// SourceFile is filled by the loader for diagnostics.
	SourceFile string
}

// Validate rejects invalid content at startup rather than spawning an inert boss.
func (c *BossConfig) Validate() error {
	where := c.SourceFile
	if where == "" {
		where = c.ID
	}
	if strings.TrimSpace(c.ID) == "" {
		return fmt.Errorf("%s: boss has no ID", where)
	}
	if strings.TrimSpace(c.NPC) == "" {
		return fmt.Errorf("%s: boss %q has no base NPC", where, c.ID)
	}
	if !c.Type.Valid() {
		return fmt.Errorf("%s: boss %q has invalid type %q (use chaser, caster, summoner or phased)",
			where, c.ID, c.Type)
	}
	if c.Spawn.X == 0 || c.Spawn.Y == 0 {
		return fmt.Errorf("%s: boss %q has no spawn position", where, c.ID)
	}
	if c.Spawn.X > MaxBossCoordinate || c.Spawn.Y > MaxBossCoordinate {
		return fmt.Errorf("%s: boss %q is outside the map at (%d,%d)", where, c.ID, c.Spawn.X, c.Spawn.Y)
	}
	if c.Spawn.RespawnSeconds < 0 || int64(c.Spawn.RespawnSeconds) > MaxBossDurationSeconds {
		return fmt.Errorf("%s: boss %q has an unsafe respawn delay %d", where, c.ID, c.Spawn.RespawnSeconds)
	}

	// A caster without skills cannot cast anything.
	if c.Type == BossTypeCaster && len(c.Skills) == 0 {
		return fmt.Errorf("%s: boss %q is a caster but has no skills", where, c.ID)
	}
	if c.Type == BossTypeSummoner && len(c.Summons) == 0 {
		return fmt.Errorf("%s: boss %q is a summoner but has no summons", where, c.ID)
	}
	if c.Type == BossTypePhased && len(c.Phases) == 0 {
		return fmt.Errorf("%s: boss %q is phased but has no phases", where, c.ID)
	}
	if len(c.Skills) > MaxBossSkills || len(c.Summons) > MaxBossSummonGroups ||
		len(c.Phases) > MaxBossPhases || len(c.Drops) > MaxBossDrops {
		return fmt.Errorf("%s: boss %q exceeds safe collection limits (skills=%d/%d summons=%d/%d phases=%d/%d drops=%d/%d)",
			where, c.ID, len(c.Skills), MaxBossSkills, len(c.Summons), MaxBossSummonGroups,
			len(c.Phases), MaxBossPhases, len(c.Drops), MaxBossDrops)
	}
	for field, message := range map[string]string{
		"name": c.Name, "spawn_message": c.SpawnMessage, "death_message": c.DeathMessage,
	} {
		if len([]byte(message)) > MaxBossMessageBytes {
			return fmt.Errorf("%s: boss %q %s exceeds %d bytes", where, c.ID, field, MaxBossMessageBytes)
		}
	}

	for i, skill := range c.Skills {
		if skill.ID < 0 {
			return fmt.Errorf("%s: boss %q skill[%d] has a negative ID", where, c.ID, i)
		}
		if skill.CooldownSeconds <= 0 || int64(skill.CooldownSeconds) > MaxBossDurationSeconds ||
			skill.Range < 0 || skill.Range > MaxBossSkillRange {
			return fmt.Errorf("%s: boss %q skill[%d] has unsafe cooldown/range (%d/%d)",
				where, c.ID, i, skill.CooldownSeconds, skill.Range)
		}
		if skill.MaxHPPercent < 0 || skill.MaxHPPercent > 100 {
			return fmt.Errorf("%s: boss %q skill[%d] has max_hp_percent %d outside [0,100]",
				where, c.ID, i, skill.MaxHPPercent)
		}
		if len([]byte(skill.Message)) > MaxBossMessageBytes {
			return fmt.Errorf("%s: boss %q skill[%d] message exceeds %d bytes", where, c.ID, i, MaxBossMessageBytes)
		}
	}
	for i, summon := range c.Summons {
		if strings.TrimSpace(summon.NPC) == "" {
			return fmt.Errorf("%s: boss %q summon[%d] has no NPC", where, c.ID, i)
		}
		if summon.Count <= 0 || summon.Count > MaxBossSummonPerCast {
			return fmt.Errorf("%s: boss %q summon[%d] has invalid count %d", where, c.ID, i, summon.Count)
		}
		if summon.MaxAlive < 0 || summon.MaxAlive > MaxBossSummonedAlive {
			return fmt.Errorf("%s: boss %q summon[%d] has max_alive %d outside [0,%d]",
				where, c.ID, i, summon.MaxAlive, MaxBossSummonedAlive)
		}
		if summon.MaxAlive > 0 && summon.MaxAlive < summon.Count {
			return fmt.Errorf("%s: boss %q summon[%d]: max_alive (%d) is below count (%d)",
				where, c.ID, i, summon.MaxAlive, summon.Count)
		}
		if summon.CooldownSeconds <= 0 || int64(summon.CooldownSeconds) > MaxBossDurationSeconds {
			return fmt.Errorf("%s: boss %q summon[%d] has invalid cooldown %d", where, c.ID, i, summon.CooldownSeconds)
		}
		if len([]byte(summon.Message)) > MaxBossMessageBytes {
			return fmt.Errorf("%s: boss %q summon[%d] message exceeds %d bytes", where, c.ID, i, MaxBossMessageBytes)
		}
	}

	seenPhase := make(map[int]struct{}, len(c.Phases))
	for i, phase := range c.Phases {
		if phase.HPPercent <= 0 || phase.HPPercent >= 100 {
			return fmt.Errorf("%s: boss %q phase[%d] has hp_percent %d outside (0,100)",
				where, c.ID, i, phase.HPPercent)
		}
		if _, repeated := seenPhase[phase.HPPercent]; repeated {
			return fmt.Errorf("%s: boss %q has two phases at hp_percent %d",
				where, c.ID, phase.HPPercent)
		}
		seenPhase[phase.HPPercent] = struct{}{}
		if phase.Type != "" && !phase.Type.Valid() {
			return fmt.Errorf("%s: boss %q phase[%d] has invalid type %q", where, c.ID, i, phase.Type)
		}
		if phase.ShieldPercent < 0 || phase.ShieldPercent > 100 {
			return fmt.Errorf("%s: boss %q phase[%d] has shield_percent %d outside [0,100]",
				where, c.ID, i, phase.ShieldPercent)
		}
		// A shield tied to adds requires adds that can be killed.
		if phase.ShieldUntilAddsDead && len(c.Summons) == 0 {
			return fmt.Errorf("%s: boss %q phase[%d] uses shield_until_adds_dead without summons: the shield cannot fall",
				where, c.ID, i)
		}
		if len([]byte(phase.Message)) > MaxBossMessageBytes {
			return fmt.Errorf("%s: boss %q phase[%d] message exceeds %d bytes", where, c.ID, i, MaxBossMessageBytes)
		}
	}

	for i, drop := range c.Drops {
		if drop.Item == 0 {
			return fmt.Errorf("%s: boss %q drop[%d] has no item", where, c.ID, i)
		}
		if drop.ChancePercent <= 0 || drop.ChancePercent > 100 {
			return fmt.Errorf("%s: boss %q drop[%d] has chance %d outside (0,100]",
				where, c.ID, i, drop.ChancePercent)
		}
		if drop.Amount < 0 || drop.Amount > MaxBossDropAmount {
			return fmt.Errorf("%s: boss %q drop[%d] amount %d outside [0,%d]",
				where, c.ID, i, drop.Amount, MaxBossDropAmount)
		}
	}

	// Area rewards require both item and amount; partial configuration is an error.
	if (c.AreaReward.Item == 0) != (c.AreaReward.Amount == 0) {
		return fmt.Errorf("%s: boss %q has incomplete area_reward (item=%d amount=%d)",
			where, c.ID, c.AreaReward.Item, c.AreaReward.Amount)
	}
	if c.AreaReward.Amount < 0 || c.AreaReward.Amount > MaxBossAreaReward {
		return fmt.Errorf("%s: boss %q has area_reward amount %d outside [0,%d]",
			where, c.ID, c.AreaReward.Amount, MaxBossAreaReward)
	}
	return nil
}

// RespawnDelay returns the respawn interval; zero disables automatic respawn.
func (c *BossConfig) RespawnDelay() time.Duration {
	if c.Spawn.RespawnSeconds <= 0 {
		return 0
	}
	return time.Duration(c.Spawn.RespawnSeconds) * time.Second
}

// BossCatalog is the set loaded from data/content/boss/.
type BossCatalog struct {
	Bosses []BossConfig
}

// Validate rejects duplicate encounter IDs across files.
func (c *BossCatalog) Validate() error {
	seen := make(map[string]string, len(c.Bosses))
	for i := range c.Bosses {
		boss := &c.Bosses[i]
		if err := boss.Validate(); err != nil {
			return err
		}
		if other, repeated := seen[boss.ID]; repeated {
			return fmt.Errorf("boss %q is defined in both %s and %s", boss.ID, other, boss.SourceFile)
		}
		seen[boss.ID] = boss.SourceFile
	}
	return nil
}
