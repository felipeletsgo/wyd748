---
id: chat-channel-filter-toggle
title: Chat tab click toggles a channel filter and its indicator
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-06
---

# Chat tab click toggles a channel filter and its indicator

## Question

Which controls does a click on the Normal/Party/Whisper/Guild chat tabs change
in the native 7.48 client, and which consumers read that state?

## Evidence boundary

- Executable/hash: `references/client748/WYD.exe`, SHA-256 in front matter.
- Ghidra project/corpus: `tmproject/build/native-research/WYD748.gpr`
  exports `FUN_00435b13.c` (binder), `decompiled/FUN_004662c5.c` (control
  events), `FUN_00481dd6`, `FUN_00481ff3`, `FUN_0052d060`, `FUN_0052d2c8`,
  `FUN_0049889a` (consumers).
- Assets: `tmproject/client748/UI/FieldScene2.bin`.
- Current source: `TMFieldScene` chat bindings in `TMFieldSceneUI.cpp`,
  `internal/ui/FieldChatControl.h`, `TMFieldSceneChat.cpp`,
  `TMHumanPackets.cpp`.
- Server: none; filtering is local presentation.

## Native 7.48 flow

### Observable entry

- Event: left-button release on tab 5697..5700 (`0x1641..0x1644`).
- Preconditions and initial state: mode 2; binder sets `+0x1e8`
  (`SButton::m_bSelected`) to 1 on the four tabs and the four indicators.
- Observable output: indicator 12835..12838 (`0x3223..0x3226`) and tab
  selection flip; matching incoming chat is dropped while off.

### Callers

`SButton` mouse handler `FUN_004032e8` (vtable `0x005a3674`) raises the
control event on release; `FUN_004662c5` dispatches it.

### Main function

`FUN_004662c5`: for `0x1641..0x1644` in mode 2, `primary.m_bSelected =
(primary.m_bSelected == 0)` and copies the value into the paired indicator
(`Field[0x9ebd..0x9ec0]` -> `Field[0x9ec1..0x9ec4]`).

### Callees

None besides the two field writes.

### Outputs and errors

Consumers in mode 2: General `+0x27af4` (`FUN_0052d060`, `FUN_0049889a`),
Party `+0x27af8` (`FUN_00481dd6`, `FUN_00481ff3`, `FUN_0052d2c8`), Whisper
`+0x27afc` (`FUN_0052d2c8`), Guild `+0x27b00` (`FUN_00481ff3`,
`FUN_0052d2c8`) drop the message when `m_bSelected == 0`.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| tab click, on | mode 2 | `FUN_004662c5` | tab and indicator off | none | none |
| tab click, off | mode 2 | same | tab and indicator on | none | none |
| chat received, off | channel off | consumers | message dropped | none | returns handled |

### Vtables, vptrs, and receivers

`SButton` vtable `0x005a3674`; `m_bSelected` at `+0x1e8` matches the source
`SButton` field order.

### Ownership

Buttons belong to the resource container; the scene borrows them.

### Partial failure

A missing tab leaves its channel always on (null checks in consumers).

### Cleanup and teardown

N/A: no allocation.

### Shutdown

N/A: state lives in resource controls.

### Logout and relogin

Rebinding a Field resets every channel to on.

## Wire, ABI, and resources

FieldScene2.bin, parent 12624: buttons 5697..5700 (texture sets 415..418,
50x15) at x=3/55/107/159, y=22; captions 5701..5704 are **SText** at
x=12/64/116/168; indicators 12835..12838 are 4x8 buttons (texture set 427)
at x=6/58/110/162, y=25.

## Current mapping

### Buildable source

The source bound the paired pointers `m_pChat*_C` to 5701..5704 and cast the
SText captions to `SButton`, writing `m_bSelected` past the text object and
never updating the real indicators. The bindings now use
`TMB_CHAT_*_C` (12835..12838) and refresh all eight buttons once at bind.

### WYD-Go

Not affected.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Paired indicator | 12835..12838 | 5701..5704 texts | 7.59 IDs | N/A | port |
| Toggle semantics | invert primary, copy | same via `field_chat::Handle` | same | N/A | keep |

## Decisions

`PARIDADE_NATIVA` resource binding fix. `field_chat::Route` still accepts
5701..5704; SText never raises control events, so the alias is inert and left
unchanged.

## Gaps

Event delivery to the tabs over the chat list was not observed in the real
client in this session.

## Validation

- Research: binder/handler/consumer exports and resource walk.
- Automation: existing `FieldChatControlTests` and ArchitectureTests PASS.
- Real client: pending on candidate SHA-256
  `5A4CFE72690469263A97E490129D2AD946EE9A9C140DB5895DF7DBAD11290906`.
