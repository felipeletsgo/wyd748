---
id: send-item-local-update
title: Local slot update through SendItem
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-01
---

# Local slot update through SendItem

## Question

Where does `0x182` update storage and UI in character selection and the world,
which indices require validation before copying, and can an unavailable Cargo
grid prevent the common completion of an accepted slot update?

## Evidence boundary

- **USED:** native binary identity confirmed in previous cycles and the
  read-only/noanalysis Ghidra project `WYD748Native_20260821`.
- **USED:** decompilation of `FUN_004B263E`, `FUN_0052EAA9`, and
  `FUN_0052A737`; focused exports `select-character-cargo-update.tsv` and
  `field-send-item.tsv` in the research exports directory.
- **USED:** retained `FUN_0052A737.c` in the ignored native-research directory,
  carrying the same program SHA-256. Its Cargo branch and common finalization
  support the current decision without a new export or corpus inventory.
- **USED:** current `ObjectManager::HandleSelectCharacterItem`,
  `TMHuman::OnPacketSendItem`, arrays in `Basedef`/`ObjectManager.h`,
  `SGridControl` ownership, and source-contract regression tests.
- **USED:** WYD-Go `wire.SendItem`, `FinishPacket`/`ReadPacket`, and
  `send_item_contract_test.go`. Server behavior remains unchanged.
- **NOT APPLICABLE to this correction:** new assets/resources; no control is
  added. Complete native grid ownership remains pending.
- **NOT APPLICABLE:** later TMProject or guides as native offset authority;
  external legacy sources are excluded as parity evidence.

## Native 7.48 flow

### Callers

`FUN_004B263E` occupies ObjectManager slot `+8` (`0x005A4604`). In character
selection `0x7531`, it intercepts `0x182` before the tree; in other scenes it
visits active children. `FUN_0052EAA9` filters the ID against the human receiver
and calls `FUN_0052A737` for `0x182`.

### Callees

`FUN_004B263E` calls `FUN_0058F220` to copy eight bytes into Cargo.
`FUN_0052A737` compares the receiver with the local human before the
Equip/Carry/Cargo copies. Equip updates the selection cache when `DestPos != 0`,
before consulting grids. The Ghidra export confirms the direct caller
`0052EAA9 -> 0052A737`; grid virtual receivers and complete teardown remain open.

In the native Cargo branch, `FUN_0052A737` copies the slot before calling
the removal operation at `DestPos % 9, DestPos / 9`, stores the return in
`local_94`, and creates/adds an item only when `sIndex > 0`. Unlike Carry/Equip,
this branch does not explicitly delete `local_94`. At the end of the local-human
block, cursor virtual slot `+0x98` is called if an attachment remains; its
semantics are unresolved here. The earlier record equating these cleanups was
incorrect: releasing the removed item is a local fix, not proven native parity.

After the destination branches, the native handler continues through
`FUN_00524DED` (equipment projection), `FUN_005277A7` (height),
`FUN_004FAF13` (race), human virtual slots `+0x38` and `+0x40`, weapon handling
through `FUN_0051BB41`, mount-HUD work, field refresh through `FUN_004431E4`,
and the final hover-index invalidation at `DAT_005B12BC`. These operations
are separate from Cargo grid replacement. Native null-grid safety and the
complete receiver identities of these indirect calls are not claimed.

## State and lifecycle

In selection, every `0x182` is consumed; only `DestType=2` writes Cargo.
In the world, the current flow updates storage, controls, and human appearance.
Accepted Cargo state is copied before visual lookup. If the grid is unavailable,
the corrected source skips visual replacement but reaches the common
appearance/HUD/hover finalization, like the existing optional Equip/Carry grids.
It neither creates a replacement without an owning grid nor undoes server state.

When a grid exists, removal transfers the old item's ownership to the caller.
Matching cursor/hover/sale aliases are cleared before deletion. A newly created
visual rejected by `AddItem` is released; its authoritative slot remains intact.
This study does not resolve all native cursor paths, item destruction, reentry,
shutdown, or logout/relogin. It does not promote full lifecycle parity.

## Wire, ABI, and resources

S->C: 24 bytes, opcode `0x182`, signed short `DestType@12`, signed short
`DestPos@14`, eight-byte item at `16`. Go emits the same frame; transport fills
Size and encryption. Native `FUN_0055890A` contains `0x182/24`, but its
reachability remains pending in the packet-size-gate record. Source assertions
protect size/offsets. Do not transplant native object offsets into current C++
arrays. No wire, asset, control ID, or vtable changes are made here.

## Current mapping

`ReceivedPacketDispatch` validates size/opcode before the legacy traversal.
ObjectManager uses `ApplyCargoSlot`, tested across 128 storage positions.
`TMHuman::OnPacketSendItem` rejects negative or out-of-capacity indices for
Equip/Carry/Cargo before `Bag_View` or copies. Selection-cache writes require
`characterSlot` in `[0,4)`; an unavailable cache does not prevent world equipment
updates. Additional local Equip slots remain preserved.

The receiver must be the local human before consulting grids or recalculating
appearance from the global cache. Negative `sIndex` or an index outside the
6,500-entry 7.48 `ItemList.bin` is consumed without changing state. Zero remains
an empty slot. This internal guard prevents out-of-catalog `g_pItemList` reads
in `SetPacketMOBItem`.

