---
id: field-hud-death-prompt-regressions
title: Resource labels, death prompts, and native dye texture stages
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-27
---

# Resource labels, death prompts, and native dye texture stages

## Question

Why do compact HP/MP maximum labels regain a slash, and why does dismissing
the death prompt allow it to reopen without another player action?

## Evidence boundary

- Current implementation: `TMHuman.cpp`, `TMFieldScene.cpp/.h`,
  `DeathMotionPolicy.h`, and `ResourceBarProjectionTests.cpp` under
  `tmproject/TMProject748/`.
- Reused native contract: [restart recall request](../transport/restart-recall-request.md)
  and its existing `exports/restart-recall-flow.tsv` evidence.
- Continuation: verified the native SHA-256 above and inspected the analyzed
  native binary with Ghidra 12.1.2 headless, read-only. Focused exports under
  ignored `tmproject/build/native-research/` include `FUN_004c3eec.c`
  (skinned rendering), `FUN_004c3a1b.c` (materials), `FUN_0052a737.c`
  (inventory updates), and `FUN_00541065.c` (field objects).
- The user's screenshot and requested behavior identify the regression, but
  do not establish the complete native dialog or material implementation.
- Existing unrelated worktree changes were retained. This record describes
  only the changes made for the September 26 report and its continuation.

## Native 7.48 flow

The existing recall contract identifies `FUN_00476006` as the five-second
recall sender and `FUN_004776C3` as the prolonged-death maintenance sender.
Both send the header-only `0x289` request; the server controls revival.
This evidence does not prove the exact native prompt suppression policy.
The aggregate record remains `LOCATED` because the portal and runtime
consumption reports remain unresolved.

Native `FUN_004c3eec` selects dye effect textures for legends 116..125 when
refinement is positive. Its common hardware path uses numeric COLOROP 24
(DOTPRODUCT3) at stage 0 and 6 (MODULATE4X) at stage 1, except black (120),
which uses 4 (MODULATE). The effect uses UV channel 1. Earlier revisions of
this record misidentified 24 as MULTIPLYADD (actually 25) and 6 as ADD
(actually 7). That interpretation was wrong and produced a whitening bug.
`DyeTextureStages.h` now follows the actual numeric operands. The native
Voodoo/Intel/G400/TNT fallback uses 4/7 (MODULATE/ADD), not ADDSIGNED.
TNT affects the RGB branch but not the alpha fallback condition. This is
not a claim of parity for ordinary refinement glow or the complete material.

## State and lifecycle

The first eligible death-animation completion or timeout offers the prompt
and records that it has been offered. Dismissal leaves that flag set, so a
later frame or animation completion cannot reopen it. A new world left- or
right-click may offer it again. Active town/revival channel timers suppress
both automatic and player-action offers. Positive authoritative HP resets
the flag for the next death.

Confirm explicitly hides the dialog, starts the existing recall channel,
and invalidates the last displayed countdown value so its first portal
effect is not skipped. Existing effect creation and server requests remain
responsible for the rest of the channel. No client-side revival is added.

## Wire, ABI, and resources

No packet, serialized resource, item rule, or server state format changes.
The existing delay-start and header-only recall requests are unchanged.
Only a local scene flag and helper are added.

## Current mapping

`TMHuman::UpdateScore` had two hardcoded maximum-value formats that bypassed
the shared `resource_ui::MaximumTextFormat` policy. Both now use that policy,
matching the already-correct score update path.

`TMFieldScene::OfferRespawnPrompt` centralizes eligibility and one-shot state
for compact/full input, the timed fallback, and local death-animation
completion. UI controls still receive input before world-click handling.

## Delta matrix

