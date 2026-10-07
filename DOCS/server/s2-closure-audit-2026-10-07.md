# S2 closure audit — 2026-10-07

## Decision and scope

The production entry point enables S2; the previous staged-only status no
longer applies. The initial audit reproduced three shared-object lifecycle
defects. The subsequent user-authorized S2 correction settles certified ground
rejections and retains global containment for uncertain ground/mob outcomes.
All 11 original acceptance cases now pass without weakened assertions. A
follow-up correction reproduces and removes the timed account release and
speculative instance-reader exposure described below. Verified recovery of an
unknown outcome remains a separate outstanding requirement.
The successful activation and client-session evidence recorded on October 6
remains historical evidence for the flows it actually exercised.

This is a bounded audit of the current working tree on `main`, based on
`34b7dcae`, not a certification of every change in the large uncommitted batch.
Existing changes were preserved. The initial audit added tests and documentation
only. A subsequent user-authorized inventory/CP correction batch is documented
in [its versioned contract](../client/inventory-stack-and-cp-contract.md); it
does not correct S2. Later S2-only corrections address the three findings and
the two follow-up risks below. Nothing was deployed, committed, or launched as a product; PostgreSQL
validation used an isolated fixture. Existing wire and persisted representations
are retained. Classification: `MODERNIZACAO_COMPATIVEL`.

Reviewed areas: activation and certified adapters, completion ownership,
account containment/disconnects, ground pickup/drop, kill reward settlement,
generic account and instance mutations, instance exit and visibility readers,
weather scheduling/damage, and client weather/cache/corner policy tests.

Documentation reviewed includes the current activation section of the
[persistence plan](critical-persistence-plan.md),
[Hell Gate store contract](../protocol/hell-gate-store.md),
[performance and visuals plan](../client/performance-and-visuals-plan.md),
[data consistency findings](../protocol/data-consistency.md), and the correction
follow-ups in the two October 3 client audits. Old audit findings were not
automatically treated as current defects where a later correction supersedes
them. Generated matrices and recorded client tests are evidence, not a fresh
runtime execution by this audit.

## Original confirmed findings (corrected)

The descriptions and reproductions in this section record the initial audit,
before remediation. Their former failures now pass; the correction and its
additional coverage are recorded below.

### S2-A01 — P2: certified ground rejection is not settled

`queueGroundPickup` and `queueGroundDrop` in
`../../wydgo748/internal/game/critical_persistence_ground.go` submit only a
success continuation. The owner needs an explicit rejection continuation to
settle a certified pre-commit rejection. Without one, even a certified rejection
enters failure containment, closes/poisons the participant, and retains the
ground reservation. Pickup leaves the unchanged object reserved; drop retains
an invisible unpublished allocation despite the source item remaining in RAM.

Reproduction: `TestClosureAuditGroundSettlement/pickup/rejection` and
`TestClosureAuditGroundSettlement/drop/rejection`. Both fail their intended
acceptance conditions; account preimages remain unchanged.

Required correction: an identity-checked certified-rejection continuation that
releases pickup ownership or unregisters the unpublished drop, resynchronizes
the participant, and does not certify an unknown error as rejected.

### S2-A02 — P1: local containment abandons reserved ground objects

The same operations carry accounts but no guild/instance aggregate. Therefore
`containCriticalFailureLocally` treats them as account-local even though their
continuations also own shared ground objects. After one minute,
`releaseCriticalQuarantine` deletes the continuation and acknowledges the queue
entry. Neither ground reservation is settled. `expireGroundItems` deliberately
skips reserved objects, so advancing expiry by a full day still cannot remove
them after the account fence and player have been released.

Reproduction: `TestClosureAuditGroundSettlement/pickup/unknown` and
`TestClosureAuditGroundSettlement/drop/unknown`. Both fail after quarantine
release and expiry processing. This proves stranded objects, not duplication.
Repeated failures can accumulate unavailable loot or invisible allocations.

Required correction: classify shared-object ownership separately from the
database write's account list, then retain shared containment or reconcile the
object with the actual durable outcome before releasing ownership. Simply
clearing the reservation or publishing a drop on an unknown outcome would be
unsafe: the account write may have committed.