Cargo captures and releases `PickupAtItem`'s return. Current `SGrid.cpp` removes
the pointer from the list, updates occupancy/scale, and transfers ownership to
the caller; it neither destroys the item nor clears interaction aliases.
`SGridControlItem` destruction releases visual/item resources, not global aliases.
The caller clears hover, last attachment, sale, and cursor pointers only when
they reference the removed item, reusing the existing `Empty` policy.

Previously, Cargo's `if (!pGrid) return 1` preserved the copied slot but skipped
every common completion operation below the destination branches. Cargo now
uses a positive `if (pGrid)` around pickup, cleanup, allocation, and insertion,
without an early return. The existing model-first ordering, coordinates,
insertion-failure cleanup, and final return value are unchanged.

## Delta matrix

| Claim | Native 7.48 | Current source | WYD-Go | Decision |
| --- | --- | --- | --- | --- |
| 24-byte frame | Native/source/Go agree | Gate before callback | Same envelope | Preserve bytes |
| Cargo in selection | Early copy and consumption | Capacity 128 | Storage snapshot | Preserve order |
| World destinations | Indices used before visual guards | Validate actual capacities | Authoritative state | MODERNIZACAO_COMPATIVEL |
| Selection cache | Character index separate from slot | Guard sentinel without blocking world | N/A | MODERNIZACAO_COMPATIVEL |
| Missing Cargo grid | Slot copy and common completion are separate; null safety unproven | Skip projection, not common completion | No change | MODERNIZACAO_COMPATIVEL, not native null-grid parity |
| Visual ownership | Still incomplete | Explicit local release/alias policy | No change | Preserve safe source policy; no full parity claim |

## Decisions

`MODERNIZACAO_COMPATIVEL`, locally implemented: reject impossible indices,
preserve valid ordering/bytes, and let an unbound Cargo grid suppress only its
projection. Do not copy pseudocode vulnerabilities. Do not reduce Cargo storage
from 128 to the usable limit 120 or remove `Equip[16..17]` merely because native
7.48 lacks them. No server behavior or transport change is needed.

## Gaps

- Execute rejection and Equip/Carry/Cargo updates in the real candidate.
- Validate cursor/hover/sale aliases after Cargo replacement at runtime.
- Resolve native grid virtual receivers and full destruction/teardown.
- Check `OnPacketEvent` calls outside the size-aware ingress.
- Test relogin, reserved slots, and appearance refresh.
- Execute the missing-Cargo-grid path in the DirectX scene; source-contract
  regressions and a build do not establish runtime behavior.

## Validation

Historical Ghidra exports completed without `SCRIPT ERROR`, with the correct
native program hash, `slot_outgoing 005A4604 -> 004B263E`, and caller
`0052EAA9 -> 0052A737`. Historical Debug/Release runs passed 232 C++ checks
and assertions; the Go wire suite and vet passed on their earlier unchanged
inputs. These tests covered frames and `ApplyCargoSlot`, not the new guards
inside a running `TMHuman` scene.

The following earlier Release artifacts were built and installed, but those
operations were not in-game flow validation:

| Historical change | Release SHA-256 | Validation boundary |
| --- | --- | --- |
| TMHuman destination guards | `5D1743014D80B9E34FEBE61CA8A3ADC34327363388A6841DCFA25C5C88A830AE` | STATICALLY VERIFIED; 232 checks, not CLIENT_TESTED |
| Cargo pickup ownership and cursor cleanup | `87F0FFCCFAC29E3950979515B864D6F9B77D7F4E930C894128E7DA76EFD323C6` | Local source-ownership fix, no native parity claim |
| Additional Cargo alias cleanup | `29485CB8C6570801C72C3F82D0B9BA944D16B09D74AF40638A2CEC6750353067` | Debug/Release 232 checks; no live hover/sale coverage |
| Shared releaseReplacedItem lambda | `79ED78DEE7826F262C017096E006762B4755A16AA9C9FA41BD699130AE5B3C10` | Debug/Release 232 checks; no UI-event execution |

The shared lambda centralizes alias cleanup and destruction of removed
Equip/Carry/Cargo visuals. Each branch preserves its pickup coordinates;
null does not change aliases, and references to other items survive. This is
source-level ownership policy, not new native ABI/resource evidence.

An earlier inspection found unchecked item-count growth in `AddItem`. The
current source uses `GridInsertion.h` validation and releases a rejected new
visual in this handler; the old finding must not be treated as an unimplemented
queue entry. It is separate from the missing-grid completion correction.

The 2026-09-23 local-human filter was classified as
`MODERNIZACAO_COMPATIVEL`, consistent with the comparison in `FUN_0052A737`,
without promoting this `LOCATED` record. The `sIndex` guard uses the active
catalog capacity and protects direct source reads, without changing frames or
valid items. Static contracts check both guards before UI effects and the first
copy. The no-deploy Release|x86 build passed with 51,869 checks, without visual
execution or `CLIENT_TESTED`.

The 2026-10-01 continuation reuses the retained native branch/finalization
evidence. Three source-contract checks cover model-before-grid ordering,
optional visual work without an early return, and the shared appearance,
score-UI, and hover completion. The missing-grid check failed before the
source patch. These tests inspect control flow; they do not execute DirectX or
prove the unresolved native virtual calls. The incremental
`Build-Client.ps1 -Configuration Release -NoDeploy` build passed with 60,990
architecture checks and 221 socket-receive checks. The Release artifact SHA-256
is `3851CC952FD7589F264191DE69F002C38297EA760B68B226E234A2533A914223`.
This is STATICALLY VERIFIED and AUTOMATED TESTED, not CLIENT_TESTED. No runtime
executable was installed and no client flow was executed.
