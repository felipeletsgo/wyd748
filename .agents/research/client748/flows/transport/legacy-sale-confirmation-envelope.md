---
id: legacy-sale-confirmation-envelope
title: Native sale confirmation reaches the field receiver with a 20-byte envelope
subsystem: transport
status: CONTRACT
native_sha256: 8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593
updated: 2026-10-01
---

# Native sale confirmation envelope

## Question

Does native 7.48 consume `0x37A` as a sale response, and does its packet-size
policy allow consistent frames longer than the current 20-byte representation?
This record closes the received envelope, not complete merchant UI or price parity.
It also records the unchanged request widths reused for server ingress hardening.
The arithmetic continuations close the response consumer's base-price bands
and the grid-type-3 quote calculation and exceptions. The type-38 continuation
also closes the native contract for the ability lookup required by that quote,
not every ability type. The candidate quote now uses that resolved lookup.
Complete merchant UI behavior and historical server policy remain open.

## Evidence boundary

- **USED:** `references/client748/WYD.exe`, SHA-256 above, x86 image base
  `0x00400000`. Historical evidence is read-only.
- **USED:** the recovered local Ghidra project at
  `tmproject/build/native-research/WYD748.gpr`, program `WYD.exe`, with the
  same SHA-256. Read-only, no-analysis inspection of `FUN_00492e7d`,
  `FUN_00487e23`, `FUN_0055890a`, table `0x005a4294`, and the quote/helper
  bodies `FUN_00418828` and `FUN_0054cd07`.
- **USED:** [focused instruction evidence](../../exports/legacy-sale-confirmation.tsv).
  Full decompilations and logs remain ignored build artifacts. Export runs
  contained the expected program hash, requested instruction/table summaries,
  and no `SCRIPT ERROR`; decompiler runs completed with `inspection_complete`.
- **USED:** the retained [request instruction export](../../exports/trade-session-input-routes.tsv),
  whose program identity matches the same native hash. `FUN_00416196` writes
  the request payload as words and sends 20 bytes; no new export is needed.
- **UNCHANGED:** runtime assets. The receive gate loads no resources; the
  quote uses the existing item catalog and message IDs 58/340 without edits.
- **USED:** buildable `LegacySalePacket.h`, `ReceivedPacketDispatch.h`, and
  `TMFieldScene::OnPacketSell`; tests in `SceneDisconnectContractTests.cpp`.
- **USED:** `SGridControl::MouseOver`, `NativeSaleQuote.h`, and quote fixtures
  in `GridInsertionTests.cpp`; the complete type-38 path is resolved below.
- **USED:** `FUN_0054e06c`, the refinement callee reached by ordinary ability
  lookup. Its matching 158-instruction export proves read-only item access,
  no further calls, and stack-only writes. Other ability types are not approved.
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

### Sale quote arithmetic

The matching native instruction export `sale-quote-boundary.tsv` resolves
the grid-type-3 quote in `FUN_00418828`, reached by a direct call at
`0x00420f88` in `FUN_004209fc`. The grid type comparison is at
`0x0041e36b`; the scene's borrowed text control is updated through virtual
slot `+0x80`. The candidate counterpart is `SGridControl::MouseOver` with
`GRID_SELL`. This does not establish every native grid class or input callback.

The quote repeats the ordinary quarter-price bands at
`0x0041e3ce..0x0041e42a`, then applies overrides in this exact order:

| Condition | Displayed price | Proof |
| --- | --- | --- |
| Ability type 38 returns 185 | Full catalog price | `PUSH 0x26`, call `FUN_0054cd07`, compare `0xb9`, catalog reload at `0x0041e46a` |
| Item index 412 | 800000, overriding the ability exception | Compare `0x19c` at `0x0041e482`, constant write at `0x0041e48a` |
| Item 412 or 413 with catalog price zero | Message 340 replaces the numeric quote | `0x0041e4d8..0x0041e522` |

Item 413 has no separate price-divided-by-eight branch. The numeric quote
uses message 58 at `0x0041e49b`. No passive or city-tax adjustment occurs
inside this sale-quote block; neighboring buy-quote tax reads are not sale proof.

### Type-38 ability used by the quote

The ability helper's second argument is masked to one byte. For the quote's
fixed type 38, none of the type remapping, requirement, speed, regeneration,
or guild branches apply. It sums matching catalog effects over twelve entries
(`0x0054cf53..0x0054cfb4`) and, for ordinary items, three instance effects
(`0x0054d222..0x0054d272`). Both catalog fields are signed words; both
instance fields are signed bytes. In particular, an instance value byte 185
contributes -71, not 185. A catalog value 185, or a sum equal to 185, still
triggers the full-price quote exception.

The whole type-38 path, including its exits, is now resolved:

