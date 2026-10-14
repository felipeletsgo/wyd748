---
id: aylin-refinement-presentation
title: Aylin result previews and gem refinement glow
subsystem: ui-layout
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Aylin result previews and gem refinement glow

## Question

How does native 7.48 build the four Aylin results, and which glow belongs to
each refined gem variant? This is distinct from the panel-positioning flow.

## Evidence boundary

- USED: native executable identity above and existing Ghidra exports under
  `tmproject/build/native-research/`: `FUN_004110f5.c`, and
  `decompiled/FUN_0054e75a.c`, `FUN_004bbcb5.c`, `FUN_004c3eec.c`,
  `FUN_00527032.c`. No historical executable is modified.
- USED: 7.48 UI bindings in `TMFieldSceneUI.cpp`: root 6110, results 6129-6132.
- USED: current `TMFieldSceneMix.cpp`, `TMMesh.cpp`, `CMesh.cpp`,
  `BasedefAppearance.cpp`, and server `internal/model/model.go::AncientCode`.
- CONTRADICTORY: the later-client texture 440 for Garnet; native uses 260.
- NOT APPLICABLE: external guides; these local native branches answer the gap.

## Native 7.48 flow

`FUN_004110f5` handles staging. Aylin stores two base items in slots 0/1,
catalyst in 2, gems in 3-6, and optional equipment in 7. The four previews
are generated from the first gem (2441-2450). After normalizing an EF_SANC
pair to the front of the first item unless a refinement/tint already leads,
the four results combine both items' additional-effect pairs. All four keep
the first item's index. Results 0/1 use the first item's other pairs;
results 2/3 use the second item's other pairs. Results 1/3 exchange pair 1.

For input refinement 9/10/11, the setter input is gem minus 2431/2427/2423.
`FUN_0054e75a` clamps negative levels to zero, rejects levels >=26, stores
levels above 9 as level+220, and prefers any EF_SANC pair over any tint pair
(115-126), then the first empty pair. No available pair means no mutation.

`FUN_004bbcb5` (UI) and `FUN_004c3eec` (world mesh) select these texture bases:

| Variant | Additional | Ancient legend | Refined legend | Texture base |
| --- | --- | ---: | ---: | ---: |
| Diamond | Drop | 5 | 9 | 153 |
| Emerald | Piercing | 6 | 10 | 166 |
| Coral | EXP | 7 | 11 | 246 |
| Garnet | Absorption | 8 | 12 | 260 |

The animation frame is added to the base. `FUN_00527032` confirms the
weapon-legend translation; the hand-copy behavior is not evidence of a bug.

### Callers

The field-scene grid input dispatch reaches `FUN_004110f5`. Mesh rendering
reaches `FUN_004bbcb5` for UI and `FUN_004c3eec` for world equipment.

### Callees

The staging branch calls `FUN_0054e75a`, item allocation/construction and grid
insertion. Render branches select the effect texture and texture stages.

## State and lifecycle

### Observable entry

Stage eligible Aylin ingredients; the result grids are presentation-only.

### Transition matrix

| Event | Result |
| --- | --- |
| Missing base item or first gem | Empty results |
| Stage/change first gem | Four independent preview copies |
| Remove ingredient | Recompute or clear results |
| Close, snapshot reset, logout | Delete temporary copies; retain no preview state |

### Vtables, vptrs, and receivers

Existing FieldScene/grid receivers and bindings are reused. No vtable or
receiver ABI is changed. Scene teardown is covered by the linked layout flow.

### Ownership

Each result grid owns its cloned SGridControlItem and STRUCT_ITEM. Removal
detaches any cursor reference before deletion. Inventory remains authoritative.

### Partial failure

Missing panel/packet/grid is guarded. Failed grid insertion deletes the clone.
Incomplete ingredients cannot create a result or authorize a server mutation.

### Cleanup and teardown

`ClearNativeMix(2)` resets staging and refreshes empty results; general scene
cleanup follows [the existing layout flow](native-mix-inventory-layout.md).

### Shutdown

No new callbacks, threads, or persistent state; scene/grid owners release copies.

### Logout and relogin

No preview survives scene destruction. A new scene binds empty result grids.

