# WYD-Go emulator technical reference

Go paths are relative to `wydgo748/`. This reference summarizes invariants;
verify specific values and layouts against current code and tests. Working
rules live only in the root `AGENTS.md`.

## Architecture and ownership

- `model`: pure state; `wire`: bytes; `net`: transport; `store`: persistence;
  `data`: loaders; `game`: coordination.
- One `World` goroutine mutates gameplay state. Sessions only deliver commands
  and send packets through their own queues.
- Divide features by responsibility. Handlers decode and route; domain
  services validate and mutate.
- `Score` is authoritative; the current coordinated contract sends 35
  `uint32` fields (140 bytes).

## 7.48 protocol boundaries

- The header is 12 bytes. Finalize/encrypt a fresh copy for each send; never
  reuse a mutated buffer for broadcast.
- Reject frames outside 12..8192 bytes and invalid checksums at the socket.
  In the game loop, require `Header.Size == len(pkt)`, the exact size of
  confirmed layouts, and an opcode compatible with the session phase.
- The client sends intentions only: a packet does not authorize damage,
  critical hits, coordinates, slots, prices, learned skills, or login state.
- Rate-limit floods per connection and count refusals with rate-limited logs;
  disconnect repeat offenders without letting logs become a denial of service.
- Confirm in client source any action for which the UI determines the flow
  before sending a packet.
- Client 7.48 `0x213 CharacterLogin` is 36 bytes: header (12), `Slot@12`,
  `Force@16`, and `SecretCode[16]@20`. Do not reduce it to a 16-byte prefix
  merely because the emulator currently consumes only the slot.
- `0x277 ApplyBonus` is 20 bytes: `BonusType@12`, `Detail@14`, and
  `TargetID@16`. The target is essential for the type-2 skill-shop flow;
  18 bytes counts fields without the actual packet size/alignment.
- `0x27B` (16 B) requests a shop; `0x28B` (20 B) is a click/quest request
  with `ClickOk@16`.
- NPC interaction requires an existing, visible NPC within Chebyshev distance
  8.
- Player movement arrives in `0x366`; `Route[24]` is a complete planned route
  and may repeat while the client interpolates. Do not apply the mob-generated
  segment limit of `2*Speed` to players. Constrain speed hacks through a
  server-side time budget, route, terrain, and height.
- Visibility is bidirectional: create/remove, movement, HP, death, respawn,
  and affects update observers.
- `CreateMob` materializes an entity upon entering view and carries position.
  A visual change to an already visible player uses 60-byte `0x36B UpdateEquip`
  (`ItemEff[16]@12` + `AnctCode[16]@44`) for the owner and observers. The final
  16 bytes are essential for dye color. Recreating during movement causes a
  snap/teleport.

## NPCs, shops, and crafting

- `Merchant` occupies one `uint32` field in the sent score. Interpret NPC
  functions according to the current client consumer.
- Skill masters use ShopType 3; ordinary shops use ShopType 1. The client uses
  24 useful skills in a 27-slot grid.
- Some crafting windows open entirely on the client and do not send `0x28B`.
  On a composition opcode, resolve the expected crafter as existing, visible,
  and nearby; do not rely only on `Player.CraftNPC`.
- Replace stale context from another crafter with the expected one in the
  local grid.
- Client-side selector: Aylin head 55 and Agatha head 56 open in any region;
  Tiny head 68 is the global fallback; Ehre head 68 requires chunk `(19,15)`
  for the correct mode; Lindy head 67 requires `(13,13)`; Compositor head 54
  requires `(19,13)`; Odin head 67 + Merchant 8 requires `(25,13)`.
- Send the text message before `0x3A7`: `0` invalid, `1` success, `2` failure.
- Player-facing text must be in English and consistent with the contract on
  both sides.
- Transactional flow: validate snapshot/recipe, copy state, mutate, persist,
  synchronize items/score, then send message/result. Restore all fields on
  error.

## AI, movement, and world

- Player movement validates origin/destination, route bytes, terrain, height,
  and a time budget derived from server-side RunSpeed. `ActionStop`/`MoveStop`
  must not correct a position by more than a few tiles.
- Attacks through blocked terrain may be rejected using a traversable line as
  emulator hardening; document that this rule is not universal in native
  sources.
- Mobs outside activation range sleep. Active mobs acquire targets within
  radius 4, retain them through radius 7, and respect leash 16.
- Perception may tick at 1 s; the attack deadline separately controls DPS.
- Every attack path revalidates `Def.IsMonster()`. `activeMobs` evicts NPCs,
  merchants, dead entities, and invalid entities.
- Native mob movement sends routes up to `2*Speed`; mob speed is capped at 4.
  Coordinate-based Action packets cause teleports and restart steps.
- HeightMap is `int8`: walking requires delta `<8`; spawn repositioning
  accepts `<50`.
- Use the spatial grid for waking/sleeping, proximity, visibility, and
  interaction. Do not scan all generators on ticks.

## Character, skills, and combat

- Recalculate runtime score after equipping, removing, dropping, refining,
  buffing, or changing a stat.
- Damage, HP/MP, and wide stats remain `uint32`; legacy projections must not
  re-enter the calculation.
- Validate that a skill is learned and check mastery, mana, cooldown, target,
  range, and type before mutating mana.
- Affects use a central lifecycle: apply/replace, recalculate, publish, tick,
  expire, and persist when necessary.
- Floating damage is separate from applied damage. Preserve the calculated
  hit even if it exceeds remaining HP.
- Summons stay outside parties and have explicit ownership.

## Inventory and persistence

- Inventory: 64 ABI slots, 63 usable. Cargo: 128 ABI slots, 120 usable.
- Never accept client price, item, effect, or slot without comparison against
  authoritative state.
- Purchases/sales revalidate NPC, merchant, visibility, and distance for every
  packet. Ordinary sales do not accept Equip/Face. Publish a drop only after
  persisting its removal from inventory.
- A 3 s autosave reduces rollback; economy, crafting, and trade require
  synchronous anti-duplication saves.
- Trades between two accounts require atomic multi-account persistence;
  never silently degrade to two sequential saves.
- Atomic save uses temporary file, sync, and rename. Multi-file transactions
  use a typed journal.
- Current position does not determine re-entry; characters spawn at the
  configured entry point.

## Operational validation

Use the validation matrix in [AGENTS.md](../../../../AGENTS.md). To close an
integrated server batch, run these commands from the repository root:

```powershell
Push-Location wydgo748
go test ./...
go vet ./...
go build -o bin/tm-check.exe ./cmd/server
Pop-Location
git diff --check
```

The Git-ignored `wydgo748/bin/` directory holds server builds.

When an action appears to do nothing, record its opcode, size, and essential
fields, reproduce once, and compare the current TMProject and server handlers.
Native boundaries require the 7.48 evidence specified in `AGENTS.md`. Logs
must explain refusals without dumping structs or flooding every tick.
