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
not every ability type. The candidate quote and the type-38 entry of
`BASE_GetItemAbility` now share that resolved lookup.
Complete merchant UI behavior and historical server policy remain open.
The grid/cursor continuation closes visual detachment and the sale callback's
cursor receiver, not complete score/appearance refresh or runtime UI parity.
The appearance continuation resolves the instruction-level wrapper order of
`FUN_00480a83`, its refinement-block layout, concrete human virtual targets,
the weapon-101 angle exception, the matched costume selector, and the
isolated base-costume look overrides for IDs 4153..4156.
The candidate now snapshots active refinement at this boundary,
while retaining its separate shadow-restoration cache. Complete callee
behavior is not thereby approved as rendering parity.

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
- **USED:** the existing [grid constructor/vtable evidence](../../exports/grid-item-mesh-scale-vtable-callers.tsv),
  [grid detachment bodies](../../exports/legacy-sale-grid-detachment.tsv), and
  [cursor constructor/detachment evidence](../../exports/legacy-sale-cursor-detachment.tsv).
  The exact slots and receiver construction, rather than matching virtual
  offsets in an unrelated table, establish the bindings below.
- **USED:** `SGrid.cpp::PickupItem` / `PickupAtItem`, `SControl.cpp::SCursor::DetachItem`,
  and the sale handler's matching-alias cleanup. This continuation changes no
  source, ownership policy, runtime resource, or authoritative snapshot flow.
- **USED:** [appearance wrapper instructions](../../exports/legacy-sale-appearance-refresh.tsv),
  the previously cached full `FUN_00480a83` decompilation, and
  `TMFieldScene::UpdateMyHuman`. The matching read-only export contains 129
  instructions and 54 references. New source-contract checks protect candidate
  operation ordering; they do not execute the renderer or resolve callee ABI.
- **USED:** [refinement and virtual-target excerpts](../../exports/legacy-sale-refinement-fields.tsv),
  containing 151 instructions and 53 references from the matching program.
  The eight refinement writes, eight initial grade writes, complete
  `FUN_00480c25`, exact human slots, and costume trampoline delimit the claims
  below. Full cached bodies and mount branches remain outside Git.
- **USED:** the human constructor/vptr binding already resolved in the
  [trade acknowledgement record](trade-check-confirmation-contract.md),
  `Structures.h::SANC_INFO`, `TMHuman::SetPacketMOBItem`, and the current
  initialization/shadow-restoration cache consumers. No server or asset
  contract is changed by the layout tests or active-snapshot correction.
- **USED:** [angle and costume-selector excerpts](../../exports/legacy-sale-appearance-angle.tsv).
  The complete angle setter, mesh setter, base angle dispatch, and costume
  selector are retained alongside the weapon-field initialization prefix:
  170 instructions and 31 references. `HumanAnglePolicy.h` is used by both
  real `TMHuman::SetAngle` mesh branches and the isolated pitch fixtures.
  Full weapon initialization and costume post-processing are not approved.
- **USED:** `AppearanceRefinementRefresh.h::Rebuild`, instantiated by the real
  field wrapper and by isolated state fixtures. Fixtures exercise the same
  snapshot/rebuild/restore implementation without claiming to execute
  `TMHuman::InitObject` or its renderer.
- **USED:** [base-costume look instructions](../../exports/legacy-sale-base-costume-look.tsv):
  309 retained instructions and 79 references from the same native program.
  These cover the catalog prefix, four look overrides, six-byte refinement
  and grade clearing, and packet-to-look field bindings. `BaseCostumeLook.h`
  shares the production mutation with isolated fixtures; source-order checks
  do not execute the DirectX renderer or approve the enclosing human ABI.
- **USED:** the retained [request instruction export](../../exports/trade-session-input-routes.tsv),
  whose program identity matches the same native hash. `FUN_00416196` writes
  the request payload as words and sends 20 bytes; no new export is needed.
- **UNCHANGED:** runtime assets. The receive gate loads no resources; the
  quote uses the existing item catalog and message IDs 58/340 without edits.
- **USED:** buildable `LegacySalePacket.h`, `ReceivedPacketDispatch.h`, and
  `TMFieldScene::OnPacketSell`; tests in `SceneDisconnectContractTests.cpp`.
- **USED:** `NativeSalePrice.h`, shared by `TMFieldScene::OnPacketSell` and
  the ordinary quote calculation; price/routing regressions retain their
  separate exception and lifecycle boundaries.
- **USED:** `SGridControl::MouseOver`, `NativeSaleQuote.h`, and quote fixtures
  in `GridInsertionTests.cpp`; the complete type-38 path is resolved below.
