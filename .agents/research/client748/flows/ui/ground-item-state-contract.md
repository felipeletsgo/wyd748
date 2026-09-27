---
id: ground-item-state-contract
title: Ground-item state and removal contract
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-25
---

# Ground-item state and removal contract

## Question

How does the 7.48 client apply `0x374` to a ground object, remove it with
`0x16F`, and reconcile a gate's collision height with the server?

## Evidence boundary

- USED: Native binary with the hash above and the existing Ghidra traces of
  `FUN_00492E7D`, `FUN_004862B6`, `FUN_004863E2`, and `FUN_0055890A`.
- USED: Active client `MSG_UpdateItem`, `OnPacketUpdateItem`,
  `OnPacketRemoveItem`, `BASE_UpdateItem2`, and `g_pGroundMask`.
- USED: WYD-Go `wire.UpdateItem`, `groundItemUpdatePacket`, the 96 records in
  `data/init_items.csv`, and the CSV/mask/terrain contract tests.
- NOT APPLICABLE: Assets and the KR guide do not define these wire envelopes.
  External legacy sources were not used as parity authority.

## Native 7.48 flow

### Callers

`FUN_00492E7D @ 0x00492E7D` forwards `0x374` to
`FUN_004862B6 @ 0x004862B6` and `0x16F` to
`FUN_004863E2 @ 0x004863E2`. `FUN_0055890A @ 0x0055890A` requires
20 and 16 bytes respectively.

### Callees

For `0x374`, `FUN_004862B6` finds the object by `ItemID@0x0C`, reads
`State` as a short at `+0x10` and `Height` as a char at `+0x12`, updates
the heightmap/state through `FUN_005554CC`, then updates the score panel via
`FUN_004431E4`. For `0x16F`, `FUN_004863E2` clears the hovered item and
asks the ObjectManager to destroy the 32-bit ID at `+0x0C`.

The native `FUN_005554CC @ 0x005554CC` references the mask table at
`0x005BED50` (operand at `0x00555539`). The table occupies 5,760 bytes in
`.data`, file offset/RVA `0x001BED50`: 1,440 little-endian 32-bit cells in
the same `[10][4][6][6]` order as the active source. Its SHA-256 is
`B53392BEB7DE3B2E74A026D4256DF822F7A5F445A858D5D348F36B196D03EF3D`.
The exact byte sequence occurs at that offset in both `WYD.exe` (hash in
front matter) and stock `WYDoriginal.exe` (SHA-256
`B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C`).
The source table matches all 1,440 native values; the server uses their
nonzero cells, as `BASE_UpdateItem2` does.

## State and lifecycle

| Event | Precondition | Result | Failure |
| --- | --- | --- | --- |
| `0x374` | 20-byte frame, existing gate | State and masked height change | Missing object leaves the world unchanged |
| `0x16F` | 16-byte frame | ObjectManager removes the item and clears hover | Missing ID is idempotent |
| Invalid frame | Size or Type mismatch | Dispatcher rejects it | No scene callback |
| Incomplete scene | ObjectManager missing | Handler returns | No dereference |
| Existing ID is not a gate | Valid `0x374` frame | World unchanged | No derived-object access |
| Relogin | Visibility snapshot | Object materializes again | No frame retained |

The server persists key consumption before changing the gate. On save failure,
the key, gate state, and collision height remain unchanged. On success, the
server changes the gate state and its authoritative terrain before sending
`0x374` to observers. Later arrivals receive the state in `0x26E`.

## Wire, ABI, and resources

`0x374` is a 20-byte server-to-client little-endian frame: 12-byte standard
header; `ItemID` u32 at `0x0C`; `State` i16 at `0x10`; `Height` i8 at
`0x12`; reserved byte at `0x13`. `0x16F` is a 16-byte frame with a standard
header and `ItemID` u32 at `0x0C`. Neither packet transfers object ownership.

The active server writes `Height=0` when opening a gate and `Height=16` when
restoring a closed gate after rejected interaction. Closed `0x26E` also
carries height 16. These are current client/server contract values, not a
claim that the historical server used those exact height values.

## Current mapping

