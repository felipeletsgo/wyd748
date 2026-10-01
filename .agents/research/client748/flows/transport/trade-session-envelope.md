---
id: trade-session-envelope
title: Native trade session consumers and emitters
subsystem: transport
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-30
---

# Native trade session consumers and emitters

## Question

Which native human consumers receive incoming trade offers and closure, what
state do they own, and must local trade cleanup depend on an available UI
container? How do native invitation, acceptance, item insertion, gold, checks,
and closure populate the outgoing envelope? Is active item removal proven by
the same native input path?

## Evidence boundary

- USED: immutable `references/client748/WYD.exe`, SHA-256 above, x86 image
  base `0x00400000`, recovered read-only Ghidra project `WYD748/WYD.exe`.
- USED: [consumer instruction export](../../exports/trade-session-consumers.tsv):
  complete instruction rows for both consumers, selected receiver branches,
  and size-policy cases. Instruction-reference rows are omitted except the
  two size-switch targets; this is not a complete socket traversal export.
- USED: [focused emitter instructions](../../exports/trade-session-emitters.tsv):
  767 instruction rows selected verbatim from read-only/no-analysis exports;
  invitation, acceptance, insertion, gold, checks, closure, and input guards.
  The Field callback table slot and constructor/destructor references are
  included. This is not a complete export of the large Field callback.
- USED: [alternate grid input routes](../../exports/trade-session-input-routes.tsv):
  complete drop and move bodies (704 and 222 instructions), 98 mouse-dispatch
  instructions, and 79 right-button guard/return instructions. The 1,103
  instruction rows are verbatim excerpts from the same native program.
- USED: the existing [human receiver and container lookup proof](trade-check-confirmation-contract.md),
  [trade/inventory lifecycle](../ui/trade-inventory-layout.md), and
  [control ownership](../ui/control-focus-ime-lifecycle.md).
- USED: active `TMHuman.cpp`, `TradeSessionContract.h`, `Basedef.h`,
  `SGrid.cpp`, `TMFieldScene.cpp`, `ReceivedPacketDispatchTests.cpp`, and
  `SceneDisconnectContractTests.cpp`.
- USED: WYD-Go `internal/wire/codec.go`, `session_packets_test.go`,
  `internal/game/trade.go`, and `trade_check_contract_test.go`.
- NOT APPLICABLE: asset changes or TMProject 7.69 as parity authority.
  Existing widgets and IDs are reused; candidate source is the adaptation
  target, not proof of native addresses.
- LIMITED: the traced input path does not establish a native item-removal
  emitter. The size-policy body's external invocation is still unresolved in
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

### Outgoing events and construction

All traced offer sends call `FUN_0055F2DD` with `0x9C` bytes. The local
buffer starts at ObjectManager `DAT_013B71E8 +0xC60`: opcode `+0xC64`,
ID `+0xC66`, items `+0xC6C`, positions `+0xCE4`, gold `+0xCF4`,
check `+0xCF8`, and opponent `+0xCFA`. These are native object offsets,
not addresses or offsets to transplant into the adapted object layout.

- **Invitation:** Field callback `FUN_004662C5`, control 643 (`0x283`)
  selected at `0x0046BF04`, retains its peer/scene eligibility checks, writes
  `0x383` at `0x0046C0FC`, local human ID at `0x0046C117`, and selected
  opponent at `0x0046C131`; send is `0x0046C14A`. The selected target is
  then cleared. This branch does not initialize a separate empty offer.
- **Acceptance:** callback call `0x0046D0AF -> FUN_004640E5` reaches the
  dialog-kind 601 (`0x259`) branch at `0x00464115`. It copies the dialog's
  peer into `+0xCFA`, copies 156 bytes to a stack envelope, overwrites its
  local human ID and `0x383`, then sends at `0x0046418D`. Peer lookup and
  participant-label setup precede `FUN_0044B890(1)`; a missing peer prevents
  opening the trade panel, but that lookup is after the acceptance send.
