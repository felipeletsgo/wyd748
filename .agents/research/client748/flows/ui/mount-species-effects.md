---
id: mount-species-effects
title: Equipped mount species effects in 7.48 and KR
subsystem: ui
status: TRACED
native_sha256: 1FA945EB589B6CD8B2E1EA1D1B1AE83E5BDE66B5251344B4AC98B025489125C1
updated: 2026-11-09
---

# Equipped mount species effects in 7.48 and KR

## Question

Which effects does 7.48 render for equipped mounts? Which missing effects can the
adapted client restore from KR for its existing mounts?

## Evidence boundary

- USED: Portuguese 7.48 `WYD.exe`, Ghidra function and cross-reference export,
  and instruction ranges for appearance, bone transforms, and effect updates.
- USED: native and active `EffectTextureList.bin` rows 101–108, corresponding
  texture files, current client source, and the current item catalog.
- USED: [mount level glow](mount-level-glow.md) for the independent level effect.
- USED: local KR 7.70+ `WYD.exe`, `ItemList.bin`, and `MountDataV.bin` for imported
  mounts. This evidence is separate from the native 7.48 flow.
- NOT APPLICABLE: 7.59 server binaries.

## Native 7.48 flow

1. Actor appearance setup `FUN_004f7c95` checks mount type 38 at `0x004fa63d`.
   It creates seven `TMEffectBillBoard` instances and stores them at `this+0x588`.
2. Bone processing `FUN_004bee3f` selects rig 38 at `0x004bfeec`. It maps bone
   IDs `33,16,11,21,26,5,2` to seven positions. It writes the positions to
   `owner+0x64` for actor type 38 and `owner+0xdc` for the mounted model.
3. Actor effect update `FUN_004fdbb0` checks mount type 38 at `0x00500551`.
   It updates each billboard from `owner+0xdc` and raises slots 1–4 by a
   scale-dependent Y offset. The actor's own rig-38 effect uses separate slots.

### Callers

- Ghidra resolves a virtual table reference at `0x005a15b4` to appearance setup.
- `FUN_0050077e` calls the actor effect update at `0x0050139f`.

### Callees

- Setup calls the billboard constructor `FUN_004c5c20` and scene child insertion
  `FUN_00546959`. The actor owns the pointers and the scene renders the children.
- The bone branch calls the vector transform at `0x0055cdac`. The update copies
  the anchor with `FUN_00431230` and calls the billboard's virtual `FrameMove(0)`.

## KR mounted skin flow

KR executable SHA-256 `71D2E7746831FAE932313602CFE827575FA3C2A0C61D3566142563B2814C872A`.
`FUN_005581a0` calls the equipped mesh render `FUN_00536de0` at `0x005588b1`.
The mesh render updates bone transforms with `FUN_00524940` at `0x00537339`.
It then calls the species dispatch `FUN_00536c50` at `0x0053735a`.

| KR rig / Mesh0 | Local catalog | KR emitter | Effect |
| --- | --- | --- | --- |
| 31 / 8 | Death Shadow Red and Black, 4191 and 4192 | `FUN_00536ab0` | Four ground flames, white with intermittent red, on bones 4, 9, 14, 19 |
| 20 / 6 | Dragon Lich, 6012 from KR 3997 | `FUN_005422d0` | Seven green billboards on bones 9, 13, 16, 11, 18, 24, 31 after 300 ms. Each lasts 2500 ms. |
| 28 / 3 | Iceland Wolf, 6011 from KR 3996 | `FUN_00542e70` | Pale mist on bone 10. Each billboard lasts 1500 ms. |

KR actor effects set the rig 31 flag at `0x00547680`–`0x005476a3`. They also
call the dragon and wolf emitters at `0x00546ebf` and `0x005476b2` for their
actor variants. The adapted client uses the same existing emitters for the
mounts, with separate timing state per mounted mesh.

Effect texture index 0 is `Effect/smog.wys` in KR, stock 7.48, and the active
client. Its SHA-256 matches in all three
(`157427E51C102B7C6BA2F4A08230D78AF6A20E96240BD3F834FC0A8E87A02186`).

## State and lifecycle

| Event | Native 7.48 result |
| --- | --- |
| Appearance setup with mount type 38 | Creates the seven emitters when slot 0 is empty. Skips a slot if allocation fails. |
| Bone and effect frame update | Updates existing emitters independently and skips null slots. |
| Appearance rebuild or actor cleanup | Releases existing emitters through the object manager and clears the pointers at `0x004f935c` and `0x004f6e29`. |

