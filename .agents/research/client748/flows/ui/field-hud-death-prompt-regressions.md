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

The later 1280x960 screenshot exposed the compact header (control 12841)
remaining visible above the expanded frame. The compatibility path now hides
that child while expanded and limits the expanded square to 31.25% of the
viewport width, 42% of its height, and the native 400-pixel maximum. The
frame, direction marker, zoom controls, and coordinate row share that size.
This responsive cap is `MODERNIZACAO_COMPATIVEL`, not native-size parity.
At Armia coordinates 2086,2093, only three of the nine surrounding `m*.wyt`
tiles exist in the runtime asset set; absent tiles were not invented or
stretched. The updated candidate passed the architecture and socket tests and
was built/deployed, but the new geometry has not been visually client-tested.

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

## Matte dyed equipment: remove the animated overlay highlight

The user confirmed improved color in the installed pink candidate but reported
a remaining slight shine; dyed equipment should have a matte finish. The
existing shader still sampled the complete effect texture using `oT1`. Current
skinmesh1 disassembly shows `oT0 = v4` (base UVs), whereas `oT1` combines a
skinned normal component, view-space position, and the animated `c0.z` offset.
CMesh::SetMaterial advances that offset over ten seconds. This leaves the
effect texture's highlight pattern moving across dyed equipment even without
additive white lighting. The previous constant-color GPU fixture did not cover
this behavior.

Classification remains `MODERNIZACAO_COMPATIVEL`. The matte pixel shader now
samples the dye texture at its fixed center, not at the environment coordinates.
The center is an explicit rendering choice, not a claimed native palette rule.
Unsigned base luminance, diffuse vertex lighting, colored/black gain, base UVs,
and texture alpha are unchanged. The shader uses ps.1.4 dependent sampling;
both texture fetches occur after the phase marker to preserve base alpha.
The 1.x pipeline retains existing vertex fog. Shader ownership, fallback and
state restoration are unchanged. Untinted meshes and mounts are not modified.

Sources: active shader bytecode/disassembly, CMesh material setup, and existing
native boundary records USED; actual 7.48 armor and dye textures USED as tests;
server and other historical clients NOT APPLICABLE. This is not new native
parity evidence and does not close the separate material attenuation gap.

### Matte validation

- `STATICALLY VERIFIED`: the shader no longer consumes environment UVs;
  texture identities and shader lifetime remain unchanged; English-only text.
- `AUTOMATED TESTED`: the Release/Win32 `DyeTextureStagesDeviceTests` target
  passed 590,065 GPU pixel checks on Intel Iris Xe. Tests now bind the actual
  blue, black, and pink effect textures instead of flat-color stand-ins, shift
  environment UVs through three phases at two lighting levels, and require
  identical pixels. They also verify dark detail, alpha, state restoration,
  shader recreation, and vertex fog at factors 0, 128, and 255. A first ps.1.4
  assembly attempt exposed alpha loss across the phase marker; moving both
  texture fetches after it fixed the issue and all final checks passed.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1
  -NoDeploy` passed the integrated incremental Release build, 58,659
  architecture checks, 221 socket checks, and the skin shader gates.
  After confirming the game process had exited, installed the candidate as
  `tmproject/client748/project.exe`; build and installed SHA-256 both equal
  `BC6825EEC1178F0A79477C8DBBA9CB3FEE09E0F5B4D6DE3600A77E6F50BF38E4`.
- `CLIENT-TESTED`: pending for the matte candidate. The user's screenshot
  validates improvement of the previous candidate, not this new draw path.
- Changed in this follow-up: `DyePixelShader.h`,
  `DyeTextureStagesDeviceTests.cpp`, this record, and generated build outputs.
  No files removed, no asset bytes changed, and no server changes.

## User-directed refinement sheen in the dye color

The user rejected the fully matte finish and requested restrained sheen in the
dye's own color, increasing with refinement without hiding the armor texture.
This supersedes the matte-only aesthetic decision above, not the established
texture, item-state, or wire contracts. Classification: `MODERNIZACAO_COMPATIVEL`;
this is a custom rendering design, not a native parity claim. The native
boundary records and active skin shader UV evidence are reused. Server and
historical-client modifications are NOT APPLICABLE.

The shader retains the fixed dye-center hue and uses the mean RGB of the same
texture at animated environment UVs solely as a scalar mask in [0,1]. For
refinement r clamped to [0,15], t = r/15 and strength = 0.12*t + 0.12*t*t.
Density is multiplied by (1 + strength*mask), clamped, and then multiplied by
the fixed dye color. Consequently, the relative local lift cannot exceed 24%
at +15; +1 is about 0.85%, +5 about 5.33%, and +9 about 11.52%. These are ceilings,
not uniform brightness increases. Zero strength reproduces the matte baseline.
There is no additive white component or positive floor on black base texels.
Black uses its existing dark graphite palette and ordinary density gain.
Texture alpha, vertex fog, and scoped shader/constant restoration are retained.

`BASE_GetItemSanc` feeds the displayed refinement through `TMHuman` and character
selection into `TMSkinMesh` and `CMesh::m_sMultiType`. The dye branch previously
mutated that value to 12 while selecting an effect texture. It now clamps only
the legacy texture index, preserving +13..+15 across frames. When the shader is
active, `CMesh` selects the first existing texture of the same dye family for
both palette and mask at every grade; the shader curve alone changes strength.
The original grade-specific texture selection remains the unsupported-shader
fallback. Other legend branches, unpainted armor, static weapons, and inventory
icon rendering are not changed. Existing +0 routing is not expanded by this
batch; the zero-strength shader is a regression baseline for these tests.

### Refinement sheen validation

- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests` passed
  5,603,569 D3D9 HAL pixel checks on Intel Iris Xe. Actual blue, black, and pink
  WYS textures and the affected breastplate run through grades 0..15, three
  environment phases, and two diffuse-light levels. Independent per-texel
  oracles check the mask, hue, density, alpha, 24% ceiling, nondecreasing
  channels, strictly increasing total brightness at each grade, animated
  response, and vertex fog. Existing matte, dark-detail, shader-state, reset,
  and fixed-function tests remain passing. Negative/oversized strength inputs
  are clamped. Tests use FVF vertices, not a complete skinned character.
