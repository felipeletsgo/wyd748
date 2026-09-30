---
id: trade-session-envelope
title: Native trade offer and closure consumers
subsystem: transport
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-30
---

# Native trade offer and closure consumers

## Question

Which native human consumers receive incoming trade offers and closure, what
state do they own, and must local trade cleanup depend on an available UI
container? Which parts of the active bidirectional envelope still need native
emitter evidence?

## Evidence boundary

- USED: immutable `references/client748/WYD.exe`, SHA-256 above, x86 image
  base `0x00400000`, recovered read-only Ghidra project `WYD748/WYD.exe`.
- USED: [consumer instruction export](../../exports/trade-session-consumers.tsv):
  complete instruction rows for both consumers, selected receiver branches,
  and size-policy cases. Instruction-reference rows are omitted except the
  two size-switch targets; this is not a complete socket traversal export.
- USED: the existing [human receiver and container lookup proof](trade-check-confirmation-contract.md),
  [trade/inventory lifecycle](../ui/trade-inventory-layout.md), and
  [control ownership](../ui/control-focus-ime-lifecycle.md).
- USED: active `TMHuman.cpp`, `TradeSessionContract.h`, `Basedef.h`,
  `ReceivedPacketDispatchTests.cpp`, and `SceneDisconnectContractTests.cpp`.
- USED: WYD-Go `internal/wire/codec.go`, `session_packets_test.go`,
  `internal/game/trade.go`, and `trade_check_contract_test.go`.
- NOT APPLICABLE: asset changes or TMProject 7.69 as parity authority.
  Existing widgets and IDs are reused; candidate source is the adaptation
  target, not proof of native addresses.
- LIMITED: native outgoing constructors/emitters and the meaning of all
  outgoing position bytes are not established by these incoming consumers.
  The size-policy body's external invocation is still unresolved in
  [packet-size-gate.md](packet-size-gate.md).

## Native 7.48 flow

### Observable entry

The local human receives an invitation/offer (`0x383`) or a session closure
(`0x384`). Incoming offers project remote state into the Field-scene UI.
Closure clears local transient trade state, even before the scene-type check.
Neither consumer transfers authoritative inventory or gold.

### Callers

Human receiver `FUN_0052EAA9` rejects deletion-pending objects, null packets,
and a `Header.ID/+6` that differs from the receiver ID at `this+0x20`.
Its concrete primary vptr/packet slot is already resolved in the linked
acknowledgement record; it is reused here, not inferred from opcode literals.

- `0x0052EDB6`: compare opcode `0x383`; selected call at
  `0x0052EDC5` targets `FUN_0052DC5D`.
- `0x0052EDD8`: compare opcode `0x384`; selected call at
  `0x0052EDE7` targets `FUN_0052E2F6`.

This proves human dispatch, not the entire socket-to-human traversal.

### Main functions

`FUN_0052DC5D @ 0x0052DC5D..0x0052E2F5`:

1. Requires the receiver to equal current scene `+0x4C` and scene type
   `30000` (Field). Looks up control 601 and applies packet byte `+0x98`.
2. With no local opponent and a nonzero packet opponent at `+0x9A`, looks
   up that human, creates the invitation message, stores its ID in the dialog,
   releases the cursor item, copies 156 bytes into the local trade buffer,
   and clears its opponent field. Missing opponent lookup returns without
   opening the dialog.
3. Otherwise iterates fifteen grids `8192+i`, picks up each old item, compares
   eight bytes against packet `+12+i*8`, clears a matching cursor alias,
   destroys the old item, and creates/adds the new visual item.
4. Formats gold from packet `+0x94`. An item or gold change clears controls
   617/601, records the last-check time, and clears the local check byte.
5. Writes the gold text; if trade panel 576 is hidden and the other human
   exists, writes participant names and calls `FUN_0044B890(1)`.

`FUN_0052E2F6 @ 0x0052E2F6..0x0052E3C7`:

1. Requires the receiver to equal current scene `+0x4C`.
2. At `0x0052E316`, clears the opponent word at ObjectManager `+0xCFA`.
3. At `0x0052E324`, clears the local check byte at `+0xCF8`.
4. At `0x0052E32B`, writes `0xFFFF` to hover index `DAT_005B12BC`.
5. Only then calls scene-type getter `FUN_00494DCF`. For Field, looks up
   panels 576/646 and closes each only if non-null and visible.
6. Returns `1`; it reads no packet payload.

The ordering is explicit instruction evidence. Native closure does not guard
a null scene/container; the adapted client retains null safety without making
model cleanup conditional on UI availability.

