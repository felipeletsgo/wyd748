# WYD-Go

A server-authoritative emulator for **With Your Destiny (WYD)**, client version
**7.48**, written from scratch in Go.

The client renders state and sends intent; the server is the sole source of
truth for inventory, stats, combat, skills, progression and persistence. There
is no trust boundary crossed from the client into game state — every price,
target, position, cooldown and effect is revalidated server-side.

## Status

Login, character selection and enter-world are fully wired to the 7.48 wire
protocol. Core systems implemented and covered by tests:

- accounts (PBKDF2 password hashing, HTTP account API, CLI creation tool)
- movement, area-of-interest visibility, and terrain collision
- player↔mob and player↔player combat, death, and respawn
- stats/equipment, leveling, and the full skill roster across all four classes
- party, trade, guild (structural), and quest systems
- NPC shops, drops, and a data-driven consumable/item-effect system
- a full mount subsystem (eggs → hatchlings → adults, feeding, combat)

## Requirements

- Go 1.26+
- A WYD 7.48 client (not included in this repository)

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

## Architecture

```text
cmd/server      composition and configuration
internal/model  pure domain types
internal/wire   byte-exact 7.48 protocol framing and encryption
internal/net    sockets, sessions, queues
internal/data   catalogs, NPC spawns, terrain, templates
internal/store  account/character persistence (atomic JSON)
internal/account signup, authentication, and hashing
internal/game   the World actor and all game systems
```

A single `World` goroutine owns all mutable state. Sessions decode the socket
and forward commands to the game loop, which processes movement, combat,
items, party actions and ticks in a strict order. This gives natural
ordering guarantees for inventory, loot, trade, and combat, and closes off a
large class of duplication/race bugs by construction.

## License

No license has been chosen yet. All rights reserved by the author unless a
license file is added.
