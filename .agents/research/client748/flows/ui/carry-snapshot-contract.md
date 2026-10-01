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

## State and lifecycle

The active consumer requires a payload, model, Field scene, and local human.
It copies all 64 Carry items and Coin, then clears opponent/check state
before examining optional controls. A missing inventory grid no longer
discards authoritative state or trade invalidation.

With a control container and visible Trade panel, the existing close routine
clears trade visuals and transient offer data. Its cancellation guard sees
the already cleared opponent, so a server snapshot is not converted into an
outgoing trade intention. Score projection runs only with a container.
Absent/hidden Trade panels do not prevent flag cleanup. Ordinary inventory
visibility remains controlled by I/menu input; this retained compatible
policy is not a claim of complete native panel-cascade parity.

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
`TMFieldScene::SetVisibleTrade`.

The consumer retains native row-major slots and visible indexes 0..62.
Previously, its initial guard discarded the snapshot when the grid was
missing, and it never invalidated trade. State copying and flag cleanup
now precede optional presentation. Five source-contract regressions in
`SceneDisconnectContractTests.cpp` protect receiver guards, data retention,
cleanup-before-close, optional UI/visibility policy, and visual ownership.

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
| Ordinary inventory visibility | native invokes shared cascade | I/menu policy | retain existing policy; no full cascade claim | compatible policy retained |
| Oversized Go slice | N/A | could overrun buffer/panic | bound to 64 | `MODERNIZACAO_COMPATIVEL` |

## Decisions

- Preserve the exact native 528-byte envelope and 64-item array.
- Keep structural slot 63 outside the visual grid.
- Reject invalid frames before destructive visual rebuilding.
- Apply valid authoritative state independently of optional presentation.
- Clear trade flags before closing UI; never send a cancellation in response
  to this snapshot.
- Preserve ordinary inventory visibility rather than adding an unproven
  toggle/cascade to this state-synchronization patch.
- Keep the bounded Go builder and all existing valid callers unchanged.

## Gaps

- Execute login, purchase, trade, AutoTrade, quests, and mixing in the candidate.
- Exercise snapshots while dragging, with full inventory, during map changes,
  and after relogin before promoting to `CLIENT_TESTED`.
- Confirm visible-trade closure, no extra outbound cancellation, and missing
  presentation behavior in the real built client. Source-order tests alone
  do not execute the consumer or DirectX/UI ownership.
- Full native `FUN_00447691` cascade adaptation and native local-offer
  removal remain separate evidence gaps; whole-snapshot invalidation does
  not prove an item-removal interaction.

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
- 2026-09-30 regression: all five new source-contract assertions failed against
  the previous consumer, then passed after the patch. ArchitectureTests
  passed 58,675 checks and static assertions; SocketReceiveTests passed 221.
  The incremental Release `Build-Client.ps1 -NoDeploy` build passed with
  existing signedness warnings. No executable was installed or run.
- Research schema, repository layout/local links, English-text review, and
  `git diff --check` are the final documentation gates for this batch.
- AUTOMATED TESTED / STATICALLY VERIFIED; not CLIENT-TESTED.
