---
id: airmove-contract
title: Native WYD 7.48 AirMove transport and flight completion
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-25
---

# Native WYD 7.48 AirMove transport and flight completion

## Question

Which intention does the client send when starting and ending AirMove, who
owns its destination, and why can the imported Griphon remain in flight?

## Evidence boundary

- Read-only canonical binary: `references/client748/WYD.exe`, SHA-256 above.
  Saved native/Ghidra artifacts: `b0f942ba0403483066c168b3dbea8854e121d258`
  (frame), `c155d2500f64052fb2d65bb1dc8786bee40302a7` (start), and
  `ddab79441b3475059f820a0115a7f0ddf6eb6793` (end).
- Native roots: `FUN_0047EF1D` (frame), `FUN_0047F7DA` (start), and
  `FUN_0047FA1D` (end). TMProject 7.69 is a secondary reference.
- Current client: `TMFieldSceneMovement.cpp`, `TMFieldScene.cpp`,
  `TMFieldScene.h`, `TMHuman.cpp`, `TMHumanMovement.cpp`, `Basedef.h`,
  `AirMoveContract.h`, `AirMoveMotion.h`, and `AirMoveRoutes.h`.
- Current server: `wydgo748/internal/game/airmove.go`, its tests, and
  `official_npc_dataset_test.go`. Imported NPC: `data/npcs/Griphon.json`.
- The 2026-10-25 user screenshot demonstrates an active flight that does not
  finish. It does not identify the running executable's path or hash.
  Before this correction, both the local incremental build and installed
  `tmproject/client748/WYD.exe` hashed
  `AB06F70B1CDAF6E52B7DA3A6F1156B56B277AD44C0B01669074DED92BE05FD1F`.
  This differs from the published 7.48.004 client hash
  `61C34613ABA2069229D2DFE31350953EF6F7DE4140F70BEE5E8CA5B439D9FCC0`
  recorded in `.agents/research/channel2-release-748004.json`.
  Process inventory returned no executable paths; neither identity can be
  assigned to the screenshot from this inspection.

## Native 7.48 flow

### Observable entry

A visible NPC with ID >= 1000, face/head 63 and Merchant low nibble 7 in
chunk (16,16) opens AirMove selection. Choosing one of five routes starts
ascent, waypoint traversal, descent, and completion. The intention contains
neither an NPC ID nor arbitrary client-selected coordinates.

### Callers

The selection UI calls `FUN_0047F7DA`. The field frame `FUN_004776C3`
invokes `FUN_0047EF1D` after the quit/logout countdown. The flight frame
calls `FUN_0047FA1D` on completion. The human action handler can also end
visual flight when receiving an authoritative server action.

### Main transition

Start sends C->S opcode 0xAD9, 20 bytes, local character Header.ID, route
0..4 in Parm1, and mode 1 in Parm2. The visual state machine surrounds
waypoint traversal with 2800 ms ascent and descent states. End sends the
same envelope and route with mode 2.

### Callees

Packet transport consumes the temporary stack structure. The scene owns
mount, height, particle, and visual position changes. The server validates
intentions and publishes the authoritative destination. Invalid route
indices must not read the table; duplicate completion must not send again.
Forged starts, replay and premature completion do not grant a destination.

## State and lifecycle

| Event | Preconditions | Result | Wire / failure |
| --- | --- | --- | --- |
| Open UI | Compatible visible NPC in chunk 16 | Route selection | No packet; missing dependencies keep it hidden |
| Start | Valid route and living local human | Active flight, state 2 | 0xAD9/20, mode 1 |
| Frame | Active flight | Ascent, waypoints, descent | Visual movement only |
| Complete | Active matching flight | Inactive flight | 0xAD9/20, mode 2 |
| Death | Pending flight | Restore visual origin; clear server route | No flight destination |
| External teleport | Authoritative position received | Discard pending visual delta | No second completion packet |
| Logout | Session active | Clear route, time and origin | Quit/logout remain allowed by the gameplay gate |

### Ownership, partial failure and cleanup

No vtable or S->C receiver is added. Scene state owns the visual route;
server session state stores route, source coordinates and start time.
Start returns before effects or packets when required owners are missing.
A server teleport failure clears the gameplay gate without granting a
new destination. Logout/relogin cannot retain or replay a previous route.

The 7.48 compact UI binds panel 12544, title 12549 and columns 12550..12552,
observed in the real FieldScene2.bin dump (`compat-field-before-hide`). The
imported layout used unavailable controls 12553/12560. Missing lists,
text controls or route labels keep the panel hidden and clear its flag.
Five labels are limited to 18 bytes and are never used as printf formats.

`UpdateMount` installs temporary mesh 40 and clears the mount look. End
restores both the saved mount mesh and look before rebuilding it, without
writing the mount type into the character's body mesh. Death and external
teleport take their cancellation branches before emitting a completion.
Waypoint advancement checks array bounds and the (0,0) sentinel before
incrementing the index.

### 2026-10-25 looping diagnosis

`AirMove_Main` queues a per-frame displacement in `m_vecAirMove`.
`TMHuman::FrameMove` formerly assigned the ground route interpolation to
`m_vecPosition` unconditionally, then consumed and cleared that displacement.
The server intentionally leaves its ground position at departure until
mode 2. Consequently every human frame replaced prior flight progress with
the ground origin: the next flight frame remained far from its waypoint
and could not reach descent or send completion.

