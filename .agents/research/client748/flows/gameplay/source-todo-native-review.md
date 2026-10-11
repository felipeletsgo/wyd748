---
id: source-todo-native-review
title: Resolve suspicious source TODOs against the native client
subsystem: gameplay
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-07
---

# Resolve suspicious source TODOs against the native client

## Question

Five TODOs inherited from the TMProject 7.69 source marked code whose indexes
or conditions were unconfirmed. Which native 7.48 behavior does each one have,
and where does the source diverge?

## Evidence boundary

- Executable/hash: `references/client748/WYD.exe`, SHA-256 above.
- Ghidra project/corpus: `tmproject/build/native-research/decompiled`.
- Assets: the cape length table was read from the PE image at `0x005BB488`.
- Current source: `TMSky.cpp`, `TMGroundRender.cpp`, `TMItem.cpp`,
  `TMHumanRender.cpp`, `TMHumanCombat.cpp` under `tmproject/TMProject748`.
- Server: not involved; all five are client rendering or targeting rules.

## Native 7.48 flow

### Observable entry

- Event: field frame (sky, ground, items, humans) and a click to attack.
- Preconditions and initial state: in the field scene.
- Observable output: sky/light color, item tint, cape position, whether an
  attack starts.

### Callers

Field render and attack paths; not needed to decide the five deltas.

### Main function

- `FUN_005484d1` (sky): blends the clear color and light of the previous sky
  texture into the next one, both indexed by `texture - 0x43` (67).
- `FUN_004f59b4` (ground item): meshes 737..739 (`0x2E0 < mesh < 0x2E4`) tint
  every vertex with `{0xFFFF4400, 0xFF3366FF, 0xFF00FF00}[mesh - 737]`
  (the decompiled stack base `auStack_bac` is 737 DWORDs below the table) and
  render texture `0xCC`.
- `FUN_00503a8c` (human cape): skin types 1/2/4/8/0x26 map to table classes
  0/1/2/3/1, and the length is `table[class * 20 + mesh]` for coat meshes below
  40 or `table[80 + class * 20 + mesh - 40]` otherwise (Ghidra folds `-0xA0`
  into `DAT_005BB528`). The table holds 8 x 20 floats. Capes `0xC7D..0xC7F`
  use length `0.12` and lift `0.04`. Skin types 0x19/0x1A/0x21/0x28 use fixed
  offsets instead of the table.
- `FUN_0051a939` (move attack): when not PK and the target is a summon
  (`+0x249 == 1`) of another guild (`warGuild != guild`, guild set), the
  attack is refused outside the castle area while no castle war runs, and
  always when both have the same cape (`+0x79B`). Outside the castle means
  `x >> 7 < 17 || x >> 7 > 19 || y >> 7 < 30`.

### Callees

Rendering device calls and `FUN_004be1ef` (cape render); unchanged.

### Outputs and errors

None beyond rendering and the refused attack.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| Sky transition | state 10..13 | `FUN_005484d1` | blended clear color and light | none | none |
| Move attack on enemy-guild summon | not PK, no castle war, outside castle | `FUN_0051a939` | no attack | none | return |

### Vtables, vptrs, and receivers

N/A: no ownership or virtual dispatch changed.

### Ownership

N/A.

### Partial failure

N/A.

### Cleanup and teardown

N/A.

### Shutdown

N/A.

### Logout and relogin

N/A.

## Wire, ABI, and resources

No wire or ABI change. The cape table is static data copied from the native
image into `MantuaLength.h`.

## Current mapping

### Buildable source

- Sky: the `-264` doubt is resolved; native uses `-67`. A sky mesh reloaded
  by the mesh cache returns its file texture, so `SkyStateIndex` clamps to the
  four states. The transition now blends the `Env/Lighting.txt` sun colors the
  settled state already used, instead of the old `m_LightVal` table, so the
  light no longer jumps when a transition ends (`MODERNIZACAO_COMPATIVEL`).
- Ground Voodoo path: `m_bVoodoo` is only ever set to 0, so the code is dead.
  The TODO became a note about its edge-tile indexing; no behavior change.
- Item tint: the 7.69 offset `mesh - 2980` read far outside the three colors
  for meshes 737..739. Fixed to `mesh - 737` (`PARIDADE_NATIVA`).
- Cape length: the source had only the first 4 x 20 rows, read row-crossing
  meshes 20..39 past the end for class 3, and reused the first rows for meshes
  40+. `MantuaLength` now holds the native 8 x 20 table (first half identical
  to the old one) and clamps unknown meshes to the class default. Capes
  3197..3199 now use the native 0.12 / 0.04 instead of 0.11 / 0.05
  (`PARIDADE_NATIVA`).
- Move attack: the 7.69 block required castle war and the same guild as the
  war guild. It now matches `FUN_0051a939`, and the field click path
  (`TMFieldSceneCombat.cpp`) already used the same `!=` guild test
  (`PARIDADE_NATIVA`).

### WYD-Go

Not involved; the server remains authoritative for attacks.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Sky index | texture - 67 | clamped texture - 67 | -67 with TODO | n/a | keep, clamp |
| Item tint index | mesh - 737 | mesh - 737 | mesh - 2980 | n/a | fixed |
| Cape table | 8 x 20, second half for 40+ | native table | 4 x 20 | n/a | fixed |
| Cape 3197..3199 | 0.12 / 0.04 | 0.12 / 0.04 | 0.11 / 0.05 | n/a | fixed |
| Cape fixed offsets | by skin class 25/26/33/40 | by skin class | by face item 22..26 | n/a | fixed |
| Summon protection | other guild, no war, outside | same | same guild, war only | n/a | fixed |

## Decisions

- Apply the native index and table fixes directly in source.
- Owner decision 2026-10-07: cape placement follows the native skin class.
  The face items are Wolf_Master 22 and Bear_Master 23 (class 26), Astaroth
  24 (33), Titan 25 (40) and Beast_Master 26 (class 4, mesh 40). Wolf, Bear
  and Astaroth were already equal; the Titan cape loses the 7.69 `+0.05`
  lift, the Beast Master face 26 uses the table by coat mesh, and Troll skins
  (25) get the native `0.07 / 1.2 / -0.12`. The extra lengths for capes
  573/1767/1770 stay (not in native; kept until shown incompatible).

## Gaps

- The move-attack source still has 7.69-only checks (class 66 shadow, the
  2362..2370 x 3927..3935 zone) that this review did not confirm natively.

## Validation

- Research: decompiled functions and the PE table read on 2026-10-07; the old
  4 x 20 table matched the native first half value for value.
- Automation: `ArchitectureTests` 1,152,681 checks PASS, including
  `MantuaLengthTests`, `SkyStateIndex` checks in `LightingProfileTests`, and
  source contracts for the item tint and move-attack filter. Release build
  installed as `project.exe` SHA-256
  `E3F4B069602E3D14D1112AF83D51F8FCD438D4E6B8527366A7C64AC04F2CDA22`.
- Real client: pending (user visual test): sky transitions, ground items with
  meshes 737..739, capes on coat meshes 40+ and on the Titan and Beast
  Master face 26 transformations, and attacking an enemy-guild
  summon outside the castle.
