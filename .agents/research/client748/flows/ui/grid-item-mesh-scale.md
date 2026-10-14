---
id: grid-item-mesh-scale
title: Native item mesh scaling in inventory and equipment grids
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Native item mesh scaling in inventory and equipment grids

## Question

Why do fairy, costume and mount equipment icons appear larger than the same
items in inventory? Does native 7.48 exempt equipment from mesh fitting?

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

## State and lifecycle

Each insertion recomputes presentation scale using the destination cell height.
Inventory, equipment, skill-item insertion and cursor pickup use the same fit.
The grid/cursor retains existing item ownership and teardown. No new cached
scale survives scene destruction or relogin. Invalid dimensions return 1.0
as a defensive adaptation, not a newly claimed native branch.

## Wire, ABI, and resources

No packet, item footprint, mesh, texture, resource ID or equipment position
changes. Mesh fitting is bounded above by 1.0. It does not resize sprite cells
or change equipment centering.

## Current mapping

SGrid::AddItem / AddSkillItem / pickup -> ApplyGridItemPresentation ->
grid_item_presentation::MeshScale(maxZ, cellHeight).

Classification: PARIDADE_NATIVA for fitting all mesh insertions;
MODERNIZACAO_COMPATIVEL for finite/positive dimension guards.

## Delta matrix

| Claim | Before | After |
| --- | --- | --- |
| Equipment mesh | Forced scale 1.0 | Same bounded fit as inventory |
| Large fairy/costume/mount | Can exceed inventory presentation | Fit to destination cell height |
| Small mesh | No enlargement | Unchanged |
| Cursor and skill-item insertion | Separate call options | Shared fitting helper |
| Assets and footprints | Existing data | Unchanged |

## Decisions

Remove the unsupported equipment exemption, not item-specific dimensions.
Preserve grid centering and asset identity; do not introduce family exceptions.

## Gaps

Real-client comparison of inventory, equipment and cursor presentation at
supported resolutions remains open. No CLIENT_TESTED claim.
Published in patch 10024; see
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.

## Validation

ArchitectureTests passed 1,209,006 checks, including rows 1..3, large/small mesh,
invalid dimensions, and source contracts for both insertion paths.
Build-Client.ps1 -NoDeploy completed Release/Win32; installed runtime untouched.
