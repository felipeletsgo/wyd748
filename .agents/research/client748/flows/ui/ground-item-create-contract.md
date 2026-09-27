---
id: ground-item-create-contract
title: Ground-item creation contract
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-25
---

# Ground-item creation contract

## Question

What is the `0x26E` ground-item creation/update contract, and which fields
may the 7.48 client renderer safely use?

## Evidence boundary

- USED: The native binary with the hash above, Ghidra project
  `WYD748Native_20260821.gpr`, and decompilation of `FUN_00492E7D`,
  `FUN_004856C3`, and `FUN_0055890A`.
- USED: Active `MSG_CreateItem`, `OnPacketCreateItem`, ItemList, world
  containers, and `ReceivedPacketDispatch` source.
- USED: WYD-Go `wire.CreateItem`, byte-for-byte test, and map-item/object
  visibility emitters.
- USED: Active `ItemList.bin` has 6,500 entries bounded by `MAX_ITEMLIST`.
- NOT APPLICABLE: The KR guide does not document this wire contract.
  External legacy sources and 7.54 sources were not consulted.

## Native 7.48 flow

### Callers

`FUN_00492E7D @ 0x00492E7D` forwards `0x26E` directly to
`FUN_004856C3 @ 0x004856C3`. `FUN_0055890A @ 0x0055890A` accepts this
opcode only with `Size=0x20`.

### Callees

`FUN_004856C3` looks up an existing object by `ItemID@0x10`, reads the
eight-byte item at `+0x12`, and chooses a gate, cannon, or ordinary item.
It initializes mesh/state, rotation at `+0x1A`, position at
`GridX/GridY@0x0C/0x0E`, height at `+0x1C`, creation flag at `+0x1D`,
and owner at `+0x1E`. Relevant callees include `FUN_0054CD07` for
abilities, `FUN_004F0C50`/`FUN_004F1960`/`FUN_004F3A9B` for object
kinds, `FUN_0054AC09` for world insertion, and `FUN_005554CC` for
ownership effects.

The native handler assumes a valid packet and ItemList index. The compatible
source rejects an `sIndex` outside the loaded ItemList before lookup.

## State and lifecycle

| Event | Precondition | Result | Side effects | Failure |
| --- | --- | --- | --- | --- |
| Create ordinary item | 32-byte frame, valid `sIndex` | `TMItem` in container | Position, mesh, optional sound | Allocation failure does not insert |
| Create gate/object | Ability 34 is positive | Initialized `TMGate` | Heightmap/state update | Missing ground/container rejects |
| Update existing ID | Same concrete renderer kind | Existing owner updated | No duplicate child | Mismatched kind rejects before derived cast |
| Invalid index | `sIndex<=0` or `>=6500` | World unchanged | No out-of-bounds lookup | Immediate return |
| Remove/relogin | `0x16F` or scene change | Normal owner destroys object | Container/scene release resources | Frame is not retained |

The transport lends the 32 bytes for the callback. Newly created objects are
transferred to the container only after initialization; an existing object
retains its owner. A gate does not copy or retain the frame.

## Wire, ABI, and resources

Server-to-client little-endian contract:

| Offset | Bytes | Field |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`, `Type=0x26E` |
| `0x0C` | 2 | `GridX` |
| `0x0E` | 2 | `GridY` |
| `0x10` | 2 | `ItemID` |
| `0x12` | 8 | `STRUCT_ITEM` |
| `0x1A` | 1 | `Rotate` |
| `0x1B` | 1 | `State` |
| `0x1C` | 1 | `Height` |
| `0x1D` | 1 | `Create` |
| `0x1E` | 2 | `Owner` |
| Total | 32 | Required by `FUN_0055890A` |

`sIndex` refers to 6,500 loaded 7.48 ItemList records. No native C++
object offset is transmitted or copied into the current source.

## Current mapping

`GroundItemCreateContract.h` names the opcode, size, offsets, and ItemList
domain. `Basedef.h` defines the concrete structure and asserts each offset.
`ReceivedPacketDispatch` verifies actual and declared size and opcode
before casting. `OnPacketCreateItem` checks the message, scene owners,
`sIndex`, and the concrete renderer kind of an existing ID before using
derived fields. WYD-Go emits the same layout in visibility snapshots and
drop publication.

## Delta matrix

| Claim | Native 7.48 | Previous source | Current source | Classification |
| --- | --- | --- | --- | --- |
| Opcode/size | `0x26E/32` | Literal without exact gate | Named contract and gate | `PARIDADE_NATIVA` |
| Offsets | `12..30` | Size assert only | Per-field asserts | `PARIDADE_NATIVA` |
| ItemList | Direct lookup | Could index beyond 6,500 | Index validated | `MODERNIZACAO_COMPATIVEL` |
| Existing ID with changed kind | Native assumes valid object lifecycle | Cast could write beyond allocated renderer | Reject mismatch before derived cast | `MODERNIZACAO_COMPATIVEL` |
| Objects/effects | Native branches | Existing implementation | Preserved for valid frames | No change |

## Decisions

- Validate the frame before casting and `sIndex` before ItemList lookup.
- Preserve objects, positions, rotations, sounds, and ownership for valid
  frames.
- Reject a reused ID whose existing concrete renderer kind differs from the
  new item's kind. No wire or valid-frame behavior changes.
- Reject a partial lifecycle when the scene lacks ground or container.
- Do not introduce assets, packets, or fallbacks from another version.

## Gaps

- Execute loot appearance, update, and removal in the candidate client.
- Confirm gates/permanent objects and ownership effects in the real client.
- Exercise relogin and region changes while items are visible.
- The invalid-kind rejection has compile/source coverage, not a client
  runtime or dedicated executable unit test.

## Validation

- Native trace `FUN_00492E7D -> FUN_004856C3` identifies the consumer;
  `FUN_0055890A` confirms `0x26E/0x20`.
- Existing fixtures cover prefixes, excess bytes, null input, Size/Type
  mismatches, offsets, byte preservation, and ItemList bounds.
  `go test -count=1 ./internal/wire` and `go vet ./internal/wire` passed
  in the earlier contract batch.
- On 2026-09-25, incremental Release `Build-Client.ps1 -NoDeploy` passed
  with 52,026 architecture and 221 socket checks. The built artifact SHA-256
  is `87288FED114739B291F9C437BFB9113836E9907EA1DE540CF5E7F27C2B8B6695`.
  It was not installed or run. Status is `AUTOMATED TESTED`, not
  `CLIENT_TESTED`.