### S2-A03 — P1: local containment abandons an unpublished dead mob

`queueCriticalKillRewards` restores account preimages and keeps a mob dead but
unpublished until completion. Ordinary kills have account-only writes and thus
enter local containment on an unknown outcome. Failure clears the success and
rejection closures; quarantine expiry releases the account, but the mob remains
registered and `Dead`. Neither successful finalization/removal nor certified
rejection/resurrection runs. Combat and movement ticks skip dead mobs, and the
normal generator/boss progression belongs to the missing finalization path.

Reproduction: `TestClosureAuditKillSettlement/unknown` fails after disconnect
and quarantine release. The matching commit and certified-rejection controls
pass, including restoration of pre-death HP on certified rejection.

Required correction: retain or reconcile the shared mob lifecycle as part of
unknown-outcome containment. Do not resurrect or credit rewards blindly.

## Follow-up risks and existing documented gaps

- The initial fixed one-minute quarantine did not prove that a database COMMIT
  whose acknowledgement was lost had finished. The follow-up reproduces fence
  release after one minute and removes automatic release altogether. A real
  PostgreSQL delayed-settlement test verifies continued containment; it models
  uncertainty in the adapter, not loss of a TCP COMMIT response. No duplication
  or restart-recovery certification is claimed.
- The initial `commitInstanceMutation` retained speculative player/instance
  state and relied on reservations. The follow-up reproduces an avatar visible
  to an unrelated observer before acknowledgement, then changes the bridge to
  restore public RAM before admission yields and install only on verified
  completion. The visibility test also covers completed-but-unacknowledged
  writes, certified rejection and unknown outcomes.
- Unknown-outcome reconciliation remains unimplemented. An account stays
  isolated, and its fences and capacity stay retained, until verified recovery.
  An ordinary account reload or elapsed time is not settlement proof. Persistent
  operation identities, safe release and cross-process recovery require a
  separately designed and tested workflow; restarting is not an approved
  substitute for reconciling an in-flight transaction.
- The performance plan correctly keeps real field crossings, weather damage
  in gameplay, lighting tuning, and accumulation measurements pending. Passing
  cache-policy tests does not establish scene object lifetime or real-play
  performance. The recorded five-client server-selection measurement covers
  that scene only.
- The data-consistency document still records server-only item rows, mesh/name
  differences, and the unmodeled native `ForceDamage` field as follow-up work.
  This audit does not promote those gaps to native parity. The client audio
  cleanup supersedes the old dangling-reference finding; duplicate sound ID
  220 and real playback remain separately documented gaps.

## Initial audit tests and validation (historical)

New file:
`../../wydgo748/internal/game/critical_persistence_closure_audit_test.go`.
It uses held workers, adapter-certified rejection, a fake clock for ground
expiry, and explicit account acknowledgement. No client session, operational
database writes, sleep-based retry, or repeated migration cycle is involved.

The new acceptance matrix has 11 leaf cases: **6 pass, 5 fail**. The failures
are the four ground failure cases and the unknown kill case above. Passing
controls cover pickup/drop commit, kill commit/rejection, weather catch-up after
acknowledgement without duplicate broadcasts, and independent weather RNG with
inclusive minimum/maximum scheduling. The five red tests are intentional
acceptance gates for real defects, not passing reproduction assertions or skips.

| Gate | Initial audit result |
| --- | --- |
| `go test -count=1 -timeout=90s ./internal/game -run '^TestClosureAudit' -v` | 6 passing leaf cases; 5 failing acceptance cases |
| `go test -json -count=1 -timeout=180s ./...` | 14 tested packages pass; `internal/game` fails only the five new acceptance cases; four additional command packages have no tests |
| `go vet ./...` | Pass |
| `CGO_ENABLED=1 go test -race -json -count=1 -p 1 -timeout=240s ./...` | Completed: 14 tested packages pass; `internal/game` fails only the same five new acceptance cases; four additional command packages have no tests. This is not a clean race acceptance gate |
| MSBuild `ArchitectureTests.vcxproj`, Release/Win32, followed by `ArchitectureTests.exe` | Pass: 1,152,647 checks and static assertions after the inventory/CP batch; includes weather, cache, corner, lighting, settings, trace, and new UI/name gates |
| PostgreSQL integration | Pass in a disposable PostgreSQL 18 fixture on verified loopback port 55484, database `s2_audit`, role `audit_runner`; store race integration completed in 3.192s. Fixture was stopped after validation; no operational database was used. Two unrelated external-fixture tests remained skipped, not passing evidence |
| Client runtime / rendering / installed executable | Not run, installed, or overwritten |
| `Test-RepositoryLayout.ps1 -UpdateMap` and scoped `git diff --check` | Pass: local links/layout validated and 183 documents indexed; no scoped whitespace errors. The central map includes the new inventory/CP contract and UI evidence record |