`GroundItemStateContract.h` names the opcode, size, and offsets; `Basedef.h`
asserts the concrete layout. `ReceivedPacketDispatch` validates the real and
declared size and opcode before casting. Client handlers check their scene
owners. Go `wire.UpdateItem` writes the two-byte state without contaminating
the height/reserved bytes; `groundItemUpdatePacket` then sets the closed
height explicitly. `RemoveItem` sends the required 16 bytes. The client
`BASE_UpdateItem2` checks both mask and rotation against the actual table
dimensions and rejects a null HeightMap before indexing; valid inputs retain
the native lookup and write behavior. `OnPacketUpdateItem` checks that the
resolved object is a `TMGate` before reading gate-specific fields.

The server loads permanent object positions/rotations from the 96-row CSV.
For masked objects, `spawnInitItems` overlays a private copy of HeightMap
using the active client's 6x6 mask. `openGateWithKey` clears the same cells
only after account persistence. The client and server therefore use the same
current mask geometry without hard-coding the CSV's object locations twice.

## Delta matrix

| Claim | Native 7.48 | Current implementation | Classification |
| --- | --- | --- | --- |
| `0x374/20` and `0x16F/16` envelopes | Confirmed by trace | Dispatch gates and layout tests | `PARIDADE_NATIVA` |
| State/height offsets | Confirmed by trace | C++ asserts and Go encoder tests | `PARIDADE_NATIVA` |
| Null-owner guard | Lifecycle assumed | Local guard | `MODERNIZACAO_COMPATIVEL` |
| Invalid mask/rotation or null map | Native lookup assumes valid inputs | Client returns without indexing | `MODERNIZACAO_COMPATIVEL` |
| `0x374` names a non-gate ID | Native assumes a valid object lifecycle | Client rejects before derived access | `MODERNIZACAO_COMPATIVEL` |
| Ground-mask geometry | Native table at `0x005BED50` matches all 1,440 source values | Go bitsets match source and native table hash is pinned | `PARIDADE_NATIVA` |
| Server dynamic terrain overlay | Native client writes height into masked cells | Server follows the same mask geometry | `MODERNIZACAO_COMPATIVEL` |

## Decisions

- Preserve object effects and ownership for valid frames.
- Reject malformed envelopes before reaching the scene.
- Reject out-of-range mask/rotation values and a null map at the collision
  primitive; this changes only invalid input handling.
- Reject `0x374` for an existing non-gate object without casting it to a
  gate; valid gate updates are unchanged.
- Keep key use and the collision transition server-authoritative and
  persistence-ordered.
- Use the 96-row CSV for object identity/location/rotation; do not infer
  additional objects from the 7.69 source.

## Gaps

- Real-client opening, closing, removal, relogin, and region-change flows
  remain untested. Windows currently prevents that visual execution.

## Validation

- Existing native trace proves `0x374/20`, `0x16F/16`, offsets, and native
  heightmap mutation.
- Binary comparison confirms a single exact 5,760-byte native table at file
  offset `0x001BED50` in both 7.48 executables. The `FUN_005554CC` operand
  references its VA, so the match is the consumed table, not an orphan byte
  pattern. The Go test pins its hash and compares every nonzero cell to the
  server bitset.
- `go test -count=1 ./internal/game -run
  'TestGatePersistenceControlsTerrainTransition|TestClosedGroundMaskRejectsRouteUntilOpened|TestCSVFixturesUpdateServerTerrainLikeClient|TestGroundMaskBitsMatchClientSource'`
  passed on 2026-09-25. The CSV test covers all 96 records and rejects mask
  overlaps; the source comparison covers all 1,440 client mask cells.
- The incremental C++ Release `-NoDeploy` build passed after the guard:
  52,026 architecture checks and 221 socket receive checks passed. The
  built artifact SHA-256 is
  `F582FB1509B59B26752445F99DD4648F60D0C1B29E77A79C6B77AA9B77CAFE30`.
  No candidate was installed or run. Status is `AUTOMATED TESTED`, not
  `CLIENT_TESTED`; invalid-rotation behavior has compile/source coverage,
  not an executable unit test.
- A later incremental Release `-NoDeploy` build passed after the concrete
  gate check: 52,026 architecture checks and 221 socket checks. Its artifact
  SHA-256 is
  `87288FED114739B291F9C437BFB9113836E9907EA1DE540CF5E7F27C2B8B6695`.
  The non-gate rejection has compile/source coverage only; no client run.