`UpdateHumanFramePosition` now preserves the previous visual position while
`m_bIgnoreHeight` is active and consumes the pending displacement once,
before frame effects. Ordinary frames still use ground interpolation.
`SetPosition` receives the resulting position without applying the delta
again. `AirMove_End` can still consume a final pending delta when needed.
Only AirMove sets this height flag in the current source; authoritative
action handling and completion clear it.

`AirMoveWaypointDelta` extracts the existing direction, stop tolerance and
diagonal speed calculation without changing its formula. The unchanged
five waypoint arrays are shared in `AirMoveRoutes.h`, allowing regressions
to traverse the actual scene data. The existing stop check compares the
next full-speed step with the waypoint, so visual arrival can be up to
speed + 0.5 units away; the server publishes the exact endpoint.

## Wire, ABI, and resources

`MSG_STANDARDPARM2`: 12-byte header, DWORD route at +12, DWORD mode at +16,
20 bytes total, opcode 0xAD9, modes 1/2, routes 0..4. Visual paths remain
`g_pAirMoveRoute[5][10]`; only endpoints belong to server state.

| Route | Authoritative endpoint (X,Y) |
| --- | --- |
| 0 | 2112,2051 |
| 1 | 2372,2099 |
| 2 | 2220,1714 |
| 3 | 2365,2279 |
| 4 | 1826,1771 |

## Current mapping

The imported Griphon now exists with face 63, Merchant 7 and source spawn
(2115,2080), unlike the initial contract audit's dataset. The server checks
a compatible visible NPC and chunk 16, validates exact packet size,
retains the start context, and permits completion only after a matching
start and minimum duration. Other gameplay intentions are blocked during
flight; death, external teleport, logout and session reset clear the gate.

## Delta matrix

| Claim | Native evidence / existing behavior | Current adaptation |
| --- | --- | --- |
| Envelope and selection | Native 0xAD9/20, modes 1/2, five routes | Preserve existing ABI |
| Destination | Native route terminals | Server-owned endpoint table |
| Forgery and replay | Native transport does not establish trust | Go session validation and cleanup |
| Compact transport UI | Real 7.48 controls and label resources | Bind existing controls; guard partial resources |
| Visual progress | Scene traverses waypoints before end | Preserve frame-to-frame position instead of resetting to ground origin |
| Waypoint storage | Existing five arrays | Share unchanged data with portable regressions |

## Decisions

- PARIDADE_NATIVA applies to the proven envelope, route selection and data.
- EXTENSAO_COORDENADA applies to authoritative server validation. The
  5400 ms minimum allows 200 ms tolerance against two 2800 ms visual states;
  it is a project security rule, not an exact native flight duration.
- MODERNIZACAO_COMPATIVEL applies to partial-resource guards, bounded label
  copies, mount restoration, single delta consumption, waypoint bounds and
  preservation of flight position between frames. These fixes change no
  packet, route, price, destination, reward or resource identifier.
- Reuse the saved 7.48 contract; no new native behavior is inferred from the
  current C++ helper layout or internal field choices.

## Gaps

- Real client acceptance remains pending for each selected route, ascent,
  traversal, descent, restored mount and input, external interruption,
  owner/observer presentation and relogin.
- Automated route stepping does not execute Direct3D rendering, the complete
  scene state machine, UI selection, packet delivery or a live session.
- A new candidate does not update the running game or published launcher.
  This correction authorizes no installation, publication or restart.

## Validation

- Historical September contract checks covered Go route/state validation,
  C++ envelope/layout assertions, UI bindings and delta/mount/bounds guards.
  The earlier single-delta regression did not exercise ground interpolation
  overwriting position on a subsequent frame; it missed this failure.
- New C++ regressions run all five actual route tables at speeds 0.2, 0.6
  and 0.71, across repeated human frames with a stationary ground origin.
  They require final-waypoint arrival within the existing stop tolerance
  and an empty pending delta. Separate checks cover ordinary ground
  interpolation and a flight frame without a new displacement.
- Source integration checks tie both production callers to the tested
  position and waypoint helpers and prohibit the unconditional overwrite.
- 2026-10-25 focused Go AirMove tests passed. The official NPC dataset test
  also passed, exercising the imported Griphon against all five destinations.
- Release build with `Build-Client.ps1 -Configuration Release -NoDeploy`
  passed: 3,176,754 architecture checks, 281 socket checks, 200 crash-capture
  checks, and 58 launcher checks. Candidate identity is recorded below;
  the installed client and published patch were not replaced.
- Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, 1,939,456 bytes,
  SHA-256 `2C6288AA71E689FC02F9F5D7B5E6B67F3A087CAE8B3FB7A26EE539E9FDF632BD`.
- The changed-record validator and scoped `git diff --check` passed. The
  repository layout gate failed on existing artifacts outside build roots:
  the original installation's `WYD.log`, `tmproject/client748.zip`, and
  `tmproject/client748/dumps/` with runtime crash-monitor logs. This task did
  not create, remove or relocate those files. The original installation now
  resides in `references/client748/WYD 7.48 PORTUGUES/`; the earlier result is
  historical. Local cleanup later moved the client debug logs into `dumps/`
  and recycled `tmproject/client748.zip` after confirming it had no paths absent
  from the current client directory.
- Cleanup was rejected by execution policy before execution. The remaining
  task artifacts are `build/griphon-build.log` and
  `tmproject/build/tests/Release/crash-capture-tests-14236-96561343/`.
  No alternative deletion method was attempted.
- STATICALLY VERIFIED and AUTOMATED TESTED; not CLIENT_TESTED.