- **Insert item:** grid mouse callback `FUN_004209FC` calls
  `FUN_004110F5` at `0x00420E58` on mouse-up `0x202`, inside the grid,
  with cursor mode zero. Grid type 7 enters the trade-inventory branch.
  A missing item or color other than `0xFFFFFFFF` returns `2`. The source
  position is the low-word cell X plus nine times low-word cell Y. Fifteen
  local grids `0x2100+i` are searched for the first empty slot. The native
  code forms the item destination at `0x004112DA`, copies eight bytes via
  the call at `0x004112E2`, writes the low-byte source
  position at `0x004112F5`, marks the source red, clears both check controls,
  updates the last-check timer, writes check zero/opcode `0x383`, and sends
  at `0x004113C8`. A full offer returns `0` without a send. It relies on
  prior header-ID initialization rather than rewriting that ID here.
- **Gold:** Field callback control 628 (`0x274`, selector `0x00468E09`),
  trade mode one at `0x0046945C`, clears controls 617/601 and the check
  byte, updates the timer, and writes the four-byte amount at
  `0x00469518`. Opcode `0x383` and a 156-byte send follow at
  `0x00469524/0x0046953F`. The focused instruction slice proves the
  destination and invalidation, not every input-parsing rejection.
- **Check:** control 617 (`0x269`) at `0x0046E2BF` requires the last-check
  time plus `0x7D0` (2000 ms) to have elapsed. The button is toggled via
  `SETZ`; its low byte is copied to `+0xCF8` at `0x0046E3C2`. A stack
  copy receives opcode `0x383` and is sent at `0x0046E3FE`. The timer is
  updated on both throttled and sent paths. This is an intention, not a
  client-side inventory/gold transfer.
- **Closure:** `FUN_0044B890(0)` clears the opponent/check and updates the
  timer. With the quit-send flag one, it zero-initializes twelve stack bytes,
  writes `0x384` and local human ID, and sends at `0x0044BD74`. Hiding
  also zeros the 156-byte offer and fills fifteen positions with `0xFF`
  (`0x0044BDBE`). This establishes the empty sentinel's raw byte, not a
  native signed comparison of populated positions.

### Item-removal boundary

The same native `FUN_004110F5` returns `1` for local-offer grid type 6
without editing the offer or sending: compare `0x00415172`, selected return
`0x0041517B`. Mouse-down also excludes type 6 (`0x00420C75`) from
`FUN_00410A91`; inspected `FUN_00410A91` handles types 1/4, not trade.
The inspected grid key callback `FUN_004107C1` restricts its Delete path to
type 5, not trade. These are scoped negative findings, not proof that no
native removal route exists elsewhere. Do not repeat these same roots to
search for a send already shown absent.

The active local-offer click instead removes the copied visual, restores the
source highlight, zeros the item, sets position `-1`, revokes checks, and
sends the unchanged `0x383/156` format. Preserve this existing supported
intention; do not label its interaction as native parity or delete it solely
because this native branch is a no-op. A parity claim needs an independently
reachable native removal path or an explicit documented deviation.

### Alternate grid input routes

The existing mouse receiver supplies two additional mouse-up routes when the
cursor is in pickup mode (`2`) and has an attached item:

- With `DAT_005CCF08 == 0`, call `0x00420EBD -> FUN_00416196` handles
  dropping the cursor item. Grid type 1 selects merchant sale (`0x37A/20`,
  opcode at `0x004162BF`, send at `0x004162EC`); type 5 selects the skill
  belt (`0x378/32`, `0x00416614/0x004166BB`). The fallback can construct
  item use (`0x373/36`, `0x00416A2B/0x00416A8C`) or attempt a grid insert
  and delegate to the current scene's mouse callback. Its local clearing
  affects Equip/Carry/Cargo item bytes or cursor visuals, not trade-offer
  slots. This complete body has no direct `0x383` construction/send; the
  delegated scene callback is not proof that all downstream routes lack one.
- With `DAT_005CCF08 != 0`, call `0x00420F62 -> FUN_00416E8A` constructs
  a normal item move (`0x376/20`), for a zero destination mask or a compatible
  item/destination mask. The two sends are `0x0041703A` and `0x004171AE`,
  following opcode writes at `0x00416FBC` and `0x0041713E`. Source and
  destination use their container/slot virtual methods and Carry's `x+9*y`
  fallback. The function finally clears `DAT_005CCF08` at `0x004171B6`;
  it does not clear an offered item/position or send `0x383`.

