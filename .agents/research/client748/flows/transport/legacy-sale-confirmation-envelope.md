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
It also records the unchanged request widths reused for server ingress hardening.
The arithmetic continuation closes the response consumer's base-price bands;
it does not establish the complete merchant quote or historical server policy.

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
- **USED:** the retained [request instruction export](../../exports/trade-session-input-routes.tsv),
  whose program identity matches the same native hash. `FUN_00416196` writes
  the request payload as words and sends 20 bytes; no new export is needed.
- **NOT APPLICABLE:** runtime assets; the changed receive gate does not load
  textures, UI resources, or item tables.
- **USED:** buildable `LegacySalePacket.h`, `ReceivedPacketDispatch.h`, and
  `TMFieldScene::OnPacketSell`; tests in `SceneDisconnectContractTests.cpp`.
- **USED:** `wydgo748/internal/game/handlers.go::onSellItem`, inbound size
  validation, `sale_ingress.go::validSaleInventorySource`, and merchant
  lifecycle/range/tax and network-ingress tests. The server does not emit the
  inherited sale response.
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

### Response price arithmetic

The retained instruction export also resolves the calculation omitted by the
decompiler around its floating-point conversion. Both equipment and Carry
branches load the detached visual's item payload at `+0x670`, sign-extend its
index, and use a catalog stride of `0x8c` with the price load based at
`0x00d449d0`. Indices outside `1..6499` retain a zero price. Equipment uses
`0x00487fc9..0x00487ffc`; Carry uses `0x004880e7..0x00488119`.

Both branches multiply the signed catalog price by `0.25f` (bits
`0x3e800000`) before `__ftol` at `0x00488005` / `0x00488122`. The native
instructions are `FILD` followed by `FMUL`, not a demonstrated intermediate
float32 rounding of the catalog integer. For ordinary nonnegative prices,
let `q` be the truncated quarter price. The response then credits:

| Quarter price `q` | Local credit | Equipment / Carry proof |
| --- | --- | --- |
| `0..5000` | `q` | Lower comparison and signed `JL` at `0x0048800d` / `0x0048812a` |
| `5001..10000` | Integer `2*q/3` | Double, denominator 3, `IDIV` at `0x00488022..0x0048802a` / `0x0048813f..0x00488147` |
| Above `10000` | Integer `q/2` | Signed halve sequence ending at `0x00488040` / `0x0048815d` |

For example, catalog prices `20000`, `20004`, `40000`, and `40004` produce
local credits `5000`, `3334`, `6666`, and `5000`. These discontinuities are
present in the binary; do not replace them with a smooth or monotonic formula.
This proves the response calculation only. It does not prove merchant tax,
stack quantities, passive bonuses, or a missing historical server emitter.

The separate outgoing branch of `FUN_00416196` zeroes a 20-byte buffer at
`EBP-0x2c`, writes opcode `0x37A` at `0x004162bf`, then writes merchant,
source type, and position as words at `0x004162d2`, `0x004162da`, and
`0x004162e2`. Relative to the buffer, these occupy bytes 12, 14, and 16.
`PUSH 0x14` at `0x004162e6` and the send call at `0x004162ec` independently
establish the request size. This does not establish that the native request
domain is carry-only; that restriction belongs to the current server policy.

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
| Invalid network sale source | Full type word is not Carry, or signed position is outside 0..62 | `World.validateInboundCommand` | Character, shop, and trade unchanged | Security violation counted; no save or response | Reject before movement advancement and sale dispatch |

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
The server's source-domain gate runs before `onSellItem` can cancel trade or
project the source into byte-sized coordinates. Valid sales keep their
existing persistence rollback and authoritative snapshot responses.

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
The outgoing native request uses the same 20-byte representation and payload
word offsets, independently proven by the retained builder instructions above.

## Current mapping

### Buildable source

`LegacySalePacket.h` holds the unchanged 20-byte representation. `0x37A`
is part of the fixed-size dispatch policy rather than the inherited
minimum-only exception. The envelope patch did not change
`TMFieldScene::OnPacketSell`.

### WYD-Go

`onSellItem` validates a 20-byte `C->S` intent, merchant interaction, carry
slot, restrictions, and price, persists the sale, then publishes `SendItem`
and `UpdateEtc` snapshots. Do not add `S->C 0x37A`: the inherited handler
would perform an additional local gold/item mutation outside this flow.
Repurchase remains intentionally removed; successful merchant sales are final.

Commit `64336041` adds full-word source validation at the shared network
ingress, after envelope and phase checks but before dispatch. Type must be
Carry (`1`), and the signed position must be within `0..62`. Previously,
`onSellItem` read only the low bytes at 14 and 16: `0x0101` could alias Carry
and `0x0100` could alias slot 0. Negative words could alias these same values.
Other invalid sources could cancel an existing trade before being rejected.
The new gate rejects all these requests without changing inventory, gold,
merchant context, trade, persistence, or outbound snapshots. It does not
refactor the legacy handler's direct-call parser; all network dispatch goes
through the validated ingress. Valid requests retain unchanged prices,
merchant checks, rollback, and `SendItem`/`UpdateEtc` publication.

