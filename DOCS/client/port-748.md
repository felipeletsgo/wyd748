# Porting the buildable client to WYD-Go

## Status

The imported source originally targeted the Global/KR 7.69+ client. In this
tree, it is being adapted as a single-version Win32/DirectX 9 implementation
for the proven WYD-Go 7.48 protocol, ABI, UI, and assets. Do not retain an
executable path for the upstream version.

The installed candidate is `tmproject/client748/project.exe`. With
`Build-Client.ps1 -NoDeploy`, only the artifact in `tmproject/build/` is
updated; the installed executable remains unchanged. Historical executables
and patchers are not fallbacks, products, or validation gates.

The source no longer exposes the later repurchase, numeric AccountLock, or
drop-list packet paths. This is not evidence that an already installed
`project.exe` contains those source changes. The [opcode catalog](../wire-opcode-catalog.md#retired-and-incomplete-legacy-paths)
separates retired paths from incomplete server integrations; in particular,
war letters, alternate mix, and account transfer are not supported end to end.

## 7.48 asset profile

The buildable client can read `tmproject/client748/` directly through
`WYD_ASSET_ROOT`. Its loaders explicitly recognize and translate:

- 7.48 texture tables with 264-byte records into the 528-byte in-memory
  TMProject representation;
- `UITextureList.bin` as the legacy name of `UITextureListN.bin`;
- `SkillData.bin` with 104 records of 96 bytes;
- `strdef.bin` with 440 strings of 128 bytes, leaving later, nonexistent rows
  empty;
- the 18 precompiled DirectX 9 shaders required by the renderer (`skinmesh`,
  `vseffect`, and `pseffect`), imported from the official `WYDESTINY` runtime
  at the same TMProject commit;
- the 7.48-only server selection screen in `SelServerScene2.bin`, including
  runtime composition of the six `NewUI_ServerList_*` and
  `NewUI_ChannelList_*` panels absent from the RC file;
- the optional parser for old RC records with inline captions, retained for
  legacy resources without replacing the main 7.48 screen;
- `sn.bin` as a fixed binary table of 11 names and 11 group orders, instead
  of the text format expected by newer TMProject. The selection scene loads
  and validates this file before creating controls; the old text read during
  app startup was removed;
- bounds on optional names, groups, and endpoints in initial selection and
  channel switching, based on the actual local table dimensions. Channel 10
  remains valid in `serverlist.bin`. If no safe optional name exists, the UI
  uses the channel number. An aggregate source outside the table is rejected
  without reading adjacent memory. Empty or unterminated endpoints clear the
  local destination instead of retaining a previous IP. This is
  `MODERNIZACAO_COMPATIVEL` and does not change the wire contract;
- the same compacted visible row for each server group's background, label,
  and click target. This keeps groups aligned when there are gaps and retains
  the positions of the dense 7.48 configuration without changing the wire;
- the final color on every group background after the selection transition,
  rather than only the first two. Duration, logos, and server contract remain
  unchanged;
- `config.txt` as the named 7.48 configuration, preserving resolution,
  window mode, UI variant, camera, cursor, audio, and animation without
  overrides.

`ItemList.bin`, `ValidIndex.bin`, `object.bin`, `serverlist.bin`, and
`AttributeMap.dat` match the sizes consumed by the code or have only an
additional checksum ignored by the loader. Asset checks should follow the
loaders, the `wyd-client748-assets` skill, and
`tools/client-assets/Audit-ClientAssets.ps1`, which distinguishes declared
debt from unclassified missing references and casing mismatches. The current
build is described in [Build and integration](../build-and-integration.md).

For same-machine development, run
`tools/client-assets/New-LocalServerList.ps1` from the repository root. It
creates the ignored `tmproject/client748/serverlist.local.bin`, preserving the
7.48 encoded table and status URLs while setting configured game channels to
`127.0.0.1`. The source-built client prefers this local table when it exists;
otherwise it reads the versioned `serverlist.bin`. A malformed or unreadable
local table is rejected rather than falling back to a different server. Do
not package the local table for global players: loopback would address each
player's own PC.
The override applies to a newly built `project.exe` or the build artifact when
started with `client748` as its working directory; the older, untracked
`client748/WYD.exe` does not use it.

A missing literal source reference alone does not prove that a 7.48 asset is
missing. In particular, `TMSkinMesh::SetCostume` still contains later-source
paths and paths overlapping the cataloged KR collection. The
`SourceMissingUnclassified` bucket is diagnostic for prioritizing research;
do not fabricate assets, remove `case` branches, or turn that bucket into a
parity gate without confirming the active flow and resource provenance.

The original 7.48 client did not have these shaders because its executable
used another rendering path. They are a runtime dependency of the buildable
client. Their compatibility with vertex declarations still needs validation
alongside the renderer; this reorganization performed neither visual
validation nor an automated shader-hash gate.

The TMProject render-target initializer was also corrected to use
`D3DPOOL_DEFAULT`, required by DirectX 9 for textures with
`D3DUSAGE_RENDERTARGET`. The imported code used `D3DPOOL_MANAGED` and
allocated the source texture twice, causing initialization failure and a leak
before the first scene.

## Cataloged costumes: selection and loading

The `Equip[13] -> TMHuman::InitObject -> TMSkinMesh::RestoreDeviceObjects`
flow now uses the full costume ID and the `Costumes-KR.json` table. Of 135
items, 130 select the 129 cataloged renderers; five base costumes delegate to
an existing renderer. Imported costumes bypass the later `SetCostume` switch.
Items outside the collection retain the existing path; absence from the
catalog alone does not justify removal.

Selection preserves the current skeleton and its variants, and uses skeleton
parity to choose King and TopRanker variants. An internal marker isolates
imported renderers from NPC types; it is not sent over the protocol. The six
parts are resolved by index, with no shared cursor: an empty part zero keeps
the native face, other empty parts are omitted, and parts outside the table,
such as weapons, retain the original path. There is no new runtime JSON parser
or change to score, bonuses, or server authority.

Classification: `MODERNIZACAO_COMPATIVEL` in source, reusing data already
validated as `PARIDADE_NATIVA`. Evidence for this work:

- Native binary/Ghidra and studied decompilation: used through the
  [KR costume record](../../.agents/research/client748/inventory/costume-native-contract.md),
  including selection, the internal marker, and skeleton/face preservation;
  no new analysis or historical-binary patch execution.
- 7.48 assets: used; manifest, 774 parts, and dependencies checked.
- Current TMProject: used to integrate selection and loading. Its later
  hardcoded mapping contradicts the manifest for imported costumes with
  different types or paths, not for the entire renderer.
- WYD-Go and tests: used; current equipment projection and slot 13 consumption
  inspected, focused client tests added. No wire change and no new server suite
  run in this batch.
- Additional guides: not applicable; no new contract was introduced.

`tools/client-assets/Export-CostumeTable.ps1` emits the C++ table for review;
`-Check` compares the compiled header with the manifest and checks referenced
files. This gate runs once per `Build-Client.ps1` invocation, before tests or
candidate installation.

Batch validation: Release build, table and dependencies checked, and 35,239
automated assertions passed. The candidate was installed with SHA-256
`9021B1FAAB444CBE85EE7141368C14890969A51499829848DE360752BC5B5E3F`.
On 2026-09-12, the user confirmed that costumes worked in game:
`CLIENT_TESTED` for the exercised flow, as well as `BUILD_VERIFIED` and
`AUTOMATED_TESTED`. That confirmation does not distinguish every item/body,
remote observer, logout/relogin, or transformation, and does not imply
exhaustive coverage of that matrix.

## Contracts

1. `model.Score` remains the sole authority for attributes.
2. `STRUCT_SCORE` is 140 bytes: 35 uint32 fields in the current coordinated
   contract.
3. Score fields received from the client never drive gameplay.
4. Additional attributes are part of score directly; no sidecar is active.
5. Migrate each packet with size and offset tests on both sides.
6. `tmproject/client748/project.exe` is the only executable candidate; the
   historical 7.48 binary is a read-only Ghidra reference.

### Rule for comparing versions

TMProject 7.69 may provide candidate architecture and algorithms. For legacy
boundaries, native 7.48/Ghidra remains the primary evidence. Classifications
and gates are in [AGENTS.md](../../AGENTS.md).

Compatible modernization preserves the proven contract. A coordinated
extension may change it when both ends are explicitly adapted and tested.
Neither should be presented as native parity. Later assets require validation
of formats, resources, loaders, and observable flow; absence from the native
client does not justify automatic removal.

Failure to read an RC file aborts initialization of a 7.48 scene. The
`FieldScene2.bin` HUD fallback applies only to a successfully loaded resource
that lacks modern control `66817`; an invalid resource must not be treated as
a legitimate layout variant.

The missing-attacker recovery path in `TMFieldScene::OnPacketAttack` now reads
damage target IDs without modifying the received frame and retains the
attacker coordinates from that frame for effect fallback. This is
`MODERNIZACAO_COMPATIVEL`: the existing `0x369/16` request and the server's
visibility and gameplay-space checks are unchanged. The native request
envelope is recorded in
[missing-entity-request](../../.agents/research/client748/flows/transport/missing-entity-request.md);
the exact target-selection predicate is not claimed as native parity. A
client-side attack/recovery run remains pending.

Attack rendering also bounds every damage-entry loop by the target capacity
of its opcode: one for `0x39D`, two for `0x39E`, and thirteen for `0x36C`.
The existing receive gate already verifies those native prefix sizes before
calling the scene. The bound prevents a valid short frame from making a
visual effect read entries beyond its payload; it does not change the wire
format or server-calculated damage. This is `MODERNIZACAO_COMPATIVEL`, using
the recorded native capacities in
[attack-frame-envelope](../../.agents/research/client748/flows/transport/attack-frame-envelope.md).

### Single-slot update destination validation

`TMHuman::OnPacketSendItem` rejects unsupported storage types before bag UI,
slot writes, or character appearance updates. Previously, a frame with a type
other than Equip (0), Carry (1), or Cargo (2) bypassed every slot branch but
still reached `InitObject`, which destroys and rebuilds the character mesh.
The shared predicate now validates both the type and signed position against
the actual destination array capacity. It preserves all existing valid slots,
including additional equipment, reserved Carry, and Cargo storage positions.

This is `MODERNIZACAO_COMPATIVEL`, reusing the unchanged 24-byte `0x182`
envelope in
[SendItem local update](../../.agents/research/client748/flows/ui/send-item-local-update.md).
It does not change server packets or claim native rejection behavior. Tests
execute 70 type/position boundary combinations and three empty-capacity cases;
a source-contract check verifies rejection before UI, model, and mesh updates.
These checks do not execute the DirectX scene. The server split-response tests
remain applicable because valid Carry destinations and wire bytes are unchanged.
`Build-Client.ps1 -Configuration Release -NoDeploy` passed with 58,274
architecture checks and 221 socket checks. The built artifact is updated in
`tmproject/build/`; no runtime executable was installed and no game was launched.

### Confirmed item-drop lifecycle

The `0x175/28` handler commits the confirmed Equip, Carry, or Cargo removal
even when the corresponding visual grid is missing. It now tolerates a
missing cursor and releases any removed grid item before checking for the
local character renderer. Hover, last-dragged, and sell/split aliases to that
item are cleared before destruction; aliases to other items are retained.
A missing local character skips only the remaining presentation refresh,
not the authoritative slot update or ownership cleanup.

This is `MODERNIZACAO_COMPATIVEL`, reusing the native envelope and ownership
evidence in the
[drop-confirmation contract](../../.agents/research/client748/flows/ui/drop-confirmation-contract.md).
Packet layout, valid slot ranges, and server persistence/confirmation order
are unchanged. Two source-contract regressions failed before the patch and
passed afterward. These checks protect cleanup and guard ordering; they do
not execute the DirectX scene or prove an in-client drop interaction.
The incremental `Build-Client.ps1 -Configuration Release -NoDeploy` build
passed with 52,032 architecture checks and 221 socket checks. The candidate
was neither installed nor run; the drop lifecycle has source-contract and
build coverage, not `CLIENT_TESTED` status.

### Legacy sale-handler safety

`TMFieldScene::OnPacketSell` now bounds Equip/Carry indices before indexing
the model or grid array, tolerates absent merchant/equipment controls, and
clears matching hover, drag, sell-dialog, and cursor aliases before deleting
the detached visual. An incomplete visual without an item payload is still
released, without crediting gold. An absent local renderer no longer makes
the final appearance refresh dereference a null pointer. The duplicated
price calculation was consolidated without changing its arithmetic.

This is `MODERNIZACAO_COMPATIVEL`, based on the current grid ownership
contract: `PickupItem`/`PickupAtItem` transfer ownership, while the item
destructor does not clear interaction aliases. The existing
[equipment-slot policy](../../.agents/research/client748/flows/ui/equipment-slot-compatibility.md)
remains in force. This is not a native-parity claim for the sale response.
The current Go `onSellItem` persists the sale and sends `SendItem` followed
by `UpdateEtc`; it does not send this inherited `0x37A` response. Server
authority, persistence, prices, and outgoing packets are unchanged.

Three source-contract regressions failed before the patch and passed after
it. The incremental `Build-Client.ps1 -Configuration Release -NoDeploy`
build passed with 52,035 architecture checks and 221 socket checks. No files
were removed, and the executable was neither installed nor run. These are
source/build checks, not execution of the DirectX sale UI.

The initial receive guard protected the inherited callback's memory reads
with a minimum 20-byte representation. The 2026-09-30
[native sale-envelope record](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md)
now closes the exact-size question: the recovered native field receiver
`FUN_00492e7d` dispatches `0x37A` to `FUN_00487e23`, and the size-policy
case in `FUN_0055890a` requires exactly 20 bytes. The packet-receiver
vtable slot and payload word offsets/signedness are recorded with focused
instruction evidence. The response is not merely inferred from its name
or the client request structure.

`LegacySalePacket.h` keeps the unchanged representation and offset assertions.
`ExpectedSize(0x37A)` is now 20, and the shared fixed-size receive policy
requires matching metadata/embedded opcodes and exact declared/actual lengths.
This envelope restriction is `PARIDADE_NATIVA`; the shared fail-closed gate
and ownership protections remain `MODERNIZACAO_COMPATIVEL`. Larger consistent
frames no longer reach the lengthless handler. No server response was added.

Five regression failures demonstrated that 21-, 24-, and 65,535-byte frames
were delivered before the exact-size patch and that the size policy was missing.
The fixed little-endian 20-byte fixture, all truncated prefixes, null storage,
mismatched discriminants, inconsistent lengths, oversized frames, and
unaligned immutable storage are covered. The updated architecture suite
passes 58,666 checks; the socket suite passes 221. Five focused Go merchant,
rejection, city-tax, and persistence-rollback tests pass without server changes.
`Build-Client.ps1 -Configuration Release -NoDeploy` completed the integrated
incremental build. Existing compiler warnings remain in unchanged legacy
source; this is not a warning-free build. The artifact stays in the ignored
build directory and the installed `project.exe` was not replaced.

Remaining boundary: full downstream native grid/UI and price parity is not
established by the envelope trace. The current server's snapshot-based sale
does not exercise this legacy callback. No visual client execution or
installation was performed; the record remains `CONTRACT`, not `CLIENT_TESTED`.

### Auto-trade visual ownership

`TMFieldScene::OnPacketItemSold` preserves the documented
[native item-sold contract](../../.agents/research/client748/flows/ui/item-sold-contract.md)
for `0x39B`: only the visible shop with the matching clone and an in-range
listing slot is affected. The delta clears its item, carry mapping, and price
even when its optional grid is absent, and hides the corresponding price label.
The server sends no replacement snapshot after each sale. Repeated deltas are
idempotent; an already absent visual needs no deletion. This internal state
synchronization is `MODERNIZACAO_COMPATIVEL`, with no wire change. The sale
notification, snapshot replacement, seller preparation, and both panel-close paths use
`WYD748_ReleaseAutoTradeItem` for detached visuals. Before deletion it clears
matching hover, drag, and sell-dialog aliases and detaches a matching cursor
item through `DetachItem`, which also resets the pickup cursor style. Null
items leave unrelated interaction state unchanged. Snapshot replacement
releases the old visual before allocating a new one; panel closure retains
the existing cargo highlighting, twelve-slot cleanup, and state reset.
Seller preparation restores cargo highlighting before releasing each visual
through the same helper. Preparing a priced offer also rejects a cargo visual
without an item payload before copying its data.

This is `MODERNIZACAO_COMPATIVEL`. Packet layout, server authority, pricing rules,
and shop snapshots are unchanged. Source-contract checks cover all five
cleanup call sites, the shared helper's guards and release ordering, and the
cargo payload guard before offer creation. Executable policy tests cover
clearing each of the twelve offers, preserving other offers, repeated deltas,
and invalid indices; source checks connect that policy to the UI handler.
The incremental
`Build-Client.ps1 -Configuration Release -NoDeploy` build passed with
58,068 architecture checks and 221 socket checks. These checks establish
`AUTOMATED TESTED` policy and source-contract coverage, not execution of the DirectX
shop lifecycle. The candidate was neither installed nor run; a two-client
purchase while hovering or selecting a listing remains a pending runtime
gate, including repeated notifications and shop close/reopen behavior.

### Auto-trade purchase confirmation

The purchase dialog retains a control ID, not a copy of the selected offer.
Previously `SendReqBuy` used that ID to read the current snapshot without
bounds checks: a replacement snapshot could redirect an old confirmation to
a different item, price, or seller. Snapshot replacement and panel closure
now invalidate purchase-dialog message `646` and its argument. An item-sold
notification invalidates only the matching slot's confirmation. Other dialogs
are preserved, and hiding an already hidden dialog does not steal UI focus.

Before reading listing arrays, `SendReqBuy` resolves controls `653..664` for
the native UI (`653..662` for the imported resource), rejects missing local
state or a hidden shop, and requires a present listing visual and a nonempty,
positive-price offer. Resource-ID static assertions tie the resolver to the
active control constants. The existing request fields and layout are unchanged;
the Go `onReqBuyAutoTrade` remains authoritative for the shop, item, price,
tax, account state, persistence, and rejection of stale purchases.

This is `MODERNIZACAO_COMPATIVEL`, not a new native-parity claim. Executable
policy tests cover control boundaries, unsigned overflow, all twelve-by-twelve
selected/sold slot combinations, unrelated dialogs, and invalidation inputs.
Source-contract checks cover the cancellation and send paths. The incremental
`Build-Client.ps1 -Configuration Release -NoDeploy` gate passed with 53,577
architecture checks and 221 socket checks. No server contract changed, so
unchanged server tests were not rerun. `CLIENT_TESTED` remains pending: no
game was launched, no candidate installed, and no real purchase was made.

### Auto-trade publication validation

The publish button checks the complete twelve-item `MSG_AutoTrade` array
through `field_interaction::HasAutoTradeOffers`. The inherited ten-slot scan
incorrectly rejected a shop whose only remaining offer occupied slot 10 or
11 (zero-based). Empty shops still fail locally. The existing
[native envelope](../../.agents/research/client748/flows/transport/auto-trade-envelope.md)
remains `0x397`, 196 bytes, with twelve offers; server validation is unchanged.
This is `MODERNIZACAO_COMPATIVEL`, not a new packet or parity claim.

The price prompt rejects zero before reserving Cargo or populating a listing.
Previously zero passed local validation, occupied a slot, and caused the server
to reject publication. The prompt remains open and focused for correction,
using existing message IDs. `IsValidAutoTradePrice` retains the existing client
range of 1 through 1,999,999,999; it does not change the server's independently
validated ceiling of 2,000,000,000. No tax or purchase accounting changed.

Opening the price prompt now validates its caption, edit control, and Cargo
slot before changing the item highlight, prompt mode, or selected position.
The shared stack-split entry also rejects missing scene/container, item data,
or prompt controls before reserving its item. Existing control-ID aliases
already resolve native controls 626/627/630; no resource remapping was needed.
This is internal failure-path hardening (`MODERNIZACAO_COMPATIVEL`), with no
wire or server change. Source-contract checks enforce the validation order;
they do not execute the DirectX dialog or prove runtime interaction.

Stack-split entry now checks the pointer hit and exact Carry-grid identity
before selecting an item. The container broadcasts mouse events to visible
controls, and cell-coordinate truncation is not a hit test; previously an
outside Shift-click could select an unintended source. Prompt controls are
resolved before selection, and an already-visible shared prompt rejects the
new intent without replacing its selected item or mode. Source-contract tests
cover both rejection paths. This preserves the existing split packet and
authoritative server validation; it is not a new native-parity claim.

Split confirmation now establishes membership in the live Carry grid before
dereferencing the non-owning selection. A removed source closes the prompt;
invalid quantities keep it focused for correction. The shared quantity policy
requires a byte-sized stack and leaves both resulting stacks nonempty, so zero
is no longer sent to the server. Confirmation and cancellation both clear the
split mode and pointer and restore the highlight only for an item still owned
by the grid. Neither path removes an item or edits its quantity locally.
The existing 24-byte `0x2E5` request is unchanged. Executable policy tests cover
quantity boundaries and invalid/foreign grid selections; source checks connect
those policies to the scene's send and close paths. The focused Go command
`go test ./internal/game -run 'Test(SplitItem|DeleteItem)' -count=1` passed,
including malformed requests, full Carry, quantity boundaries, independent
UIDs, repetition against the remaining quantity, and save rollback. Persistence
tests use the existing in-memory store double, not a live database. Real dialog
execution remains untested and is not inferred from these checks.

`TestSplitItemOutboundSlotSnapshots` adds 18 executable server scenarios across
the first, middle, and last visible Carry slots, all three `EF_AMOUNT` effect
positions, and successful/failed saves. It observes the persistence boundary
before any confirmation is queued, checks both stacks in the account snapshot,
then decrypts the two actual `Session.Send` frames. Each must be a 24-byte
`0x182` addressed to the player, source slot first and destination second, with
the exact authoritative item index and all six effect bytes. A failed save must
restore both slots and send their original contents, including the empty
destination. UID and timestamp metadata remain outside the eight-byte wire
item. `go test ./internal/game -run '^TestSplitItem' -count=1` passed for this
test-only addition; the existing client frame gate and local slot application
are unchanged, so the earlier client build remains applicable. This does not
execute the C++ scene or validate a live database transaction.

`AUTOMATED TESTED`: executable client policy tests cover all 4,096 occupancy
patterns and negative item sentinels; a source-contract check ties the policy
to publication before sending. Go tests accept an isolated offer in each of
the twelve slots, preserve its cargo position and price, and reject empty
shops and forged items. Price policy tests cover zero, negative values, the
prompt ceiling, and 64-bit extremes; source checks verify rejection before
listing mutation. Go tests cover price boundaries in all twelve slots and
prove parsing leaves Cargo unchanged for accepted and rejected requests.
The focused parser/purchase suite and incremental
`Build-Client.ps1 -Configuration Release -NoDeploy` passed (58,200 architecture
checks and 221 socket checks). No candidate was installed or game launched;
publishing and buying a final-slot offer remains a pending client runtime gate.

## TCP buffering

`CPSock` compacts only the consumed prefix before receiving more bytes and
retains the full unread suffix, including a read that exactly fills the
remaining capacity. A stale asynchronous notification returning
`WSAEWOULDBLOCK` preserves the connection; orderly shutdown still reports
disconnection. Framing validates the size and waits for the entire packet
before consuming a configured rolling receive key.

This is `MODERNIZACAO_COMPATIVEL`: packet sizes, encryption, opcodes, and
server behavior are unchanged. It is independent of the user's resolved
connection failure caused by an incorrectly configured IP address.

Outbound appends also reclaim the already-sent prefix before checking queue
capacity, retaining the encrypted pending suffix without changing its bytes
or order. Automatic sends consume a configured rolling key only after the
frame is accepted into the queue, not on local size/capacity/socket rejection.
A partial flush does not undo an accepted frame's key. The Go server's
`wire.CharList` currently sends zero `SecretCode` bytes and therefore does
not activate rolling keys; the capacity correction applies in that mode too.
Explicit-key sends remain independent of the automatic sequence.

Flushes reject invalid cursors without erasing the queue and return failure
for definitive Winsock errors. Positive short writes continue until the
queue drains or `WSAEWOULDBLOCK` retains the unsent suffix. The active connection
entry point subscribes to `FD_WRITE` and accepts event registration only on
its zero success result. `NewApp` routes notifications through the tested
socket dispatcher: write-ready retries pending bytes, events for a different
socket are ignored, and read EOF, close, or event errors use the existing
scene disconnect notification. A repeated close after cleanup is ignored.
The event/error decoding and write retry rules follow Microsoft's
[WSAAsyncSelect contract](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsaasyncselect).

These outbound changes are also `MODERNIZACAO_COMPATIVEL`, not a new native
parity claim. Production C++ and the Go login contract were inspected; the
existing cipher and key derivations are unchanged. No new native/Ghidra or
asset research was needed for this internal queue-state correction.

`SocketReceiveTests` compiles the production socket implementation and covers
every split of an encrypted frame, repeated incomplete reads, buffer
compaction, exact-capacity reads, would-block, and peer shutdown using local
TCP. Outbound regressions reproduce rejection and capacity failures before
the fix, then verify unchanged pending ciphertext, decoded frame order and
payload, key progression, explicit keys, and zero-SecretCode operation.
Additional tests reproduce the previous false-success send error and invalid
cursor data loss, then cover bounded real TCP backpressure, exact-byte retry
delivery, synthetic event dispatch, fatal retry errors, and failed event
registration without creating a window. Actual Windows message delivery and
scene behavior in the game remain untested; synthetic notifications do not
establish those gates. Socket identity filtering covers different handles,
not reuse of an identical handle by Winsock. Both login connection paths now
queue the raw four-byte `InitCode` through the same output buffer as encrypted
frames. A short send or `WSAEWOULDBLOCK` retains the unsent handshake prefix
before the first frame; a definitive send error closes the socket. Loopback
tests verify the handshake bytes, frame ordering and decoding, rejection of a
duplicate handshake, and definitive send failure. The Go session test accepts
a split `InitCode` followed by the first frame in the same write. Actual
asynchronous Windows notification delivery remains a client-runtime gate.
Closing or destroying a socket now releases any live handle and clears the
previous session's cursors, queued keys, and counters before a reconnect.
Focused tests cover that teardown and an idempotent second close.
The unused `StartListen` server-listener path, `ReadMessage2` alternate
parser, `SingleConnect` duplicate connection path, explicit-key send facade,
and send-queue compaction facade were removed after checking their callers.
The active client still uses the same outbound socket owner for login, field
traffic, and migration;
the native 7.48 socket-owner record supports that boundary. This removal
does not alter handshake bytes, packet framing, or server behavior.
Malformed frame length, rolling keyword, or checksum now closes the stream
and notifies the scene through its existing disconnect path. Previously the
read loop stopped without closing, so an invalid frame could be retried at
the same cursor on later notifications. This is compatible error handling;
valid packet bytes and dispatch behavior are unchanged. Parser tests cover
all three errors, and a source contract checks close-before-notify ordering.
The parser also returns no packet view when checksum validation fails, so a
caller cannot accidentally treat that rejected frame as dispatchable.
`Build-Client.ps1` runs this gate before building or installing a candidate.
Validation: 221 socket checks, 52,020 architecture checks, the focused Go
handshake tests, and the incremental Release `-NoDeploy` build passed. Status:
`AUTOMATED TESTED`, not `CLIENT-TESTED`; the game was neither installed nor
executed for this batch.

## Motion/emote `0x36A`

The native 7.48 client sends a 20-byte emote request and waits for its own
returned frame before allowing the next emote. The server validates a living
character, accepts only ordinary motions with zero Parm, replaces the claimed
ID and Direction with authoritative values, and echoes to the owner and
visible observers. A dead character cannot use this route to restart an
animation. The death transition also clears any pending emote because its
late response is intentionally ignored; otherwise revival could leave emote
input blocked. Scene-owned effects are allocated only while their container
exists, avoiding a leak if that container is absent. The
[native flow record](../../.agents/research/client748/flows/transport/motion-emote-roundtrip.md)
holds the wire offsets and evidence; client execution remains pending.

## Active score layout

### Zero-HP death transition

The server sends the authoritative `0x181` vitals snapshot before `0x338`
kill confirmation on lethal mob damage. The source client now enters `Die()`
when a valid vitals snapshot sets HP to zero, even if kill confirmation is
delayed or absent. A later `0x338` remains idempotent through `Die()`'s
existing guard. Pending travel motion may not restart a dead character's run
animation. These are coordinated-client resilience changes, not claims that
native 7.48 derives death from `0x181`.

The numeric HP cell and the textured HP orb are separate controls. The native
compact HUD has a main HP progress control (`TMP_HP_PROGRESS`) and a translucent
overlay (`TMP_HP_PROGRESS_TR`). Progress reaching zero does not suppress the
controls' panel art; the overlay also uses a negative texture set. Both vitals
updates and score refreshes now hide both visuals at zero HP and restore them
when HP becomes positive. No asset bytes or general renderer behavior changed.
The field scene can open the respawn prompt when the death animation finishes,
and its mouse handler has a click fallback.
The compact 7.48 input adapter previously discarded dead-player clicks before
that fallback; it now preserves the same dead-player, familiar, and town checks.
The route rejection preceding the logged death has no proven causal link to
kill confirmation or respawn.

The later screenshot still showed looping movement and no respawn prompt after
death. `Die()` previously truncated future route points but left active route
indices and movement state in place, so `FrameMove` could continue advancing
the route. It now freezes the route at the current position. It also enforces
the one-shot death animation and resets its start time after `SetAnimation`:
that method can return early if the mesh rejects the death clip, otherwise
leaving the previous run animation's loop state in place and preventing the
completion callback that opens the prompt. Late non-revival motion packets are
ignored while dead. These are source-level resilience fixes; the screenshot
alone cannot identify which of these paths occurred at runtime.

The source build and architecture tests passed (51,987 checks). The Release
candidate containing the death-state, HP-orb, input-adapter, route-correction,
and shared-terrain changes was copied to `client748/project.exe` with matching
SHA-256 `7F5430279644A8B285BBBE1F04628B6B8C49DBF3528047C0379C39862255E96E`.
The server game and wire package tests passed. Death animation, orb appearance,
route correction, and respawn interaction in the running client remain untested.

The native field tick also sends a 12-byte `0x289` recall request if the
character remains dead for more than three minutes outside the restricted
town tiles. The compact 7.48 scene previously returned before that fallback;
both scene lifecycles now share the timed decision and send a zero-initialized
request once. The server still decides whether revival is valid. The Release
`-NoDeploy` build and 51,995 architecture checks passed for this change, but
the resulting executable has not been installed or tested in the running
client. This does not prove that the earlier five-second prompt or visible
death effects work.

The five-second prompt countdown now uses its own recall start time instead
of the unrelated server-selection timer. After sending `0x289`, the pending
flag also prevents the portal effect from replaying during the remaining
timer-cleanup interval. This is a source-level lifecycle correction; it does
not change the packet or claim a completed in-client respawn test.

The same recall request no longer clears `m_cDie` or starts the revival
animation locally. A rejected or lost request therefore leaves the character
dead; only the server's positive-HP `0x181` response clears that state and
starts the revival animation. The request-time effect remains cosmetic. A
source contract check covers the separation, but a running-client respawn
test remains pending.

The server death publisher includes the victim in its nearby-player query;
`TestPublishPlayerDeathReachesVictimAndVisibleObservers` covers delivery to
the victim and observers. The client kill-confirmation handler now also
handles an attacker absent from the local scene when formatting the
resurrection-item notice. Neither static contract proves the visible death
or respawn interaction without an in-client test.

The same kill-confirmation path now skips an unavailable inventory grid or
item backing record and omits the optional resurrection-item notice when its
Help list is unavailable. This prevents a partial FieldScene2 resource load
from dereferencing a missing control during death; it does not alter the
server's kill or restart packets. The Release `-NoDeploy` build and 52,002
architecture checks passed. The resulting executable was not installed or
tested in the running client.

The base scene creates and registers the respawn message box for both field
lifecycles. The compact 7.48 tick previously returned before the regular HUD
code that dismissed this modal prompt after authoritative HP recovery. That
dismissal now runs before the lifecycle split and applies only to message 11.
This is covered by a source contract check; visible dismissal still requires
an in-client test.

During air travel, the `0x181` handler previously updated the local entity's
HP but skipped the ObjectManager score along with the temporarily suppressed
HUD redraw. The compact respawn prompt and recovered-HP dismissal read that
score, so a lethal snapshot could leave them looking at stale HP. The handler
now copies only the four resolved HP/MP fields for the local character even
while air travel is active, preserving unrelated score fields. Nonlethal
in-flight HUD redraws remain gated. A lethal snapshot now enters `Die()`
before that visual gate: death cancels flight, so the same `0x181` redraws
the local HP bar at zero instead of waiting for another packet. Air-travel
completion also no longer restores a pre-death animation over the corpse. The server
already handles death during air travel without granting its pending destination.
This `MODERNIZACAO_COMPATIVEL` changes neither the 28-byte vitals contract nor
the `0x289` restart request. A source contract guards the ordering; client
execution remains unavailable. A subsequent client-side flight fix captures
the preflight visual origin and explicitly cancels the route during death,
including death signals that arrive before HP reaches zero. It discards the
pending displacement, restores the origin and mount, and skips the flight-end
packet. An unrelated server-authoritative teleport instead keeps the new
position, discards only the flight displacement, and likewise skips the
flight-end packet. Ordinary arrival still uses the existing 7.48 start/end
wire contract. The server now clears its pending route when publishing the
death, without waiting for the next client packet; a late end still cannot
grant the destination. The shared player-movement path also refuses to publish
an `ActionStop` (`0x367`) route at zero HP and discards any pending route before
another authoritative step can advance. Focused tests cover a late stop, an
in-flight route, and normal routing after HP recovery. This is
`MODERNIZACAO_COMPATIVEL`; source and
pure-state client checks and focused server tests cover the branches, but
the running-client appearance remains unverified. Air-travel start now leaves
state untouched when the scene's effect container is unavailable; ordinary
completion skips its cosmetic effect if that owner disappeared, while still
sending the existing end packet. The wire and server destination rules remain
unchanged. The death clip's optional particle, class-specific death effects,
and recall, teleport, and relocation countdown visuals also allocate only
while the scene owns an effect container. Missing cosmetic ownership cannot
suppress the recall packet, death sound, or death-state transition.

The final incremental Release `-NoDeploy` build and 52,018 architecture
checks passed for this batch. The executable was not installed or run.

### Shared static route maps

The rejected `0x366` route at 2026-09-23 16:59:35 ended at `(3647,3112)`.
The server's `data/maps/HeightMap.dat` records blocked height `127` there,
but the client terrain tile plus static object masks reconstruct height `-88`.
The client originally accepted that destination, while the server refused it.
The denial preceded the lethal hit; it does not by itself explain the missing
death/respawn UI.

The 7.48 client's `Env/AttributeMap.dat` is the more populated attribute
asset: its consumed 1,048,576-byte payload has 97,102 nonzero cells, versus
88,069 in the former server asset. It contains 9,033 populated cells absent
from the server asset; none are populated only in the server asset. The older
server asset also had 206 bit patterns not contained in the corresponding
client bytes, including 193 server-only blocked cells. The choice is therefore
the 7.48 client payload, not a bitwise union of potentially conflicting maps.
The client retains its four-byte asset trailer, which its loader ignores;
the server stores exactly the consumed payload. The server asset SHA-256 is
`995FAC5A89E14B2A08DC39B4E922DF33FD7AB31E52763A1E7598F41BFED09C4E`.

The server's 16,777,216-byte `HeightMap.dat` is the complete world-height
source. The client has only 96 terrain tiles, so reconstructing a global map
from those tiles would erase or block regions that still exist on the server.
An exact copy is packaged at `tmproject/client748/Env/HeightMap.dat`. Client
startup rejects a missing or malformed copy. `BASE_ApplyAttribute` projects
the shared global heights into each route window before applying the shared
attribute mask, including initial load and neighbor transitions. The terrain
tile and object masks still provide rendering geometry; dynamic collision
changes are separate. The shared height payload SHA-256 is
`B83A9DE78A32A79FC2647699EB44BE1561BED62FBC48EAA7165BA26C1297C640`.

Run `pwsh -NoProfile -File tools/client-assets/Sync-TerrainMaps.ps1` to check
both asset identities; use `-Apply` only to regenerate the derived copies.
`Compare-TerrainCollision.ps1 -RequireParity` compares the route maps and
also reports the original tile/object reconstruction. In the 41-by-41 area
around `(3647,3112)`, all 1,681 static route cells and their route-height
edges now agree; the legacy terrain/object reconstruction still differs in
height at 36 cells. With the same attribute mask on both surfaces, 12 of
those cells are walkable in the reconstructed terrain but blocked in the
shared route map. Another 24 cells are walkable in both maps but have
different heights (at most six height units). No cell is blocked only in the
reconstructed terrain. A wider 81-by-81 sample around `(3648,3136)` also has
zero static route-map disagreements. At the rejected destination, both route
maps now read `127`; the original terrain/object reconstruction reads `-88`.
These are asset and source checks, not an executed-client observation. The
route-map identity prevents this static mismatch from producing different
client/server route decisions, but the rendered terrain and static route
surface are not equivalent in this sample. Dynamic collision and visible
death/respawn behavior remain unverified in the executable client.

The server now overlays permanent ground-object collision onto its private
HeightMap copy during world creation. Object identity, position, and rotation
come from the 96-row `wydgo748/data/init_items.csv`; mask geometry comes from
the active client's `g_pGroundMask[10][4][6][6]`. A gate opening clears those
cells only after key consumption is persisted, matching the `0x374` update
sent to observers. Automated tests compare all 1,440 mask cells against the
client source, reject overlaps among the CSV objects, and check route blocking,
successful opening, and persistence rollback. The exact 5,760-byte table
also matches both native 7.48 executables at file offset `0x001BED50`;
`FUN_005554CC` references its VA `0x005BED50`. The automated test pins the
native table hash, so the active client/server mask geometry has native
evidence. The client collision primitive now rejects out-of-range mask and
rotation indices and a null HeightMap before indexing; valid object packets
use the unchanged mask logic. The incremental Release `-NoDeploy` build and
its architecture/socket gates passed, but executable-client behavior remains
pending.

The server now stops any pending authoritative route and sends its current
position to the owner when it rejects a `0x366` route. This is a coordinated
client/server extension using the existing 52-byte `0x366` action envelope
with `Effect=8`, zero speed, an empty route, and equal position and target.
The paired source client resets its local route at the authoritative position
without calling the teleport handler, clearing death state, or creating warp
effects. Corrections arriving after death are ignored by the client; dead
players do not receive new corrections from the server. This effect value must
not be sent to an unmodified native client: its generic illusion handler would
interpret the value as a teleport. The server does not accept a blocked
destination or advance its own position. Focused tests cover the packet,
rejection, and dead-player guard. This is a desynchronization safeguard, not a
map fix or evidence that death and respawn UI works in the running client.

The [canonical contract](../SCORE.md) replaces the historical 48-byte layout.
The current size is protected by `static_assert` in
`tmproject/TMProject748/internal/core/WYD748Compat.cpp` and by the encoder and
tests in `wydgo748/internal/wire/score.go` and `score_test.go`. Changes must
also cover every packet embedding the score.

## Adaptation sequence

1. Confirm the native flow and the historical reference SHA in Ghidra 7.48.
2. Locate corresponding callers, callees, structs, and assets in live source.
3. Adapt a small packet group or window at a time, removing an incompatible
   path only when evidence and a replacement contract exist.
4. Protect wire/ABI with `static_assert` and byte-for-byte tests.
5. Validate assets and compile. If installation is part of the gate, confirm
   the SHA-256 of `tmproject/client748/project.exe`. With `-NoDeploy`, record
   only the artifact in `tmproject/build/` and the pending installation.
6. Test owner, observer, failure, and relogin before promoting behavior.

Do not change several structural packets at once: `STRUCT_SCORE` is embedded
in larger structures, so each change needs a corpus and test for the final
packet. Never add protocol-variant selection during authentication or
elsewhere; this source implements only the 7.48 contract.
