---
id: field-hud-death-prompt-regressions
title: Resource labels, death prompts, and native dye texture stages
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-27
---

# Resource labels, death prompts, and native dye texture stages

## Question

Why do compact HP/MP maximum labels regain a slash, and why does dismissing
the death prompt allow it to reopen without another player action?

## Evidence boundary

- Current implementation: `TMHuman.cpp`, `TMFieldScene.cpp/.h`,
  `DeathMotionPolicy.h`, and `ResourceBarProjectionTests.cpp` under
  `tmproject/TMProject748/`.
- Reused native contract: [restart recall request](../transport/restart-recall-request.md)
  and its existing `exports/restart-recall-flow.tsv` evidence.
- Continuation: verified the native SHA-256 above and inspected the analyzed
  native binary with Ghidra 12.1.2 headless, read-only. Focused exports under
  ignored `tmproject/build/native-research/` include `FUN_004c3eec.c`
  (skinned rendering), `FUN_004c3a1b.c` (materials), `FUN_0052a737.c`
  (inventory updates), and `FUN_00541065.c` (field objects).
- The user's screenshot and requested behavior identify the regression, but
  do not establish the complete native dialog or material implementation.
- Existing unrelated worktree changes were retained. This record describes
  only the changes made for the September 26 report and its continuation.

## Native 7.48 flow

The existing recall contract identifies `FUN_00476006` as the five-second
recall sender and `FUN_004776C3` as the prolonged-death maintenance sender.
Both send the header-only `0x289` request; the server controls revival.
This evidence does not prove the exact native prompt suppression policy.
The aggregate record remains `LOCATED` because the portal and runtime
consumption reports remain unresolved.

Native `FUN_004c3eec` selects dye effect textures for legends 116..125 when
refinement is positive. Its common hardware path uses MULTIPLYADD at stage 0
and ADD at stage 1, except black (120), which uses MODULATE. The effect uses
UV channel 1. The imported armor path instead used ADDSIGNED with channel 0
and a warm texture factor. `DyeTextureStages.h` restores the observed common
path and retains the existing legacy-adapter fallback. An additional native
fallback flag at device offset 0x2a5c8 remains unmapped; this is not a claim
of parity for every hardware configuration or for ordinary refinement glow.

## State and lifecycle

The first eligible death-animation completion or timeout offers the prompt
and records that it has been offered. Dismissal leaves that flag set, so a
later frame or animation completion cannot reopen it. A new world left- or
right-click may offer it again. Active town/revival channel timers suppress
both automatic and player-action offers. Positive authoritative HP resets
the flag for the next death.

Confirm explicitly hides the dialog, starts the existing recall channel,
and invalidates the last displayed countdown value so its first portal
effect is not skipped. Existing effect creation and server requests remain
responsible for the rest of the channel. No client-side revival is added.

## Wire, ABI, and resources

No packet, serialized resource, item rule, or server state format changes.
The existing delay-start and header-only recall requests are unchanged.
Only a local scene flag and helper are added.

## Current mapping

`TMHuman::UpdateScore` had two hardcoded maximum-value formats that bypassed
the shared `resource_ui::MaximumTextFormat` policy. Both now use that policy,
matching the already-correct score update path.

`TMFieldScene::OfferRespawnPrompt` centralizes eligibility and one-shot state
for compact/full input, the timed fallback, and local death-animation
completion. UI controls still receive input before world-click handling.

## Delta matrix

| Delta | Classification | Scope |
| --- | --- | --- |
| Reuse maximum label formatting | `MODERNIZACAO_COMPATIVEL` | Correct an inconsistent internal update path |
| Suppress repeated automatic prompts | `MODERNIZACAO_COMPATIVEL` | Local UI lifecycle; unchanged server authority and wire |
| Hide on confirm and initialize effect countdown | `MODERNIZACAO_COMPATIVEL` | Existing recall implementation |
| Common-path dye composition and UV channel | `PARIDADE_NATIVA` | `FUN_004c3eec`, legends 116..125 with positive refinement; implemented, not client-tested |
| Real-catalog Repliction regression coverage | `MODERNIZACAO_COMPATIVEL` | No product behavior or contract change |

## Decisions

- Implement the isolated UI regressions without changing recall authority.
- Do not infer native material settings or insert guessed portal objects.
- Do not consume items unconditionally on the client: rejection and server
  persistence failures must preserve authoritative inventory.

## Gaps

- Client execution remains required: HP/MP updates after combat/equipment,
  cancel then idle, cancel then movement/cast click, confirm with a visible
  five-second effect and authoritative revival, and a second death.
- Noatum/Nippleheim: server endpoints exist at `(1053,1709)` and
  `(3650,3109)`. The missing visual object/effect is unresolved; no portal
  patch was made. Field0813/Field2824 have no visual object within four units
  of these endpoints. Nearby mesh bounds do not cover the endpoints. The
  removed legacy CreateGate path only used InitItem entries; the inspected
  entries contain nearby victory towers, not an identified portal resource.
  Do not insert an unrelated mesh as a guessed replacement.
- Materials/dyes: the common dyed-mesh stage path is corrected, but visual
  comparison with native WYD.exe remains pending. Ordinary refinement glow,
  zero-refinement dyes, and the additional native fallback flag are not
  covered by this patch. No assets were changed.
- Grade A-E Repliction: real configuration entries `4016..4025` already
  specify `consume: true`. The user confirms the equipment bonus changes.
  New real-catalog tests cover all ten IDs, implicit/single/stacked amounts,
  inventory slot 37, successful save, decoded outbound 0x182 removal/update,
  and replay after the last unit. These pass; the remaining client symptom
  is not reproduced and no consumable product fix is claimed. Actual
  persistence/relogin and client icon behavior still require execution.

## Validation

- `STATICALLY VERIFIED`: maximum format call sites, automatic prompt entry
  points, confirm lifecycle, unchanged recall packets, and English authored
  text in affected source/test files were inspected.
- `AUTOMATED TESTED`: `Build-Client.ps1` passed 58,479 architecture checks,
  221 socket checks, and costume asset validation (135 items, 129 renderers,
  774 parts). Added policy cases cover automatic offer, dismissal, renewed
  input, channel suppression, and next-death eligibility. Release build and
  candidate deployment succeeded. Dye checks cover ten legends, two adapter
  paths, and two alpha modes (200 assertions). The build used the shared
  working tree, including other pending changes; it is not an isolated build
  of the task-only commit.
- Focused server command passed from `wydgo748/`:
  `go test -count=1 ./internal/game -run 'TestOnUseItemTintUntintReplictionAndFallback|Test.*Repliction|Test.*ConsumeOne'`.
- Candidate: `tmproject/client748/project.exe`, SHA-256
  `0DFC25714BF05E35B3216034486C7AFBE1E7260BF8C2ED21FC7C3ED38597BC5F`.
- `CLIENT-TESTED`: not performed. Neither the built candidate nor native
  WYD.exe was driven through these scenarios in this session.