| Delta | Classification | Scope |
| --- | --- | --- |
| Reuse maximum label formatting | `MODERNIZACAO_COMPATIVEL` | Correct an inconsistent internal update path |
| Suppress repeated automatic prompts | `MODERNIZACAO_COMPATIVEL` | Local UI lifecycle; unchanged server authority and wire |
| Hide on confirm and initialize effect countdown | `MODERNIZACAO_COMPATIVEL` | Existing recall implementation |
| Common-path dye composition and UV channel | `PARIDADE_NATIVA` | `FUN_004c3eec`, legends 116..125 with positive refinement; implemented, not client-tested |
| Real-catalog Repliction regression coverage | `MODERNIZACAO_COMPATIVEL` | No product behavior or contract change |

## Decisions

- Implement the isolated UI regressions without changing recall authority.
- Do not infer native material settings or insert guessed portal objects.
- Do not consume items unconditionally on the client: rejection and server
  persistence failures must preserve authoritative inventory.

## Gaps

- Client execution remains required: HP/MP updates after combat/equipment,
  cancel then idle, cancel then movement/cast click, confirm with a visible
  five-second effect and authoritative revival, and a second death.
- Noatum/Nippleheim: server endpoints exist at `(1053,1709)` and
  `(3650,3109)`. The missing visual object/effect is unresolved; no portal
  patch was made. Field0813/Field2824 have no visual object within four units
  of these endpoints. Nearby mesh bounds do not cover the endpoints. The
  removed legacy CreateGate path only used InitItem entries; the inspected
  entries contain nearby victory towers, not an identified portal resource.
  Do not insert an unrelated mesh as a guessed replacement.
- Materials/dyes: the common dyed-mesh stage path is corrected, but visual
  comparison with native WYD.exe remains pending. Ordinary refinement glow,
  zero-refinement dyes, and the additional native fallback flag are not
  covered by this patch. No assets were changed.
- Grade A-E Repliction: real configuration entries `4016..4025` already
  specify `consume: true`. The user confirms the equipment bonus changes.
  New real-catalog tests cover all ten IDs, implicit/single/stacked amounts,
  inventory slot 37, successful save, decoded outbound 0x182 removal/update,
  and replay after the last unit. These pass; the remaining client symptom
  is not reproduced and no consumable product fix is claimed. Actual
  persistence/relogin and client icon behavior still require execution.

## Prior batch validation

- `STATICALLY VERIFIED`: maximum format call sites, automatic prompt entry
  points, confirm lifecycle, unchanged recall packets, and English authored
  text in affected source/test files were inspected.
- `AUTOMATED TESTED`: `Build-Client.ps1` passed 58,479 architecture checks,
  221 socket checks, and costume asset validation (135 items, 129 renderers,
  774 parts). Added policy cases cover automatic offer, dismissal, renewed
  input, channel suppression, and next-death eligibility. Release build and
  candidate deployment succeeded. Dye checks cover ten legends, two adapter
  paths, and two alpha modes (200 assertions). The build used the shared
  working tree, including other pending changes; it is not an isolated build
  of the task-only commit.
- Focused server command passed from `wydgo748/`:
  `go test -count=1 ./internal/game -run 'TestOnUseItemTintUntintReplictionAndFallback|Test.*Repliction|Test.*ConsumeOne'`.
- Candidate: `tmproject/client748/project.exe`, SHA-256
  `0DFC25714BF05E35B3216034486C7AFBE1E7260BF8C2ED21FC7C3ED38597BC5F`.
- `CLIENT-TESTED`: not performed. Neither the built candidate nor native
  WYD.exe was driven through these scenarios in this session.

## September 27 minimap and shader follow-up

The user reproduced malformed minimap geometry, missing coordinates, invisible
Noatum/Nippleheim portals, and incorrect dyed armor after the prior batch.
The prior build was therefore not evidence of visual correctness.

### Minimap

`FUN_0044ca65` distinguishes the classic resource from the UI2 resource:
compact dimensions are 160 and 137 respectively, and expansion uses a
400-pixel map. UI2 owns separate frame controls 5705..5713 and zoom controls
5714/5715. The loaded FieldScene2 graph contains those controls even when the
CLASSIC option selects UI version 1. The compatibility path resized only the
root and classic border, leaving the UI2 children at their loaded positions.
It also returned before the ordinary coordinate-label update.

