# Cross-cutting technical contracts

Working rules and skill routing live only in the root `AGENTS.md`. This
reference records product invariants; it does not claim that every flow has
passed integrated validation. Package and data paths below are relative to
`wydgo748/`. For the client, see `client-ui-748.md` and `ghidra-client748.md`;
for builds and artifacts, see `DOCS/build-and-integration.md`.

## Architecture

### World

`World` is the sole owner of mutable gameplay state. No other package may
maintain a second source of truth for game state.

### Score

`Score` v2 is authoritative. The active wire format uses the 140-byte
`STRUCT_SCORE`, equivalent to `model.Score`. The coordinated contract is in
`DOCS/SCORE.md`; no legacy projection is active. Never derive authoritative
state from values received from the client.

### Server-side validation

Validate, where applicable: packet, size, opcode, phase, session state, target,
identity, distance, terrain, gameplay space, cooldown, item, UID, slot, price,
requirements, balance, inventory, capacity, membership, sequence, and deadline.
Do not trust packet fields merely because the client usually sends them
correctly.

## Persistence and duplication prevention

PostgreSQL is authoritative persistence. The server accepts only
`database_driver=postgres`; the JSON store remains a test adapter in
`internal/store`.

There is no automatic fallback. If PostgreSQL is configured but unavailable,
the server must fail during startup rather than start with another state.

Every operation that can duplicate state follows this order:

```text
validate -> snapshot/copy -> mutate -> persist -> publish
```

If persistence fails, restore the **entire** snapshot; never perform a partial
rollback. Check this order especially for crafting, purchases/sales, trades,
tickets, instances, rewards, ground drops, ascension, counters, guilds,
inventory, and equipment. Never publish confirmation to the client before
persistence when that could confirm an uncommitted operation.

## Identities

```text
ClientID       = ephemeral live entity / wire identity
CharacterUID   = stable character identity
Item UID       = stable server-side identity of a materialized item
RuntimeID      = server-side gameplay-space identity
```

`ClientID` can be reused after disconnect. Do not use it as a persistent or
durable identity for ownership, persisted affects, reconnect, rewards,
resumable membership, long-lived delayed actions, or database state. Check the
risk of ID reuse for every reference that outlives the tick or session that
created it. Item UIDs never go on the wire.

## Gameplay space

`RuntimeID` is an authoritative gameplay boundary:

```text
RuntimeID == ""  -> public world
RuntimeID != ""  -> private/shared runtime
```

Dynamic entities share a space only when their RuntimeIDs are exactly equal.
Apply this rule, where relevant, to Player/Player, Player/Mob, Mob/Player,
Mob/Mob, summons, bosses, skills, AoE, affects, AI, aggro, party EXP, drops,
ground items, rewards, collision, movement, spawn, teleport, and reconnect.
Never infer gameplay space from coordinates: entities in different runtimes
may occupy exactly the same coordinates.

Global NPCs, merchants, and permanent objects need explicit exceptions when
necessary. Do not create an implicit exception merely because `RuntimeID == ""`.
`Merchant != 0` is never hostile or attackable.

## Channel ownership

Entities, spatial indexes, movement, combat, drops, shops, bosses, and
instances belong to one channel-owned live world. Channel-local IDs may repeat
in another channel. Route every entity and session lookup with channel ownership
or a stable session identity; never use a numeric live ID alone.

Account admission and handoff use one shared coordinator with one owner and
generation across all channels. Characters, inventory, currency, progression,
and item identities use shared authoritative persistence; only the active owner
may write them.

Guild identity, membership, leadership, citizenship, kingdom, fame, and
alliances use one global guild authority. City, challenge, bid, war, tower,
tax, and event records remain channel-scoped, while a guild remains global.
Persistence coordination is shared, but each write carries its owning channel,
identity, and generation before completion publishes to that world.

Tests must use equal channel-local entity and instance IDs in two channels.
They must prove local movement, visibility, administration, and instance
snapshots remain isolated while account admission, guild state, and persistence
coordination retain their documented shared scope.
Current regression coverage includes `channel_isolation_test.go`,
`channel_instance_regression_test.go`, `channel_cluster_test.go`, and
`channel_admission_regression_test.go`. Extend these cases when a new
channel-owned identifier is added.

## Visibility and updates

Do not use `CreateMob` to update an already visible player. Use the appropriate
incremental packets for score, affects, equipment, movement, HP, and state.
`CreateMob` is for initial materialization.
Do not replay materialization/spawn visuals during ordinary score, equipment
or affect refreshes. Preserve the native cast animation and effect lifecycle;
an update must not restart unrelated effects or interrupt a buff. Resolve
incomplete appearance state through the appropriate delta, not entity recreation.

When visibility fails in a runtime, trace authoritative entity existence and
RuntimeID, spatial-index membership, per-observer known state, and create/remove
ordering. Requiring the player to move is not recovery proof; a remaining-monster
counter alone does not show that its entities are alive, reachable and visible.

## Spatial index

Use the spatial index for local queries. Do not scan all mobs or players on
frequent ticks for a spatial query. Explicitly check the complexity of each
new tick mechanic.

## Inventory

```text
Inventory: 64 structural slots; 63 visible slots
Cargo:     128 structural slots; 120 visible slots
```

Do not confuse a structural index with a visible client cell.

## Authoritative data

Files under `data/` are the authoritative source of content:

- `itemlist.csv` defines items, static effects, and static item values.
- `Itemname.csv` overrides names only; it must not change semantics.
- `ItemEffect.h` defines the `ID <-> EF_*` mapping for persisted effects.
- `SkillData.csv` defines every skill parameter.

Do not duplicate file-defined values in hardcoded maps, Go tables, correction
switches, post-load overrides, or duplicate constants. Load authoritative
values from their files.

## Package responsibilities

- `model`: pure domain logic; no protocol, store, or session.
- `wire`: protocol, cryptography, serialization, and builders only; no gameplay
  rules.
- `net`: connections, sessions, and transport only; no game rules.
- `store`: persistence.
- `data`: loaders and validation of authoritative files.
- `game`: coordination and gameplay rules, split by feature.

Handlers decode, validate the basic envelope, and route. Do not put extensive
feature logic in `world.go` or `handlers.go`; create a feature-specific file
when the rules grow.

## Language

Engineering identifiers and prose use English, including logs, comments,
diagnostics, documentation, tests, and scripts. Player-facing text sent to the
client uses the selected EN/PT locale on both sides, with English fallback.
Keep protocol bytes, resource IDs, persisted values, and established identifiers
unchanged. Translate legacy engineering prose only in changed sections.

## WYD 7.48 protocol

The protocol is byte-exact. A packet is not correct merely because its fields
appear correct. When creating or changing a packet, confirm its opcode, size,
offsets, types, signedness, padding, header ID, client behavior, and, where
possible, compare it with a native packet. One missing or extra byte can make
the client or server reject the packet. Do not directly port 7.54/7.59 layouts
to 7.48.
