# WYD-Go

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Go](https://img.shields.io/badge/Go-1.26+-00ADD8.svg)
![Status](https://img.shields.io/badge/status-playable-brightgreen.svg)

*This document uses ASD-STE100 Simplified Technical English.*

WYD-Go is a game server for **With Your Destiny (WYD)**, client version **7.48**.
The server controls all the game state. The code is new and written in Go.

> This is a non-commercial project. Its purpose is to study three subjects: how
> to analyze a network protocol, how to make a game server, and the Go language.
> Refer to the [Disclaimer](#disclaimer).

The client shows the game state and sends commands. The server keeps the true
state of the inventory, the statistics, the combat, the skills, the progression,
and the data storage. The server does not trust the client. It calculates each
price, target, position, cooldown, drop, and effect again before the effect
starts.

This is an independent build. We got the wire protocol from packet captures of a
real 7.48 client. We ported the gameplay formulas from public WYD server code.
We used only the algorithms. We did not copy structures, memory offsets, or
addresses.

## Status

You can play the game from start to end with the source-built 7.48 client. The server
connects these functions through the native protocol:

- account creation and character creation;
- login and world entry;
- progression, combat, and loot;
- data storage.

The server processes more than 80 packet types.

## Documentation

This README is the public tutorial for the GitHub project. It explains how to
prepare, build, configure, start, and use the server, and it gives an overview
of every available system.

The technical documentation has two canonical files:

- [`DOCS/IMPLEMENTED.md`](DOCS/IMPLEMENTED.md) describes the architecture and
  every system that exists in the current server;
- [`DOCS/ROADMAP.md`](DOCS/ROADMAP.md) is the only roadmap and contains only work that
  is not complete.

The active client workflow is documented in
[`client-source/README.md`](client-source/README.md). Historical executables,
patchers, and their notes stay read-only under
`client748/wyd.exe nativo+patches/` and are not part of the build.

## Systems

- **Server-side security** — The 7.48 client is treated as untrusted. The server
  validates framing, checksum, packet size and session phase; rate-limits input;
  limits pre-auth TCP connections per IPv4 or IPv6 `/64`, deadlines InitCode and partial frames,
  applies a local CIDR policy for VPS/VPN/datacenter ranges, rejects unknown
  C→S opcodes before dispatch, verifies movement routes, terrain
  and line of sight; calculates damage,
  critical hits and cooldowns; and persists item/economy mutations before
  confirming them. Replayed or forged WPE packets cannot become game authority.

The server has these systems. The server has authority on each system.

- **Accounts** — The server hashes each password with PBKDF2. It gives an HTTP
  signup interface and a local command-line tool. It refuses a second login of
  the same account and limits one public IPv4 or IPv6 `/64` to four authenticated game clients.
  Pre-auth socket limits remain separate, so incomplete handshakes cannot use
  the four gameplay slots. The server does not keep a password as plain text.
- **Characters** — Each account can make four characters in four classes. The
  server checks each name. It gives the start layout, items, statistics, and
  spawn position from data files.
- **Extended statistics** — An extension carries 32-bit HP, MP, STR, INT, DEX,
  CON, attack, magic attack, and defense. It keeps the fixed 7.48 packet
  prefixes. The gameplay uses only this extended score. The legacy score is a
  wire projection.
- **World** — The server uses the native height map and attribute map for
  collision. Mob AI sleeps by area. Mobs patrol routes and follow a target. The
  view window is 65 by 65 tiles. The server creates a pet, a summon, or a mob
  only at spawn or at view entry. Player movement preserves the validated
  7.48 route for observers, so remote clients interpolate the same path instead
  of receiving repeated position corrections. The World advances authoritative
  coordinates over time, so a future route destination cannot be used early for
  attacks, pickup, trade, or NPC interaction.
- **Load testing** — `cmd/loadtest` provisions short-lived `bot0001`–`bot0990`
  accounts from the Arch in slot 1 of the `felipe` snapshot with new
  CharacterUIDs and ItemUIDs, then connects them to Tauron spots in Noatum.
  Bots attack every cycle and independently attempt movement. It uses a
  disposable PostgreSQL database and loopback expvar/pprof metrics. Movement
  keeps client prediction separate from the last server-confirmed position and
  reconciles on authoritative 0x366 teleports/corrections.
- **Combat** — The server calculates melee and range attacks, PvE and PvP,
  physical and magic damage, resistances, buffs, debuffs, regeneration, death,
  resurrection, and a collision-safe recall. The server does all these
  calculations. Players, mobs and summons start at 100% accuracy and 0%
  evasion. Defender DEX builds up to 80% evasion at 4,000 points and directly
  lowers the attacker's accuracy, which never falls below 20%; Concentration
  adds 15 accuracy percentage points and raises that floor to 35%. Physical actions use
  a fixed 400 ms interval, while Attack Speed above 100% becomes Double Hit chance and
  reaches 100% Double Hit at 200%. A PvP kill never transfers experience or
  gold from either player. Use `/parry <player>` to inspect the authoritative
  accuracy/evasion matchup against an online character.
- **Progression** — The server uses the native Mortal experience table. You can
  configure the experience floor and rate. The server does the level-up. It
  controls the statistic, mastery, and skill-point budgets. It applies the
  equipment requirements and the item bonuses.
- **Skills** — The server has code paths for the 104-skill catalog. The four
  classes are TK, Foema, BeastMaster, and Huntress. The catalog includes
  multi-target magic, healing, buffs, transformations, BeastMaster summons, and
  the five Sephira skills. Arch, Celestial, and SubCelestial may buy a skill
  regardless of its level requirement; class, skill points, mastery,
  prerequisite chains, and gold remain server-authoritative.
- **Party** — A party can have a maximum of 13 players. The party has a chat
  channel. Each near member gets the full experience and a configurable bonus
  for each member.
- **Guilds** — The server does guild creation, invitation, acceptance, exit,
  expulsion, and leader succession. It gives a guild chat channel. It keeps the
  membership and writes the native `Guilds.txt` name list. The native `0x3D5`
  recruit button is server-authoritative, checks kingdom/rank/gold/cooldown,
  and commits both accounts with the guild. The server now packs the native
  guild id/rank fields in `CreateMob`, `UpdateScore`, and `EnterWorld`, and
  rewrites `Guilds.txt`; this is the complete server side of the 7.48 mark
  flow. The client still downloads `b010000<guild-id>.bmp` from its hardcoded
  mark host, so a public mark requires that asset or an equivalent URL behavior
  implemented in the 7.48 client source.
  Guild war remains future work.
- **Kingdoms** — A player selects Akelonia or Hekalotia at the kings. The server
  applies the Sapphire cost. The player leaves through the broker. The server
  does the realm teleports, the guild affiliation, and the Basic, Knight, Elite,
  Hero, and Master cape steps. The realm war is a later phase.
- **NPCs and economy** — The server reads NPCGener. It controls regional
  visibility, shops, skill masters, server-side prices, and drops. Each player
  has 63 usable inventory slots and 120 Cargo slots. The server does player
  trade and prevents item duplication in the data storage. Selling to a normal
  merchant also fills the native ten-entry `0x3E8` repurchase window; the
  server keeps item UIDs and recalculates the repurchase price from `itemlist`.
  The attribute Skill Master consumes 30 Sapphires or Return Of Ability and
  atomically returns up to 100 distributed points from each attribute.
- **Crafting** — The server has server-side recipes and native success and
  failure messages for Agatha, Aylin, Tiny, Lindy, Compositor, Ehre, and
  Alquimista Odin. The Compositor needs four materials of set D or E, each
  refined +7 to +9. Its success chance comes from `data/server.txt`: a base
  value plus a bonus for each material, selected by the refine level.
- **Quests** — The server reads the quest requirements and gives atomic rewards
  from data files. The quest areas are repeatable. The server does a 10-minute
  area reset and gives the native quest reward boxes. The full retail quest
  catalog is not complete.
- **Volatiles** — You can configure potions, gold, teleports, refining and
  tinting, mount items, timed affects, the Magical Pill, the Hunting Scrolls,
  the summon contracts, the Sephira books, the gate keys, and the complete
  Mystic/Normal/Arcane Water chain (24 regular rooms plus three Nessus rooms).
  Each cleared regular room gives the party leader the next scroll, including
  the boss letter after room eight. Room 8 opens an independent boss instance,
  and a Room 1 scroll can close the cycle during exit grace. Water tickets
  create party instances with physical room exclusion, native two-minute timers,
  mixed boss populations,
  mob counters, transactional rewards, and automatic exit. A gate key finds
  its door by the `EF_KEYID` effect, not by the item index. Magic Chamber,
  Nightmare, timed Hidden Ruins/Service access, mount boxes/revival, event
  passes, medals and economy counters are also server-authoritative. Cube now
  runs its 25 native rooms with timed O/X questions, per-player elimination and
  one consumed invitation per entrant (up to six players in the first room);
  Big Cube has its private O/X platform flow, and Hell Gate runs server-owned
  waves with an absolute deadline and transactional party entry/exit. Water,
  Cube, Big Cube, Nightmare, Hell Gate and Uxmal isolate their participants,
  deadlines and rewards by runtime instance.
  Dead mounts remain equipped for revival but are projected as visually
  unequipped to their owner and nearby players until their HP is restored.
  Diamond, Emerald, Coral and Garnet now apply their native equipment variants
  to +10..+15 gear and to Ancient weapons below +10 without losing refinement.
  Adamantite upgrades compatible equipment through the authoritative item
  catalog, preserving item UID and rolling back atomically on persistence errors.
  Courage applies its fixed PvE hit bonus, and
  Love Chocolate/Candy resolve their native SkillData affects and apply the
  complete package atomically. Item-derived kill EXP bonuses (Coral, active
  fairy and Experience Box) come from the killer and are shared with every
  eligible party member, while receiver-specific evolution reductions remain
  authoritative. Combat EXP follows the W2PP receiver curve: Arch is reduced
  progressively by level, while Celestial/SubCelestial starts at `1/40` and
  becomes progressively slower; Mortal keeps the normal server reward. All
  124 codes are explicitly classified. The
  3443 Spirit's Seal flow is implemented atomically with 0x2CD/0x3CC query and
  extraction. Item 3455 remains no-direct-use because W2PP does not accept it
  in PutoutSeal.
- **Character counters** — A character has named counters, such as the Kefra
  entrance ticket and the fame points. They live in a per-character sidecar
  file, not in the account. A quest can require, spend, or grant them. A
  counter does not pass from one character to another one in the same account.
- **Mounts** — An equipped egg advances with Ori/Lac and a server-side 6–8 hour
  online incubation timer; unequipping or disconnecting pauses it. A hatchling
  grows into an adult and follows its owner outside the party. A mounted
  character absorbs some damage. The server calculates hunger and longevity.
- **Player shops** — A Ghost Shop sells Cargo items in Armia. A stationary clone
  does the sale. The owner stays free to move and to play.
- **Ascension (Arch)** — A Mortal of level 371 or higher can become an Arch.
  The chain has three steps: the class skill master makes the Sefirot from
  eight Sephira stones, the Black Oracle forges the Eternal Stone, and the king
  does the ascension. The Arch is a new character in a free slot; the Mortal
  stays. The Arch gets more attribute points if the Mortal has a higher level,
  and the server calculates this again at each login. An Arch stops at level
  355 and at level 370: it receives no experience until Lindy lifts the limit
  with the retail recipe. The level 370 unlock also costs one fame point. The
  four elemental oaths of Kefra cost 100 million experience each, and you must
  take them in order.
- **Celestial and SubCelestial** — An eligible Arch becomes Celestial in the
  same character slot through the native Ideal Stone chain. Celestial and
  SubCelestial have separate class, experience, attributes, masteries, learned
  skills, skill bar, and affects, while they share identity, inventory,
  equipment, kingdom, fame, quests, Soul configuration, and the native
  cross-form status-point bonus. The server implements the 7.54 level-200
  table, level 40/90 locks, cumulative late-level combat EXP reductions,
  Cythera milestones, Soul of Limits, Fury/Arcana, city-only form switching,
  and the native Spirit's Seal capsule/extraction flow.
  Creating an Arch or Celestial returns the player to character selection and
  refreshes all four slots after persistence, ensuring the 7.48 client reloads
  the evolved body, score, and skills. A Celestial creation is announced to
  every online player in English.
  Their combat score follows the normal W2PP evolution branch: Arch has its own
  HP/MP and defense progression, while Celestial/SubCelestial receive the
  crystal-dependent defense and class HP/MP package, with the 399-level base
  applied only to physical attack.
- **Boss encounters** — A boss is a normal mob with a parallel behavior
  runtime. Four behaviors are compiled into the server: chaser, caster,
  summoner, and phased. You configure each encounter in a Lua file in
  `data/boss/`. The file sets the assets, the statistics, the position, the
  respawn time, the skills, the adds, the HP phases, and the drops. The Lua
  file selects a behavior; it cannot make a new rule.
- **Communication** — The protocol carries local, party, guild, and global chat.
  It carries whisper, death letters, and server announcements. A message needs
  the other player online. If that player is offline, the server tells you.
- **Protocol hardening** — Session phase, packet framing, confirmed 7.48 sizes,
  temporally authoritative movement, authoritative slots and item indices are checked before
  gameplay handlers run. Delete/split item, keyed ground objects, ranking
  lookup, hidden day synchronization, keepalive, and the Huntress Illusion
  Action2 packet have explicit routes and regression tests. Client-sent score
  updates are recognized and discarded.
- **Premium Firecracker** — The original 10×10 drawing panel is server-backed:
  the server validates the real item and authoritative position, commits the
  consumption, then broadcasts the exact custom pattern only to nearby players.
- **Data storage** — PostgreSQL is the authoritative production store. Account,
  character, item, Cargo, character-state, and guild changes use transactions.
  Each materialized item has a server-only UUID with a global database
  constraint; it never enters the native 8-byte client item structure. An
  asynchronous queue keeps periodic saves away from the game loop. A critical
  operation, for example a trade, a refine, or a gold change, commits before the
  server confirms it to the client. A failed commit restores the previous game
  state. Critical transactions share one configurable total deadline across all
  retries. JSON is an explicit development adapter, not an automatic fallback.

## Language

The 7.48 client is the global (English) client, so all the text that the
player reads is English. The terms agree with the client interface: Party,
Guild, Whisper, Trade, Quest, Gold, Kingdom, Cargo, Mount, Refine, and Auto
Trade. The chat commands accept two languages: `/create` and `/criar`,
`/invite` and `/convidar`, and so on.

## Requirements

- You must have Go 1.26 or a later version.
- You must have Windows to run the client in `client748/`.
- You must have PostgreSQL for the production server.
- The server uses `gopher-lua` (MIT) and `pgx` (MIT). `go build` downloads them.

## Build the software

Build the three programs. Do these commands:

```powershell
go build -o tm.exe ./cmd/server
go build -o account-api.exe ./cmd/account-api
go build -o account-create.exe ./cmd/account-create
```

## Prepare PostgreSQL

Install PostgreSQL on the server. On Ubuntu:

```bash
sudo apt update
sudo apt install -y postgresql
sudo -u postgres psql
```

Create the role and the database inside `psql`:

```sql
CREATE ROLE wydgo LOGIN PASSWORD 'replace-this-password';
CREATE DATABASE wydgo OWNER wydgo;
\q
```

Keep PostgreSQL on the local interface. Do not publish port 5432. The game
server installs and validates schema v3 at boot.

## Start the server

Set the database URL before you start a production server:

```powershell
$env:WYD_DATABASE_URL="postgres://wydgo:password@127.0.0.1:5432/wydgo?sslmode=disable"
```

Run the compiled `tm.exe` file. This is the fast method and the correct method
for a real server. First build the file (refer to [Build the software](#build-the-software)).
Then do this command:

```powershell
./tm.exe
```

Use `go run` only for development. This command builds the code again at each
start, so it is slower:

```powershell
go run ./cmd/server
```

The server reads the configuration from `data/server.txt`. A command-line flag
replaces a data-file value. Examples: `-addr`, `-npcs`, and `-items`. The
`-accounts` flag applies only to the explicit JSON development adapter. To see
all the flags, do `./tm.exe -h`.

Operational limits also live there: TCP connections globally/per IP, InitCode,
idle and partial-frame timeouts, inbound packet/byte rates, login and chat
limits, PostgreSQL critical-operation budget, `world_command_queue_capacity`,
`auth_hash_concurrency`, and `channel_id`. Defaults are
safe for the stock 7.48 heartbeat; do not reduce the 600-second idle timeout
without capturing the real client first.

`gameplay_log=summary` is the production-friendly mode: it aggregates combat
and loot counters once per minute instead of synchronously writing one line per
hit. Use `gameplay_log=verbose` only for a short reproduction; `quiet` disables
those hot-path diagnostics while preserving errors and security events.

To monitor the server, set `debug_address` in `data/server.txt`. The server
then gives metrics at `/debug/vars` and profiles at `/debug/pprof`. The host
must be loopback: these pages show internal state. If you give a public
address, the server refuses to start. For remote access, use an SSH tunnel.

The server flushes all pending account transactions before it stops. Send
SIGTERM or press Ctrl+C. The database schema is installed automatically at
boot. Keep port 5432 private and use `pg_dump` for backups.

Start the server from the `wyd-go/` directory. Then the server finds the
`data/...` paths.

You make an account with one of two tools:

- Use `account-create`, the local command-line tool:

  ```powershell
  ./account-create.exe
  ```

  You can preset only the username. The tool always reads the password from the
  terminal:

  ```powershell
  ./account-create.exe -username felipe
  ```

- Or start the HTTP API on loopback:

  ```powershell
  ./account-api.exe -addr 127.0.0.1:8080
  ```

  Create an account:

  ```powershell
  Invoke-RestMethod -Method Post `
    -Uri http://127.0.0.1:8080/v1/accounts `
    -ContentType application/json `
    -Body '{"username":"felipe","password":"123456","passwordConfirmation":"123456"}'
  ```

  `/healthz` reports process health and `/readyz` verifies the database. Publish
  this API only through an HTTPS reverse proxy that replaces forwarded headers.

## Build and verify the client

The supported 7.48 client is built exclusively from `client-source/tmproject`.
The executables and PowerShell patchers under
`client748/wyd.exe nativo+patches/` are read-only historical material for
Ghidra and must never be executed, edited, or used as a validation gate.

Validate the active assets and build the source with:

```powershell
cd client-source/tmproject
.\Test-Client748Assets.ps1 -AssetRoot ..\..\client748
.\Build-Client.ps1 -Configuration Release
```

Every successful build automatically installs `client748/project.exe`, verifies
that its hash matches the transient build output, and prints the installed hash.
An installation or hash failure fails the build workflow.

Every client behavior change must be adapted in the 7.48 source; visual data
changes belong in the assets. Configure the client connection for the public
address of your server, keep TCP port 8281 allowed in the game host firewall,
and keep PostgreSQL and the diagnostic HTTP endpoint private.

The client package contains 135 costumes imported from the supplied current KR
clients. Their exact male, female, or dynamic body mapping and per-part assets
must be consumed by the source-built client. Five `ShopCostum*` merchants in
Armia expose at most 27 costumes each. Every costume grants 80 defense and 10%
mana saving and expires 30 calendar days after its first equip.

It also contains 59 KR mount appearances with complete source dependencies ported as standalone 7.48 mounts.
They set movement speed to the server maximum of 6, grant +520 physical damage and +65% magic attack, and
expire 30 calendar days after first equip. `ShopKRMt01`, `ShopKRMt02`, and
`ShopKRMt03` expose them in Armia (26 + 19 + 14 items).

Premium deadlines are server-authoritative metadata bound to the item's UID.
They continue while the item is unequipped or the account is offline, never
restart on trade or relogin, and are not trusted from the 7.48 packet. Fairies
use the same absolute UID-bound expiration system.

## Do the static checks

Do these commands:

```powershell
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
```

For the final Linux/CI validation, also run:

```bash
go test -race ./...
```

## Run the load test

Use a separate disposable PostgreSQL database. Never provision bots in the
production database. Define the test database, source database, and bot
password:

```powershell
$env:WYD_LOADTEST_DATABASE_URL="postgres://wydgo:password@127.0.0.1:5432/wydgo_loadtest?sslmode=disable"
$env:WYD_SOURCE_DATABASE_URL="postgres://wydgo:password@127.0.0.1:5432/wydgo?sslmode=disable"
$env:WYD_LOADTEST_BOT_PASSWORD="123456"
```

Provision clones from slot 1 of the source account and then connect them:

```powershell
go run ./cmd/loadtest `
  -source-database-url-env WYD_SOURCE_DATABASE_URL `
  -source-account felipe -source-slot 1 `
  -bots 990 -reset -provision -provision-only

go run ./cmd/loadtest `
  -server 127.0.0.1:8281 -bots 990 `
  -ramp 1m -duration 5m -move-percent 50
```

The source account is never used as a bot. Every clone receives independent
CharacterUIDs and ItemUIDs. You can log in with the real source account while
the test is active.

## Architecture

```text
cmd/server        composition and configuration
internal/model    pure domain types
internal/wire     byte-exact 7.48 protocol frames and encryption
internal/net      sockets, sessions, one send queue for each connection
internal/data     catalogs, NPC spawns, terrain, templates
internal/store    PostgreSQL storage and the explicit JSON development adapter
internal/account  signup, authentication, and password hashing
internal/game     the World actor and each game system
```

One `World` goroutine keeps all the changeable state. The sessions decode the
socket. They send commands to the game loop. The game loop does the movement,
the combat, the items, the party actions, and the ticks. It does them in a
strict sequence.

This sequence gives a correct order for the inventory, the loot, the trade, and
the combat. It prevents a large group of duplication bugs and race bugs. No
other code changes the game state.

The `internal/game` package has many files, one file for each function: combat,
equipment, skills, affects, party, movement, drops, trade, teleports, NPC shops,
mounts, quests, and guilds. It is not one large file.

## Data files

The game content is in the `data/` directory. The files are CSV, JSON, and text.
The server reads these files when it starts. The running server does not change
them.

| File | Function |
|---|---|
| `itemlist.csv` | item catalog: statistics, requirements, static effects |
| `Itemname.csv` | authoritative item display names |
| `ItemEffect.h` | authoritative numeric IDs for persisted `EF_*` effects |
| `SkillData.csv` | skill cost, delay, target type, range, parameters |
| `NPCGener.txt` + `npcs/*.json` | NPC and mob spawns, statistics, shop items |
| `character_templates.json` | start statistics and items for each class |
| `volatiles.json` | server behavior for each consumable item-effect code |
| `instances.json` | instance templates, rooms, waves, tickets, and state machines |
| `repliction.json` | native paired bonus pools and A–E Repliction limits |
| `mounts.json` | combat bonus for each mount type |
| `quests.json` | quest definitions and prerequisites |
| `quest_zones.json` | timed quest-area limits and reset behavior |
| `init_items.csv` | permanent world objects: gates, doors, cannons |
| `charstate/<characterUID>.json` | JSON-dev sidecar: affects and named counters; production uses PostgreSQL |
| `boss/*.lua` | one boss encounter for each file |
| `guilds.json` + `Guilds.txt` | guild registry and 7.48 client name list |
| `droprate.json` | loot table weights |
| `network_admission.json` | local allow/deny/per-network client limits for VPS, VPN, and datacenter CIDRs |
| `server.txt` | server configuration and gameplay rules |

`go run ./cmd/network-admission` compiles reviewed text/JSON CIDR feeds into
`network_admission.json` without adding network access to the login path.

## Roadmap

[`DOCS/ROADMAP.md`](DOCS/ROADMAP.md) is the only project roadmap. The next recommended
implementation is the transactional economy ledger. The later phases cover
in-game validation, the remaining retail quests, the native Big Cube table,
client HP/MP and guild-mark work, public-server operations, and the war systems.
Guild War, Kingdom War, and Castle War stay as the final gameplay phase.

## Disclaimer

WYD-Go is an independent, **non-commercial** project. Its purpose is education.
It helps you to learn how to analyze a network protocol, how to make a game
server, and the Go language. The owners of *With Your Destiny* do not authorize
this project and are not related to it.

*With Your Destiny* and all related names, logos, artwork, trademarks, and game
data are the property of their owners.

This repository includes server data files and a client bundle in `client748/`.
The server needs these files to work with version 7.48. The original game data,
the client, the artwork, the audio, and the map assets are not original work.
They stay the property of the game owners. This project includes them only for
study.

If you own the rights to a file in this repository and want the removal of that
file, open an issue. We will remove it quickly.

## License

This project uses the **GNU General Public License v3.0**. Refer to
[LICENSE](LICENSE).

You can use, study, change, and distribute this code under the GPLv3. A
distributed derivative must keep the same license. It must also keep its source
code available.