- An initial doubled-and-clamped mask saturated the pink texture to a constant.
  The animated-response assertion caught it; using the unscaled mean mask
  preserves texture variation for all three real dye fixtures.
- `STATICALLY VERIFIED`: dye-only draw binding, non-mutating refinement clamp,
  unchanged resources/protocol, and English-only authored text. The separate
  material attenuation gap is not closed by this customization.
- Integrated incremental Release build passed with `pwsh -NoProfile
  -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1 -NoDeploy`, including
  58,659 architecture checks, 221 socket checks, and four skin shader gates.
  With no running project.exe process, installed the build into
  `tmproject/client748/project.exe`; source and destination SHA-256 match
  `B2421BF86D18F15B22B34802678CC97C1C20E7F914D3C397BFA62AC237C53AA1`.
  Historical and runtime WYD.exe were not replaced. Research validation and
  `git diff --check` passed.
- `CLIENT-TESTED`: pending for the colored refinement candidate. User review
  should compare low/high refinement, black/blue/pink, and field/selection views.
- Changed: `DyePixelShader.h`, `CMesh.cpp`, `DyeTextureStagesDeviceTests.cpp`,
  this record, and generated local build outputs. No files removed.

### Darker contrast follow-up: 48% application strength

The user could not distinguish the same-tone reflection and requested one
shade darker with a 48% intensity limit. This supersedes the preceding 24%
brightness-lift design. Interpret one darker shade as 75% of the dye's RGB,
preserving hue. Strength is now 0.24*t + 0.24*t*t, capped at 0.48 for +15.
The environment mask controls interpolation toward that darker shade. The
result is matteBase * (1 - 0.25*strength*mask). Thus 48% is the application
weight, not a 48% global darkening: the local reduction is bounded by 12%.
The user was informed of the 25% darker-shade interpretation before the edit.

Apply contrast AFTER clamping the diffuse density. The previous shader applied
its brightness lift before that clamp, so saturated areas could erase the
entire animated reflection. The new order retains contrast on bright cloth,
with unchanged alpha, dye hue, diffuse detail, fog, shader ownership, and
refinement routing. Black becomes a darker graphite reflection, not white.
Zero strength still reproduces the matte baseline. The existing compatible
modernization classification and evidence boundary remain valid; no new native
parity claim, asset edit, server change, or protocol change is made.

