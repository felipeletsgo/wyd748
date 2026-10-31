---
id: player-reported-skill-effects
title: Restore reported spell components and preserve wide mana
subsystem: combat
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-30
---

# Reported spell effects and wide mana

## Question

Which client paths can truncate local MP or omit components of the reported
Phoenix, Meteor Storm, Inferno, Blizzard, Resurrection, and Judgment effects?

## Evidence boundary

- USED: saved native 7.48 exports under `tmproject/build/native-research/decompiled/`:
  `FUN_00489a3e`, `FUN_004e9cf0`, `FUN_004e6590`, `FUN_004e74b5`,
  `FUN_004e754b`, and `FUN_004d5480`.
- USED: existing [skill dispatch record](skill-visual-dispatch.md), current
  C++ effect classes and server combat/vitals serialization.
- USED: owner screenshots identify symptoms, not the full packet sequence.
- CONTRADICTORY: the later Meteor constructor creates its core only for level 6;
  native 7.48 explicitly creates components for levels 0, 4, and 6.
- NOT APPLICABLE: server 7.59 exports cannot prove client 7.48 visual parity.
- No reference binary was executed or modified. Asset identities are unchanged.

## Native 7.48 flow

### Callers

`FUN_00489a3e` dispatches attack packets. Branches 35 and 39 construct
`FUN_004e9cf0`; branch 99 constructs `FUN_004d5480`. Branches 30 and 23
construct `FUN_004e6590`.

### Callees

`FUN_004e9cf0` creates luminous billboards with `FUN_004c8f70`, shades with
`FUN_004df250`, and attaches them through `FUN_0054ac09`. Level 1 advances a
modulo-five sequence before multiplying the projectile lifetime by 30.
Levels 0 and 4 also create ground lights; their core scales are 1 and 0.5,
while level 6 uses 2.5 and 1.5. The restored branches use the native colors.
`FUN_004d5480` constructs the character skin effect. The attack branch copies
the target look, uses a 3000 ms lifetime, bright blending, fade, and motion 8.
`FUN_004e6590` creates finite 1500-10000 ms billboards despite its 300 ms
controller lifetime. `FUN_004e74b5` removes referenced children at destruction;
this legacy lifetime coupling is not retained as a correctness requirement.

## State and lifecycle

Attack receipt selects the caster, target or ground coordinate, motion and
deferred event. Scene containers own temporary visual objects and delete them
on expiration or scene teardown. Meteor core pointers retain existing cleanup.
Phoenix updates its child mesh independently of shared particle throttling.
Judgment relinquishes finite scene-owned billboard pointers after creation;
type 1 retains explicit cleanup because its child lifetime is infinite.
The controller expires even when outside the view. Partial allocation failures
retain existing guards. Resurrection tolerates a missing skin mesh.

Login, death and teleport state remain server-owned. A compact attack carries
MP in a WORD; it cannot represent the screenshot's maximum MP of 322670.
Local attack updates now preserve wide authoritative score/vitals instead of
installing that WORD. Logout discards actors; relog receives a fresh wide score.
No packet capture proves which exact attack/preview caused the reported drop.

## Wire, ABI, and resources

No packet size, opcode, effect ID, texture index, mesh path, or asset format
changes. Existing compact attacks use MP at 26, skill at 24 and FlagLocal at 30;
wide score/vitals remain the resource authority. Candidate admission revision
16 is paired in both projects. Revision 15 remains the published active build.
Guardian Cannon retains item 746, reserved map IDs 15001-15100, distance at
least four, server target authorization, and the existing projectile path.
Its client arc uses cosine to handle the zero-angle seam.

## Current mapping

- `AttackAttackerState.h`: preserve local wide MP on compact attacks.
- `TMSkillMeteorStorm.cpp`: core creation and Blizzard cadence.
- `TMSkillMagicArrow.cpp`: independent per-frame child mesh update.
- `TMSkillJudgement.cpp`: finite child ownership and controller expiry.
- `TMFieldSceneCombatPackets.cpp`: Resurrection character effect and aura.
- `CannonTargeting.h`, `TMFieldSceneSkills.cpp`, server `skills.go`: cannon targeting.
- Server `affects.go`, `skill_pvp.go`, `charstate.go`, `model.go`, and
  `model/charstate.go`: four poison ticks and optional persisted cadence.
- Server `handlers.go` and `timed_items.go`: authoritative secondary hit and
  equipped countdown refresh. No gameplay or source files were removed.
- `TMFieldSceneMerchant.cpp` and `PremiumNell.html`: localized hover content.
- Root/EN/PT `skillhelp.dat` and `UI/strdef.bin`, their locale policy and tests:
  reviewed poison duration and number-before-unit timer labels.

## Delta matrix

| Claim | Classification | Evidence |
| --- | --- | --- |
| Meteor/Inferno cores, Blizzard cadence, Resurrection character | PARIDADE_NATIVA | Saved native constructor/dispatch branches |
| Wide MP preservation, Phoenix update, Judgment ownership, cannon seam | MODERNIZACAO_COMPATIVEL | Current source defect; unchanged wire/resources |
| Poison Mist duration and Iron Spear adjacency | EXTENSAO_COORDENADA | Explicit owner request, authoritative server rules and regression tests |
| Nell hover, full-heal numbers, localized timer | MODERNIZACAO_COMPATIVEL | Existing help, formula and duration data |

## Decisions

Restore only traced native components. Keep compatible lifecycle corrections
separate from parity claims. Publish a mandatory client patch before activating
the new server minimum. This implementation does not authorize publication.

## Gaps

No game launch, screenshot comparison, installed executable replacement, or
live acceptance occurred. The exact return-teleport packet sequence remains
unobserved. Compilation cannot establish complete visual equivalence.
Database validation covers the production PostgreSQL JSON payload encoder;
no live PostgreSQL instance was exercised in this batch. No SQL schema changed.

## Validation

- STATICALLY VERIFIED: native comparisons and current source/wire paths above.
- AUTOMATED TESTED: Release build; architecture, socket, crash and launcher
  checks; full Go suite/vet; focused race tests; Nell VM and EN/PT locale checks.
- Return-scroll test preserves MP 322670. Attack policy tests reject compact
  local overwrites. Cannon tests cover the seam and rear half-plane.
- Poison tests cover four floating hits, tick delay, repetition and reconnect.
  Iron Spear tests cover adjacent selection without nomination and one extra hit.
- CLIENT-TESTED: pending. Build output is candidate 16, not the installed client.