At that stage, no clean full-suite or race acceptance was claimed while the
new regressions were red. Previous passing suites did not exercise these production containment
combinations. Adapter integration ran on isolated PostgreSQL, but database
fault injection of unknown-outcome/shared-object reconciliation remains pending
independently of these deterministic in-memory failures. The affected
game/model/wire race batch after the inventory/CP corrections reproduced only
the same five acceptance failures and reported no data race. Seven
PostgreSQL-dependent leaf cases skipped in that follow-up because the earlier
fixture was already stopped; its still-valid adapter evidence is reused, not
represented as a fresh database run of the new merge.

The repository-wide census also reviewed changed client resources, project
XML registrations, audio identities, data matrices, weather boundaries, and
configuration bounds. Twenty new Python audit checks passed; existing
extraction/literal/opcode/relocation/worktree checks passed. Source-separation
hash checks still report stale baselines for independently changed methods;
they were not blindly regenerated. A nonignored user-owned root
`build/audit-pg-80354e58c08048039e4bcfed5e5ad061/data` contains 1,312 local
PostgreSQL/build files and must be reviewed before publication; it was not
removed, staged, or treated as product source.

## Completed remediation — 2026-10-07

- `critical_persistence_ground.go` now supplies explicit certified-rejection
  callbacks for pickup and Carry/Cargo drop. The same bound identity, object,
  reservation, context and economic-source checks protect both success and
  rejection. Pickup releases the unchanged loot; drop unregisters its never
  published allocation. Open transports receive authoritative preimage
  snapshots and a failure message, never a success confirmation. Closed
  transports receive no packets.
- `criticalContinuation.sharedWorld` records actor-owned gameplay ownership
  separately from durable aggregate type. Ground operations and kill rewards
  use the shared-write admission wrapper. Failure retains this metadata even
  after the non-replayable closures are cleared.
- Account-local quarantine refuses shared-world operations. Unknown outcomes
  and failed settlement checks close admission, poison/disconnect players and
  enter global maintenance. Account and object reservations remain held;
  quarantine/expiry cannot release them. Shutdown reports failure and does not
  persist the retained RAM preimage. There is no blind resurrection, loot
  publication, rollback, retry or automatic shared-object reconciliation.
- At this first correction stage, account-only containment stayed unchanged.
  The subsequent follow-up below removes its unsafe one-minute release.

Additional tests:

- `critical_persistence_shared_settlement_test.go`: 35 deterministic cases:
  15 certified-rejection cases (pickup/fallback/currency/Carry/Cargo crossed
  with open/closed/private-instance-release), 14 changed-identity/source cases,
  and six uncertain shared-outcome/shutdown cases. Assertions cover encrypted
  packet ordering, no observer publication, reservation guards, duplicate
  completions, new intentions after definite rejection and retained capacity.
- `critical_persistence_shared_postgres_test.go`: 12 cases, pickup/drop/kill
  crossed with commit, certified ledger rejection, lost acknowledgement after
  commit, and lost acknowledgement after rejection. Transactions and ledger
  rollback execute in PostgreSQL 18; the test adapter deliberately masks a
  known outcome after execution. Durable reloads verify both possible database
  states while actor preimages and shared fences stay contained when unknown.
  This is not a TCP-loss/late-COMMIT or account-readmission timing test.