The compatibility scene now binds the actual UI2 graph, lays out its frame
pieces on each transition, centers expansion in the current viewport, and
updates the coordinate label before control rendering. Tests cover the
hidden/compact/expanded/hidden cycle at 800x600, 1024x768, 1280x960 and
1920x1080 for both resource types. The placeholder server label is hidden;
coordinates use a centered row below the map. These placement choices are
`MODERNIZACAO_COMPATIVEL`, not an exact native-layout parity claim.

### Weighted skin shader contract

`CMesh::SetMaterial` and native `FUN_004c3a1b` provide material power in c0.y,
animation time in c0.z, fog in c6, ambient/emissive in c7 and diffuse in c8.
The mesh path uploads inverse-view constants at c92. The loaded
`Shader/skinmesh1.bin` through `skinmesh4.bin` instead omitted the inverse-view
normal transform, used c95 for fog, time for specular output, and material
power for the second texture-coordinate animation. This defect survives the
prior texture-stage fix and affects ordinary lighting as well as dyed armor.

`tools/client-assets/Sync-SkinShaders.ps1` adapts the existing root
`shader1.bin` through `shader4.bin` programs to D3D9 input declarations and
destination masks. It preserves their arithmetic, weights, constant
registers, and both texture outputs. It parses the complete input corpus
before writing, rejects unsupported/truncated instructions, and supports
read-only `-Check`. Build-Client now checks those four runtime assets before
building. Unlit shaders 5..8 and historical evidence are unchanged.
Classification: `PARIDADE_NATIVA` for the recorded weighted material contract;
implemented, not client-tested. This is not a claim that all dye cases or
hardware fallbacks are visually correct.

### Portal investigation limit

Native `FUN_00541065` explicitly marks object types 657/658 as null objects,
just like the active loader; removing that flag would not be a supported
fix. The earlier endpoint/object findings remain unresolved. Neither the
height mask nor the server teleport intention identifies the missing visual
resource. No portal asset, height, or game-state change was made. The next
useful evidence is the native client at the same endpoint with coordinates,
followed by its concrete object/effect creation path, not another broad
field-file inventory.

## Validation

- `STATICALLY VERIFIED`: native minimap control branches, material constant
  uploads, and changed English text. `git diff --check` passed.
- `AUTOMATED TESTED`: 58,535 architecture checks (56 new layout checks),
  221 socket checks, costume dependency validation, and incremental Release
  build passed. Existing signedness/deprecated-socket warnings remain.
- `Sync-SkinShaders.ps1 -Check` rejected the old incompatible asset before
  regeneration and passed afterward. SDK `fxc.exe /dumpbin` comparisons for
  all four variants matched native arithmetic after normalizing only D3D9
  declarations and destination masks. This does not exercise a D3D device.
- Initial test compilation rejected integer-to-float conversions under the
  warning-as-error policy; explicit float test inputs fixed that failure.
- The initial deployment failed because project.exe was running. After the
  user closed it, the already-built candidate was copied and its SHA-256
  verified without rebuilding: `448435AAA87F4F7B41B2EF958CBF8901BF567507FF20F14D518CEF2F97238347`.
- `CLIENT-TESTED`: not performed. Required visual gates remain compact and
  expanded minimap geometry/coordinates, dyed chest lighting and animated
  overlay, and comparison with native WYD.exe. The portal remains unfixed.
- Changed: TMFieldScene.cpp, MiniMapLayout.h, ResourceBarProjectionTests.cpp,
  Build-Client.ps1, Sync-SkinShaders.ps1, four runtime skin shaders, this
  record, and the local candidate. Removed files: none. Existing unrelated
  untracked files were preserved; the generated executable is not a commit
  deliverable.

## Startup shader regression and black +1 follow-up

### Startup failure

