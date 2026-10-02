# TMHuman separation plan

Status: mechanical separation and the first specialized policy completed;
static and automated acceptance passed, visual runtime acceptance pending.
Snapshot: 2026-10-01, HEAD `77e8dcc7f1af929be1c6418d08b97c7dd8412713`.
This record describes the current working-tree implementation, not a commit,
deployed candidate, global release approval, or `CLIENT_TESTED` result.

## Implementation progress

All 128 definitions now have exactly one owner across the ten units below.
`TMHuman.cpp` decreased from approximately 18,661 to 3,391 lines; total class
storage and ownership remain unchanged. All complete bodies were mechanically
preserved before specialization. The final gate compares 127 unchanged bodies
and pins the exact reviewed `SetHumanCostume` policy extraction separately.
`TMHuman.h` remains unchanged, including fields, declarations and overloads.

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

## Objective and scope

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

## Implemented file ownership

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

## Invariants

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
  legacy text in touched files as required by the English-only rule, but review
  contract-bearing translations separately. Keep such edits explicit in review.

## Implementation sequence

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

## Build and test adaptation

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

## Validation and acceptance

For each logical move:

1. Compare pre/post method signatures and definition counts, including overloads;
   inspect bodies for unintended semantic edits and static-data duplication.
2. Check includes, build registration, class declarations, and source-path consumers.
3. Run the affected architecture/contract tests and an incremental affected-target
   compile/link. Record exact commands and outcomes, including missing prerequisites.
4. Run `git diff --check`; inspect authored text for the English-only requirement.

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

## Second phase: specialized components

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

### Completed specialized policy

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

### Acceptance evidence and remaining gates

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
global language-migration/release completion is claimed.

## Complete relocation manifest

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
