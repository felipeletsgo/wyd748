# TMProject source organization and refactoring

This document combines the general source split, [TMFieldScene](#tmfieldscene) and [TMHuman](#tmhuman) records.
Each record retains its own snapshot, scope and validation. Use the [architecture map](../architecture-map.md) for structure.

Status: complete. File organization, data tables, method decomposition, test
split and include pruning are done and `CLIENT-TESTED` (visual acceptance).
Snapshot: 2026-10-02, HEAD `34b7dcae` plus working tree.
Scope: the large or multi-responsibility TMProject748 files found by the
2026-10-02 survey, excluding `TMFieldScene*` and `TMHuman*`, which have their
own [TMFieldScene](#tmfieldscene) and
[TMHuman](#tmhuman) sections below.

All work here is `MODERNIZACAO_COMPATIVEL`: no opcode, packet layout, resource
ID, asset, class layout or virtual table changes. It needs no native/Ghidra
research. A moved definition must be token-identical; any intentional change
of a definition is a separate, reviewed and tested step.

## Architecture rules

These rules make code easy to find and keep the split maintainable.

1. **Stable public headers.** Existing headers (`SControl.h`, `SGrid.h`,
   `Basedef.h`, `TMScene.h`, ...) keep their names and paths. When a header is
   split, it becomes an umbrella that includes the new headers, so no consumer
   include changes.
2. **One owner per file name.** A split class keeps its own name as the file
   prefix and the responsibility as the suffix, beside the original file:
   `SGridTooltip.cpp`, `TMSceneResources.cpp`, `BasedefItems.cpp`. This is the
   convention already used by `TMFieldScene*.cpp` and `TMHuman*.cpp`.
3. **Independent classes get their own files.** A file that merely collects
   unrelated classes is divided one class per file pair. A family that is
   always used together (for example a list box and its item types) stays
   together. Widget classes live in `internal/ui/controls/`.
4. **Data is not code.** Long `if`/`switch` chains that only map inputs to
   constants become constant tables next to the owner, verified against the
   original logic for every input before replacement.
5. **No empty or catch-all files.** Do not create `Utils`/`Misc`/`Common`
   files. A function shared by several owners stays with its main domain and
   is declared in the existing public header.
6. **Normal translation units.** Each new `.cpp` starts with `#include "pch.h"`,
   is registered once in `TMProject748.vcxproj` and its filters, and never
   includes another `.cpp`. Split units start with their owner's include block
   so they compile with the same declarations. Unused includes are removed
   afterwards only with object-identity proof (see [Include pruning](#include-pruning)).
7. **Shared private helpers.** Helpers used by several units of one owner move
   from an anonymous namespace to an `<Owner>Support.h` declaration with one
   definition in the owner's main unit, as `FieldScene*Support.h` already does.
8. **Tests follow ownership.** Source-contract tests read the units that own
   the definitions (`LoadUnits` in `SceneDisconnectContractTests.cpp`) or
   extract a method by its own boundaries; never by its former neighbor.
   No assertion is removed.

## Gate for every batch

- `python .agents/research/source-relocation.py --manifest .agents/research/relocations/<batch>.json`
  proves every top-level definition of the baseline files appears exactly once
  in the new files, token for token (comments, whitespace and includes ignored).
  Mutations of literals, operators, dropped or duplicated definitions are
  rejected (self-tested when the tool was introduced).
- Release `Build-Client.ps1 -NoDeploy` (ArchitectureTests, SocketReceiveTests),
  and Debug Win32 compile/link once per phase.
- For method extraction: `extract-method-tests.py` and the re-inlining check of
  each spec; for include pruning: object identity per removed include.
- `go test ./...` in `wydgo748/` when a client source that a server test reads
  moves: `internal/game/ground_item_collision_test.go` reads the ground-mask
  table, now in `BasedefTables.h`, and failed until it was pointed there.
- Repository layout validation and `git diff --check`.
- Runtime (`CLIENT-TESTED`) acceptance stays pending until the built client is
  exercised; a passing gate is `STATICALLY VERIFIED` and `AUTOMATED TESTED` only.

## Where code lives now

| Owner (former size) | Units after the split | Responsibility per unit |
| --- | --- | --- |
| Widget library: `ui/SControl.h/.cpp` (696 + 3,250) | `ui/controls/<Class>.h/.cpp` for `SControlBase`, `SPanel`, `S3DObj`, `SCursor`, `SText`, `SEditableText`, `SButton`, `SButtonBox`, `SCheckBox`, `SProgressBar`, `SScrollBar`, `SListBox` (with its item types), `SMessageBox`, `SMessagePanel`, `SReelPanel` | One control per file pair; `SControlBase` holds `SControl`, `CONTROL_TYPE` and the render-list helpers. `ui/SControl.h` is the umbrella; `ui/SControl.cpp` was emptied and removed. |
| `ui/SGrid.cpp` (5,701) | `SGrid.cpp`, `SGridInput.cpp`, `SGridCommerce.cpp`, `SGridTooltip.cpp`, `SGridItem.cpp`, `SGridSupport.h` | Grid model and drawing; mouse/keyboard/right-click; buy/sell/trade/swap; item tooltip and capsule info; `SGridControlItem`; shared helpers |
| `core/Basedef.cpp` (3,432) | `Basedef.cpp`, `BasedefItems.cpp`, `BasedefAppearance.cpp`, `BasedefCombat.cpp`, `BasedefWorld.cpp`, `BasedefText.cpp`, `BasedefSystem.cpp` | Global tables and loaders; item/equipment/effect rules; meshes and colors; skills, damage and mob abilities; routes, distances, map attributes and projections; string validation; OS, HTTP and screen helpers. `Basedef.h` declares all of them, as before. |
| `app/scenes/TMScene.cpp` (3,049) | `TMScene.cpp`, `TMSceneResources.cpp`, `TMSceneGround.cpp` | Scene lifecycle, input and packet dispatch, camera; UI resource and message-text loading; terrain attach, queries and warps |
| `app/scenes/NewApp.cpp` (1,442) | `NewApp.cpp`, `NewAppWindow.cpp`, `NewAppServices.cpp` | Application lifecycle and main loop; window message handling; web, help and server-list services |
| `app/scenes/TMSelectCharScene.cpp` (2,077) | `TMSelectCharScene.cpp`, `TMSelectCharSceneInput.cpp`, `TMSelectCharScenePackets.cpp` | Scene setup and character list; control, keyboard and mouse input; packet handlers |
| `platform/windows/RenderDevice.cpp` (3,774) | `RenderDevice.cpp`, `RenderDeviceState.cpp`, `RenderDeviceShaders.cpp`, `RenderDevicePrimitives.cpp`, `RenderDeviceDiagnostics.cpp` | Device lifecycle and display modes; render, sampler and transform state; vertex declarations and shaders; 2D rectangles and UI geometry; state logging |
| `render/world/terrain/TMGround.cpp` (4,504) | `TMGround.cpp`, `TMGroundData.cpp`, `TMGroundRender.cpp`, `TMGroundQueries.cpp`, `TMGroundAttach.cpp`, `TMGroundAttachTable.h` | Lifecycle and tile-map loading; static coordinate and checksum tables; rendering; height/mask/color/water queries; neighbor attachment; per-cell attachment flags |
| `core/Basedef.h` (3,322) | `Basedef.h`, `BasedefLimits.h`, `BasedefStructs.h`, `BasedefPackets.h`, `BasedefTables.h` | Chapters in original order, each including the previous one: named colors, limits, opcodes and base parameter packets; game structures; wire packets with their layout checks; constant tables. `Basedef.h` includes them and declares the `BASE_*` functions. No `#pragma pack` is involved. |
| `render/mesh/TMSkinMesh.cpp` (5,823) | `TMSkinMesh.cpp`, `TMSkinMeshCostumes.cpp`, `FallbackCostumeTable.h` | Mesh lifecycle, animation and rendering; costume and mantle selection; costume meshes for types without a native renderer |

Kept as they are, with the reason:

- `app/scenes/TMSelectServerScene.cpp` (1,603): one responsibility. Its size
  came from `OnControlEvent`, reduced from 431 to 75 lines in phase 3; splitting
  the file would only add a support header for the launcher helper.
- `core/ResourceControl.h` (2,203): control ID constants only.
- `ObjectManager.cpp`, `TextureManager.cpp`, `MrItemMix.cpp`: medium files with
  small or medium functions.

## Data that was code

| Former code | Now | Equivalence evidence |
| --- | --- | --- |
| `TMGround::SetAttatchEnable`: 688-line branch chain | 97-row `ground_attach::kRules` in `TMGroundAttachTable.h` | `GroundAttachTableTests.cpp` compares the table with a literal copy of the chain over every cell from -64 to 320 on both axes |
| `RenderDevice::InitVertexShader`: 101 vertex elements assigned field by field | 23 braced `D3DVERTEXELEMENT9` tables | `vertex-decl-tables.py --check --revision 71c4b313` compares every element value with the baseline assignments |
| `TMSkinMesh::MantleException`: 30-entry `strcmp` chain | `kMantleExceptionTextures` list in `TMSkinMeshCostumes.cpp` | The same 30 names, extracted from the baseline by the applying script and pinned as a reviewed change |
| `TMSkinMesh::SetCostume`: 4,460-line branch chain | 153-row `fallback_costume::kCostumes` plus `Apply` in `FallbackCostumeTable.h` | `fallback-costume-table.mjs --check` evaluates the baseline chain for types -64..1024 and parts -16..16 and requires the header to match; `FallbackCostumeTableTests.cpp` pins the selection rule. Types outside the table still delegate to `SetOldCostume`. |

Each replaced function is pinned as a reviewed change in its relocation
manifest; any later edit to those bodies fails the gate until reviewed.
The data conversions remove no costume, cell or vertex element.

## Method decomposition (phase 3)

`.agents/research/extract-method.py` moves a statement block or a contiguous
statement range of one method into a private helper and declares it in the
class header. Each extraction is verified before writing: re-inlining every
call must reproduce the original method token for token, and each helper body
must equal the moved text. Safety rules: every outer local or parameter the
code uses is passed by reference under its own name; a name declared anywhere
else in the method that the code uses without receiving it is rejected, so it
cannot silently bind to a member or global; a range may not declare a name
used after it; lambdas and `goto` are rejected.

Control flow that leaves the code has two verified forms:

- **Returning block.** A block whose last top-level statement is a `return`
  becomes `return Helper(...);` with the method's return type.
- **Flow mode** (`"flow": true`). The helper returns `ExtractedFlow`
  (`internal/core/ExtractedFlow.h`: `Next`, `Continue`, `Break`, `Return`).
  An escaping `return X;` becomes `{ extractedResult = X; return
  ExtractedFlow::Return; }` and an escaping `continue`/`break` becomes
  `return ExtractedFlow::Continue;`/`Break;`. The call site maps each code back
  to the same jump. Verification applies the inverse transform and also checks
  that every rewritten jump sits exactly where an escaping jump stood. Only
  scalar return types are allowed.

A block anchor may be the first line of a condition that continues on later
lines; the block starts after the parenthesis that closes it. A helper can be
decomposed again: `pin-extraction.py` then replaces the helper's pinned digest,
because it is not in the baseline. `extract-method-tests.py` covers these
rules (18 tests).
`pin-extraction.py` records each verified extraction in the owner's relocation
manifest. Specs live in `.agents/research/relocations/method-*.json`.

Helpers marked * use flow mode.

| Method | Lines before -> after | Helpers |
| --- | --- | --- |
| `SGridControl::MouseOver` | 1,668 -> 556 | `MouseOverWithoutHandCursor`, `DescribeItem3443`*, `DescribeItem3444`*, `DescribeItem4147`, `DescribeOutsideShop`, `DescribeItemName`, `DescribeSkillItem`, `DescribeTimedItem`, `DescribeGeneralItem` |
| `SGridControl::TradeItem` | 570 -> 82 | one `TradeItemOn<GridType>` handler per grid type (7) |
| `SGridControl::RButton` | 450 -> 306 | `RButtonUseItemType11Or13`*, `RButtonUseItems3468To3471`, `RButtonUseItem3467` |
| `SGridControl::SellItem` | 378 -> 155 | `SellItemOnShop`*, `SellItemOnSkillBelt`*, `SellItemOnOtherGrid`* |
| `SGridControl::OnMouseEvent` | 384 -> 217 | `OnLeftButtonDown`*, `OnLeftButtonUp`*, `OnShiftLeftButtonDown`* |
| `TMScene::ReadRCBin` | 573 -> 111 | one `ReadRC<Control>`* reader per control type (9) |
| `TMScene::OnPacketEvent` | 486 -> 146 | `OnShoutMessage`*, `OnMessagePanelPacket` (309 -> 126 lines: `OnIndexedSceneMessage`, `ShowFireworkMessage`*, `ShowSmsMessage`, `AppendMessagePanelToChat`) |
| `NewApp::MsgProc` | 508 -> 313 | `OnInputLanguageChange`, `OnKeyDownMessage`*, `OnNetworkMessage`*, `OnCloseMessage` |
| `TMSelectServerScene::OnControlEvent` | 431 -> 75 | `OnServerGroupList`*, `OnServerSelectOk`, `OnLoginOk`* |
| `TMSelectCharScene::OnMouseEvent` | 488 -> 182 | `OnMouseEventWhileSelecting` |
| `RenderDevice::RenderGeomRectImage` | 702 -> 45 | `RenderTexturedGeomRect`*, `RenderGuildMarkImage` |
| `RenderDevice::RenderRectProgress2` | 212 -> 7 | `RenderProgressFill` |
| `TMGround::Render` | 585 -> 287 | `BuildVoodooTileVertices`*, `SetTileTextureCoordinates`*, `BuildTileVertices` |

Contract tests that read an extracted body now read the helper with
`source_contract::Method`, including the flow-mode early-return form.

## Tests

`tests/ReceivedPacketDispatchTests.cpp` (3,000 lines in one function) is an
orchestrator. Its cases live in ten units grouped by packet domain,
`tests/ReceivedPacketDispatch<Domain>Tests.cpp` (`Selection`, `GroundItems`,
`WorldState`, `EntitiesAndShop`, `Trade`, `ActionsAndInventory`, `Features`,
`MessagesAndServerList`, `SessionAndParty`, `RequestsAndEvents`), sharing
`tests/ReceivedPacketDispatchTestSupport.h`. The split was verified token for
token, and the text was then translated to English without changing code
tokens. Two fixture strings were replaced by English strings of the same length.

## Include pruning

`.agents/research/include-prune.py` replays MSBuild's recorded Debug|Win32
compiler options for each split unit (private precompiled header, `/Brepro`,
no debug information or `/JMC`). It removes an include only when the object
file stays byte-for-byte identical, both with the line blanked and with it
deleted. It never removes the header that declares what the unit defines,
even when another include brings it in. The run over the 49 split units
removed 182 includes: mostly headers copied into every piece of a split (for
example `Basedef.h`, `TMFieldScene.h` and `EventTranslator.h` in widget units,
or terrain effect headers in `TMGroundData.cpp`) and duplicates already
supplied by the unit's own header. Run it after a Debug|Win32 build:
`python .agents/research/include-prune.py --list <units.txt> [--write]`.

## Source encoding

The client is compiled without `/utf-8`, so non-ASCII bytes in literals depend
on the file encoding and the build machine's code page.
`.agents/research/literal-escape.py` rewrote every such literal (in
`NewAppWindow.cpp`, `TMScene.cpp`, `Basedef.cpp`, `DirShow.cpp` and
`SEditableText.cpp`) as octal escapes holding the bytes compiled today. It also
converted the CP949 comments of `ui/ItemEffect.h` to UTF-8 with BOM.
`--against HEAD` compiles the 30 affected units with the old and the new files
and requires identical objects; it passed. The multi-byte `'•'` mask
literal became the single byte MSVC already kept, which removes C4305/C4309.

Not changed, because the native 7.48 value is unverified (a Ghidra question,
not an encoding one): the password-mask byte, the IME comparison text and the
billing label. `wydgo748/data/catalog/ItemEffect.h` lost 735 comment characters before
this work (U+FFFD); it differs from the client copy, so the comments cannot be
restored mechanically.

## Remaining work

1. Blocks under about 40 lines stay inline, where a helper would not make the
   code easier to find.
2. `TMSkinMesh::SetOldCostume` (mixed tables and string edits) and
   `God2Exception` (a boolean expression) stay as code.

## Progress and validation

| Batch | Result |
| --- | --- |
| Tools | `source-relocation.py` (mutation-tested), `source-split.py`, `vcxproj-register.py`, `vertex-decl-tables.py`, `fallback-costume-table.mjs` |
| Widget library, SGrid, Basedef | Relocation gates PASS (235, 66, 111 definitions); Release builds and tests PASS |
| TMScene, NewApp, TMSelectCharScene, RenderDevice, TMGround, TMSkinMesh | Relocation gates PASS (57, 25, 20, 60, 24, 25 definitions); Release build and tests PASS after one test was moved from neighbor-delimited extraction to `source_contract::Method` |
| Data tables | Attachment table: 152,881 oracle checks PASS; vertex declarations: 101 elements PASS; costume table: 153 types PASS |
| Basedef.h chapters | Relocation gate PASS (536 definitions); Release build and tests PASS |
| Method decomposition | 54 helpers in 13 methods (27 in flow mode), each verified by re-inlining and pinned; contract tests read helper bodies with `source_contract::Method` |
| Tests | `ReceivedPacketDispatchTests` split into 10 domain units and translated; bodies verified token for token |
| Include pruning | 182 includes removed from 49 units, each object byte-identical; relocation gates, Release build and tests, and Debug build PASS |

Runtime acceptance (`CLIENT-TESTED`), 2026-10-02: the Release candidate
(SHA-256 `D567A2B2E0636FEF470FB7CF122567808C78FD757BCC054360CDD35D550BDE17`)
was installed as `client748/project.exe`. It ran against the native 7.48 server
with a local test account, and the login scene rendered with its UI controls
and cursor. The project owner then played it and reported everything visually
correct. This is a visual acceptance; no per-flow checklist was recorded. A
regression found later in one of the moved flows (UI controls, inventory and
shop grids, scene loading and terrain, the window loop, server and character
selection, rendering, terrain attachment, costume appearance) should be traced
through the relocation manifests to its original method.

<a id="tmfieldscene"></a>
## TMFieldScene refactoring record

The dates, completion claims and validation results below apply only to this recorded refactoring scope.

<a id="tmfieldscene-tmfieldscene-refactoring-and-architecture-plan"></a>

Status: Stage 1 source separation complete. Stage 2 batches 9-13 (chat filters,
skill requests, attack visual damage, attacker and target state, chat submission
and coin input) implemented and automatically validated. Stage 3 method
decomposition of the six largest methods is implemented and automatically
validated; built-client acceptance of Stage 3 is pending.
Snapshot: 2026-10-02, HEAD `34b7dcae` plus the working tree; the relocation
gate still compares against the fixed baseline `77e8dcc7f1af929be1c6418d08b97c7dd8412713`.
This record describes source changes, not a deployed candidate, runtime
acceptance or global release approval.

<a id="tmfieldscene-implementation-progress"></a>
### Implementation progress

Batches 0-8 are implemented: all 293 complete member definitions have one owner
across 18 translation units. The core retains construction, destruction, static
data and explicit routing; it now has 9,545 lines rather than 31,280. Its include
block was reviewed against actual dependencies. This is source organization,
not a runtime performance improvement. Tests load complete methods from their
actual owning units without using neighboring definitions as delimiters. No
behavioral assertion was removed.

Batch 9 extracts the native chat-filter branch of `OnControlEvent` into a pure
policy and a borrowed-control adapter. The gate preserves 292 member bodies
exactly at the token/literal level and pins the sole reviewed dispatcher change
and both new headers. The scene declaration/layout, static data, all 11 helper
bodies, packet bytes, resource IDs and packet dispatch remain unchanged.

ArchitectureTests passes 128,112 checks and compile-time assertions, including
37 chat source-contract checks and 1,171 executable chat-policy/adapter checks.
SocketReceiveTests passes 221 checks. The relocation gate has 17 regression
tests, including deliberately rejected mutations of bodies, policy, layout,
helpers, declarations, ownership, PCH and project/filter registrations. Release
and Debug Win32 compile/link passed; the integrated Release build used
`-NoDeploy`. Shared trade, inventory and minimap helpers retain one implementation
and narrow scene-internal declarations. No UI/entity ownership moved. TMHuman's
relocation/policy gate and its 11 regression tests passed before this continuation;
its installed-runtime gate remains pending.

Batches 10-13 continue Stage 2 with pure policies under `internal/application/`
and `internal/wire/`, each called at the original position by its existing
owner. The gate now pins four reviewed member bodies (`OnControlEvent`,
`SkillUse`, `AutoSkillUse`, `OnPacketAttack`), one reviewed helper
(`GetWYD748AttackVisualDamage`) and nine policy headers; the other 289 bodies
and 10 helpers remain token-identical to the baseline. See
[Implemented Stage 2 contracts](#tmfieldscene-implemented-stage-2-contracts-batches-10-13).
ArchitectureTests now passes 974,712 checks; the growth is exhaustive oracle
sweeps, not new runtime coverage.

The remaining decomposition (attack visual dispatch, the other `OnControlEvent`
domains, `FrameMove`, `SkillUse`/`AutoSkillUse` sequences and `InitializeScene`)
needs new private member helpers or narrowly owned components. That changes the
pinned scene header and therefore requires the separate review described in
[Remaining work](#tmfieldscene-remaining-work-after-stage-2-historical).
No runtime executable was replaced and no game process was launched or stopped.
All runtime flows listed below remain pending; these results are not
`CLIENT_TESTED` and do not prove complete visual or gameplay parity.

<a id="tmfieldscene-objective-and-scope"></a>
### Objective and scope

Split the 31,280-line
[`TMFieldScene.cpp`](../../tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp)
into responsibility-based translation units, then reduce coupling and oversized
methods through narrowly scoped policies and application components.
Preserve the existing 7.48 client/server contract and observable behavior.

The working tree already contains the TMHuman source separation. Preserve those
changes and establish their validation status before modifying shared consumers;
their presence does not prove that their runtime validation is complete.
Follow the [TMHuman separation plan](#tmhuman) where it applies,
but do not copy its entity-oriented ownership model into a scene controller.

The initial extraction retains the class, member layout, virtual methods, and
ownership in [`TMFieldScene.h`](../../tmproject/TMProject748/internal/app/scenes/TMFieldScene.h).
A behavior-preserving implementation is `MODERNIZACAO_COMPATIVEL`.
Documentation alone changes no functional contract and needs no native research.
Reuse valid 7.48 evidence; investigate only a newly crossed or unproven boundary.

Out of scope: gameplay fixes, new opcodes, deleting 7.69-derived behavior,
reintroducing repurchase or numeric-password flows, changing resources, changing
server authority, installing Jev, replacing the runtime executable, or unrelated
TMHuman work. Record discovered defects separately from mechanical moves.

<a id="tmfieldscene-current-structural-problems"></a>
### Current structural problems

- Scene initialization, UI binding, input, requests, packet application,
  inventory ownership, combat presentation, and periodic updates share one file.
- `OnControlEvent` spans approximately 3,696 lines before the next method;
  `OnPacketAttack` spans approximately 2,396 lines. Moving either intact
  improves navigation but does not resolve internal complexity.
- `SkillUse`, `AutoSkillUse`, and `FrameMove` also contain large cross-domain
  sequences. Their branch order and timing are behavior, not formatting.
- Anonymous-namespace helpers share trade, grid, numeric-input, and UI concerns.
  They cannot be copied into every destination without reviewing ownership.
- Both native-resource compatibility and imported source paths exist. Do not
  collapse them or treat imported code as unreachable without evidence.
- Tests load the original file and sometimes use the following method as an
  extraction delimiter. A correct move can invalidate those assumptions.

Line counts and spans are navigation aids for this snapshot, not architectural
boundaries or future acceptance criteria.

<a id="tmfieldscene-architecture-two-stages"></a>
### Architecture: two stages

<a id="tmfieldscene-stage-1-separate-definitions-without-changing-the-class"></a>
#### Stage 1: separate definitions without changing the class

Keep methods as `TMFieldScene` members and compile each new `.cpp` normally.
Member definitions in another translation unit retain access to private members;
moving them does not require making fields public or adding `friend` access.
The existing introductory comment suggesting otherwise must be corrected when
the source is changed. Keeping lifecycle ownership in the scene remains valid.

Retain construction, destruction, static member definitions, top-level scene
initialization, `FrameMove`, and the central packet/input dispatchers in the core
initially. Move their domain helpers in focused batches. Keep `.cpp` files out
of headers and other `.cpp` files; do not use a unity build to hide dependencies.

Proposed files belong beside the current source under
`tmproject/TMProject748/internal/app/scenes/`. These are target names, not an
instruction to create empty files. Representative methods are not a complete
relocation manifest; each batch must resolve complete signatures and dependencies.

| Target | Responsibility and representative methods |
| --- | --- |
| `TMFieldScene.cpp` | Scene ownership, constructor/destructor, static data, `InitializeScene`, `FrameMove`, top-level dispatch and transition coordination |
| `TMFieldSceneChat.cpp` | Channel toggles, chat/whisper packet handlers, `SysMsgChat`, `InsertInChatList` |
| `TMFieldSceneUI.cpp` | Resource binding and panel positioning: `PositionCompat*`, runtime counter controls, help/quest tabs; split further only by coherent ownership |
| `TMFieldSceneInput.cpp` | Keyboard/mouse domain helpers, `OnKey*`, `OnESC`; top-level dispatch stays in the core until routing contracts are tested |
| `TMFieldSceneInventory.cpp` | Carry/cargo slot mapping, selection, grid state, inventory/cargo visibility, drop/get/use requests |
| `TMFieldSceneInventoryPackets.cpp` | Item creation/update/removal, drop/get confirmations, swap and cargo confirmation application |
| `TMFieldSceneMerchant.cpp` | Merchant/shop visibility, buy/sell requests, shop snapshots and confirmations |
| `TMFieldSceneTrade.cpp` | Player trade and AutoTrade state, offer ownership, dialog cancellation, request and confirmation helpers |
| `TMFieldSceneMix.cpp` | `GetNativeMix*`, `ResetNativeMixPacket`, staging/removal, combine wrappers and result application |
| `TMFieldSceneSkills.cpp` | `SkillUse`, `AutoSkillUse`, cooldowns, selection, belt updates and skill-specific shortcuts |
| `TMFieldSceneCombat.cpp` | Attack intentions, target selection, combat calculations and local presentation helpers |
| `TMFieldSceneCombatPackets.cpp` | `OnPacketAttack`, kill/death and related combat state application |
| `TMFieldSceneMovement.cpp` | `MobMove`, `MobMove2`, `MobStop`, `AirMove_*`, teleport/respawn prompt helpers |
| `TMFieldSceneWorld.cpp` | Entity materialization and world-item lifecycle not owned by inventory, weather, camera, portals and minimap presentation |
| `TMFieldSceneSocial.cpp` | Party/guild interactions, guild-mark helpers and related packet application |
| `TMFieldSceneEvents.cpp` | Quiz, fireworks, Toto, gamble, quest/event panels and event packet application |
| `TMFieldSceneAutomation.cpp` | `GameAuto`, CC mode, automatic target/skill helpers and their client-side timing |
| `TMFieldSceneSessionPackets.cpp` | Login/logout, server removal/migration and delayed-quit handlers; scene transitions remain coordinated by the core |

Place packet handlers with their owning domain. Do not create a new giant
`TMFieldScenePackets.cpp` containing every handler. Keep `OnPacketEvent` as the
single routing authority during extraction; preserve the base-class call,
null check, opcode aliases, compatibility branch, and return values.
Resolve ambiguous methods such as `TimeDelay`, `UpdateScoreUI`, and
`OnMsgBoxEvent` individually; leave them in the core until their dependencies
are mapped. Do not distribute branches by arbitrary line ranges.

<a id="tmfieldscene-stage-2-reduce-shared-state-and-oversized-methods"></a>
#### Stage 2: reduce shared state and oversized methods

After Stage 1 passes integration gates, select one bounded domain at a time:

1. Extract deterministic decisions into the existing `internal/application/`,
   `internal/ui/`, or `internal/wire/` layers according to responsibility.
   Reuse `FieldInteractionPolicy`, `CCModePolicy`, `SkillCooldownPolicy`,
   `MiniMapLayout`, and existing packet contracts rather than duplicating them.
2. Break `OnControlEvent` into domain-specific handlers with an explicit outcome
   such as unhandled, handled, or rejected. Preserve current consumption and
   precedence; a rejected action must not fall through to an unrelated domain.
3. Separate skill target/request decisions from effect creation and UI updates.
   The server still decides validity, consumption, damage, and authoritative state.
4. Separate attack-frame validation, target resolution, state application, and
   visual dispatch while preserving order, null guards, death transitions, and
   native timing. Do not rewrite the wire format during this work.
5. Introduce domain components only when their state, reset behavior, ownership,
   and callers can be defined explicitly. Preserve existing ABI assumptions;
   class-layout changes require a separate review and stronger gates.

New components should receive narrow references or inputs, not a global service
locator or an unrestricted pointer to all scene fields. Avoid a generic
`FieldSceneContext` that merely reproduces the original class. UI controls and
entities remain owned by their established containers until an explicit
ownership migration defines creation, teardown, and invalidation.

<a id="tmfieldscene-first-implementation-batch-chat"></a>
### First implementation batch: chat

Move these complete member definitions into `TMFieldSceneChat.cpp`:

- `SetWhisper`, `SetPartyChat`, `SetGuildChat`, `SetKingDomChat`;
- `OnPacketMessageChat`, `OnPacketMessageChat_Index`,
  `OnPacketMessageChat_Param`, `OnPacketMessageWhisper`;
- `SysMsgChat` and `InsertInChatList`.

Keep chat-related branches of `OnControlEvent`, `OnCharEvent`, and
`OnPacketEvent` in place for this pilot. Leave `GetTimeString` in the core until
all callers establish whether it is chat-owned or shared. Preserve text bytes,
channel flags, resource IDs, timestamps, focus behavior, bounded packet writes,
null checks, list insertion, and return values. The current no-op parameterized
chat handler remains a no-op; extraction is not authorization to implement it.

Before moving `SetWhisper`, adapt the AutoTrade-close test that currently uses
the following `SetWhisper` definition as a delimiter. Otherwise this unrelated
test can break or silently inspect an incorrect source range.
Add focused checks for unique definitions, preserved channel selection logic,
absent chat/party controls, and unchanged chat/whisper dispatch routes.
Build the affected target and run the relevant architecture tests. Client
validation must later exercise channel toggles, whisper reception, chat focus,
and scene exit/re-entry; static source checks cannot prove those flows.

<a id="tmfieldscene-execution-sequence-and-gates"></a>
### Execution sequence and gates

| Batch | Scope | Required exit condition |
| --- | --- | --- |
| 0 | Baseline and relocation manifest | Record source/HEAD, complete signatures, helper references, consumers, existing failures and TMHuman dependency status; adapt method extraction tests |
| 1 | Chat pilot | One definition per moved method, unchanged routing, focused tests and incremental client build |
| 2 | Inventory/cargo and inventory packets | Preserve slot mapping, item ownership and confirmation order; test invalid slots, rejected requests, replacement and selection invalidation |
| 3 | Merchant, trade, AutoTrade and mixing, in separate slices | Preserve offer/cursor aliases, dialog cancellation, pending requests, timeout/reset behavior and server-owned results |
| 4 | UI helpers and input helpers | Preserve control IDs, focus, event consumption, panel interactions, compatibility paths and input precedence |
| 5 | Skills and combat intentions | Preserve target selection, cooldowns, attack timing, request bytes and no-action/death guards |
| 6 | Combat packet application | Preserve frame validation, entity lookup, damage/effect order and death lifecycle; targeted packet and source-contract tests |
| 7 | Movement, world, social, events and automation, in separate slices | Preserve route cancellation reasons, effects, timers, resource bindings and cleanup; validate each affected flow |
| 8 | Session handlers and core reduction | Preserve logout/relogin, disconnect, migration and teardown order; integrated client build and transition regression gates |
| 9 | Stage 2 domain extraction | A bounded component/policy with explicit contracts, tests and reviewed lifecycle; no bulk rewrite |

Each row can contain several small patches; it is not a requirement to move an
entire domain in one edit. If the next batch depends on an unvalidated shared
change, close that dependency first. Do not rerun unaffected suites or native
research for every mechanical move.

<a id="tmfieldscene-dependency-and-ownership-safeguards"></a>
### Dependency and ownership safeguards

- Register each new translation unit in `TMProject748.vcxproj` and its filters.
  Keep `pch.h` first and add actual includes instead of copying the original
  dependency block to every file.
- Track anonymous-namespace helpers such as `WYD748_ResetTradeOffer`,
  `WYD748_CancelAutoTradePurchase`, `WYD748_ReleaseAutoTradeItem`,
  `WYD748_AddOwnedGridItem`, and `WYD748_ParseDecimal` by all callers.
  Move single-domain helpers with their domain. Shared helpers need a narrow
  internal declaration and one implementation, or a justified pure policy.
  Do not publish UI ownership operations as generic utilities.
- Move `GetWYD748AttackVisualDamage` with its complete consumer set, not merely
  with a nearby packet method. Keep static member definitions unique.
- Preserve borrowed packet-buffer lifetime; handlers must not retain pointers
  after dispatch. Preserve entity/control ownership, delayed deletion, global
  aliases, request buffers, and reset ordering on disconnect or replacement.
- Preserve compatibility guards and optional controls. Do not instantiate
  controls absent from the native resources as an extraction workaround.
- Translate explanatory legacy engineering prose in changed source files as
  required by the English prose rule. Review contract-bearing text separately; do not
  rename persisted values, resource IDs, opcodes, or established identifiers
  merely to make a mechanical diff look cleaner.

<a id="tmfieldscene-test-migration-and-validation"></a>
### Test migration and validation

[`SceneDisconnectContractTests.cpp`](../../tmproject/TMProject748/tests/SceneDisconnectContractTests.cpp)
and [`LoginCredentialContractTests.cpp`](../../tmproject/TMProject748/tests/LoginCredentialContractTests.cpp)
load the monolith directly. Inventory their assertions before moving any
referenced definition. The working tree contains
[`SourceMethod.h`](../../tmproject/TMProject748/tests/SourceMethod.h), which can
extract a complete method without using its neighbor as a delimiter. Validate
it for the signatures being moved rather than assuming it handles every case.

Tests must load the actual owning translation unit and fail on missing or
duplicate definitions. For cross-domain ordering, distinguish call order in
the dispatcher from local order inside the called method; concatenating files
must not invent an execution order. Preserve all existing behavioral assertions
and update project registrations for additional focused test files.

Use focused architecture/packet tests and incremental affected-target builds
per slice. Run an integrated executable build once per coherent C++ batch:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1 -Configuration Release -NoDeploy
git diff --check
```

`-NoDeploy` does not overwrite `tmproject/client748/project.exe`. Do not deploy,
launch, or stop the game as an incidental refactoring step. Coordinate an
authorized runtime gate when the environment permits it. If a contract change
becomes necessary, stop treating that slice as mechanical extraction and define
client/server tests, rejections and integration coverage separately.

Report independently:

- `STATICALLY VERIFIED`: signatures, ownership, registrations and source checks;
- `AUTOMATED TESTED`: named executed tests and affected builds;
- `CLIENT-TESTED`: actual execution of the relevant built-client flows.

Pending runtime coverage includes world entry/exit, logout/relogin, disconnect,
resource-compatible HUD, inventory/cargo, shop/trade/mix confirmations, movement,
combat/death/respawn, chat and domain-specific events. Execute only the relevant
subset per batch and an integrated smoke flow before declaring the complete
refactor behavior-preserving. A passing build is not `CLIENT_TESTED` evidence.

<a id="tmfieldscene-completion-and-context-efficiency"></a>
### Completion and context efficiency

Stage 1 is complete when the relocation manifest has one owner per definition,
the core retains explicit orchestration, all new units are registered, tests
follow ownership rather than adjacency, and the applicable gates are recorded.
Stage 2 is complete per extracted domain, not when a file-count target is met.

Use module boundaries to load only the current domain, its header, callers,
contract and focused tests. Update this document when decisions or batch status
change; do not create a handoff per patch. Track large-method reduction and
cross-domain dependencies, not just total lines or number of files. Smaller
files can reduce unnecessary context reads, but do not establish a token-saving
percentage, runtime speedup, or lower total source volume by themselves.

For implementation, start with Batch 0 and the chat pilot. Do not combine the
first move with gameplay fixes or publish generated executables. Commits and
pushes require a separate user request.

<a id="tmfieldscene-implemented-stage-2-contract-native-chat-filters"></a>
### Implemented Stage 2 contract: native chat filters

[FieldChatControlPolicy.h](../../tmproject/TMProject748/internal/application/FieldChatControlPolicy.h)
routes the eight existing primary/state control IDs to General, Party, Whisper
or Guild only in native-HUD compatibility mode. Its decision takes a channel,
primary-control presence and selection value; it returns an outcome and the
requested enabled state. Selection zero enables the channel; any nonzero
selection disables it, matching the original branch.

[FieldChatControl.h](../../tmproject/TMProject748/internal/ui/FieldChatControl.h)
receives four borrowed primary/state button pairs and a synchronous channel
callback. It neither receives a scene pointer nor allocates, stores or deletes
controls. General updates the primary and optional state button directly.
Party, Whisper and Guild invoke the existing scene setters, which retain their
flag and optional-control behavior. Primary updates precede optional state
updates; these operations execute at the original dispatcher position.

- Imported-HUD or unrelated IDs return `Unhandled` with no mutation.
- A native-domain ID without its primary control returns `Rejected`.
- A supported native ID with its primary returns `Handled`; its paired state
  control remains optional. Both handled and rejected outcomes are consumed by
  the dispatcher with its original return value of zero.
- The adapter has no persistent state, reset or teardown. The scene remains
  responsible for controls, flags, initialization and destruction. Borrowed
  references and the callback expire on return. Packet generation and reception
  are outside this adapter.

Executable tests sweep every ID, both HUD modes, absent/present primary and
state controls, and negative/zero/positive selection values. They check callback
identity, enabled state, update order, mutations and rejection consumption.
Source-contract checks pin dispatcher precedence and the existing channel
setters; they also exercise pointer/const, compound-return, multiline,
constructor-initializer and destructor signatures in the actual owning files.
Runtime acceptance still needs native channel toggles, missing-resource behavior,
whisper reception, focus and exit/re-entry; fake controls do not prove rendering.

<a id="tmfieldscene-implemented-stage-2-contracts-batches-10-13"></a>
### Implemented Stage 2 contracts (batches 10-13)

All are `MODERNIZACAO_COMPATIVEL`: no packet layout, opcode, resource ID, scene
declaration or ownership changed, so no native/Ghidra or Go gate applies. Each
policy is header-only, has no scene/entity/control pointer and is pinned by
fingerprint. Every executable test compares the policy with an independent
literal copy of the replaced statements; source-contract checks pin call order.

| Batch | Policy | Replaced decision | Executable coverage |
| --- | --- | --- | --- |
| 10a | [`SkillRequestPolicy.h`](../../tmproject/TMProject748/internal/application/SkillRequestPolicy.h), [`SkillAttackRequest.h`](../../tmproject/TMProject748/internal/wire/SkillAttackRequest.h) | Area/primary-target routing, range checks, area radius/limits and `MSG_Attack` preparation/envelope selection in `SkillUse` and `AutoSkillUse` | `SkillAttackRequestTests.cpp` |
| 10b | [`AttackVisualDamage.h`](../../tmproject/TMProject748/internal/wire/AttackVisualDamage.h) | Optional `DMGX` wide-damage projection used only for damage text | `AttackVisualDamageTests.cpp` |
| 10c | [`AttackAttackerState.h`](../../tmproject/TMProject748/internal/application/AttackAttackerState.h) | Swing force/start time, attacker mana application with local snapshot callback, visual-dispatch gate in `OnPacketAttack` | `AttackAttackerStateTests.cpp`: all mesh/class/weapon precedences, present/absent swings, every flag/skill/motion combination, actor-before-local order |
| 11 | [`FieldChatSubmitPolicy.h`](../../tmproject/TMProject748/internal/application/FieldChatSubmitPolicy.h) | Chat-edit flood window, chat/whisper recall lists, local commands, prefix routing, relocation aliases (before truncation), command routing (after truncation), server keywords, handover level and progress gates | `FieldChatSubmitPolicyTests.cpp`: unsigned wrap, all 255x256 prefix pairs, literal/table collisions |
| 12 | [`FieldCoinInputPolicy.h`](../../tmproject/TMProject748/internal/application/FieldCoinInputPolicy.h) | `B_IG_OK` non-digit scan and `%` rewrite, rejection order for every prompt mode, `all` amount source | `FieldCoinInputPolicyTests.cpp`: every mode, boundary values, and an identical count of carried-coin reads |
| 13 | [`AttackTargetState.h`](../../tmproject/TMProject748/internal/application/AttackTargetState.h) | Target HP projection for healing/damage on regular and big pools in both receive loops | `AttackTargetStateTests.cpp`: unsigned/short boundary values, negative rates, zero-rate normalization |

Behavior that is preserved deliberately and recorded as separate defects, not
fixed here:

- Target `CurHP` is unsigned: the scene's `CurHP < 0` clamps never fire, an
  over-subtraction in the second receive loop wraps, and big-pool HP passes
  through `short`. Fixing this changes visible HP and needs a separate review.
- Whisper preparation copies the untruncated typed name into the 16-byte
  `MobName` and 32-byte `m_cWhisperName` before truncation; long names can
  overflow those buffers. Command names of sixteen or more bytes are cut to
  fourteen, not fifteen.
- The coin prompt's carried-coin read dereferences `g_pObjectManager` without
  the null guard used later in the same block; the policy reads it lazily at
  the original point so behavior and failure mode are unchanged.

Runtime acceptance for these batches still requires chat submission, whisper,
relocation and guild commands; every coin/price/server-war prompt; and attack,
healing, death and big-pool targets in the built client.

<a id="tmfieldscene-stage-3-method-decomposition-header-review-decided"></a>
### Stage 3: method decomposition (header review decided)

The header review that closed Stage 2 was decided on 2026-10-02: private,
non-virtual helper declarations may be added to `TMFieldScene.h`. They do not
change the object layout or the virtual table. Each added header line, each
extracted helper and each before/after method digest is pinned in
`.agents/research/relocations/tmfieldscene-extractions.json` by
`tmfieldscene-separation.py --pin <spec>`. `--with-policies` accepts only those
pins and requires the header to equal the baseline once the pinned lines are
removed. Extractions use `extract-method.py` (re-inlining verification and
`ExtractedFlow` for early returns). Specs:
`.agents/research/relocations/method-fieldscene-*.json`.

| Method | Lines before -> after | Helpers |
| --- | --- | --- |
| `OnPacketAttack` | 2,319 -> 162 | `DispatchAttackSkillVisuals`, `ApplyTargetedAttackHits`, `ApplyGroundAttackHits` |
| `OnControlEvent` | 3,554 -> 1,931 | 23 per-control handlers (`OnCompatFieldControl`, `OnChatEditSubmit`, `OnPgt*Button`, `OnSpecialMastery1..4Button`, `OnPartyListSelect`, `OnPortalButton`, ...) |
| `InitializeScene` | 2,280 -> 1,362 | `BindChatAndHudControls`, `ApplyCompactScreenLayout`, `LoadOwnedItemGrids` |
| `SkillUse` | 1,495 -> 450 | `SendAreaSkillRequest`, `MoveIntoSkillRange`, `AddPartySkillTargets` |
| `AutoSkillUse` | 1,034 -> 323 | `SendAreaAutoSkillRequest`, `ApplyAutoSkillPkRules`, `AddAutoPartySkillTargets` |
| `FrameMove` | 1,409 -> 662 | `UpdateMiniMapFrame`, `UpdateZoneMusicAndPanels`, `UpdateAffectIcons`, `UpdateLastMoveStop`, `UpdateWeatherEffects` |

Contract tests read these methods with `source_contract::WithHelpers`, which
places each helper body after its call. Their order-based assertions are
therefore unchanged.

Not extracted yet, with the reason:

1. `OnControlEvent`'s `B_IG_OK` handler (441 lines) contains a lambda and early
   returns; flow mode rejects lambdas. Its inner blocks can be extracted.
2. `FrameMove`'s compatibility block captures the local lambda
   `recallDeadPlayer`, whose type cannot be named as a parameter.
3. The `InitializeScene` panel-binding stretch declares locals used later in
   the method, so it is not a self-contained range.
4. `DispatchAttackSkillVisuals` (about 1,100 lines) and `SendAreaSkillRequest`
   (about 900 lines) are per-skill `if`/`else if` chains; their larger
   branches can be extracted next with the same tooling.

Built-client acceptance for these helpers is pending.

<a id="tmfieldscene-reviewed-changes-after-the-relocation"></a>
### Reviewed changes after the relocation

Behavior changes made after the relocation (the UI removal batches of the
[UI compatibility matrix](ui-compatibility-matrix.md)) are recorded with
`tmfieldscene-separation.py --review` in
`.agents/research/relocations/tmfieldscene-reviews.json`: for every changed,
added or removed definition, the digest it replaces and its reviewed digest,
plus the header lines removed or added. `--with-policies` applies these after
the Stage 3 extractions, so the gate still fails on any unrecorded edit and on
any later edit to a reviewed body. `tmhuman-separation.py --review` does the
same for `TMHuman` (method bodies only; its header must not change).

<a id="tmfieldscene-remaining-work-after-stage-2-historical"></a>
### Remaining work after Stage 2 (historical)

Stage 2 is complete per extracted domain. The domains above are closed at
`AUTOMATED TESTED`. The following remain, in recommended order:

1. Attack visual dispatch in `OnPacketAttack` (per-skill effect creation,
   about 1,800 lines) and the damage-text projection.
2. The other `OnControlEvent` domains: AutoTrade preparation/publication, gamble,
   Hell store, mix, CC mode and panel toggles.
3. `SkillUse`/`AutoSkillUse` effect and UI sequences after request preparation.
4. `FrameMove`, `TimeDelay` and `InitializeScene` phases.

None of these contain further self-contained decisions comparable to the
batches above; they are mostly ordered side effects on scene-owned controls,
entities and effect containers. Splitting them requires private member helpers
(or narrowly owned components) declared in `TMFieldScene.h`. Adding non-virtual
member declarations does not change the object layout, but the relocation gate
pins the header byte-for-byte and the Stage 2 rules require a separate review
for declaration changes. That review was decided for Stage 3 (above).

<a id="tmfieldscene-helper-ownership-and-caller-inventory"></a>
### Helper ownership and caller inventory

The following inventory excludes commented-out legacy code. Shared declarations
are scene-internal, pinned by the relocation gate, and registered once in both
the normal project and its filters. `Guildmark_Download` retains its existing
declaration in the unchanged scene header.

| Definition | Implementation owner | Complete member caller set |
| --- | --- | --- |
| `ObservedAffectPanel` | `UI` | `InitializeCompatFieldScene` |
| `WYD748_ResetTradeOffer` | `Trade` | `InitializeScene`, `OnControlEvent`, `OnMsgBoxEvent`, `SetVisibleTrade` |
| `WYD748_LogTradeSend` | `Trade` | `OnControlEvent`, `OnMsgBoxEvent` |
| `WYD748_ParseDecimal` | `Events` | `TotoBuy`, `TotoSelect` |
| `WYD748_CancelAutoTradePurchase` | `Trade` | `OnPacketAutoTrade`, `OnPacketItemSold`, `SetVisibleAutoTrade` |
| `WYD748_ReleaseAutoTradeItem` | `Trade` | `OnControlEvent`, `OnPacketAutoTrade`, `OnPacketItemSold`, `SetVisibleAutoTrade` |
| `WYD748_AddOwnedGridItem` | `Core` | `InitializeScene` |
| `WYD748_IsUnsupportedCompatEquipSlot` | `InventoryPackets` | `OnPacketCNFDropItem`, `OnPacketSell` |
| `GetWYD748AttackVisualDamage` | `CombatPackets` | `OnPacketAttack` |
| `Guildmark_Download` | `Social` | `Guildmark_Create` |
| `SetMinimapPos` | `World` | `InitializeCompatFieldScene`, `InitializeScene` |

<a id="tmfieldscene-reproducible-acceptance-gates"></a>
### Reproducible acceptance gates

Run from the repository root. The default relocation mode remains strict and
intentionally rejects the Stage 2 dispatcher change; `--with-policies` accepts
only its pinned transformation, not arbitrary changes to that method.

```powershell
python .agents/research/tmfieldscene-separation.py --complete --with-policies
python .agents/research/tmfieldscene-separation-tests.py
pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1 -Configuration Release -NoDeploy
& 'C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe' tmproject/TMProject748/TMProject748.vcxproj /t:Build /p:Configuration=Debug /p:Platform=Win32 /p:SolutionDir=C:/Users/xereca/Documents/PROJETOS/wyd-go/tmproject/ /m /nologo /v:minimal /clp:ErrorsOnly
git diff --check
```

Results on 2026-10-02, after batch 13: relocation gate PASS (293
definitions; the "289 unchanged bodies" then printed subtracted only the
reviewed-method set and overstated preservation, see the audit's A06; the
gate now derives its counts, e.g. on 2026-10-03: 293 baseline / 289 current,
191 identical normalized bodies, 1 pinned extraction, 67 reviewed changes, 30
added, 34 removed); its 18 regression tests PASS; Release `-NoDeploy` build PASS
with ArchitectureTests 974,712 checks and SocketReceiveTests 221 checks; Debug
Win32 compile/link PASS; repository layout PASS (162 documents);
`git diff --check` PASS. The installed `project.exe` was not modified.

Release integration also passed the costume dependency audit (135 items,
129 renderers, 774 parts), four shader contracts and shader rejection cases.
Existing compiler warnings remain; passing compile/link does not mean a
warning-free build. No Go or new Ghidra gate was needed: the wire and native
boundaries are unchanged.

<a id="tmfieldscene-complete-member-ownership-manifest"></a>
### Complete member ownership manifest

These are full definition signatures, not name-only aliases. Every row has
exactly one implementation in `TMFieldScene<Owner>.cpp`; `Core` means
`TMFieldScene.cpp`. Line counts cover the current complete body and signature,
not the space until its former neighbor. They are navigation data, not success
criteria. The gate compares against the fixed HEAD baseline above.

| Complete signature | Owner | Definition lines |
| --- | --- | --- |
| `TMFieldScene::TMFieldScene() : TMScene()` | `Core` | 496 |
| `TMFieldScene::~TMFieldScene()` | `Core` | 57 |
| `int TMFieldScene::InitializeScene()` | `Core` | 2280 |
| `int TMFieldScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)` | `Core` | 3642 |
| `int TMFieldScene::OnCharEvent(char iCharCode, int lParam)` | `Core` | 173 |
| `int TMFieldScene::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)` | `Core` | 281 |
| `int TMFieldScene::OnPacketEvent(unsigned int dwCode, char* buf)` | `Core` | 203 |
| `int TMFieldScene::FrameMove(unsigned int dwServerTime)` | `Core` | 1409 |
| `int TMFieldScene::TimeDelay(unsigned int dwServerTime)` | `Core` | 285 |
| `int TMFieldScene::OnMsgBoxEvent(unsigned int idwControlID, unsigned int idwEvent, unsigned int dwServerTime)` | `Core` | 505 |
| `void TMFieldScene::GetTimeString(char* szVal, int sTime, int nTime, int i)` | `Core` | 36 |
| `void TMFieldScene::InitializeCompatCCControls()` | `Automation` | 124 |
| `void TMFieldScene::FindAuto()` | `Automation` | 14 |
| `int TMFieldScene::FindProcess(unsigned int processID)` | `Automation` | 4 |
| `void TMFieldScene::SetAutoOption(int nIndex, char* szString)` | `Automation` | 3 |
| `void TMFieldScene::SetAutoSkillNum(int nCount)` | `Automation` | 33 |
| `void TMFieldScene::SetAutoTarget()` | `Automation` | 15 |
| `int TMFieldScene::OnPacketMacroWater(stWaterScrollMacro* pStd)` | `Automation` | 6 |
| `void TMFieldScene::GameAuto()` | `Automation` | 407 |
| `int TMFieldScene::ToggleNativeCCMode(int mode)` | `Automation` | 9 |
| `void TMFieldScene::NewCCMode(bool bResetCombat, bool bCapturePosition)` | `Automation` | 159 |
| `void TMFieldScene::SetWhisper(char cOn)` | `Chat` | 14 |
| `void TMFieldScene::SetPartyChat(char cOn)` | `Chat` | 14 |
| `void TMFieldScene::SetGuildChat(char cOn)` | `Chat` | 14 |
| `void TMFieldScene::SetKingDomChat(char cOn)` | `Chat` | 8 |
| `int TMFieldScene::OnPacketMessageChat(MSG_MessageChat* pStd)` | `Chat` | 33 |
| `int TMFieldScene::OnPacketMessageChat_Index(MSG_MessageChat* pStd)` | `Chat` | 53 |
| `int TMFieldScene::OnPacketMessageChat_Param(MSG_STANDARD* pStd)` | `Chat` | 4 |
| `int TMFieldScene::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)` | `Chat` | 139 |
| `void TMFieldScene::SysMsgChat(char* str)` | `Chat` | 12 |
| `void TMFieldScene::InsertInChatList(SListBox* pChatList, STRUCT_MOB *pMobData, SEditableText* pEditChat, unsigned int dwColor, int colorId, unsigned int startId)` | `Chat` | 53 |
| `int TMFieldScene::MobAttack(unsigned int wParam, D3DXVECTOR3 vec, unsigned int dwServerTime)` | `Combat` | 447 |
| `void TMFieldScene::FrameMove_KhepraDieEffect(unsigned int dwServerTime)` | `Combat` | 32 |
| `void TMFieldScene::SetMyHumanExp(long long unExp, int nFakeExp)` | `Combat` | 48 |
| `void TMFieldScene::SetPK()` | `Combat` | 22 |
| `int TMFieldScene::GetWeaponDamage()` | `Combat` | 43 |
| `void TMFieldScene::SetPosPKRun()` | `Combat` | 4 |
| `int TMFieldScene::OnPacketAction(MSG_STANDARD* pStd)` | `CombatPackets` | 13 |
| `int TMFieldScene::OnPacketCNFMobKill(MSG_CNFMobKill* pStd)` | `CombatPackets` | 73 |
| `int TMFieldScene::OnPacketSetHpMode(MSG_SetHpMode* pStd)` | `CombatPackets` | 28 |
| `int TMFieldScene::OnPacketAttack(MSG_STANDARD* pStd)` | `CombatPackets` | 2395 |
| `int TMFieldScene::OnPacketNuke(MSG_STANDARD* pStd)` | `CombatPackets` | 5 |
| `void TMFieldScene::PGTVisible(unsigned int dwServerTime)` | `Events` | 55 |
| `void TMFieldScene::SetVisibleGamble(int bShow, char cType)` | `Events` | 62 |
| `void TMFieldScene::UpdateGambleRequestTimeout()` | `Events` | 22 |
| `void TMFieldScene::InitializeFireWorkControls()` | `Events` | 23 |
| `void TMFieldScene::UpdateFireWorkButton(int nIndex)` | `Events` | 11 |
| `void TMFieldScene::ClearFireWork()` | `Events` | 11 |
| `void TMFieldScene::UseFireWork()` | `Events` | 89 |
| `void TMFieldScene::DrawCustomFireWork(int nIndex)` | `Events` | 4 |
| `void TMFieldScene::TotoSelect()` | `Events` | 30 |
| `void TMFieldScene::TotoBuy()` | `Events` | 27 |
| `void TMFieldScene::TotoClose()` | `Events` | 14 |
| `void TMFieldScene::SetQuestStatus(bool bStart)` | `Events` | 10 |
| `void TMFieldScene::UpdateQuestTime()` | `Events` | 14 |
| `int TMFieldScene::OnPacketLongMessagePanel(MSG_LongMessagePanel* pMsg)` | `Events` | 35 |
| `int TMFieldScene::OnPacketClearMenu(MSG_STANDARD* pStd)` | `Events` | 5 |
| `int TMFieldScene::OnPacketCastleState(MSG_STANDARDPARM* pStd)` | `Events` | 13 |
| `int TMFieldScene::OnPacketStartTime(MSG_STANDARDPARM* pStd)` | `Events` | 18 |
| `int TMFieldScene::OnPacketRemainCount(MSG_STANDARDPARM* pStd)` | `Events` | 28 |
| `int TMFieldScene::OnPacketWarInfo(MSG_STANDARDPARM3* pStd)` | `Events` | 26 |
| `int TMFieldScene::OnPacketRemainNPCCount(MSG_STANDARDPARM* pStd)` | `Events` | 26 |
| `int TMFieldScene::OnPacketRESULTGAMBLE(MSG_ResultGamble* pStd)` | `Events` | 40 |
| `int TMFieldScene::OnPacketREQArray(MSG_STANDARD* pStd)` | `Events` | 29 |
| `void TMFieldScene::InitializeQuizEventControls()` | `Events` | 42 |
| `int TMFieldScene::OnPacketQuizEvent(MSG_STANDARD* packet)` | `Events` | 22 |
| `int TMFieldScene::OnPacketRandomQuiz(MSG_RandomQuiz* pStd)` | `Events` | 25 |
| `int TMFieldScene::OnPacketSendExpMsg(MSG_Exp_MsgPanel* pStd)` | `Events` | 11 |
| `int TMFieldScene::OnPacketBattle(MSG_TowerWar* pStd)` | `Events` | 6 |
| `int TMFieldScene::OnPacketInforPlay(MSG_SendInfoPlay* pStd)` | `Events` | 13 |
| `int TMFieldScene::OnPacketRunQuest12Start(MSG_STANDARDPARM* pStd)` | `Events` | 23 |
| `int TMFieldScene::OnPacketRunQuest12Count(MSG_STANDARDPARM2* pStd)` | `Events` | 15 |
| `int TMFieldScene::MouseClick_QuestNPC(unsigned int dwServerTime, TMHuman* pOver)` | `Events` | 126 |
| `int TMFieldScene::OnMouseEventCompat(unsigned int dwFlags, unsigned int wParam, int nX, int nY)` | `Input` | 99 |
| `int TMFieldScene::OnKeyDownEvent(unsigned int iKeyCode)` | `Input` | 195 |
| `int TMFieldScene::OnAccel(int nMsg)` | `Input` | 116 |
| `int TMFieldScene::MouseClick_NPC(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)` | `Input` | 190 |
| `void TMFieldScene::MouseMove(int nX, int nY)` | `Input` | 25 |
| `int TMFieldScene::MouseLButtonDown(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)` | `Input` | 5 |
| `void TMFieldScene::OnESC()` | `Input` | 202 |
| `int TMFieldScene::OnKeyDebug(char iCharCode, int lParam)` | `Input` | 5 |
| `int TMFieldScene::OnKeySkill(char iCharCode, int lParam)` | `Input` | 89 |
| `int TMFieldScene::OnKeyDash(char iCharCode, int lParam)` | `Input` | 16 |
| `int TMFieldScene::OnKeyPlus(char iCharCode, int lParam)` | `Input` | 61 |
| `int TMFieldScene::OnKeyPK(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyName(char iCharCode, int lParam)` | `Input` | 11 |
| `int TMFieldScene::OnKeyAutoTarget(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyAuto(char iCharCode, int lParam)` | `Input` | 69 |
| `int TMFieldScene::OnKeyHelp(char iCharCode, int lParam)` | `Input` | 20 |
| `int TMFieldScene::OnKeyRun(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyFeedMount(char iCharCode, int lParam)` | `Input` | 7 |
| `int TMFieldScene::OnKeyHPotion(char iCharCode, int lParam)` | `Input` | 7 |
| `int TMFieldScene::OnKeyMPotion(char iCharCode, int lParam)` | `Input` | 7 |
| `int TMFieldScene::OnKeyPPotion(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeySkillPage(char iCharCode, int lParam)` | `Input` | 21 |
| `int TMFieldScene::OnKeyQuestLog(char iCharCode, int lParam)` | `Input` | 16 |
| `int TMFieldScene::OnKeyReverse(char iCharCode, int lParam)` | `Input` | 5 |
| `int TMFieldScene::OnKeyAutoRun(char iCharCode, int lParam)` | `Input` | 24 |
| `int TMFieldScene::OnKeyGuildOnOff(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyShortSkill(char iCharCode, int lParam)` | `Input` | 146 |
| `int TMFieldScene::OnKeyVisibleSkill(char iCharCode, int lParam)` | `Input` | 20 |
| `int TMFieldScene::OnKeyCamView(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyVisibleInven(char iCharCode, int lParam)` | `Input` | 21 |
| `int TMFieldScene::OnKeyVisibleCharInfo(char iCharCode, int lParam)` | `Input` | 22 |
| `int TMFieldScene::OnKeyVisibleMinimap(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyVisibleParty(char iCharCode, int lParam)` | `Input` | 8 |
| `int TMFieldScene::OnKeyReturn(char iCharCode, int lParam)` | `Input` | 96 |
| `int TMFieldScene::OnKeyNumPad(unsigned int iKeyCode)` | `Input` | 78 |
| `int TMFieldScene::OnKeyTotoTab(char iCharCode, int lParam)` | `Input` | 20 |
| `int TMFieldScene::OnKeyTotoEnter(char iCharCode, int lParam)` | `Input` | 20 |
| `void TMFieldScene::InitializeCompatInventory()` | `Inventory` | 151 |
| `SGridControl* TMFieldScene::GetCarryGridForSlot(int slot) const` | `Inventory` | 9 |
| `SGridControl* TMFieldScene::GetCargoGridForSlot(int slot) const` | `Inventory` | 8 |
| `void TMFieldScene::GetCarryCellForSlot(int slot, int& cellX, int& cellY) const` | `Inventory` | 7 |
| `void TMFieldScene::GetCargoCellForSlot(int slot, int& cellX, int& cellY) const` | `Inventory` | 6 |
| `int TMFieldScene::GetCarrySlotForCell(const SGridControl* grid, int cellX, int cellY) const` | `Inventory` | 7 |
| `int TMFieldScene::GetCargoSlotForCell(const SGridControl* grid, int cellX, int cellY) const` | `Inventory` | 7 |
| `void TMFieldScene::DropItem(unsigned int dwServerTime)` | `Inventory` | 41 |
| `int TMFieldScene::GetItem(TMItem* pItem)` | `Inventory` | 27 |
| `void TMFieldScene::SetVisibleInventory()` | `Inventory` | 142 |
| `void TMFieldScene::SetVisibleCargo(int bShow)` | `Inventory` | 56 |
| `void TMFieldScene::SetVisibleCargo1(int bShow)` | `Inventory` | 28 |
| `void TMFieldScene::SetInVisibleInputCoin()` | `Inventory` | 44 |
| `void TMFieldScene::SetInventoryGridType(TMEGRIDTYPE gridType)` | `Inventory` | 10 |
| `void TMFieldScene::SetGridState()` | `Inventory` | 6 |
| `void TMFieldScene::SetEquipGridState(int bDefault)` | `Inventory` | 21 |
| `void TMFieldScene::UpdateMyHuman()` | `Inventory` | 26 |
| `void TMFieldScene::SetSanc()` | `Inventory` | 4 |
| `int TMFieldScene::GetItemFromGround(unsigned int dwServerTime)` | `Inventory` | 51 |
| `char TMFieldScene::UseHPotion()` | `Inventory` | 99 |
| `char TMFieldScene::UseMPotion()` | `Inventory` | 94 |
| `void TMFieldScene::UsePPotion()` | `Inventory` | 80 |
| `int TMFieldScene::IsFeedPotion(short sMountIndex, short sItemIndex)` | `Inventory` | 41 |
| `char TMFieldScene::FeedMount()` | `Inventory` | 87 |
| `void TMFieldScene::UseTicket(int nCellX, int nCellY)` | `Inventory` | 65 |
| `char TMFieldScene::UseQuickSloat(char key)` | `Inventory` | 118 |
| `void TMFieldScene::VisibleInputCharName(SGridControlItem* pItem, int nCellX, int nCellY)` | `Inventory` | 44 |
| `void TMFieldScene::UseItem(SGridControlItem* pItem, int nType, int nItemSIndex, int nCellX, int nCellY)` | `Inventory` | 89 |
| `bool TMFieldScene::SendCapsuleItem()` | `Inventory` | 72 |
| `void TMFieldScene::DropListUpdate()` | `Inventory` | 30 |
| `void TMFieldScene::ClearInventorySelectedItem()` | `Inventory` | 14 |
| `void TMFieldScene::UpdateGridDropList(int page)` | `Inventory` | 171 |
| `void TMFieldScene::Bag_View()` | `Inventory` | 113 |
| `int TMFieldScene::OnPacketUpdateCargoCoin(MSG_STANDARDPARM* pStd)` | `InventoryPackets` | 6 |
| `int TMFieldScene::OnPacketCNFDropItem(MSG_CNFDropItem* pMsg)` | `InventoryPackets` | 94 |
| `int TMFieldScene::OnPacketCNFGetItem(MSG_CNFGetItem* pMsg)` | `InventoryPackets` | 71 |
| `int TMFieldScene::OnPacketUpdateItem(MSG_UpdateItem* pMsg)` | `InventoryPackets` | 25 |
| `int TMFieldScene::OnPacketRemoveItem(MSG_STANDARDPARM* pStd)` | `InventoryPackets` | 8 |
| `int TMFieldScene::OnPacketSwapItem(MSG_STANDARD* pStd)` | `InventoryPackets` | 295 |
| `int TMFieldScene::OnPacketDeposit(MSG_STANDARD* pStd)` | `InventoryPackets` | 9 |
| `int TMFieldScene::OnPacketWithdraw(MSG_STANDARD* pStd)` | `InventoryPackets` | 9 |
| `int TMFieldScene::OnPacketCapsuleInfo(MSG_CAPSULEINFO* pStd)` | `InventoryPackets` | 31 |
| `int TMFieldScene::CheckMerchant(TMHuman* pOver)` | `Merchant` | 82 |
| `void TMFieldScene::SetVisibleShop(int bShow)` | `Merchant` | 85 |
| `void TMFieldScene::SetVisibleHellGateStore(int bShow)` | `Merchant` | 48 |
| `int TMFieldScene::OnPacketShopList(MSG_STANDARD* pStd)` | `Merchant` | 153 |
| `int TMFieldScene::OnPacketRMBShopList(MSG_RMBShopList* pMsg)` | `Merchant` | 156 |
| `int TMFieldScene::OnPacketBuy(MSG_STANDARD* pStd)` | `Merchant` | 51 |
| `int TMFieldScene::OnPacketSell(MSG_STANDARD* pStd)` | `Merchant` | 95 |
| `int TMFieldScene::OnPacketCloseShop(MSG_STANDARD* pStd)` | `Merchant` | 5 |
| `int TMFieldScene::OnPacketItemPrice(MSG_STANDARDPARM2* pStd)` | `Merchant` | 6 |
| `void TMFieldScene::BuyItemNewStore(int idwControlID)` | `Merchant` | 23 |
| `void TMFieldScene::UpdateNewStore(int idwControlID)` | `Merchant` | 216 |
| `int TMFieldScene::OnPacketNewCashRev(PacketRevDonate* P)` | `Merchant` | 66 |
| `int TMFieldScene::OnPacketNewBuyCash(MSG_STANDARD* pStd)` | `Merchant` | 24 |
| `int TMFieldScene::OnPacketNewCashRev2(PacketRevDonate2* pStd)` | `Merchant` | 20 |
| `void TMFieldScene::MouseClick_PremiumNPC(TMHuman* pOver)` | `Merchant` | 6 |
| `SPanel* TMFieldScene::GetNativeMixPanel(int mixIndex) const` | `Mix` | 13 |
| `SGridControl* TMFieldScene::GetNativeMixGrid(int mixIndex, int slot) const` | `Mix` | 16 |
| `MSG_CombineItem* TMFieldScene::GetNativeMixPacket(int mixIndex)` | `Mix` | 16 |
| `int TMFieldScene::GetNativeMixSlotCount(int mixIndex) const` | `Mix` | 18 |
| `void TMFieldScene::ResetNativeMixPacket(int mixIndex)` | `Mix` | 22 |
| `void TMFieldScene::ClearNativeMix(int mixIndex)` | `Mix` | 35 |
| `void TMFieldScene::DoNativeMix(int mixIndex)` | `Mix` | 108 |
| `void TMFieldScene::SetVisibleNativeMix(int mixIndex, int bShow)` | `Mix` | 55 |
| `int TMFieldScene::TryStageNativeMixItem(SGridControl* sourceGrid, SGridControlItem* sourceItem, SGridControl* preferredTarget)` | `Mix` | 71 |
| `int TMFieldScene::TryRemoveNativeMixItem(SGridControl* mixGrid)` | `Mix` | 43 |
| `void TMFieldScene::ClearCombine()` | `Mix` | 4 |
| `void TMFieldScene::ClearCombine2()` | `Mix` | 4 |
| `void TMFieldScene::ClearCombine3()` | `Mix` | 4 |
| `void TMFieldScene::ClearCombine4()` | `Mix` | 4 |
| `void TMFieldScene::ClearCombine5()` | `Mix` | 4 |
| `void TMFieldScene::ClearCombine6()` | `Mix` | 4 |
| `void TMFieldScene::DoCombine()` | `Mix` | 4 |
| `void TMFieldScene::DoCombine2()` | `Mix` | 4 |
| `void TMFieldScene::DoCombine3()` | `Mix` | 4 |
| `void TMFieldScene::DoCombine4()` | `Mix` | 4 |
| `void TMFieldScene::DoCombine5()` | `Mix` | 4 |
| `void TMFieldScene::DoCombine6()` | `Mix` | 4 |
| `void TMFieldScene::SetVisibleMixItem(int bShow)` | `Mix` | 4 |
| `void TMFieldScene::SetVisibleMixItem2(int bShow)` | `Mix` | 4 |
| `void TMFieldScene::SetVisibleMixItem3(int bShow)` | `Mix` | 4 |
| `void TMFieldScene::SetVisibleMixItemTiini(int bShow)` | `Mix` | 4 |
| `void TMFieldScene::SetVisibleMixItem5(int bShow)` | `Mix` | 4 |
| `void TMFieldScene::SetVisibleMixItem6(int bShow)` | `Mix` | 4 |
| `int TMFieldScene::OnPacketCombineComplete(MSG_STANDARD* pStd)` | `Mix` | 24 |
| `void TMFieldScene::SetVisibleMixPanel(int bShow)` | `Mix` | 43 |
| `void TMFieldScene::ClearMixPannel()` | `Mix` | 39 |
| `void TMFieldScene::SetVisibleMissionPanel(int bShow)` | `Mix` | 43 |
| `void TMFieldScene::ClearMissionPannel()` | `Mix` | 39 |
| `int TMFieldScene::MouseClick_MixNPC(TMHuman* pOver)` | `Mix` | 55 |
| `int TMFieldScene::UpdateTeleportPrompt()` | `Movement` | 146 |
| `bool TMFieldScene::OfferRespawnPrompt(bool playerAction)` | `Movement` | 18 |
| `int TMFieldScene::MobMove(D3DXVECTOR3 vec, unsigned int dwServerTime)` | `Movement` | 64 |
| `int TMFieldScene::MobMove2(TMVector2 vec, unsigned int dwServerTime)` | `Movement` | 54 |
| `void TMFieldScene::SetRunMode()` | `Movement` | 10 |
| `void TMFieldScene::MobStop(D3DXVECTOR3 vec)` | `Movement` | 91 |
| `int TMFieldScene::OnPacketReqSummon(MSG_ReqSummon* pStd)` | `Movement` | 9 |
| `int TMFieldScene::OnPacketCancelSummon(MSG_STANDARD* pStd)` | `Movement` | 5 |
| `void TMFieldScene::AirMove_Main(unsigned int dwServerTime)` | `Movement` | 149 |
| `void TMFieldScene::AirMove_Start(int nIndex)` | `Movement` | 37 |
| `void TMFieldScene::AirMove_End(AirMoveEndReason reason)` | `Movement` | 66 |
| `int TMFieldScene::AirMove_ShowUI(bool bShow)` | `Movement` | 214 |
| `int TMFieldScene::OnPacketCNFCharacterLogout(MSG_STANDARD* pStd)` | `SessionPackets` | 17 |
| `int TMFieldScene::OnPacketCNFRemoveServer(MSG_CNFRemoveServer* pStd)` | `SessionPackets` | 70 |
| `int TMFieldScene::OnPacketCNFAccountLogin(MSG_CNFAccountLogin* pStd)` | `SessionPackets` | 16 |
| `int TMFieldScene::OnPacketCNFCharacterLogin(MSG_CNFCharacterLogin* pStd)` | `SessionPackets` | 42 |
| `int TMFieldScene::OnPacketAutoKick(MSG_STANDARD* pStd)` | `SessionPackets` | 4 |
| `int TMFieldScene::OnPacketDelayQuit(MSG_SysQuit* pStd)` | `SessionPackets` | 6 |
| `void TMFieldScene::InitializeCompatSkillBelts()` | `Skills` | 50 |
| `int TMFieldScene::GetSkillDelay(int skillIndex) const` | `Skills` | 20 |
| `bool TMFieldScene::IsSkillCoolingDown(int skillIndex, unsigned int now) const` | `Skills` | 6 |
| `void TMFieldScene::UpdateSkillCooldownUI(unsigned int now)` | `Skills` | 29 |
| `int TMFieldScene::SkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)` | `Skills` | 1561 |
| `int TMFieldScene::AutoSkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)` | `Skills` | 1094 |
| `void TMFieldScene::SetVisibleSkillMaster()` | `Skills` | 87 |
| `void TMFieldScene::SetVisibleSkill()` | `Skills` | 60 |
| `void TMFieldScene::UpdateSkillBelt()` | `Skills` | 95 |
| `void TMFieldScene::IncSkillSel()` | `Skills` | 64 |
| `void TMFieldScene::SetShortSkill(int nIndex, SGridControlItem* pGridItem)` | `Skills` | 109 |
| `void TMFieldScene::SetSkillColor(TMHuman* pAttacker, char cSkillIndex)` | `Skills` | 132 |
| `void TMFieldScene::SetMyHumanMagic()` | `Skills` | 4 |
| `int TMFieldScene::OnPacketSetShortSkill(MSG_SetShortSkill* pStd)` | `Skills` | 19 |
| `int TMFieldScene::MouseClick_SkillMasterNPC(unsigned int dwServerTime, TMHuman* pOver)` | `Skills` | 38 |
| `void TMFieldScene::SetVisibleParty()` | `Social` | 12 |
| `void TMFieldScene::SetVisibleServerWar()` | `Social` | 18 |
| `void TMFieldScene::SetVisibleRefuseServerWar()` | `Social` | 18 |
| `void TMFieldScene::VisibleInputGuildName()` | `Social` | 19 |
| `int TMFieldScene::OnPacketREQParty(MSG_REQParty* pStd)` | `Social` | 84 |
| `int TMFieldScene::OnPacketAddParty(MSG_AddParty* pStd)` | `Social` | 51 |
| `int TMFieldScene::OnPacketRemoveParty(MSG_RemoveParty* pStd)` | `Social` | 72 |
| `int TMFieldScene::OnPacketReqChallange(MSG_STANDARD* pStd)` | `Social` | 13 |
| `int TMFieldScene::OnPacketGuildDisable(MSG_STANDARDPARM* pStd)` | `Social` | 7 |
| `int TMFieldScene::Guildmark_Create(stGuildMarkInfo* pMark)` | `Social` | 40 |
| `void TMFieldScene::Guildmark_MakeFileName(char* szStr, int nGuild, int nChief, int nChannel)` | `Social` | 5 |
| `int TMFieldScene::Guildmark_Find_ArrayIndex(int nGuild)` | `Social` | 10 |
| `int TMFieldScene::Guildmark_Find_EmptyArrayIndex()` | `Social` | 10 |
| `int TMFieldScene::Guildmark_DeleteIdleGuildmark()` | `Social` | 34 |
| `int TMFieldScene::Guildmark_IsCorrectBMP(char* szMarkBuffer)` | `Social` | 20 |
| `void TMFieldScene::Guildmark_Link(SPanel* pPanel, int nMarkIndex, int nGuildIndex)` | `Social` | 13 |
| `void TMFieldScene::SetVisibleTrade(int bShow)` | `Trade` | 187 |
| `void TMFieldScene::SetVisibleAutoTrade(int bShow, int bCargo)` | `Trade` | 346 |
| `void TMFieldScene::SendReqBuy(unsigned int dwControlID)` | `Trade` | 23 |
| `void TMFieldScene::VisibleInputTradeName()` | `Trade` | 28 |
| `int TMFieldScene::OnPacketItemSold(MSG_STANDARDPARM2* pStd)` | `Trade` | 39 |
| `int TMFieldScene::OnPacketAutoTrade(MSG_STANDARD* pStd)` | `Trade` | 103 |
| `void TMFieldScene::UpdateCompatScoreUI()` | `UI` | 332 |
| `void TMFieldScene::UpdateCompatLearnedSkillUI()` | `UI` | 72 |
| `void TMFieldScene::InitializeRuntimeCounterTexts()` | `UI` | 54 |
| `int TMFieldScene::InitializeCompatFieldScene()` | `UI` | 757 |
| `void TMFieldScene::PositionCompatFeaturePanels()` | `UI` | 34 |
| `void TMFieldScene::PositionCompatNativeMixPanels()` | `UI` | 28 |
| `void TMFieldScene::PositionCompatShopPanels()` | `UI` | 23 |
| `void TMFieldScene::PositionCompatTradePanels()` | `UI` | 23 |
| `void TMFieldScene::PositionCompatGamblePanel()` | `UI` | 14 |
| `void TMFieldScene::PositionCompatPartyPanel()` | `UI` | 14 |
| `void TMFieldScene::PositionCompatQuestPanel()` | `UI` | 11 |
| `void TMFieldScene::SelectQuestTab(int tabIndex)` | `UI` | 22 |
| `void TMFieldScene::SelectHelpTab(int tabIndex)` | `UI` | 130 |
| `void TMFieldScene::SetQuestPanelVisible(bool visible)` | `UI` | 40 |
| `void TMFieldScene::SetVisibleCharInfo()` | `UI` | 39 |
| `void TMFieldScene::UpdateScoreUI(unsigned int unFlag)` | `UI` | 914 |
| `void TMFieldScene::InitBoard()` | `UI` | 237 |
| `int TMFieldScene::LoadMsgText(SListBox* pListBox, char* szFileName)` | `UI` | 38 |
| `void TMFieldScene::VisibleInputPass()` | `UI` | 19 |
| `void TMFieldScene::SetButtonTextXY(SButton* pButton)` | `UI` | 33 |
| `void TMFieldScene::UpdateCompatObservedAffects()` | `UI` | 48 |
| `int TMFieldScene::Affect_Main(unsigned int dwServerTime)` | `UI` | 355 |
| `unsigned int TMFieldScene::GetLascDescParamId()` | `UI` | 10 |
| `int TMFieldScene::StrByteCheck(char* szString)` | `UI` | 19 |
| `void TMFieldScene::SetVisiblePotal(int bShow, int nPos)` | `World` | 81 |
| `void TMFieldScene::SetVisibleMiniMap()` | `World` | 252 |
| `void TMFieldScene::SetCameraView()` | `World` | 76 |
| `void TMFieldScene::InitCameraView()` | `World` | 15 |
| `void TMFieldScene::SetVisibleNameLabel()` | `World` | 9 |
| `void TMFieldScene::SetWeather(int nWeather)` | `World` | 50 |
| `void TMFieldScene::SetVisibleKhepraPortal(bool bVisible)` | `World` | 18 |
| `int TMFieldScene::OnPacketCreateMobCompat(MSG_STANDARD* pStd)` | `World` | 88 |
| `int TMFieldScene::OnPacketSoundEffect(MSG_STANDARDPARM* pStd)` | `World` | 7 |
| `int TMFieldScene::OnPacketCreateMob(MSG_STANDARD* pStd)` | `World` | 690 |
| `int TMFieldScene::OnPacketWeather(MSG_STANDARDPARM* pStd)` | `World` | 5 |
| `int TMFieldScene::OnPacketCreateItem(MSG_CreateItem* pMsg)` | `World` | 122 |
| `int TMFieldScene::OnPacketEnvEffect(MSG_STANDARD* pStd)` | `World` | 24 |

<a id="tmhuman"></a>
## TMHuman separation record

The dates, completion claims and validation results below apply only to this recorded refactoring scope.

<a id="tmhuman-tmhuman-separation-plan"></a>

Status: mechanical separation and the first specialized policy completed;
static and automated acceptance passed, visual runtime acceptance pending.
Snapshot: 2026-10-02, HEAD `34b7dcae` plus working tree (third phase); the relocation baseline remains `77e8dcc7f1af929be1c6418d08b97c7dd8412713`.
This record describes the current working-tree implementation, not a commit,
deployed candidate, global release approval, or `CLIENT_TESTED` result.

<a id="tmhuman-implementation-progress"></a>
### Implementation progress

All 128 definitions now have exactly one owner across the ten units below.
`TMHuman.cpp` decreased from approximately 18,661 to 3,391 lines; total class
storage and ownership remain unchanged. All complete bodies were mechanically
preserved before specialization. The gate derives its counts from the baseline
and current definitions (`method_counts.py`, audit A06). On 2026-10-03 it
reports 128 baseline / 131 current definitions: 109 identical normalized
bodies, 5 pinned extractions, 5 reviewed changes, 12 added and 9 removed. Later
`TMHuman.h` edits (removed dead declarations) are accepted only as recorded,
reviewed header lines; a normalized-body match is not a runtime assertion.

The movement pilot was completed, UI was separated, and `SetMotion` now belongs
to animation. Combat, appearance, mounts, rendering, effects and packets were
then extracted in focused batches with compile/link and contract-test gates.
`FrameMove`, initialization, teardown, score/affect orchestration and shared
classification remain core-owned. This is still one class, not ten resource
owners or a claim that all internal coupling has been removed.

Integrated validation: `STATICALLY VERIFIED` by the complete relocation gate,
source-consumer review, English-text review, layout and whitespace checks;
`AUTOMATED TESTED` by Release `-NoDeploy` and Debug Win32 compile/link,
ArchitectureTests (126,904 checks and static assertions), SocketReceiveTests
(221 checks), 11 relocation-tool regression tests and the existing static
skill-visual audit. No game process was launched, closed or installed.
The installed executable hash is unchanged; runtime visual execution remains
pending and is not `CLIENT_TESTED`.

<a id="tmhuman-objective-and-scope"></a>
### Objective and scope

Separate the approximately 18,661-line
[`TMHuman.cpp`](../../tmproject/TMProject748/internal/game/entities/TMHuman.cpp)
by responsibility, then extract narrowly scoped, testable policies where useful.
Improve navigation, review boundaries, and selective agent context loading
without changing the existing 7.48 client behavior or server contract.

The first phase moves definitions into independent translation units while
retaining [`TMHuman.h`](../../tmproject/TMProject748/internal/game/entities/TMHuman.h)
and the same class. The second phase reduces coupling inside selected methods.
Smaller files alone do not make the class modular, reduce total source volume,
or establish runtime performance or token savings.

This is `MODERNIZACAO_COMPATIVEL` only while observable behavior and contracts
remain unchanged. Reuse valid evidence; do not reopen native research for a
mechanical move. A discovered wire, ABI, resource, or lifecycle change must be
handled separately under the [repository rules](../../AGENTS.md).

Out of scope: gameplay fixes, removing legacy flows, replacing assets, changing
opcodes, transferring authority to the client, installing Jev, changing class
ownership, and deleting unrelated code. Do not combine those tasks with a move.

<a id="tmhuman-implemented-file-ownership"></a>
### Implemented file ownership

All proposed files belong beside the existing source in
`tmproject/TMProject748/internal/game/entities/`. All ten units now exist and
compile independently. Representative methods do not constitute the complete
relocation manifest; the full signature list is recorded below.

| Target | Responsibility | Representative existing methods |
| --- | --- | --- |
| `TMHuman.cpp` | Static data, construction, initialization, teardown, and top-level update orchestration | Constructor/destructor, `InitObject`, `Init`, `FrameMove`, device restoration/invalidation, `DelayDelete` |
| `TMHumanRender.cpp` | Base rendering and scene-dependent materials | `Render`, `SetColorMaterial` |
| `TMHumanAnimation.cpp` | Motion selection and animation progression | `SetAnimation`, `AnimationFrame`, `SetMotion` |
| `TMHumanMovement.cpp` | Position/orientation, route generation, and movement intentions | `InitPosition`, `InitAngle`, `SetAngle`, `SetPosition`, `MoveTo`, `OnlyMove`, `GetRoute`, route-table helpers |
| `TMHumanCombat.cpp` | Attack reactions, combat intentions, and death transitions | `MoveAttack`, `MoveGet`, both `Attack` and `Punched` overloads, `Fire`, `Die`, `Stand`, `MAutoAttack` |
| `TMHumanAppearance.cpp` | Equipment-driven appearance, dye inputs, weapons, and character costumes | `CheckWeapon`, `SetPacketMOBItem`, `SetPacketEquipItem`, `SetColorItem`, `SetHumanCostume`, mantle selection |
| `TMHumanMounts.cpp` | Mount appearance selection and mount updates | `SetImportedMountCostume`, `SetMountCostume`, `UpdateMount`, `GetMyHeight` |
| `TMHumanEffects.cpp` | Effect creation, rendering, and frame updates | `RenderEffect`, `FrameMoveEffect`, avatar effect methods, costume/creature effect methods |
| `TMHumanUI.cpp` | Labels, controls, chat presentation, picking, and minimap marker presentation | `LabelPosition`, `LabelPosition2`, `HideLabel`, `CreateControl`, `DestroyControl`, `SetChatMessage`, `SetInMiniMap` |
| `TMHumanPackets.cpp` | Existing packet dispatch and human-specific handlers | `OnPacketEvent`, all `OnPacket*` handlers |

Inspect callers before assigning remaining methods such as `UpdateScore`,
`CheckAffect`, and classification helpers. Keep them in the core initially
unless a coherent existing responsibility clearly owns them. One definition
must have one owner; cross-domain calls remain ordinary member calls.

Keep `FrameMove` orchestration in the core during mechanical separation.
Splitting its internal branches is a later behavioral-risk review. Similarly,
moving `FrameMoveEffect` or `SetHumanCostume` does not solve their internal size.
If the effects unit remains unwieldy, separate avatar and costume effects by
their lifecycle after the first phase, not by arbitrary line counts.

<a id="tmhuman-invariants"></a>
### Invariants

- Preserve fields, base class, virtual-method declarations, signatures,
  overloads, access control, and class layout during the mechanical phase.
- Preserve numeric constants, resource IDs, packet layouts, branch order,
  timing, state mutations, global interactions, and effect ownership.
- Preserve delayed-deletion guards, null checks, scene-owner checks, and
  initialization/cleanup ordering, including logout and reconnect paths.
- Define static data once. Review namespace-scope helpers, macros, and static
  initialization dependencies before crossing translation-unit boundaries.
- Keep `pch.h` first where required and declare actual dependencies in each
  unit. Do not depend on accidental includes from the former monolithic file.
- Compile each `.cpp` normally; do not include `.cpp` files, introduce a unity
  build, or copy entire common include blocks merely to simulate separation.
- Keep implementation details local. Do not expose private fields or add a
  generic shared header solely to make extraction easier.
- Preserve language-compatible identifiers and values. Translate explanatory
  legacy engineering prose in touched files as required by the English prose
  rule, but review
  contract-bearing translations separately. Keep such edits explicit in review.

<a id="tmhuman-implementation-sequence"></a>
### Implementation sequence

Every batch starts from the current tree, preserves unrelated changes, and
ends with focused validation before the next batch. Do not repeat a global
inventory or immutable evidence audit after each move.

| Batch | Work | Exit condition |
| --- | --- | --- |
| 0: preparation | Record all method signatures and overloads, their intended owner, namespace/static dependencies, and source-based test consumers | Every definition accounted for; known coupling and tests identified |
| 1: movement pilot | Move complete position, angle, and route methods; update their test lookup and build entries together | Definition inventory preserved; angle/route checks pass; affected target links |
| 2: UI | Move complete label, control, and chat-presentation methods | Control ownership and teardown remain unchanged; focused checks pass |
| 3: animation and combat | Move animation first, then combat in separately reviewable patches | Animation/death guards and transition ordering remain covered |
| 4: appearance and mounts | Move appearance first, then mount selection/update methods | Equipment/dye inputs, mounted paths, and resource selection preserved |
| 5: rendering and effects | Move render/material methods, then complete effect methods | Rendering inputs and effect creation/update/destruction ordering preserved |
| 6: packets and final core | Move dispatcher/handlers without rewriting dispatch; review residual core responsibilities | All handlers retained, signatures and wire behavior unchanged, no duplicate/missing definitions |
| 7: integrated acceptance | Validate the complete executable batch and record runtime gates separately | Static/automated gates pass; unavailable runtime checks explicitly pending |

Batch 0's manifest belongs in this document or an existing continuity record,
not a new document inside the source tree. Resolve ambiguous ownership during
that batch instead of relocating a method twice. Do not enforce a fixed line
limit that splits coherent logic. Keep patches small enough to review function
bodies and dependency changes independently.

<a id="tmhuman-build-and-test-adaptation"></a>
### Build and test adaptation

Register every new unit in
[`TMProject748.vcxproj`](../../tmproject/TMProject748/TMProject748.vcxproj)
and its [filters](../../tmproject/TMProject748/TMProject748.vcxproj.filters),
retaining Debug/Release Win32 settings and precompiled-header behavior.
Check affected files and consumers for hard-coded source paths when moving them.

[`SceneDisconnectContractTests.cpp`](../../tmproject/TMProject748/tests/SceneDisconnectContractTests.cpp)
now reads explicit core, movement, animation, combat and packet owners. Its
former animation and angle checks depended on neighboring definitions; those
boundaries have been replaced with complete-signature extraction by
[`SourceMethod.h`](../../tmproject/TMProject748/tests/SourceMethod.h).
The core `FrameMove` and `InitObject` assertions are likewise method-scoped.
All previous contract assertions remain; eight extractor checks cover nesting,
comments, ordinary/raw literals, overloads, missing/duplicate definitions,
incomplete bodies and unterminated comments/literals. Empty extraction fails.

The executable source consumer
[`skill-visual-audit.ps1`](../../.agents/research/client748/skill-visual-audit.ps1)
now reads combat and effects separately, without concatenation. Its ground
motion and deferred-dispatch assertions remain intact, and missing event/reset
markers fail explicitly. Historical inventory/evidence paths are snapshots,
not executable lookups, and were not regenerated or rewritten.

Locate each asserted method in its owning source and delimit its body reliably,
without requiring a method in another file as a boundary. If introducing an
extractor, test missing and duplicate signatures, overloads, and nested braces;
do not accept an empty extraction as success. Do not concatenate files just to
preserve old textual adjacency, remove assertions to obtain a pass, or treat
source-text checks as full behavioral coverage.

Reuse existing policy tests for `HumanAnglePolicy`, `DeathMotionPolicy`,
`SkinMotionPolicy`, and `BaseCostumeLook`. Preserve their coverage and add
focused tests when a policy's actual inputs or behavior change.

<a id="tmhuman-validation-and-acceptance"></a>
### Validation and acceptance

For each logical move:

1. Compare pre/post method signatures and definition counts, including overloads;
   inspect bodies for unintended semantic edits and static-data duplication.
2. Check includes, build registration, class declarations, and source-path consumers.
3. Run the affected architecture/contract tests and an incremental affected-target
   compile/link. Record exact commands and outcomes, including missing prerequisites.
4. Run `git diff --check`; inspect changed engineering prose and EN/PT product
   locale coverage.

For the integrated C++ batch, use the supported non-deploy workflow from the
repository root, as described in [build and integration](../build-and-integration.md):

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -Configuration Release -NoDeploy
```

This builds/runs architecture tests and builds the solution without replacing
`tmproject/client748/project.exe`. Do not repeat the full wrapper after every
individual move when a focused gate suffices. Validate Debug Win32 configuration
coverage for the new units; do not require a clean rebuild without a dependency
or configuration reason. No server-wide suite is required for a contract-neutral
source relocation unless a shared dependency or actual contract delta warrants it.

When runtime execution is permitted and available, cover login/world entry,
movement/rotation, mounted and unmounted appearance, dyed/refined equipment,
inventory appearance, combat/death/resurrection, effects, labels/minimap,
logout/reconnect, and applicable device-resource restoration. This document
does not authorize launching, installing, or closing the game. Until those
checks run, report `STATICALLY VERIFIED` and `AUTOMATED TESTED` separately;
do not claim `CLIENT_TESTED` or completed visual parity.

Mechanical acceptance requires preserved definitions and declarations,
independently compiled units, passing applicable tests/builds, maintained
source-based assertions, and explicit pending runtime gates. A failure stops
its dependent batch; preserve the last validated state without broad resets.

<a id="tmhuman-second-phase-specialized-components"></a>
### Second phase: specialized components

After mechanical acceptance, extract one narrowly bounded policy at a time.
Prioritize pure calculations and selectors with explicit inputs/outputs, using
existing policy helpers as the pattern. Keep scene/device resources and mutable
ownership in their existing owner until a separate lifecycle design is proven.
Do not introduce new classes merely to pass a `TMHuman*` everywhere and access
all of its state; that relocates coupling rather than reducing it.

Candidate areas include appearance selection, route decisions, animation
selection, and effect eligibility. Each extraction needs focused tests for
normal, invalid, and boundary inputs and a review of affected consumers.
Changes to class storage, dispatch, or ownership are not mechanical moves.

For context efficiency, route future tasks to the relevant unit and its callers
rather than reading every new file. Measure representative investigations
before claiming savings; see [agent token efficiency](../agent-token-efficiency.md).
Maintain this plan as batches complete, recording changed inputs, validation,
and the next unresolved dependency without creating a handoff per patch.

<a id="tmhuman-completed-specialized-policy"></a>
#### Completed specialized policy

[`HumanCostumeRefinement.h`](../../tmproject/TMProject748/internal/game/entities/HumanCostumeRefinement.h)
extracts only the existing fixed body-refinement costume membership from
`SetHumanCostume`. Its `constexpr` input/output is `int -> bool`; it has no
scene, device, network or `TMHuman` dependency. The caller retains its existing
six refinement and six legend-byte writes and all resource/mutable ownership.
The two identical positive branches become one policy-selected branch; all
other appearance selection, branch effects and return values are preserved.
This is renderer membership, not new item validity or server refinement rules.

[`HumanCostumeRefinementTests.cpp`](../../tmproject/TMProject748/tests/HumanCostumeRefinementTests.cpp)
uses an independent explicit list of the original 128 accepted IDs. It checks
every signed-short value (65,536 runtime inputs), four out-of-domain integer
inputs and compile-time boundary assertions, including holes 4306/4307/4308,
4319/4375 and the upper edge 4420/4421. The test is part of ArchitectureTests.
The relocation gate rejects any other changed method, signature, header,
policy implementation or unreviewed version of this extraction.

<a id="tmhuman-acceptance-evidence-and-remaining-gates"></a>
#### Acceptance evidence and remaining gates

| Requirement | Result |
| --- | --- |
| Batches 0-6: inventory, all domains and residual core | PASS: 128 unique definitions; all proposed owners implemented; no methods removed |
| Fields, ABI, overloads, constants and lifecycle | PASS: unchanged class header and 127 body-token fingerprints; the one specialized body and policy have explicit reviewed fingerprints and exhaustive tests |
| Static and namespace-scope dependencies | PASS: `m_vecPickSize` and `m_dwNameColor` defined only in core; no new namespace-scope implementation in extracted units |
| Includes, PCH and normal translation units | PASS: per-unit dependencies, `pch.h` first, one project/filter entry each, no `.cpp` inclusion or unity build |
| Source-based consumers and existing policy coverage | PASS: explicit owners and balanced extraction; prior assertions retained; angle, death, skin and base-costume tests still pass |
| Batch 7: executable integration | PASS: supported Release `-NoDeploy`, Debug Win32 compile/link, ArchitectureTests and SocketReceiveTests |
| Specialized component | PASS: pure costume selector, no moved ownership, normal/invalid/boundary and exhaustive coverage |
| Documentation, language and tooling | PASS: relocation-tool tests, static audit success/rejections, repository layout, changed-file English review and whitespace checks |
| Visual/in-game and device-resource flows | PENDING: unavailable runtime gate; no `CLIENT_TESTED` or visual-parity claim |

Existing header-local `g_pItemGridXY` (`SGrid.h`) and `sSwingScale`
(`TMEffectSWSwing.h`) retain their values and linkage. Active references only
read these lookup tables; no table writes or address-identity consumers were
found. Moving consumers does not split shared mutable state. Class static
tables were not copied. The effects unit is 3,153 lines, comparable to core and
appearance, so no arbitrary further file split was introduced.

Final reproducible commands from the repository root:

```powershell
python .agents/research/tmhuman-separation.py --complete --with-policies
python .agents/research/tmhuman-separation-tests.py
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -Configuration Release -NoDeploy
& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe' tmproject/TMProject748/TMProject748.vcxproj /t:Build /p:Configuration=Debug /p:Platform=Win32 /p:SolutionDir=C:\Users\xereca\Documents\PROJETOS\wyd-go\tmproject\ /m /nologo /v:minimal
pwsh -NoProfile -ExecutionPolicy Bypass -File .agents/research/client748/skill-visual-audit.ps1
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tools\repository\Test-RepositoryLayout.ps1
git diff --check
```

All completed commands passed. The static skill audit covers 104 records,
89 active skills and 86,400 animation projections with no missing listed
animations or failures; it does not validate every particle or visual flow.
Injected missing-event and missing-reset sources were rejected. Existing
compiler signed/unsigned and legacy DirectX-header warnings remain, including
the four moved UI comparisons; unrelated warning cleanup was not attempted.
The final whitespace-only cleanup passed the same token gate and did not
invalidate the compile/link evidence. No server code or contracts changed,
so a server-wide Go suite and new native/Ghidra research were not required.

The installed `tmproject/client748/project.exe` SHA256 before and after was
`AEC3FD508CBC49F74C1E81DC2C6FE10214564B806835D31EA62B342206413679`.
Build outputs remain in ignored build directories. Changes consist of the
nine companion units, the policy, tests/extractor, project registrations,
the core relocation, research tooling/audit lookup and this record. Diagnostic
strings/comments in the touched packet-test runner were translated to English;
packet fixtures, identifiers and wire bytes were not translated or changed.
No files were removed. Pre-existing documentation changes, local binaries and
other concurrent files were preserved; no commit, push or deployment was made.

Next gate when explicitly permitted and available: login/world entry,
movement/rotation, mounted and unmounted appearance, dyed/refined equipment,
inventory appearance, combat/death/resurrection, effects, labels/minimap,
logout/reconnect and device-resource restoration. Other candidate policy
extractions are future separately scoped work, not prerequisites to this
bounded first specialization. No performance/token-savings measurement or
scoped language-migration/release completion is claimed.

<a id="tmhuman-third-phase-method-decomposition"></a>
### Third phase: method decomposition

Private helper declarations may be added to `TMHuman.h` (decided 2026-10-02;
no layout or virtual-table change). `tmhuman-separation.py --pin <spec>` records
each added header line, helper digest and before/after method digest in
`.agents/research/relocations/tmhuman-extractions.json`. `--with-policies`
accepts only those pins. Specs: `.agents/research/relocations/method-tmhuman-*.json`.

| Method | Lines before -> after | Helpers |
| --- | --- | --- |
| `FrameMoveEffect` | 1,883 -> 864 | `UpdateHiddenEffects`, `PlayEffectEvent` |
| `InitObject` | 1,039 -> 756 | `InitSkinLookInfo`, `InitEyeFireEffects`, `InitShadeEffects` |
| `FrameMove` | 947 -> 505 | `UpdateEarthQuake`, `UpdateMoveTargetsOnArrival`, `UpdateShadeVisibility`, `FrameMoveSkinMesh` |
| `AnimationFrame` | 814 -> 31 | `PlayWalkFrame`, `PlayNonAttackFrame`, `PlayAttackFrame` |

`SetHumanCostume` and `SetMountCostume` are single costume/mount selection
chains. Moving them into a helper would only rename the problem; they are
candidates for verified tables, like `TMSkinMesh::SetCostume`. Built-client
acceptance of this phase is pending.

<a id="tmhuman-complete-relocation-manifest"></a>
### Complete relocation manifest

Baseline: `77e8dcc7f1af929be1c6418d08b97c7dd8412713`, 128 definitions.
The reproducible inventory and code/literal-token comparison are maintained in
the [relocation gate](../../.agents/research/tmhuman-separation.py), with focused
[regression tests](../../.agents/research/tmhuman-separation-tests.py).
Use `--with-policies` for the final state: strict mechanical comparison alone
intentionally rejects the documented second-phase extraction. Comments and whitespace are
excluded from token comparison; all literals and code tokens remain covered.
`TMHuman.h` is compared byte-for-byte after line-ending normalization.

The two static tables (`m_vecPickSize`, `m_dwNameColor`) stay in the core.
There are no active namespace-scope functions, macros, or conditional compilation
blocks in the baseline. Commented historical examples are not definitions.
`UpdateScore`, `CheckAffect`, merchant/class classification and lifecycle remain
in the core: their callers span presentation, combat and packet processing.
`_locationCheck` is UI-owned because only the label methods call it; `SetSpeed`
owns movement speed; character height, avatar/leg/blood appearance selectors
remain appearance-owned. Cross-domain callers keep ordinary member calls.

The source-contract consumer is `SceneDisconnectContractTests.cpp`: animation,
death/combat, position/angles, packet handlers and core initialization/frame
updates. It must load the explicit owner and extract a full method signature
without depending on the following method. Each unit declares its actual
scene, mesh, control, effect, global or policy dependencies before compilation.

| Complete signature (including overloads) | Intended owner |
| --- | --- |
| `void TMHuman::SetMotion(ECHAR_MOTION eMotion, float fAngle)` | `TMHumanAnimation.cpp` |
| `TMHuman::TMHuman(TMScene* pParentScene)` | `TMHuman.cpp` |
| `TMHuman::~TMHuman()` | `TMHuman.cpp` |
| `int TMHuman::InitObject()` | `TMHuman.cpp` |
| `int TMHuman::Render()` | `TMHumanRender.cpp` |
| `int TMHuman::FrameMove(unsigned int dwServerTime)` | `TMHuman.cpp` |
| `void TMHuman::RestoreDeviceObjects()` | `TMHuman.cpp` |
| `void TMHuman::InvalidateDeviceObjects()` | `TMHuman.cpp` |
| `int TMHuman::IsMouseOver()` | `TMHumanUI.cpp` |
| `int TMHuman::OnCharEvent(char iCharCode, int lParam)` | `TMHumanUI.cpp` |
| `int TMHuman::OnPacketEvent(unsigned int dwCode, char* buf)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketMove(MSG_Action* pAction)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketChaosCube(MSG_Action* pAction)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketIllusion(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketFireWork(MSG_Motion* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketPremiumFireWork(MSG_PremiumFirework* pFirework)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketRouteCorrection(MSG_Action* pAction)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketRemoveMob(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketSendItem(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketUpdateEquip(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketUpdateAffect(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketUpdateScore(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketSetHpMp(MSG_SetHpMp* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketSetHpDam(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketMessageChat(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketMessageChat_Index(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketMessageChat_Param(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketUpdateEtc(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketUpdateCoin(MSG_STANDARDPARM* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketUpdateRMB(MSG_STANDARDPARM* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketTrade(MSG_Trade* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketQuitTrade(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketCarry(MSG_Carry* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketCNFCheck(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketSetClan(MSG_STANDARDPARM* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketReqRanking(MSG_STANDARDPARM2* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::OnPacketVisualEffect(MSG_STANDARD* pStd)` | `TMHumanPackets.cpp` |
| `int TMHuman::IsMerchant()` | `TMHuman.cpp` |
| `void TMHuman::Init()` | `TMHuman.cpp` |
| `void TMHuman::SetRace(short sIndex)` | `TMHumanAppearance.cpp` |
| `void TMHuman::UpdateScore(int nGuildLevel)` | `TMHuman.cpp` |
| `void TMHuman::SetAnimation(ECHAR_MOTION eMotion, int nLoop)` | `TMHumanAnimation.cpp` |
| `void TMHuman::SetColorMaterial()` | `TMHumanRender.cpp` |
| `void TMHuman::AnimationFrame(int nWalkSndIndex)` | `TMHumanAnimation.cpp` |
| `void TMHuman::LabelPosition()` | `TMHumanUI.cpp` |
| `void TMHuman::LabelPosition2()` | `TMHumanUI.cpp` |
| `void TMHuman::HideLabel()` | `TMHumanUI.cpp` |
| `void TMHuman::RenderEffect()` | `TMHumanEffects.cpp` |
| `void TMHuman::FrameMoveEffect(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::FrameMoveEffect_AvatarTrans()` | `TMHumanEffects.cpp` |
| `void TMHuman::FrameMoveEffect_AvatarFoema()` | `TMHumanEffects.cpp` |
| `void TMHuman::FrameMoveEffect_AvatarBMaster()` | `TMHumanEffects.cpp` |
| `void TMHuman::FrameMoveEffect_AvatarHunter()` | `TMHumanEffects.cpp` |
| `void TMHuman::MoveAttack(TMHuman* pTarget)` | `TMHumanCombat.cpp` |
| `void TMHuman::MoveGet(TMItem* pTarget)` | `TMHumanCombat.cpp` |
| `void TMHuman::Attack(ECHAR_MOTION eMotion, TMVector2 vecTarget, char cSkillIndex)` | `TMHumanCombat.cpp` |
| `void TMHuman::Attack(ECHAR_MOTION eMotion, TMHuman* pTarget, short cSkillIndex)` | `TMHumanCombat.cpp` |
| `void TMHuman::Punched(int nDamage, TMVector2 vecFrom, short sSkillIndex)` | `TMHumanCombat.cpp` |
| `void TMHuman::Punched(int nDamage, TMHuman* pFrom)` | `TMHumanCombat.cpp` |
| `void TMHuman::Fire(TMObject* pTarget, int nSkill)` | `TMHumanCombat.cpp` |
| `void TMHuman::Die()` | `TMHumanCombat.cpp` |
| `void TMHuman::Stand()` | `TMHumanCombat.cpp` |
| `void TMHuman::SetWeaponType(int nWeaponType)` | `TMHumanAppearance.cpp` |
| `void TMHuman::CheckWeapon(short sIndexL, short sIndexR)` | `TMHumanAppearance.cpp` |
| `void TMHuman::PlayAttackSound(ECHAR_MOTION eMotion, int nLR)` | `TMHumanCombat.cpp` |
| `void TMHuman::PlayPunchedSound(int nType, int nLR)` | `TMHumanCombat.cpp` |
| `void TMHuman::SetHandEffect(int nHandEffect)` | `TMHumanEffects.cpp` |
| `void TMHuman::CheckAffect()` | `TMHuman.cpp` |
| `void TMHuman::SetChatMessage(const char* szString)` | `TMHumanUI.cpp` |
| `int TMHuman::GetChatLen(const char* szString, int* pHeight)` | `TMHumanUI.cpp` |
| `void TMHuman::SetPacketMOBItem(STRUCT_MOB* pMobData)` | `TMHumanAppearance.cpp` |
| `void TMHuman::SetPacketEquipItem(unsigned short* sEquip)` | `TMHumanAppearance.cpp` |
| `void TMHuman::SetColorItem(char* sEquip2)` | `TMHumanAppearance.cpp` |
| `void TMHuman::SetInMiniMap(unsigned int dwCol)` | `TMHumanUI.cpp` |
| `void TMHuman::UpdateGuildName()` | `TMHumanUI.cpp` |
| `void TMHuman::GetLegType()` | `TMHumanAppearance.cpp` |
| `int TMHuman::GetBloodColor()` | `TMHumanAppearance.cpp` |
| `void TMHuman::DelayDelete()` | `TMHuman.cpp` |
| `void TMHuman::SetCharHeight(float fCon)` | `TMHumanAppearance.cpp` |
| `int TMHuman::StartKhepraDieEffect()` | `TMHumanEffects.cpp` |
| `void TMHuman::SetAvatar(char cAvatar)` | `TMHumanAppearance.cpp` |
| `void TMHuman::UpdateMount()` | `TMHumanMounts.cpp` |
| `float TMHuman::GetMyHeight()` | `TMHumanMounts.cpp` |
| `void TMHuman::SetGuildBattleHPColor()` | `TMHumanUI.cpp` |
| `void TMHuman::SetGuildBattleHPBar(int nHP)` | `TMHumanUI.cpp` |
| `void TMHuman::SetGuildBattleMPBar(int nMP)` | `TMHumanUI.cpp` |
| `void TMHuman::SetGuildBattleLifeCount()` | `TMHumanUI.cpp` |
| `int TMHuman::Is2stClass()` | `TMHuman.cpp` |
| `int TMHuman::IAmkhepra()` | `TMHuman.cpp` |
| `void TMHuman::CreateControl()` | `TMHumanUI.cpp` |
| `void TMHuman::DestroyControl()` | `TMHumanUI.cpp` |
| `int TMHuman::StrByteCheck(const char* szString)` | `TMHumanUI.cpp` |
| `void TMHuman::SetMantua(int nTexture)` | `TMHumanAppearance.cpp` |
| `int TMHuman::SetCitizenMantle(int BaseSkin)` | `TMHumanAppearance.cpp` |
| `int TMHuman::UnSetCitizenMantle(int BaseSkin)` | `TMHumanAppearance.cpp` |
| `int TMHuman::MAutoAttack(TMHuman* pTarget, int mode)` | `TMHumanCombat.cpp` |
| `bool TMHuman::SetImportedMountCostume(unsigned int itemIndex)` | `TMHumanMounts.cpp` |
| `void TMHuman::SetMountCostume(unsigned int index)` | `TMHumanMounts.cpp` |
| `int TMHuman::SetHumanCostume()` | `TMHumanAppearance.cpp` |
| `void TMHuman::RenderEffect_RudolphCostume(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_Khepra(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_LegendBerielKeeper(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_LegendBeriel(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_Pig_Wolf(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_DungeonBear(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_Hydra(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_DarkNightZombieTroll(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_DarkElf(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_Minotauros(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_EmeraldDragon(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_BoneDragon(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_Golem(unsigned int dwServerTime)` | `TMHumanEffects.cpp` |
| `void TMHuman::RenderEffect_Skull()` | `TMHumanEffects.cpp` |
| `void TMHuman::InitPosition(float fX, float fY, float fZ)` | `TMHumanMovement.cpp` |
| `void TMHuman::InitAngle(float fYaw, float fPitch, float fRoll)` | `TMHumanMovement.cpp` |
| `void TMHuman::SetAngle(float fYaw, float fPitch, float fRoll)` | `TMHumanMovement.cpp` |
| `void TMHuman::SetPosition(float fX, float fY, float fZ)` | `TMHumanMovement.cpp` |
| `void TMHuman::MoveTo(TMVector2 vecPos)` | `TMHumanMovement.cpp` |
| `void TMHuman::OnlyMove(int nX, int nY, int nLocal)` | `TMHumanMovement.cpp` |
| `int TMHuman::IsGoMore()` | `TMHumanMovement.cpp` |
| `void TMHuman::SetWantAngle(float fAngle)` | `TMHumanMovement.cpp` |
| `void TMHuman::GetRoute(IVector2 vecTarget, int nCount, int bStop)` | `TMHumanMovement.cpp` |
| `void TMHuman::GenerateRouteTable(int nSX, int nSY, char* pRouteBuffer, TMVector2* pRouteTable, int* pMaxRouteIndex)` | `TMHumanMovement.cpp` |
| `int TMHuman::StraightRouteTable(int nSX, int nSY, int nTargetX, int nTargetY, TMVector2* pRouteTable, int* pMaxRouteIndex, int distance, char* pHeight, int MH)` | `TMHumanMovement.cpp` |
| `int TMHuman::ChangeRouteBuffer(int nSX, int nSY, TMVector2* pRouteTable, int* pMaxRouteIndex)` | `TMHumanMovement.cpp` |
| `void TMHuman::SetSpeed(int bMountDead)` | `TMHumanMovement.cpp` |
| `bool TMHuman::_locationCheck(TMVector2 vec2, int mapX, int mapY)` | `TMHumanUI.cpp` |
