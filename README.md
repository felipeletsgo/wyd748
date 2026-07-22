# WYD-Go

A server-authoritative emulator for **With Your Destiny (WYD)**, client version
**7.48**, written from scratch in Go.

The client renders state and sends intent; the server is the sole source of
truth for inventory, stats, combat, skills, progression and persistence. No
price, target, position, cooldown, drop, or effect is ever trusted from the
client — everything is revalidated against server-side state before it takes
effect.

This is a clean-room implementation: the wire protocol was reverse-engineered
from packet captures against a real 7.48 client, and gameplay formulas were
ported (never copy-pasted structs, memory offsets, or addresses) from public
WYD server source references.

## Status

Login, character selection and enter-world are fully wired to the 7.48 wire
protocol (77 opcodes implemented), and the server is playable end-to-end.
**336 automated tests** cover the game logic, wire encoding, and persistence
layers.

Systems implemented, server-authoritative, with tests:

- **Accounts** — PBKDF2 password hashing, an HTTP account-signup API, and a
  CLI creation tool. No plaintext passwords anywhere in the data model.
- **World simulation** — real-time movement with server-side collision against
  the native height/attribute maps, and area-of-interest visibility (entities
  outside a player's window are never sent to that client).
- **Combat** — player↔mob and player↔mob PvE, player↔player PvP, death,
  recall, and respawn, all resolved server-side.
- **Progression** — stats and equipment, point allocation, experience and
  leveling, and the full skill roster (104 skills) across all four classes
  with class-specific execution paths.
- **Social systems** — party (up to 13 members) with shared experience, guilds
  (structural), and player-to-player trade with an anti-duplication commit
  protocol.
- **NPCs & items** — shops, loot tables, quests with server-validated
  prerequisites, and a data-driven consumable/item-effect system covering 116
  distinct item-effect codes across ~2,900 catalog items.
- **Mounts** — a full lifecycle: eggs incubate and hatch, hatchlings grow into
  adults by level, mounted combat absorbs a share of incoming damage, and
  starving a mount degrades it over time.
- **Persistence** — atomic JSON writes (temp file + fsync + rename) with an
  async write queue so disk I/O never blocks the game loop; an autosave runs
  every few seconds and critical operations (trades, refining, gold changes)
  persist *before* confirming to the client, with rollback on failure.

## Requirements

- Go 1.26+
- A WYD 7.48 client (not included in this repository — this is server-only)

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

## Testing

```powershell
go test ./...
go vet ./...
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
| `droprate.json` | loot table weights |
| `server.txt` | server-wide configuration and gameplay rules |

## License

No license has been chosen yet. All rights reserved by the author unless a
license file is added.