The mount branch does not check whether the actor is the local player. A fresh
actor starts with empty slots. Logout, scene changes, and re-entry still need a
real-client check for owner and observer views.

The adapted KR dispatch runs after the current mount updates its bones, before
the rider mesh can reuse those anchor slots. It applies to both views. Replacing
or deleting the equipped mesh stops emission and discards its clock. Existing
particles expire in the scene container or leave with that scene.

## Wire, ABI, and resources

- Local visual effect from resolved mount type 38. No separate effect packet is involved.
- Each billboard starts at texture 101 with eight frames at 80 ms per frame.
  The setup uses `0xffff5500`, the bright blend type, and no automatic frame update.
- The sizes at `0x004fa68f` are 1.0 for slot 0, 3.0 for slot 5, and 1.2 for
  all other slots, times actor scale. `0x0059f4a0` supplies the 0.5 Y offset
  factor for slots 1–4.
- Both effect tables bind 101–108 to `Effect/gfire01.wys`–`Effect/gfire08.wys`.
  All eight active texture files match their native files by SHA-256.
- Both clients map rig 38 to `mesh/tg01`. The `tg01.bon` files match by
  SHA-256 and contain each of the seven attachment IDs once.

## Current mapping

- `TMHumanAppearance.cpp` resolves Fire Tiger items 2379 and 3990 to model 334
  for owner and observer. Model 334 uses class 51 and skin mesh type 38.
- `TMHumanMounts.cpp::UpdateMount` registers seven billboards after building
  the native mount type 38 mesh. A failed mesh or billboard allocation leaves
  the missing effect hidden.
- `CFrame.cpp` uses the seven rig 38 bones and updates the mount billboards
  only when the bone belongs to the equipped mount mesh. The actor's own
  rig 38 continues to write to its existing anchor array.
- `TMHumanRender.cpp` hides the billboards before each actor render. Only the
  bones processed by that render restore them. A mount rebuild, delayed actor
  removal, and actor destruction unlink and release the billboards.
- `CMesh.cpp` still renders the separate mount level glow through `NativeMountGlow.h`.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Fire Tiger mounted flames | Seven billboards use separate mount bone positions | Restores all seven on the equipped mesh | Active assets already match | Mount appearance provides the model | Restore |
| Mount level glow | Separate grade texture mapping | Matching route in the prior record | Active texture table | Publishes mount grade | Keep |
| KR Dragon Lich and Iceland Wolf | No equivalent mounted skin dispatch | Enables the existing actor emitters on their exact KR mount rig and mesh pairs | Existing assets and attachment bones | Publishes mount appearance | Restore from KR |
| KR Death Shadow variants | No equivalent mounted skin dispatch | Selects hoof flames independently of the rider's effects | Existing assets and attachment bones | Publishes mount appearance | Bind to the equipped mesh |

## Decisions

`PARIDADE_NATIVA`. Restore the mounted Fire Tiger effect for owner and observer.
The 7.48 actor effect routine has one species branch keyed to mount type.
The native skin-mesh render at `0x004bb179` returns after the bone update and
mesh draw.

`MODERNIZACAO_COMPATIVEL`. Restore the missing KR effects on the corresponding
equipped mount meshes. Reuse texture 0 and existing emitters. Keep the client's
30 Hz emission clock for wolf mist and horse flames.

## Gaps

- Confirm the equipped effect in-game for the rider and an observer. Include
  movement, swapping mounts, removal, and relogin.
- The dispatch review covers equipped actor and skin-mesh rendering in the two
  reference binaries. Per-model material and animation comparisons are outside
  the evidence scope.

## Validation

- STATICALLY VERIFIED: Ghidra `ExportWydFlow.java` with seeds `004f7c95`,
  `004fdbb0`, `004bfeec`, `004fa63d`, `00500551`, and `004c5c20`. The export
  matched the native hash. Compared current appearance and render source and
  verified all eight effect texture hashes. The KR Ghidra exports and
  instruction ranges verify the three render branches and their call sites.
- AUTOMATED TESTED: Go `internal/wire` and `internal/game` client admission
  tests and the server build passed for the Fire Tiger candidate.
- AUTOMATED TESTED: KR catalog selectors match the four compiled visual rows.
  The three equipped rigs match KR and contain each required bone once.
  The texture path and hash match in all three clients.
- AUTOMATED TESTED: candidate 107 with the KR effects passed the incremental
  x86 Release build and its architecture, socket, crash capture, and asset gates.
- CLIENT-TESTED: pending.
