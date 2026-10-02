# TMProject source organization plan

Status: complete. File organization, data tables, method decomposition, test
split and include pruning are done and `CLIENT-TESTED` (visual acceptance).
Snapshot: 2026-10-02, HEAD `71c4b313` plus working tree.
Scope: the large or multi-responsibility TMProject748 files found by the
2026-10-02 survey, excluding `TMFieldScene*` and `TMHuman*`, which have their
own [TMFieldScene](tmfieldscene-refactoring-plan.md) and
[TMHuman](tmhuman-separation-plan.md) plans.

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
