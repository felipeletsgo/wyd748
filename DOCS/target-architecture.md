# TMProject 7.48 target architecture

## Decision

This is an incremental proposal, not a claim that the migration is complete.
The current layout is documented in [architecture-map.md](architecture-map.md).
On the client, domain logic means presentation, prediction, and reconciliation;
local formulas do not replace validation or authoritative state in `wydgo748/`.

The project should evolve toward a layered architecture with inward-pointing
dependencies. Legacy compatibility belongs at the boundaries; game rules and
contracts must not depend on DirectX, Win32, or visual controls.

```text
platform -> adapters -> application -> domain
presentation (ui/scenes) -> application
wire <-> adapters        -> application
render <- presentation/adapters
```

`core` should not remain a catch-all. Reduce it incrementally: move domain
types to `domain`, message contracts to `wire`, and system services to
`platform` or `adapters`.

## Layers

### `domain`

Pure state and rules: entities, inventory, combat, formulas, positions, and
value objects. No `HWND`, DirectX, global singletons, UI logging, or direct
packet sends. Functions should receive explicit inputs and return a result or
error.

### `application`

Use cases and orchestration: login, scene changes, movement, trade, combat,
quests, and the field lifecycle. Defines ports for transport, time, resources,
audio, and rendering. `TMFieldScene` should become a use-case coordinator,
not the owner of game rules.

### `wire`

Messages, serialization, size/opcode validation, and translation into
application commands. Legacy layouts remain stable and require
`static_assert` and size tests before any change.

### `presentation`

`ui` and scenes display state and translate input into commands. Controls
should not mutate entities directly; callbacks invoke use cases and update the
view. Lifecycle order remains explicit.

### `adapters`

Concrete port implementations: current transport, tables/files, resources,
DirectX, and older APIs. This is the appropriate place to encapsulate globals
and `Basedef` functions during the transition.

### `platform`

Win32, windows, input, media, timing, and bootstrap. No game rules or protocol
decisions.

## Dependency rules

1. `domain` does not include headers from `platform`, `render`, `ui`, or `wire`.
2. `application` depends only on `domain` and its own interfaces.
3. `wire`, `render`, and `platform` implement or adapt interfaces; domain code
   does not import them.
4. Existing globals are accessed through one adapter, initially compatible
   with `BasedefGlobals.h`.
5. Each extraction preserves ABI, packing, opcodes, ownership, and teardown.

## Migration sequence

1. Freeze the inventory and add layout/opcode tests.
2. Extract ports for transport, time, resources, and rendering.
3. Move `TMFieldScene` message handling into application handlers, retaining a
   temporary facade.
4. Move `TMHuman` entities into `domain`, starting with pure functions.
5. Split `SGrid` into a data model and visual control.
6. Encapsulate `Basedef` globals in adapters and reduce the facade.
7. Remove forbidden cross-layer dependencies, one compiling step at a time.

Each step should be small, compilable, and reversible. A build proves
compilation, not completion of a migration or client flow.

## Incremental transport boundary

On receive, `NewApp` passes `PacketView` to
`ObjectManager::OnPacketView`, a nonvirtual entry point that validates the
envelope and adapts it to the legacy virtual callback. The CPSock buffer
remains borrowed, writable, and valid for the duration of the call; this
entry point does not accept originally const storage. Scene routing and its
`char*` callbacks are unchanged. The pure `Dispatch` policy preserves the
pointer, opcode, and size, and delivers the view once; it does not validate
payloads by opcode. Disconnects and local events without a frame still use
`OnPacketEvent`.

The `0xFAA` receive path passes through `wire/ReceivedPacketDispatch.h`
before legacy adaptation in ObjectManager. The helper uses the view's actual
length and copies only the header for aligned access. It requires 52 bytes,
`Size=52`, and agreement between the view opcode and header `Type`.
Rejection invokes no handler and changes no bytes. Other opcodes retain the
previous minimum gate. `CharacterTransferPacket.h` uniquely owns the old
struct, re-exported by Basedef, with size, offset, and signed-width asserts.
No virtual slot, routing order, or layout changed. This validation does not
migrate the `char*` callbacks or prove other packets safe; add the next
contract only with its own evidence and test.

