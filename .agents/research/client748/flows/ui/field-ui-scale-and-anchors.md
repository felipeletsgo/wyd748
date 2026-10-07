---
id: field-ui-scale-and-anchors
title: Field UI controls keep resource size and anchor to the viewport
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-06
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

### WYD-Go

Not affected.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Control size | resource pixels | resource pixels | scaled | N/A | ported |
| HUD anchors | table above | table above | 7.59 sticks | N/A | ported |
| Quest remain time | `(W - 140, 5)` | `(W - 140, 5)` | 7.59 | N/A | ported |
| Font | Tahoma 13 | Tahoma 13 | resolution-scaled | N/A | ported |

## Decisions

`PARIDADE_NATIVA` for the UI scale model and the anchors.

## Gaps

- Previous layout records assumed native scaling; they hold only for mode 1.
- Native 12834 and the three controls created at `+0x9e45..0x9e47` (placed
  from 5745) are not repositioned by the source.
- Select-character slide checks (`TMSelectCharSceneInput`, `307/547 * ratio`)
  still compare scaled positions; they only gate camera/sound animation.

## Validation

- Research: constructor, anchor helpers and binder listing.
- Automation: ArchitectureTests PASS (build with the quest-timer fix).
- Real client: pending (manual test by the user).
