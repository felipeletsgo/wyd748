---
id: equipment-slot-compatibility
title: Equipment slots supported by the WYD-Go 7.48 projection
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-18
---

# Equipment slots supported by the WYD-Go 7.48 projection

## Question

Which equipment positions must be exposed, particularly Mystic Amulet slot 9,
without accidentally enabling later-client slots 16/17?
How should equipment and warehouse mouse shortcuts reach those grids without
triggering world interaction, item use, or stack splitting?

## Evidence boundary

- USED: native FUN_00435B13 binds equipment controls 0x160..0x168.
- USED: current item definitions 559..562, slot mask 512, class/level rules,
  TMFieldScene inventory bindings, transaction contracts and server handlers.
- CONTRADICTORY: the earlier projection deliberately hid Necklace slot 9 and
  migrated every saved item there. That policy prevents the supported amulets.
- SECONDARY: historical ITEMTYPE_NECKLACE names corroborate the slot but do
  not alone prove native item eligibility.
- NOT APPLICABLE: modern Belt/NewSlot controls are not part of Equip[16].

## Native 7.48 flow

### Callers

The native inventory binder FUN_00435B13 resolves equipment controls from
root 257. Current scene initialization binds control 354 to equipment slot 9.

Native grid vtable entry `005A402C` points to `FUN_004209FC`. The window
procedure `FUN_0055DAB8` refreshes Ctrl at translator offset `0x438` from
`GetKeyState(0x11)` and Shift at `0x440` from `GetKeyState(0x10)`.
On right-button release (`0x205`), `FUN_004209FC` uses Ctrl to select the
equipment path. Shift with a selling fairy selects the existing sale path.
The focused read-only Ghidra export on October 18 confirmed the grid entry,
references, callees, and instructions against the SHA-256 above; the existing
window-procedure export supplies the modifier mapping.

### Callees

Existing grid population, SendItem, SwapItem and sell handlers use the shared
equipment-position validation. No new inventory control or message is added.
The native quick-equip path calls item ability lookup `FUN_0054CD07`,
equipment eligibility `FUN_00553016`, and packet sending `FUN_0055F2DD`.
It uses ability 17 for the equipment mask, ability 33 for item footprint,
and the existing 20-byte `0x376` swap intention.

## State and lifecycle

Bootstrap enables and populates control 354. Controls 1048976/1048977 for
positions 16/17 stay empty, disabled and hidden. Scene teardown owns all grids;
no receiver survives relogin.

On server entry, a slot-9 item whose Pos includes bit 9 remains equipped.
An incompatible legacy item is migrated to visible Carry/Cargo using the
existing persistence/rollback path. Full storage preserves the item rather
than deleting it. Eligibility and score continue to require the correct Pos.

Modified right-button presses reserve the item gesture instead of using or
dragging the item. On release, Ctrl equips/unequips and takes priority over
Shift. Visible inventory/warehouse panels receive Ctrl-right input before the
world interaction menu. Mouse-event modifier bits refresh the cached flags.
Shift-right transfers between Carry and Cargo only with a visible warehouse,
a nonzero warehouse NPC ID, and an empty cursor. The existing Ctrl-left
warehouse shortcut uses the same guards. Both require a hit inside the actual
storage grid and choose the first free footprint. Neither detaches the item or
changes grid state before the server acknowledgement. Cargo items must pass
through Carry before equipment. Shift-left splitting is unchanged; Shift-right
fairy selling remains available when the warehouse is closed.

## Wire, ABI, and resources

Equip[16], packet sizes and offsets are unchanged. Position 9 is a valid
equipment destination; 16/17 remain rejected before grid dereference.
Existing GateMapData compatibility handling is preserved.

## Current mapping

MODERNIZACAO_COMPATIVEL: restore the supported slot across client binding,
transaction validation, server eligibility, entry migration, score and gems.
This is not a claim that every amulet rule is independently native-validated.

`PARIDADE_NATIVA`: restore Ctrl-right quick equip/unequip through the proven
native equipment path and existing packet. `MODERNIZACAO_COMPATIVEL`: guarded
input routing, modifier refresh, and the Shift-right warehouse alias requested
by the user. The alias is a project choice, not independently proven native
Shift behavior. No new opcode, resource, server rule, or persisted value.

## Delta matrix