### Callees

Container slot `+0x48` resolves to `FUN_0040CDD7`, proven by the linked
container vptr/table/getter record. The offer uses grid pickup `+0xA4`,
grid add `+0x8C`, item virtual destruction, and text/check virtual methods.
These call sites are recorded; their complete downstream bodies are outside
the envelope claim. `FUN_0040D13E` constructs the allocated visual item;
`FUN_0058F220` copies the item/offer bytes.

Closure calls `FUN_0040C0F0` for panel visibility,
`FUN_0044B890(0)` for Trade, and `FUN_0044AE38(0,0)` for AutoTrade.
Their panel composition/cleanup is reused from the trade/inventory record.

### Outputs and errors

Both native consumers return `1`. Offer lookup of a missing human is a
no-op; optional panel absence is tolerated by closure. Native offer control
lookups are not consistently null-guarded. The active receive gate rejects
null, truncated, excess, or mismatched frames before invoking consumers, and
the active handlers additionally guard scene/model/UI dependencies.

## State and lifecycle

### Transition matrix

| Event | Preconditions | State change | Visible effect | Failure |
| --- | --- | --- | --- | --- |
| Incoming invitation | Local human, Field, no local opponent, peer found | Native copies invitation; active client initializes an empty local offer | Invitation dialog | Missing peer returns |
| Incoming offer | Local human, Field, active negotiation | Remote item/gold snapshot replaces display; changed offer clears checks | Fifteen remote slots and names/gold | Active optional-control guards skip missing controls |
| Incoming closure | Local human and live scene/model | Opponent/check/hover cleared before UI checks | Visible Trade/AutoTrade closed when available | Missing UI does not retain local model state |
| Closure for another human | Receiver is not scene's local human | No local trade mutation | None | Return |
| Invalid envelope | Any | No callback or mutation | None | Receive gate rejects |

### Vtables, vptrs, and receivers

Reuse the resolved human primary vtable `0x005A557C`, packet slot
`0x005A5580 -> 0x0052EAA9`, constructor/destructor vptr writes, and
container `+0x48` binding from the acknowledgement record. Native container
and standard control have different methods at the same numerical slot.
No candidate vtable or native object offset is changed by this patch.

### Ownership

The frame is borrowed only during dispatch. Offer items are copied into
grid-owned visual objects, not retained as pointers into the packet. The
native replacement clears a matching cursor alias before item destruction.
The active invitation deliberately initializes a separate empty local offer
with all positions `-1`; it does not adopt the remote invitation as a reusable
outgoing offer. This pre-existing compatible hardening is not native parity.

### Partial failure

The active gate rejects the whole invalid envelope before any mutation.
A valid offer with missing UI dependencies is ignored safely. Closing with
a live scene/model but no container now clears local state and skips only
visual cleanup; repeated closure remains harmless. Null scene/model still
returns before dereference. The four new source-contract assertions check
ownership guards and cleanup order, not executable-client rendering.

### Cleanup and teardown

Native closure clears model state first and delegates visible panel/item
cleanup to the existing Field-scene routines. In the active client, absent
or already hidden panels do not prevent opponent/check/hover cleanup.
The scene remains owner of unavailable UI controls and their eventual teardown;
this patch does not allocate, replace, or manually delete scene controls.

### Shutdown

N/A: these consumers introduce no worker, subscription, or independent timer.
Existing scene/control destruction remains responsible for shutdown; this
patch only separates model cleanup from optional panel cleanup.

### Logout and relogin

The server cancels pending trades on disconnect/logout and closes the peer.
The linked UI ownership record covers scene teardown and recreated bindings.
Real two-client cancellation, disconnect, and relogin are still pending; no
executable-client observation is inferred from the source tests.

## Wire, ABI, and resources

Active version: WYD 7.48, little-endian Win32/x86. The paired client/server use
bidirectional `0x383/156` and header-only `0x384/12`.

| Field | Offset | Width | Active type/use | Native incoming evidence |
| --- | ---: | ---: | --- | --- |
| Header | 0 | 12 | Size/opcode/ID and transport | Human ID/opcode dispatch; size-policy entries |
| Items | 12 | 120 | Fifteen eight-byte items | Eight-byte comparison/copy and fifteen-slot loop |
| Carry positions | 132 | 15 | i8, empty `-1` | Copied in invitation; semantics not isolated here |
| Padding | 147 | 1 | x86 alignment | Not independently consumed |
| Gold | 148 | 4 | i32; server rejects negative/out-of-range gold | Four-byte read at `+0x94`; no native economic validation claim |
| Check | 152 | 1 | Active domain 0/1 | Byte read at `+0x98` |
| Padding | 153 | 1 | x86 alignment | Not independently consumed |
| Opponent | 154 | 2 | u16 | Word read and unsigned widening at `+0x9A` |

