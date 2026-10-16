---
id: aylin-refinement-presentation
title: Aylin result previews and gem refinement glow
subsystem: ui-layout
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-15
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

## Ordinary inventory grade lookup follow-up (2026-10-15)

Classification: `PARIDADE_NATIVA`, reusing the same native hash and cached
`FUN_004bbcb5` export. The ordinary normal-hardware branch selects texture
233 plus the grade lookup at 0x005baa04, already retained in `NativeMountGlow.h`:
`10,1,2,3,4,6,7,7,8,9,10,11,12`. Legacy hardware selects 233 plus grade directly.
The current ordinary UI branch had used the latter formula on both paths.
`InventoryTextureIndex` now shares the native lookup, with all grades 0..12
tested in both hardware modes. Gem/dye branches remain separate.

The general dull-icon report is not closed by this correction. Runtime item
3140, named Glihtning_Stone in the catalog, has mesh 2721, texture offset 0,
visual effect 0 and legend 0. It therefore takes the native unrefined
Render(0, offset) branch before grade selection. Both mesh lists map 2721 to
`mesh/junsin.msa`; the active file and the available OnGame/KR copies share
SHA-256 `EED01EFF2FB558C67236689164933D1296E979C3DA7BFDBC8DF607D6B6E25BB3`.
No evidence supports replacing that mesh or inventing a glow layer. The later
constructor trace below proves that a separate native halo was omitted; the
RenderForUI-only trace did not cover it. Visual confirmation still requires
client execution.

AUTOMATED TESTED: the batch's ArchitectureTests passed 1,209,233 checks and
the incremental Release/x86 solution build passed. This follow-up was later
published in patch 10027; the earlier 10023/10024 receipts do not cover it.
The owner subsequently reported that the dull-icon defect persists in 10027.

## Native inventory lighting follow-up (2026-10-15)

Classification: `PARIDADE_NATIVA`, using the same native executable SHA-256
`8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
The cached `FUN_0042e8a1` decompile omits the state-block call argument.
Native instructions at 0x0042eccb are `6A 03` (push 3), followed by the
virtual call at 0x0042ecd5 (`FF 52 40`). Ordinary 3D inventory icons therefore
use native block 3, not block 1. Cached `FUN_0042dea5` supplies the missing
state contract: main diffuse/specular RGB 1.0, fill diffuse/specular RGB 0.6,
white ambient, alpha reference 0xAA at 32 bits or 0xFF otherwise, alpha blend
and alpha test enabled, clipping disabled, and depth writes enabled. Light
alpha and unrelated light fields remain unchanged.

The port had selected world block 1, inheriting map lighting and a different
alpha cutoff. Its block 3 already serves a separate 2D setup, so the native
inventory contract is mapped to internal block 4 through
`NativeInventoryLighting.h`; existing 2D callers remain unchanged. Only the
ordinary 3D icon branch selects this block. World lighting, special meshes
737..739, item identity, textures, and refinement passes are preserved.
This corrects a concrete shared lighting/transparency divergence affecting
the reported dull icons, including unrefined item 3140; no extra glow pass or
asset replacement is introduced.

AUTOMATED TESTED: ArchitectureTests passed 1,209,240 checks, including fixed
RGB values, preserved light fields, both alpha cutoffs and production wiring.
The incremental Release/x86 solution build passed. Patch 10027 subsequently
published this executable and the costume meshes; no local runtime executable
installation or agent-initiated game execution was performed. The owner's
10027 screenshot is negative visual evidence: Glihtning Stone and other icons
remain dull. The fixed-lighting divergence is corrected in source, but it is
not sufficient to close the reported defect. The later constructor trace below
identifies an additional omitted halo.

## Native inventory halo follow-up after 10027 (2026-10-15)

Classification: `PARIDADE_NATIVA`, reusing the same native hash and cached
Ghidra exports of `FUN_0040d13e`, `FUN_0042e8a1` and `FUN_0042eeea`.
The native binary and exports are USED; the runtime effect assets and current
source are USED. TMProject 7.69 is not parity authority; server state and wire
contracts are NOT APPLICABLE because this is local icon rendering.

Entry: creating a mesh-backed grid item. `FUN_0040d13e` writes item object
offset 0x1c0 (GeomControl offset 0x15c, dwBGColor). Item 3140 receives
0xFFFFFFFF, 3407..3416 receive the same value, and selected other items receive
the native colored palette. The eight-entry palette at native VA 0x005b129c
is retained verbatim in `NativeInventoryGlow.h`. Type 206 with first effect
59 selects 0xFFAA8888, subject to the later item-ID overrides.

The active constructor previously left dwBGColor zero for every item. This
prevented the existing RenderGeomControl branch from selecting effect texture
273 for the white halo (56/122/123 for the other native colors). The helper now
sets that field for mesh-backed items only. Original models, item identities,
refinement passes and texture files are unchanged. The halo is owned by the
same GeomControl as the icon; no new allocation, callback, persistence or
logout lifecycle is introduced. Grid destruction retains existing ownership.

Direct native instruction verification resolves three errors in the existing
halo renderer: 0x0042efa1 calls the sine wrapper (0x0058fe22 is FSIN), then
0x0042efa9/0x0042efaf multiply/add the floats at 0x005a41f4/0x005a41f0,
respectively 0.35 and 0.65. The period is 2000 ms. Native alpha is preserved,
not multiplied by the pulse. The top-right quad UV is (1,0), not (0.1,0).
The production renderer now uses the tested sine/color helper and full UV.
Existing native texture selection, additive blending and halo dimensions stay
unchanged. No extra world effect or guessed brightness adjustment is added.

AUTOMATED TESTED: ArchitectureTests passed 1,221,246 checks, including all
catalog halo boundaries, palette and override priority, first-effect gating,
pulse extrema, alpha retention and 2000-ms periodicity, plus production wiring.
Socket 231, crash capture 83, launcher 56, shader and costume gates passed;
incremental Release/x86 build passed. Candidate SHA-256:
`75868D55FA061A5256F8A213785D2B6A81EF563BCDD26D161FE4D5BFB99CA2FD`.
CLIENT-TESTED remains pending. The owner's 10027 screenshot is retained as
negative evidence for the previous fix, not acceptance of this follow-up.
Published as executable-only patch 10028; fresh HTTP/HTTPS ZIP and executable
hashes matched. See the launcher receipt for artifact identities. No local
runtime installation, service restart or agent-initiated game execution.

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
