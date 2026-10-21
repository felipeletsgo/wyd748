# Native Combat Audit — 2026-10-21

## Scope and provenance

This audit covers the damage, defense, elemental resistance, healing, and
pending-HP paths touched during the recent corrections, followed by a final
review of the October 19-21 changes and the current working-tree batch on
`main`, based on `7ff2cfc94344c757db05525f60fe6be18a77d537`. Reviewed domains
include event clocks and admission, critical persistence, world fixtures,
boss configuration/runtime, item-table dependency closure, staff controls,
wire consumers, and client combat projection. This is not a claim that every
line or every possible gameplay sequence has been proven correct.

The available server
decompilation is `TMSRVIN.c` from the `descompilacao tmsrv 7.59` collection.
The `W2PP-master` sources are secondary historical evidence. Neither source is
treated as independent proof of the 7.48 contract. The repository rules require
7.48 runtime evidence or reproducible 7.48-contract validation before a native
parity claim.

## Corrections applied

- `wydgo748/internal/game/combat.go` now exposes the recovered physical mob-hit
  path for mob and summon attackers. It combines effective attack, Strength,
  Dexterity, and level, then uses the existing `BASE_GetDamage`-compatible
  `hitDamageWithRNG` path before applying target defense. This replaces the
  simplified `attack - defense/2` calculation used by summon-versus-mob hits.
- `wydgo748/internal/game/affects.go` and
  `wydgo748/internal/game/skills.go` apply the recovered class-1 elemental
  defense factor (`defense * 3 / 2`) to mob skill targets. The same isolated
  defense helper is used by the lightning-storm tick path.
- `wydgo748/internal/game/skill_pvp.go` applies the same class-1 elemental
  defense factor in the direct player-versus-player elemental-skill path. The
  existing double-defense rule remains explicit and separate.
- `tmproject/TMProject748/internal/application/AttackTargetState.h` and
  `tmproject/TMProject748/internal/app/scenes/TMFieldSceneCombatPackets.cpp`
  keep pending HP in a signed, clamped 32-bit calculation. Damage cannot
  underflow, healing cannot exceed maximum HP, and damage-rate scaling is
  applied only to regular pools. Large HP pools no longer pass through a native
  `short` truncation.

The existing Go `hitDamageWithRNG` and `skillFinalDamageWithRNG` implementations
already match the recovered `BASE_GetDamage` and `BASE_GetSkillDamage` integer
stages from the secondary source, including the combat clamps, random offset,
integer division, and minimum damage behavior. The correction was therefore to
use the recovered path at the missing mob/summon call site, not to rewrite the
shared formulas without 7.48 evidence.

## Final-review findings and corrections

All findings below were confirmed in the reviewed source. The fixes are
implemented and covered by focused regressions; gameplay acceptance is pending.
The internal fixes are `MODERNIZACAO_COMPATIVEL`. Client wide-resource changes
repair consumers of the existing `EXTENSAO_COORDENADA` uint32 snapshot/DMGX
contract; they add no opcode, packet layout, capability, or native-parity claim.

| Severity | Location | Trigger and violated invariant | Correction and evidence |
| --- | --- | --- | --- |
| High | `internal/game/ghost_shop_honor.go`, `tickGhostShopHonor` | With critical persistence active, the synchronous save guard rejected honor after live mutation, preventing durable periodic credit. | Queue the account/sidecar transaction, restore preimages before admission, and publish the deadline/letter only after commit. `TestGhostShopHonorCriticalCommitAndRetry` covers held, rejected, retried and committed writes without duplicate credit or synchronous fallback. |
| High | `internal/game/colosseum.go`, entry and event clock | A pending entry charge could allow waves/gates to advance before payment committed. A clock jump or delayed entry could also bypass normal finalization. | Keep entrants unpaid until commit, fence wave progression, and finalize at the recorded deadline even outside the event hour. Tests cover commit, rejection, a skipped hour and revival through the coordinator. |
| High | `internal/game/colosseum.go`, arena exit | A rejected exit write was forgotten, potentially leaving a player inside after the event. | Retain pending exits independently of the event stage; retry outside the event hour without duplicating held writes, and clear only after commit or departure. `TestColosseumRetriesRejectedExitAfterEventHour` exercises the real admission/completion path. |
| High | `AttackTargetState.h`, `TMFieldSceneCombatPackets.cpp` | Overkill wrapped unsigned pools; nonpositive rates and compact-prefix damage could produce invalid or stale HP, including valid damage above one million. | Use signed 64-bit intermediates with saturation, normalize rates, and apply validated wide damage/healing in both receive loops. Regression grids cover UINT32 boundaries, wide signed amounts, regular/big pools and pending HP. Legacy comparisons are restricted to their non-overflow domain. |
| Medium | `TMHumanPackets.cpp`, `OnPacketSetHpMp` | An authoritative snapshot changed the score but left active big HP/MP pools stale; subsequent drawing or combat could restore the old value. | Synchronize active pools after score clamping and before drawing; retain inactive-pool behavior. Helper tests plus a production-call ordering check cover both resources. Current HP/MP text uses unsigned formatting. |
| Medium | `internal/game/boss_actions.go`, shield mitigation | `damage * (100 - absorbed)` overflowed uint32 before division. | Widen multiplication to uint64. `TestBossMitigationDoesNotOverflowWideDamage` covers maximum uint32 damage and shield boundaries. |
| Medium | `internal/game/boss.go`, `RegisterBoss` | Registering a second runtime for one mob discarded encounter/add ownership. | Reject duplicate registration without replacing the existing runtime. `TestBossRegistrationPreservesExistingEncounter` also verifies add cleanup. |
| Medium | `internal/game/ox_event.go`, side rectangles | The X/O regions overlapped near the gate while status counts used separate sides; result handling could move the opposite side or neutral column. | Use the same disjoint regions for status and result handling, with the gate neutral. `TestOXResultMatchesStatusAtTheGateBoundary` covers columns 1340-1343 and both result commands. |

