---
id: grid-item-quantity-and-cp-names
title: Quantity text and CP-driven name presentation
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-07
---

# Quantity text and CP-driven name presentation

## Question

Why are item amounts/split captions absent, and how should player CP refreshes
affect name colors without rebuilding an existing entity?

## Evidence boundary

Native: read-only `references/client748/WYD.exe`, hash above. Ghidra 12.1.2
project/corpus: ignored `tmproject/build/native-research/`; targeted inspection
script `.agents/research/InspectInventoryName748.java` and ignored
`tmproject/build/inventory-name-native.log`. Existing decompilations were reused.
Active resource: `tmproject/client748/UI/strdef.bin`, 440 records of 128 bytes;
row 408 is empty. Historical server evidence in local `references/W2PP-master/`
is secondary and does not establish native parity for CP rewards.

## Native 7.48 flow

### Callers

`FUN_00492e7d` dispatches CreateMob variants `0x363/0x364` to `FUN_004829f2`.
That consumer creates/refreshes humans and calls `FUN_004fff0b` for score/name
presentation. Grid item construction and virtual frame rendering enter
`FUN_0040d13e` and `FUN_0040dd00`, respectively; this record closes the text
node contract, not every caller of the grid constructor.

### Callees

`FUN_0040d13e` obtains effect 61 and formats `%2d` into the text control at
receiver offset `0x39a`, not into the item sprite node. It excludes mount IDs
2330–2389. `FUN_0040dd00` queues the visual and separately queues nonempty text
at the centered footprint/parent layer. `FUN_004fff0b` interpolates the native
name palette at `0x005bb464` and applies the label color/border. The palette is
black, red, orange, yellow, pale yellow, green, cyan, white, and pale violet.
`FUN_00486808` handles `0x376` by swapping local grid cells and finishing the
cursor; later authoritative item snapshots must follow this acknowledgement.

## State and lifecycle

Amounts/refinement are initialized in the text node, queued independently of
sprite/mesh rendering, and detached during item teardown. Native CreateMob
places player raw CP at packet offset `0x1e` (`MobName[12]`), removes that byte
from name text, and refreshes existing entities. New native NPCs use raw CP 75,
so the native default NPC name is yellow. A metadata refresh does not require
new entity ownership or a movement reset in the active adaptation.

## Wire, ABI, and resources

Raw CP is signed CP plus 75. In `FUN_004fff0b`, raw values below 10 select black
with border; values above 150 clamp. Otherwise index is `(rawCP - 5) / 20`,
fraction is `(rawCP - (index * 20 + 5)) / 20`, and each RGB channel is linearly
interpolated then truncated. Signed zero gives `0xFFFFFF4C`; +65 gives
`0xFFE5FFFF`; +70 gives white; −65 gives `0xFF3F0000`.

Native split controls are panel 626, edit 627, confirm 628, cancel 629, and
caption 630. The modern mapping retains the existing control binding and
request. Empty resource row 408 receives an English source fallback.
No opcode/size, resource ID, persisted CP representation, or historical asset
has been patched.

## Current mapping

`internal/ui/GridItemPresentation.h` supplies the caption and independent text
queueing used by `SGridInput.cpp` and both `SGridItem.cpp` draw paths.
`internal/ui/NameColorPolicy748.h` implements interpolation and the explicit
white NPC policy. `TMHuman::UpdateNameColor748` is shared by initial score
presentation and `TMFieldScene::OnPacketCreateMobCompat` metadata refresh.
Both label projections restrict the raw-zero red pulse to player IDs.

Server rules and persistence/publication are versioned in
[the integrated contract](../../../../../DOCS/client/inventory-stack-and-cp-contract.md).

## Delta matrix

| Claim | Native 7.48 | Current adaptation | Decision |
| --- | --- | --- | --- |
| Amount label | Separate text draw node; mount exclusion | Same node separation and both draw paths | `PARIDADE_NATIVA` |
| Player color | Raw CP metadata, palette interpolation | Same bounds, truncation, metadata refresh | `PARIDADE_NATIVA` |
| Empty split caption | Active asset row is empty | Source fallback, nonempty captions preserved | `MODERNIZACAO_COMPATIVEL` |
| NPC white | Native raw 75 yields yellow | IDs at or above 1000 explicitly white | `MODERNIZACAO_COMPATIVEL`, intentional product policy |
| Five CP per level | Historical server, not proof from client | Authoritative level reward with persistence tests | `MODERNIZACAO_COMPATIVEL` |
| Server merge policy | Native ACK swaps local cells | Detached server merge, ACK then snapshots | `MODERNIZACAO_COMPATIVEL`; no native server parity claim |

## Decisions

Preserve native player interpolation and mount semantics. Do not mistake CP
zero for white or native NPC defaults for the requested white policy. Consume
CP updates on existing humans without rebuilding their skins. Use existing
packet envelopes and durable acknowledgements; never let a client quantity
or color determine authoritative state. Historical binaries remain read-only.

## Gaps

Real-client rendering, text placement, split confirmation, drag merging, and
CP refresh during movement are not executed in this session. Builds and source
contracts do not prove those observations. Broader grid/HUD lifecycle and
native server reward/merge algorithms are outside this bounded record.

## Validation

- Research: native Ghidra/decompiled functions, exact palette, metadata offsets,
  and active empty resource row checked; existing evidence reused.
- Automation: 1,152,647 C++ architecture checks/static assertions pass; new Go
  CP/stack gates pass. Affected race integration retains only the five known
  S2 closure failures, not a clean acceptance result.
- Build: incremental Release/Win32 candidate and native Go server pass.
- Real client: not run; no `CLIENT_TESTED` claim or installed executable change.