- **USED:** `Basedef.cpp::BASE_GetItemAbility` and `NativeItemVolatile.h`.
  The shared query also serves existing item initialization, grid movement,
  pickup, and item-use callers; no caller lifecycle or packet builder changes.
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
For nonnegative signed 32-bit catalog prices, integer division by four is
equivalent to the exact x87 quarter followed by truncation. The two-thirds
branch only doubles values up to 10000; the other branch only halves the
quarter. Neither calculation can overflow that price domain. An intermediate
float32 cast loses this equivalence: price `16777223` produces `2097153`
instead of `2097152`, and `INT_MAX` produces `268435456` instead of `268435455`.
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
test other types before the final sum is returned. The fixed type-38 lookup
can therefore omit the refinement call without losing side effects. This does
not approve changing refinement computation for other consumers.

The sale quote itself already bounds catalog reads to `1..6499`. The
shared lookup retains that bound rather than reproducing the native helper's index
6500 access. This invalid-index protection is `MODERNIZACAO_COMPATIVEL`;
the proven type-38 sums, exclusions, and mount paths are `PARIDADE_NATIVA`.
No item bytes, catalog data, price policy, or persisted effects are changed.

For valid catalog indices `1..6499` and nonnegative signed-int prices,
integer division by four reproduces the native `FILD`/`FMUL 0.25`/`__ftol`
without an intermediate float32 rounding. This is the cleared quote delta,
classified as `PARIDADE_NATIVA`; safe rejection of invalid catalog indices
is `MODERNIZACAO_COMPATIVEL`. Neither changes inventory, gold, packets,
merchant validation, persistence, or the authoritative sale policy.

### Grid detachment and ownership transfer

The retained constructor `FUN_0040df9e` writes vptr `0x005a4024` at
`0x0040e0a1`. Exact slot `0x005a40c8` (`+0xa4`) points to
`FUN_0040f3a3`; slot `0x005a40cc` (`+0xa8`) points to `FUN_0040f55d`.
These are the native counterparts of `SGridControl::PickupItem` and
`PickupAtItem`, respectively. This is a behavior mapping, not a recovered
original C++ class name or approval of every resource binding.

Both functions scan the current item list at receiver `+0x200`, with count
at `+0x1fc`. `FUN_0040f3a3` calls `FUN_0040dee8` to test whether the
requested cell is inside the visual's half-open footprint; `FUN_0040f55d`
calls `FUN_0040df5a` to require the visual's exact origin cell. The helpers
read item origin/extent fields at `+0x1d0..+0x1dc` and mutate nothing.

On a match, the grid functions clear the footprint's occupancy entries through
the buffer at `+0x1f0`, shift later list entries left, null the old tail,
decrement the count, and return the detached visual. They do not destroy it,
clear its item payload, alter gold, or send packets. A missing match returns
null without changing the grid. The sale handler, not these grid helpers,
owns the later deleting callback. The candidate already implements these
semantics. Its clipped occupancy access and pickup mesh-scale adjustment are
existing safety/presentation adaptations, not newly proven native behavior.

### Cursor release after sale

`FUN_00409cbc` writes vptr `0x005a3e00` at `0x00409d20` and publishes
that same receiver in `DAT_005ccec0` at `0x00409d52`. It initializes style
at `+0x1e4` and attached visual at `+0x1e8`. This is the cursor receiver,
not the object manager. Exact slot `0x005a3e98` (`+0x98`) points to
`FUN_0040a147`, whose complete 18-instruction body returns the old attachment,
clears `+0x1e8`, and changes style 2 to 0. It calls nothing and performs no
score, model, appearance, gold, or network update.

The sale handler first clears a matching cursor alias before destroying the
detached visual, then calls this slot at `0x0048824e` after the score/sound
work. That native final call clears even an unrelated cursor attachment.
The candidate deliberately calls `DetachItem` only for the sold visual,
before deletion, preserving unrelated interactions under the existing alias
safety policy. `SCursor::DetachItem` itself matches the native callback's
return/clear/style semantics. Do not mistake this resolved callback for an
unimplemented manager refresh or reintroduce unconditional alias clearing.

### Appearance refresh wrapper

The sale tail calls `FUN_00480a83` even after a merchant mismatch. The wrapper
borrows the human receiver at scene `+0x4c` and mob state from `DAT_013b71e8`.
It zeroes a 16-byte stack buffer, copies human bytes at `+0x1f2`, and calls
`FUN_00524ded` with mob state at `+0x6ec`. When the saved first byte is nonzero
and the equipment word at mob base `+0x748` is not 32, it restores the 16 bytes
to human `+0x1f2`. The block is the active refinement/grade state resolved
below, not a separate old-state cache.

The remaining sequence is:

1. Convert the signed human word at `+0x45a` to float and call `FUN_005277a7`.
2. Call `FUN_004faf13` with the equipment word at mob base `+0x748`.
3. Query `FUN_0054cd07` with the item at mob base `+0x778` and ability 21.
   On result 41, copy human words `+0x1ee/+0x1f0` to `+0x1ea/+0x1ec`
   and bytes `+0x1f9/+0x201` to `+0x1f8/+0x200`.
