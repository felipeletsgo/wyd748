---
id: ground-click-marker
title: Ground click marker during local movement
subsystem: movement
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-11-09
---

# Ground click marker during local movement

## Question

Does the native field scene render a marker at the left-click ground position?

## Evidence boundary

- USED: native `references/client748/WYD.exe` and read-only Ghidra `WYD748`.
- USED: active field scene source, mesh entries, and effect texture entry.
- NOT APPLICABLE for parity: the player supplied a screenshot from a modern server.
  It illustrates the effect but cannot establish the 7.48 implementation.

## Native 7.48 flow

### Callers

`FUN_004AF854` dispatches `WM_LBUTTONDOWN` to the scene. Field handler
`FUN_004625DA` applies its input gates and picks a world position.

### Callees

- `FUN_00435B13` constructs two meshes via `FUN_004D2230` and a ground
  billboard via `FUN_004CB900`. It adds them to the scene's effect subtree
  via `FUN_0054AC09`. The scene stores their pointers at `0x26E84..0x26E8C`.
- The ground movement branch copies the picked world position into all
  three effects at `0x00463640..0x004636A5`. It derives their Y position from
  terrain height and lowers the billboard relative to the meshes. It then
  calls `FUN_00461ED8` for movement.
- Field update `FUN_004776C3` rotates the meshes, pulses the billboard scale,
  and sets visibility at `0x004786D2..0x00478739`.

## State and lifecycle

The scene creates and reuses one set of three effects. The frame update shows
them when the local actor walks or runs and the native target and scene
guards permit it. It hides all three otherwise. The effect subtree owns the
nodes and follows the scene lifecycle. The native frame update checks that
all three pointers exist before using them.

## Wire, ABI, and resources

The marker is local. It has no packet or server state.

| Native resource | Active asset |
| --- | --- |
| Mesh 316, color `0xFF111188` | `Effect/point1.msa` |
| Mesh 317, color `0xFFFF0000` | `Effect/point2.msa` |
| Effect texture 232, color `0xFF9999AA` | `Effect/point.wys` |

`Mesh/MeshList.txt` maps both meshes. `Effect/EffectTextureList.bin` maps
texture 232. All three active asset files match the native reference bytes.

## Current mapping

`InitializeCompatFieldScene` creates `m_pTarget1`, `m_pTarget2`, and
`m_pTargetBill` in the effect subtree. `OnMouseEventCompat` positions the
marker after accepting a ground click. The field tick animates it while
the local actor walks or runs toward a retained ground destination.

## Delta matrix

| Behavior | Native 7.48 | Current source |
| --- | --- | --- |
| Marker resources | Meshes 316 and 317, texture 232 | Present |
| Marker instance | Three scene-owned effects | Three scene-owned effects |
| Position and visibility | Ground click and field update | Ground click and field update |

## Decisions

Classification: `PARIDADE_NATIVA`. Reuse the existing resources and clear
the marker when its ground destination is superseded, reached, or canceled.

## Gaps

In-game appearance, hide-effect settings, and lifecycle acceptance remain
unverified. The diagnosis does not establish the cause of movement pauses.

## Validation

Static only. Read `tmproject/build/native-research/ground-click-marker/sites.txt`
for the instruction windows and `flow.txt` for the related functions. The
full initializer decompilation timed out. Focused instruction windows
verified its three constructors, pointer stores, and subtree insertion.
Compared active asset bytes and mapped IDs against the 7.48 reference.
