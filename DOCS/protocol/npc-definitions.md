# Live NPC definition management

Active NPC templates are stored as JSONB in PostgreSQL and keyed by channel and
template name. The admin panel edits the full definition with optimistic
versioning and records the actor, reason, timestamp, and resulting JSON.

## Active catalog

At startup, the server builds each channel's required template set from enabled
`NPCGener` entries and loaded boss, quest, instance, volatile, and Colosseum
configuration. It seeds only those definitions into PostgreSQL. Templates with
no discovered runtime reference remain in the source directory and do not
appear in the editor until a runtime configuration references them.

`internal/npccontent` declares typed references for generators, quests, quest
zones, bosses, summons, Colosseum waves, and every loaded instance population.
Display names, dialogue, IDs, and summon appearance names do not activate templates.
A missing admitted template fails initialization before synchronization, with its content
path and exact name. There is no reflective string search.

Generator references retain the legacy space/underscore equivalence. Exact names
take precedence; ambiguous aliases fail validation. Quest zones declare their
guard template through `npc`; `name` remains the area identifier. Older zone
profiles retain their runtime entry binding when `npc` is absent.
Reserved public generators use the same admission policy as spawning; their
instance or event owner supplies the effective template references.

`data/npcs/*.json` is the common versioned source for both channels. The former
channel 2 snapshot supplied 727 templates; its identical unused `Reimers` entry
was consolidated into `reimers`. Another 304 required templates preserve quests,
instances, events, and other configured content. The catalog contains 1,030 files.
The consolidated `channel2_population.json` file and overlay loader were removed.

PostgreSQL keeps a source snapshot and the active definition for each
channel. Startup refreshes the source snapshot and rebases immutable service
selectors onto staff overrides. Mutable attributes and stock remain staff-owned.
An effective change creates an audited version. Restoring source creates a new version; it does not erase history.
Each World clones mutable source data before loading channel-specific overrides.
Names match exact runtime identifiers. Source synchronization preserves staff
overrides independently in each channel.

Sharing source files does not synchronize later channel-specific edits. Use the
panel's explicit **Both channels (1 and 2)** scope to keep edited content equal.

Only proven aliases were consolidated. For example, `Reimers` and `reimers` had
identical attributes and stock. Only `reimers` spawned, at `(2468, 1996)` in both
channels. Its spawn and stock remain unchanged. Unsupported supplemental item
slots were cleared without shifting inventory or drop positions.

`NPCGener` continues to own spawn locations, routes, counts, and timing. The
editor changes template attributes only. It does not create spawn records.

Both channels load `data/spawns/NPCGener.txt` once and receive the same source groups.
The former channel 2 population is the baseline. Another 396 groups retain
missing project, quest, test, and legacy content. The owner's 40 co-located
replacements keep the former channel 2 templates and spawns. Trainer quests now
reference `Treinador_I`, `Treinador_II`, and `Treinador_III`; rewards and quest IDs
remain unchanged. Their server Merchant values remain 36/40/41. The existing
`clientMerchant` projection selects quest type 11 for the supported 7.48 client.
Server routing compares the full crafting Merchant value, so 40 cannot match 8.
`NPCGener2.txt` and the `npcgener2` configuration key were removed.
Remove that obsolete key from preserved deployment configuration before activation.

The shared file contains 5,501 source groups. The loader admits 5,485 enabled
groups; existing disabled, out-of-map, and nonpositive-capacity records remain
inactive. The import preserves distinct routes, quantities, and existing overlaps
for the owner's manual review. Dedicated event files retain their lifecycle owners.
DeathWorm group 5134 at `(2137,2052)` was removed for incorrect coordinates by owner request.

## Validation and live application

- The editor accepts only active `tipo: npc` templates.
- The server strictly decodes the JSON and validates NPC fields and item IDs.
- Name, type, service identity, combat role, and merchant selectors stay fixed.
- Service faces stay fixed: Premium Nell (57) and air transport (63, merchant nibble 7).
- Source synchronization repairs these selectors in older overrides and retains unrelated staff edits.
- Shop stock is limited to 27 visible slots. NPC identity and spawn routes stay
  outside this editor.