| Item domain | Type-38 result | Proof |
| --- | --- | --- |
| Index at most 0 or above 6500 | Zero before catalog access | `0x0054cd17..0x0054cd2f` |
| 3200..3300 inclusive | Zero before effect reads | `0x0054cd34..0x0054cd49` |
| 2330..2389 | Catalog sum only; packed mount bytes are not instance effects | `0x0054cfbc..0x0054cfd3`, exit `0x0054d13d` |
| 3980..3999 | Catalog sum only; second mount domain also skips instance effects | `0x0054d14b..0x0054d162`, exit `0x0054d208` |
| Other valid indices | Catalog sum plus all three matching signed instance values | `0x0054d222..0x0054d272`, final load `0x0054d4af` |

Ordinary items call `FUN_0054e06c` at `0x0054d278` before deciding whether to
scale. That callee reads the item and writes only its own stack, with no calls
or external mutation in the full 158-instruction body. Its return value cannot
change type 38: `0x0054d374..0x0054d377` skips scaling, and all later modifiers
test other types before the final sum is returned. The quote-specific lookup
can therefore omit the refinement call without losing side effects. This does
not approve changing refinement computation for other consumers.

The sale quote itself already bounds catalog reads to `1..6499`. The
quote-specific lookup retains that bound rather than reproducing the native helper's index
6500 access. This invalid-index protection is `MODERNIZACAO_COMPATIVEL`;
the proven type-38 sums, exclusions, and mount paths are `PARIDADE_NATIVA`.
No item bytes, catalog data, price policy, or persisted effects are changed.

For valid catalog indices `1..6499` and nonnegative signed-int prices,
integer division by four reproduces the native `FILD`/`FMUL 0.25`/`__ftol`
without an intermediate float32 rounding. This is the cleared quote delta,
classified as `PARIDADE_NATIVA`; safe rejection of invalid catalog indices
is `MODERNIZACAO_COMPATIVEL`. Neither changes inventory, gold, packets,
merchant validation, persistence, or the authoritative sale policy.

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
The quote lookup synchronously borrows the catalog and item effect arrays as
const references, allocates nothing, and retains or modifies neither input.

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

The quote's local item ABI is eight bytes: signed index word at byte 0, then
three type/value byte pairs at offsets 2/3, 4/5, and 6/7. Native `MOVSX` reads
the effects with sign extension. The candidate's `STRUCT_ITEM` keeps unsigned
storage bytes; the quote must explicitly sign-extend each value, not change
the shared representation. Catalog entries have stride `0x8c`, with twelve
signed type/value word pairs at `+0x50` and price at `+0x80`. Existing item
size/offset assertions remain unchanged; the new lookup must accept precisely
the existing twelve/three effect array extents. No wire or asset changes apply.

## Current mapping

### Buildable source

`LegacySalePacket.h` holds the unchanged 20-byte representation. `0x37A`
is part of the fixed-size dispatch policy rather than the inherited
minimum-only exception. The envelope patch did not change
`TMFieldScene::OnPacketSell`.

`SGridControl::MouseOver` now delegates its sell quote to `NativeSaleQuote.h`.
The helper preserves the native ordinary bands, full-price ability-185
override, final item-412 price of 800000, ordinary item-413 calculation, and
zero-catalog message-340 condition. Valid nonnegative prices use integer
quarter division rather than intermediate float32 rounding. Catalog access
is guarded by the existing `1..6499` domain. Ability input now comes from
`native_sale_quote::GetVolatileAbility`, with signed instance-byte values,
the inclusive 3200..3300 exclusion, and catalog-only mount domains
2330..2389 and 3980..3999. `EF_VOLATILE == 38` is asserted at the production
call site; the template takes the actual twelve/three effect arrays without
changing their shared storage ABI. It omits refinement computation because
the native type-38 result is independent of that side-effect-free callee.
The general `BASE_GetItemAbility` and all of its other consumers are unchanged;
this focused adaptation does not establish parity for every ability type.

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
the complete authoritative shop policy. The native client quote and its
special-item overrides are now traced and adapted, without changing server
payment. No passive/tax adjustment occurs in that native quote block; the
authoritative policy still requires separate validation. The quote-specific
type-38 lookup is now adapted and automated-tested; general ability parity
and real client execution are not established by that result.
For the current catalog, item 412 quotes 800000 while Go's straight quarter
of 1000000 is 250000 before other adjustments. This mismatch is recorded,
not silently treated as economic parity. Existing low-price server tests do
not distinguish the response bands: their quarter prices remain below 5001.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Received `0x37A` consumer | Field receiver calls `FUN_00487e23` | Inherited sale handler exists | Secondary candidate | Does not emit response | CONFIRMED; retain guarded consumer |
| Exact envelope | Size-policy case requires 20 | Exact-size receive gate rejects larger frames | Struct is 20 bytes | Input intent is 20 bytes | PARIDADE_NATIVA: require exact receive size |
| Actual-size/opcode consistency | Native size/opcode words identified | Shared gate checks both discriminants and actual size | Internal guard | Unchanged | MODERNIZACAO_COMPATIVEL: reuse fail-closed gate |
| Complete source words | Request writes words at 14/16; response sign-extends them | Representation unchanged | No new sender or response | Reject high-byte aliases before dispatch | MODERNIZACAO_COMPATIVEL: enforce existing Carry policy on full fields |
| Response base-price bands | Quarter price, then two-thirds for 5001..10000 or half above 10000 | Same bands; floating precision still unproven | Candidate calculation is secondary | Omits these response bands | CONFIRMED response arithmetic; authoritative policy remains separate |
| Grid-type-3 sale quote | Ordinary bands; ability 185 full price; item 412 fixed at 800000; item 413 ordinary | Implemented in NativeSaleQuote.h with 154 focused arithmetic checks | Secondary candidate | Payment unchanged | PARIDADE_NATIVA for cleared calculation; invalid-index protection is MODERNIZACAO_COMPATIVEL |
| Quote type-38 ability | Signed catalog words and instance bytes; special exclusion and catalog-only mount paths; no refinement scaling | GetVolatileAbility integrated into MouseOver with 2134 additional checks | General helper not changed or approved | Payment unchanged | PARIDADE_NATIVA for proven lookup; reject index 6500 as MODERNIZACAO_COMPATIVEL |
| Complete UI/price parity | Authoritative policy and refresh not fully validated | Quote corrected; runtime pending | Different architecture | Authoritative snapshots | No broader parity claim or server price change |