4. Call human virtual slot `+0x38` (`0x00480bcb`), then `FUN_0051bb41`
   with the item words at mob base `+0x778/+0x780`.
5. Call human virtual slot `+0x40` (`0x00480c0b`) with zero, the human
   value at `+0x34`, and zero; call `FUN_0052433d`, then `FUN_00480c25`.
   Return at `0x00480c24` without a stack argument pop.

The candidate wrapper has corresponding preservation, height/race, ability-41
mirroring, initialization, weapon/angle, affect, and refinement-update steps
in this order. Their semantic names do not prove each native callee's complete
behavior. Concrete human vtable targets are now resolved below.
Fifteen source-contract checks protect the boundary and fourteen ordered
steps, with snapshot/rebuild/restore now executed by a shared, tested helper.
The order comparison alone does not justify rewriting the remaining callees.
The sale handler's existing null-human guard remains a deliberate adaptation;
the native wrapper itself does not supply that guard.

### Refinement layout and concrete initialization targets

`FUN_00524ded` calls the already resolved `FUN_0054e06c` on eight-byte
equipment records beginning at mob `+0x5c`. Writes at `0x005250d0` through
`0x00525181` identify the first eight bytes of human `+0x1f2`. Initial grade
writes at `0x00525bcf` through `0x00525c9c`, in the shared continuation owned
by `FUN_00525395`, identify the following eight bytes:

| Equipment slot | Refinement byte | Grade byte | Candidate fields |
| --- | --- | --- | --- |
| 0..5 | human +0x1f2..+0x1f7 | human +0x1fa..+0x1ff | Sanc0..Sanc5 / Legend0..Legend5 |
| 6 (left weapon) | human +0x1f9 | human +0x201 | Sanc7 / Legend7 |
| 7 (right weapon) | human +0x1f8 | human +0x200 | Sanc6 / Legend6 |

Color writes start at human `+0x202`, outside the preserved 16-byte block.
This proves the isolated `SANC_INFO` layout, not the full candidate `TMHuman`
ABI or all grade/color calculations. Seventeen compile-time assertions guard
the actual candidate type's size and every member offset; a byte fixture
also checks its unsigned high-bit values.

The proven primary human vptr is `0x005a557c`: constructor `FUN_004f7ea6`
stores it at `0x004f7f98`. Exact slot `0x005a55b4` (`+0x38`) points to
`FUN_004faf92`, corresponding to initialization; `0x005a55bc` (`+0x40`)
points to `FUN_00500e15`, corresponding to angle initialization. These are
concrete receiver bindings, not inference from an unrelated table's offsets.

Initialization is not a read-only consumer of the refinement block.
`FUN_004faf92` clears the first six refinement and grade bytes for its
4151..4200 helmet branch and propagates refinement in other skin branches.
Its out-of-range branch enters `FUN_013c2680`: this uses the caller's EBP,
searches 135 eight-byte costume records, and jumps to `0x004fb25a` on a
match or `0x004fb22e` on failure. Similarly, `FUN_00524ded` jumps at
`0x0052538b` into `0x013d2000`, whose branches rejoin `0x00525395` or
`0x00525954`. These are inherited-stack continuations, not independent
no-argument C++ callbacks despite the decompiler's function boundaries.
The matched selector at `0x013c2000` is now resolved. It reconstructs the
costume index as `(human[+0x7ae] & 0x0fff) + 0x1000`, searches the same 135
eight-byte records at `0x013c2200`, and selects the female type only when
the row's skin byte is `0xff` and skeleton parity is odd. A match stores
`type | 0x4000` in the caller's local word; failure retains 1. The bytes
`E9 D9 92 13 FF` at `0x013c206c` jump to `0x004fb34a`; despite Ghidra's
call classification, this is an inherited-stack continuation, not a C++
callback. Existing candidate costume selection already implements the
resolved row/parity policy. Complete initialization and post-processing
remain open; no manifest or asset rewrite follows from this evidence.

The continuation `0x004fb34a..0x004fb6d2` resolves four base-costume look
overrides after catalog/type selection and before skin replacement:

| Full costume ID | Local type | Face mesh | Helmet/body meshes |
| --- | --- | --- | --- |
| 4153 | 2 | 29 | 30 |
| 4154 | 3 | 12 | 36 |
| 4155 | 4 | 36 | 36 |
| 4156 | 5 | 16 | 16 |

Face skin and coat/pants/gloves/boots skins become zero. The native block
writes coat skin twice, not helmet skin; retain helmet skin and both weapon
look pairs rather than inferring another mutation. Calls at `0x004fb6ba`
and `0x004fb6cd` clear only the first six refinement and grade bytes. Weapon
refinement/grades remain intact. `FUN_00524ded` binds equipment slots to the
eight mesh/skin pairs at human `+0x1d2..+0x1f0`; isolated `HUMAN_LOOKINFO`
size/offset assertions protect the candidate representation, not full ABI.

