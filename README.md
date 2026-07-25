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

You can play the game from start to end with the patched 7.48 client. The server
connects these functions through the native protocol:

- account creation and character creation;
- login and world entry;
- progression, combat, and loot;
- data storage.

The server processes more than 80 packet types.

## Systems

The server has these systems. The server has authority on each system.

- **Accounts** — The server hashes each password with PBKDF2. It gives an HTTP
  signup interface and a local command-line tool. It refuses a second login of
  the same account. The server does not keep a password as plain text.
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
  only at spawn or at view entry.
- **Combat** — The server calculates melee and range attacks, PvE and PvP,
  physical and magic damage, resistances, buffs, debuffs, regeneration, death,
  resurrection, and a collision-safe recall. The server does all these
  calculations.
- **Progression** — The server uses the native Mortal experience table. You can
  configure the experience floor and rate. The server does the level-up. It
  controls the statistic, mastery, and skill-point budgets. It applies the
  equipment requirements and the item bonuses.
- **Skills** — The server has code paths for the 104-skill catalog. The four
  classes are TK, Foema, BeastMaster, and Huntress. The catalog includes
  multi-target magic, healing, buffs, transformations, BeastMaster summons, and
  the five Sephira skills.
- **Party** — A party can have a maximum of 13 players. The party has a chat
  channel. Each near member gets the full experience and a configurable bonus
  for each member.
- **Guilds** — The server does guild creation, invitation, acceptance, exit,
  expulsion, and leader succession. It gives a guild chat channel. It keeps the
  membership and writes the native `Guilds.txt` name list. Guild war and the
  visible guild mark are future work.
- **Kingdoms** — A player selects Akelonia or Hekalotia at the kings. The server
  applies the Sapphire cost. The player leaves through the broker. The server
  does the realm teleports, the guild affiliation, and the Basic, Knight, Elite,
  Hero, and Master cape steps. The realm war is a later phase.
- **NPCs and economy** — The server reads NPCGener. It controls regional
  visibility, shops, skill masters, server-side prices, and drops. Each player
  has 63 usable inventory slots and 120 Cargo slots. The server does player
  trade and prevents item duplication in the data storage.
- **Crafting** — The server has server-side recipes and native success and
  failure messages for Agatha, Aylin, Tiny, Lindy, Compositor, Ehre, and
  Alquimista Odin.
- **Quests** — The server reads the quest requirements and gives atomic rewards
  from data files. The quest areas are repeatable. The server does a 10-minute
  area reset and gives the native quest reward boxes. The full retail quest
  catalog is not complete.
- **Volatiles** — You can configure potions, gold, teleports, refining and
  tinting, mount items, timed affects, the Magical Pill, the Hunting Scrolls,
  the summon contracts, and the Sephira books. A deferred code stays a generic
  handler that does not consume the item.
- **Mounts** — An egg incubates and hatches. A hatchling grows into an adult.
  The pet follows its owner outside the party. A mounted character absorbs some
  damage. The server calculates hunger and longevity.
- **Player shops** — A Ghost Shop sells Cargo items in Armia. A stationary clone
  does the sale. The owner stays free to move and to play.
- **Ascension (Arch)** — A Mortal of level 371 or higher can become an Arch.
  The chain has three steps: the class skill master makes the Sefirot from
  eight Sephira stones, the Black Oracle forges the Eternal Stone, and the king
  does the ascension. The Arch is a new character in a free slot; the Mortal
  stays. The Arch gets more attribute points if the Mortal has a higher level,
  and the server calculates this again at each login.
- **Boss encounters** — A boss is a normal mob with a parallel behavior
  runtime. Four behaviors are compiled into the server: chaser, caster,
  summoner, and phased. You configure each encounter in a Lua file in
  `data/boss/`. The file sets the assets, the statistics, the position, the
  respawn time, the skills, the adds, the HP phases, and the drops. The Lua
  file selects a behavior; it cannot make a new rule.
- **Communication** — The protocol carries local, party, guild, and global chat.
  It carries whisper, death letters, and server announcements. A message needs
  the other player online. If that player is offline, the server tells you.
- **Data storage** — The server writes JSON files atomically. It writes to a
  temporary file, does an fsync, and does a rename. An asynchronous queue keeps
  the disk operations away from the game loop. An autosave runs at a regular
  interval. A critical operation, for example a trade, a refine, or a gold
  change, writes to disk before the server confirms it to the client. If the
  write fails, the server returns the data to its previous state.

## Language

The 7.48 client is the global (English) client, so all the text that the
player reads is English. The terms agree with the client interface: Party,
Guild, Whisper, Trade, Quest, Gold, Kingdom, Cargo, Mount, Refine, and Auto
Trade. The chat commands accept two languages: `/create` and `/criar`,
`/invite` and `/convidar`, and so on.

## Requirements

- You must have Go 1.26 or a later version.
- You must have Windows to run the client in `client748/`.
- The server uses one external Go module: `gopher-lua` (MIT). It reads the
  boss files. `go build` downloads it.

## Build the software

Build the three programs. Do these commands:

```powershell
go build -o tm.exe ./cmd/server
go build -o account-api.exe ./cmd/account-api
go build -o account-create.exe ./cmd/account-create
```

## Start the server

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
replaces a value in that file. Examples: `-addr`, `-npcs`, `-accounts`,
`-items`. To see all the flags, do `./tm.exe -h`.

To monitor the server, set `debug_address` in `data/server.txt`. The server
then gives metrics at `/debug/vars` and profiles at `/debug/pprof`. The host
must be loopback: these pages show internal state. If you give a public
address, the server refuses to start. For remote access, use an SSH tunnel.

The server writes all the accounts to disk before it stops. Send SIGTERM or
press Ctrl+C.

Start the server from the `wyd-go/` directory. Then the server finds the
`data/...` paths.

You make an account with one of two tools:

- Use `account-create`, the local command-line tool.
- Or start `account-api` and send an HTTP signup request.

## Do the static checks

Do these commands:

```powershell
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
```

## Architecture

```text
cmd/server        composition and configuration
internal/model    pure domain types
internal/wire     byte-exact 7.48 protocol frames and encryption
internal/net      sockets, sessions, one send queue for each connection
internal/data     catalogs, NPC spawns, terrain, templates
internal/store    account and character data storage (atomic JSON)
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
| `SkillData.csv` | skill cost, delay, target type, range, parameters |
| `NPCGener.txt` + `npcs/*.json` | NPC and mob spawns, statistics, shop items |
| `character_templates.json` | start statistics and items for each class |
| `volatiles.json` | server behavior for each consumable item-effect code |
| `mounts.json` | combat bonus for each mount type |
| `quests.json` | quest definitions and prerequisites |
| `quest_zones.json` | timed quest-area limits and reset behavior |
| `boss/*.lua` | one boss encounter for each file |
| `guilds.json` + `Guilds.txt` | guild registry and 7.48 client name list |
| `droprate.json` | loot table weights |
| `server.txt` | server configuration and gameplay rules |

## Roadmap

These are the next gameplay phases:

- the remaining retail quest catalog;
- the Celestial evolution, after the Arch;
- the visible guild mark;
- the guild war, the kingdom war, and the Castle war;
- more work on the client assets.

The current build does not have these functions complete.

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
