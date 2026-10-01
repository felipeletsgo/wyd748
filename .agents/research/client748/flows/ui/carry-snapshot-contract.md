---
id: carry-snapshot-contract
title: Complete Carry snapshot contract
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-30
---

# Complete Carry snapshot

## Question

How does native 7.48 receive `0x185`, replace the interactive inventory,
retain 64 structural items with only 63 visual cells, and invalidate trade?

## Evidence boundary

- USED: the native binary identified above and the existing Ghidra
  decompilations of `FUN_0052EAA9`, `FUN_0052E3C8`, `FUN_0040D13E`,
  and `FUN_0055890A`. The same-identity Carry consumer is already available
  in the ignored `tmproject/build/native-research/FUN_0052e3c8.c` cache;
  no repeated export or binary analysis was required.
- USED: current `MSG_Carry`, `TMHuman::OnPacketCarry`, the 9x7 grid,
  `SGridControlItem` ownership, and `ReceivedPacketDispatch`.
- USED: WYD-Go `wire.UpdateCarry`, `model.MaxCarry`, existing emitters and
  tests for login, commands, purchase, trade, AutoTrade, quests, and mixing.
  These unchanged builders remain authoritative for inventory and Coin.
- USED: existing [trade UI lifecycle](trade-inventory-layout.md) and
  [trade emitters](../../exports/trade-session-emitters.tsv), which establish
  `FUN_0044B890(0)` closure and its cancellation guard.
- USED: cached native `FUN_00447691` and initializer `FUN_00435B13`, plus the
  [UI2 cascade export](../../exports/carry-panel-cascade.tsv), containing 426
  instructions and 241 references. A focused read-only/no-analysis inspection
  of `FUN_00447F47` closes the grid-reset helper boundary. No binary was changed.
- NOT APPLICABLE: the KR guide does not define this wire contract; no new
  asset is needed. External legacy sources were not consulted.

## Native 7.48 flow

### Callers

`FUN_0052EAA9 @ 0x0052EAA9` routes `0x185` to
`FUN_0052E3C8 @ 0x0052E3C8` for the human selected by `Header.ID`.
`FUN_0055890A @ 0x0055890A` requires `0x210`/528 bytes.

### Callees

`FUN_0052E3C8` removes the grid's 63 cells, clears matching cursor aliases,
copies `0x200`/512 item bytes from packet `+0x0C`, and materializes indexes
`>40` through `FUN_0040D13E` at `x=slot%9, y=slot/9`. Visual iteration
stops at 63; the logical copy retains 64 `STRUCT_ITEM` values.

After copying Coin from packet `+0x20C` to manager `+0x704`, the consumer
clears opponent at manager `+0xCFA` and check at `+0xCF8`. In Field
(scene type 30000), it calls `FUN_0044B890(0)` and `FUN_00447691`, then
`FUN_004431E4(0)` updates the score UI. Clearing the opponent before
closure prevents the closure path from emitting an extra cancellation.

### Shared UI2 inventory cascade

`FUN_00447691 @ 0x00447691` first forces Inventory and Character hidden if
Gamble is visible. It captures the inventory toggle target before closing
AutoTrade and Gamble, because AutoTrade closure itself hides Carry. For UI2,
closing clears all six artisan packets, hides their roots and Cargo, Shop,
Hellgate, and InputGold, resets grids through `FUN_00447F47`, closes visible
Trade, and detaches the cursor item. Both directions update skill-button
selection from the Skill panel, apply the captured target, and play sound 51.
Opening does not import the other UI profiles' Skill/SkillM closing branch.

Initializer bindings establish Inventory 257, Character 513, Cargo 1825,
Shop 1793, Hellgate 6185, Gamble 6400, InputGold 626, and Skill 1905. The six
artisan roots are 1360, 6110, 6145, 6432, 6481, and 6512; skill button is 295.
Panel visibility uses virtual slot `+0x60`, control lookup `+0x48`, and skill
selection `+0x8C`. `FUN_00447F47` resets Carry grid state and equipment state 1.

## State and lifecycle

The active consumer requires a payload, model, Field scene, and local human.
It copies all 64 Carry items and Coin, then clears opponent/check state
before examining optional controls. A missing inventory grid no longer
discards authoritative state or trade invalidation.

With a control container, the consumer calls trade closure even when Trade
is hidden, then the inventory toggle and score projection. Normal closure
hides Carry first, so the subsequent toggle opens it. The cancellation guard
sees the cleared opponent and cannot emit an extra normal-trade cancellation.
This does not forbid AutoTrade's existing quit intention when its Cargo
chooser is active. Absent controls do not prevent authoritative flag cleanup.

The compatible UI2 toggle now shares native peer cleanup with I/menu input.
Optional panels and cursor are guarded; existing scene ownership is retained.
The implementation groups independent panel cleanup rather than reproducing
each native visibility call's order. Snapshot UI cleanup remains before grid
projection, unlike native projection-before-closure; runtime validation of
dragging and transient aliases is still pending. No new control is allocated.

If the inventory grid exists, `Empty()` releases old visuals and interaction
aliases before rebuilding. Allocated items transfer ownership only when
`AddItem` accepts the cell. Allocation/insertion failure leaves the model
intact and omits only the affected visual.

Structural slot 63 crosses wire, cache, and persistence, but is neither drawn
nor exposed as an input destination. The frame is borrowed only for the
callback. Repeated snapshots replace state rather than accumulating it.
Relogin and map changes receive a fresh snapshot; scene teardown owns controls.
No new worker, subscription, or independent shutdown lifetime is introduced.