`BaseCostumeLook.h::ApplyLook` implements these four full-ID overrides after
`SetHumanCostume` and before the existing skin deletion in `InitObject`.
Unrelated and masked-alias IDs remain untouched. Existing class/skeleton
selection, weapon state, assets, renderer routing, and old refinement cache
are unchanged. This closes only the look/refinement mutation; class/skeleton
and subsequent skin construction still require independent evidence.

`FUN_00480c25` reads mob base `+0x768`, queries `FUN_0054e06c`, and writes
scene `+0x26e78`. Since mob state begins at `+0x6ec`, this is equipment
slot 4. The candidate `SetSanc` already reads `Equip[4]`; a source-contract
check protects that correspondence.

The earlier candidate `UpdateMyHuman` copied `m_stOldSancInfo` instead of
active `m_stSancInfo`. Current source inspection proves these blocks need not
be identical: `InitObject` can clear active costume refinement and clears the
whole active block for shadow mode, while packet initialization retains its
pre-render state in the cache. Shadow exit restores selected active fields
from that cache. It is therefore a live lifecycle dependency, not dead code.

The correction is limited to the proven native snapshot boundary.
`AppearanceRefinementRefresh.h::Rebuild` captures all sixteen active bytes,
calls `SetPacketMOBItem` once, and restores that snapshot only when its first
byte is nonzero and the resulting head index is not 32. It does not replace
or restore the old-state cache: that cache retains the fresh packet state
written by `SetPacketMOBItem`, preserving the shadow-exit consumer. A cleared
active block no longer makes a nonzero old cache bypass the native gate.
This closes the snapshot-selection mismatch without claiming full native
costume/shadow parity or changing their initialization and exit paths.

Executable fixtures use distinct active/cache/packet values, first bytes
0/1/128/255, head indices 0/31/32/33, all sixteen bytes, and repeated refresh.
They assert one rebuild per call and fresh cache retention even when active
state is restored. Initialization-cleared input is modeled explicitly; actual
post-initialization rendering remains a separate runtime gate.

### Concrete angle dispatch and weapon-101 exception

Exact primary human slot `0x005a55c4` (`+0x48`) binds `FUN_00500d50`.
Angle initialization reaches it through `FUN_0053e98a`, which dispatches
through the same human receiver's vptr, rather than bypassing to a mesh.
The setter first rejects delayed deletion, then retains the original pitch
in human `+0x34`. Ordinary unmounted mesh pitch is negated; ordinary mounted
mesh pitch is negated and increased by the float at `0x005a4290`, whose
bits `0x40c90fdb` encode a full turn. If human `+0x160` equals 101, both
branches instead pass pitch through unchanged. Mesh pointer guards remain,
and the mounted rider body receives zero local angles independently of the
mount mesh pointer. `FUN_004be1a3` simply stores the three mesh angles.

The `FUN_0051bb41` prefix closes field identity: ability 21 of the left
weapon is stored at human `+0x160`; the right weapon uses `+0x164`. Thus
101 is a left-weapon type exception, not a character-class exception.
`HumanAnglePolicy.h::MeshPitch` applies this isolated native policy in the
actual setter, preserving the existing deletion/null guards, logical pitch,
class-44 initialization, and movement targets. No wire, ownership, resource,
or enclosing ABI change is introduced.

Seventy-nine additional checks cover both production branch bindings and
guards, the exact full-turn bits, ordinary/101/neighbor weapon types, finite
pitch boundaries, and unchanged weapon-101 signed zero, infinities, and a
quiet-NaN payload. Source checks do not execute the renderer; complete
appearance parity and real-client execution remain separate gates.

### Callees

The handler uses grid receiver vtable offsets `+0xa4` (equipment branch) and
`+0xa8` (carry branch), then the detached visual's slot 0 deleting callback
with argument 1. It calls `FUN_004431e4`, sound lookup `FUN_00429a6d(0x1f)`,
`FUN_0042ad2b`, the cursor receiver's resolved `+0x98` detachment slot, and
`FUN_00480a83` before returning 1. Its wrapper order is resolved above, but
complete score/appearance callee behavior remains open. The cursor identity is proven
from its constructor/global write, not inferred from the offset alone.

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
| Grid detachment | A list visual matches footprint or origin | `FUN_0040f3a3` / `FUN_0040f55d` | Occupancy cleared; list compacted; ownership returned | No destruction, model, balance, or network mutation | No match returns null with no grid change |
| Cursor release | Borrowed cursor receiver is valid | `FUN_0040a147` | Attachment cleared; pickup style becomes hand | Old attachment returned, not destroyed | Other cursor styles remain unchanged |
| Native size mismatch | `0x37A`, size other than 20 | `FUN_0055890a`, case `0x0055927c` | Rejection flag set | No size-policy payload mutation | Returns invalid-size result |
| Invalid network sale source | Full type word is not Carry, or signed position is outside 0..62 | `World.validateInboundCommand` | Character, shop, and trade unchanged | Security violation counted; no save or response | Reject before movement advancement and sale dispatch |
| Local type-38 query | Valid item index and borrowed twelve/three effect arrays | `BASE_GetItemAbility` or sale quote -> `native_item_volatile::GetAbility` | Signed native sum, catalog-only mount result, or zero exclusion | No allocation, mutation, retained pointer, or packet | Invalid indices return zero; other ability types retain existing branches |

