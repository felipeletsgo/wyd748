---
id: deferred-route-intent
title: Defer local route application until packet admission
subsystem: movement
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-11-09
---

# Deferred route intent

## Question

Does the native client apply a route before the one-second packet gate opens?

## Evidence boundary

USED: cached `FUN_00520216.c` under `tmproject/build/native-research/decompiled/`.
The cache header matches the original 7.48 executable hash above.
USED: current `TMHumanMovement.cpp`, `TMHuman::FrameMove`, and `PendingMoveTarget.h`.
NOT APPLICABLE: TMSRVIN and Core.dll cannot establish local rendering or input cadence.
Their server evidence is separate in the native REA archive.

## Native 7.48 flow

### Callers

Direct calls occur in `FUN_004776c3`, `FUN_00432959`, `FUN_00461ed8`,
`FUN_0046218b`, `FUN_004ac81e`, `FUN_0051a939`, and `FUN_0051a56c`.
The current field input, auto-run, and actor chase paths call `TMHuman::GetRoute`.

### Callees

`FUN_00520216` reads the server timer through its timer receiver, builds the
route and action message, sends it, and invokes local packet application.
The speed-4/5 outer gate uses 500 ms; speed-6/7 uses 100 ms.
The final network gate remains 1,000 ms, so local application can run alone.
The original implementation also contains this mismatch.

## State and lifecycle

Keep one pending destination per local actor. A later click replaces it.
Do not build or apply that route while the final send gate remains closed.
Retry through `GetRoute` when the gate opens. Recheck transition and route rules.
Apply route and stop state only after `SendOneMessage` accepts the packet.
Send failure retains the latest destination. This acceptance is socket queue
admission, not server approval; server collision and movement checks still apply.
Clear pending input on death, authoritative position reset, and actor destruction.
The special local-only stop value 2 retains its existing use.
Observers and selection/demo actors do not use this queue.

## Wire, ABI, and resources

Reuse the existing client-to-server `0x366`/`0x367` action messages, 52 bytes.
No new packet, coordinate, route encoding, or admission revision is allocated.
The actor owns the pending value; it holds no scene or actor pointers.

## Current mapping

`PendingMoveTarget` stores the latest destination and checks elapsed unsigned ticks.
`TMHuman::FrameMove` retries it. `InitPosition` cancels obsolete input.
`GetRoute` writes route buffers and local prediction after successful send admission.

Ground clicks retain their own world destination until arrival or cancellation.
`TMFieldScene::FrameMove` calls `UpdateGroundMove`. A click can replace the
current segment at step progress 0 or at least 0.9. Continuation waits for the
last step or for the actor to stop. It reuses `GetRoute` and the one-second
send gate. The local ground segment ends at its last encoded step. Padding
cells do not delay continuation. Each new endpoint must reduce the remaining
distance. A blocked route cancels the destination. Death, position reset,
air movement, auto-run, and a new movement action cancel obsolete ground input.

## Delta matrix

| Claim | Evidence | Decision |
| --- | --- | --- |
| Native final gate is one second | `FUN_00520216` | Preserve network cadence |
| Native can predict an unsent route | Same function | Defer prediction and retain the latest intent |
| New input must survive the gate | Queue fixtures and production wiring checks | Replace one pending value |

## Decisions

Classification: `MODERNIZACAO_COMPATIVEL`.
This corrects an inherited mismatch; it does not claim identical native input behavior.
The server continues to own movement validation and authoritative position.

## Gaps

Real client: pending. Check rapid destination changes, stop, chase, stun,
teleport, death, relogin, and observer movement at speeds 4 through 7.
No native binary was executed.

## WYD KR comparison

USED for comparison: `references/WYD_KR 7.70+/WYD.exe`, SHA-256
`71D2E7746831FAE932313602CFE827575FA3C2A0C61D3566142563B2814C872A`.
Read-only Ghidra exports are in `tmproject/build/native-research/kr-movement/`.
The KR binary is a separate client lineage and supplies no 7.48 parity proof.

- `FUN_004fc0c0` handles `WM_LBUTTONDOWN` after the UI and target checks.
  The ground branch calls `FUN_004e0b30` at `0x004fcba3`.
  This stores the picked tile and calls the route builder at step progress
  zero or at least 0.9. The constant at `0x00870f88` verifies the threshold.
- `FUN_0055f640` calls `FUN_0057dbe0` with a field limit of 12 steps.
  The outer gate uses 500 ms at speeds 4 and 5 and 100 ms at 6 and 7.
  The 52-byte normal walk uses `0x36C`. The send branch waits over 1,000 ms.
- The actor dispatch with `0x36C` runs outside that send branch. The actor
  handler `FUN_005662b0` reaches `FUN_00552a50`, which installs the route
  table and stamps the movement start. The KR client can replan locally
  during the send interval. WYD-Go queues the latest request during it.

The 7.48 field tick `FUN_004776C3` also retries the stored destination through
`FUN_00520216` near step completion. The compact tick omitted that call.
The compatible restoration keeps the final packet gate and applies only
admitted routes. A short segment can still end before the send gate opens.
KR FPS, frame pacing, and server behavior were not measured.

## Validation

`CoreDataSafetyTests` checks replacement, due time, wrap, and cancellation.
`EntitySafetyTests` checks send-before-apply wiring and authoritative reset.
`EntitySafetyTests` also checks mid-step retargeting, continuation at the
last step, and suppression when the active segment reaches the destination.
Build and integration receipts: `build/client-native-corrections-20261102/REPORT.md`.
