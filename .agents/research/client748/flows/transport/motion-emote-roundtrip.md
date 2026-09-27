---
id: motion-emote-roundtrip
title: Motion and emote 0x36A roundtrip
subsystem: world-input-motion
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-24
---

# Motion and emote 0x36A roundtrip

## Question

What does the native 7.48 client send for an emote, how does the response
release pending input, and which values must remain server-owned?

## Evidence boundary

- Native executable: `references/client748/WYD.exe`, SHA-256 in frontmatter.
- Ghidra: `WYD748Native_20260821.gpr`; focused export
  `exports/motion-emote-flow.tsv`, SHA-256
  `75038059083E12532ED4D83B573C3F5D2DFA9C3CE3706FB073CA29D1F99015CD`.
- Assets: NOT APPLICABLE; no animation, sound, or effect assets change.
- Active client: `TMFieldScene.cpp`, `TMHuman.cpp`, `Basedef.h` in
  `tmproject/TMProject748/`.
- Server: `internal/game/character_session.go`, `visibility.go`, `security.go`,
  `internal/wire/codec.go`, and their tests in `wydgo748/`.
- Later TMProject and external guides: NOT APPLICABLE to this contract;
  the native binary, active source, and authoritative server decide it.

## Native 7.48 flow

### Observable entry and outcome

The numpad/menu selects an emote, or a click on the local human toggles
sitting/standing. The FieldScene must be active; the local character must be
alive, the 500 ms debounce expired, and no earlier motion pending. The client
sends `0x36A/20`. A returned frame carrying its own ID applies the animation
and clears the pending motion, allowing the next emote.

### Callers

- `FUN_004541F3` forwards commands `0x9CA8..0x9CB1` to `FUN_00455950`
  with keys `0x60..0x69`; `FUN_00454763` also forwards numpad input.
- FieldScene vtable slot `0x005A429C` points to `FUN_004625DA`, the
  sitting/standing click sender.
- Human vtable slot `0x005A5580` points to `FUN_0052EAA9`, which dispatches
  `0x36A` to `FUN_005296E8`.

### Main function

`FUN_00455950` clears a 20-byte stack buffer, writes `Type=0x36A`, the
local ID, `Motion@12`, and `Direction=0@16`, then calls `FUN_0055F2DD` with
size `0x14`. `FUN_004625DA` builds the same frame.

On receipt, `FUN_0052EAA9 -> FUN_005296E8` reads signed `Motion@12`,
signed `Parm@14`, and the DWORD/float at `+16`. `Motion=100` creates a
firework; `Parm=1` applies character variants, `Parm=2` clears death, and
`Parm=3` creates level-up. For `Motion < 256`, the local ID clears pending
motion. `FUN_005296E8 -> FUN_00523533` applies motion/direction; special
branches attach effects to the scene container.

### Callees

`FUN_0055F2DD` frames and sends the stack buffer. `FUN_0055890A` accepts
this opcode only at `0x14` bytes. On receipt, `FUN_0052EAA9` dispatches to
`FUN_005296E8`, which calls `FUN_00523533` for motion/direction and attaches
valid special effects to the scene container.

### Outputs and errors

Invalid keys, a dead character, incompatible motion, active debounce, or a
pending motion produce no frame. Native emote senders use `Parm=0` and
`Direction=0`; observed motions are `13`, `15..24`, `25`, and `27`.
`Motion=100` and `Parm=1..3` are server-to-client effects, never client
intentions. A received frame of any size other than 20 is rejected before
the human handler.

## State and lifecycle

| Event | Preconditions | Native path | State change | Failure |
| --- | --- | --- | --- | --- |
| Select emote | Alive; debounce free; no pending motion | `FUN_004541F3/FUN_00454763 -> FUN_00455950` | Sets pending motion; sends `0x36A/20` | Invalid state sends nothing |
| Click sitting/standing | Local human; permitted state | `FUN_004625DA` | Sets pending `25` or `27`; sends `0x36A/20` | Incompatible state sends nothing |
| Own-ID response | Valid frame and local human | `FUN_0052EAA9 -> FUN_005296E8` | Clears pending; applies motion | Motion >= 256 does not animate |
| Observer response | Remote human present | `FUN_005296E8` | Updates remote animation/effect | Missing human receives no dispatch |
| Authoritative effect | Server-originated `100` or special Parm | `FUN_005296E8` | Applies effect/motion | Null effect allocation is not attached |