Validation commands and final integrated results are recorded below. The
fixture uses the verified ignored `tmproject/build/pg-audit-67d6b7d98c534474885db9a477dec24f`
directory, loopback port 55484, database `s2_audit`, and role `audit_runner`.
No operational database or client is used.

| Correction gate | Result |
| --- | --- |
| Original `TestClosureAudit` acceptance matrix | Pass: all 11 cases; assertions preserved |
| Focused shared-settlement/identity/shutdown matrix | Pass: all 35 new cases |
| `go test -count=1 -timeout=90s ./internal/game -run '^TestCriticalSharedPostgresSettlement$'` | Pass: all 12 PostgreSQL cases, 0.905 s |
| `go vet ./...` | Pass |

Final correction integration (not a rerun of the initial failing audit):

| Gate | Result |
| --- | --- |
| `WYD_TEST_POSTGRES_URL` set to the isolated fixture; `CGO_ENABLED=1 go test -race -count=1 -timeout=300s ./...` | Pass: all 15 tested packages; four command packages have no tests. Game completed in 87.740 s and webadmin in 124.065 s; no races reported. Includes the new PostgreSQL shared-object matrix |
| `go build -o ../tmproject/build/s2-fix-server.exe ./cmd/server` | Pass: `STATICALLY VERIFIED`; ignored candidate only, never run or deployed |
| `Test-RepositoryLayout.ps1` | Pass: local links/layout valid; 183 documents indexed; no map rewrite needed |
| Scoped `git diff --check`, plus `git -c core.autocrlf=false diff --no-index --check -- NUL <file>` for the new Go sources | Pass: no whitespace errors; new untracked files checked explicitly |
| Isolated PostgreSQL cleanup | Fixture stopped and stop status verified; no files deleted and no operational service changed |
| Client execution | Not performed; this server-only batch does not provide new `CLIENT-TESTED` evidence |

`TestStartWebAdminPostgres` requires the separate `WYD_WEB_TEST_DATABASE_URL`
fixture; `TestProjectAccountsUseCurrentSchema` requires local JSON accounts.
Those unrelated external-fixture gates remain skipped and are not passing
evidence. The S2 PostgreSQL gates were enabled, executed and passed.

Correction file scope: `critical_persistence_ground.go`,
`critical_persistence_kill_rewards.go`, `critical_persistence_owner.go`,
`critical_persistence_lifecycle.go`, `critical_persistence_containment.go`,
`critical_persistence_closure_audit_test.go`, and the two new shared-settlement
test files listed above, all under `wydgo748/internal/game/`; this audit and
`critical-persistence-plan.md` are the two documentation updates. Only the
comment changed in the original acceptance test; its assertions were preserved.

## Original remediation order (completed)

1. Settle certified ground rejections with bound rejection continuations.
2. Extend unknown-outcome ownership/containment to shared ground and mob state;
   require durable reconciliation or safe global containment, not blind rollback.
3. Make these five acceptance cases pass, then exercise the same failures
   against a verified isolated PostgreSQL fixture and complete integration/race
   acceptance. Assess timer fencing and instance-reader hypotheses separately.

The three steps above are completed by the first S2 correction. The follow-up
below addresses the timing and instance-reader risks. No files were removed.
Initial authored changes include the closure,
configuration, weather-wire, field-boundary, C++ weather/corner, resource and
matrix audit tests/tools, this report, the persistence-plan audit link, Hell Gate
activation wording, and central inventory. The later production/UI changes and
their tests are listed in the inventory/CP contract. Generated isolated test
artifacts stay under the existing ignored `tmproject/build/` directory.

## Follow-up correction: account timing and instance readers

- Removed `releaseCriticalQuarantine` and the World tick call. The quarantine
  timestamp is diagnostic only. Account-local unknown outcomes still permit
  unrelated accounts to progress, but retain participant identity, account
  fences, pending disconnects and queue capacity indefinitely. No speculative
  terminal save, timed readmission or blind replay is introduced.
- `commitInstanceMutation` captures detached account, position and runtime
  proposals, executes the caller's rollback synchronously, and rebuilds the
  membership index before queue admission. Installation verifies unchanged
  participant/account/position and aggregate identities, preserving existing
  account/character and runtime pointers. Changed preimages or unknown outcomes
  enter global containment without installing, rolling back or publishing them.
  Definite rejection uses the already-restored state and releases admission.