### Vtables, vptrs, and receivers

The field receiver table and its exact packet slot are resolved above. The
sale receiver uses its current scene instance and borrows the packet pointer.
The grid and cursor vptr/slot bindings are resolved above. In particular,
the object-manager table at `0x005a45f0` is not this callback's receiver;
adding `0x98` to an unrelated table base does not establish a binding.

### Ownership

The receive gate owns no packet or scene. Storage remains transport-owned and
is borrowed synchronously once. Native detachment/deletion observations above
do not change the candidate's existing grid ownership or alias safety policy.
Grid detachment transfers the visual to its caller; cursor detachment only
releases a borrowed interaction alias and never deletes the visual. The sale
handler clears matching aliases before deleting the transferred visual.
The shared type-38 lookup synchronously borrows the catalog and item effect arrays as
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
`NativeSaleQuote.h` delegates that query to the core `NativeItemVolatile.h`.
`BASE_GetItemAbility` returns the same query only when `Type == EF_VOLATILE`,
after its existing catalog bound and before the inherited ability branches.
Compile-time assertions retain type 38 and the 6500-entry catalog limit.
The existing callers in `TMItem::InitObject`, `SGridControl` movement,
`TMHuman` pickup, and `TMFieldScene` item-use/drop routes therefore receive
the same type-38 calculation. Catalog codes above 127, such as the existing
230 selector, stay signed-word catalog values; only instance bytes are
sign-extended. Mount instance fields remain packed and are not interpreted as
ordinary effects. The server catalog's 3200..3299 lottery entries have no
type-38 effects; the exclusion is justified by the native helper, not that
server data. No catalog entries are removed or translated.
Other ability types, static/no-refinement helpers, caller state machines,
and wire formats are unchanged. This closes the fixed helper result, not
native parity of every consumer or the entire general ability algorithm.

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