The two vtable slots above establish input and receive ownership. Senders
own their stack buffers; `FUN_0055F2DD` does not retain them. The transport
owns the receive buffer, borrowed only during the human callback. Created
effects transfer to the scene container on successful allocation. A failed
send creates no new pending motion; an invalid frame never reaches the human.
FieldScene teardown removes humans and effects, so no packet pointer survives
the callback. Shutdown has no separate thread/socket owner for this flow.
Logout destroys FieldScene and its humans; relogin initializes pending motion
to none rather than restoring it from the previous session.

## Wire, ABI, and resources

Bidirectional opcode `0x36A`, exactly 20 bytes, with Win32 natural alignment
and no trailing padding:

| Field | Offset | Width/type | Native evidence |
| --- | ---: | --- | --- |
| `MSG_STANDARD` | `+0` | 12 bytes | 7.48 header |
| `Motion` | `+12` | signed int16 | `MOVSX [packet+0x0C]` |
| `Parm` | `+14` | signed int16 | `MOVSX [packet+0x0E]` |
| `Direction` | `+16` | float32/DWORD | read `[packet+0x10]` |

`FUN_0055890A` checks size `0x14`. No asset or UI ID is embedded in the
frame.

## Current mapping

The source client's `MSG_Motion` in `Basedef.h` has the same fields and size.
`TMFieldScene` zeroes the frame, sends zero Parm/Direction, and marks
`m_SendeMotion`. `TMHuman::OnPacketFireWork` clears that marker only when the
returned frame carries the local ID.

The server's `onMotion` accepts only a 20-byte frame from an in-world living
character, `Parm=0`, and motions `13`, `15..25`, or `27`. It ignores the
claimed client ID and Direction, rebuilds `wire.Motion` with the authoritative
player ID and zero Direction, and sends it to the owner and visible observers.
The earlier record incorrectly said the server discarded every request;
that mapping is no longer current. The zero-HP guard rejects a forged or
stale emote after death without affecting the wire format.

## Delta matrix

| Claim | Native 7.48 | Active client | Server | Decision |
| --- | --- | --- | --- | --- |
| Wire | 20 bytes and offsets above | Equivalent `MSG_Motion` | Builder and gate match | Preserve ABI |
| Emote C->S | Parm/Direction zero; motions above; alive only | Equivalent senders | Whitelist, dead guard, authoritative echo | Implemented; runtime unverified |
| Response ID | Own ID clears pending | Equivalent callback | Uses session player ID | Ignore client-claimed ID |
| Special effects | `100` and Parm `1..3` are S->C | Equivalent callback | Rejects as C->S intent | Keep server-owned |
| Lifecycle | Return frees next emote | Pending requires return | Owner included in fan-out | Runtime unverified |

## Decisions

- Classification: `PARIDADE_NATIVA/CONTRACT` for the native wire/roundtrip;
  rejecting forged dead-player intent is contract-preserving server hardening.
- Keep the 20-byte ABI and the existing client senders; do not accept
  client-authored visual effects or an untrusted entity ID.
- Include the owner in the visibility fan-out so its pending state clears.

## Gaps

- The actual roundtrip, ten keys, sit/stand, two-client observation, scene
  switch, and relogin have not been exercised in `project.exe`. No visual
  client test will be attempted while the Windows display is unavailable.
- Nonzero Direction is absent from the studied native senders and remains
  outside this emote contract.

## Validation

- Native research: read-only headless Ghidra export finished without a
  `SCRIPT ERROR`, matching the recorded program hash and exact vtable slots.
- Earlier integration: `go test -count=1 ./...` and Debug/Release
  `Build-Client.ps1` passed with 1,925 checks at that time. The then-installed
  Release SHA-256 was
  `DB0BEE35327ED0E6DEBD987BB3F515554D4EF5E7FFF01D212593E5AA1D68DB3E`;
  this is historical evidence, not a claim about the current candidate.
- Current focused server checks: `go test ./internal/game -run 'TestMotion' -count=1 -v`
  passed owner/observer echo, reserved-effect rejection, dead-player rejection,
  and restored emotes after revival. Character lifecycle consumers passed a
  separate focused game-package run. The current client was not executed;
  this flow is not `CLIENT_TESTED`.
