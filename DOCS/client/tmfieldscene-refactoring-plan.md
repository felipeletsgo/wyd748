# TMFieldScene refactoring and architecture plan

Status: Stage 1 source separation complete. Stage 2 batches 9-13 (chat filters,
skill requests, attack visual damage, attacker and target state, chat submission
and coin input) implemented and automatically validated. Header-changing
decomposition and built-client acceptance remain pending.
Snapshot: 2026-10-02, HEAD `71c4b313` plus the working tree; the relocation
gate still compares against the fixed baseline `77e8dcc7f1af929be1c6418d08b97c7dd8412713`.
This record describes source changes, not a deployed candidate, runtime
acceptance or global release approval.

## Implementation progress

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
[Implemented Stage 2 contracts](#implemented-stage-2-contracts-batches-10-13).
ArchitectureTests now passes 974,712 checks; the growth is exhaustive oracle
sweeps, not new runtime coverage.

The remaining decomposition (attack visual dispatch, the other `OnControlEvent`
domains, `FrameMove`, `SkillUse`/`AutoSkillUse` sequences and `InitializeScene`)
needs new private member helpers or narrowly owned components. That changes the
pinned scene header and therefore requires the separate review described in
[Remaining work](#remaining-work-and-the-next-decision).
No runtime executable was replaced and no game process was launched or stopped.
All runtime flows listed below remain pending; these results are not
`CLIENT_TESTED` and do not prove complete visual or gameplay parity.

## Objective and scope

Split the 31,280-line
[`TMFieldScene.cpp`](../../tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp)
into responsibility-based translation units, then reduce coupling and oversized
methods through narrowly scoped policies and application components.
Preserve the existing 7.48 client/server contract and observable behavior.

The working tree already contains the TMHuman source separation. Preserve those
changes and establish their validation status before modifying shared consumers;
their presence does not prove that their runtime validation is complete.
Follow the [TMHuman separation plan](tmhuman-separation-plan.md) where it applies,
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

## Current structural problems

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

## Architecture: two stages

### Stage 1: separate definitions without changing the class

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

### Stage 2: reduce shared state and oversized methods

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

## First implementation batch: chat

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

## Execution sequence and gates

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

## Dependency and ownership safeguards

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
- Translate explanatory legacy text in changed source files as required by
  the English-only rule. Review contract-bearing text separately; do not
  rename persisted values, resource IDs, opcodes, or established identifiers
  merely to make a mechanical diff look cleaner.

## Test migration and validation

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

## Completion and context efficiency

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

## Implemented Stage 2 contract: native chat filters

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

## Implemented Stage 2 contracts (batches 10-13)

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

## Remaining work and the next decision

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
for declaration changes. That review is the next explicit decision; do not
bypass the header pin to continue.

## Helper ownership and caller inventory

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

## Reproducible acceptance gates

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

Latest results (2026-10-02, after batch 13): relocation gate PASS (293
definitions; 289 unchanged bodies, 4 pinned bodies, 10 unchanged helpers and
1 pinned helper); its 18 regression tests PASS; Release `-NoDeploy` build PASS
with ArchitectureTests 974,712 checks and SocketReceiveTests 221 checks; Debug
Win32 compile/link PASS; repository layout PASS (162 documents);
`git diff --check` PASS. The installed `project.exe` was not modified.

Release integration also passed the costume dependency audit (135 items,
129 renderers, 774 parts), four shader contracts and shader rejection cases.
Existing compiler warnings remain; passing compile/link does not mean a
warning-free build. No Go or new Ghidra gate was needed: the wire and native
boundaries are unchanged.

## Complete member ownership manifest

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
