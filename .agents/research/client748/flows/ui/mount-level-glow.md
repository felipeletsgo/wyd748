---
id: mount-level-glow
title: Mounted pet level to owner and observer glow textures
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Mounted pet level to owner and observer glow textures

## Question

Why can an adult mount keep the same apparent glow between levels 1, 70,
and 120 even though the server publishes different refinement grades?

## Evidence boundary

- USED: native 7.48 `FUN_005266e0` equipment decoding and `FUN_004c3eec`
  mesh rendering, from the recorded binary hash and Ghidra corpus.
- USED: native lookup table at VA `0x005baa04`, 13 little-endian int32
  values: `10,1,2,3,4,6,7,7,8,9,10,11,12`.
- USED: active `Effect/EffectTextureList.bin`, 264-byte records. Entries
  440..452 are empty. Entries 234..245 name `Effect/multi001.wys` through
  `multi012.wys`; entry 121 names `Effect/m001.wys`.
- USED: server mount-level visual encoding and appearance publication;
  client `TMHumanAppearance.cpp`, `CMesh.cpp`, focused tests.
- CONTRADICTORY: the imported newer-client mount-specific 440+grade
  texture family has no materialized asset in this 7.48 runtime.
- NOT APPLICABLE: external server sources and fixed-grade premium costumes
  do not establish the ordinary adult-mount rendering contract.

## Native 7.48 flow

### Callers

CreateMob `FUN_004829f2` enters equipment handling through `FUN_00526140`
and its legacy implementation `FUN_005266e0`; equipment slot 14 carries
the item model in the low 12 bits and grade in the high nibble. Updated
equipment uses the same appearance state, which is copied to the mount skin.

### Callees

`FUN_004c3eec` calls effect loader `FUN_004b925c`. In the ordinary multitexture
branch, modern hardware uses `233 + table[grade]` for grades below 13 and
entry 121 otherwise. Its simple-hardware branch uses `233 + grade`.
Grade zero does not enter the positive-refinement texture branch. The
table repeats grades 6/7; native progression is not one unique color per level.

## State and lifecycle

The owner derives the grade from authoritative mount level. Observers decode
the high nibble on CreateMob and UpdateEquip. The server's existing
`floor(level/10)` encoding gives grades 0, 7, and 12 for levels 1, 70, and 120.
`refreshAppearance` broadcasts updates to owner and current observers; late
observers receive the current grade in CreateMob. No new wire state, timer,
save, or entity is introduced. Skin replacement and scene cleanup retain the
existing texture-manager ownership. Logout/relogin reconstructs the grade from
equipment; no local glow choice survives shutdown.

## Wire, ABI, and resources

Unchanged `0x36B` UpdateEquip, 60 bytes, item models at +12; unchanged
CreateMob model array at +34. Slot 14 uses an unsigned uint16 visual code.
Dead mounts publish an empty visual slot without destroying the equipment.

The patch changes only the ordinary adult mount's modern-hardware texture
selection. Other equipment, premium costumes, shader data, texture files, and
the native simple-hardware path remain unchanged. This is `PARIDADE_NATIVA`
for the renderer mapping, not a new client/server protocol.

## Current mapping

`model.VisualItemCode` -> `bodyMesh` -> CreateMob/`playerAppearancePacket` ->
`TMHuman::SetPacketEquipItem` -> mount skin refinement -> `CMesh::RenderMesh`
-> `native_mount_glow::TextureIndex` -> materialized 7.48 effect texture.

## Delta matrix

| Claim | Before | After |
| --- | --- | --- |
| Server mount grade | level/10 | Unchanged |
| Owner/observer publication | High nibble of slot 14 | Unchanged, regression tested |
| Adult mount texture | Missing 440+grade family | Native 233+lookup family |
| Level 1 / 70 / 120 | Grades 0 / 7 / 12, missing textures | No refinement overlay / entry 240 / entry 245 |

## Decisions

Repair the renderer consumer instead of inventing new grades or copying
later-client assets. Preserve native steps, including equal intermediate
grades. Scope is equipped adult mounts; a following juvenile is a distinct
entity flow and is not implicitly covered.

## Gaps

Owner/observer visuals, different mount species and graphic settings still
require real in-game testing. Published in patch 10024; no `CLIENT_TESTED` claim.

## Validation

Independent C++ texture oracle and server encrypted owner/observer update and
late-view tests cover levels 1..120 and the 69/70 and 119/120 boundaries.
The native table is reproducible by mapping VA 0x005baa04 through the binary's
PE sections and reading 13 int32 values. Runtime effect records are read at
`index * 264`; file bytes are not changed. Build results follow execution.

## Independent mount magic correction

Classification: `MODERNIZACAO_COMPATIVEL`, without a wire change. The server
excluded slot 14 from ordinary item aggregation, then added adult mount magic
as flat displayed damage. Spells instead consume MagicAmp, so they received
no mount bonus. Include living adult mount EF_MAGIC before the shared
`(EF_MAGIC + EF_MAGICADD + 1) / 4` conversion and remove the flat addition.
Timed premium mounts retain their exact percentage and now apply it to magical
skills as well as the displayed score. Recalculation never writes into base stats.

Native FUN_0054eb20 aggregates equipped items through FUN_0054cd07. The latter
uses `(level + 15) * mountMagic / 100` for living adult mount EF_MAGIC (0x3c).
FUN_00557aff consumes magic amplification before the 5/4 skill factor. The
historical W2PP Basedef.cpp corroborates the /4 conversion, but is secondary
evidence. The project's existing broader mount eligibility is unchanged; this
does not claim native parity for all 30 catalog species.

AUTOMATED TESTED: all 30 catalog types at every level 1..120, displayed score,
spell damage, encoded score fields, repeated recalculation, dead/baby/egg/wrong
slot/missing catalog, death and unequip transitions, and active/expired premium
mounts. Full Go suite and vet passed. Release/Win32 -NoDeploy build passed with
1,209,006 architecture checks, 231 socket, 83 crash capture and 56 launcher
checks. Published in server release `20261014T234237Z` and patch 10024; see
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.
Not CLIENT_TESTED; the installed local runtime was not overwritten.