Paths without a source-root prefix in this table are relative to `wydgo748/`;
the C++ filenames are under `tmproject/TMProject748/`.

The suspected Magic Chamber restart defect was not changed: its private rooms
are explicitly process-local and non-resumable in `instance_state.go` and the
existing bug register. Runtime isolation does not imply durable restoration.

## Deliberately unchanged pending evidence

- The native 7.59 decompilation shows a target-player defense multiplier of 2
  in one physical path, while the secondary source shows 3 for a related
  normal-physical path. The available evidence does not identify the exact
  7.48 call contract, so player physical paths were not silently changed.
- Critical-hit ranges also conflict: the native path exposes `% 3 + 13` for a
  player target and `% 4 + 15` for a mob target, while the secondary source
  exposes different ranges. No 7.48 evidence was found to justify replacing
  the current project behavior.
- The native helper called by the direct physical path is unresolved in the
  available decompilation. Its final distribution, flat absorb/bonus stages,
  and all caller-specific reductions cannot be safely reconstructed from the
  name or from a later-version listing.
- The current Foema healing rule (`140%` with a `548` cap) conflicts with the
  secondary source's evolution-specific `510`/`1100` caps and familiar-index
  reductions. The exact 7.48 healing contract was not recovered, so no healing
  formula was replaced by an unverified later or secondary rule.
- The special Hell Gate resistance-compression path remains isolated from the
  normal elemental formula. It was not generalized to all damage types.

## Validation

The following checks passed after the corrections:

- Focused Go combat regression tests, including class-1 elemental defense,
  mob/summon physical damage, lightning storm, summon combat, and PvP defense.
- `go test -count=1 -p 1 ./...` with `WYD_TEST_POSTGRES_URL` pointing only to a
  disposable PostgreSQL 18 instance on `127.0.0.1:55483/wydgo_test`.
- Dedicated verbose PostgreSQL tests confirmed actual execution, not skips:
  account transfers/repetition/rollback, all critical game-state aggregate
  forms, late rollback, instance atomicity and player-state batch atomicity.
  `TestCriticalActivatedWorldPostgresEndToEnd` also passed. Its shutdown log
  included a canceled premium-poll context after World shutdown; the test
  passed and this is not evidence of a production persistence failure.
- Focused honor, Colosseum, OX and boss safety regressions, including the final
  added skipped-hour case.
- `go vet ./...`
- `go build ./...`
- Client release build through `tmproject/Build-Client.ps1 -Configuration
  Release -NoDeploy`.
- Client checks: 3,105,526 architecture checks, 281 socket tests, 198 crash
  capture tests, and 56 launcher tests, all passed.
- Portal `npm run check`: 77 files, zero errors/warnings/hints; `npm test`:
  60 passed, zero failures/skips.
- `git diff --check` and `tools/repository/Test-RepositoryLayout.ps1`: passed,
  215 indexed documents. The new record was indexed with `-UpdateMap`.

The resulting candidate executable is
`tmproject/build/TMProject748/Release/WYD.exe`, 1,916,928 bytes, SHA-256
`64C9BDDEEC0EE6E6197B63A0CA915B10BE1AEA8F1C6B03A1F9D3EC47428C3F82`.
The architecture count changed because comparisons against legacy wrapping
arithmetic now exclude its overflow domain; new independent saturation and
snapshot regressions cover those boundaries instead.

## Delivery boundary

At the original review boundary this verified source and candidate-build
validation only. No installation, publication, restart, disconnection, commit,
push or actual gameplay occurred in that review phase.

At that boundary the local client build also depended on the untracked
`tmproject/client748/ItemListWide.csv` sidecar. The subsequent authorized
publication committed that sidecar in `6746e784` and
pushed the source and portal receipt (`60c2b618`) to GitHub `main`.
See [current state](../../DOCS/current-state.md) for the subsequent publication:
controlled shutdown completed with one account saved and zero failures.
Transient SSH TCP timeouts delayed activation; access recovered after backoff
without a security-policy workaround. Patch 10041 is publicly hash-verified,
and `/srv/wyd-go/releases/20261021T190239Z` is active with matching process
hashes and successful readiness checks. No actual client gameplay was validated
by that operation.

Native 7.48 formula conflicts above, real-client owner/observer and relogin
flows, installed-artifact identity, and global English-migration acceptance
remain separate gates. Arithmetic tests at UINT32 limits do not validate the
signed-range progress-bar renderer at HP above INT_MAX; that UI boundary still
needs end-to-end review and must not be inferred from helper results.

## Cleanup limitation

The disposable PostgreSQL instance was stopped successfully with `pg_ctl -m
fast -w stop`. Environment policy rejected the subsequent cleanup command
before execution. No alternate deletion mechanism was attempted. Leftovers:

- `build/final-audit-20261021/postgres/` (stopped, disposable test cluster);
- `build/final-audit-20261021/postgres.log`.

These are this audit's ignored artifacts, not player/account production state.
The current incremental client build is retained under its sanctioned root.
