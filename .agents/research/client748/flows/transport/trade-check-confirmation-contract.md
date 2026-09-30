---
id: trade-check-confirmation-contract
title: Native first trade-check acknowledgement
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-30
---

# Native first trade-check acknowledgement

## Question

Which native receiver handles `0x386`, what does it mutate, and how does the
active client/server pair confirm the first trade check without prematurely
transferring items or gold?

## Evidence boundary

- USED: immutable `references/client748/WYD.exe`, SHA-256 above, x86 image
  base `0x00400000`; recovered read-only Ghidra project `WYD748/WYD.exe`.
- USED: [focused instruction and table excerpts](../../exports/trade-check-confirmation.tsv).
  These are selected rows, not complete exports of every surrounding function.
  The complete 20-instruction acknowledgement consumer is included.
- USED: existing [trade/inventory lifecycle](../ui/trade-inventory-layout.md)
  and [control ownership](../ui/control-focus-ime-lifecycle.md) evidence.
- USED: active `TMHuman.cpp`, `SControl.h`, `ResourceControl.h`,
  `TradeCheckConfirmationContract.h`, and `ReceivedPacketDispatchTests.cpp`.
- USED: WYD-Go `internal/game/trade.go`,
  `internal/game/trade_check_contract_test.go`, and `internal/wire/trade.go`.
- NOT APPLICABLE: asset changes. Existing control bindings are reused; no
  new widget, resource ID, or materialized asset is introduced.
- NOT APPLICABLE: TMProject 7.69 as parity authority. Current source is the
  adaptation target, not evidence that an address belongs to native 7.48.
- LIMITED: the native size-policy body contains the `0x386/12` case, but
  its external invocation remains unresolved in the
  [packet-size gate record](packet-size-gate.md). This record does not
  establish a live native transport rejection path.

## Native 7.48 flow

### Observable entry

The acknowledgement is received for the human whose ID matches
`Header.ID`. In the active server, the first participant to check receives
this acknowledgement; the other participant receives the checked offer.
It is not an acknowledgement sent by the peer directly to the client.

Expected visible result: the local trade check control becomes selected.
The acknowledgement consumer does not transfer items or gold.

### Callers

Human packet receiver `FUN_0052EAA9` first rejects deletion-pending receivers
(`this+0x214`), null packets, and a packet ID that differs from the receiver's
ID at `this+0x20`. At `0x0052EE19` it compares the opcode with `0x386`;
the selected branch calls `FUN_0052E684` at `0x0052EE28`.

The human primary vtable `0x005A557C` binds packet reception at slot
`+0x04` (`0x005A5580 -> 0x0052EAA9`). Constructor `FUN_004F7EA6`
stores this vptr at `0x004F7F98`; destructor `FUN_004F8EBB` restores it
at `0x004F8EDF`. This establishes the concrete receiver binding, not the
entire socket-to-object traversal.

### Main function

`FUN_0052E684 @ 0x0052E684..0x0052E6C5`:

1. Reads the current scene from `DAT_0067CF38`.
2. Loads its control container at scene offset `+0x28`.
3. Requests control ID `0x269` (617, `TMB_TRADE_MYCHECK`).
4. Calls container vtable slot `+0x48`.
5. Writes integer `1` to the returned control at `+0x1E8`
   (`MOV dword ptr [EAX + 0x1e8],0x1` at `0x0052E6B1`).
6. Returns `1` with `RET 4`.

The write is **32 bits**, not a byte. The consumer neither reads a payload
nor retains the packet. It performs no allocation or economic mutation.

### Callees

The container primary vptr is `0x005A3F34`, stored by constructor
`FUN_0040C2CD` at `0x0040C32D`. Field scene constructor `FUN_00493E70`
calls that constructor at `0x004940B1` and publishes the result at scene
`+0x28` at `0x004940DE`.

Slot `+0x48` at `0x005A3F7C` points to `FUN_0040CDD7`, the control
lookup. It walks the tree rooted at container `+0x28`, ignores deleted
nodes, compares the requested ID with the virtual control-ID getter, and
returns a match or null. The standard control getter `FUN_0040BFF0`
reads the 32-bit ID at control `+0x44`; its binding at slot `+0x48`
is confirmed in the base control and panel tables
(`0x005A34F4`, `0x005A3E48`).

Container `+0x30` is a different panel field, not this lookup's tree root.
The same numerical virtual offset on the container and a control represents
different methods; the resolved tables must not be conflated.

### Outputs and errors

Native `FUN_0052E684` does not guard the scene, container, or lookup result.
The active handler adds these guards, including a Field-scene check, and
returns `1` without mutation when a prerequisite is missing. Native
lookup returning null is not evidence of a native safe dereference.

## State and lifecycle

### Transition matrix

| Event | Preconditions | Result | Economic side effects | Failure |
| --- | --- | --- | --- | --- |
| First valid check | Valid mutual server session and unchanged offer | Owner gets `0x386/12`; peer gets checked `0x383/156` | None | Invalid intent is rejected |
| Repeated first check | Peer has not checked | Same acknowledgement/offer; only owner stays checked | None | No premature commit |
| Uncheck | Existing offer/session | Peer sees unchecked offer; no local success acknowledgement | None | No persistence |
| Client receives valid acknowledgement | Matching human, Field/container/control present | Local `MyCheck` integer becomes 1 | None | Missing UI is a no-op in active source |
| Invalid client envelope | Null, truncated, oversized, inconsistent metadata/header | No callback delivery | None | Active receive gate rejects |
| Second participant checks | Both offers revalidated | Existing atomic commit path | Atomic transfer/persistence | Existing rollback/cancellation |

### Vtables, vptrs, and receivers

