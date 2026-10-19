---
id: environment-effect-contract
title: Rune Track floor environment effect publication
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-18
---

# Environment effect publication

## Question

Can the Rune Track maze publish its historical floor pulse using an existing
7.48 packet, without introducing a client extension?

## Evidence boundary

- USED: native 7.48 Ghidra corpus, dispatcher `FUN_00492E7D` and consumer
  `FUN_00489618`, under `tmproject/build/native-research/decompiled/`.
  Both exports identify the binary hash above.
- USED: current `TMFieldSceneWorld.cpp::OnPacketEnvEffect` and
  `ReceivedPacketDispatch.h` minimum size for `0x3A2`.
- USED: Go environment-effect builder and Rune Track floor publisher.
- USED, secondary: 7.59 `TMSRVIN.c::sub_502BC0` constructs a 24-byte packet
  with scene ID 30000, rectangle coordinates and effect 32.
- NOT APPLICABLE: new assets, UI input and persistent client state. The
  existing explosion resource and scene effect container are reused.
- NOT APPLICABLE: the historical server does not establish 7.48 gameplay
  parity. Only this independently traced packet boundary is native parity.

## Native 7.48 flow

### Callers

`FUN_00492E7D` dispatches type `0x3A2` to `FUN_00489618`.

### Callees

The consumer rejects inverted rectangles, loops Y in steps of 3 and X from
`x1 + 1` in steps of 4, with exclusive upper bounds. Effect 32 creates an
explosion through `FUN_004E2360`, using size 1.5, duration 210 and color
`0x44444444`, and attaches it through `FUN_0054AC09` to the scene container.
Ground height comes from `FUN_0049C70F`.

## State and lifecycle

The packet is borrowed during dispatch; the created effects belong to the
scene container. They expire after their effect duration and scene teardown
removes them. No replay or relogin state exists. Server damage remains
authoritative and is independent of rendering. Publication is limited to
nearby members of the exact event runtime; public-space observers receive none.

## Wire, ABI, and resources

Server to client, `0x3A2`, 24 bytes, 12-byte header, scene ID 30000.
Six little-endian 16-bit fields start at offset 12: x1, y1, x2, y2, effect,
reserved zero. Native reads signed shorts; all authored coordinates are
positive and below 32768. No resource IDs or client files change.

## Current mapping

`internal/wire/environment_effect.go` builds the fixed envelope.
`rune_track_terrain.go` publishes effect 32 for each unprotected maze rectangle.
Existing C++ dispatch and `OnPacketEnvEffect` already consume that layout.

## Delta matrix

| Claim | Classification | Evidence |
| --- | --- | --- |
| Environment-effect envelope and effect 32 | PARIDADE_NATIVA | Native dispatcher and consumer above |
| Historical 7.59 encounter orchestration | MODERNIZACAO_COMPATIVEL | See Rune Track product document; not claimed as native 7.48 gameplay |

## Decisions

Keep the existing packet and consumer. Do not add an opcode or client state.
Keep event membership, damage, leader protection and pulse timing on the server.

## Gaps

Rendering in an actually running client, owner/observer playback and scene
teardown remain pending. No CLIENT_TESTED claim is made.

## Validation

The Go test uses independent literal offsets and payload bytes derived from
the native consumer; gameplay tests cover protected tiles and runtime isolation.
Execution results are recorded in `DOCS/server/rune-track.md`.
