---
id: equipment-slot-compatibility
title: Equipment slots supported by the WYD-Go 7.48 projection
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Equipment slots supported by the WYD-Go 7.48 projection

## Question

Which equipment positions must be exposed, particularly Mystic Amulet slot 9,
without accidentally enabling later-client slots 16/17?

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

### Callees

Existing grid population, SendItem, SwapItem and sell handlers use the shared
equipment-position validation. No new inventory control or message is added.

## State and lifecycle

Bootstrap enables and populates control 354. Controls 1048976/1048977 for
positions 16/17 stay empty, disabled and hidden. Scene teardown owns all grids;
no receiver survives relogin.

On server entry, a slot-9 item whose Pos includes bit 9 remains equipped.
An incompatible legacy item is migrated to visible Carry/Cargo using the
existing persistence/rollback path. Full storage preserves the item rather
than deleting it. Eligibility and score continue to require the correct Pos.

## Wire, ABI, and resources

Equip[16], packet sizes and offsets are unchanged. Position 9 is a valid
equipment destination; 16/17 remain rejected before grid dereference.
Existing GateMapData compatibility handling is preserved.

## Current mapping

MODERNIZACAO_COMPATIVEL: restore the supported slot across client binding,
transaction validation, server eligibility, entry migration, score and gems.
This is not a claim that every amulet rule is independently native-validated.

## Delta matrix

| Claim | Previous projection | Current decision |
| --- | --- | --- |
| Slot 9 control 354 | Hidden/null | Bound, enabled and populated |
| Mystic Amulet 559..562 | Cannot equip | Existing class/level/Pos validation |
| Compatible saved slot-9 item | Always migrated | Kept equipped |
| Incompatible saved item | Migrated or retained safely | Preserve migration and rollback |
| Slots 16/17 | Unsupported | Still unsupported |
| Equip ABI | Sixteen items | Unchanged |

## Decisions

Expose the already supported amulet slot without resizing equipment or enabling
modern controls. Preserve item identity and persistence rollback.

## Gaps

Equipment drag, score refresh, full-storage relogin and scene transitions still
need actual client execution. Not CLIENT_TESTED.
Published in server release `20261014T234237Z` and patch 10024; see
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.

## Validation

Focused tests cover all four amulet classes, level requirements, UID retention,
normal and critical entry, full storage, replay, score and gem eligibility.
Full Go tests/vet and client architecture tests passed.
Release/Win32 -NoDeploy build passed without replacing the installed runtime.
