---
id: field-ui-scale-and-anchors
title: Field UI controls keep resource size and anchor to the viewport
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-10
---

# Field UI controls keep resource size and anchor to the viewport

## Question

How does the native 7.48 client size and place FieldScene2 controls at a
viewport other than 800x600, and where does the source differ?

## Evidence boundary

- Executable/hash: `references/client748/WYD.exe`, SHA-256 in front matter;
  floats read from the PE image (`0x005a34a0` = 0.5, `0x005a3670` = 10, ...).
- Ghidra project/corpus: `tmproject/build/native-research/WYD748.gpr`;
  `FUN_0040104c` (SControl constructor), anchor helpers `FUN_00401304`,
  `FUN_004013c2`, `FUN_00401367`, `FUN_004013ad`, `FUN_0040137c`,
  `FUN_004013f3`; binder `FUN_00435b13` decompile and instruction listing
  (receivers resolved from `MOV ECX,[reg+field]`).
- Assets: `tmproject/client748/UI/FieldScene2.bin`.
- Current source: `SControl::SControl` (`internal/ui/controls/SControlBase.cpp`),
  `TMFieldScene::InitializeCompatFieldScene` and `PositionCompat*`.
- Server: none.

## Native 7.48 flow

### Observable entry

- Event: Field scene construction.
- Preconditions and initial state: UI mode `DAT_005b892c == 2`, forced by
  `FUN_0055bc0a`.
- Observable output: control rectangles.

### Callers

`FUN_00435b13` (Field initializer) builds and anchors the controls.

### Main function

`FUN_0040104c` multiplies position and size by `1.0` in mode 2 (only mode 1
uses the viewport ratios `DAT_005b490c/DAT_005b4910`). Controls therefore
keep their resource pixel size. Anchors applied afterwards:

| Control | Native placement |
| --- | --- |
| 5716 main info | scale position to viewport, bottom, `x = W/2 - w/2 + 4` |
| 12624 chat strip | scale, bottom, `x = x5716 - w` |
| 5377 chat list | `y = y12624 - h - 6` |
| 5722 | `(W/2 - w/2, H - h)` |
| 5723 HP/MP orb | scale, `(0, 0)` |
| 5732 | `(0, H - h)` |
| 5733 | `(W - w, H - h5716 - 30)`; 5735 at `(W - 10, y5733)` |
| 5739 chat edit bar | position of 12624 |
| 5744 main menu button | scale, right, bottom |
| 292 main menu | scale, right, `y = y5744 - h`, `x -= 2` |
| 5745 short skill bar | `((W-800)/2, (H-600)/2)` offset, bottom, `x = x5716 + w5716` |
| 513 Character | `(W/2 - 1.5w - 10, H/2 - h/2)` |
| 257 Inventory | `(W/2 + w/2 + 10, H/2 - h/2)`; 1825 Cargo right-aligned to it |
| 1905, 632, 640, 626, 646, 669, 576, 1793, 1889, mix roots | centred |
| 6400 gamble | centred + 10 |
| 880 | `(W/2 - w/2, H/2 - 200)` |
| 5742 party button, 1857 | `(0, H - h - 165)` |
| quest remain time | `(W + 35, 5)` below 800 px, else `(W - 140, 5)` |
| 5722/5732/5735/5742 hints (`SButton::m_pAltText`, +0x1f0) | `(w5716/2 - 58, y - 15)`, `(x + 5, h5723/2 - 5)`, `(x - 40, h5733/2 - 5)`, `(x + 5, h1857/2 - 13)` |
| status labels (strdef 157/232/172: PK Mode, Auto Running Mode, Run Mode) | 60/80/60 x 18, bordered `0x88770000/0x88000077/0x88007700`; first at `(x5745 + 61, y5745 - 20)`, the others in a row |
| 12834 affect description | right edge minus 40, `y = y(Auto Running) - 20` |
| HP/MP bars 1169/1170, overlays 1158/1159, mount 12804/12805 | height + 6, `y - 3` |
| input-language marker (scene +0x60) | `(x5739 + 3, y5739 + 5)` |
| 6135 event chat | `(300, 300)` |

### Callees

The anchor helpers above and the control `SetPos` slot `+0x70`.

### Outputs and errors

Missing controls are skipped.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| scene construction | mode 2 | `FUN_00435b13` | anchored, unscaled UI | none | missing control skipped |

### Vtables, vptrs, and receivers

Receiver is the Field scene; `SetPos` is control vtable slot `+0x70`.

### Ownership

Controls belong to the resource container.

### Partial failure

N/A: placement only.

### Cleanup and teardown

N/A: no allocation.

### Shutdown

N/A: scene-owned.

### Logout and relogin

A new scene reapplies the anchors.

## Wire, ABI, and resources

No wire. Resource coordinates are 800x600 authored values.

## Current mapping

