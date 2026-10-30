---
id: sephira-resurrection-prompt
title: Sephira Resurrection prompt on a dead right click
subsystem: combat
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-30
---

# Sephira Resurrection prompt

## Question

How does the native 7.48 client let a dead player use Resurrection (skill 99)?
The source client rejected 99 in `SkillUse` and offered only the recall prompt.

## Evidence boundary

- Executable/hash: native `WYD.exe`, hash above.
- Ghidra corpus: `tmproject/build/native-research/decompiled/FUN_004625da.c`
  (field mouse input) and `FUN_004640e5.c` (message box events).
- Assets: `UI/strdef.bin` row 227, "Do you want to resurrect?" (EN) and the
  matching PT row. The address `DAT_00a3b2f8` is row 27 (`DAT_00a34ef8`) + 200.
- Current source: `TMFieldSceneInput.cpp`, `TMFieldSceneMovement.cpp`,
  `TMFieldScene.cpp`, `DeathMotionPolicy.h`; server `affects.go` case 99.

## Native 7.48 flow

### Observable entry

- Event: right button down (`0x204`) on the field without `MK_CONTROL`.
- Preconditions: the player is dead (HP < 1 or `m_cDie == 1`) and no
  Resurrection request is pending (`+0x27458 == 0`).
- Observable output: message box 227 with event 99.

### Callers

The field scene mouse handler runs for every mouse message in the field.

### Main function

`FUN_004625da` scans the 20 short-skill bytes at `g_pObjectManager + 0xEF6`
for 99. When found it sets `+0xEF5`, refreshes the bar and calls
`SetMessage(str[227], 99, 0)` and `SetVisible(1)`. A left click instead
offers the recall prompt (row 27, event 11). The same function hides a
visible event-99 box once HP is positive again.

### Callees

`FUN_004640e5` handles event 99: it sets `m_cResurrect = 1` and stores the
server time. The field tick sends the attack packet with `SkillIndex = 99`
after five seconds (`FUN_00476006`).

### Outputs and errors

Without 99 on the bar, no Resurrection prompt appears.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| Right click | dead, 99 on bar, none pending | `FUN_004625da` | box 227, event 99 | none | no 99: no box |
| Confirm | box 227 | `FUN_004640e5` | `m_cResurrect = 1` | timer | cancel: nothing |
| Five seconds | pending | field tick | 0x39D skill 99 sent | server decides | server failure: still dead |
| HP restored | box visible | field tick | box hidden | none | none |

### Vtables, vptrs, and receivers

Message box vtable slots `+0x8C` (SetMessage) and `+0x60` (SetVisible).

### Ownership

The server owns revival; the client only requests it.

### Partial failure

A rejected or failed request leaves the character dead; the recall prompt
remains available.

### Cleanup and teardown

The pending flag clears after the request or after six seconds.

### Shutdown

N/A.

### Logout and relogin

No persisted client state.

## Wire, ABI, and resources

No new packet. The existing 48-byte `0x39D` request with skill 99 targets the
caster. Message row 227 already exists in root, EN and PT resources.

## Current mapping

### Buildable source

`TMFieldScene::OfferResurrectionPrompt` and
`death_motion::CanOfferResurrectionPrompt`; the field tick hides event 11 and
event 99 boxes after revival.

### WYD-Go

`affects.go` case 99 revives a dead caster's self-cast.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Dead right click | box 227, event 99 when 99 is on the bar | recall prompt only | prompt restored | unchanged | PARIDADE_NATIVA |
| Prompt dismissal | hidden at positive HP | event 11 only | events 11 and 99 | unchanged | PARIDADE_NATIVA |

## Decisions

Restore the native input branch only. Keep `SkillUse` rejecting 99 for living
players. The `+0xEF5` write only refreshes the bar highlight and is omitted.

## Gaps

The server success chance `(level + 1) / 5` percent has no located source.
W2PP `_MSG_Attack.cpp` always revives with 1..50% HP and may recall. TMSRVIN
has three `== 99` checks without the full rule. Owner decision is pending.

## Validation

- Research: STATICALLY VERIFIED, native decompile and strdef row 227.
- Automation: AUTOMATED TESTED, `ResourceBarProjectionTests.cpp`.
- Real client: CLIENT-TESTED pending.
