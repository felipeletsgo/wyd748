# data/boss — boss encounters

Each `.lua` file in this directory defines **one** boss. The server loads all
`.lua` files in alphabetical order at startup.

The four `exemplo_*.lua` files are **active** — one boss of each type. To disable
one, remove its file or change its extension (only `.lua` files are loaded).

An error in any file **stops startup** intentionally. Failing immediately with
a message identifying the field is better than running with a boss that never
spawns or is unexpectedly weak because of a typo.

## How it works

- **Behavior is hardcoded in Go** (`internal/game/boss_types.go`). Lua selects
  the type by name and supplies numbers. A Lua file cannot invent a new rule
  or alter server state.
- **Appearance and base stats come from the NPC** named by `npc` (face,
  equipment, and base attributes come from `data/npcs/<name>.json`). The `stats`
  block overrides selected values; omitted values retain the NPC defaults.
- **Prefer an NPC that `NPCGener.txt` does not spawn.** Reusing the appearance
  of an ordinary or quest mob (`Immort_Hydra`, `Aparicao`, `Elf_Servant1`, etc.)
  can confuse players. The catalog contains 491 NPCs, many of which are not
  spawned and are available as distinct boss appearances. Examples include
  `Demon_Lord__` (level 400), `Dark_Shadow___` (400), `Astaroth`,
  `Cyclop_Arch` (200), `Fire_Golem` (218), and `E-Dracolich`. To list unused
  NPCs:

  ```bash
  comm -23 <(ls data/npcs/*.json | sed 's|.*/||; s|\.json$||' | sort) \
           <(tr -s ' \t' '\n' < data/NPCGener.txt | grep -oE '^[A-Za-z_.][A-Za-z0-9_.]*$' | sort -u)
  ```

  Adds (`summons`) are different: using ordinary mobs is expected because
  the adds are ordinary mobs.
- **Bosses do not go through NPCGener.** This file controls spawn position and
  respawn. An ordinary world mob does not become a boss.
- **HP does not regenerate.** Damage persists: bosses are intended to have
  very high HP and may take hours or multiple sessions to defeat. Only death
  creates a new instance after `respawn_seconds`.
- **Pursuit uses the game's standard behavior.** Acquisition, leash, and
  attack ranges are the same as for other mobs and are not configurable here.
  The leash of 16 is intentional; a larger leash would let the boss roam too
  far from its origin.
- The base NPC's `carry` is ignored; boss drops come from `drops`.

## Sandbox

The interpreter runs **without** `os`, `io`, `package`, `debug`, `dofile`,
`loadfile`, or `load`, and has a five-second limit per file. Safe libraries
(`string`, `table`, `math`) remain available, so expressions such as
`max_hp = 500 * 1000` and loops that build lists are supported.

## Types

| Type | Behavior | Required field |
| --- | --- | --- |
| `chaser` | Pursues and attacks in melee. | — |
| `caster` | Casts configured skills when damaged. | `skills` |
| `summoner` | Summons groups of adds during combat. | `summons` |
| `phased` | Changes behavior at HP thresholds. | `phases` |

The type determines what is **required**, not what is permitted: a `phased`
boss can also have `skills` and `summons` and receives those behaviors.

## Fields

```lua
return {
  id   = "unique_identifier",      -- required; unique across files
  npc  = "NPC_Name",               -- required; must exist in data/npcs
  name = "Display name",           -- optional; empty uses the NPC name
  type = "phased",                -- chaser | caster | summoner | phased

  spawn = {
    x = 2100, y = 2100,           -- required; neither may be 0
    respawn_seconds = 1800,       -- 0 or absent means no respawn
  },

  stats = {                       -- all optional; omitted values use NPC stats
    level = 300, max_hp = 500000,
    attack = 900, defense = 450,
    attack_run = 0x64,
    exp_reward = 2000000, gold = 50000,
  },

  skills = {                      -- used by caster
    { id = 23, cooldown_seconds = 8, range = 4,   -- id/range: SkillData.csv
      max_hp_percent = 100,       -- cast only at HP <= this value; 100 = always
      message = "text" },
  },

  summons = {                     -- used by summoner
    { npc = "Skeleton", count = 4,
      max_alive = 8,              -- concurrent cap; 0 = no cap
      cooldown_seconds = 30, message = "text" },
  },

  phases = {                      -- used by phased
    { hp_percent = 60,            -- threshold crossed from above
      type = "summoner",          -- empty retains the top-level type
      shield_percent = 100,       -- 0 = no shield; 100 = immune
      shield_until_adds_dead = true,
      message = "text" },
  },

  drops = {                       -- boss-specific rewards beyond EXP/gold
    { item = 697, chance_percent = 100, amount = 3 },   -- item: itemlist.csv
  },

  spawn_message = "text",         -- announced to the world when spawned
  death_message = "text",
}
```

An unknown field is an **error**, not a warning: `atack = 500` would otherwise
be silently ignored and produce an unexpectedly weak boss.

## Check the IDs

The server validates *structure*, but cannot determine whether an item or
skill is appropriate:

- **Skills** (`data/SkillData.csv`): use **offensive** magic. Skill 27 is
  `Cura` and 26 is `Flash`; configuring them would make the boss "attack" by
  healing. Possible offensive examples are 23 `Tempestade_de_Gelo` (range 4),
  28 `Choque_Divino` (5), 7 `Destino` (4), and 22 `Exterminar` (3). These are
  existing skill-data names, not text to translate in this guide.
- **Items** (`data/itemlist.csv`): a nonexistent index produces a drop the
  client cannot render. Check the index with:

  ```bash
  awk -F, -v i=697 '$1==i {print $1" = "$2}' data/itemlist.csv
  ```
