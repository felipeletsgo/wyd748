---
id: grid-item-mesh-scale
title: Native item mesh scaling in inventory and equipment grids
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-17
---

# Native item mesh scaling in inventory and equipment grids

## Question

Why do fairy, costume and mount equipment icons appear larger than the same
items in inventory? Does native 7.48 exempt equipment from mesh fitting?
Why are Odin's staged items anchored at the top-left of tall receptacles?

## Evidence boundary

- USED: native executable hash above and Ghidra exports for FUN_0040d13e,
  FUN_0040cea0, FUN_0040e6aa, FUN_0040e817, FUN_004b14fd,
  FUN_0040fc3e and FUN_0040dd00.
- USED: existing exports/grid-item-mesh-scale-xrefs.tsv,
  grid-item-mesh-scale-vtable-window.tsv and grid-item-mesh-scale-vtable-callers.tsv.
- USED: current SGrid.cpp, SGridSupport.h and GridItemPresentation.h.
- CONTRADICTORY: the previous record described forced equipment scale 1.0 as
  native behavior. That was an adaptation exemption, not native evidence.
- NOT APPLICABLE: later-client mesh dimensions do not override this branch.

## Native 7.48 flow

### Callers

Grid insertion and cursor presentation reach FUN_0040e6aa/FUN_0040e817 through
virtual slots +0x8c/+0x90 of vtable 0x005a4024 (entries 0x005a40b0/0x005a40b4).
The recorded indirect-call exports close the insertion boundary.

### Callees

FUN_0040d13e constructs the item presentation; FUN_0040cea0 resolves the mesh.
FUN_004b14fd loads it with arguments mesh, 0, 180000. MaxZ at +0x354 is compared
with cell height times 0.3 (constant at 0x005a40ec). A smaller item is never
enlarged. Neither insertion branch exempts equipment grids or item families.

FUN_0040fc3e determines the grid origin and FUN_0040dd00 centers presentation;
centering is independent of mesh scale. Legacy sprites remain separate:
skill books 5000..5102 use atlas 19923; other sprites retain their existing path.

The cached Ghidra export `FUN_0040fc3e.c` also resolves Odin's alignment:
native grid types 6, 8, 9, 11, 12, 14, 16, 18, 20 and 22 bypass the ordinary
cell loop. For an occupied receptacle, the child virtual call at +0x58 receives
the parent/grid origin plus half of `(grid extent - item extent)` on both axes.
`FUN_0040dd00` adds half the item extent before queuing the mesh, yielding the
receptacle center. Sprites keep the centered box origin. Native mix type 20
is not the quickslot role suggested by the inherited 7.69 enum name.

## State and lifecycle

Each insertion recomputes presentation scale using the destination cell height.
Inventory, equipment, skill-item insertion and cursor pickup use the same fit.
The grid/cursor retains existing item ownership and teardown. No new cached
scale survives scene destruction or relogin. Invalid dimensions return 1.0
as a defensive adaptation, not a newly claimed native branch.

For the mix follow-up, existing scene initialization binds Odin's seven grids
under root 6481 to raw type 20. Staging retains the authoritative Carry item
and owns a separate visual clone; rendering does not transfer ownership or
write the composition packet. Removal and panel closure still detach and
delete the clone and restore the Carry visual through `ClearNativeMix`.
Insertion rejection leaves ownership with its caller. Scene teardown/relogin
retains the existing control destruction/recreation path; there is no new
allocation, cached position, input handler or shutdown state. The previously
traced panel lifecycle is in `native-mix-inventory-layout.md`; this follow-up
closes only its rendering gap, not a new lifecycle or recipe-parity claim.

## Wire, ABI, and resources

No packet, item footprint, mesh, texture, resource ID or equipment position
changes. Mesh fitting is bounded above by 1.0. It does not resize sprite cells
or change equipment centering.

## Current mapping

SGrid::AddItem / AddSkillItem / pickup -> ApplyGridItemPresentation ->
grid_item_presentation::MeshScale(maxZ, cellHeight).

Classification: PARIDADE_NATIVA for fitting all mesh insertions;
MODERNIZACAO_COMPATIVEL for finite/positive dimension guards.