Native size-policy `FUN_0055890A` branches to `0x0055935F` for `0x383`
and `0x0055937B` for `0x384`; it compares the packet size with
`0x9C` and `0x0C` respectively. These are body/table facts, not proof of a
live transport rejection gate. Outgoing emitters, position interpretation,
and complete field signedness still need independent native tracing.

`TradeSessionContract.h` and `Basedef.h` protect the active ABI.
No resource, asset, opcode, payload, or server policy changes in this batch.

## Current mapping

### Buildable source

`TMHuman::OnPacketEvent` dispatches both shared opcodes; the exact-size gate
runs before casts. `OnPacketTrade` retains optional-control guards and the
separate local-offer initialization. `OnPacketQuitTrade` now clears model
state before checking the Field-scene container. Null scene/model and
local-human identity remain required.

### WYD-Go

`wire.Trade` and `wire.CloseTrade` are in `internal/wire/codec.go`.
`parseTradeRequest` checks the 156-byte intention, nonzero opponent,
nonnegative affordable gold, check domain, unique valid inventory positions,
and exact authoritative item bytes. The world validates participants,
range, gameplay space, invitation/session state, and item tradability.
Confirmation, persistence/rollback, and teardown remain server-owned.

## Delta matrix

| Claim | Native 7.48 | Previous active source | Current source/server | Decision |
| --- | --- | --- | --- | --- |
| Incoming dispatch and fields | Resolved consumers and key reads | Correct active envelopes, undocumented native roots | Same wire; reproducible evidence | Preserve; `TRACED`, not full bidirectional parity |
| Closure model order | Clears opponent/check/hover before UI | Container guard skipped model cleanup | Model first; optional UI later | `MODERNIZACAO_COMPATIVEL` |
| Null prerequisites | Native assumes scene/model/container | Guards present | Null scene/model still guarded | Preserve compatible safety |
| Invitation local buffer | Native copies remote frame | Active separate empty offer | Unchanged | Preserve prior hardening, not parity |
| Economic authority | Not re-established by these consumers | Server-owned | Unchanged | No coordinated extension |

## Decisions

- Correct the proven cleanup-order difference without changing the wire,
  economic authority, UI bindings, or other handlers.
- Promote the formerly `UNMAPPED` incoming flow to `TRACED` using concrete
  dispatch, consumer, receiver, and lifecycle evidence.
- Do not promote the whole bidirectional ABI to `CONTRACT` while outgoing
  native constructors/positions remain untraced.
- Retain active exact-size rejection and optional-control protections.

## Gaps

- Trace native outgoing offer/closure construction and position semantics
  before a full bidirectional native-contract claim.
- Resolve the size-policy external invocation only when needed; do not
  repeat exhausted static-caller searches with unchanged inputs.
- Execute invitation, acceptance, changed item/gold, both checks, closure,
  disconnect, and logout/relogin with two real clients when permitted.
- Complete downstream item/grid virtual-body parity is outside this trace.

## Validation

- STATICALLY VERIFIED: read-only/no-analysis Ghidra export for both consumer
  instruction bodies (388 and 58 instructions), reused dispatch branches and
  size-policy cases, and existing receiver/lookup/lifecycle bindings. The
  versioned rows are selected verbatim from the corresponding native exports.
- AUTOMATED TESTED: the new cleanup-order regression failed before the patch
  (two assertions); after correction, ArchitectureTests passed 58,670 checks
  and static assertions, including the unchanged offer/closure envelope tests.
  SocketReceiveTests passed 221 checks. Source-contract tests do not execute
  `TMHuman` with real controls.
- Build: incremental Release x86 `Build-Client.ps1 -NoDeploy` passed.
  Artifact `tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
  `8E308566035634E6AFFDA6ED39E9BF8F21E5982CD6B2DC25E4548A431942C016`.
  Existing legacy compiler warnings remain. Installed `project.exe` untouched.
- Server results reused: published `e92c5749` covers exact trade/close
  responses, repetition/revocation, fifteen rejection scenarios, first-check
  non-transfer, and atomic commit/rollback. No server code/input changed;
  those tests were not rerun for this client-only cleanup batch.
- Research schema and repository layout/links: passed; native export rows
  matched the reused source exports. Final diff check passed before publication.
- CLIENT-TESTED: not performed; no candidate installation or game execution.