### Buildable source

Implemented (product decision: native layout):

- `RenderDevice::m_fUIScaleX/Y = 1.0` is the 2D UI scale; `SControl`
  construction, `SText` offsets, grids, grid items, tooltips, message boxes,
  guild marks, skill-belt hit tests, scene-built panels and `BASE_ScreenResize`
  use it. `m_fWidthRatio/HeightRatio` stay W/800, H/600 only where the native
  client also scales in mode 2: 3D meshes in UI (`1 / ratio`, `FUN_0042e8a1`),
  floating world text (`TMFont3`, `FUN_00423034`), overhead label Y offsets
  (`FUN_00504a80`), instance/rank texts (`y = 30 * ratioY`), terrain culling
  and the select-server logos (`FUN_004a8f14`).
- Font: Tahoma 13 at every resolution (`FUN_0042ced1`), `Font.txt` override
  kept.
- HUD anchors follow the native table; `PositionCompatNativeRoots` adds the
  remaining modal roots; AutoTrade/Cargo (`FUN_0044ae38`, `FUN_004484f3`) and
  character selection (`FUN_004a250f`) use the mode-2 formulas.

### Interface scale (product decision 2026-10-07, `MODERNIZACAO_COMPATIVEL`)

The 800x600 mode-2 layout looked too small at larger resolutions, so the
whole UI now scales uniformly: `ClientSettings.txt` `UIScale=Auto` (default)
sets `m_fUIScaleX/Y` to `min(W/800, H/600)`, at least 1 and rounded down to
0.05 (`ResolveUIScale`); a fixed value overrides it. The font is
`13 x scale` pixels (`ScaledFontSize`) and the glyph texture grows to 1024
pixels above 20-pixel fonts. Controls multiply their resource position and
size by the scale; the native anchors keep their formulas with the scaled
sizes, and their fixed offsets go through `RenderDevice::UiPx` (for example
the 4-pixel MainInfo shift, the 10-pixel Inventory gap, Character Select's
30/90/65/40). `scaleResourcePos` maps through the scaled 800x600 layout. At
800x600 every value equals native mode 2.

The 2026-10-10 review found a repeated-call defect in `PositionCompatPartyPanel`:
adding five scaled pixels to the current hint X moved it on every invocation.
`PartyHintOffsetX` now derives X from the scaled button width minus five scaled
pixels, preserving the first native adjustment from SButton's width-minus-ten
initial position. This is an idempotence correction, not new native evidence.
`ClientSettings` also rejects incomplete Auto tokens, non-finite/overflowing
numbers and numeric suffixes while retaining whitespace and trailing comments.
Invalid values leave the previous setting unchanged.

### Message box and message panel

`FUN_00493e70` (scene base) builds MessagePanel2 at `(150, 80, 480, 28)` and
both message boxes at `((W/ratioX - 256)/2, (H/ratioY - 152)/2)` (PE floats
`0x005a41e4` = 256, `0x005a4438` = 152, `0x005a365c` = 2). The constructor
`FUN_00403eb8` keeps a 256x128 root, centres only X through `FUN_0040116e`
with the 232-pixel NewUI width, and creates both message lines at (10, 36) and
(10, 52) with the width of the construction text. The source already matches
this geometry; at larger resolutions it now follows the interface scale.

### Overlays and minimap under the interface scale

Floating combat text (`TMFont3`), overhead label offsets, instance counters
and 3D meshes inside controls used the W/800 and H/600 ratios, which equal
the interface scale at 800x600. They now use `m_fUIScaleX/Y` (meshes:
`scale / widthRatio`), so they keep their proportions on widescreen. The
minimap layout (`mini_map_layout::Next`) is computed in 800x600 layout
pixels and multiplied by the scale, as are its frame, buttons, position text
and the player marker. `SButton` measures its font compensation and hint
height in layout pixels so they are not scaled twice.

### IME candidate row, 2026-10-10

The existing `FUN_00493e70` export proves one `SText`-equivalent control at
scene slot `0x17`, not ten vertically stacked controls. Its constructor call
to `FUN_00401d03` uses X `0x41e00000` (28), Y `H - 20` in mode 2,
width `300 + (W < 800 ? 50 : 0)`, height 16, foreground `0xffffffff` and
background `0x9955aa55`. Targeted reads of the reference PE constants confirm
`0x005a4314 = 20.0f` and `0x005a43c4 = 300.0f`. The earlier gap description
incorrectly stated X = 30; that was not the candidate constructor argument.
The constructor hides the row through vtable slot `+0x60` and registers it
with the scene control container through slot `+0x44`.

`TMScene` now creates and registers only `m_pTextCandidate[0]`; the remaining
array slots retain their existing null initialization. All consumers were
checked: `EventTranslator::SetVisibleCandidateList` already formats the page
in the first control and null-checks spare controls when hiding them;
`NewAppWindow` routes candidate notifications to that method. Container
ownership and scene teardown remain unchanged. No new shared pointer or wire
field is introduced. The source still retains its existing allocation-failure
behavior; this change does not claim to harden control allocation.

