# Server data layout

`wydgo748/data/` holds the authored inputs of the game server. The server reads
them at startup. It never writes to this directory. The root contains only the
six areas below; `tools/repository/Test-RepositoryLayout.ps1` rejects loose files.

```text
wydgo748/data/
├─ config/    operational profiles; values change per environment
├─ catalog/   native 7.48 tables that must match the client
├─ world/     terrain maps and fixed world objects
├─ npcs/      one JSON template per NPC or monster
├─ spawns/    generator files: where and how many entities appear
└─ content/   gameplay rules and event definitions
```

## Rules

- Classify a file by what it is, not by its format.
- Keep native file names (`itemlist.csv`, `NPCGener.txt`, `SkillData.csv`).
  Only their directory belongs to this layout.
- Put files the server writes while running outside `data/`. The local
  `Guilds.txt` export goes to `wydgo748/runtime/`, which Git ignores.
- Persist accounts, characters, and guilds in PostgreSQL. `data/` has no
  account or character state.
- Change a path in one place: the default in
  `wydgo748/internal/data/server_config.go`, then the profiles in
  `data/config/` and `tools/deployment/vps/server.txt`.

## Areas

| Area | Files | Configuration key |
|---|---|---|
| `config/` | `server.txt` (local profile), `server-loadtest.txt` | `-config` flag |
| | `staff.json` (local staff accounts) | `web_admin_staff` |
| | `network_admission.json` | `network_admission` |
| | `client_integrity.json` | `client_integrity` |
| `catalog/` | `itemlist.csv`, `Itemname.csv`, `ItemEffect.h`, `SkillData.csv` | `items`, `itemnames`, `itemeffects`, `skills` |
| `world/` | `maps/HeightMap.dat`, `maps/AttributeMap.dat` | `heightmap`, `attributemap` |
| | `teleports.ini` | `teleports` |
| | `init_items.csv` (gates, doors, cannons, towers) | `init_items` |
| `npcs/` | `<template>.json`, shared by both channels | `npcs` |
| `spawns/` | `NPCGener.txt` (population of both channels) | `npcgener` |
| | `NPCGenerTest.txt` (opt-in test population) | `-gener-extra` flag |
| | `instances/*.txt` (Carta, Coliseu, Cube, Hell Gate, Nightmare, Rune Track) | `instance_generators` |
| | `events/RvR.txt` (inactive source groups; not loaded or packaged) | none until the event is implemented |
| `content/` | `quests.json`, `quest_zones.json` | `quests`, `quest_zones` |
| | `droprate.json`, `repliction.json`, `mounts.json`, `character_templates.json` | key of the same name |
| | `volatiles.json`, `instances.json` | `volatiles`, `instances` |
| | `boss/*.lua` (one encounter per file) | `boss` |

Generator references inside `content/instances.json` are file names relative to
`instance_generators` (for example `"CartaA.txt"`). The Colosseum loads
`ColiseuN/M/A.txt` from the same directory.

Keep Rune Track generators in `instances/RuneTrack0.txt` through `RuneTrack6.txt`.
The `uxmal.roomGenerators` array maps these files to entry refinements +0 through +6.
Keep each room's generators in its own file; reject misplaced or duplicate labels.
The general population must not reuse their spawn origins, even under new labels.
Startup reserves these origins before public registration and population creation.
Keep Cube generators exclusively in `instances/CubeN.txt`, `CubeM.txt`, and
`CubeA.txt`, with 25 rooms per tier. Each stage selects its own file and block.
The loader reads definitions at startup; only instance activation creates mobs.
Room teardown removes the owned population. These files never join public respawn.
Keep the public Arnold merchant; its Cube header does not make it a combat wave.
Armia invasion groups (`Inv_Gremilin` and `Inv_Lobo`) are removed; this event
has no active lifecycle in this project.

Cube stages use `generatorMode: "initial_group"`. This mode requires fixed
`MinGroup`/`MaxGroup`, and expands one leader plus that many followers, capped
by `MaxNumMob`. Different leader and follower templates remain separate spawns.
The Normal final group has one `OrcTrooper@` and five `Carbunkle@` followers;
its capacity of 100 does not mean 100 creatures per activation. Other generator
consumers retain their existing capacity-based expansion when this mode is absent.

Keep the RvR population outside the general file until its lifecycle exists.
RvR means Realm versus Realm: Hekalotia (red) against Akelonia (blue).
Preserved RvR routes do not define a schedule or authorize automatic spawning.

`internal/npccontent` validates explicit template references after content loads.
Missing references fail initialization with their content path. Display text and
other metadata never select templates. See [live NPC management](../protocol/npc-definitions.md)
for channel scopes and the PostgreSQL editing contract.

## Consumers outside the game server

- `cmd/web-export` and `internal/webcatalog` read `catalog/` and
  `content/quests.json`, `content/quest_zones.json` from the data root. The
  exported `sources[].file` values are paths relative to that root.
- `cmd/player-api -data-dir` receives the data root and reads `catalog/`.
- `tools/deployment/Build-VpsRelease.ps1` packages runtime inputs, including
  network admission and client integrity JSON from `config/`. The VPS profile
  and staff file ship from `tools/deployment/vps/`.
- `tools/client-assets/Sync-TerrainMaps.ps1` writes `world/maps/`.

## Deployment compatibility

Use a full release for the first migration to this layout. Update the game,
player API, and operational profile together. Preserve private settings explicitly.

Packages include `game/data-layout.version`. Game-only activation requires the
installed and incoming markers to match. It also checks configured data paths
against the incoming archive before backup, replacement, or restart. Missing
markers, obsolete keys, or missing configured inputs require a full release.

For an authorized package from current files, run `Build-VpsRelease.ps1 -WorkingTree`.
This mode includes allowed untracked runtime files and excludes deleted tracked files.
It excludes ignored files, backups, logs, and private local profiles.
Use `AdditionalRuntimeData` only for explicitly selected allowed inputs.
Tracked mode rejects missing tracked inputs before building.

Each package includes `release-manifest.json` beside the archive.
It records HEAD, dirty paths, source hashes, removed inputs, build settings,
payload hashes, configuration hashes, and the archive checksum.
Upload the complete staging bundle for the first layout migration.

## Classification

`MODERNIZACAO_COMPATIVEL`: the layout changes only server-side paths. It does not
change packets, persisted values, resource IDs, or the client.
