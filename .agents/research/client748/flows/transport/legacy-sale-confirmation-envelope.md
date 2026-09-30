---
id: legacy-sale-confirmation-envelope
title: Native sale confirmation reaches the field receiver with a 20-byte envelope
subsystem: transport
status: CONTRACT
native_sha256: 8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593
updated: 2026-09-30
---

# Native sale confirmation envelope

## Question

Does native 7.48 consume `0x37A` as a sale response, and does its packet-size
policy allow consistent frames longer than the current 20-byte representation?
This record closes the received envelope, not complete merchant UI or price parity.

## Evidence boundary

- **USED:** `references/client748/WYD.exe`, SHA-256 above, x86 image base
  `0x00400000`. Historical evidence is read-only.
- **USED:** the recovered local Ghidra project at
  `tmproject/build/native-research/WYD748.gpr`, program `WYD.exe`, with the
  same SHA-256. Read-only, no-analysis inspection of `FUN_00492e7d`,
  `FUN_00487e23`, `FUN_0055890a`, and table `0x005a4294`.
- **USED:** [focused instruction evidence](../../exports/legacy-sale-confirmation.tsv).
  Full decompilations and logs remain ignored build artifacts. Export runs
  contained the expected program hash, requested instruction/table summaries,
  and no `SCRIPT ERROR`; decompiler runs completed with `inspection_complete`.
- **NOT APPLICABLE:** runtime assets; the changed receive gate does not load
  textures, UI resources, or item tables.
- **USED:** buildable `LegacySalePacket.h`, `ReceivedPacketDispatch.h`, and
  `TMFieldScene::OnPacketSell`; tests in `SceneDisconnectContractTests.cpp`.
- **USED:** `wydgo748/internal/game/handlers.go::onSellItem`, inbound size
  validation, and merchant lifecycle/range/tax tests. The server does not emit
  the inherited sale response.
- **NOT APPLICABLE:** external legacy clients and guides. TMProject 7.69 is
  only the candidate source's origin, not evidence for this envelope.
- **CONTRADICTORY:** the previous assumption that no native consumer was
  available. The recovered field receiver directly calls the sale handler.

## Native 7.48 flow

### Observable entry

An incoming field-scene packet with the unsigned opcode word at byte 4 equal
to `0x37A` reaches `FUN_00487e23` from `FUN_00492e7d`. The native size-policy
function separately requires the unsigned size word at byte 0 to equal `0x14`.
This is a received-response consumer, not merely a request builder or opcode name.

### Callers

`FUN_00492e7d` compares opcode `0x37A` at `0x00493226` and calls
`FUN_00487e23` at `0x00493235`. It is the packet-receiver entry in slot 1
(`+0x4`, address `0x005a4298`) of the table at `0x005a4294`.
That table has constructor/destructor data references at `0x00434407`
and `0x004358fb`, owned by `FUN_004343a4` and `FUN_004358da`.
The size-policy switch at `0x00558ad3` reaches case `0x0055927c` for `0x37A`.
This record does not assert that a new network call-chain trace was completed
for the size-policy function.

### Main function

`FUN_00487e23` is an x86 `__thiscall` receiver with one packet pointer
argument (`RET 4` at `0x00488264`). It matches the merchant identifier against
the scene's merchant controls, or accepts target zero for a specific character
mode/passive combination. Type zero selects equipment; type one selects carry.
The position is sign-extended before indexing or carry row/column division.
Successful branches detach the visual, clear the eight-byte model item,
credit locally calculated gold, clear a matching interaction alias, and destroy
the detached visual. A final scene refresh also runs after a merchant mismatch.
These observations identify the consumer; its full arithmetic and safety are
not the parity claim made by this record.

### Callees

The handler uses grid receiver vtable offsets `+0xa4` (equipment branch) and
`+0xa8` (carry branch), then the detached visual's slot 0 deleting callback
with argument 1. It calls `FUN_004431e4`, sound lookup `FUN_00429a6d(0x1f)`,
`FUN_0042ad2b`, the global manager receiver's `+0x98` slot, and
`FUN_00480a83` before returning 1. Exact grid/manager class identities and
complete refresh behavior are outside the envelope change and are not
inferred from vtable offsets alone.

### Outputs and errors

`FUN_0055890a` sets its rejection flag when the declared size differs from
20; the case does not accept a larger envelope. The candidate gate additionally
requires the actual buffer size to match, preventing truncated or oversized
buffers from reaching a lengthless callback. Invalid frames do not mutate
the borrowed bytes or call the receiver.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| Received sale frame | Both opcodes match; declared and actual lengths are 20 | Candidate receive gate | One borrowed callback | No copy or mutation of payload | Reject before callback otherwise |
| Native merchant match | Target and type select an existing sale branch | `FUN_00487e23` | Model item cleared; detached visual destroyed | Local gold and UI refresh | Merchant mismatch skips sale mutation |
| Native size mismatch | `0x37A`, size other than 20 | `FUN_0055890a`, case `0x0055927c` | Rejection flag set | No size-policy payload mutation | Returns invalid-size result |

### Vtables, vptrs, and receivers

The field receiver table and its exact packet slot are resolved above. The
sale receiver uses its current scene instance and borrows the packet pointer.
Downstream grid/manager slots are observed, not renamed as proven classes.

### Ownership

The receive gate owns no packet or scene. Storage remains transport-owned and
is borrowed synchronously once. Native detachment/deletion observations above
do not change the candidate's existing grid ownership or alias safety policy.

### Partial failure

Malformed envelope rejection happens before a handler cast or model/UI mutation.
Semantic rejection and persistence failure remain server-authoritative;
this change adds no local success state or packet emission.

### Cleanup and teardown