| Claim | Previous projection | Current decision |
| --- | --- | --- |
| Slot 9 control 354 | Hidden/null | Bound, enabled and populated |
| Mystic Amulet 559..562 | Cannot equip | Existing class/level/Pos validation |
| Compatible saved slot-9 item | Always migrated | Kept equipped |
| Incompatible saved item | Migrated or retained safely | Preserve migration and rollback |
| Slots 16/17 | Unsupported | Still unsupported |
| Equip ABI | Sixteen items | Unchanged |
| Ctrl-right item input | Captured by world interaction menu | Visible item panels receive it first |
| Warehouse quick move | Ctrl-left could run outside the clicked grid | Guarded Ctrl-left and Shift-right |
| Split/use/fairy gestures | Existing behavior | Shift-left/plain right preserved; Ctrl takes precedence |

## Decisions

Expose the already supported amulet slot without resizing equipment or enabling
modern controls. Preserve item identity and persistence rollback.

## Gaps

Equipment drag, score refresh, full-storage relogin and scene transitions still
need actual client execution. Not CLIENT_TESTED.
The historical slot-9 batch was published in server release `20261014T234237Z`
and patch 10024; see
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.
The October 18 shortcut batch is separate: installation, publication and actual
gameplay validation are pending. Native Shift warehouse behavior is not proven.

## Validation

### Historical slot-9 batch

Focused tests cover all four amulet classes, level requirements, UID retention,
normal and critical entry, full storage, replay, score and gem eligibility.
Full Go tests/vet and client architecture tests passed.
Release/Win32 -NoDeploy build passed without replacing the installed runtime.

### October 18 shortcut batch

The focused Go regression batch passed 18 top-level tests covering inventory
swap, equipment eligibility, cargo rejection/reserved slots, critical-write
commit/rejection/unknown outcomes, and the 20-byte swap acknowledgement layout.
New C++ policy tests cover modifier precedence, press reservation, contextual
warehouse guards, hidden/deleted parents, panel-relative hit testing, cursor
ownership, and Equip/Carry/Cargo slot limits. Candidate build validation is
recorded below; this is not gameplay evidence.

- Architecture tests: 3,282,132 checks and static assertions passed. Socket
  receive: 281 checks passed; crash capture: 198 checks, zero failures; launcher:
  56 checks, zero failures. Asset/shader gates passed through `Build-Client.ps1`.
- Release/Win32 `Build-Client.ps1 -NoDeploy` passed. A final incremental solution
  build passed after changing rejected equipment attempts to consume input;
  unchanged policy, socket, crash and launcher test evidence was reused.
- Built candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `82D30DA50B5606C55A9062E8216D58E3FB88DEFF6A534243EE1424A07C74C362`.
- Installed runtime: `tmproject/client748/WYD.exe`, SHA-256
  `251CF754409F8AE4C0251C9FDD88EDA14C6799E78A377BD0665B6E858FDAFAB0`.
  It was not replaced. No running client was stopped, no patch published, and
  no VPS service restarted for this batch.
- Nonblocking build warnings: legacy Winsock `inet_addr`/`WSAAsyncSelect`
  deprecations (C4996), and the bundled DirectX `dxfile.h` control character
  (C5298). No build error.
- This evidence record passes the research validator. The repository-wide
  validator exits 1 for unrelated existing records: `persistent-buff-visuals.md`
  lacks Gaps; `instance-runtime-counter-controls.md` lacks Decisions, Delta
  matrix, Gaps, State and lifecycle, and Wire, ABI, and resources. Those files
  were not changed. Whitespace checks and the affected-file language audit pass.

Remaining gameplay matrix: Ctrl-right equip/unequip, ineligible items and full
Carry; Shift-right transfers in both directions with multi-cell items and full
storage; modified presses on consumables; Shift-left split and fairy sale;
outside/hidden-panel clicks, occupied cursor, and world interaction outside the
panels. Verify server rejection/resync and reconnect without promoting this
record to CLIENT_TESTED before execution.

Cleanup limitation: removal of the validated task-only scratch directories was
blocked by execution policy. No alternate deletion mechanism was attempted.
Leftovers are `build/inventory-shortcuts/` (native exports and validation logs)
and `tmproject/build/tests/Release/crash-capture-tests-31332-712756125/`
(isolated crash-test artifacts). The current incremental build output remains
intentionally preserved. These leftovers do not change the passing build or
the pending installation/gameplay status.
