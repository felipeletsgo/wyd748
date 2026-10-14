---
id: kefra-portal-visuals
title: Kefra hall static portal model and effects
subsystem: gameplay
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Kefra hall static portal model and effects

## Question

Which legacy map-loader boundary draws the mirror in front of Kefra, and can
its mesh and both effects follow an authoritative defeat snapshot without
changing unrelated portals or retaining invalid streaming pointers?

## Evidence boundary

- USED: Existing Ghidra export `FUN_00541065.c` in the local native-research
  corpus, generated from the executable identified above. Its model `0x7F3`
  branch constructs effects `0x1A7` and `0x1A8` and caches the hall objects.
- USED: Active `TMObjectContainer::Load`, `TMObject::IsVisible`/`Render`,
  `TMEffectBillBoard::IsVisible`/`Render`, and the field-scene lifecycle.
- USED: Owner screenshot with living Kefra, a visible mirror and minimap
  position `(2366,3925)`; not a run of this candidate.
- USED: The owner's later screenshot shows the defeat notice and visible mirror,
  but passage use fails. The live journal records the ADMIN guild defeat at
  `2026-10-14 18:30:36 UTC`; the local client log then repeatedly records
  `teleport ignored x=2364 y=3924 attr=0x54 motion=1 index=-1`.
- NOT PROVEN: A native defeat-state packet or the native hide/show trigger.
  The new world snapshot is an independent coordinated extension.
- The official gameplay guide is cited in the
  [versioned contract](../../../../../DOCS/protocol/kefra-portal.md); it does
  not establish a native ABI or 7.48 schedule/reward values.

## Native 7.48 flow

### Callers

The native loader root is `FUN_00541065 @ 0x00541065`. The call sites and the
native defeat-to-visibility transition are not traced here. This record stays
`LOCATED`; it makes no native parity or `CLIENT_TESTED` claim.

### Callees

The branch for object type `0x7F3` (2035) calls `FUN_004C8F70` to construct
billboards with textures `0x1A7` (423) and `0x1A8` (424), then links them with
`FUN_0054ADBC`. The model is constructed through `FUN_0053E1D2`. Position
comparisons cache scene fields at offsets `0x28E34`, `0x28E38` and `0x28E3C`.
These observations identify one mesh and two effect consumers, not a packet.
The float globals used by the native comparison were not independently decoded
in this investigation; the active loader identifies `(2366.5,3926.5)`.

## State and lifecycle

The map container owns and streams the mesh/effects. Cached field-scene object
pointers can outlive a streamed object, so the extension does not dereference
them. Each visual queries the currently active field scene when tested for
visibility or rendered. A newly streamed instance inherits the stored snapshot.
Scene initialization defaults to hidden; a valid server snapshot changes it.

## Wire, ABI, and resources

Model 2035 and textures 423/424 are preserved, as are their position, scale,
animation, alpha mode and ownership. No historical executable or resource was
patched. The new 16-byte `0x7F30` envelope and its rejects are specified in the
[contract](../../../../../DOCS/protocol/kefra-portal.md), not attributed to
native 7.48. The existing teleport request and server admission are unchanged.

## Current mapping

`TMObjectContainer.cpp` selects `VisualGate<TMObject>` and
`VisualGate<TMEffectBillBoard>` only for the identified hall instance.
`KefraPortalPacket.h::ClientState` is reset on scene initialization.
`TMFieldScene::OnPacketKefraPortalState` accepts the validated snapshot;
`ReceivedPacketDispatch` rejects malformed versions/flags before casting.
The Go server derives the flag from persisted `Wars.Kefra.Defeated` on entry,
death and weekly reset, never from client-visible boss presence.

The post-kill activation failure is separate from visibility. The client
`TMFieldSceneMovement.cpp::UpdateTeleportPrompt` aligns each coordinate with
`0xFFFC`, then requires a matching `TMGlobal.h::g_TeleportTable` row before
offering confirmation. The hall mirror block `(2364,3924)` was missing. Both
active attribute maps already contain `0x54` at that block, and the existing
server route is `(2366,3925) -> (3250,1703)`, priced at 700 gold. The repair adds
one client entry for Uxmal with that fee, without changing either map, the
server's defeat check or the 16-byte `0x290` request. The unchanged request
boundary is covered by the existing
[teleport record](../transport/req-teleport.md); no new native table-parity claim
or Ghidra investigation is needed for this data correction.

## Delta matrix

| Claim | Evidence | Classification |
| --- | --- | --- |
| Model/effect identity | Native loader branch and current loader | Legacy boundary located, not complete native hide/show parity |
| Visibility follows a durable defeat snapshot | New client/server contract and tests | `EXTENSAO_COORDENADA` v1 |
| Other portals, resources and masks remain unchanged | Scoped loader selection and adapter tests | Part of the same coordinated extension |
| Post-kill Uxmal prompt resolves the mirror block | Client `index=-1` log, existing route/maps and table regression | `MODERNIZACAO_COMPATIVEL`; no wire change or new native-parity claim |

## Decisions

- Gate the mesh and both effects; hiding just the mirror leaves a false portal.
- Keep default hidden until the server snapshot; preserve valid state on bad input.
- Broadcast after persistence acknowledgement, including the weekly close.
- Keep server teleport admission authoritative and all unrelated mechanics intact.
- Register the missing Uxmal activation block; do not bypass the lookup or send
  client-selected destinations to work around a missing table entry.

## Gaps

- Native caller chain and native defeat-to-visibility trigger remain untraced;
  they are not required to claim this explicitly new contract.
- Actual candidate rendering, post-kill passage use, streaming, relogin and
  reset remain manual gates. Windows prevents the visual test requested in
  earlier project work; do not replace it with a build claim.
- The owner's published-client screenshot confirms a visible portal after a
  kill, not successful passage or acceptance of the table repair in patch 10020.

## Validation

`TestKefraPortalStateV1` shares both canonical frames with `KefraPortalTests`.
Game tests inspect encrypted packets for entry/restoration, death, reset,
duplicates and persistence failures, with critical commit/rejection/unknown
outcome coverage on both transitions. The C++ test instantiates the actual
adapter for a mesh and two effects without starting the game. The final local
build/test receipt is in
[current state](../../../../../DOCS/current-state.md#published-combat-cape-and-kefra-batch--2026-10-14).

The added table regression fails before the missing row is added and passes
afterward. It checks a unique Uxmal entry and fee, the movement lookup's grid,
all 16 mirror tiles in both real attribute maps, and the real server route.
Focused Go tests preserve rejection before defeat, passage/charging after
defeat and closure at weekly reset. These are automated/static checks, not an
actual game run. Patch 10020 publishes the exact validated executable without
promoting this record to `CLIENT_TESTED`. The publication receipt is in
[current state](../../../../../DOCS/current-state.md#published-kefra-passage-activation-repair--2026-10-14).
