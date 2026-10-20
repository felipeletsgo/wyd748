---
id: gate-target-distance-closure
title: Distant movement target closes a visible gate locally
subsystem: gameplay
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-20
---

# Distant movement target closes a visible gate locally

## Question

Can a movement target close an open Noatun gate before the player leaves its view?

## Evidence boundary

- USED: The recorded 7.48 `WYD.exe` identity and the matching decompiled `FUN_004f2b52 @ 0x004F2B52` in `tmproject/build/native-research/decompiled/`.
- USED: `TMGate::FrameMove`, `Render`, the create/update/remove item handlers, and the existing ground-item state contract.
- USED: WYD-Go ground-item visibility at 32 tiles and authoritative gate state/terrain projection.
- NOT APPLICABLE: Assets do not define this distance transition; historical server projects are not parity authority.
- GAP: The original Ghidra project and live Noatun reproduction were not available for a complete call graph or gameplay validation.

## Native 7.48 flow

### Observable entry

The gate frame callback is reached for an existing object. In the matching native export, `FUN_004f2b52` compares gate coordinates with the focused human's target-coordinate fields at offsets `0x47C/0x480`, with a 33-tile margin. Outside that margin it writes gate state 2 (closed), clears its authorization short, and deletes item 359. An unauthorized gate stamps its mask at height `0x7E`. This matches the active source's former `TMGate::FrameMove` branch. This is native behavior, but not proof that it is safe with the current server's actual-position visibility contract.

### Callers

Virtual frame dispatch is inferred from the `TMGate::FrameMove` source; the native vtable/caller remains unresolved. Do not promote this record to `TRACED` without that check.

### Main function

`FUN_004f2b52 @ 0x004F2B52` has the distance comparison and local state mutation.

### Callees

It calls native mask update `FUN_005554CC` after authorization is cleared. The exact indirect ObjectManager deletion target remains unresolved in this bounded export.

### Outputs and errors

No packet is sent by the local distance branch. A missing focused human skips it.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| Distant target, visible gate | Target beyond 33 tiles | Native gate frame | Locally closed and unauthorized | Closed mask stamped; item 359 removed | No server confirmation |
| Server gate update | Existing gate | `0x374` handler | Server-projected state | State and heightmap refreshed | Missing object ignored |
| Leave actual view | Ground item visible | Server `0x16F` | Object removed | Hover cleared | Recreated on return via `0x26E` |

### Vtables, vptrs, and receivers

Native frame dispatch and receiver ownership remain unverified; this record stays `LOCATED`.

### Ownership

The scene/ObjectManager owns the gate; the local distance branch only mutates or deletes that object.

### Partial failure

A movement target can be far away while the player and gate remain adjacent, so a server visibility delta need not resend the gate.

### Cleanup and teardown

Normal `0x16F` removal and scene destruction remain unchanged.

### Shutdown

No independent gate shutdown transition was established.

### Logout and relogin

Server visibility snapshots recreate the object; live relogin remains untested.

## Wire, ABI, and resources

No wire or asset change. Existing `0x26E`, `0x374`, and `0x16F` boundaries are covered by `ground-item-state-contract.md`; this change affects only client-local frame behavior.

## Current mapping

### Buildable source

`TMGate::FrameMove` no longer closes or deletes a gate from movement-target distance. Packet create/update still authorize and set gate state; server removal still destroys it. `Render` retains the target-distance hover reset.

### WYD-Go

`visibility.go` uses actual player coordinates and a 32-tile view. `visibility_delta.go` sends `0x16F` when the item exits that view and `0x26E` on entry. `gate_key.go` retains authoritative opening and terrain updates.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Distant movement target closes the gate | Present in `FUN_004f2b52` | Removed from `FrameMove` | Former branch matched native | Visibility follows actual position | `MODERNIZACAO_COMPATIVEL`: server owns gate lifecycle; intentional local-behavior deviation |
| War/key state updates | Packet-driven | Unchanged | Unchanged | Authoritative | Preserve |

## Decisions

Remove only the local distance-driven state/auth/deletion branch. Preserve packet-driven close/lock, timed animations, and rendering. Do not claim native parity for this deviation.

## Gaps

The original project call graph and in-game Noatun scenario remain unverified. A server-side close or stale height-window bug could be independent; capture packet/state/collision if the issue persists.

## Validation

- Research: Matching native decompilation located; full call graph not proven.
- Automation: `Build-Client.ps1 -NoDeploy` passed, including the source contract
  test. Candidate `tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
  `A4C78CFA74786F6A9573B5887A09D73DDE37888873BB4A80E3A1F6AF3C40A5FC`.
  The installed runtime has a different hash and was not replaced.
- Real client: Pending installed-candidate movement, war closure, leave/reenter, and relogin checks.