`internal/application/ports/PacketView.h` defines the non-owning DTO for
framed messages. `CPSock::ReadPacketView` preserves the size validated by
the socket; `CPSock::SendPacket` validates the range before delegating to
the legacy method. `NewApp` uses size-aware reads, while account and
character login use size-aware sends. Other senders remain compatible and
should be migrated in contract-specific groups, avoiding mechanical edits
that could hide size or ownership differences.

The legacy utility path also exposes `SendPacket(const MutablePacketView&)`.
It retains the existing frequency limit in `SendOneMessage`, while letting
macros and older modules specify opcode, buffer, and size without depending
on the concrete socket API.

### Source-verified boundaries and ownership (2026-09-05)

Local source provenance; `MODERNIZACAO_COMPATIVEL`, with no new native-parity
claim. `PacketView::HasSizeBetween` centralizes pure pointer and inclusive
size checks. Send facades require `sizeof(MSG_STANDARD)` before reading or
writing the header and reject overflow before narrowing `size_t`. The view
opcode remains metadata; the legacy send path uses `MSG_STANDARD::Type`.
There is no change to layouts, opcodes, encryption, or the order of valid
sends.

Important: `CPSock::AddMessage` writes `Size`, `KeyWord`, `CheckSum`, and
`Tick` into the sender's buffer before copying encrypted output into its
internal queue. The send facades therefore require `MutablePacketView`:
its `char*` makes the writable-buffer requirement explicit. Its 140 senders
no longer cast away constness to send; receive paths still use `PacketView`.
`AsReadOnly` permits only the safe conversion from writable to read-only.
The view neither owns nor extends the buffer's lifetime and cannot prove
its allocated capacity. The `INT_MAX` bound prevents conversion overflow,
but does not replace queue-capacity limits or per-opcode semantic validation.

`TMProject748/tests/ArchitectureTests.vcxproj` compiles pure tests without
Win32 or DirectX. `Build-Client.ps1` runs them before compiling or installing
the client. Static checks and assertions cover bounds, nullability, overflow,
mutable typing, borrowed buffers, single delivery, visible mutation,
rejection without sending, and failure propagation without retry. They do
not cover real sockets, encryption, or an in-game flow. The legacy dispatch
in NewApp still adapts received views to `char*` callbacks; their signatures
were not migrated in this batch.

`ITransport` no longer depends on `Basedef` or MSG types.
`SocketTransport<CPSock>` has a live consumer in character-selection sends
through `RequestCharacterLogin`; it has local lifetime and neither owns nor
closes the socket. The use case validates intent, `CharacterLoginSender`
constructs the wire request, and the scene retains UI and lifecycle duties.
`MessageHeader.h` and `CharacterLoginPacket.h` uniquely own the extracted
struct definitions, re-exported by the former facades.

The recorded 98 checks include byte-for-byte tests of all four login slots,
invalid-index rejection, failure without retry, and adapter borrowing. The
tests still avoid Win32/DirectX headers, but the wire structs require the
Windows integer model, enforced by asserts. A separate compilation unit
exercises the use case with a fake semantic port and no encoder: it tests
all four slots, `INT_MIN/-1/4/INT_MAX`, failure propagation, and no retry.
This prevents the encoder's additional defense from masking a regression in
application validation.

For this extracted flow, application code depends only on its ports. The use
case calls `ICharacterLoginSender`, implemented by the wire encoder
`CharacterLoginSender`. The encoder depends on `ITransport`, implemented by
`SocketTransport` in the platform layer. The scene composes these local
adapters without changing ownership. The four consumers of the former
`wire/PacketView.h` facade now include the application port directly. The
redundant facade was removed; definitions and behavior remain in the owning
header. This inversion applies to the extracted flow, not all of phases 1
and 2.

### Legacy send-result debt

In current source, `CPSock::SendOneMessage` ignores the result of
`AddMessage` and returns `SendMessageA`. Consequently the port's intended
meaning (`true` = locally accepted) is not guaranteed by the real backend
when the queue rejects a packet. Tests with a fake backend demonstrate
propagation but do not fix this defect. As before, the scene ignores the
result; no flow changed here. A later batch should test queue rejection,
overflow, key consumption, and partial sends before fixing propagation.
`RefreshSendBuffer` also copies from the receive queue and needs focused
review; it was not changed in this extraction.