- PostgreSQL checks the expected version and saves the new definition and audit
  revision in one transaction.
- The World actor replaces the definition in place so existing entities keep
  their IDs, position, generator state, and ownership.
- Boot and live updates prepare mount stock on the World actor before use.
  Zero-state mounts receive HP, food, and longevity; explicit state remains unchanged.
- Visible clients receive an updated NPC visual. Stock changes invalidate open
  shop contexts. Reopen the merchant before buying from its new stock.
- Old shop requests cannot buy replacement stock. Already admitted purchases
  finish with their original item; inventory updates reconcile the committed item.
- Other server processes query only the compact revision stream every ten
  seconds, then apply changed definitions. They do not reload the full catalog.
- Writers serialize revision allocation through commit with a per-channel
  transaction lock. Poll batches alone advance checkpoints; notifications do not.
- Boot reads the checkpoint before loading definitions, so concurrent changes replay safely.
- The panel confirms the loaded channel and blocks edits after changing its input.
- Changing edit scope loads fresh catalogs before accepting target versions.
- `ok` confirms World application. `saved_pending` confirms durable storage only.
  Refresh the catalog to compare `appliedVersion` with `version` before reporting application.

## Atomic edits across channels

Single-channel requests remain compatible. The optional `targets` field declares
an atomic edit of channels 1 and 2, with an expected version for each channel.
The primary `channel` and `expectedVersion` must match their target entry.

Both Worlds must be attached to the same persistence authority and have the
template active. Validation checks each channel's immutable source selectors.
The service rejects differing effective definitions, including differing sources
during restore. Staff authorization is checked again immediately before saving.

One PostgreSQL transaction locks channels in ascending order, checks every
version and captured source hash, then writes both definitions and revisions.
A conflict, cancellation, or failed write rolls back the entire batch. Concurrent
reversed target orders use the same lock order. Direct editing remains unsupported.

After commit, each World applies the revision through its own command queue.
The response retains the primary `npc` and includes per-channel `npcs` views.
`saved_pending` means both saves committed but at least one application remains
unconfirmed. The panel shows each channel's saved and applied versions. Runtime
application is eventually consistent; it is not simultaneous across Worlds.

## Code boundaries and diagnostics

- `internal/npccontent` owns strict decoding, editable-content validation,
  stock blueprint defaults, and explicit template references.
- `internal/store` owns transactions, optimistic versions, source checks, and audits.
- `game/npc_definition_admin.go` coordinates authorization, validation, saving, and acknowledgements.
- `game/npc_definitions.go` owns polling and actor-only replacement of live definitions.
  NPC commands have a dedicated event; they do not pass through Premium Nell events.

Existing tick duration and loop-lag metrics remain authoritative for loop cost.
`npc_definition_apply_delay_micros` records the last observed save-to-application
delay by channel. Zero timestamps are ignored; negative clock differences clamp
to zero. Poll failures and dropped queue notifications have cumulative counters:
`npc_definition_poll_failures_total` and `npc_definition_queue_drops_total`.
Polling recovers dropped notifications without advancing its checkpoint early.

## Staff access

The panel requires the embedded server, PostgreSQL, and the
`content.npc.shop` capability. The tracked maintainer accounts currently have
that capability in `wydgo748/data/config/staff.json` and the VPS staff template.

## Operations

The initial server release applies schema migrations 13 and 14. Migration 14
aligns stored keys and audit records with exact runtime template names. Later edits do not
require a restart. A rejected database transaction leaves the stored version unchanged. If the
client has an older version open, reload the NPC before editing again.
Use the validated editor/API for changes. Direct JSONB updates bypass validation,
audit, and revision locking; they are not a supported editing procedure.

The multi-channel panel/API contract is `EXTENSAO_COORDENADA`. It changes no game
packet, client revision, spawn ownership, or persistence schema.