- Uxmal reward floor allocations are reserved and invisible while pending.
  Commit installs the detached runtime completion marker and releases/publishes
  the allocations; definite rejection or admission failure unregisters them.
  Ground identity/content changes retain global containment. Uxmal remains a
  process-local, non-resumable event: this patch does not persist floor loot or
  add Uxmal to the durable instance snapshot contract.

Follow-up regression coverage:

| Test | Coverage |
| --- | --- |
| `TestCriticalAccountContainmentIsolatesOnlyUnknownParticipants` | One minute, one day and thirty days do not unlock a poisoned account; unrelated writes progress and readmission remains denied |
| `TestCriticalInstanceMutationHidesProvisionalState` | Three outcomes; unrelated visibility reader before completion and before actor acknowledgement |
| `TestCriticalInstanceMutationRejectsChangedPreimage` | Eight cases: account, position, runtime contents and runtime-pointer replacement crossed with commit/rejection |
| `TestCriticalInstanceConsumersRestoreBeforeAdmission` | Twelve cases: room move, quiz, Big Cube and expiry exit crossed with commit/rejection/unknown; active coordinates are checked separately from normalized durable entry positions |
| `TestCriticalUxmalRewardPublicationAndMarker` | Ten cases: inventory/floor rewards, commit/rejection/unknown, admission failure and floor mutation; no premature reward marker or packets, no stranded rejected allocation, repeated completed grant adds no write |
| `TestCriticalAccountPostgresLateCommitRetainsContainment` | Real PostgreSQL account write completes after the worker reports unknown and the clock advances a day; durable gold changes while old RAM never overwrites it and the account remains fenced |

Focused regressions passed. Source changes: `critical_persistence_containment.go`, `world.go`,
`critical_persistence_instance_commit.go`, and `uxmal.go`. Existing containment,
closure-audit and shared-settlement/PostgreSQL tests use the real tick instead
of the removed release helper. Three new regression files cover visibility,
instance consumers and delayed PostgreSQL settlement. This audit and the
persistence plan are updated in place; no new handoff, source documentation,
schema, wire contract or operational database modification is introduced.

Follow-up integration results:

| Gate | Result |
| --- | --- |
| `WYD_TEST_POSTGRES_URL` set to the isolated fixture; `CGO_ENABLED=1 go test -race -count=1 -timeout=300s ./...` | Pass: all 15 tested packages; four command packages have no tests. Game completed in 88.671 s and webadmin in 124.178 s; no races reported. Includes the delayed real PostgreSQL commit regression |
| `go vet ./...` | Pass |
| `go build -o ../tmproject/build/s2-pending-server.exe ./cmd/server` | Pass: `STATICALLY VERIFIED`; ignored candidate only, never run or deployed |
| `Test-RepositoryLayout.ps1` | Pass: links/layout valid; 183 documents indexed |
| Scoped whitespace and English checks | Pass: affected untracked Go sources checked explicitly as well as tracked changes |
| Isolated PostgreSQL cleanup | Fixture stopped; process status and absence of a listener on port 55484 verified. No files deleted or operational services changed |
| Client execution | Not performed; no new `CLIENT-TESTED` evidence |

The two unrelated external-fixture skips described above remain pending. The
S2 PostgreSQL gates executed against the isolated fixture. This correction does
not close verified unknown-outcome recovery: durable operation identities,
cross-process reconciliation and a verified account-unlock workflow still need
implementation and validation. A timer or process restart is not proof that an
uncertain commit settled. No files were removed.

## User-reported gameplay validation and publication

On 2026-10-07, the user reported an in-game test with apparently correct
behavior and authorized committing the accumulated changes to GitHub `main`.
This is user-reported runtime evidence, not an agent-observed test or exhaustive
acceptance of every flow. No individual catalog entry is promoted solely from
this general report; the unknown-outcome recovery gaps above remain open.

Publication excludes the local PostgreSQL/build directory, the generated client
executable and local client configuration changes. Source, tests, product
assets, documentation and research records are included. The root `build/`
directory is ignored without deleting its contents.