The current `TMFieldScene::OnPacketSell` routes its common equipment/Carry
price block through `native_sale_price::Calculate` in `NativeSalePrice.h`.
This removes the initial float32 rounding while retaining the existing catalog
index guard, model/visual cleanup, and credit ordering. The quote uses the same
ordinary arithmetic, then applies its own ability-185 and item-412 overrides;
the response must not inherit those quote-only exceptions. Focused fixtures
cover native discontinuities, float32 precision boundaries, and `INT_MAX`.
Signed-storage truncation checks do not approve a negative-price policy.
Final balance overflow, complete UI behavior, and server payment parity are
outside this price-only correction.
`onSellItem` currently computes `uint64(def.Price)/4`, then the merchant
passive and city tax, without the two response bands. This is a concrete
comparison gap, not proof that the response formula can be transplanted as
the complete authoritative shop policy. The native client quote and its
special-item overrides are now traced and adapted, without changing server
payment. No passive/tax adjustment occurs in that native quote block; the
authoritative policy still requires separate validation. The fixed type-38
lookup is now shared and automated-tested; other ability types, full consumer
parity, and real client execution are not established by that result.
For the current catalog, item 412 quotes 800000 while Go's straight quarter
of 1000000 is 250000 before other adjustments. This mismatch is recorded,
not silently treated as economic parity. Earlier low-price server tests did
not distinguish the response bands: their quarter prices remained below 5001.
The subsequently published current-payment tests distinguish those inputs,
but deliberately preserve the server policy until its separate decision.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Received `0x37A` consumer | Field receiver calls `FUN_00487e23` | Inherited sale handler exists | Secondary candidate | Does not emit response | CONFIRMED; retain guarded consumer |
| Exact envelope | Size-policy case requires 20 | Exact-size receive gate rejects larger frames | Struct is 20 bytes | Input intent is 20 bytes | PARIDADE_NATIVA: require exact receive size |
| Actual-size/opcode consistency | Native size/opcode words identified | Shared gate checks both discriminants and actual size | Internal guard | Unchanged | MODERNIZACAO_COMPATIVEL: reuse fail-closed gate |
| Complete source words | Request writes words at 14/16; response sign-extends them | Representation unchanged | No new sender or response | Reject high-byte aliases before dispatch | MODERNIZACAO_COMPATIVEL: enforce existing Carry policy on full fields |
| Response base-price bands | Exact quarter price, then two-thirds for 5001..10000 or half above 10000 | Shared NativeSalePrice.h removes float32 rounding; response does not apply quote-only exceptions | Candidate calculation is secondary | Omits these response bands | PARIDADE_NATIVA for nonnegative signed catalog arithmetic; authoritative policy remains separate |
| Grid-type-3 sale quote | Ordinary bands; ability 185 full price; item 412 fixed at 800000; item 413 ordinary | Implemented in NativeSaleQuote.h with 154 focused arithmetic checks | Secondary candidate | Payment unchanged | PARIDADE_NATIVA for cleared calculation; invalid-index protection is MODERNIZACAO_COMPATIVEL |
| Fixed type-38 ability | Signed catalog words and instance bytes; special exclusion and catalog-only mount paths; no refinement scaling | Core GetAbility shared by MouseOver and BASE_GetItemAbility; 2134 existing fixtures plus exclusive-routing regression | Other ability types and caller lifecycles unchanged | Payment unchanged | PARIDADE_NATIVA for proven lookup; reject index 6500 as MODERNIZACAO_COMPATIVEL |
| Grid visual detachment | Concrete vptr/slots resolve footprint/origin removal, occupancy/list mutation, and ownership transfer | Existing PickupItem/PickupAtItem match; clipping and mesh scaling remain adaptations | Names are secondary | Unchanged snapshots | CONFIRMED core transition; no source edit needed |
| Cursor callback | Constructed global cursor; +0x98 clears attachment and resets pickup style | DetachItem matches; handler limits cleanup to the sold visual before deletion | Names are secondary | No legacy response emitter | CONFIRMED callback; preserve existing MODERNIZACAO_COMPATIVEL alias protection |
| Appearance wrapper order | 16-byte preservation gate, ability-41 copies, direct/virtual call sequence resolved | Shared snapshot helper followed by unchanged ordered steps; fifteen source checks | Complete callee behavior and enclosing field ABI remain secondary | Unchanged snapshots | CONFIRMED wrapper order; no complete rendering-parity claim |
| Refinement layout and refresh targets | Eight refinement/grade bytes each; concrete +0x38/+0x40 targets; scene refinement reads Equip[4] | Actual SANC_INFO layout asserted and byte-tested; SetSanc source contract protected | Full initialization remains secondary | Unchanged | CONFIRMED isolated layout and target bindings |
| Active refinement snapshot | Copies active human +0x1f2 before packet rebuild; conditional sixteen-byte restore | Rebuild helper uses active state, not old cache; packet cache remains fresh | Previous cache substitution was not equivalent for cleared/normalized active state | Unchanged | PARIDADE_NATIVA for snapshot/rebuild/restore boundary; full costume/shadow rendering remains open |
| Weapon-101 mesh pitch | Concrete +0x48 dispatch; left-weapon ability 21 == 101 preserves pitch in both mount states | Shared production angle policy; 79 additional checks preserve guards and exact full-turn bits | Previous setter always reversed pitch | Unchanged | PARIDADE_NATIVA for isolated orientation; no complete rendering-parity claim |
| Matched costume selector | 135 rows; packed index reconstruction; female selection requires skin 0xff and odd skeleton parity; inherited-stack jump | Existing row/parity selection retained without asset edits | Full initialization remains secondary | Unchanged | CONFIRMED selector only; post-processing remains open |
| Base costumes 4153..4156 | Four face/body overrides; six refinement and grade bytes cleared; helmet skin and weapons retained | Shared production helper, byte fixtures, and initialization-order check | Catalog-only look propagation missed the native overrides | Unchanged | PARIDADE_NATIVA for isolated look/refinement mutation; full rendering remains open |
| Complete UI/price parity | Authoritative policy and refresh not fully validated | Quote corrected; runtime pending | Different architecture | Authoritative snapshots | No broader parity claim or server price change |

## Decisions

Use the native exact envelope in the shared receive gate; keep the established
server snapshot lifecycle and handler safety. Preserve identifiers and bytes.
Do not remove the consumer merely because WYD-Go currently uses a different
authoritative confirmation path.
Classify the server source-domain gate as `MODERNIZACAO_COMPATIVEL`, not
proof that every native sale source follows the server's Carry-only policy.
Adapt the cleared quote calculation and share its fixed type-38 lookup with
`BASE_GetItemAbility`; retain other ability types, authoritative payment, and
snapshot lifecycle. Quote tests are not evidence of payment parity or
validation of other ability types.
Share the proven ordinary price bands between quote and legacy response,
without sharing the quote's item/ability overrides. Keep the server's
snapshot-based confirmation; no new response emitter is authorized here.
Keep the existing grid/cursor implementation: the resolved native functions
do not justify another functional patch. Correct the earlier manager label
and retain the candidate's deliberate matching-only alias cleanup.
Keep the separate packet-state cache and its shadow-exit consumers. Correct
only the wrapper's snapshot source to active refinement, as native
`FUN_00480a83` explicitly does. The cache is not a substitute when
initialization clears or normalizes active state. Do not infer full costume
or shadow parity from this isolated correction.
Apply the proven weapon-101 exception in both mesh branches, retaining
logical angles and existing guards. Keep costume assets and the already
matching row/parity selection; do not infer complete initialization parity.
Apply only the four proven base-costume look/refinement overrides. Preserve
helmet skin, weapons, existing class/skeleton selection, and unrelated IDs.