`ImeCandidateLayout` returns logical coordinates for `SText` to scale once.
At scale 1 it matches mode 2, including the physical-width threshold; other
scales use `H / uiScaleY - 20` to keep the row above the viewport bottom.
This is native geometry (`PARIDADE_NATIVA`) with the existing product scaling
(`MODERNIZACAO_COMPATIVEL`), not a new native scaling claim. Composition,
candidate paging/selection, IME API failures and text encoding are not changed
or fully validated by this layout batch.

### Layout probe

`WYD748_LAYOUT_PROBE=1` logs, to `client-debug.log`, every visible top-level
window rectangle and its offset from the screen centre whenever that set
changes (`WYD748_LayoutProbe`).

### WYD-Go

Not affected.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Control size | resource pixels | resource pixels times uniform UI scale | nonuniform viewport scaling | N/A | native baseline plus product scaling |
| HUD anchors | table above | native formulas with uniformly scaled offsets | 7.59 sticks | N/A | adapted |
| Quest remain time | `(W - 140, 5)` | `(W - UiPx(140), UiPx(5))` | 7.59 | N/A | adapted |
| Font | Tahoma 13 | scaled font height, minimum 13 | resolution-scaled | N/A | native baseline plus product scaling |

## Decisions

`PARIDADE_NATIVA` for the unit-scale mode-2 baseline and recorded anchor
formulas. `MODERNIZACAO_COMPATIVEL` for the optional uniform scale, strict
settings parsing and repeat-safe Party hint positioning. Uniform scaling at
larger resolutions is a product decision, not native mode-2 behavior.

## Gaps

- Previous layout records assumed native scaling; they hold only for mode 1.
- Resolved 2026-10-07: every anchor of `FUN_00435b13` is now ported (table
  above, mapped with the slot-to-ID listing of the decompile). The three
  controls at `+0x9e45..0x9e47` are the PK Mode, Auto Running Mode and Run
  Mode labels (shown by `FUN_0044ecae`, `FUN_00452737`/`FUN_00499f0e` and the
  score update `FUN_004431e4` while mounted); the source creates them in
  `CreateCompatStatusTexts` and syncs them every frame.
- Select-character slide checks (`307/547 * ratio`): the native slide runs
  only outside UI mode 2 (`FUN_004a2aef`, `DAT_005b892c != 2`), so the checks
  never fire in mode 2 in either client; no change.
- IME candidate geometry is adapted as described above. Candidate rendering,
  paging, composition, focus changes and close/reopen still require a CJK IME
  session in the newly built client; constructor/source tests do not prove
  those runtime flows.

## Validation

- Interface scale: `ClientSettingsTests` (scale, font, glyph texture);
  server-selection, login panel and login message captured at 800x600 and
  1024x768 (scale 1.25) with `tools/client-assets/Capture-ClientScreens.ps1`;
  the layout probe reports the message panel centred (offset 0) at both.
- Field, client-tested 2026-10-07 at 800x600 and 1024x768 with a local
  WYD-Go server and a test account: bottom HUD, status orb, inventory,
  PK Mode / Auto Running Mode labels, PK notice panel, return-to-village
  message box and the compact and expanded minimap. The message box was 16
  pixels left of centre because `SetCenterPos` used the control's 256-pixel
  root instead of the width passed in; it now centres by that width like
  `FUN_0040116e` (x = 284 at 800x600, the native value). Rain and snow were
  not seen in play (no weather period reached); they are covered by
  `WeatherParticlesTests`.
- Native reference captures with `references/client748/WYD.exe` are not
  possible with the current runtime: the KR terrain, mesh and object tables
  (`MAX_ENV_TEXTURE` 2048, 4096 object masks, 16-bit tile indices) do not load
  in the 7.48 executable. Ghidra is the reference instead.

- Research: constructor, anchor helpers and binder listing.
- Automation: ArchitectureTests PASS (build with the quest-timer fix).
- 2026-10-10 automation: `ClientSettingsTests` rejects malformed numeric and
  Auto values; `UiModeContractTests` covers stable Party hint positioning at
  five scales after 100 repeated calculations and checks the production binding.
- IME follow-up: `UiModeContractTests` adds 164 checks across five viewport
  widths, four heights and four scales, plus single-row construction, hidden
  initial state, container registration and null-safe notification bindings.
  `ArchitectureTests` passed 1,153,253 checks and static assertions. These are
  geometry/helper and source-binding tests, not live Windows IME tests.
- Real client: the October 7 captures above do not validate the October 10
  changes. Repeated Party toggles, hover placement and scaled hitboxes still
  need execution in the newly built client.