Right-button down (`0x204`) calls `FUN_0041EF0F` at `0x00420FBE`.
Its special skill-control branch requires IDs `0x223..0x23A`, excluding
local offer IDs `0x2100..0x210E`. After that branch, pickup mode detaches
the cursor. Ordinary item use requires grid type 0 or 3, selected by
`0x0041F51C/0x0041F52B`; type 6 branches to the return at `0x004209EB`.
These three additional routes do not establish native local-offer removal.
The right-button excerpt contains the relevant guards, not the whole 1,382-
instruction function; the negative claim is limited to these reachable
branches. No code, opcode, resource, or economic policy is changed.

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
| Native item insertion | Type 7, mouse-up, source unselected, free local slot | Eight-byte item and low-byte `x+9*y` position copied; checks revoked | Source marked red; local offer copy | Missing/selected source returns 2; full offer returns 0 |
| Native gold/check intent | Trade mode or check control, valid interaction | Gold invalidates checks; check toggles after 2000 ms | Amount/check feedback | Throttled check does not send |
| Active item removal | Visible trade, local human/opponent, matching offer grid | Item zeroed, position -1, checks revoked; server validates snapshot | Visual removed; source highlight restored | Missing dependencies or unmatched grid return |
| Incoming closure | Local human and live scene/model | Opponent/check/hover cleared before UI checks | Visible Trade/AutoTrade closed when available | Missing UI does not retain local model state |
| Closure for another human | Receiver is not scene's local human | No local trade mutation | None | Return |
| Invalid envelope | Any | No callback or mutation | None | Receive gate rejects |

### Vtables, vptrs, and receivers

Reuse the resolved human primary vtable `0x005A557C`, packet slot
`0x005A5580 -> 0x0052EAA9`, constructor/destructor vptr writes, and
container `+0x48` binding from the acknowledgement record. Native container
and standard control have different methods at the same numerical slot.
No candidate vtable or native object offset is changed by this patch.

The native Field table `0x005A4294` binds slot `+0x58` at `0x005A42EC`
to `FUN_004662C5`; table references originate in constructor
`FUN_004343A4` and destructor `FUN_004358DA`. Reuse the
[grid vptr and mouse callback proof](../../exports/grid-item-mesh-scale-vtable-callers.tsv):
constructor `FUN_0040DF9E` installs table `0x005A4024`, whose slot `+8`
is `FUN_004209FC`. The Field table's `+8` is a different callback, not the
control-event receiver; numerical slots cannot be substituted across tables.

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

Native insertion can publish even when visual allocation fails and does not
check the grid add result. The active insertion transfers the visual
successfully before claiming an offer slot, and validates the source against
authoritative Carry bytes. Preserve those compatible safeguards. They are
not defects to remove when matching the native packet layout.

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

| Field | Offset | Width | Active type/use | Native evidence |
| --- | ---: | ---: | --- | --- |
| Header | 0 | 12 | Size/opcode/ID and transport | Human ID/opcode dispatch; size-policy entries |
| Items | 12 | 120 | Fifteen eight-byte items | Eight-byte comparison/copy and fifteen-slot loop |
| Carry positions | 132 | 15 | i8, empty `-1` | Outgoing low-byte `x+9*y`; hide writes raw `0xFF`; incoming invitation copies bytes |
| Padding | 147 | 1 | x86 alignment | Not independently consumed |
| Gold | 148 | 4 | i32; server rejects negative/out-of-range gold | Four-byte read at `+0x94`; no native economic validation claim |
| Check | 152 | 1 | Active domain 0/1 | Byte read at `+0x98` |
| Padding | 153 | 1 | x86 alignment | Not independently consumed |
| Opponent | 154 | 2 | u16 | Word read and unsigned widening at `+0x9A` |