The September 27 startup dumps reported access violation `0xC0000005` in
`d3d9.dll` at `0x643D90D6`, reading `0x24`. Matching PDB stack resolution
reached `CMesh::RenderMesh` at its `DrawIndexedPrimitive`, then `CFrame`,
`TMSkinMesh`, `TMTree`, and the startup scene render loop.

The D3D9 conversion had two invalid assumptions: variant 1 declared a blend
weight absent from VertexDecl1, and variants 2..4 blended an uninitialized
`w` after matrix instructions that write only `xyz`. Removing the absent
declaration alone still crashed (dump `client-crash-20260927-140712.dmp`).
Restricting those bone-blend destinations to `xyz` resolved assembly
validation and startup; `w` is initialized by the existing later instruction.
Classification: `MODERNIZACAO_COMPATIVEL`, D3D9 shader validity only.

`Test-SkinShaders.ps1` now disassembles and reassembles all four programs with
the validating D3D compiler, checks their input declarations against the
vertex layouts, and rejects in-memory negative fixtures for both defects.
`Build-Client.ps1` runs this gate. Disassembly alone did not catch invalid
source-register reads and was insufficient validation in the preceding batch.

The shader-only candidate remained responsive after launch from client748;
the user's subsequent character-selection screenshot confirms startup, but
also demonstrates that startup repair did not fix the black armor color.
Computer-use capture was stopped with Escape; no subsequent visual automation
was used for the dye follow-up.

### Exact black-dye case and decision

The user identified a black Hard Leather Breastplate (Normal) +1, with a
black inventory icon and a gray equipped model in selection and in the field.
The runtime table selects effect 327, `Effect/dk0000.wys`, for legend 120 +1.
Decoded DXT1 base-level values are approximately 82..173/255, averaging
95/255; the effect resource exists and is not a solid black replacement.

The previous stage-0 MULTIPLYADD produced `light + base * light`, clamped
before stage 1 multiplied by the black texture. Thus even a black base texel
acquired a gray floor, and fully lit selection could saturate base detail.
The D3D9 correction uses MODULATE at both stages for black only:
`base * light * blackEffect`. It also avoids the legacy ADDSIGNED clipping
path for black. Other nine dye colors, resource IDs, UV animation, alpha
handling, and authoritative item data are unchanged.

Historical classification: `MODERNIZACAO_COMPATIVEL`. This black-only
workaround was not native parity and is superseded by the enum correction
below. Its premise that the native common path used MULTIPLYADD was false.

Additional focused native inspection reused the existing Ghidra project,
confirmed its recorded SHA, and completed without script errors. Writes to
device offset `0x2a5c8` in `FUN_00427119` identify the NVIDIA RIVA/TNT/TNT2
fallback flag (active `m_bTNT`), closing the earlier identification gap.
`FUN_004c3a1b` also attenuates refined diffuse/ambient channels by 3/3.4/3.8
before further refinement branches; the active material code omits that
step. That separate lighting difference was not changed in this focused
black-dye patch, and is not claimed as its cause or as resolved.

Source authority: native binary/Ghidra and runtime effect assets USED for
the boundary and resource mapping; active source USED for composition;
server NOT APPLICABLE (no item/wire/state changes); other historical clients
NOT APPLICABLE. No new asset, packet, ownership, or teardown path is added.

### Validation and delivery

- `STATICALLY VERIFIED`: black-only override and unchanged other dye paths;
  startup declaration/mask corrections; all newly authored text is English.
- `AUTOMATED TESTED`: 58,571 architecture checks and 221 socket checks passed
  through `pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1`.
  Dye regression cases cover black +1 texture values, field/selection lighting,
  both adapter paths, zero-valued base texels, dark detail, and all ten legends.
  Shader validation, negative fixtures, and costume dependencies passed.
- Release build and installation of `tmproject/client748/project.exe` passed;
  SHA-256 `D6994A002DFD845E0359F820DD1EC30A583B33E227E3B3A9A0677424631D96FF`.
  The user's GPU-preference export edits in TMProject.cpp/.h were preserved.
  Their existing runtime `WYD.exe` was not replaced; the candidate is project.exe.
