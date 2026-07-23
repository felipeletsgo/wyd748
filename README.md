# WYD-Go

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Go](https://img.shields.io/badge/Go-1.26+-00ADD8.svg)
![Status](https://img.shields.io/badge/status-playable-brightgreen.svg)

A server-authoritative emulator for **With Your Destiny (WYD)**, client version
**7.48**, written from scratch in Go.

> A non-commercial project built to study network-protocol reverse-engineering,
> game-server architecture, and Go. See the [Disclaimer](#disclaimer).

The client renders state and sends intent; the server is the sole source of
truth for inventory, stats, combat, skills, progression and persistence. No
price, target, position, cooldown, drop, or effect is ever trusted from the
client — everything is revalidated against server-side state before it takes
effect.

This is an independent reimplementation: the wire protocol was reverse-engineered
from packet captures against a real 7.48 client, and gameplay formulas were
ported (algorithms only — never copy-pasted structs, memory offsets, or
addresses) from public WYD server source references.

## Status

The server is playable end-to-end with the patched 7.48 client. Account and
character creation, login, world entry, progression, combat, loot and
persistence are connected through the native protocol; more than 80 packet
types are currently handled.

Systems implemented and server-authoritative:

- **Accounts** — PBKDF2 password hashing, HTTP signup API, local CLI creator and
  duplicate-session rejection. No plaintext passwords are stored.
- **Characters** — four-class, four-slot creation with validated names and
  data-driven starting layouts, items, stats and spawn position.
- **Extended stats** — a server/client compatibility extension carries 32-bit
  HP, MP, STR, INT, DEX, CON, attack, magic attack and defense while preserving
  the fixed 7.48 packet prefixes. Gameplay uses only the authoritative extended
  score; the legacy score is a wire projection.
- **World simulation** — native height/attribute collision, spatially sleeping
  mob AI, patrol routes, target pursuit and a 65×65 area-of-interest window.
  Pets, summons and mobs are materialized only at spawn or visibility entry.
- **Combat** — melee and ranged PvE, PvP, physical and magical floating damage,
  resistances, buffs/debuffs, regeneration, death, resurrection and
  collision-safe recall, all resolved server-side.
- **Progression** — native Mortal EXP table, configurable EXP floor/rate,
  level-up, stat/mastery/skill-point budgets, equipment requirements and
  server-side item bonuses.
- **Skills** — server paths for the 104-skill catalog across TK, Foema,
  BeastMaster and Huntress, including multi-target magic, healing, buffs,
  transformations, out-of-party BM summons and the five Sephira skills.
- **Party** — stable groups of up to 13 players, party chat and full EXP for
  every nearby member plus a configurable bonus per member.
- **Guilds** — creation, invitation, acceptance, leaving, expulsion, leadership
  succession, guild chat, persistent membership and native `Guilds.txt` name
  publication. Guild war and visible guild marks remain future work.
- **Kingdoms** — Akelonia/Hekalotia selection through the kings, Sapphire
  costs, leaving through the broker, realm teleports, guild affiliation and
  Basic/Knight/Elite/Hero/Master cape progression. Realm wars are a later phase.
- **NPCs & economy** — NPCGener, regional visibility, shops, skill masters,
  server-side prices, drops, 63 usable inventory slots, 120 Cargo slots and
  player trade with anti-duplication persistence.
- **Crafting** — server-side recipes and native success/failure feedback for
  Agatha, Aylin, Tiny, Lindy, Compositor, Ehre and Alquimista Odin.
- **Quests** — data-driven requirements and atomic rewards, repeatable quest
  areas, ten-minute area reset and native quest reward boxes. The complete
  retail quest catalog is still being expanded.
- **Volatiles** — configurable potions, gold, teleports, refining/tinting,
  mount items, timed affects, Magical Pill, Hunting Scrolls, summon contracts
  and Sephira books. Deferred codes remain non-consuming generic handlers.
- **Mounts** — eggs incubate and hatch, hatchlings grow into adults, the pet
  follows its owner outside the party, mounted combat absorbs damage, and
  hunger/longevity are simulated.
- **Player shops** — Armia-only Ghost Shops sell Cargo items through a
  stationary clone while the owner remains free to move and play.
- **Communication** — local, party, guild and global chat, whisper, offline
  mail/death letters and server announcements through the 7.48 protocol.
- **Persistence** — atomic JSON writes (temp file + fsync + rename) with an
  async write queue so disk I/O never blocks the game loop; an autosave runs
  every few seconds and critical operations (trades, refining, gold changes)
  persist *before* confirming to the client, with rollback on failure.

## Requirements

- Go 1.26+
- Windows to run the bundled client in `client748/`

## Building

```powershell
go build -o tm.exe ./cmd/server
go build -o account-api.exe ./cmd/account-api
go build -o account-create.exe ./cmd/account-create
```

## Running

```powershell
go run ./cmd/server
```

The server reads its configuration from `data/server.txt`, with command-line
flags available as overrides (`-addr`, `-npcs`, `-accounts`, `-items`, and so
on — run `./tm.exe -h` for the full list). Start it from the `wyd-go/`
directory so the `data/...` paths resolve correctly.

Accounts are created out-of-band, either through `account-create` (local CLI)
or by running `account-api` and calling its HTTP signup endpoint.

## Static checks

```powershell
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
```

## Architecture

```text
cmd/server        composition and configuration
internal/model    pure domain types
internal/wire     byte-exact 7.48 protocol framing and encryption
internal/net      sockets, sessions, per-connection send queues
internal/data     catalogs, NPC spawns, terrain, templates
internal/store    account/character persistence (atomic JSON)
internal/account  signup, authentication, and password hashing
internal/game     the World actor and every game system
```

A single `World` goroutine owns all mutable state. Sessions decode the socket
and forward commands to the game loop, which processes movement, combat,
items, party actions and ticks in a strict order. This gives natural ordering
guarantees for inventory, loot, trade, and combat, and closes off a large
class of duplication/race bugs by construction — nothing mutates game state
outside that one goroutine.

`internal/game` is intentionally fragmented by responsibility (combat,
equipment, skills, affects/buffs, party, movement, drops, trade, teleports,
NPC shops, mounts, quests, guilds...) rather than one monolithic handler file.

## Data files

Game content lives in `data/` as CSV/JSON/text, loaded at boot and never
hand-edited by the running server:

| File | Purpose |
|---|---|
| `itemlist.csv` | item catalog: stats, requirements, static effects |
| `SkillData.csv` | skill cost, delay, target type, range, parameters |
| `NPCGener.txt` + `npcs/*.json` | NPC/mob spawns, stats, shop inventory |
| `character_templates.json` | starting stats/items per class |
| `volatiles.json` | server behavior for consumable item-effect codes |
| `mounts.json` | per-mount-type combat bonuses |
| `quests.json` | quest definitions and prerequisites |
| `quest_zones.json` | timed quest-area boundaries and reset behavior |
| `guilds.json` + `Guilds.txt` | guild registry and 7.48 client name list |
| `droprate.json` | loot table weights |
| `server.txt` | server-wide configuration and gameplay rules |

## Roadmap

The next major gameplay phases are the remaining retail quest catalog, visible
guild marks, guild/kingdom/Castle wars and further client asset modernization.
These are not presented as complete in the current build.

## Disclaimer

WYD-Go is an independent, **non-commercial fan project** made for **educational
purposes** — learning network-protocol reverse-engineering, game-server design,
and Go. It is not affiliated with, authorized, or endorsed by the owners of
*With Your Destiny*.

*With Your Destiny* and all related names, logos, artwork, trademarks, and game
data are the property of their respective owners.

For interoperability with version 7.48, this repository includes server data
files and a client bundle under `client748/`. The original game data, client,
artwork, audio, and map assets are **not** original work and remain the property
of the game's owners; they are included only to make the project usable for
study.

If you are a rights holder and want any file in this repository removed, please
open an issue and it will be taken down promptly.

## License

Licensed under the **GNU General Public License v3.0** — see [LICENSE](LICENSE).

You may use, study, modify, and redistribute this code under the terms of the
GPLv3; any distributed derivative must remain under the same license and keep
its source available.