Native size-policy `FUN_0055890A` branches to `0x0055935F` for `0x383`
and `0x0055937B` for `0x384`; it compares the packet size with
`0x9C` and `0x0C` respectively. These are body/table facts, not proof of a
live transport rejection gate. Traced outgoing sends independently prove
156/12-byte sizes; they do not resolve that separate ingress call path.
Native position arithmetic/write and the empty raw byte are now established;
signed validation remains an active client/server rule, not an inferred native
comparison. Complete native input validation is outside the selected slices.

`TradeSessionContract.h` and `Basedef.h` protect the active ABI.
No resource, asset, opcode, payload, or server policy changes in this batch.

## Current mapping

### Buildable source

`TMHuman::OnPacketEvent` dispatches both shared opcodes; the exact-size gate
runs before casts. `OnPacketTrade` retains optional-control guards and the
separate local-offer initialization. `OnPacketQuitTrade` now clears model
state before checking the Field-scene container. Null scene/model and
local-human identity remain required.

`SGridControl::TradeItem` retains the native insertion envelope and local
slot IDs, adds authoritative-source and visual-ownership checks, and supports
item removal with the same envelope. `TMFieldScene` invitation, acceptance,
gold, check, and closure preserve the traced sizes/field offsets. It writes
local ID before every send and initializes a separate empty local offer,
rather than trusting stale fields inherited from a remote invitation.

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
| Outgoing insertion and positions | Fifteen slots, eight-byte items, low-byte `x+9*y` | Same 7.48 envelope plus source/ownership checks | Unchanged | Native format proven; retain compatible safeguards |
| Invitation/gold/check/close sends | `0x383/156`, check throttle 2000 ms, `0x384/12` | Same sizes and check revocation, explicit ID writes | Unchanged | Reuse proven emitter boundaries |
| Local-offer click removal | Traced type-6 branch returns without sending | Supported removal snapshot | Unchanged | Interaction parity unproven; no automatic deletion |

## Decisions

- Correct the proven cleanup-order difference without changing the wire,
  economic authority, UI bindings, or other handlers.
- Promote the formerly `UNMAPPED` incoming flow to `TRACED` using concrete
  dispatch, consumer, receiver, and lifecycle evidence.
- Outgoing construction/position evidence now covers invitation, acceptance,
  insertion, gold, checks, and closure. Keep the overall session `TRACED`
  while local-offer removal interaction and full native input validation are
  not resolved; do not claim full bidirectional behavior parity from sizes.
- Retain active exact-size rejection and optional-control protections.

## Gaps

- Resolve native item-removal reachability independently of the now-proven
  type-6 no-op. A focused next query should start from offer-buffer mutations
  or the drop fallback's scene delegation, not redecompile the already
  inspected TradeItem, mouse dispatcher, drop/move, key, and right-button
  guards. The alternate routes narrow this search; they do not prove that
  every native removal route is absent.
- Complete signed-input/domain evidence only if a dependent parity change
  needs it. Active server rejection/authority remains unchanged.
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
- Emitter continuation: read-only/no-analysis Ghidra decompilation and
  instruction/table exports completed with the same native SHA-256. The new
  focused export contains 767 verbatim instruction rows and callback binding
  evidence. No source, ABI, asset, or server input changed in this continuation;
  previous automated/build evidence remains applicable and was not rerun.
  Research schema passed for 94 records. Row provenance passed for all 782
  export rows, including 767 unique instructions, native identity, critical
  sends/input guards, and the Field callback binding. Repository layout/local
  links passed with 152 documents indexed; the central map was refreshed.
- Alternate-input continuation: reused the completed read-only/no-analysis
  export and decompilations, with matching program identity, all three
  requested instruction summaries, and a completion log without SCRIPT ERROR.
  The focused export retains complete drop/move bodies and selected mouse/
  right-button guards. Source and server inputs remain unchanged, so product
  tests/builds are reused rather than rerun. No native interaction-parity
  promotion follows from these scoped negative results.
  Row provenance passed for all 1,135 export rows, including 1,103
  instructions, two complete bodies, native identity, and 21 critical
  anchors. Research schema passed for all 94 records; 59 indexed flow states
  match their current front matter. Repository layout/local links passed
  with 152 documents indexed and the new export added to the central map.
- CLIENT-TESTED: not performed; no candidate installation or game execution.