- `CLIENT-TESTED`: earlier startup recovery only. The final black-dye candidate
  still requires visual confirmation on the selected character and in the field.
  Pixel-operation tests are not a substitute for that gate. Portal and minimap
  visual gaps from the earlier batch remain outside this follow-up.
- Changed in this follow-up: DyeTextureStages.h, EffectVertexColorTests.cpp,
  Sync-SkinShaders.ps1, Test-SkinShaders.ps1, Build-Client.ps1, four skin shader
  assets, this record, and the ignored/generated build outputs and local candidate.
  No product files removed. Existing unrelated changes and installers retained.

## Blue +1 whitening: corrected numeric operation mapping

The next user screenshot showed a blue Hard Leather Breastplate (Normal) +1
rendering cyan/white while its inventory icon retained dark blue. The running
process was `client748/project.exe`, with the previous candidate hash
`D6994A002DFD845E0359F820DD1EC30A583B33E227E3B3A9A0677424631D96FF`;
this was not a stale renamed WYD.exe. Effect 275 (`Effect/bl0000.wys`)
decodes to RGB ranges 16..74, 81..150, 183..247. Pixel (0,0) is (24,82,186).

Re-reading the already exported `FUN_004c3eec` against the SDK enum exposed
the prior interpretation error: `(stage=0, state=1, value=0x18)` means
DOTPRODUCT3, and `(1,1,6)` means MODULATE4X. Neither means addition.
The [Microsoft D3DTEXTUREOP definition](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dtextureop)
and installed Windows SDK `shared/d3d9types.h` agree on these values.
No new Ghidra export or corpus scan was necessary; the evidence itself was
valid, but its earlier translation into enum names and tests was not.

The helper now uses the native signed RGB dot product for base density,
then multiplies it by the dye texture (fourfold gain for colors, ordinary
multiplication for black). The black-only workaround is removed. The
Voodoo/Intel/G400/TNT branch uses native MODULATE/ADD; TNT is now passed
separately by `CMesh` so it does not incorrectly change the alpha condition.
Texture IDs, animated UVs, shader programs, resource bytes, inventory state,
and the server contract are unchanged. The previously noted refined-material
attenuation gap is separate and remains open.

Classification: `PARIDADE_NATIVA` for these texture-stage operands and the
adapter branch only. Native binary/Ghidra USED (existing export and identity);
runtime assets USED (blue texel fixture); active source USED (helper/caller);
Windows SDK USED (enum decoding); server NOT APPLICABLE (no state change);
other historical clients NOT APPLICABLE. Ownership/teardown is unchanged.

### Validation of the corrected mapping

- `STATICALLY VERIFIED`: numeric operands matched to SDK definitions;
  English authored text; existing user changes preserved. Prior enum-based
  tests repeated the implementation error; they were not independent proof.
- `AUTOMATED TESTED`: `Build-Client.ps1 -NoDeploy` passed 58,659 architecture
  checks, 221 socket checks, shader validation, and the incremental Release
  build. Pure tests now compare literal native values, all ten legends,
  three adapter branches, both alpha modes, and blue/black pixel equations.
- The separate `DyeTextureStagesDeviceTests.vcxproj` Release/Win32 target
  passed 241 actual Direct3D9 HAL pixel-readback checks on Intel Iris Xe.
  It invokes the production helper on a hidden 4x4 render target and checks
  an independent oracle, including field/selection lighting and adapter
  branches. Reintroducing the old MULTIPLYADD/ADD pair is explicitly rejected.
  It does not manipulate the user's game or require a logged-in character.
- Once the game process exited, the built artifact was copied without
  rebuilding to `tmproject/client748/project.exe`; source and destination
  SHA-256 matched `2192CE6E3A522452CB308CE4AA9B87EDDFD6A57E272924940A3A889CD1631B9C`.
  The separate runtime `WYD.exe` and historical evidence were not replaced.
