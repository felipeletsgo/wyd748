---
id: deferred-route-intent
title: Defer local route application until packet admission
subsystem: movement
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-11-02
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

## Validation

`CoreDataSafetyTests` checks replacement, due time, wrap, and cancellation.
`EntitySafetyTests` checks send-before-apply wiring and authoritative reset.
Build and integration receipts: `build/client-native-corrections-20261102/REPORT.md`.
