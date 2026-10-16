---
id: update-equip-visual-contract
title: Equipment appearance snapshot and transformation effects
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-15
---

# Equipment appearance snapshot

## Question

How does 7.48 apply `0x36B` appearance without transferring authoritative
items or restoring unsupported Necklace/Belt slots? When may this snapshot
play transformation effects rather than interrupting ordinary buff casts?

## Evidence boundary

- USED: native binary with the hash above, Ghidra project
  `tmproject/build/native-research/WYD748.gpr` and cached decompilations of
  `FUN_0052eaa9`, `FUN_0052b116` and `FUN_0055890a`. The October 15 focused
  `ExportWydFlow.java` instruction export confirmed the unchanged conditional
  branch to `0x0052b46c`; no new corpus export is needed.
- USED: current `MSG_UpdateEquip`, `TMHuman::OnPacketUpdateEquip`,
  `SetPacketEquipItem`, `SetColorItem` and `ReceivedPacketDispatch` source.
- USED: WYD-Go `wire.SelfEquip`, `VisualEquip`, `bodyMesh`, `bodyAncient`,
  `clientEquipProjection`, appearance publisher and existing contract tests.
  The October correction does not change server source or packet emission.
- USED: C++ packet contract tests and source-scope regression checks in
  `SceneDisconnectContractTests.cpp`.
- NOT APPLICABLE: no new asset, resource binding or later-client algorithm
  participates. KR guides and third-party sources do not define this wire.

## Native 7.48 flow

### Callers

`FUN_0052eaa9 @ 0x0052eaa9` dispatches `0x36B` to
`FUN_0052b116 @ 0x0052b116` for the character selected by `Header.ID`.
`FUN_0055890a @ 0x0055890a` requires `0x3C`/60 bytes.

### Callees

`FUN_0052b116` reads sixteen visual WORDs at `+0x0C` and sixteen color/AnctCode
bytes at `+0x2C`. If face mesh and `EF_CLASS` both match the current actor,
it jumps to `0x0052b46c`, bypassing the entire transformation effect block:
particle creation, sound 158, bright shade, motion 14, and level-up effect.
Only the level-up effect additionally checks the hidden-character flag.

The unconditional snapshot tail applies equipment (`FUN_00526140`), colors
(`FUN_00527032`), height (`FUN_005277a7`), race (`FUN_004faf13`), weapon state
(`FUN_0051bb41`), object rebuilding/angle through virtual calls, and score UI
(`FUN_004431e4`). Effects are submitted through `FUN_0054ac09`; motion changes
use `FUN_0051dba4`. No item ownership or inventory transfer occurs.

## State and lifecycle

The handler updates the owner or an observed `TMHuman`; the frame is borrowed
only for the callback. It applies sixteen visuals, imported mount selection,
colors, height, weapons and HUD. Newly created effects belong to the scene
effect container; no new retained pointer or cleanup responsibility is added.
Visibility re-entry or relogin rematerializes appearance from the next snapshot.

Ordinary same-face/class updates do not request transformation motion, sound
or effects. Genuine face/class transitions retain the existing effects and
their hidden-character guard. Both branches still execute the snapshot tail.
This correction does not suppress packets or skip real equipment updates.

The server keeps slot 9 empty in `bodyMesh`, `bodyAncient` and
`clientEquipProjection`; slots 16/17 cannot fit in this frame. Unsupported
Necklace/Belt/NewSlot UI and equipment rules are outside this change.

## Wire, ABI, and resources

`0x36B`, S->C, 60 bytes:

| Offset | Size | Field |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` selects the character |
| `0x0C` | 32 | `sEquip[16]`, one visual WORD per slot |
| `0x2C` | 16 | `Equip2[16]`, one AnctCode/color byte per slot |

`internal/wire/UpdateEquipContract.h` names the opcode, size, offsets and
capacities. `Basedef.h` asserts both arrays. Dispatch validates actual and
declared size and opcode before casting. No UI resource or mesh is added.

## Current mapping

`TMHumanPackets.cpp :: TMHuman::OnPacketUpdateEquip` now encloses the entire
effect block in the existing face/class condition, rather than guarding only
the first particle allocation. The following snapshot application is unchanged.
This removes the unconditional bright shade and animation replacement reported
during buff use. Existing particle/effect parameters are preserved; this is a
branch-scope correction, not a redesign of transformation effects.

On WYD-Go, `SelfEquip` derives the arrays from equipment, and `VisualEquip`
bounds each block to sixteen entries. `visibility.go :: refreshAppearance`
publishes this snapshot to the player view. The server zeros unsupported slots
before building it; the client keeps the received visual/color bytes.

## Delta matrix

| Claim | Native 7.48 | Previous source | Current behavior | Classification |
| --- | --- | --- | --- | --- |
| Envelope | `0x36B/60` | 60B without gate | Existing gate/asserts retained | `PARIDADE_NATIVA` |
| Visual/color | 16 WORDs at +12; 16 bytes at +44 | Same layout | Unchanged | Preserve |
| Transformation effect scope | Face mesh OR class changed | Only particle conditional | Entire effect/motion block conditional | `PARIDADE_NATIVA` |
| Ordinary snapshot | Applies visual state without transformation effects | Always played shade/sound/motion | Still applies visuals, no transformation effects | Same correction as above |
| Necklace/Belt | Outside requested projection | Server zeros unsupported slots | Unchanged | Preserve published correction |

## Decisions

- Preserve the native 60-byte contract and parallel arrays.
- Reject invalid frames before effects and avatar rebuilding.
- Restore the native effect branch, not a blanket removal of legitimate effects.
- Keep ordinary snapshot application outside that branch for owner and observers.
- Keep unsupported slot projection authoritative on the server.

## Gaps

Real-client checks remain for buffs under repeated equipment snapshots,
genuine transformation/untransformation, hidden actors, owner/observer views,
equipment/refinement/color/cape/mount changes, movement, death, visibility
re-entry and relogin. Automated source checks cannot prove visual continuity.

## Validation

STATICALLY VERIFIED: native branch bypass at `0x0052b46c`, dispatch, consumer,
size, offsets, arrays and unchanged server projection. Current source retains
the native face/class comparison and unconditional appearance tail.

AUTOMATED TESTED, October 15: Release/x86 `Build-Client.ps1 -NoDeploy` passed;
ArchitectureTests 1,228,406 checks, socket 231, crash capture 83 and launcher
56. Thirteen new source-contract checks require all five effect/motion call
sites inside the complete native guard, retain the hidden-character check,
and require equipment/color/object/weapon/HUD updates outside it. Existing
packet fixtures cover truncated/oversized/null/mismatched frames and arrays.
Historical Go packet coverage is reused, not rerun for this client-only scope.

Candidate `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256:
`BBCF4B139CC89B4A732E1461FC3CD85BA02D891270C3A04960DF94AC63F2D7BA`.
Installed runtime remains
`0B0EB676E192DB1626B03EC38E8FA06DE1F6235CF5EC9F4B4557F12E21CB7DD6`.
Real client: pending. No runtime replacement, public patch, VPS deployment
or restart; this flow remains CONTRACT, not CLIENT_TESTED.