- `AUTOMATED TESTED`: MSBuild Release/Win32
  `DyeTextureStagesDeviceTests.vcxproj` and its executable passed 8,110,321
  D3D9 pixel checks. The blue/black/pink fixtures cover grades 0..15, three
  phases, two lighting levels, darker-shade formula, 48% weight cap, 12%
  contrast bound, alpha, fog, and retained cloth detail. Added fully white
  base fixtures with partial alpha to force saturated density: the moving
  reflection must survive there too. Real cloth increases aggregate contrast
  at every grade; uniform fixtures permit adjacent low-grade 8-bit rounding
  plateaus but reject any reversal or disappearance at +15.
- `STATICALLY VERIFIED`: only shader composition/strength and their tests
  changed in this follow-up; English text and `git diff --check` pass.
- Integrated incremental Release build passed using `Build-Client.ps1
  -NoDeploy`, including 58,659 architecture checks, 221 socket checks, and four
  skin shader gates. With no running project.exe process, installed the build
  as `tmproject/client748/project.exe`. Build and runtime SHA-256 match:
  `F77F4B2F135A1A2C47C6D522FF88A1325BAA266B628842BCCC2995C0354F33EC`.
  Native/runtime WYD.exe remain unchanged. Research validation passed.
- `CLIENT-TESTED`: pending for this darker candidate; GPU readbacks do not
  establish perceived visibility on the actual equipped character.
- Changed in this follow-up: `DyePixelShader.h`,
  `DyeTextureStagesDeviceTests.cpp`, this record, and generated local outputs.
  No files removed. The preceding `CMesh.cpp` change is preserved.

### Lighter reflection clarification: retain the 48% application ceiling

The user clarified that the reflection must be lighter, not darker, than the
dye. This supersedes the darker-shade interpretation above. Keep the same
refinement curve and environment mask, but blend toward a 25% lighter shade:
`saturate(matteBase * (1 + 0.25*strength*mask))`. The application weight remains
capped at 48%, giving at most 12% local RGB lift before channel saturation.
Black uses its existing graphite palette; no independent white term is added.

A direct sign reversal failed the GPU oracle on bright cloth. Carry only the
bounded lift through the shader phase, then apply it with the final saturated
multiply-add after density clamping and palette multiplication. This preserves
the measurable lighter reflection on saturated density without changing alpha,
fog, texture coordinates, device ownership, or unpainted equipment. This remains
`MODERNIZACAO_COMPATIVEL`; existing native evidence is reused without a new
parity claim or any server, wire, or asset change.

- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.vcxproj` and
  its executable pass 8,110,321 D3D9 pixel checks. The independent oracle now
  requires lighter output, nondecreasing refinement strength, bounded lift,
  animation, alpha and fog preservation for blue, black and pink dyes, including
  fully white base textures with partial alpha. Added failure diagnostics.
- Integrated `Build-Client.ps1 -NoDeploy` passed, including 58,659 architecture
  checks and 221 socket checks. Log: `tmproject/build/lighter-dye-build.log`.
  With project.exe closed, installed only `tmproject/client748/project.exe`.
  Build and runtime SHA-256 match:
  `B4E1FDECFA9CC55FFEB5D546C040DF31EBBBA32C249B14DB6DE831BB2DE1E83B`.
- `STATICALLY VERIFIED`: English authored text, research validation and
  `git diff --check`. Source changes are limited to `DyePixelShader.h` and
  `DyeTextureStagesDeviceTests.cpp`; this record and the local executable were
  updated. No files removed; preceding changes are preserved.
- `CLIENT-TESTED`: pending visual confirmation on the equipped character.
  Automated device readback does not establish perceived in-game contrast.

### Grade +9 reflection visibility on dark and red-dyed armor

The user supplied an in-game screenshot showing a black breastplate and red
trousers at +9 without perceptible reflection. This contradicts any claim that
the prior relative lift was visually adequate. The previous maximum lift was
only 12% of the *already darkened* matte pixel, then reduced again by the
environment mask. On black cloth it could be less than one 8-bit channel step.
The user's actual client observation is evidence of this gap, not a passed
`CLIENT-TESTED` result for the correction.

The compatible rendering correction retains the matte diffuse pass and adds
an animated reflection in the dye palette's own RGB. Reflection strength is
still capped at 48% at +15 and reaches 23% at +9; the texture-derived mask
suppresses its low end and provides spatial movement. The final color is
`saturate(matteDiffuse + tint * strength * saturate(2*(mask - 0.20)))`.
Unlike a multiplicative lift, this remains measurable on dark clothing while
preserving underlying armor detail and avoiding a white highlight. Base alpha,
fog, dye selection, the existing shader lifetime, server state and packets are
unchanged. Classification remains `MODERNIZACAO_COMPATIVEL`.

- `AUTOMATED TESTED`: the Release/Win32 D3D9 test now includes the real red
  dye asset as well as blue, black and pink. It passed 10,813,681 pixel checks
  over refinement +0..+15, three environment phases, two light levels, actual
  armor texels, alpha, fog, shader restoration and fully saturated base texels.
  A +9 regression gate requires at least 128 armor pixels to exceed an 8-level
  black or 15-level colored-channel delta from matte at each phase. Black
  exceeded 2,700 and red exceeded 330 in the measured phases; these are
  synthetic GPU thresholds, not an in-game perception test.
- Integrated `Build-Client.ps1 -NoDeploy` passed; log:
  `tmproject/build/visible-dye-build.log`. With no project.exe process running,
  installed only `tmproject/client748/project.exe`. Its SHA-256 matches the
  build artifact:
  `D752AA20497AE3DE24C36D77C1E3EBB279AD9F297C78939B9955278ABD0F8803`.
- `STATICALLY VERIFIED`: English authored text, `git diff --check`, research
  record validation. Changed `DyePixelShader.h`,
  `DyeTextureStagesDeviceTests.cpp`, this record and the local executable;
  no files removed. Earlier uncommitted rendering changes remain intact.
- `CLIENT-TESTED`: pending user inspection of the installed candidate on the
  equipped black breastplate and red trousers at +9.

### Colored reflection intensity follow-up: 90% ceiling

The user found the previous 48% maximum too weak in the live client and
requested a 90% ceiling. This changes only the coefficient in the existing
refinement curve, not dye selection, the shader mask, base armor texture,
alpha, fog, packets, or server state. The peak palette-colored lift is 43.2%
at +9 and 90% at +15 in the brightest mask regions; +0 remains matte. The
prior 48% ceiling described above is historical and is superseded here.
Classification remains `MODERNIZACAO_COMPATIVEL`.

- `AUTOMATED TESTED`: the Release/Win32 D3D9 device fixture passed 10,813,681
  pixel checks with the 90% cap and independent strength oracle. It covers
  +0..+15, four real dye textures, two lighting levels, three animated phases,
  alpha, fog, shader restoration, and +9 contrast on real armor texels.
- The integrated `Build-Client.ps1 -NoDeploy` passed and produced
  `tmproject/build/TMProject748/Release/WYD.exe` with SHA-256
  `4A31523AFC9574D8CDFDB2F5A01DAE9B928C03C7B718C55891C4470B5430FE1C`.
  The build log is `tmproject/build/dye-sheen-90-build.log`.
- After the game process closed, installed only
  `tmproject/client748/project.exe`; its SHA-256 matches the build artifact.
  `STATICALLY VERIFIED`: authored English text and `git diff --check` passed;
  the research record validator accepted the update. No files were removed.
  `CLIENT-TESTED`: pending in-game inspection of the black breastplate and
  red trousers at +9. The device test does not establish perceived brightness.

### Silver refinement reflection for every armor dye

The user rejected the dye-colored reflection and requested silver on every
dye color. This supersedes the earlier colored-reflection choice without
changing the dyed diffuse base, texture identity, alpha, fog, server state, or
packets. The existing refinement curve still reaches 43.2% at +9 and caps at
90% at +15; +0 remains matte. The reflection now adds a neutral silver RGB
term after diffuse dye shading. Each dye uses a suitable palette channel for
the reflection mask, rather than averaging saturated colors with dark channels.
Yellow's available palette channel is uniform, so its silver lift is uniform
instead of UV-animated. Classification: `MODERNIZACAO_COMPATIVEL`.

- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.exe` passed
  27,033,841 pixel checks across all ten dye legends 116..125, refinement
  +0..+15, three environment phases, two light levels, the real armor texture,
  a saturated base fixture, alpha, fog, and shader-state restoration. The
  independent oracle checks silver RGB, the 90% bound, matte +0, monotonic
  strength and visible +9 contrast. UV movement is required only where the
  palette channel has variation; yellow's uniform asset is checked against
  the same silver reflection formula without an impossible movement assertion.
- Integrated `Build-Client.ps1 -NoDeploy` passed, including architecture and
  socket tests. Installed only `tmproject/client748/project.exe` after
  confirming no `project.exe` process was running. Build and runtime SHA-256
  match: `78617328F1EC441D1B843189A1F589AEFFA56CFCFA3ECDAD8432660384F415F8`.