## Gaps

- Real DirectX client sale execution is not performed. This remains `CONTRACT`,
  not `CLIENT_TESTED`.
- Grid detachment, cursor release, appearance wrapper order, isolated refinement
  layout, concrete human slots `+0x38/+0x40/+0x48`, the isolated angle setter,
  the matched costume selector, and base-costume look/refinement overrides
  for IDs 4153..4156 are resolved. Full score refresh through
  `FUN_004431e4`, other appearance callees, full costume/shadow post-initialization behavior,
  the enclosing human ABI, resource-to-scene bindings,
  and real UI behavior remain open. The candidate intentionally differs in
  unconditional cursor clearing and invalid/null-input handling. The response
  bands and quote exceptions do not establish server payment parity.
- The shared type-38 lookup is adapted and automated-tested. Real UI
  execution is still pending, and other ability types and full consumer
  lifecycles remain outside this evidence boundary. Do not use these fixtures to approve a global rewrite
  of `BASE_GetItemAbility`.
- Next economic gate: establish the authoritative ordinary/special-item,
  passive, and city-tax policy, including the item-412 quote/payment mismatch.
  Published current-policy characterization in `aa5e88fb` covers catalog
  boundaries `20000/20004/40000/40004`, items 412/413, and ability 185 through
  persistence, rollback/retry, repetition, caps, and decrypted snapshot
  publication. Its 98 scenarios preserve current payment; they are not
  approval of native payment parity. The requested policy choice is unanswered.
- Other documented legacy gaps (`0xED7/0xED8`, dormant `0x2C4`, unavailable
  transfer `0xFAA`) remain separate work; this evidence does not close them.

## Validation

- Base-costume correction (2026-10-01): reused matching cached post-processing
  and packet field bindings; a focused read-only Ghidra run exported
  `instructions:004fb22e` without `SCRIPT ERROR`. Reproduce the other cache
  with `instructions:004fb34a`; the committed excerpt retains 309 instructions
  and 79 references, not the complete initialization body. Release/Win32
  architecture validation passed 61,356 checks plus static assertions,
  including 89 new checks and seventeen look-layout assertions. The integrated
  `Build-Client.ps1 -Configuration Release -NoDeploy` gate passed 221 socket
  checks, costume/shader gates, and incremental product compilation. Existing
  signedness/deprecated Winsock warnings remain. This is `STATICALLY VERIFIED`
  native mutation and `AUTOMATED TESTED` isolated candidate logic, not
  `CLIENT_TESTED`. No client installation/execution, Go suite, asset change,
  or deletion ran. Class/skeleton selection and skin construction remain open.
- Angle/selector continuation (2026-10-01): matching read-only Ghidra runs
  completed without `SCRIPT ERROR`; cached evidence was reused for resolved
  inputs. The focused excerpt retains 170 instructions/31 references, with
  only the left/right weapon-field prefix rather than the complete weapon
  routine. Release/Win32 architecture validation passed 61,267 checks plus
  static assertions, including 79 new angle checks. The integrated
  `Build-Client.ps1 -Configuration Release -NoDeploy` gate passed 221 socket
  checks, costume/shader gates, and incremental product compilation.
  Existing signedness/deprecated Winsock warnings remain. This is
  `STATICALLY VERIFIED` native orientation and `AUTOMATED TESTED` isolated
  policy, not `CLIENT_TESTED`. Three existing Portuguese text remnants in
  the changed human source were translated, including a display-only tower
  score suffix; persisted identifiers and protocol bytes were preserved.
  No client installation/execution, Go suite, asset edit, or deletion ran.
- Active snapshot correction (2026-10-01): reused the unchanged native wrapper,
  layout/slot excerpts, and cached initialization evidence; no Ghidra sweep or
  new export was needed. Source inspection identified active-state mutations
  and live old-cache shadow-exit consumers. The shared production helper passed
  35 new state/source checks; replacing four old source-order steps with one
  helper call gives a net increase of 32 checks. Release/Win32 architecture
  validation passed 61,188 checks plus static assertions. The integrated
  `Build-Client.ps1 -Configuration Release -NoDeploy` gate passed, including
  221 socket checks, costume/shader validation, and incremental product build.
  Existing signedness/deprecated Winsock warnings remain; they did not fail
  compilation. This is `STATICALLY VERIFIED` native boundary adaptation and
  `AUTOMATED TESTED` isolated state logic, not `CLIENT_TESTED`. No Go suite,
  client installation, client execution, asset change, or cache deletion ran.