The gate allocates nothing and retains no packet or receiver after the call.
The existing handler remains responsible for detached visual cleanup.

### Shutdown

N/A: this stateless envelope check creates no resource to release at shutdown.
It does not change native scene teardown or manager ownership.

### Logout and relogin

N/A: no state survives the synchronous receive gate. Existing scene lifecycle
and authoritative character snapshots are unchanged.

## Wire, ABI, and resources

Direction: native received `S->C` confirmation. Opcode: `0x37A`.
Total envelope: exactly 20 bytes; no larger embedding is accepted by the
native size-policy case. Win32 x86 little-endian representation uses the
existing 12-byte header and default four-byte structure alignment.

| Offset | Width | Meaning / signedness | Evidence |
| --- | --- | --- | --- |
| 0 | 2 | Declared size, unsigned | `0x00559281` reads a word; compares 20 |
| 4 | 2 | Opcode, unsigned | `0x00493222` reads a word; compares `0x37A` |
| 12 | 2 | Merchant target, unsigned | Handler word reads with zero extension |
| 14 | 2 | Source type, signed | `MOVSX` at `0x00487ec3`; branches 0 and 1 |
| 16 | 2 | Source position, signed | `MOVSX` at `0x00487f92` |
| 18 | 2 | Tail padding | Included in total size; no meaning assigned |

Other header fields retain the existing header contract; they are not newly
interpreted here. The fixed 20-byte regression fixture, size/offset assertions,
and mismatch tests make the decision falsifiable. No assets or resource IDs change.
The native server emitter is not available; receipt is proven by the field
consumer, not by claiming a recovered historical server implementation.

## Current mapping

### Buildable source

`LegacySalePacket.h` holds the unchanged 20-byte representation. Add `0x37A`
to the existing fixed-size dispatch policy instead of retaining the inherited
minimum-only exception. `TMFieldScene::OnPacketSell` remains unchanged.

### WYD-Go

`onSellItem` validates a 20-byte `C->S` intent, merchant interaction, carry
slot, restrictions, and price, persists the sale, then publishes `SendItem`
and `UpdateEtc` snapshots. Do not add `S->C 0x37A`: the inherited handler
would perform an additional local gold/item mutation outside this flow.
Repurchase remains intentionally removed; successful merchant sales are final.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Received `0x37A` consumer | Field receiver calls `FUN_00487e23` | Inherited sale handler exists | Secondary candidate | Does not emit response | CONFIRMED; retain guarded consumer |
| Exact envelope | Size-policy case requires 20 | Minimum-only gate accepts larger frames | Struct is 20 bytes | Input intent is 20 bytes | PARIDADE_NATIVA: require exact receive size |
| Actual-size/opcode consistency | Native size/opcode words identified | Shared gate checks both discriminants and actual size | Internal guard | Unchanged | MODERNIZACAO_COMPATIVEL: reuse fail-closed gate |
| Complete UI/price parity | Not fully validated by this slice | Existing safety modernization | Different architecture | Authoritative snapshots | No broader parity claim or server change |

## Decisions

Use the native exact envelope in the shared receive gate; keep the established
server snapshot lifecycle and handler safety. Preserve identifiers and bytes.
Do not remove the consumer merely because WYD-Go currently uses a different
authoritative confirmation path.

## Gaps

- Real DirectX client sale execution is not performed. This remains `CONTRACT`,
  not `CLIENT_TESTED`.
- Full downstream native grid class identity, price arithmetic, and UI refresh
  parity are not established by this envelope record and are not adapted here.
- Other documented legacy gaps (`0xED7/0xED8`, dormant `0x2C4`, unavailable
  transfer `0xFAA`) remain separate work; this evidence does not close them.

## Validation

- Research: recovered consumer, exact receiver slot, signed payload reads,
  and exact-size case from the matching native Ghidra program.
- Reproduction: use Ghidra 12.1.2 headless with `-process WYD.exe -readOnly
  -noanalysis` against the recovered project above. `InspectNative.java`
  accepts an ignored output directory followed by `00492e7d` and `00487e23`.
  Run the repository `ExportWydFlow.java` separately with an ignored output
  TSV and selectors `instructions:00492e7d instructions:0055890a`, then
  `exact:00487e23 instructions:00487e23 table:005a4294:2 exact:005a4298
  exact:0055927c`. Use one script directory per headless invocation; compare
  program identity, completion summaries, and errors before accepting output.
- Regression: five failures before the gate patch (three oversized deliveries,
  callback count, and missing fixed-size policy); all pass afterward. An initial
  test compile warning about implicit narrowing was corrected before the red run.
- Automation: `Build-Client.ps1 -Configuration Release -NoDeploy` passed:
  58,666 architecture checks, 221 socket checks, asset/shader gates, and the
  integrated incremental x86 build. Existing warnings in unchanged legacy source
  remain. Artifact `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `56A0A7E1D2D7B1B36C6B9658A7D77D1110AFF64F643636435E2F09CF22F9AB57`.
- Server: focused `go test -count=1 -v ./internal/game -run` executed and passed
  `TestShopOpenBuyAndSellLifecycle`, `TestShopAndCargoRejectInvalidOperations`,
  `TestShopOperationsRevalidateRangeAndRejectEquipmentSale`,
  `TestCityTaxBuySellAndTOTOArithmetic`, and
  `TestCityTaxPersistenceFailureRollsBackPlayerAndTreasury`. This does not prove
  a live PostgreSQL or DirectX UI flow; no server source changed.
- Structure: `validate_research.py --repo .` passed; repository layout and
  local links passed with 152 indexed documents. `git diff --check` passed.
- Real client: not run; no candidate installation or visual validation.
