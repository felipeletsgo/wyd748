---
id: skill-belt-page-selection
title: Select the skill-belt page when dropping a skill
subsystem: ui
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-23
---

## Question

Which page receives a shortcut when a skill is dropped onto the second belt?
Scope: selecting offset 0 or 10, without changing the wire contract or skills.

## Evidence boundary

- USED: read-only Ghidra project `WYD748Native_20260821` with no analysis,
  export `exports/skill-belt-page-flow.tsv`, and the SHA-256 verified on its
  `program` line. The export completed without a script error. Inspected
  `FUN_00416196`, `FUN_004209FC`, `FUN_00435B13`, and `FUN_004470B9`.
- USED: current `SGrid::SellItem/SellItem2`,
  `TMFieldScene::InitializeCompatSkillBelts/UpdateSkillBelt`, and
  `ResourceControl.h`.
- USED: materialized `FieldScene2.bin` controls in `client-debug.log`:
  grids 573/586 under root 5745 and grid 571 under root 1905. The log proves
  those IDs, not execution of this change in the built client.
- USED: WYD-Go `onSetShortSkill/filterShortSkills` and `wire.SetShortSkill`.
- NOT APPLICABLE: later TMProject and external guides as native proof. No
  new asset or protocol field is introduced.

## Native 7.48 flow

### Callers

`FUN_004209FC` calls `FUN_00416196` at `00420EBD` after hit-testing a grid
with an attached cursor item on event `0x202`. The receiver is the hit grid.

### Callees

The grid-type-5 branch of `FUN_00416196` checks control ID `0x24A` (586) and
uses offset 10 for it; other grids use offset 0. It removes the previous
visual through slot `+0xA4`, creates a replacement through `FUN_0040D13E`,
and adds it through slot `+0x8C`. After detaching the cursor and destroying
the previous visual, it writes `ShortSkill[x + offset]`, sends through
`FUN_0055F2DD`, and refreshes through `FUN_004431E4/FUN_004470B9`.
`FUN_004470B9` reconstructs each ten-slot page from `ShortSkill[0..9]` and
`ShortSkill[10..19]`. `FUN_00435B13` binds control 586 to page two.

## State and lifecycle

The page is selected per event without persistent state or new ownership.
The container owns the grids; the scene borrows their bindings, recreated by
`InitializeCompatSkillBelts` on scene entry and relogin. The grid owns its
visuals; the caller destroys the detached predecessor after clearing cursor
aliases. Scene teardown still owns close/shutdown. This page-selection change
does not retain pointers, create timers, or change allocation behavior.

## Wire, ABI, and resources

Unchanged. The existing emitter sends `0x378`/32 bytes with twenty shortcuts;
the server filters unlearned skills and returns authoritative state. This
record traces page selection, not the complete wire contract. Native controls
already bound: 573 (first page), 586 (second page), buttons 587/588.

## Current mapping

`SellItem` and `SellItem2` formerly compared `m_dwControlID` with later ID
65645, absent from the materialized 7.48 grids. The scene already bound
`m_pGridSkillBelt3` to 586, but those consumers ignored the binding. Both
callers now use grid identity for the offset. The shared
`WYD748_ReplaceGridVisual` helper rejects an `AddItem` failure before
detaching the cursor, changing the shortcut array, or sending the packet.
On acceptance it removes the old visual, restores occupancy, and leaves
cursor detachment and old-visual destruction to the caller. The old record's
unspecified failure-rollback gap was therefore stale for the current source.

## Delta matrix

| Claim | Native 7.48 | Earlier source | Decision |
| --- | --- | --- | --- |
| Second page | ID 586 selects +10 | ID 65645 selected +0 | Use the bound `m_pGridSkillBelt3` identity |
| First page | +0 | +0 | Preserve |
| Ownership and send | Replace visual and send twenty entries | Same flow | Preserve in this change |

## Decisions

`MODERNIZACAO_COMPATIVEL`: use the existing scene binding in both callers,
without a second ID table. The resulting page selection matches the traced
native transition. No class layout, vtable, resource, packet, or server-side
authority changes. The current insertion rejection already preserves state;
do not add a second rollback path without a demonstrated failure.

## Gaps

Real-client verification of drops on both pages, selection, switching with
Z/587/588, and relogin remains pending. The insertion-failure path has not
been independently exercised in the built client. Do not claim
`CLIENT_TESTED` from static inspection or isolated tests.

## Validation

Native research and current-source inspection cover the page-selection cause
and the rejection ordering. Existing grid-insertion tests cover rejection
without mutation, but not a real skill-belt drop. No new client execution
was attempted; real-client testing is unavailable on this machine.