The current `TMFieldScene::OnPacketSell` already has the proven quarter-price
bands. Its initial float32 cast is not proven identical to the native x87
sequence for every catalog value; negative/large catalog inputs and integer
overflow are not approved by this arithmetic continuation.
`onSellItem` currently computes `uint64(def.Price)/4`, then the merchant
passive and city tax, without the two response bands. This is a concrete
comparison gap, not proof that the response formula can be transplanted as
the complete authoritative shop policy. The actual client quote at
`SGridControl::MouseOver` has additional branches, so its native input flow,
passive/tax order, and supported item domain must be traced before changing
the server's economic result. Existing low-price tests do not distinguish
these formulas: their quarter prices remain below 5001.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Received `0x37A` consumer | Field receiver calls `FUN_00487e23` | Inherited sale handler exists | Secondary candidate | Does not emit response | CONFIRMED; retain guarded consumer |
| Exact envelope | Size-policy case requires 20 | Minimum-only gate accepts larger frames | Struct is 20 bytes | Input intent is 20 bytes | PARIDADE_NATIVA: require exact receive size |
| Actual-size/opcode consistency | Native size/opcode words identified | Shared gate checks both discriminants and actual size | Internal guard | Unchanged | MODERNIZACAO_COMPATIVEL: reuse fail-closed gate |
| Complete source words | Request writes words at 14/16; response sign-extends them | Representation unchanged | No new sender or response | Reject high-byte aliases before dispatch | MODERNIZACAO_COMPATIVEL: enforce existing Carry policy on full fields |
| Response base-price bands | Quarter price, then two-thirds for 5001..10000 or half above 10000 | Same bands; floating precision still unproven | Candidate calculation is secondary | Omits these response bands | CONFIRMED response arithmetic; trace the quote before economic adaptation |
| Complete UI/price parity | Quote, passive/tax order, and refresh not fully validated | Existing safety modernization | Different architecture | Authoritative snapshots | No broader parity claim or price change |

## Decisions

Use the native exact envelope in the shared receive gate; keep the established
server snapshot lifecycle and handler safety. Preserve identifiers and bytes.
Do not remove the consumer merely because WYD-Go currently uses a different
authoritative confirmation path.
Classify the server source-domain gate as `MODERNIZACAO_COMPATIVEL`, not
proof that every native sale source follows the server's Carry-only policy.

## Gaps

- Real DirectX client sale execution is not performed. This remains `CONTRACT`,
  not `CLIENT_TESTED`.
- Downstream native grid class identity, merchant quote including passive/tax
  order, and UI refresh parity remain open. The response's base-price bands
  are now proven, but no product price changed in this continuation.
- Next economic gate: trace the native sale quote against
  `SGridControl::MouseOver`, establish ordinary/special-item and passive/tax
  branches, and cover catalog boundaries `20000/20004/40000/40004` through
  authoritative persistence and snapshot publication before adapting prices.
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
- Initial envelope batch: focused `go test -count=1 -v ./internal/game -run` executed and passed
  `TestShopOpenBuyAndSellLifecycle`, `TestShopAndCargoRejectInvalidOperations`,
  `TestShopOperationsRevalidateRangeAndRejectEquipmentSale`,
  `TestCityTaxBuySellAndTOTOArithmetic`, and
  `TestCityTaxPersistenceFailureRollsBackPlayerAndTreasury`. This does not prove
  a live PostgreSQL or DirectX UI flow; no server source changed in that batch.
- Server ingress continuation (`64336041`): eight malformed-source scenarios
  failed before the gate and passed afterward through `World.handle`.
  `TestSaleIngressRejectsFullWidthSourceBeforeSideEffects` covers high-byte
  aliases, negative words, equipment, cargo, and reserved slot 63, including
  unchanged trade and borrowed bytes. `TestSaleIngressValidSlotsPersistOnceAndRejectReplay`
  covers slots 0/62, uninterpreted tail padding, and repeated requests;
  `TestSaleIngressPersistenceFailureRestoresState` covers save failure.
  `go test -count=1 ./...` and `go vet ./...` passed for that server batch.
  These automated results do not establish a live database or client UI flow.
  This documentation continuation reuses them; it changes no product input.
- Structure: `validate_research.py --repo .` passed; repository layout and
  local links passed with 152 indexed documents. `git diff --check` passed.
- Real client: not run; no candidate installation or visual validation.
- Arithmetic continuation: reused the matching ignored
  `sale-handler-boundary.tsv` instruction export and successful log, retaining
  only price-related rows in the focused versioned export. The raw instructions
  recover loads/multiplication that the cached decompilation omits before
  `__ftol`. No new census, Ghidra run, source test, build, or installation was
  needed; unchanged earlier product validation is not a new runtime result.