- Refinement continuation (2026-10-01): reused the constructor/vptr proof and
  cached callee bodies; inspected only the previously unresolved costume
  trampoline `013c2680`. Read-only Ghidra runs used the matching program hash
  and completed without `SCRIPT ERROR`. Reproduce instruction caches with
  `instructions:00524ded instructions:00480c25 exact:005a55b4 exact:005a55bc`,
  `instructions:013d2000 instructions:013c2680`, and `instructions:00525954`.
  The committed excerpt retains only the refinement/initial-grade writes,
  exact slots, complete scene refinement helper/trampoline, and inherited-stack
  jumps; its final row records the retained 151 instructions/53 references,
  not the complete cache counts. Incremental Release/Win32 architecture build
  and execution passed 61,156 checks plus static assertions, including two
  new checks and seventeen new layout assertions. This is `STATICALLY VERIFIED`
  native layout/binding evidence and `AUTOMATED TESTED` candidate representation,
  not rendering or cache-policy parity. Production code, assets, wire, and
  server behavior are unchanged; no product rebuild or client execution ran.
- Appearance wrapper continuation (2026-10-01): reused the cached complete
  decompilation and exported only `instructions:00480a83` with Ghidra 12.1.2,
  `-process WYD.exe -readOnly -noanalysis`, and absolute project/output paths.
  Accepted output has the matching native SHA-256, 129 instructions, 54
  references, completion summary, and no `SCRIPT ERROR`. An initial relative
  project path was rejected; the corrected absolute-path run succeeded.
  Incremental `ArchitectureTests.vcxproj` Release/Win32 build and execution
  passed 61,154 checks, including eighteen new source-contract checks. This is
  `STATICALLY VERIFIED` wrapper evidence and `AUTOMATED TESTED` source ordering,
  not renderer execution. No production source, server, assets, or wire changed;
  no product build, runtime installation, or client execution was needed.
- Grid/cursor continuation (2026-10-01): reused the published grid constructor,
  exact slots, and four detachment/hit-test instruction bodies. Inspected the
  global writer and concrete cursor callback in the matching read-only Ghidra
  program; constructor/callback exports contain 52/18 instructions and the
  exact slot resolves to `0040a147`. Reproduce the cursor export with
  `instructions:00409cbc exact:005a3e98 instructions:0040a147`.
  Runs contained the expected SHA-256 and completion summaries with no
  `SCRIPT ERROR`. Source comparison confirmed no functional patch is needed
  for these helpers and recorded the existing matching-only cleanup difference.
  No Go/C++ tests, build, runtime installation, or client execution were repeated
  for this evidence/documentation-only batch. This is `STATICALLY VERIFIED`,
  not a promotion to `CLIENT_TESTED` or approval of complete UI parity.
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
- Shared type-38 continuation (2026-10-01): reused the complete helper path
  and side-effect-free refinement-callee evidence above; no new native
  export or catalog sweep. Extracted the unchanged query into
  `NativeItemVolatile.h` and routed only type 38 from `BASE_GetItemAbility`.
  The new source-routing regression failed before the production edit and
  passed afterward. The existing 2134 lookup fixtures now exercise the shared
  core through the quote wrapper; production compilation checks both callers'
  actual array types. Other ability branches are unchanged in the scoped diff.
  `Build-Client.ps1 -Configuration Release -NoDeploy` passed with 60,969
  architecture checks, 221 socket checks, asset/shader gates, and an incremental
  x86 build. Existing signedness and deprecated Winsock warnings remain.
  Artifact `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `1F36C6A6FE1364D8BB18C6A91266720E1A4579763FDD85E73451D6906B06ACC5`.
  No server source changed or Go tests were rerun. No runtime installation,
  real item-use/pickup execution, or `CLIENT_TESTED` promotion occurred.
- Response precision continuation (2026-10-01): reused the retained equipment
  and Carry x87 instructions; no new native export. Added the shared ordinary
  `NativeSalePrice.h` calculation, routed the response through it after the
  unchanged catalog guard, and retained quote-only overrides in
  `NativeSaleQuote.h`. The source-routing regression failed before the patch.
  Seventeen arithmetic checks cover the existing fifteen boundary/precision
  fixtures, separation from quote overrides, and signed-storage truncation;
  the latter is not native negative-price policy approval. Together with the
  routing regression, these add eighteen checks to the existing suite.
  `Build-Client.ps1 -Configuration Release -NoDeploy` passed: 60,987 architecture
  checks, 221 socket checks, asset/shader gates, and an incremental x86 build.
  Existing signedness and deprecated Winsock warnings remain. Artifact
  `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `3AA30AD35A61FC9E807A1CA068EE7FF657624EBE79D89805D27EA73FA94750A3`.
  No server source or wire changed; unchanged Go validation was not repeated.
  No installation, client execution, or `CLIENT_TESTED` promotion occurred.