`WYD748_IsCenteredSingleCellGrid` now uses
`grid_item_presentation::IsNativeMixReceptacle` for the six even native mix
types 12..22. `FrameMove2` selects the existing `FrameMoveAtCenter748` path;
the latter queues both the item and its label. Classification: PARIDADE_NATIVA
for this receptacle selection, independently of the level-40 recipe policy.

## Delta matrix

| Claim | Before | After |
| --- | --- | --- |
| Equipment mesh | Forced scale 1.0 | Same bounded fit as inventory |
| Large fairy/costume/mount | Can exceed inventory presentation | Fit to destination cell height |
| Small mesh | No enlargement | Unchanged |
| Cursor and skill-item insertion | Separate call options | Shared fitting helper |
| Assets and footprints | Existing data | Unchanged |
| Native mix types 16/18/20/22 | Missing from centered selection | Same centered path as types 12/14 |
| Odin item origin | Ordinary top-left cell transform | Receptacle center; existing mesh fit retained |

## Decisions

Remove the unsupported equipment exemption, not item-specific dimensions.
Preserve grid centering and asset identity; do not introduce family exceptions.
Interpret active 7.48 numeric mix roles without renumbering the inherited enum.
Do not modify item offsets, resource geometry or hitboxes to conceal the missing
centered branch. Ordered recipe acceptance and Fame policy are documented
separately in `DOCS/client/port-748.md` and are not inferred from this renderer.

## Gaps

Real-client comparison of inventory, equipment and cursor presentation at
supported resolutions remains open. No CLIENT_TESTED claim.
Published in patch 10024; see
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.
That historical receipt covers mesh fitting, not the 2026-10-17 mix follow-up.
The mix follow-up is local only and has not been installed or published.

## Validation

ArchitectureTests passed 1,209,006 checks, including rows 1..3, large/small mesh,
invalid dimensions, and source contracts for both insertion paths.
Build-Client.ps1 -NoDeploy completed Release/Win32; installed runtime untouched.

2026-10-17 follow-up: ArchitectureTests passed 1,240,536 checks, including all
mix types and adjacent non-mix values, the production centered-render dispatch,
and the ordered Odin/cape recipe guards. SocketReceiveTests (231),
CrashCaptureTests (83) and LauncherTests (56) passed. Release/Win32 build with
`Build-Client.ps1 -NoDeploy` passed; candidate SHA-256:
`8D811FF60B70D8ADAE4AB37E89BC6A6268235035D9574142D1F9A43359ED10F1`.
Candidate: `tmproject/build/TMProject748/Release/WYD.exe`. Assets, installed
runtime, launcher feed and VPS were not changed. Real client: not run; Odin
placement, removal, closure/reopen and relogin still require visual acceptance.

## Mysterious Stone icon correction — 2026-10-30

Classification: `MODERNIZACAO_COMPATIVEL`. The owner selected the Lactolerium
100% icon for Mysterious Stone. Server catalog item 4148 and Ideal Stone 5338
both declare `2875.0`; Lactolerium 100% item 3022 declares `2874.8`.
`MeshList.txt` maps mesh 2874 to `secretst.msa` and 2875 to `idealst.msa`.
`BASE_GetMeshIndex` retains these mesh indices for these unequipped items.

`GridItemPresentation.h` overrides both mesh and texture for item 4148 only:
2874 and 8. The shared `SGridControlItem` constructor applies the pair before
grid fitting. Item identity, footprint, effects, tooltip, Ehre recipe and
server state retain their existing paths. No resource, loader, wire or locale
content changes. This is a selected presentation correction, not a native
parity claim.

`STATICALLY VERIFIED`: catalog rows, mesh list and existing mesh files.
`AUTOMATED TESTED`: paired icon regression, Ideal Stone preservation and
fallback coverage; ArchitectureTests passed 3,183,424 checks. Socket (281),
crash capture (147) and launcher (58) checks passed. Incremental Release build
through `Build-Client.ps1 -NoDeploy` passed. Log:
`build/mysterious-stone-icon/client-build.log`. Candidate SHA-256:
`063631E1ADF531F5E56CCFD14BCC429CEA26599C682089168DEB8C5CCB296AAA`.

`CLIENT-TESTED`: pending for Ehre output, inventory and cursor presentation.
The local correction is not installed or published. Allocate the next mandatory
client revision with its matching server minimum during release preparation.
