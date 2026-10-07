---
id: score-button-hold-repeat
title: Holding a Character '+' button repeats the point allocation
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-06
---

# Holding a Character '+' button repeats the point allocation

## Question

How does the native 7.48 client keep allocating status or mastery points
while the left button stays down on a Character '+' button?

## Evidence boundary

- Executable/hash: `references/client748/WYD.exe`, SHA-256 in front matter.
- Ghidra project/corpus: `decompiled/FUN_004776c3.c` (Field FrameMove),
  `FUN_004662c5.c` (control events), `FUN_004032e8.c` (SButton mouse),
  `FUN_00493cd0.c` (`m_bOver` getter), `FUN_00435b13.c` (panel 513 binding).
- Assets: FieldScene2.bin buttons 1073, 1076, 1079, 1088, 1139, 1142, 1145,
  1154 and panel 513.
- Current source: `TMFieldScene::UpdateScoreButtonRepeat`,
  `native_score_hud::ButtonRepeat`, `TMFieldScene::FrameMove`.
- Server: `internal/game/handlers.go :: onApplyBonus` validates each
  `MSG_ApplyBonus 0x277`; session limit 256 packets/s.

## Native 7.48 flow

### Observable entry

- Event: left button held over a '+' button while Character panel 513 is
  visible.
- Preconditions and initial state: `Field[0x9dad] = Field[0x9dae] = 0`
  (cleared by `FUN_004343a4`).
- Observable output: repeated `0x277` requests through the normal control
  event path.

### Callers

`FUN_004776c3` runs every Field frame after TimeDelay, AirMove
(`FUN_0047ef1d`) and the affect row (`FUN_004770ad`).

### Main function

When `Field[0x9f1f]` (panel 0x201) is visible, the scene looks up
`0x431, 0x434, 0x437, 0x440, 0x473, 0x476, 0x479, 0x482`; the last with
`m_bOver == 1` is the held ID. If no ID is hovered or
`EventTranslator::button[0]` (`DAT_005d0504 + 0x1c`) is zero, both fields
reset. Otherwise the first frame stores the time, `time + 1500 < now`
enters repeat mode, and each `time + 100 < now` calls the scene's own
OnControlEvent (vtable `+0x58`) with `(id, 0)`.

### Callees

Scene OnControlEvent; for those IDs `FUN_004662c5` checks death
(`TMHuman + 0x245`), available points or mastery cap, then sends `0x277`.

### Outputs and errors

Every repeat obeys the same gates as a click; no extra packet type.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| hold starts | panel visible, over '+' | FrameMove | timer armed | none | none |
| held > 1500 ms | same | FrameMove | repeat mode | none | none |
| each 100 ms | repeat mode | OnControlEvent(id,0) | point request | `0x277` | handler gates |
| release/leave | any | FrameMove | reset | none | none |
| panel hidden | any | FrameMove | timer untouched natively | none | none |

### Vtables, vptrs, and receivers

Receiver is the Field scene (`this` of `FUN_004776c3`); slot `+0x58` is its
OnControlEvent.

### Ownership

Two scene integers; buttons are borrowed resource controls.

### Partial failure

A missing button contributes no hover. Server rejections leave the score
unchanged and the next repeat is gated again.

### Cleanup and teardown

N/A: no allocation.

### Shutdown

N/A: scene-local integers.

### Logout and relogin

Field initialization clears the state.

## Wire, ABI, and resources

No new wire. Repeats reuse `MSG_ApplyBonus 0x277` (20 bytes).

## Current mapping

### Buildable source

`UpdateScoreButtonRepeat` reproduces the poll and calls
`OnControlEvent(nativeID, 0)`, which already translates 1073..1154 to the
existing ApplyBonus handlers. The source also resets the timer while the
panel is hidden so a stale arm cannot fire on reopening.

### WYD-Go

`onApplyBonus` remains authoritative; no change.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Hold repeat | 1500 ms then 100 ms | absent | absent | N/A | port |
| Hidden panel | poll skipped | timer reset | N/A | N/A | modernize |

## Decisions

`PARIDADE_NATIVA` timing; the hidden-panel reset is
`MODERNIZACAO_COMPATIVEL`. The native delay is 1.5 s, not 3 s.

## Gaps

None for the decision; real-client observation pending.

## Validation

- Research: FrameMove, handler and SButton exports.
- Automation: `tests/NativeScoreHudTests.cpp` timing boundaries;
  ArchitectureTests PASS.
- Real client: pending on candidate SHA-256
  `5A4CFE72690469263A97E490129D2AD946EE9A9C140DB5895DF7DBAD11290906`.
