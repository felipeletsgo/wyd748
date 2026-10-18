---
id: instance-runtime-counter-controls
title: Runtime instance time and objective counters
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-16
---

# Runtime instance time and objective counters

## Question

How should the 7.48-compatible path construct and update the runtime text
controls driven by WYD-Go time and objective packets?

## Evidence boundary

- Used: the native binary identified above, Ghidra project
  `WYD748Native_20260821.gpr`, and decompiled `FUN_00492E7D`,
  `FUN_00489192` and `FUN_00489260`.
- Used: current `InitializeScene`, `InitializeCompatFieldScene`, `FrameMove`,
  counter handlers and `SControlContainer` ownership.
- Used: WYD-Go emitters of `OpInstanceTime=0x3A1` and `OpInstanceMobs=0x3B0`
  in Big Cube, Uxmal and item instances, including Nightmare and Water rooms.
- No additional texture or serialized control resource is required.
- Secondary server sources and official gameplay guides do not establish
  native client control construction or geometry.

## Native 7.48 flow

### Callers

`FUN_00492E7D @ 0x00492E7D` dispatches `0x3A1` to
`FUN_00489192 @ 0x00489192` and `0x3B0` to
`FUN_00489260 @ 0x00489260`.

### Callees

`FUN_00489192` stores the received value, records the current time, formats the
timer and makes its text visible. `FUN_00489260` formats the remaining count,
records the current time and shows the second text. Both call `FUN_0058F078`
and virtual control methods. The exact native construction and teardown have
not been located, so this record remains `LOCATED`.

The existing decompilation at
`tmproject/build/native-research/decompiled/FUN_00489260.c` independently proves
the counter format: `s__d_Left_005b88b0` is `%d Left`, with the packet value at
`+0xC`. The handler records time at `+0x27910` and updates the control at
`+0x278dc` through virtual offsets `+0x80` (text) and `+0x60` (visible, value
one), including a count of zero. This narrow format correction is
`PARIDADE_NATIVA`; it does not promote the entire lifecycle to native parity.

## State and ownership

The full source initializer allocates three runtime `SText` objects and
transfers ownership to `SControlContainer`: time, remaining count and quest
time. The compatible initializer used to return before allocation, leaving
null members. Instance packets and dungeon transitions in `FrameMove` could
then dereference them.

`InitializeRuntimeCounterTexts` now constructs these controls for both
initializers, preserving initial state, fonts, colors and coordinates. Each
consumer tolerates individual allocation failure without discarding unrelated
state. Accepted controls remain owned by the container.

## Packet-driven lifecycle

The timer and counter follow server packets, not hardcoded map coordinates.
`0x3A1` starts the timer; zero hides it. `0x3B0` displays `%d Left`. A zero
count remains visible while the timer is active: zero protected Nightmare
NPCs is meaningful state, not encounter completion. The server clears the
timer before sending zero count on departure, hiding both controls. The
`0x3BB` objective counter retains its `current / max` format.

This compatible lifecycle supports transitions, quizzes, continuous respawn
and alternate rooms without depending on `RenderDevice::m_bDungeon` or its
historical offset list. Its complete native equivalent is not yet proven.

## Wire, ABI and resources

The wire format is unchanged. Go emitters retain the 16-byte
`MSG_STANDARDPARM`, matching type/opcode and the unsigned value at `+12`.
Runtime controls use `ControlID=-2`; no serialized ID or new asset was added.

## Current mapping

`InitializeScene` and `InitializeCompatFieldScene` use the shared constructor.
`OnPacketStartTime`, `OnPacketRemainCount`, `OnPacketRemainNPCCount`, both
Quest12 handlers, `SetQuestStatus`, `UpdateQuestTime` and `FrameMove` tolerate
individual missing controls.

## Decisions and gaps

- Preserve the existing packet boundary and container ownership.
- Use the proven native remaining-count label, including zero during a timer.
- Do not infer native geometry from the current source initializer.
- Locate the exact native allocation and teardown before claiming that parity.
- Validate Big Cube, Uxmal and item instances in the built client at multiple
  resolutions, including expiry, scene changes, departure and relogin.

## Validation

Historical baseline: the Release build passed `Build-Client.ps1` with 26,968
checks and static assertions; `go test ./... -count=1` and `git diff --check`
passed. Transport tests covered truncation, oversize/null input, mismatched
types, single delivery and byte preservation. Historical candidate SHA-256:
`840D3E2B124B732640F9556F023CFC16920F8F86D7EDA6ACD5EE98A697BA17E2`.

2026-10-16 follow-up: `Build-Client.ps1 -NoDeploy` passed with 1,229,517
architecture checks, 231 socket receive checks, 83 crash-capture checks and
56 launcher checks. Candidate SHA-256:
`E96B8514B8B5CEC3A6C7D832DE8E0FE3AA11F4A390816A74C96A34285CCB5A36`.
The focused Nightmare suite and complete Go game package passed, including
survivor-count emissions and departure clearing. The installed client was not
replaced or executed. These are static/build and server-test results, not
`CLIENT_TESTED` evidence.