- `CLIENT-TESTED`: pending visual inspection of the installed candidate on
  dyed +9 armor. Automated D3D9 readback does not establish perceived sheen.

### Grade-texture streak instead of a whole-surface reflection

The user's later field screenshot rejected the silver wash: most of the dyed
cloth must stay at its dye color while a narrow line moves over it. The user
identified the equipped item's inventory preview as a working reference. In
the current client, `TMMesh::RenderForUI` selects the per-dye, per-grade effect
texture for inventory icons (the same 275/288/301/314/327/340/353/366/425/392
families used by `CMesh::RenderMesh`), and `TMMesh::Render` animates UV channel
1. `TMItem::Render` handles world items, not inventory icons. This agrees with the
recorded native `FUN_004c3eec` texture selection, but does not prove that the
new pixel shader exactly duplicates the native fixed-function composition.
Classification: `MODERNIZACAO_COMPATIVEL`; the preceding whole-surface silver
choice is superseded.

The adapted armor shader now samples the unrefined and grade-specific textures
at the same environment UV, thresholds only their positive RGB difference,
and applies the 90%-capped silver term to that local streak. A separate fixed
palette sample retains the dye color elsewhere. The original grade-specific
texture remains the unsupported-shader fallback. The yellow family is solid
at all grades, so the shader uses the silver family's animated difference as
its streak mask while keeping yellow's own palette. No WYS file, resource ID,
item state, packet, or server behavior changed.

- Source evidence: `TMMesh.cpp::TMMesh::RenderForUI`,
  `CMesh.cpp::CMesh::RenderMesh`, `DyePixelShader.h`; the native dye texture
  selection is already cited above at `WYD.exe` `FUN_004c3eec`.
- Asset evidence: existing `Effect/{bl,re,gr,si,dk,vi,or,pi,ye,db}0000..0011.wys`
  families; yellow's texture is uniform, whereas silver contains spatial
  grade variation. No asset was edited.
- Server: NOT APPLICABLE; display-only composition. ABI/wire: unchanged.
- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.exe` passed
  27,033,841 pixel checks across all ten dye legends, grades +0..+15, three
  moving UV phases, real armor and saturated fixtures. Readback verified matte
  base coverage, the bounded streak, fog, alpha and shader/texture restoration.
  The integrated `Build-Client.ps1 -NoDeploy` also passed its architecture,
  socket and client build gates.
- After confirming the game was closed, installed the candidate at
  `tmproject/client748/project.exe`; its SHA-256 matches the build artifact:
  `23861BD39FA33F1AEE4A5593DCB70904B3F6592BDEF82D7EAE3B3BE2A34065E4`.
  The prior executable is backed up under the ignored build directory.
- `STATICALLY VERIFIED`: research validator and `git diff --check` passed.
  `CLIENT-TESTED` remains pending an in-game visual inspection of the +9
  breastplate and trousers. This record remains `LOCATED` for its unrelated
  unresolved UI/portal fronts and must not be promoted by the dye-only test.

### Restore color depth in refined armor dyes

The user reported that dyed armor looked too pale. The existing colored-dye
shader multiplied the matte base by four, clipping bright texels and flattening
texture contrast even though the palette colors are saturated. For dye legends
other than black, the effective diffuse gain is now two. Black keeps its
existing gain of one. The moving silver refinement streak, dye palettes, and
unrefined fixed-function path are unchanged. Classification:
`MODERNIZACAO_COMPATIVEL`; no wire, server, or asset contract changed.

- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.exe` passed
  27,033,841 pixel checks with the updated independent diffuse oracle across
  all ten dye legends, grades +0..+15, two lighting levels, moving streak
  phases, real armor texels, fog, alpha, and state restoration.
- Integrated `Build-Client.ps1` passed its preflight tests and client build,
  and installed `tmproject/client748/project.exe` with SHA-256
  `2586980D953B8E6FC8D8446342038C792F444083405580C978ACA636B8AA1BA0`.
- `CLIENT-TESTED`: pending visual inspection of the installed client. Automated
  readback confirms the composition but not the perceived color depth.

### Increase dye saturation without raising exposure

After seeing the restored cloth contrast, the user requested more vivid dye
colors. The refined-armor shader now expands palette chroma by 25% around its
average channel value, then clamps each channel. The twofold colored-dye
diffuse gain, black dye, and the separate moving silver refinement streak keep
their previous behavior. This remains a display-only
`MODERNIZACAO_COMPATIVEL`; assets, IDs, item state, and wire protocol are
unchanged. The pixel shader stays within its eight-arithmetic-slot phase
limit by folding the existing light gain into the diffuse dot-product weight.

- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.exe` passed
  27,033,841 pixel checks using the saturated-palette oracle across all ten
  dye legends, grades +0..+15, two lighting levels, moving streak phases,
  real armor texels, fog, alpha, and state restoration.
- Integrated `Build-Client.ps1 -NoDeploy` passed its preflight tests and
  client build. After the game process closed, the built artifact was installed
  at `tmproject/client748/project.exe`; both SHA-256 values match:
  `4247D91D5E140163FBCF79BA6D921D7C85721F6B894EE681B4953CF9A6957488`.
- `CLIENT-TESTED`: pending in-game visual judgment of the new saturation.

### Match equipped armor contrast more closely to its inventory icon

The inventory item renderer combines its dye and base texture through the
legacy fixed-function stages, while the equipped mesh uses the separate
refined-armor pixel shader. The user's black-dye screenshot shows stronger
light/dark separation in the inventory icon than on the equipped character.
Classification: `MODERNIZACAO_COMPATIVEL`; this adjusts only the equipped
armor's display composition, not the item, asset, wire, or server contract.

The shader now expands the base texture's grayscale albedo around its 50%
midpoint by 25% before diffuse lighting and dye multiplication. This deepens
dark weave and strengthens existing light details without increasing palette
saturation, altering the dye hue, or brightening the entire armor. The moving
silver grade streak remains independent. The base texture is sampled in both
pixel-shader phases so its alpha remains defined after the phase marker. This
is an approximation of the inventory icon's contrast, not a claim of exact
pixel identity across different geometry and lighting.

- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.exe` passed
  27,033,841 pixel checks with an updated independent contrast oracle across
  all ten dyes, grades +0..+15, lighting levels 140/255, real armor texels,
  moving grade streak, fog, alpha, and render-state restoration.
- Integrated `Build-Client.ps1` passed preflight tests and installed
  `tmproject/client748/project.exe`; build and candidate SHA-256 match:
  `68EBC0B5D92131FE2AD84774A1D98D13C7ABD8EA28B3489D3EAE583555CFC726`.
- `CLIENT-TESTED`: pending in-game comparison of inventory and equipped armor.

### Correct inventory dye contrast direction

The user clarified that the preceding contrast adjustment targeted the wrong
consumer: the inventory icons were too dark and high-contrast, while the armor
on the character was already legible. The preceding 25% albedo-contrast change
to equipped armor is therefore superseded and reverted. Classification:
`MODERNIZACAO_COMPATIVEL`; this changes display composition only, not assets,
item state, server behavior, or the wire contract.

`RenderDevice.cpp` dispatches equipment cells to `TMMesh::RenderForUI`. Its
legacy dyed-item path combines the base and grade texture with fixed-function
`DOTPRODUCT3`/`MODULATE4X`, which clips dark texels. For legends 116-125 with a
graded two-UV mesh, the inventory path now uses the same matte palette and
localized moving refinement-streak shader as equipped armor. It reuses the
existing effect texture IDs, including the silver-family streak mask for
yellow dye. The old fixed-function path remains the fallback when the pixel
shader or required textures are unavailable. The shader and sampler binding
restore prior device state after the icon draw.

- Source evidence: `RenderDevice.cpp` 3D control dispatch,
  `TMMesh.cpp::TMMesh::RenderForUI`, `CMesh.cpp::CMesh::RenderMesh`, and
  `DyePixelShader.h`. The native dye selection was previously recorded from
  `WYD.exe` `FUN_004c3eec`; this is not a claim of pixel-identical rendering.
- `AUTOMATED TESTED`: Release/Win32 `DyeTextureStagesDeviceTests.exe` passed
  27,033,841 pixel checks after the contrast reversal, covering all ten dyes,
  grades, lighting, real armor texels, moving streak, fog, alpha, and binding
  restoration. An in-game icon comparison remains necessary.
- Integrated `Build-Client.ps1` passed its asset checks, architecture tests,
  socket tests, and Release/x86 client build. It installed
  `tmproject/client748/project.exe`; the built and installed SHA-256 match:
  `AEC3FD508CBC49F74C1E81DC2C6FE10214564B806835D31EA62B342206413679`.
- `CLIENT-TESTED`: pending side-by-side visual comparison of the inventory
  icons and worn armor in the running client. No other file or asset was
  removed by this adjustment.