The human vtable identifies the receiver; the container vtable identifies
lookup. Both are tied to constructor writes rather than inferred from 7.69.
Control getter bindings establish the ID comparison used by lookup. No
callback or timer is registered by the acknowledgement.

### Ownership

The packet is borrowed during the call. The selected flag belongs to the
scene-owned control; the handler keeps no pointer after returning. The
server's trade state belongs to the authoritative world, not to that flag.

### Partial failure

The active receive gate rejects invalid frames before UI mutation. A missing
scene/container/control produces no mutation. Native lookup is nullable,
but the native consumer itself assumes a successfully materialized control.
Server rejection does not publish `0x386` or a checked peer offer and does
not persist or transfer items/gold.

### Cleanup and teardown

Native close-trade consumer `FUN_0052E2F6` uses the existing trade-close
path. Reused `FUN_0044B890(show=0)` evidence clears highlights, gold text,
temporary offers, and transient state and hides Trade/Inventory.
This acknowledgement introduces no additional cleanup resource.

### Shutdown

No acknowledgement-specific allocation, timer, or callback needs disposal.
Scene/control teardown remains the owner of the visual state.

### Logout and relogin

The scene-owned control cannot survive Field-scene teardown. Re-entry builds
new controls and bindings. Actual two-client cancellation, disconnect, and
logout/relogin observation remains pending; static ownership evidence is
not a runtime lifecycle test.

## Wire, ABI, and resources

Direction S->C, opcode `0x386`, total size 12 bytes, header-only, little-endian
Windows x86 representation. No payload or resource change.

| Field | Offset | Width | Interpretation |
| --- | ---: | ---: | --- |
| Header.Size | 0 | 2 | u16, 12 |
| Header.KeyWord | 2 | 1 | u8, transport |
| Header.CheckSum | 3 | 1 | u8, transport |
| Header.Type | 4 | 2 | u16, `0x386` |
| Header.ID | 6 | 2 | u16, acknowledged human |
| Header.Tick | 8 | 4 | u32, transport |

Native size-policy `FUN_0055890A` maps the `0x386` switch case to
`0x005593AF`, compares declared size with `0x0C` at `0x005593B7`,
and marks mismatch at `0x005593BC`. This proves the table entry, not
that the policy is invoked for live network frames. Its unresolved external
entry remains a separate gap.

`TradeCheckConfirmationContract.h` ties the active opcode to
`sizeof(MSG_STANDARD)`, with a 12-byte static assertion. Native object
offsets are evidence only; the active code accesses named members.

## Current mapping

### Recompilable client

`TMHuman::OnPacketEvent` selects the shared opcode.
`TMHuman::OnPacketCNFCheck` guards packet/scene/container/Field/control
and writes `SControl::m_bSelected`, an `int`.
The shared receive gate requires exact actual/declared length and matching
metadata/embedded opcode before delivery.

### WYD-Go

`wire.CNFTradeCheck(id)` builds `OpCNFTradeCheck` with size 12.
The first valid `onTrade` check acknowledges the owner and publishes the
authoritative checked offer to the peer. The second check follows the existing
revalidation and atomic commit path. A visual acknowledgement never grants
economic authority to the client.

## Delta matrix

| Claim | Native evidence | Active implementation | Classification |
| --- | --- | --- | --- |
| Local check feedback | Concrete human case, lookup and integer write | Selects the same control | `PARIDADE_NATIVA` |
| Header-only representation | Consumer reads no payload; size-policy entry is 12 | Shared 12-byte contract | `PARIDADE_NATIVA` representation only |
| Safe delivery/missing UI | Native consumer lacks nullable-UI guards; transport caller unresolved | Exact-size gate and optional-control guards | `MODERNIZACAO_COMPATIVEL` |
| Trade authority | Native client does not implement server persistence | Server validates and commits | No native backend parity claim |

## Decisions

- Promote knowledge of the local acknowledgement transition to `CONTRACT`.
- Preserve the current receiver, 12-byte envelope, and server-owned trade
  lifecycle; no functional implementation change is needed for this batch.
- Correct the old byte-write and peer-origin descriptions.
- Do not promote the independent native transport gate or runtime validation.

## Gaps

- Native external invocation of `FUN_0055890A` remains unresolved; retain
  the separate packet-size-gate record at `LOCATED`.
- Entire socket-to-human virtual traversal and concrete button construction
  are not newly mapped here; the bound human receiver, container lookup, and
  existing UI bindings are the scope of this contract.
- Execute two built clients through first/second check, cancel, disconnect,
  and logout/relogin before claiming `CLIENT_TESTED`.

## Validation

- STATICALLY VERIFIED: native receiver branch, human/container vptr writes,
  lookup target/body, standard control-ID getter, 32-bit selected write,
  and size-policy entry are recorded as reproducible instruction excerpts.
  Headless exports used read-only/no-analysis mode against the same native hash.
- AUTOMATED TESTED (reused, unchanged inputs): published commit `e92c5749`
  passed `TestTradeCheckContractPublicationAndRepetition`,
  all fifteen `TestTradeCheckContractRejections` scenarios, and affected
  atomic-commit, rollback, close-trade, and `TestTrade748Layout` tests.
  Responses were decrypted and checked for exact envelope, recipient,
  offer bytes, no extra success packets, and no first-check persistence.
- Earlier client validation (reused, not rerun): the unchanged receive-gate
  tests cover all truncated prefixes, excess, null storage, mismatched
  discriminants, and valid delivery. Commit `65ce1247` recorded 58,666
  architecture checks, 221 socket checks, and a Release x86 no-deploy build.
- Documentation/evidence validation: the research schema, repository
  layout/local-link validator, and `git diff --check` passed for this batch.
- CLIENT-TESTED: not performed. No candidate was installed or run.
