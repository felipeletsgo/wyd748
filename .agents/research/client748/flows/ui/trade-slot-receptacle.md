---
id: ui-trade-slot-receptacle
title: Occupied player-trade slot keeps the FieldScene2 receptacle
subsystem: ui-grid
status: UNMAPPED
native_sha256: UNRESOLVED
updated: 2026-10-14
---

# Occupied player-trade slot keeps the FieldScene2 receptacle

## Question

What does a player-trade slot (`GRID_TRADEOP`, `GRID_TRADEMY`) draw behind its
item once an item is placed in it?

## Evidence boundary

- Executable/hash: not consulted in this change; the native binary and Ghidra
  project live in the local-only `references/` collection.
- Ghidra project/corpus: `FUN_0040fc3e` (grid frame) is located in
  `inventory/functions.tsv`; its trade-slot branch is not traced yet.
- Assets: `tmproject/client748/UI/FieldScene2.bin` supplies the trade panel
  `576` and its slot receptacles.
- Current source: `SGridControl::FrameMove2` in
  `tmproject/TMProject748/internal/ui/SGrid.cpp`.
- Server: not applicable; slot rendering is client-only.
- Real client: a 2026-10-14 screenshot of the recompiled client shows every
  occupied slot of both trade grids as a solid black square while empty slots
  show the stock receptacle.

## Native 7.48 flow

### Observable entry

- Event: an item enters either side of the player trade window.
- Preconditions and initial state: trade panel `576` visible, slot empty.
- Observable output: the item is drawn centered over the stock receptacle,
  the same background an empty slot shows.

### Callers

Not traced; the grid frame is reached through the control vtable slot
recorded in `exports/grid-item-mesh-scale-vtable-callers.tsv`.

### Main function

`FUN_0040fc3e` (located, trade-slot branch not traced).

### Callees

Not traced.

### Outputs and errors

None beyond the render list.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| item placed in trade slot | trade visible | `SGridControl::FrameMove2` | item over stock receptacle | none | empty slot draws nothing extra |

### Vtables, vptrs, and receivers

N/A: no vtable or receiver changes.

### Ownership

N/A: the change removes a per-frame render node; no allocation changes.

### Partial failure

N/A: rendering only.

### Cleanup and teardown

N/A: the item's `m_GCEnable` is no longer queued for trade slots, and the
existing item teardown still removes it when another grid queued it.

### Shutdown

N/A: unchanged.

### Logout and relogin

N/A: unchanged.

## Wire, ABI, and resources

No packet, ABI, struct, or asset change.

## Current mapping

### Buildable source

The centered single-cell branch of `SGridControl::FrameMove2` queued the
item's `m_GCEnable` as an opaque `0xFF000000` rectangle for `GRID_TRADEOP`
and `GRID_TRADEMY`. That backdrop came from the 7.69 source and covered the
FieldScene2 receptacle. `GRID_TRADEMY2` had already dropped it for the same
reason. The branch now queues no backdrop for any trade slot.

### WYD-Go

No change; the server remains authoritative for trade contents.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| occupied slot background | stock receptacle (player report) | stock receptacle | opaque black backdrop | N/A | drop the backdrop |

## Decisions

- Classification: `MODERNIZACAO_COMPATIVEL` removal of a 7.69 overlay that
  the 7.48 asset does not need; no wire or resource contract changes.
- Keep the blocked-item red overlay and placement preview of regular grids.

## Gaps

Trace the trade-slot branch of `FUN_0040fc3e` if a parity claim is needed.

## Validation

- Research: static source review only.
- Automation: `SceneDisconnectContractTests` asserts the centered single-cell
  branch queues no `m_GCEnable` backdrop.
- Real client: pending; open a trade, add items on both sides, and confirm
  the occupied slots show the stock background.
