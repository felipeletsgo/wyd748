# Porting the buildable client to WYD-Go

## Status

The imported source originally targeted the Global/KR 7.69+ client. In this
tree, it is being adapted as a single-version Win32/DirectX 9 implementation
for the proven WYD-Go 7.48 protocol, ABI, UI, and assets. Do not retain an
executable path for the upstream version.

The installed candidate is `tmproject/client748/WYD.exe`, the executable the
launcher starts. Until October 11, 2026 it was installed as `project.exe` to
avoid replacing an older `WYD.exe`; earlier records use that name. With
`Build-Client.ps1 -NoDeploy`, only the artifact in `tmproject/build/` is
updated; the installed executable remains unchanged. Historical executables
and patchers are not fallbacks, products, or validation gates.

The source no longer exposes the later repurchase, numeric AccountLock, or
drop-list packet paths. This is not evidence that an already installed
`WYD.exe` contains those source changes. The [opcode catalog](../wire-opcode-catalog.md#retired-and-incomplete-legacy-paths)
separates retired paths from incomplete server integrations; in particular,
war letters, alternate mix, and account transfer are not supported end to end.

## Recent integrated contracts and validation limits

The October 7 source/tests and documentation are published in
[`83a94da7`](https://github.com/felipeletsgo/wyd748/commit/83a94da7c531ebdebe2ccc2faedbc35b3bbffd0a).
Use the following current records instead of treating older audit failures or
uncommitted-batch descriptions as the live implementation queue:

- [Inventory stacks and CP contract v1](inventory-stack-and-cp-contract.md):
  effect 61 quantity rendering, split-caption fallback, authoritative merging,
  CP rewards/refresh and the deliberate white-NPC name policy. Native palette
  evidence and product-policy changes are distinguished in that contract.
- [S2 closure audit](../server/s2-closure-audit-2026-10-07.md): shared-object
  settlement, indefinite account containment and detached instance visibility
  corrections passed the integrated Go race/PostgreSQL batch; verified recovery
  after unknown outcomes is still open.
- [Performance and visuals plan](performance-and-visuals-plan.md): implemented
  weather/cache/light-profile work retains its recorded measurements and its
  separate field-crossing, weather/damage, tuning and accumulation gates.
- Shutdown double free (2026-10-07): leaving the game from the field crashed in
  `RtlFreeHeap` because `NewApp::Finalize` freed the bone animation buffers and
  `~MeshManager` freed them again. Both now call the idempotent
  `MeshManager::ReleaseBoneAnimations`, which clears the pointers; the minidump
  was symbolized against the build PDB and `UiModeContractTests` guards the
  single release. Exit from the field is pending a user client test.

The user reported apparently correct gameplay on October 7. This is not a new
agent-executed test, an exhaustive acceptance matrix, or a reason to promote
unexercised catalog entries to `CLIENT_TESTED`. The sections below retain the
limits of each proven flow; unresolved sale policy, legacy opcode and asset
evidence must not be closed merely because the general session looked correct.

## Input and layout review, 2026-10-10

This batch reviews the current dirty tree, not only the last published commit.
The native Character bindings, uniform UI scaling, minimap transformations and
selected-direction zoom correction were already present; they are not new work
from this review. Automated checks do not establish that every native control
is populated, clickable or visually aligned in the installed executable.

Corrections in this batch (`MODERNIZACAO_COMPATIVEL`, no wire/asset change):

- `EventTranslator` initializes mouse/button/control-modifier state and clears
  movement deltas before polling. A failed DirectInput read discards the sample,
  tries reacquisition once per frame and cannot repeat the preceding wheel or
  rotation movement. Camera processing consumes a successful sample only once
  and checks the scene/camera lifetime before dereferencing. This restores the
  successful-read gate in the existing native `FUN_004aec3d` evidence while
  avoiding an unbounded acquisition loop.
- `CameraZoomDistance` isolates the existing direction correction for behavior
  tests: both camera settings, both rotation settings, near/far limits, large
  scroll deltas and stationary input. Failed polling and recovery exercise the
  production polling helper with a fake device, not a connected physical mouse.
- The Party button hint is positioned from its width instead of accumulating
  an X offset every time `PositionCompatPartyPanel` runs. The first placement
  remains unchanged. Tests cover repeated placement at five UI scales.
- `ClientSettings` accepts complete Auto/numeric values with whitespace or
  trailing comments, rejecting suffix junk, overflow and non-finite scales.
  A rejected line preserves the preceding valid value. This is the modern
  options parser; native `config.txt` and RC binary formats are unchanged.

Validation: `ArchitectureTests` passed 1,152,749 checks and static assertions;
`SocketReceiveTests` passed 231 checks. The Release client build passed via
`pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1 -NoDeploy`.
Asset/shader gates also passed. The socket test build still reports the existing
`inet_addr` and `WSAAsyncSelect` deprecation warnings. Status is `STATICALLY
VERIFIED` and `AUTOMATED TESTED`, not `CLIENT-TESTED`. Only the build artifact
was produced; the installed `client748/project.exe` was not replaced.

Changed files: new `internal/core/CameraInput.h`; existing
`internal/core/{EventTranslator.cpp,EventTranslator.h,ClientSettings.h,UiLayout.h}`,
`internal/app/scenes/TMFieldSceneUI.cpp`, `TMProject748.vcxproj` and
`tests/{ClientSettingsTests.cpp,UiModeContractTests.cpp,FieldViewContractTests.cpp}`
under `tmproject/TMProject748/`, plus this document and the UI evidence record.
No files were removed. Repository layout/local links and research schema passed;
`git diff --check` found no whitespace errors. Pre-existing edits were preserved.

Remaining acceptance/research boundaries:

- Execute scroll in both directions, near/far limits, Alt+Tab recovery and
  middle-button rotation in the built client. Verify the deliberate existing
  direction correction with `CAMERAVIEW`/`CAMERAROTATE` combinations.
- Reopen Party repeatedly and hover its hint at 800x600, 1024x768 and a
  widescreen resolution; inspect Character fields, Inventory, modal placement
  and hitboxes at automatic and fixed scales. Prior captures do not validate
  this batch.
- RC structural preflight, unsafe geometry, texture-set references and
  control-tree allocation-failure rollback are covered by the follow-ups
  below. Indexed-caption bounds, startup failures and the three shipped Scene2
  caption tables are also covered. The font-upload continuation covers texture
  failures and RC button/text caption rejection. The GDI follow-up covers
  transactional font creation, partial cleanup and repeated restore/release.
  The device-restore follow-up covers failed state submissions, cache retries,
  texture-binding invalidation and required restore-result propagation.
  Actual Direct3D reset, other graphics-resource failures, visual geometry acceptance and captions
  outside these resources remain pending.
  The IME candidate layout has been adapted;
  its remaining runtime gates are recorded in the
  [UI evidence record](../../.agents/research/client748/flows/ui/field-ui-scale-and-anchors.md).
- War letters, alternate mix and account transfer retain their existing
  incomplete-integration status. This batch does not implement those flows or
  claim exhaustive native-client parity.

### IME candidate layout follow-up, 2026-10-10

Native `FUN_00493e70` creates one candidate row at `(28, H - 20)`, sized
`300x16`, widened to 350 below a physical viewport width of 800. The source
already formatted candidates into one line, but still constructed the later
client's ten narrow rows on the right. `TMScene` now creates only the first
row, starts it hidden and registers it with the existing control container.
The spare pointers remain null and their consumers already check for null.

`ImeCandidateLayout` preserves the native mode-2 geometry at scale 1
(`PARIDADE_NATIVA`) and applies the existing uniform-scale product policy at
other scales (`MODERNIZACAO_COMPATIVEL`). Native PE constants and constructor
arguments are recorded in the UI evidence linked above; the prior X = 30
documentation was corrected to the proven value 28.

`UiModeContractTests` adds 164 checks for geometry, viewport thresholds,
scaling and production bindings. `ArchitectureTests` passed 1,153,253 checks
and static assertions. Live CJK IME rendering, paging, focus and composition
remain pending; these checks do not establish `CLIENT_TESTED`. No assets,
wire contracts or server behavior were changed. The changed source files are
`internal/core/UiLayout.h`, `internal/app/scenes/TMScene.cpp` and
`tests/UiModeContractTests.cpp`; no files were removed.

### RC parser follow-up, 2026-10-10

Classification: `MODERNIZACAO_COMPATIVEL`. Record sizes, control IDs, native
resource bytes and packet contracts are unchanged. This batch does not add a
new native widget or claim that all native functions are implemented.

- `ResolveRCBinaryPath` replaces unchecked suffix arithmetic. Null, short,
  extensionless and oversized paths fail without writing outside the buffer.
  Directory underscores no longer remove characters from the scene name. The
  existing optional three-character filename suffix remains supported.
- `ValidateRCStream` checks every tag and payload before `ReadRCBin` constructs
  any controls. It rejects empty/truncated resources, unknown record types,
  absent or forward parents, invalid indexed captions, unterminated inline
  captions and editable-text limits outside the actual 256-byte destination.
  The input is held read-only with write sharing denied during validation and
  construction. Failure logs the file, byte offset, control and reason.
- Existing parents and compatibility aliases remain accepted. Duplicate raw
  IDs are deliberately retained: the shipped FieldScene2 and SelCharScene2
  files contain duplicates, and exact-ID-first lookup is existing behavior.
  A missing parent no longer silently creates an unattached control from a
  malformed resource. This batch supplied preflight protection; the later
  construction continuation adds allocation-failure rollback. Rendering-resource
  failure handling remains outside both batches.

Validation: `ArchitectureTests` passed **1,153,089 checks**, including 340 new
checks since the input/layout batch, all nine supported RC record types in
both caption layouts, rejection cases, stream-position restoration, aliases,
duplicates and the actual FieldScene2/SelCharScene2/SelServerScene2 files.
Source-binding checks require preflight before control dispatch. One test-only
unsafe-copy warning was fixed before the passing run. The incremental Release
solution build passed without new warnings; the earlier socket and asset gates
remain applicable because their code and inputs were unchanged.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, and
MSBuild `tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release
/p:Platform=x86`. No deployment or client execution was performed. Status:
`STATICALLY VERIFIED`, `AUTOMATED TESTED`, and `BUILD_VERIFIED`; not
`CLIENT-TESTED`.

Changed files in this follow-up: `internal/ui/UIBinary.h`,
`internal/app/scenes/TMSceneResources.cpp`, new `tests/UIBinaryTests.cpp`,
`tests/{ArchitectureTests.vcxproj,PacketViewTests.cpp,SceneDisconnectContractTests.cpp}`
under `tmproject/TMProject748/`, and this document. No files were removed.

### RC geometry preflight continuation, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. The RC record ABI, identifiers,
resource bytes and supported control types are unchanged. No native function
or packet contract is added by this internal loader hardening.

`ValidateRCStream` now rejects negative width/height for all nine record
types before widget construction. Grids additionally require positive cell
counts and extents, a signed-int-safe rows-by-columns product, and array byte
sizes representable by the target's `size_t`. The live loader supplies
`sizeof(GeomControl)` rather than assuming a renderer structure size; both
the render-cell array and the integer occupancy array are checked. Lists
require nonnegative capacity, a positive visible-row count and nonzero
height, preventing the actual divisors in their rendering and hit testing
from becoming zero. Existing list capacity clamping remains unchanged.

Coordinates are not restricted to the viewport, passive zero-sized controls
remain legal, and an empty list can retain its visible-row layout. These are
not arbitrary geometry normalization rules or evidence of visual parity.
The existing whole-file preflight still restores the stream position and
rejects malformed input before creating any controls.

Tests exercise both indexed and inline-caption layouts, all nine record
types, negative sizes, zero divisors, signed multiplication overflow,
allocation byte-size boundaries, offscreen placement and retained list
clamping. The newly rejected geometry cases failed against the preceding
implementation. The shipped FieldScene2, SelCharScene2 and SelServerScene2
fixtures still pass. A production binding check requires the renderer cell
size to be supplied before control dispatch.

Changed files: `internal/ui/UIBinary.h`,
`internal/app/scenes/TMSceneResources.cpp`, `tests/UIBinaryTests.cpp` and
`tests/SceneDisconnectContractTests.cpp` under `tmproject/TMProject748/`,
plus this document. No files were removed. Existing dirty-tree work remains
preserved. Validation commands are MSBuild
`tests/ArchitectureTests.vcxproj /t:Build /p:Configuration=Release
/p:Platform=Win32`, `ArchitectureTests.exe`, and MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`.

The focused suite passed 1,153,490 checks and static assertions, including 196
new checks in this batch. Repository layout/local links passed with 186
documents indexed; the scoped whitespace and English-text checks passed.
The incremental Release build passed without new warnings. Only the build artifact was updated;
there was no deployment or client execution. Status is `STATICALLY VERIFIED`,
`AUTOMATED TESTED` and `BUILD_VERIFIED`, not `CLIENT-TESTED`. Geometry checks
do not guarantee available heap memory; the following continuation covers
control-tree rollback when construction fails. Live visual acceptance remains
pending.

### RC construction rollback continuation, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. RC bytes, IDs, caption layouts,
widget order and packet contracts are unchanged. This is internal loader
recovery, not a new native feature or a claim of exhaustive client parity.

`ReadRCBin` now owns its file handle for the entire preflight/construction
scope. Before creating controls, `RCControlTransaction` snapshots the child
heads of the existing tree. RC loading only prepends children and never
removes or reparents existing nodes: on an early return or allocation exception,
the checkpoint deletes newly attached subtrees while retaining old controls,
sibling order and backlinks. Rollback allocates no memory. Only successful
completion commits the new controls; failure to allocate the checkpoint itself
leaves the tree untouched.

All nine RC helpers retain a local control owner until parent attachment
succeeds. Grid occupancy/render arrays, button font/tooltip and the container's
cursor/root retain local ownership across successive allocations. Scrollbar
panels become owned tree children immediately, in the original insertion order.
Panel/progress-bar destruction also tolerates a null current scene during
unwinding. The shared font-buffer loader owns its asset handle through allocation
and complete reads (see the font-upload continuation). Existing successfully allocated font-cache data remains
shared; the transaction covers control-tree construction, not all scene state.

Tests add 106 checks using the production transaction template with an intrusive
tree fixture: empty/populated roots, nested parents, each construction-failure
boundary, checkpoint-allocation failures, successful commits and a failed later
load preserving an earlier commit. Live source-binding checks require the
checkpoint, commit, file owner, all nine helper owners and constructor cleanup
paths. These are simulated allocation/read-failure boundaries and source
bindings, not fault injection into actual DirectX widget constructors.

The focused suite passed **1,153,596 checks** and static assertions. The
incremental Release solution build passed without new warnings. Repository
layout/local links passed with 186 documents indexed; scoped whitespace and
English-text checks passed. Commands:
MSBuild `tests/ArchitectureTests.vcxproj /t:Build /p:Configuration=Release
/p:Platform=Win32`, `ArchitectureTests.exe`, and MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`.
Only the ignored build artifact was updated; the installed `project.exe` was
not overwritten and the client was not launched. Status: `STATICALLY VERIFIED`,
`AUTOMATED TESTED`, `BUILD_VERIFIED`; not `CLIENT-TESTED`. Existing renderer/font
paths that signal failure through return values or HRESULTs do not become
transactional through exception handling. The font-upload continuation below
covers font texture return/HRESULT failures; other graphics-resource failures
and live visual acceptance remain pending.

Changed files under `tmproject/TMProject748/`: new
`internal/ui/RCControlTransaction.h`, `internal/app/scenes/TMSceneResources.cpp`,
`internal/ui/{SGrid.cpp,SControlContainer.cpp}`,
`internal/ui/controls/{SButton.cpp,SScrollBar.cpp,SPanel.cpp,SProgressBar.cpp}`,
`internal/platform/windows/TMFont2.cpp`,
`tests/{UIBinaryTests.cpp,SceneDisconnectContractTests.cpp}`, plus this document.
No files were removed; other dirty-tree work was preserved.

### Indexed-caption loading continuation, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. Resource bytes, string IDs, inline
caption layouts and wire contracts are unchanged. No new captions were invented
for controls whose text is intentionally empty or supplied dynamically.

Two loader gaps were corrected:

- `ReadUIString` now propagates the transactional table loader's result.
  Startup reports a missing/invalid `UIString.txt` and stops before device/scene
  construction instead of continuing with an empty table.
- RC preflight and caption copying now use the actual 500-row `g_UIString`
  array. The unrelated `MAX_STRING` message-table limit is 2000 and previously
  admitted out-of-bounds caption reads. The production copy helper rejects
  negative/out-of-range indexes, missing nonzero rows and unterminated rows
  without modifying the destination. Index zero retains its intentional blank
  caption; bounded truncation remains supported. A construction-time rejection
  uses the existing RC transaction to unwind newly attached controls.

The current `FieldScene2`, `SelCharScene2` and `SelServerScene2` contain 709,
85 and 22 complete records, respectively. Every button/text index resolves in
the shipped UI table; there are no missing nonzero caption rows in these three
resources. A durable test loads the real table and walks all three binary files
through the production caption-copy helper. This does not prove later-version
widgets, inline captions, dynamic labels, rendering or visual placement.

The focused suite adds 505 checks and passes **1,154,101 checks**, including
static assertions. It covers the actual table boundary (including indexes 500
and 1999), blank/dynamic rows, absent/unterminated captions, short destinations,
missing/empty table files, unchanged destination state after rejection, all
shipped Scene2 references and source bindings for startup failure propagation.
Startup bindings are not execution of the GUI failure path.

Validation commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
and `tools/repository/Test-RepositoryLayout.ps1`. The incremental Release build
passed without new warnings; layout/local links passed with 186 documents
indexed. Scoped English-text and whitespace checks passed. Status:
`STATICALLY VERIFIED`, `AUTOMATED TESTED`, `BUILD_VERIFIED`; not `CLIENT-TESTED`.

Changed files under `tmproject/TMProject748/`:
`internal/ui/UIBinary.h`, `internal/core/{Basedef.cpp,Basedef.h}`,
`internal/app/scenes/{NewApp.cpp,TMSceneResources.cpp}` and
`tests/{UIBinaryTests.cpp,SceneDisconnectContractTests.cpp}`, plus this document.
No files or assets were removed. Installed `client748/project.exe` was not
overwritten and the client was not launched. Visual acceptance remains pending.

### Font texture failure continuation, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. String IDs, wrapping limits, RC
layouts, texture format and packet contracts are unchanged. No native asset
or historical executable was modified. This is failure handling, not evidence
of visual parity or a fix for every missing in-game label.

`TMFont2::SetText` now rejects missing device/GDI resources and invalid mask
extents before rasterization. It checks GDI results, uses a valid black brush,
and supplies the explicit length of the non-terminated space buffer instead of
reading beyond it with `strlen`. Both texture-creation paths check their
HRESULT and release partial/failed textures. The production `UploadMask` helper
checks level format/extent, lock status, bits, positive pitch, row capacity and
pitched extent overflow before writing. A successful lock is unlocked exactly
once, including malformed results; a failed lock is never written or unlocked.
Unlock failure propagates and discards the texture. `Render` stops after a failed
retry instead of drawing an absent or incomplete texture.

The shared font seed loader owns its file handle throughout allocation and
reading. Missing, prefix-only and incomplete reads do not publish cache bytes
or length. The existing four-byte prefix skip, binary payload and 18-byte TGA
trailer area are preserved; previously uninitialized trailer padding is zeroed.
This loader does not validate TGA encoding: that remains the D3DX loader's role.

RC button/text construction checks nonempty caption texture readiness before
attachment, including button alternate text. Failure returns while the local
widget owner and existing RC tree checkpoint can unwind new controls. Empty
captions remain valid. Nonvirtual readiness methods add no instance fields or
virtual-table entries. Deferred editable-text rendering and other widget
graphics-resource failure paths are not covered by these two RC checks.

The focused suite passes **1,154,550 checks** and static assertions, adding 449
checks. It exercises the production upload helper with a simulated texture API:
all 256 alpha inputs, padded rows and guards, null/invalid/overflowing extents,
description failures, incompatible format/dimensions, failed and malformed
locks, unlock rejection, retry and partial-row uploads. Seed tests cover missing
and truncated files, retained prior state, binary payload/trailer preservation
and byte-identical loading of the real runtime `UI/minimap.wyt`. Source bindings
check production calls and RC rejection placement. These are not actual GDI or
Direct3D driver fault injection, nor execution of a widget failure in the GUI.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
and `tools/repository/Test-RepositoryLayout.ps1`. The incremental Release build
passed without new warnings. Status: `STATICALLY VERIFIED`, `AUTOMATED TESTED`,
`BUILD_VERIFIED`; not `CLIENT-TESTED`. No installation or client launch occurred.
GDI creation/cleanup coverage was subsequently added by the follow-up below.
Actual Direct3D reset, other widget/resource failures and live visual acceptance
remain pending.

Changed files under `tmproject/TMProject748/`: new
`internal/platform/windows/FontTextureUpload.h` and
`tests/FontTextureUploadTests.cpp`; updated
`internal/platform/windows/{TMFont2.cpp,TMFont2.h}`,
`internal/ui/controls/{SText.h,SButton.h}`, `internal/app/scenes/TMSceneResources.cpp`,
`tests/{SceneDisconnectContractTests.cpp,PacketViewTests.cpp,ArchitectureTests.vcxproj}`
and `TMProject748.vcxproj`, plus this document. No files were removed; other
dirty-tree work was preserved.

### Font GDI lifecycle follow-up, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. The top-down square 32-bit DIB,
font height/weight/face, `CreateFontA` flags, white text and black background
are preserved. No wire, instance layout, native asset or historical binary
was changed; existing successful-path evidence was reused.

Font resources are now created locally and published only after the DC,
bitmap/storage, font, both selections and both colors succeed. Failure rolls
back partial ownership; cleanup deletes the DC before its selected objects.
Every invalidation clears the pixel view and DIB metadata. If cleanup itself
fails, unreleased handles remain owned for a later retry, without being treated
as a usable font cache. Failed old cleanup prevents replacement allocations.
Finalization also cleans partial initialization state when the base restored
flag was never set, and guards gamma operations against a missing device,
application, window or window DC.

`RestoreDeviceObjects` now returns the actual failed sprite HRESULT instead of
`S_OK`, rejects a missing device and propagates `GetDeviceCaps` failure. GDI
creation failure returns `E_FAIL`, allowing the existing base restore/reset
caller to invalidate partial device objects. The font configuration reader
bounds the 128-byte face and rejects numeric overflow while retaining the
legacy first-token face and nonpositive-weight fallback. Missing files keep
the existing default-face/current-weight behavior.

The focused suite passes **1,154,804 checks** and static assertions, adding 254
checks. Failure-injection tests exercise each creation/selection/color boundary,
bitmap-without-storage, invalid extents, complete/partial caches, cleanup order,
repeated invalidation, retry and failures during cleanup/rollback. Configuration
tests cover empty/truncated input, first-token faces, oversized faces, malformed
weights, overflow and fallback. Sixteen real Win32 GDI create/release cycles
verify bitmap storage, selected objects, colors and top-down row order using
an off-screen pixel write. `GetObject` reports a normalized height on this
machine; the row-order assertion uses actual GDI coordinates rather than
assuming that the returned header retains a negative height.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
and `tools/repository/Test-RepositoryLayout.ps1`. The incremental Release build
passed without new warnings. Status: `STATICALLY VERIFIED`, `AUTOMATED TESTED`,
`BUILD_VERIFIED`; not `CLIENT-TESTED`. No installation, window or game launch
occurred. Real GDI resource tests are not Direct3D driver fault injection or a
device-loss/reset run. The ignored font-reset, world-transform and
vertex-shader initialization results are covered by the device-restore
follow-up below. Other widget/resource failures remain separate follow-ups.
Live UI acceptance remains pending under
the existing restriction on visual client testing.

Changed files under `tmproject/TMProject748/`: new
`internal/platform/windows/FontGdiResources.h` and
`tests/FontGdiResourcesTests.cpp`; updated
`internal/platform/windows/RenderDevice.cpp`,
`tests/{PacketViewTests.cpp,ArchitectureTests.vcxproj}` and `TMProject748.vcxproj`,
plus this document. No files were removed; existing dirty-tree work was preserved.

### Direct3D restore and state-cache follow-up, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. Successful state values, shader
formats/declarations, initialization order, public signatures, instance layout,
wire contracts and assets are unchanged. This internal failure-handling batch
reuses the existing rendering boundary; it makes no new native-parity claim.

The four `RenderDevice` cached setters previously published their requested
value before checking the Direct3D result. A rejected change could then be
skipped on retry. `RenderStateCache.h` publishes only accepted submissions,
preserves the last known value after rejection, and checks device/table bounds.
The existing all-bits numeric sentinel can also be a real parameter value;
such values are always submitted instead of being mistaken for a cache hit.
The helper accepts the existing unsigned 32-bit arrays without changing their
types or dimensions.

Non-owning texture bindings are forgotten before resource invalidation.
Restoration explicitly unbinds the eight cache-supported stages and publishes
each null binding only after acceptance; the previous unchecked loop also
attempted unsupported stages 8 and 9. Failed/unvisited slots remain unknown,
so even a null-binding retry reaches the device. The unknown pointer token is
comparison-only and cannot be submitted through the helper. No texture
ownership is introduced.

`DeviceRestore.h` checks normal/large font reset, world-state initialization
and vertex-shader initialization in their original order. Font HRESULTs are
preserved; the legacy integer initialization failures return `E_FAIL`.
`InitializeRenderingState` now reports failed world-transform submission.
The existing base initialization/reset caller invalidates partial objects
after a failed restore instead of setting its restored flag. Invalidation also
releases all four effect vertex declarations alongside their shaders, matching
their existing finalization ownership and avoiding loss of those references
when shaders are recreated.

The focused suite passes **1,156,871 checks** and static assertions, adding
2,067 checks. Production-helper tests cover all 256 render, 8x29 texture-stage
and 8x14 sampler cache entries, rejection/retry/cache hits, preserved accepted
values, sentinel collisions, invalid indices, missing devices, every failed
unbind slot, partial binding state, repeated unbind and pre-reset-pointer
rebinding. Table-boundary tests use a fake device, not a claim that every enum
or parameter is accepted by a real driver. Restore tests cover optional fonts,
every required-stage failure, exact HRESULTs, ordering, short-circuiting and
successful retry. Source checks bind those helpers and declaration cleanup to
the production methods. No actual Direct3D reset or driver fault injection was
performed.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
`tools/repository/Test-RepositoryLayout.ps1` and scoped `git diff --check`.
The incremental Release build passed without new warnings. Status:
`STATICALLY VERIFIED`, `AUTOMATED TESTED`, `BUILD_VERIFIED`; not
`CLIENT-TESTED`. No installation, window or game launch occurred.

The shader file/buffer allocation, short-read and partial-batch cleanup gaps
were addressed by the follow-up below. Remaining work: base-device back-buffer acquisition checks; other unchecked
viewport/render-block/font-loss calls; actual device-loss/reset and UI visual
acceptance when the existing testing restriction is lifted. Checking the
shader initializer's return does not prove that its internal error paths are
complete. Public void setters still do not propagate errors to draw callers.

Changed files under `tmproject/TMProject748/`: new
`internal/platform/windows/{RenderStateCache.h,DeviceRestore.h}` and
`tests/{RenderStateCacheTests.cpp,DeviceRestoreTests.cpp}`; updated
`internal/platform/windows/{RenderDevice.cpp,RenderDeviceState.cpp}`,
`tests/{SceneDisconnectContractTests.cpp,PacketViewTests.cpp,ArchitectureTests.vcxproj}`
and `TMProject748.vcxproj`, plus this document. No files were removed; existing
dirty-tree work was preserved.

### Shader reading and resource publication follow-up, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. The eight skin, four effect vertex
and six pixel shader paths, bytecode, declarations, projection register and
unsupported-pixel fallback remain unchanged. Public signatures, instance
layout, assets and wire contracts are unchanged. This internal ownership and
failure-handling work reuses the established rendering boundary; it does not
claim newly researched native parity.

`ShaderResources.h` reads shader files through a checked production helper.
It rejects failed, too short, unaligned or oversized 64-bit lengths before
allocation, checks allocation results and buffer outputs, handles segmented
reads, rejects premature EOF/read errors and unexpected trailing bytes, and
checks file closure. A stage/version-prefix and final END envelope check
precedes creation. Local ownership releases the code buffer and closes the
file on failure, including a non-null allocation output returned with a failed
HRESULT. These are file/envelope checks, not a full instruction/operand parser:
Direct3D creation retains semantic validation. Arbitrary malformed bytecode
inside an otherwise valid envelope is not proven safe by this batch.

`RenderDeviceShaders.cpp` checks device presence and capability-query results.
Readiness now requires every shader and declaration, not only slot zero.
Missing entries rebuild their whole family into locally owned arrays, keeping
previous resources intact on failure. Vertex initialization publishes both
skin and effect families only after required reads, declarations, shaders and
projection-constant submission succeed; pixel initialization similarly
publishes only a complete six-shader family. Publication releases replaced
resources, and failure releases all pending outputs. Mode flags are updated
only after successful vertex initialization. The unsupported-pixel path still
disables PS 1.1 and keeps its existing 64-pixel shadow fallback.

The focused suite passes **1,157,409 checks** and static assertions, adding
538 checks. New tests exercise the production read/ownership helper with
missing files, invalid lengths, allocation failure and invalid outputs,
segmented reads, truncated/growing files, read/close failure, wrong stage/END,
byte preservation and cleanup. All 18 required runtime shader fixtures are
accepted byte for byte; missing fixtures fail rather than skip. Batch tests
enumerate every readiness combination for the existing 8/4/6-slot families,
pending-resource failure at each position, preserved previous resources,
retry, replacement, repeated publication and final release. Source checks
bind these helpers and publication ordering to the actual initializer methods.
The failure seams use fake IO/COM-style objects, not a real graphics driver.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
`tools/repository/Test-RepositoryLayout.ps1` and scoped `git diff --check`.
The incremental Release build passed without warnings. Status:
`STATICALLY VERIFIED`, `AUTOMATED TESTED`, `BUILD_VERIFIED`; not
`CLIENT-TESTED`. No installation, window or game launch occurred.

Base-device back-buffer acquisition is addressed by the follow-up below.
Remaining work: other unchecked viewport/render-block/font-loss calls;
bytecode instruction-boundary hardening
if arbitrary shader-file corruption must be supported; actual shader startup,
device-loss/reset and UI visual acceptance after the existing testing
restriction is lifted. These checks do not close the overall compatibility
or native-function implementation effort.

Changed files under `tmproject/TMProject748/`: new
`internal/platform/windows/ShaderResources.h` and `tests/ShaderResourcesTests.cpp`;
updated `internal/platform/windows/RenderDeviceShaders.cpp`,
`tests/{SceneDisconnectContractTests.cpp,PacketViewTests.cpp,ArchitectureTests.vcxproj}`
and `TMProject748.vcxproj`, plus this document. No files were removed; existing
dirty-tree work was preserved.

### Base-device back-buffer and recreation follow-up, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. Checked API results and temporary
COM ownership preserve the existing rendering boundary, public signatures,
instance layout, assets and wire contracts. The existing
[screenshot evidence](../../.agents/research/client748/flows/ui/screenshot-capture.md)
is reused: Print Screen, `ScreenShot\\Capture%04d.jpg`, JPG encoding, collision
search and the requirement for an existing directory are unchanged. No new
native parity claim or catalog promotion is made.

`DeviceBackBuffer.h` initializes acquisition outputs, rejects a missing device
or a successful result without a surface, propagates failing HRESULTs and
releases every non-null surface output, including an output returned with a
failed HRESULT. Descriptions are queried into a local value and published
only on success, preserving the last valid descriptor after a partial write
or failure. Capture never invokes the encoder after failed acquisition and
releases the reference after either encoder success or failure.

`D3DDevice.cpp` uses these helpers for initialization, reset and capture.
Initialization also checks device presence and capability-query failure before
initializing scene resources. Reset cannot reach restoration or publish
restored readiness after a failed back-buffer query. Capture retains the
existing failure log. Both initialization fallback and adapter/type recreation
retain the Direct3D factory across full cleanup; previously cleanup released
the factory before `Initialize3DEnvironment()` reused it. Final teardown still
releases the retained reference. Failed device creation with a non-null output
also enters cleanup rather than leaking that output.

Validation: **1,157,976 checks PASS**, static assertions PASS, adding 567
checks. Fake surface/device tests cover successful and failed acquisition,
null outputs, partial descriptor writes, exact error propagation, preservation
and replacement of the cached descriptor, retry, missing devices, encoder
failure, scope release and refusal to overwrite an already owned reference.
Fake factory tests cover repeated retry cleanup, reference balance and final
release. Source checks bind acquisition ordering, initialization/reset failure
gates, capture format/logging and both factory-retention callers to production.
These tests do not execute the real device initialization/reset/window flow or
a graphics driver.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
`tools/repository/Test-RepositoryLayout.ps1` and scoped `git diff --check`.
The incremental Release build passed without warnings. Status:
`STATICALLY VERIFIED`, `AUTOMATED TESTED`, `BUILD_VERIFIED`; not
`CLIENT-TESTED`. No installation, window or game launch occurred.

Remaining work: ignored invalidation/font-loss HRESULTs and other unchecked
viewport/render-block calls; instruction-boundary validation inside shader
envelopes if arbitrary corruption must be supported; real driver startup,
fallback/reset and screenshot acceptance after the existing visual-testing
restriction is lifted. The legacy screenshot search at its numbering limit
is not changed or newly certified by this batch. This does not close the
overall compatibility or native-function implementation effort.

Changed files under `tmproject/TMProject748/`: new
`internal/platform/windows/DeviceBackBuffer.h` and `tests/DeviceBackBufferTests.cpp`;
updated `internal/platform/windows/D3DDevice.cpp`,
`tests/{SceneDisconnectContractTests.cpp,PacketViewTests.cpp,ArchitectureTests.vcxproj}`
and `TMProject748.vcxproj`, plus this document. No files were removed; existing
dirty-tree work was preserved.

### Frame failure and recovery follow-up, 2026-10-11

Classification: `MODERNIZACAO_COMPATIVEL`. This batch changes internal failure
handling, not gameplay, packet layouts, assets, public signatures, instance
layout or the successful character-preview/control rendering order. No new
native parity claim or catalog promotion is made.

`DeviceFrame.h` gives `RenderDevice::Lock` and `Unlock` strict 0/1 results.
Previously an unexpected cooperative-level or failed-reset HRESULT could be
returned as a nonzero integer and mistaken for successful scene entry. Clear,
BeginScene, EndScene and Present failures now stop their dependent operations;
both device-lost and not-reset results schedule recovery. A recovery attempt
never opens a scene. Failed preparation/reset remains retryable, and a device
that becomes cooperative while resources remain unrestored retries reset
instead of drawing with incomplete resources. Windowed reset preparation
checks factory/adapter presence and queries the display mode into a temporary
value, preserving the prior settings/format on query failure.

`NewApp::RenderScene` checks required dependencies and uses a local scene owner
for every Lock/Unlock segment. Failed scene entry prevents world/control draw;
reported view/state-block failures abort the frame; a failed segment exit
prevents later segments/presentation. An early return closes only a scene that
was opened successfully and does not present it. The redundant application
cooperative-level check is removed so recovery belongs to the device helper.
The preview's existing placement between segments is preserved, not newly
certified by these tests. Public void viewport/state-cache setters still do
not propagate every underlying API error; those remain separate work.

Validation: **1,158,370 checks PASS**, static assertions PASS, adding 394
checks. Fake devices cover combinations of cooperative level, resource
readiness, reset preparation/reset failure, clear/begin/end/present results,
missing devices, failed-reset retry after cooperation returns, next-frame
resumption, exact scene ownership, early returns, and single presentation
across multiple segments. Source checks bind the helpers, display-mode
publication and application failure gates to production callers. These are
isolated and source-contract checks, not execution of the application loop or
a real graphics driver.

Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
`tools/repository/Test-RepositoryLayout.ps1` and scoped whitespace checks.
The test build initially rejected a shadowed local test variable under `/WX`;
renaming that variable resolved it. The final test and incremental Release
builds passed without warnings. Status: `STATICALLY VERIFIED`,
`AUTOMATED TESTED`, `BUILD_VERIFIED`; not `CLIENT-TESTED`. No installation,
window or game launch occurred.

Changed files under `tmproject/TMProject748/`: new
`internal/platform/windows/DeviceFrame.h` and `tests/DeviceFrameTests.cpp`;
updated `internal/platform/windows/RenderDevice.cpp`,
`internal/app/scenes/NewApp.cpp`,
`tests/{SceneDisconnectContractTests.cpp,PacketViewTests.cpp,ArchitectureTests.vcxproj}`
and `TMProject748.vcxproj`, plus this document. No files were removed; existing
dirty-tree work was preserved. Remaining gates include invalidation/font-loss
failure handling, viewport/state-block API propagation, preview segmentation
and real-driver recovery acceptance once visual testing is permitted. This
batch does not close the overall development/parsing/native-function effort.

### UI texture-set follow-up, 2026-10-10

Classification: `MODERNIZACAO_COMPATIVEL`. `UI\UITextureSetList.txt` keeps its
format and content; no asset, RC resource or wire contract changed.

- `TextureManager::InitUITextureSetList` previously used one `fscanf` loop:
  an index at or above 600 was skipped without consuming its rows (the next
  header read then desynchronized), negative indices wrote before the table,
  and a block shorter than its `ItemCount` left the remaining entries
  uninitialized. It now reads the whole file and installs the result of
  `ui_texture_set::Parse` (`internal/render/resources/UITextureSetList.h`).
  Invalid blocks are skipped together with their rows, short blocks keep only
  the rows they have, extra rows are dropped, a repeated index replaces the
  earlier block without leaking it, and anomalies are logged once. The
  set 526 icon-grid override is unchanged.
- Every consumer now resolves a state through `UITextureCoordAt`, which rejects
  absent sets, null rows, negative states and `index == nCount`; the renderer
  previously accepted `index == nCount` and read one entry past the set. Covered
  consumers: both `RenderGeomRectImage` paths, `SProgressBar`, the `TMGround`
  minimap (set 11) and the `TMFont3` digit glyphs.
- The shipped list has 446 sets in 0..456, no duplicates and texture slots
  0..261. Set 198 (`chat_sel2`) declares 5 rows and contains 4; it now has
  `nCount` 4, so a fifth state draws nothing instead of uninitialized
  coordinates.

Validation: `UITextureSetListTests` adds 13 checks covering CRLF/LF parsing,
skipped, short, long and malformed blocks, duplicates, the coordinate guard,
the shipped list and every texture-set reference in FieldScene2,
SelCharScene2 and SelServerScene2 (all resolve). `Build-Client.ps1 -NoDeploy`
passed: `ArchitectureTests` 1,153,266 checks and static assertions,
`SocketReceiveTests` 231 checks. No deployment or client execution was
performed. Status: `STATICALLY VERIFIED`, `AUTOMATED TESTED`; not
`CLIENT-TESTED`.

Changed files: new `internal/render/resources/UITextureSetList.h` and
`tests/UITextureSetListTests.cpp`;
`internal/render/resources/{TextureManager.h,TextureManager.cpp}`,
`internal/platform/windows/{RenderDevicePrimitives.cpp,TMFont3.cpp}`,
`internal/ui/controls/SProgressBar.cpp`,
`internal/render/world/terrain/TMGround.cpp`,
`TMProject748.vcxproj` and `tests/{ArchitectureTests.vcxproj,PacketViewTests.cpp}`
under `tmproject/TMProject748/`, and this document. No files were removed.

### UI texture-set review and continuation, 2026-10-11

The continuation in Claude added the line-based texture-set parser and guards
in the renderer, progress bar, minimap and damage-number drawing described
above. This review preserves that work and closes two remaining boundaries
(`MODERNIZACAO_COMPATIVEL`, no native resource or wire changes):

- `TMFont3` still read glyph widths directly in three constructor paths before
  the guarded draw method ran. Missing rows could therefore fail while laying
  out plain, experience or critical damage text. All three paths now use
  `UITextureGlyphAdvance`, which resolves the guarded coordinate and reserves
  the existing unsupported-character gap when a glyph is absent. Valid glyphs
  retain their prior scaling multiplication and integer rounding order. Drawing
  continues to skip absent glyphs.
- Header conversion did not check `strtol` overflow on Win32, rejected the
  valid lower signed integer boundary, and accepted suffixes hidden after a
  NUL byte. Coordinate conversion also accepted a NUL-hidden eighth field.
  Conversion now checks `ERANGE`, the full string extent and signed limits;
  rejected headers leave their output unchanged. Block-level index/count
  validation remains responsible for rejecting negative or oversized values.

Four regression cases failed before these corrections and passed afterwards.
`UITextureSetListTests` adds 28 checks for these cases, valid glyph scaling and
missing/short/null sets. `ArchitectureTests` passed 1,153,294 checks and static
assertions, including the existing shipped UI and Scene2 fixtures. Live damage
text, CJK IME interaction and the broader input/layout acceptance scenarios
remain pending; automated tests do not establish `CLIENT_TESTED`.

The incremental Release solution build also passed with no new warnings.
Commands: MSBuild `tests/ArchitectureTests.vcxproj /t:Build
/p:Configuration=Release /p:Platform=Win32`, `ArchitectureTests.exe`, MSBuild
`tmproject/WYDESTINY.sln /t:Build /p:Configuration=Release /p:Platform=x86`,
`tools/repository/Test-RepositoryLayout.ps1` and scoped `git diff --check`.
Layout/local links passed for 186 indexed documents and the whitespace check
passed. The generated artifact is `tmproject/build/TMProject748/Release/WYD.exe`;
no deployment or game execution was performed.

Changed files in this review: `internal/render/resources/UITextureSetList.h`,
`internal/platform/windows/TMFont3.cpp`, `tests/UITextureSetListTests.cpp` and
this document. No files were removed. The earlier Claude socket/asset test
results are retained as prior evidence; their inputs were not changed by this
review. The installed `client748/project.exe` remains the October 7 candidate.

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
The override applies to the installed `WYD.exe` or the build artifact when
started with `client748` as its working directory.

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

### Slot 13 familiars and special skills (2026-10-12)

Two regressions from the 7.69 `UpdateScoreUI` replacement were restored from
native evidence (`PARIDADE_NATIVA`, decompiled 7.48 functions already in the
local corpus; no new Ghidra run):

- Familiars: native `FUN_00486808` (equipment swap), `FUN_004a626e`
  (character selection) and `FUN_00485d43` copy `Equip[13]` into the human's
  familiar field, and `FUN_00506f9d` builds Imp (753), Naiads (769),
  Grewpain (1726) and the Angels (3900..3908). The client had zeroed the
  familiar in every projection on the assumption that slot 13 was only a
  costume, so equipped familiars never appeared. `TMHuman::SetFamiliarItem`
  now publishes slot 13 as the familiar for the local player, other players
  and character selection, and disposes a changed pet. Costumes moved to slot
  12 (see the costume-slot extension below). A shadowed player's released
  familiar pointer is also cleared.
- Special skills: `FUN_004431e4` projects `LearnedSkill` bits 24..31 into the
  4x2 skill-panel grid 571 as items 5096..5103 (UI mode 2). The compat
  projection covered only bits 0..23, so Sephira books (Conjure Cannon, Thorn
  Wall, Resurrect, Cancellation, Spectral Force) were learned and saved by the
  server but never appeared in the client. `UpdateCompatLearnedSkillUI` now
  fills those slots (`NativeSpecialSkills.h`) and also clears a slot whose bit
  was removed.

- Inventory X: native `FUN_004662c5` routes control `0x170` to the
  `FUN_00447691` cascade. The client only hid the Inventory panel, so a Cargo
  opened with it stayed open; `case 368` now runs `SetVisibleInventory`.

Validation: `UiModeContractTests.cpp` and the drop-handler contract in
`SceneDisconnectContractTests.cpp`; `Build-Client.ps1 -NoDeploy` passed.
Pending: `CLIENT-TESTED` for all three (equip/unequip a familiar, learn and
drag a special skill, close Cargo through the Inventory X).

### WYD-Go extensions: costume slot, expiry display, timed horses (2026-10-12)

Classification: `EXTENSAO_COORDENADA`. Contracts live in
`wydgo748/internal/model/costume_slot.go` and `timed_display.go`, and in
`tmproject/TMProject748/internal/application/NativeCostumeSlot.h`.

- Costume slot. Native 7.48 equips costumes and familiars in slot 13 and the
  guild medal in slot 12. Medals are server-side state here, so costumes move
  to slot 12 and a costume and a familiar can be worn together. A costume is
  an item-list row with mesh 36 and position 8192 (the 135 costumes, the same
  set as `Costumes-KR.json` plus the base sets); the server catalog loader and
  the client `ItemList.bin` loader both remap its position to 4096. Character
  entry moves a costume persisted in slot 13 to an empty slot 12 with its UID
  and deadline, in the same write as timed-item activation; an occupied slot
  12 leaves it in place. Costume lifetimes now run in slot 12. The client draws
  the costume from slot 12 for the local player, CreateMob and character
  selection, and the medal-gated F action ignores a costume in slot 12.
- Expiry display. `ExpiresUnix` stays server-only. `wire.PutItem` projects it
  into empty effect pairs of the wire copy, the fields the native tooltip
  (`FUN_00418828`) reads: Angels 3900..3979 get day/hour/minute
  (EF_WDAY/EF_HOUR/EF_MIN); other timed items get day/year-2000/month
  (EF_WDAY/EF_YEAR/EF_WMONTH). Times use the server process time zone (`TZ`
  in the unit). Occupied pairs are never overwritten and nothing is persisted.
  `Item.WireEqual` accepts the echoed projection in trade, shops and crafting,
  so a forged byte still fails. The client tooltip shows the date for every
  costume, including imported KR rows above 4420.
- Timed horses 3980..3989. Their item-list rows (client and server) carry no
  lifetime or stats; the client drew stats from `g_pMountBonus2`, while the
  server granted nothing: no speed (Run Mode had no effect), no bonus and no
  expiry. `data/mounts.json` now has a `premium` table mirroring those rows
  (attack, magic %, evasion, resistance, final speed 6) plus the lifetime
  encoded in each name. Activation, expiry and score use it.

Validation: Go `model`, `data` and `game` tests for remapping, migration,
coexistence with a familiar, deadline preservation, projection, echoed and
forged wire items, the premium table and its rejections, and horse
speed/bonus/expiry; full `go vet ./...` and `go test -race ./...` passed.
Client contracts in `UiModeContractTests.cpp`; `Build-Client.ps1 -NoDeploy`
passed. Pending: `CLIENT-TESTED` (equip a costume in slot 12 next to an Angel,
tooltip dates, Run Mode on a Shire Horse) and deployment of server + client
together, since an old client cannot equip costumes in slot 12.

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
6. `tmproject/client748/WYD.exe` is the only executable candidate; the
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

The 2026-10-01 Cargo correction also makes its visual grid optional: the
accepted slot is copied first, and an unbound grid skips only pickup and
replacement, not the shared appearance, score-UI, and hover finalization.
This is `MODERNIZACAO_COMPATIVEL`, not a claim of native null-grid safety.
Three source-contract regressions cover ordering and completion; the
missing-grid regression failed before the patch. The incremental no-deploy
Release build passed with 60,990 architecture checks and 221 socket checks.
DirectX execution and native indirect-call ownership remain pending in the
same research record; no runtime executable was installed.

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
authority, persistence, prices, and outgoing packets were unchanged by that
client safety patch.

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

Remaining boundary after the envelope trace: full downstream native grid/UI
and price parity was not established by that trace alone. The detachment
continuation below resolves the grid/cursor subset, not the complete UI.
The current server's snapshot-based sale
does not exercise this legacy callback. No visual client execution or
installation was performed; the record remains `CONTRACT`, not `CLIENT_TESTED`.

The subsequent arithmetic inspection reused the existing native instruction
export: the response truncates the quarter catalog price, applies integer
two-thirds for quarter prices `5001..10000`, and halves those above `10000`.
The common equipment/Carry price block in `OnPacketSell` contains these bands, while Go
`onSellItem` uses a straight quarter price before its passive and city-tax
adjustments. That response evidence alone does not establish the complete
shop policy. The native loads use x87 `FILD`/`FMUL`. The response now uses
the same ordinary integer calculation as the quote through `NativeSalePrice.h`,
removing intermediate float32 rounding while preserving the existing index
guard and lifecycle. Fixtures cover the discontinuities, `16777223`, and
`INT_MAX`; the response does not receive the quote-only ability-185/item-412
overrides. Negative-price policy and final balance overflow remain outside
this arithmetic correction.
See the existing record's
[response arithmetic](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#response-price-arithmetic)
for addresses, exact boundaries, and the remaining adaptation requirements.

The 2026-10-01 quote continuation traces native grid type 3 and corrects
`SGridControl::MouseOver` through `NativeSaleQuote.h`: ordinary quarter-price
bands, the full-price ability-185 override, item 412 fixed at 800000, item 413
using ordinary bands, and the existing zero-catalog message condition. Integer
division preserves native nonnegative price precision without float32 rounding.
The quote now uses a fixed type-38 lookup proven from the native helper:
signed catalog words and instance bytes, the inclusive 3200..3300 exclusion,
catalog-only mount domains 2330..2389 and 3980..3999, and no refinement scaling.
The query is now shared in `NativeItemVolatile.h`: `BASE_GetItemAbility`
delegates only `EF_VOLATILE` to it, so existing item initialization, movement,
pickup, and use callers no longer retain a separate 7.69 type-38 calculation.
Other ability types, static/no-refinement helpers, and consumer state machines
are unchanged and are not approved by this focused evidence.
See the same record's
[sale quote arithmetic](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#sale-quote-arithmetic)
and [type-38 lookup contract](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#type-38-ability-used-by-the-quote).
Server payment remains unchanged, including the current item-412 quote/payment
mismatch; authoritative special-item/passive/tax policy is still pending.
The latest no-deploy incremental build passed with 60,987 architecture checks
(154 quote arithmetic checks, 2134 shared lookup checks, an exclusive type-38
routing regression, and eighteen response arithmetic/routing checks), 221 socket checks, and
asset/shader gates. No
candidate was installed or visually tested; no `CLIENT_TESTED` claim is made.
The published server tests in `aa5e88fb` characterize 98 current-payment
scenarios, including price-band boundaries, special items, merchant passive,
tax, rollback/retry, replay, caps, and encrypted snapshots. They do not settle
the pending authoritative payment policy and were not rerun for this client-only batch.

The subsequent server ingress correction (`64336041`) reads the complete
source type and position words at bytes 14/16 before sale dispatch. Carry
type `1` and positions `0..62` remain the allowed domain. Low-byte aliases
such as type `0x0101`, position `0x0100`, and negative words no longer sell a
different slot; other invalid sources cannot cancel a live trade before
rejection. The [same evidence record](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md)
links the retained native 20-byte request builder and the ingress regressions.
This is `MODERNIZACAO_COMPATIVEL`: no request format, price, persistence
ordering, or confirmation opcode changed. Tests through `World.handle`
cover eight rejections without side effects, valid edge slots, replay, and
save rollback. The full Go suite and vet passed for that batch; these results
are reused for this documentation update, not rerun as client validation.

The native grid/cursor continuation now resolves the ownership transition:
the concrete grid vptr binds footprint/origin detachment to `FUN_0040f3a3`
and `FUN_0040f55d`. They clear occupancy, compact the list, and transfer the
visual to the caller without destroying it. Existing `PickupItem` and
`PickupAtItem` already match this core behavior; safe clipping and pickup
mesh scaling remain deliberate adaptations.

The global receiver previously described as a manager is actually the cursor:
its constructor publishes `DAT_005ccec0`, and exact slot `+0x98` resolves to
`FUN_0040a147`, the native counterpart of `SCursor::DetachItem`. It clears
the attachment and resets pickup style, with no model/score/appearance update.
The existing sale handler intentionally clears only aliases to the sold visual
before deletion, preserving unrelated interactions instead of copying native
unconditional cursor clearing. No functional patch or build is needed for this
resolved subset. The [evidence record](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#grid-detachment-and-ownership-transfer)
retains the concrete slots, functions, and remaining score/appearance, resource
binding, economic-policy, and real-client gates. This is static verification,
not `CLIENT_TESTED` status.

The appearance continuation resolves the native `FUN_00480a83` wrapper's
preservation gate, ability-41 copies, and refresh call order. The existing
`UpdateMyHuman` has corresponding steps; eighteen source-contract checks at
that stage protected their order without rewriting production code. The incremental
Release/Win32 architecture target passed 61,154 checks. Native callee behavior,
the enclosing human ABI, full score refresh, and
real rendering remain open. The [wrapper evidence](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#appearance-refresh-wrapper)
does not establish full appearance parity or `CLIENT_TESTED` status. No client
installation, product rebuild, or unchanged Go validation was repeated.

The refinement continuation resolves the isolated sixteen-byte `SANC_INFO`
layout, concrete human virtual slots `+0x38/+0x40`, and the scene's `Equip[4]`
refinement refresh. Seventeen layout assertions and two additional checks
passed in the incremental Release/Win32 architecture target (61,156 checks).
It also exposed a preservation difference: native refresh saves active
refinement, while the earlier `UpdateMyHuman` saved the old-state cache used
by costume/shadow lifecycle consumers. Initialization mutates active
refinement, so these snapshots are not interchangeable.
The [refinement evidence](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#refinement-layout-and-concrete-initialization-targets)
records the concrete bindings, inherited-stack continuations, and next gate;
complete appearance parity and real-client execution remain open.

The active-snapshot correction now uses `AppearanceRefinementRefresh.h` in
the actual wrapper. It preserves all sixteen active bytes before rebuilding,
restores only under the native nonzero-first-byte/head-not-32 gate, and leaves
the newly populated packet cache available to unchanged shadow-exit consumers.
Thirty-five new state/source checks cover distinct active/cache values,
cleared input, unsigned first-byte boundaries, head-index exceptions, all
sixteen bytes, repetition, and exactly one rebuild per refresh. With the
consolidated wrapper checks, Release/Win32 architecture validation passed
61,188 checks; the integrated `-NoDeploy` build also passed 221 socket checks
and costume/shader gates. This closes snapshot selection, not the remaining
initialization/costume/shadow rendering gates. No client was installed or run,
and no unchanged Go suite or native export was repeated.

The next orientation correction resolves concrete human slot `+0x48` and
its base virtual dispatch. Native left-weapon ability 21 equal to 101
preserves mesh pitch both mounted and unmounted; ordinary meshes retain
pitch reversal and the mounted full-turn addition. `HumanAnglePolicy.h`
now supplies both guarded production branches without changing logical
movement angles or mounted rider reset. Seventy-nine new checks include
the exact native float bits and weapon-101 bit-preserving pass-through.
Release/Win32 architecture validation passed 61,267 checks; the integrated
`-NoDeploy` build passed 221 socket checks and costume/shader gates.
The [angle evidence](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md#concrete-angle-dispatch-and-weapon-101-exception)
also resolves the matched costume row/parity selector, which already matches
the candidate and requires no asset edit. Full costume post-processing,
enclosing ABI, and real-client rendering remain open. No client was installed
or run, and no unchanged Go suite was repeated.

The base-costume continuation now corrects IDs `4153..4156` after catalog/type
selection and before skin replacement. `BaseCostumeLook.h` applies the native
face/body meshes, zero skins, and six-part refinement/grade clearing while
preserving helmet skin and both weapons. Unrelated IDs and masked aliases
remain unchanged. The [native evidence](../../.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md)
retains the exact instructions and field bindings. Eighty-nine new checks and
seventeen look-layout assertions passed; Release/Win32 architecture validation
passed 61,356 checks, and the integrated `-NoDeploy` build passed 221 socket
checks and costume/shader gates. Class/skeleton selection, skin construction,
enclosing ABI, and real-client rendering remain open. No client was installed
or run; no assets or authoritative server behavior changed.

### First trade-check acknowledgement

The [native acknowledgement contract](../../.agents/research/client748/flows/transport/trade-check-confirmation-contract.md)
now resolves `0x386` to human receiver `FUN_0052EAA9`, its concrete vtable
binding, and consumer `FUN_0052E684`. The consumer looks up control 617
(`TMB_TRADE_MYCHECK`) through the resolved container lookup and writes a
32-bit selected flag. It does not transfer items or gold. The old evidence
record's byte-write and unresolved-consumer descriptions were incorrect.

The active 12-byte header-only contract and optional-UI guards already match
this transition. The first server check acknowledges the owner and sends the
checked `0x383/156` offer to the peer; the second check remains an atomic
server-owned transaction. Published tests in `e92c5749` cover repetition,
revocation, fifteen rejection scenarios, exact decrypted recipient/envelopes,
and absence of first-check transfers or persistence. These unchanged results
are reused, not rerun for a documentation/comment-only batch.

The native size-policy body has a `0x386/12` entry, but its external invocation
remains unresolved; this is not proof of a live native transport rejection
path. Two-client trade, cancellation, disconnect, and logout/relogin remain
pending runtime gates. The acknowledgement record is `CONTRACT`, not
`CLIENT_TESTED`.

### Trade session consumers and emitters

The [native session trace](../../.agents/research/client748/flows/transport/trade-session-envelope.md)
now resolves human dispatch for `0x383` to `FUN_0052DC5D` and `0x384` to
`FUN_0052E2F6`. The offer consumer reads fifteen eight-byte items, gold at
148, check at 152, and opponent at 154. The closure clears local opponent,
check, and hover state before scene-type and panel checks. The previous
adapted handler returned early when the UI container was absent, retaining
stale local trade state. It now skips only visual cleanup in that case,
while preserving null scene/model and local-human guards.

This cleanup-order fix is `MODERNIZACAO_COMPATIVEL`; no packet, asset, or
server rule changed. Four source-contract regressions protect the order and
guards; two assertions failed before the fix. ArchitectureTests now passes
58,670 checks, SocketReceiveTests passes 221, and the incremental Release
`-NoDeploy` build passed with existing legacy warnings. The installed
executable was not replaced or run.

The complete Carry snapshot also invalidates trade, as recorded in the
[native Carry contract](../../.agents/research/client748/flows/ui/carry-snapshot-contract.md).
`OnPacketCarry` now copies all 64 items and Coin even without an inventory
grid, clears opponent/check before closure even of hidden Trade, then invokes
the shared native UI2 inventory toggle. Its closing cascade clears all six
artisan roots and native peers without importing the later mix topology.
Normal-trade closure cannot emit an extra cancellation; AutoTrade's separate
quit behavior is retained. Six source-contract checks failed before this
cascade adaptation and passed afterward; ArchitectureTests passed 58,680
checks, SocketReceiveTests passed 221, and the
incremental Release `-NoDeploy` build passed. No installation or client-runtime
validation was performed. Native offer-removal, other UI profiles, and runtime
panel/cursor lifecycle validation remain open; source assertions do not prove
DirectX/UI execution or complete panel-cascade parity.

The outgoing continuation now traces invitation, acceptance, item insertion,
gold, check, and closure sends. Native insertion searches fifteen local slots,
copies eight-byte items, and writes the low byte of Carry position `x+9*y`;
hiding resets the empty positions to `0xFF`. Item/gold changes revoke checks,
and check toggling is throttled by 2000 ms. Send call sites independently
confirm `0x383/156` and `0x384/12`; the native size-policy body's external
invocation is still unresolved.

The record remains `TRACED`, not full interaction parity: the traced native
local-offer click branch returns without removing an item, whereas the active
client supports removal through an unchanged, server-validated offer snapshot.
Preserve that supported behavior and the active source/visual-ownership
safeguards; neither is to be deleted solely to match a native no-op. Resolve a
different reachable native removal route before claiming parity. The
additional pickup/drop and move routes construct item-use/sale/belt/move
packets rather than a direct trade-offer send; the right-button guards exclude
local offer slots. The drop fallback's scene delegation is now resolved:
Field calls the base scene, which forwards to the concrete container's mouse
callback and consumes a processed event. The container traverses live visible
controls; it is not a separate trade emitter. The separate cursor-world-drop
branch requires left-button down and constructs `0x272/32`, not an offer
removal. Current scene/container code already preserves the traced processed
return; no input source change is needed for this evidence batch. These scoped
findings do not prove global absence.

The offer-field continuation now separates insertion and invitation copying
from Field initialization and full panel cleanup. `DAT_013B71E8` stores a
pointer: the fields are offsets from the loaded ObjectManager, not from the
global's address. The initializer clears the entire 156-byte offer and marks
all fifteen positions empty; both inspected hide branches also perform full
cleanup. An apparent `+0xCE4` write in another function is float UI geometry
on its incoming receiver, not the manager's offer-position array. The record
and focused emitter export retain the decisive receiver/mutation instructions.
This does not establish single-item removal or its absence. A further native
removal trace needs a concrete manager alias/helper path or a native runtime
interaction supplying a new entry; do not repeat the same field query or
already inspected grid/scene roots. Preserve the active server-validated
removal behavior, keep its native-parity claim open, and do not let that
specific evidence gap block independent adaptation fronts.
Full native input-domain proof and two-client lifecycle validation remain
pending. This continuation changed evidence/documentation only, so previous product tests
and builds were not repeated; source checks are not client observations.

### Party intention validation

The server's internal invitation parser now accepts only the recorded
44-byte `0x37F` envelope and its DWORD target at `+40`. Public ingress already
enforced that size; the later-source 48-byte/WORD-at-`+44` fallback was unused
in the live flow and has been removed. Zero and values above `0xFFFF` remain
invalid invitation targets.

The 16-byte `0x37E` removal intention validates the complete DWORD at `+12`
before narrowing to a live entity identity. High-word aliases previously
selected a different member or became a voluntary leave after truncation.
They now leave membership, leadership, persistence, and publication unchanged.
Canonical zero/self leave and leader-authorized removal retain their existing
behavior, including leadership transfer and the encrypted clear/remove frames.

This is `MODERNIZACAO_COMPATIVEL`, reusing the existing
[Party contract](../../.agents/research/client748/flows/ui/party-panel-layout-lifecycle.md).
No packet layout, client callback, experience formula, or persistence contract
changed. Regression tests reproduced six removal aliases and later invitation
envelopes before the fix; coverage also checks repetition, borrowed request
bytes, exact handler envelope rejection, valid boundary IDs, and canonical
leave/removal. Existing invitation, expiry, permissions, party experience,
gameplay-space, and buff consumers remain covered. These are automated server
checks only: no client was built, installed, or run, and the overall 7.48 port
and language migration remain incomplete.

### Auto-trade browse target validation

The existing 16-byte `0x39A ReqTradeList` carries a DWORD target at `+12`.
The server now validates the complete value as a nonzero `uint16` identity
before looking up the ghost shop. Previously the lookup narrowed it first,
allowing high-word aliases to bind the buyer and publish an unrelated shop
snapshot. Rejected values leave the previous browse binding intact and send
no shop list; valid values still require a nearby registered shop and its
matching live owner binding.

This is `MODERNIZACAO_COMPATIVEL`: the request and 196-byte `0x397` response,
virtual clone identity, listing data, purchase validation, and persistence
contract are unchanged. It is not a new native-parity claim. Regression tests
first reproduced three high-word aliases through `World.handle`, then verified
their rejection, repeated invalid requests, borrowed-buffer preservation, and
the canonical encrypted response at both a normal ID and `0xFFFF`. Lifecycle
tests retain the dead/out-of-world buyer, range, missing-shop/owner, and stale
owner-binding rejections. The focused ghost-shop suite also covers purchase,
insufficient gold, atomic rollback, and stock exhaustion. This establishes
automated server coverage only; no client was built, installed, or run.

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

### Legacy skin animation dispatch

The skin-31 attack remap in `TMHuman::SetAnimation` previously cast motion
values `4`, `5`, and `6` to pointers and wrote through them, causing an invalid
low-address access whenever this branch ran. `SkinMotionPolicy.h` now changes
the enum value instead: attacks 1/2/3 select attacks 4/5/6. Other attacks,
travel, death, and emotes remain unchanged, as do other skins. This is
`MODERNIZACAO_COMPATIVEL`: an internal memory-safety correction preserving
the source's existing selection rule, not new native-parity evidence. No
packet, animation table, asset, server state, or ownership contract changes.
The source regression failed before the correction. Executable tests cover
all three remaps and unchanged motions for skin 31 and four other skins.
`Build-Client.ps1 -Configuration Release -NoDeploy` passed with 61,136
architecture checks, 221 socket checks, shader/asset gates, and the incremental
production build. Repository layout/local links and `git diff --check` also
passed. Status: `STATICALLY VERIFIED` / `AUTOMATED TESTED`, not `CLIENT-TESTED`.
The installed executable was not replaced; visible skin-31 attacks in the
running client remain pending.

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

**WYD KR maps (2026-10-06).** The client `Env/AttributeMap.dat` payload, its
`HeightMap.dat` and the server `data/maps` copies are now the WYD KR maps,
byte for byte (product decision: the 7.48 version is modernized with the KR
world data). Same formats and bit meanings (`0x01` town, `0x02` blocked,
`0x08` house, `0x10` portal, `0x40` PK). Relative to 7.48 the KR attribute map
adds 28 portal cells (Noatum `(1052,1708)` <-> Karden Outpost `(3648,3108)`,
Kingdom War Field `(1056,1724)`), removes the PK bit from the dungeon fields
and adjusts town areas; the KR height map differs mostly in fields without
7.48 terrain, plus small changes in Fields 2611 and 1830 (Kefra hall walls).
Of 8,258 server points checked, only NPCGener `(1988,1754)` changed, from
blocked to walkable. The client trailer is `BASE_GetSum` of the payload
(`0x027B1D17`). Attribute payload SHA-256 `B527E85E712DE4FDDF0CE9C833CC089B5E032E15055797C9456D87FAEB54264F`, height
`032E5A1F6433F4DDC9559E6565C139782423A6A67A2875BD82E7D8FC5BD00740`. The
hashes quoted above are the earlier 7.48 identities.

**WYD KR terrain (2026-10-06, `MODERNIZACAO_COMPATIVEL`, client only).** The
whole client `Env` folder (except the attribute/height maps above) now comes
from WYD KR: 123 `Field*.trn`, 119 `Field*.dat`, `Character.trn`,
`EnvTextureList3.bin` and the tile textures; `Mesh/MeshList.txt`,
`Mesh/MeshTextureList.bin`, `object.bin` and the 614 meshes/textures they
reference but the 7.48 runtime lacked were added. No wire or server data
changes. Format deltas handled in source:

- KR `.trn` tiles keep 12 bytes but store `u16` texture indices:
  `[0]` height, `[1]` tile coord, `[2]` mask coord, `[3]` reserved,
  `[4-5]` tile index, `[6-7]` mask index, `[8-11]` color (7.48: byte indices
  at `[1]`/`[3]`). `FileTileInfo` lives in `TerrainTileMapReader.h`.
- `EnvTextureList3.bin` is a 2048-entry modern list: front tiles at
  `index + 10` (10..283), mask tiles at `index + 1024` (7.48: `+ 256`); the
  fixed mask textures move from 344/`nIndex + 286` to `1024 + 88` and
  `nIndex + 1024 + 30` (same `MTile` files). `MAX_ENV_TEXTURE` is 2048.
- `MeshTextureList.bin` has 3048 entries (`MAX_MODEL_TEXTURE` 3048).
- `MeshList.txt` uses indices up to 3097; `MAX_COMMON_MESH` is 3200. With the
  previous 3048 the loader rejected the whole list and the client stopped at
  startup ("Initialize Mesh Manager Failed"). The limits now live in
  `RuntimeAssetLimits.h`, and `RuntimeAssetLimitsTests` checks the shipped
  files against them.
- `object.bin` has 4096 masks with the same encryption and checksum
  (`MAX_OBJECT_MASK` 4096); the first 2048 decoded masks match 7.48 except one.

KR files not loaded by the source (`Field0815/0816/0915/0916.trn` with empty
names and `.map` files) were copied as is. Still missing in both 7.48 and KR:
81 MeshList entries (for example `abox01/02.msa`, referenced by KR fields)
and 162 mesh textures; their objects are skipped as before. SHA-256:
`EnvTextureList3.bin` `8EDD784FA526FD23CA3952C11D3C0AE026808140140E4C5FBE7575A7B669C5B2`,
`object.bin` `2086031704EDCD84D7A714A5E3A413E0EC0D9E9757BBF8400C8E017CC6AD32C0`,
`MeshTextureList.bin` `B553CE580BF73EC5A4D00BDD1D87C6EE71554AA3158A83BDA72DC55CA055FD64`,
`MeshList.txt` `137B706F371444B3D885B1320F00D4571B150D61AB79EC6D51B86B3DA6CC5BCF`.

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

## Quest countdown and fairy duration audit (2026-10-12)

Classification: `EXTENSAO_COORDENADA` for public quest-zone countdowns;
`MODERNIZACAO_COMPATIVEL` for the remaining-time projection and timer lifecycle.
The existing 16-byte `0x3A1` envelope carries a positive duration at offset 12
with signal ID `0x7530`; zero hides the counter. Native dispatch evidence is
recorded in `.agents/research/client748/flows/ui/instance-runtime-counter-controls.md`.
Native HUD construction and teardown remain untraced, so this is not a claim
of complete native visual parity.

Quest admission sends the rounded-up seconds until the existing global
ten-minute reset, not a new personal ten-minute allowance. Leaving the public
zone or its reset sends zero once. Private instance membership owns the same
HUD and is excluded from public quest admission, guards and reset recalls.
Retiring a previous quest flag must not erase an instance countdown.
Character logout clears the flag before another character reuses the session.
The client updates the counter every frame, clamps expired durations to zero,
handles unsigned tick wrap, and hides it when the scene lifecycle ends.

Fairies use remaining days, hours and minutes rather than an absolute expiry
date or an extra unrelated hour suffix. Expiry effects are a wire-copy
projection; authoritative item identity, expiry metadata and stack quantities
remain unchanged. Item echoes tolerate only those presentation differences.

Audit corrections include instance/quest HUD ownership, character-state reset,
and serialized append-only launcher publication. New tests cover admission,
exit, reset, instance overlap, closed sessions, deadline rounding, timer expiry
and wrap, and immutable item projection. Client Release `-NoDeploy` passed
1,158,490 architecture checks, 231 socket checks and 56 source-launcher checks.
The final asset gate passed 1,159,824 architecture checks. Four new sound-table
entries referenced absent files (`menu04-B.wav`, `fireball.wav`, `firearrow.wav`
and `weath03.wav`); they were removed rather than replaced with unrelated audio.
All 333 remaining entries reference existing runtime files. The legacy repeated
ID 220 is preserved; dependency validation does not redefine loader ordering.
No audio listening or native audio-equivalence claim is made.
The source launcher is not the deployed legacy launcher and is not included in
the live patch. Executed-client visual validation remains pending.

The full Go race run passed every package except one game test that exposed
the missing character-reset flag. A JSON game rerun isolated that single
failure; all other game tests, including PostgreSQL committed-disconnect
recovery, passed without race reports. After fixing the reset, focused race
tests for character reset, quest timers and wire projection passed, and
`go vet ./...` passed. This is reused integration evidence plus a verified
repair, not a claim that the entire suite was rerun after the repair. Portal
tests (four), type checking and production build passed; admin-PIN tests
(four) passed. Player settings were excluded from the commit and patch.

## Adaptation sequence

1. Confirm the native flow and the historical reference SHA in Ghidra 7.48.
2. Locate corresponding callers, callees, structs, and assets in live source.
3. Adapt a small packet group or window at a time, removing an incompatible
   path only when evidence and a replacement contract exist.
4. Protect wire/ABI with `static_assert` and byte-for-byte tests.
5. Validate assets and compile. If installation is part of the gate, confirm
   the SHA-256 of `tmproject/client748/WYD.exe`. With `-NoDeploy`, record
   only the artifact in `tmproject/build/` and the pending installation.
6. Test owner, observer, failure, and relogin before promoting behavior.

Do not change several structural packets at once: `STRUCT_SCORE` is embedded
in larger structures, so each change needs a corpus and test for the final
packet. Never add protocol-variant selection during authentication or
elsewhere; this source implements only the 7.48 contract.