## Wire, ABI, and resources

`0x185`, S->C, 528 bytes:

| Offset | Size | Field |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` selects the human |
| `0x0C` | 512 | `STRUCT_ITEM Carry[64]`, eight bytes each |
| `0x20C` | 4 | Coin u32 |

`CarrySnapshotContract.h` names opcode, size, offsets, and capacities.
`Basedef.h` asserts layout, array size, and Coin offset. Dispatch rejects
actual/declared size and Type/opcode mismatches before entity traversal.
The resource remains the existing 9x7 grid; no control is created.

## Current mapping

Source: `tmproject/TMProject748/internal/game/entities/TMHuman.cpp` ::
`TMHuman::OnPacketCarry`; optional UI closure delegates to
`tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp` ::
`TMFieldScene::SetVisibleTrade` and `TMFieldScene::SetVisibleInventory`.

The consumer retains native row-major slots and visible indexes 0..62.
Previously, its initial guard discarded the snapshot when the grid was
missing, and it never invalidated trade. State copying and flag cleanup
now precede optional presentation. Source-contract regressions in
`SceneDisconnectContractTests.cpp` protect receiver guards, data retention,
cleanup-before-close, hidden-trade closure before the inventory toggle,
visual ownership, and five additional UI2 cascade properties. Artisan cleanup
reuses `ClearNativeMix` rather than importing the later mix/mission topology.

Server: `wire.UpdateCarry` writes at most `model.MaxCarry` items; oversized
slices cannot overwrite Coin, and shorter slices leave remaining slots zero.
All existing emitters continue using that builder. This batch changes no
server behavior, packet, asset, or persistence rule.

## Delta matrix

| Claim | Native 7.48 | Previous source/Go | Current decision | Classification |
| --- | --- | --- | --- | --- |
| Envelope | `0x185/528` | 528 bytes without gate | retain gate/asserts | `PARIDADE_NATIVA` |
| Array | 64 copied items | 64 structural items | retain all 64 | unchanged |
| UI | 63 cells, 9x7 | same projection | retain slots 0..62 | unchanged |
| Trade flags | clear opponent/check before closure | not cleared by Carry | invalidate before optional closure | `PARIDADE_NATIVA` |
| Missing presentation | native assumes controls exist | dropped complete snapshot | retain cache/flags; skip unavailable UI | `MODERNIZACAO_COMPATIVEL` |
| UI2 inventory visibility | shared toggle/cascade after trade closure | direct toggle; snapshot skipped it | restore native UI2 cleanup and snapshot invocation | `PARIDADE_NATIVA` |
| Oversized Go slice | N/A | could overrun buffer/panic | bound to 64 | `MODERNIZACAO_COMPATIVEL` |

## Decisions

- Preserve the exact native 528-byte envelope and 64-item array.
- Keep structural slot 63 outside the visual grid.
- Reject invalid frames before destructive visual rebuilding.
- Apply valid authoritative state independently of optional presentation.
- Clear trade flags before closing UI; suppress extra normal-trade cancellation.
- Share the proven UI2 inventory toggle/cascade with the Carry consumer;
  retain AutoTrade's independently proven quit behavior and guarded controls.
- Keep the bounded Go builder and all existing valid callers unchanged.

## Gaps

- Execute login, purchase, trade, AutoTrade, quests, and mixing in the candidate.
- Exercise snapshots while dragging, with full inventory, during map changes,
  and after relogin before promoting to `CLIENT_TESTED`.
- Confirm hidden/visible-trade closure, no extra normal-trade cancellation, and missing
  presentation behavior in the real built client. Source-order tests alone
  do not execute the consumer or DirectX/UI ownership.
- Exercise the UI2 cascade with each artisan root, Gamble, AutoTrade, Cargo,
  Shop, and cursor ownership; source assertions do not execute those controls.
- Other native UI profiles and native local-offer removal remain independent
  gaps; whole-snapshot invalidation does not prove an item-removal interaction.

## Validation

- Existing native evidence establishes dispatch, consumer, size, offsets,
  capacity, visual formula, and trade cleanup order.
- Historical wire automation covers 528 truncated prefixes, oversized/null
  frames, Type/Size mismatch, receiver, slots 0/62/63, Coin, single dispatch,
  and preservation. The existing Go test uses 65 entries and checks the
  structural bound without overwriting Coin. The recorded historical full
  Go suite passed; it was not rerun for this client-only lifecycle patch.
- Historical installed candidate:
  `39117672AAA8DD939CFB2B503344932195E4B179F5812AB9D28AE8F2E990FA6D`.
  This is prior evidence, not the current installed executable's identity.
- The preceding 2026-09-30 snapshot patch passed 58,675 architecture checks.
  For this UI2 batch, six source-contract checks failed before adaptation
  (one updated Carry assertion and five new cascade assertions), then passed.
  ArchitectureTests passed 58,680 checks and static assertions;
  SocketReceiveTests passed 221.
  The incremental Release `Build-Client.ps1 -NoDeploy` build passed with
  existing signedness warnings. No executable was installed or run.
- Research schema, repository layout/local links, English-text review, and
  `git diff --check` are the final documentation gates for this batch.
- AUTOMATED TESTED / STATICALLY VERIFIED; not CLIENT-TESTED.