## Wire, ABI, and resources

STRUCT_ITEM stays eight bytes: signed 16-bit index followed by three two-byte
effect/value pairs. IDs, recipe packets and server validation remain unchanged.
Raw 230-233 encodes +10 Diamond/Emerald/Coral/Garnet. The server AncientCode
high nibble and BASE_GetItemTenColor agree with these variants.

## Current mapping

`PARIDADE_NATIVA`: the inherited preview implementation and UI/world texture
corrections reproduce the exported native branches. Pure helpers make the
combinations, setter priorities and texture choices directly testable.

`MODERNIZACAO_COMPATIVEL`: apply the same gem mapping to the existing imported
weapon mesh branch 2889-2901, which previously handled only legends 5-8 and
fell back to Diamond for refined variants. These mesh IDs are not claimed as
native parity. Preserve the separate forced-texture policy for 2906-2909.

## Delta matrix

| Claim | Previous gap | Decision |
| --- | --- | --- |
| Aylin results | Bound but not populated | Preserve inherited builder; test four copies |
| UI Diamond +10 | Legend 9 omitted | Include 9 |
| UI/world Garnet | Texture 440 | Native texture 260 |
| Imported weapons | Refined variants fall back to 153 | Reuse gem selector |
| Hand presentation | Suspected legend copying | Preserve native behavior |

## Decisions

- The Aylin presentation changes no server formula, item value, recipe or protocol.
- Preserve staged items; modify copies only.
- Do not infer visual success from a build or publish without authorization.

## Gaps

Actual built-client testing remains pending: all four Aylin previews and
tooltips, removal/closing/reopening, and UI/world glow for all four gem variants.
Imported mesh 2906-2909 retains its pre-existing fixed glow intentionally.

## Validation

- STATICALLY VERIFIED: native branches and current server encoding compared.
- AUTOMATED TESTED: the real eight-byte STRUCT_ITEM is exercised for all four
  combinations, input immutability, normalization, tint/setter priority, missing
  inputs, all accepted gem IDs and setter boundaries. Source contracts connect
  the helper to the scene and imported weapon path. ArchitectureTests passed
  1,208,967 checks; socket 231, crash capture 83, launcher 56 also passed.
- Incremental `Build-Client.ps1 -NoDeploy` completed Release/Win32. Candidate:
  `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `18B87EE8B32F6333BD0838C46E0BED85F0FD2B172A7C2AF61804EED054ECC418`.
- Research validator and repository layout/link validation passed. Temporary
  crash-test evidence cleanup was blocked by the execution policy. The directory
  `tmproject/build/tests/Release/crash-capture-tests-10680-361407312` remains;
  no native research or runtime files were removed and no bypass was attempted.
- Published in patch 10023 and retained in 10024. Not CLIENT-TESTED; no client
  launch or local runtime installation. See
  [current state](../../../../../DOCS/current-state.md) for publication receipts.

## Independent equipped-earring Lac exception

Classification: `MODERNIZACAO_COMPATIVEL`, a server policy correction using the
existing UseItem contract. Ordinary Lac item 413 (volatile effect 5, RefineMax 9)
may refine an equipped slot-8 earring with exact Pos 256 from +9 to +10.
Normal and critical persistence paths share equipmentPowderChance. Ordinary
weapons, armor, necklaces, inventory/cargo targets and Ori do not gain this
exception; the existing +10 to +11 rule remains unchanged.

The user-requested W2PP _MSG_UseItem.cpp branch rolls rand()%100 <=15:
16 successful outcomes out of 100. It also destroys failed earrings and allows
later levels. Those destructive/higher-level rules were not imported. Preserve
the project's existing non-destructive failure: consume Lac, retain the earring
and its effects. Persistence rejection restores both; publication follows commit.
This is not a claim of complete native 7.48 refinement parity.

AUTOMATED TESTED: earrings 591..595, success rolls 0/15 and failure rolls 16/99,
UID/additional-effect retention, save rejection, ineligible targets, ordinary
powder consumption, critical commit/rejection/unknown outcome/stale UID/replay.
Full Go suite and vet passed. The earring exception is published in server
release `20261014T234237Z`; in-game refinement remains untested.