- `CLIENT-TESTED`: not yet. The device test verifies composition, not the
  complete skinned-character draw, material setup, or native visual parity.
  The installed candidate still needs the user's blue +1 field/selection
  check. No visual completion is inferred from compilation or readback.
- Changed in this batch: `DyeTextureStages.h`, `CMesh.cpp`,
  `EffectVertexColorTests.cpp`, the new `DyeTextureStagesDeviceTests.cpp`
  and `.vcxproj`, this record, and the local generated executable. No files
  removed. No asset conversion or server change was made in this batch.

## Pink +1 follow-up: preserve dark cloth detail

The user's next field screenshot showed that the white washout was gone, but
the pink breastplate was almost entirely black. This invalidates visual
completion of the previous candidate, despite its correct native stage enums.
The equipped Hard Leather Breastplate (Normal), item 1116, resolves to model
texture 420, `mesh/ch010203.wys`; pink uses `Effect/pi0000.wys`. The signed
DOTPRODUCT3 formula clips dark base texels to zero before tint application.
At lighting 140/255, 9,003 texels in the actual 128x128 base texture lose
positive detail this way. Changing only the dye color cannot recover it.

Classification: `MODERNIZACAO_COMPATIVEL`, not native visual parity. Reuse the
existing native exports for the resource/stage boundary; no new native claim
or corpus scan is needed. The equipped skinned-mesh draw now scopes a ps.1.1
shader around dyed armor only. It computes unsigned mean base luminance times
mean vertex lighting, with fourfold gain for colored dye and ordinary gain for
black. It clamps that density before multiplying by the existing dye texture.
This preserves dark detail without adding white light or a brightness floor.
Base texture alpha, dye IDs, animated UVs, assets, item data, and wire contracts
are unchanged. The inventory-icon rendering path is not changed in this batch.

The render device owns the lazily created shader and releases it on device
invalidation and finalization. A scoped binding restores the previous pixel
shader and constants even when the draw fails; outlines and subsequent meshes
do not inherit the override. Unsupported shader creation retains the previous
fixed-function stages. The separately recorded material attenuation gap remains
open. Server changes are NOT APPLICABLE; historical assets are read-only input.

### Validation and installed candidate

- `STATICALLY VERIFIED`: scoped shader binding, reset/finalization ownership,
  unchanged texture identities and protocol; authored text is English.
- `AUTOMATED TESTED`: `DyeTextureStagesDeviceTests.vcxproj` Release/Win32 built
  with MSBuild and its executable passed 98,545 D3D9 HAL pixel-readback checks
  on Intel Iris Xe. The tests load the actual WYS armor texture and pink,
  blue, and black dye fixtures; compare every texel against an independent
  formula at two light levels; retain the 241 fixed-function checks; verify
  restoration of constants/shader and shader recreation. These tests use FVF
  vertices, not the complete skinned-character pipeline.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1
  -NoDeploy` passed 58,659 architecture checks, 221 socket checks, shader gates,
  and the integrated incremental Release build. Existing compiler warnings
  remain; the build succeeded.
- After the user closed the game, the built artifact was installed as
  `tmproject/client748/project.exe`. Source and destination SHA-256 matched
  `262D58EFB4AF97E9247330DFD8B4A4184B830599D47A7B7AEA2775B8336F1E01`.
  Neither the separate runtime WYD.exe nor historical binaries were replaced.
- `CLIENT-TESTED`: pending for this candidate. The pink equipped breastplate
  must still be checked in the field and character selection; GPU fixtures and
  a successful build do not establish full-character visual correctness.
- Changed in this follow-up: new `DyePixelShader.h`, `CMesh.cpp`,
  `RenderDevice.h/.cpp`, `DyeTextureStagesDeviceTests.cpp/.vcxproj`, this record,
  and the generated local executable. No files removed by this follow-up.