## Decisions

Use the native exact envelope in the shared receive gate; keep the established
server snapshot lifecycle and handler safety. Preserve identifiers and bytes.
Do not remove the consumer merely because WYD-Go currently uses a different
authoritative confirmation path.
Classify the server source-domain gate as `MODERNIZACAO_COMPATIVEL`, not
proof that every native sale source follows the server's Carry-only policy.
Adapt the cleared quote calculation and its fixed type-38 lookup; retain the
general ability helper, authoritative payment, and snapshot lifecycle. Quote
tests are not evidence of payment parity or validation of other ability types.

## Gaps

- Real DirectX client sale execution is not performed. This remains `CONTRACT`,
  not `CLIENT_TESTED`.
- Downstream native grid class identity and UI refresh parity remain open.
  The response bands and quote exceptions are proven; only the displayed
  quote changed in this continuation, not server payment.
- The quote-specific type-38 lookup is adapted and automated-tested. Real UI
  execution is still pending, and other ability types/consumers remain outside
  this evidence boundary. Do not use these fixtures to approve a global rewrite
  of `BASE_GetItemAbility`.
- Next economic gate: establish the authoritative ordinary/special-item,
  passive, and city-tax policy, including the item-412 quote/payment mismatch.
  Cover catalog boundaries `20000/20004/40000/40004` through persistence,
  rollback, repetition, and snapshot publication before adapting payment.
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
- Quote continuation (2026-10-01): accepted the matching read-only
  `sale-quote-boundary.tsv` export and successful log. Reproduce with
  `ExportWydFlow.java` using an absolute ignored output path and selectors
  `exact:00418828 instructions:00418828 exact:0054cd07 instructions:0054cd07`.
  Retained 41 quote/helper instructions in the existing versioned export.
- Quote automation: 154 added checks cover discontinuities, override order,
  item 413, invalid indices, zero-catalog messages, and large integer prices.
  `Build-Client.ps1 -Configuration Release -NoDeploy` passed with 58,834
  architecture checks, 221 socket checks, asset/shader gates, and the integrated
  incremental x86 build. Existing legacy warnings remain. Artifact
  `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `BF2B447836F5884B3D1F087D10BD683B3F2AA88EE47A5CA9986E8DBA8026A42F`.
  No server source changed; earlier server tests were not repeated. No runtime
  installation or DirectX execution occurred; this is not `CLIENT_TESTED`.
- Type-38 research continuation (2026-10-01): reused the matching complete
  `FUN_0054cd07` instruction export and inspected its full type-38 path.
  Accepted a read-only refinement-callee export with 158 instructions and
  the same native program hash. Reproduce with `ExportWydFlow.java`, an
  absolute ignored output path, and selectors
  `exact:0054e06c instructions:0054e06c`. Retained 42 additional focused
  instructions in the existing versioned export. This continuation changes
  evidence and the implementation contract only; no product change, new build,
  installation, or client execution is claimed.
- Type-38 adaptation (2026-10-01): 2134 added checks exercise all 256 instance
  value bytes in each slot, quote override consequences, all twelve catalog
  slots with signed-word extremes, accumulated values, every instance type byte,
  refinement independence, input preservation, invalid indices, and special
  domain endpoints. The production SGrid build verifies the template against
  the actual catalog/item array types and the type-38 ID assertion.
  `Build-Client.ps1 -Configuration Release -NoDeploy` passed with 60,968
  architecture checks, 221 socket checks, asset/shader gates, and the integrated
  incremental x86 build. Existing signed/unsigned warnings in unrelated SGrid
  comparisons remain. Artifact `tmproject/build/TMProject748/Release/WYD.exe`,
  SHA-256 `584ECC7D4ED2DDF0DB48A17EB6EB1CAC2DCB1882AE8167BD77710343D05BBF57`.
  No server input changed, so earlier server tests were not repeated. No new
  native export was needed for this adaptation. No installation or real client
  execution occurred; the record remains `CONTRACT`, not `CLIENT_TESTED`.
