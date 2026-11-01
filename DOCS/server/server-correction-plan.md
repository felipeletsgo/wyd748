# Server correction plan

Updated: 2026-11-01. Status: W0–W7 active since release `20261101T050356Z` (R1–R3); W8 and W9 active since release `20261101T145133Z` (R4); one process per channel since release `20261101T165857Z` (R5); G is closed by the owner's client confirmation.
The [bug register](../bug-register.md#w6-activation-cost--source-2026-10-31) records W0–W6 evidence.

## Scope and ownership

Use the [bug register](../bug-register.md) as the only status queue.
This document defines execution order, dependencies and acceptance for every supplied audit ID.
It also defines the remaining work for BUG-001 through BUG-011.
Do not create defects for missing report numbers or reopen owner-accepted behavior.

Baseline: `main`, HEAD `e0ae4abc`, with existing uncommitted persistence and localization work.
Reuse current rollback, containment, admission and committed-account recovery coverage.
Check each work package against that source before implementing it; do not repeat delivered migrations.
The audit findings describe local source and probes, not a fresh VPS inspection.

Classify planned internal server changes as `MODERNIZACAO_COMPATIVEL`.
Keep existing classifications for the recorded gameplay and client changes.
Preserve packets, resource IDs, probabilities, progression and EN/PT channel selection.
Use the recipient's World for failure messages sent across channels.

The [critical persistence plan](critical-persistence-plan.md#current-gaps-and-acceptance-criteria)
owns the durable recovery contract. Update that contract when its implementation changes.
This document does not authorize deployment, restarts, client execution or security changes.

## Required behavior

1. Keep unrelated players, Worlds and new admission responsive after an operation-local persistence failure.
2. Cancel a proven uncommitted action and preserve its inputs; let the player retry.
3. Retain current RAM after a certified autosave rejection and schedule a fresh snapshot.
4. Never publish success, consume ingredients or grant rewards before the required transaction commits.
5. Reconcile ambiguous COMMIT results for their exact participants; never treat silence as rollback.
6. Recover account access from a verified result without requiring a server restart.
7. Preserve evidence for manual reimbursement; do not automate compensation for an unresolved transaction.
8. Keep every queue, retry, retained snapshot and diagnostic buffer bounded.

A database outage can prevent password validation or durable actions.
Return a temporary-unavailable response for those operations while the responsive game paths continue.
Do not convert database failure into invalid credentials or a global maintenance flag.
Manual reimbursement is an operational fallback, not proof that a late transaction cannot commit.

### Persistence outcome contract

| Outcome | Live-state action | Ownership and recovery |
| --- | --- | --- |
| Rejected before queue admission | Preserve the current state; report only the failed action | Retain no new reservation; retry from a new validated request |
| Certified rejection before COMMIT | Keep or restore the proposal's pre-image; publish no success | Settle once, release exact participant reservations and permit a fresh retry |
| Certified autosave rejection | Preserve the latest live state; keep it dirty | Release this save's ownership and schedule a fresh eligible snapshot |
| Confirmed COMMIT | Install and publish the committed result once | Validate operation, session, UID, lifecycle and aggregate versions |
| Confirmed COMMIT with failed publication | Do not replay the mutation or overwrite it with old RAM | Reuse existing account recovery; extend verified recovery to affected shared owners |
| Ambiguous COMMIT | Keep only involved participants reserved | Establish the exact durable result, reload state, then release the reservation once |

## Execution order

Each row is a reviewable work package. A dependency blocks that package, not all earlier corrections.
The listed gates are future requirements, not tests run during this planning task.

| Package | Scope | Depends on | Exit gate |
| --- | --- | --- | --- |
| W0 | P6 metrics, P8 and repeatable load/failure fixtures | Existing audit | Bounded diagnostics expose close causes, pool waits, critical outcomes and actor availability |
| W1 | D2 pre-COMMIT classification, D1 and rejection-continuation coverage | W0 | A certified failure cancels only its action; fresh admission works on both channels |
| W2 | D8, P3 and D9 | W0, W1 | Slow authentication and entry reads do not block either actor or claim invalid credentials |
| W3 | D3 output batching and visibility pressure | W0 | Dense arrivals and slow receivers preserve required packet order within measured memory limits |
| W4 | D2 ambiguous outcomes, durable recovery and D4 reconnect ownership | W1, W2 | COMMIT loss, crash/restart and repeated reconnect recover exact participants without duplicate writes |
| W5 | D5 health/recovery, D10, D11 and measured P6 limits | W1; W4 for recovery-dependent changes | Database restart, panic supervision and terminal drain have explicit, tested outcomes |
| W6 | P7 and P4 activation cost | W0 | Identical target/activation results and measured improvement across representative fixtures |
| W7 | P2 autosave latency and SQL work | W1, W4 | Newer state cannot be overwritten; routine snapshots no longer impose unnecessary input fences |
| W8 | P10 instance/guild ownership | W4, W7 | Unrelated rooms and guilds progress while another aggregate is saved or reconciled |
| W9 | P1 channel scheduling and final D5 failure isolation | W2, W4, W8 | Independent channel execution preserves shared ownership and contains a channel-process failure |
| G | BUG-001–BUG-011 diagnosis and gameplay acceptance | Relevant W1/W3/W4 stability gates | Exact installed candidate completes each recorded player scenario |

Deliver W0–W3 as small corrections before attempting W8 or W9.
Do not combine transport changes, schema partitioning and channel separation into one release candidate.

### Implementation status — 2026-11-01

R1–R3 (W0–W7) are active in release `20261101T050356Z` since 2026-11-01. Evidence is in the bug register.

| Package | Status | Remaining |
| --- | --- | --- |
| W0 | Done | Load under real players in production (local real network load passed 2026-11-01) |
| W1 | Done | Activation; COMMIT loss moves to W4 |
| W2 | Done | VPS measurement of pool size and hash slots |
| W3 | Done | None in source; local real-network and slow-link load passed 2026-11-01 |
| W4 | Done | Real-TCP COMMIT loss and process-crash acceptance passed locally 2026-11-01; closed-connection hotfix pending activation |
| W5 | Done in source | VPS resource measurements (P6); systemd restart-budget test |
| W6 | Done | Production confirmation of the movement cost on the VPS |
| W7 | Done in source | Slow-autosave matrix on the VPS; command-delay percentiles under load (acceptance) |
| W8 | Done: active in R4 (release `20261101T145133Z`, schema v15/v16); 600-player simulation without skipped instance ticks | None |
| W9 | Done: phases 1–5 active (R4, schema v17–v20); one process per channel since R5; VPS isolation check passed | None |
| G | Closed | The owner confirmed BUG-001–BUG-011 fixed in the client on 2026-11-01 |

W7–W9 change durable ordering, the schema or the process model. Start each as its own reviewed
release candidate after R1–R3 run on the VPS, as the release table below requires.
G needs the exact installed candidate and the owner's visual client tests; no source change is
justified before a concrete reproduction.

## W0 — diagnostics and reproducible acceptance

Affected paths: `internal/net/session.go`, `internal/game/metrics.go`,
`internal/control/runtime.go`, `internal/store/postgres.go` and the staff metrics consumer.
Paths in these work packages are relative to `wydgo748/`, unless stated otherwise.

1. Add a single first-cause close record shared by reader, writer, overflow and explicit disconnect paths.
2. Use bounded cause values: overflow, write timeout/error, read error, admission, persistence, shutdown and panic containment.
3. Expose output packets/bytes, high-water marks and oldest queued age without account-labelled metric series.
4. Expose pool acquired/idle/total connections, acquisition wait, cancellation and empty-acquire counts.
5. Expose critical queued/running/completed/quarantined work, outcome counts, deferred input and oldest pending age.
6. Add actor heartbeat and per-channel command/tick latency; retain existing staff metrics and authentication.
7. Keep operation IDs in bounded diagnostic records, separate from low-cardinality metric labels.

For P8, route routine hits, misses and pickup messages through summary or bounded diagnostic logging.
Keep security failures and transaction evidence distinct from routine action noise.
Persist required economy audit data with its transaction; do not rely on a best-effort asynchronous console queue.
Measure logger time and dropped routine diagnostics with a slow test sink.
Do not solve routine log volume by suppressing persistence or security failures.

Make load fixtures repeatable: fix World RNG, clock, map tie ordering, population and command trace.
Record hardware, Go build, configuration, seed, allocations and exact measured function.
Separate command work, movement, tick duration, total actor lag, TCP traffic and database time.
Retain the supplied probes as historical measurements; they are not a real network capacity limit.

Acceptance: one close cause per session, bounded logs under combat load and usable staff metrics without `debug_address`.
Run a real network load test with a disposable database before using burst counts as disconnect evidence.

W0 source status: items 1–6, P8 and the repeatable actor probes are implemented.
Item 7: `CRITICAL` lines carry completion IDs, and the staff quarantine list keeps each unresolved
operation's ID, accounts and last status, bounded by the quarantine set (W4 item 10).
The real network load test, VPS inspection and activation remain pending.

## W1 — certified rejection without disconnection

### D2: transaction failure before COMMIT

Affected paths: `internal/store/postgres_critical_transaction.go` and its certified adapters.

1. Track whether BEGIN succeeded, the callback completed and COMMIT was actually invoked.
2. Use a new bounded cleanup context for rollback; never reuse the expired operation context.
3. Dispose of an unusable connection before returning it to the pool.
4. Certify rejection only when the adapter can prove that the callback cannot commit independently or publish external effects.
5. Keep transport failure during COMMIT on the ambiguous-outcome path.
6. Preserve one total operation deadline and bounded serialization retries; avoid a new full deadline per attempt.

Acceptance: fail before BEGIN, during SQL and before COMMIT with a real PostgreSQL fixture.
Assert no COMMIT, no durable mutation, released locks, clean connection reuse and successful next operation.
Also cover rollback failure, exhausted retries and connection loss; callback-only mocks are not enough.

### D1: autosave rejection and remaining producers

Affected paths: `internal/game/critical_persistence_autosave.go`, owner settlement and producer rejection callbacks.

1. Add an explicit autosave rejection continuation with current account/session/UID checks.
2. Keep current RAM and dirty state; acknowledge only the rejected operation and release its exact reservations.
3. Retry a fresh snapshot on an eligible slice, with bounded backoff during repeated storage failure.
4. Preserve autosave spreading and avoid synchronized retry bursts after database recovery.
5. Audit every critical producer for missing certified-rejection handling; reuse the existing rollback tests.
6. Restore or retain ingredients, gold, inventory, position and shared objects according to each proposal's pre-image.
7. Keep success publication after COMMIT and rejection messages limited to the failed action.

Acceptance: repeat rejection/retry, duplicate and late completion, new live mutations, trade, disconnect and account reentry.
Assert `sessionClosed=false`, no global maintenance, released rejected-operation fences and no stale snapshot retry.
An unrelated account must authenticate, select a character and enter both channels during each injected failure.
Keep ambiguous COMMIT cases separate; they must not pass through the certified-rejection callback.

W1 source status: D2 pre-COMMIT cleanup and D1 for autosave and 15 detached producers are implemented.
Rollback failure before COMMIT counts as rejection; only a sent COMMIT stays ambiguous.

## W2 — login, entry and authentication capacity

### D8: independent read budgets and truthful responses

Affected paths: `cmd/server/main.go`, store configuration, `LoadAccount` and `onLoginResult`.

1. Separate account/read timeouts from critical transaction deadlines and startup/administrative budgets.
2. Include pool acquisition in each operation's budget; record queue wait separately from query time.
3. Propagate caller cancellation where appropriate; do not cancel an accepted durable mutation as an ordinary read.
4. Map invalid credentials, storage timeout and sanction-backend failure to their actual localized response.
5. Measure contention before changing the eight-connection template or worker limits.
6. Reserve usable capacity for reads and control traffic; do not let retries occupy every connection indefinitely.

Acceptance: delayed query, pool exhaustion, missing account, wrong password and sanction failure produce distinct expected outcomes.
Check PT on channel 1 and EN on channel 2, including a late worker result after disconnect.

### P3: remove remaining entry reads from the actor

Affected paths: `internal/game/handlers.go`, `charstate.go`, admission and worker-result commands.

1. Inventory the authoritative account reload, character-state read and admission-time name/storage reads.
2. Move these reads to bounded workers with per-session pending states and cancellation.
3. Reserve ownership before the authoritative reload; retain the fresh-state protection against pre-trade authentication snapshots.
4. Install results only on the owning actor after checking admission epoch, session, account, UID, runtime and sanction version.
5. Release ownership through one cleanup path for failure, disconnect, shutdown and stale completion.
6. Reject or defer duplicate entry requests without starting duplicate reads or consuming extra slots.

Acceptance: hold reads while commands/ticks and another login progress in both channels.
Race a successful trade/disconnect against authentication and prove the new session receives the latest durable inventory and gold.
Cover recycled actor IDs, changed sanctions, late results and shutdown during every entry phase.

### D9: control hash work without weakening passwords

Affected paths: `internal/game/operational.go`, authentication slot ownership and `internal/account/`.

1. Retain existing PBKDF2 strength, constant-result comparison and unknown-account enumeration protection.
2. Separate bounded hash concurrency from time spent waiting on account reads or sanctions.
3. Add a bounded, fair admission queue or an explicit retry response; use cancellable queue entries and a maximum wait.
4. Keep per-origin/account limits and add a measured process-wide CPU admission budget where required.
5. Tune concurrency against effective CPU quota and actor latency, not the developer machine's core count.
6. Return capacity promptly after disconnect; do not create one waiting goroutine per incoming request.

Acceptance: restart-style login bursts, valid users mixed with unknown accounts, origin-limit collisions and slow storage.
Measure completion rate, wait time, busy responses, CPU throttling and gameplay command latency.
Do not accept the report's ten-IP saturation estimate as a measured production attack.

W2 source status: D8, P3 and D9 are implemented. Pool size and hash concurrency stay unchanged until VPS measurement.

## W3 — D3 output pressure and visibility publication

Affected paths: `internal/net/session.go`, listener configuration, visibility and teleport publication.

1. Keep per-session output bounded by both packet count and retained bytes.
2. Batch adjacent packets in the writer with a bounded byte/time budget and correct partial-write handling.
3. Define packet ownership and finalization once; a failed enqueue must leave a safe retryable payload.
4. Return explicit enqueue status to visibility producers instead of silently closing on every temporary full queue.
5. Publish large scene transitions through a per-session cursor with a bounded actor work budget.
6. Distinguish desired entities from entities already published; retain create/update/remove ordering for each entity and scene epoch.
7. Cancel obsolete transitions on teleport, disconnect or runtime change, without losing required removal packets.
8. Preserve essential control ordering and bounded failure for an indefinitely stalled receiver.

Do not block the shared actor on network drain, discard required state or adopt an unbounded overflow buffer.
Use measured memory to select a queue increase, if needed; capacity alone is not the complete correction.

W3 source status: items 1, 2 and 5–8 are implemented with a recomputed view instead of a stored cursor.
Item 3 keeps fail-closed enqueue; item 4 uses a free-space check before optional creations.

Acceptance: real TCP arrival at 0/50/100/200 nearby players, dense mobs, repeated teleports, Hell Gate and guild-war traffic.
Add 300-player sustained traffic, slow reads, network blackholes, partial writes and consecutive scene changes.
Assert complete views, no ghost entities, correct order, bounded RSS and no overflow disconnect for supported healthy receivers.
Repeat with two channels and verify a slow session does not delay another player's commands.

## W4 — verified recovery and stale-session replacement

### D2: ambiguous COMMIT and restart-safe ownership

Affected paths: certified store adapters, persistence owner/containment, schema migration and admission recovery.

1. Give each logical mutation a stable operation ID; distinguish its transaction attempts and participant versions.
2. Record the terminal operation result atomically with the account/shared-state transaction.
3. Add durable pending intent and participant ownership sufficient to discover unresolved work after a process crash.
4. Define startup recovery before accepting writes for affected accounts or aggregates; keep unrelated admission available.
5. Reconcile on a worker using the exact operation record and, when available, the exact top-level transaction ID.
6. Treat missing records, `in progress`, unknown transaction status and unavailable storage as unresolved until the original attempt is excluded.
7. For a confirmed commit, reload durable participant state and publish only the allowed recovery result.
8. For a proven abort, cancel the proposal, reload where needed and release exact retained owners.
9. Make reconciliation idempotent; reject stale results after a newer operation, session or runtime owns the participant.
10. Expose recovery status and an audited staff retry action; provide no unconditional unlock or time-based release.

Reuse existing confirmed account-only continuation recovery rather than replaying its callback.
Extend recovery explicitly for multi-account, ground object, mob, guild and instance ownership.
Measure the additional database work before applying durable intent records to high-frequency snapshots.
Use backward-compatible schema expansion and a recovery-capable rollback binary.
Never drop recovery records to make an older binary appear usable.

Acceptance: real connection loss before, during and after COMMIT; late COMMIT and lost acknowledgement.
Crash after intent creation, after durable mutation, before publication and during recovery.
Repeat reconciliation and login concurrently; assert one reward/payment, no stale overwrite and no owner released prematurely.
Only affected participants may remain unavailable while the outcome is unresolved.

W4 source status: items 5–8 run on the worker through the exact transaction ID before delivery.
Startup now ends every backend of a previous game instance and waits for it before admission, so no
old COMMIT can land after accounts are read. Account-only quarantines are rechecked by exact transaction
and released only on a final status. Items 1–4 use the per-attempt XID, PostgreSQL's commit status and the
startup fence instead of an operation table; the
[durable-record decision](critical-persistence-plan.md#w4-durable-record-decision-2026-11-01) records why.
Item 9: a write of a shared object stays fenced until a restart reloads it. Item 10: staff with
`game.persistence.recovery` list quarantines and request an audited exact-outcome check; no action
unlocks without a final status.

### D4: detect failed transport and reclaim the exact session

Affected paths: TCP setup, session lifetime, account admission and disconnect settlement.

1. Add configurable Linux `TCP_USER_TIMEOUT` with platform-specific implementation and supported fallback diagnostics.
2. Tune keepalive and write deadlines from measured peer-failure behavior; test active and idle blackholes separately.
3. Measure the existing client heartbeat before adding any authenticated read-idle deadline.
4. Introduce a generation-checked replacement flow for an authenticated login whose previous transport is stale or closed.
5. Quiesce the old owner, close its transport, settle accepted writes and finish trade/instance cleanup before installing the new owner.
6. Await W4 recovery for uncertain writes; never clear ownership solely because `Session.IsClosed()` is true.
7. Preserve current healthy duplicate-session policy unless a separate replacement contract is deliberately approved.

Acceptance: cable loss, vanished peer, one-way blackhole, idle legitimate player, slow connection and rapid repeated login.
Verify bounded stale-session recovery, no simultaneous account owners and no duplicate saves, trades or rewards.

D4 source status: items 1, 2 and 7 are implemented. Items 4–6: a login whose account still belongs to a
closed transport waits up to five seconds for that owner's writes and cleanup, then takes the ordinary
claim; it never evicts the old owner and never waits on a quarantine. Item 3 (client heartbeat) needs a
client measurement. Real loopback TCP tests cover a slow reader and a reader that stops reading.

## W5 — health, service lifecycle and memory limits

### D5: truthful health and supervised recovery

Affected paths: `safeHandle`, `ChannelCluster.run`, `cmd/server/web_admin.go` and supervision configuration.

1. Publish per-channel actor heartbeat, admission state and persistence recovery state through atomic health snapshots.
2. Make readiness return unavailable for the affected channel when its actor stops or admission is closed.
3. Keep liveness distinct from database readiness; a database outage alone must not trigger a restart loop.
4. Catch handler/tick panic at the ownership boundary, record bounded stack evidence and mark potentially partial state unusable.
5. Prevent autosave and terminal persistence from writing poisoned RAM.
6. Rebuild only state whose durable ownership is verified; otherwise use supervised failure instead of continuing a corrupt actor.
7. Test supervisor restart budgets and backoff; never leave a dead actor indefinitely behind readiness 200.

The current shared process limits failure isolation. A safe restart may still disconnect both channels until W9 is delivered.
Do not claim that moving a channel to another goroutine isolates an unhandled Go panic.
Final closure requires the W9 one-channel-process failure test and shared-state recovery evidence.

Acceptance: inject handler and tick panic in an isolated test process, including a panic during result installation.
Check readiness, stack evidence, poisoned-save suppression, restart behavior and the unaffected channel's observed state.

W5 source status: items 1–7 are implemented. A contained panic requests one supervised restart
(exit 75) when the production option is on; the unit's start limit stops a restart loop.
D11 local measurement (PostgreSQL 18, Windows, populated accounts with 30 items): 50/150/300 final saves
took 358 ms/937 ms/1.6 s with one worker and 68/107/191 ms with four. VPS timing remains pending.

### D10: decouple explicit database restart

Affected path: `tools/deployment/vps/wyd-game.service` and effective VPS unit/drop-ins during later authorized deployment.

1. Prove W1/W4 behavior across PostgreSQL stop, restart and connection recovery.
2. Change the proposed startup dependency to `Wants=` plus `After=` once runtime recovery is accepted.
3. Keep database-dependent readiness false during the outage while responsive gameplay paths and diagnostics remain active.
4. Check pgx pool recovery, retry backoff and fresh authentication after PostgreSQL returns.

Acceptance: restart PostgreSQL on the disposable service fixture without stopping the game process.
Verify no global maintenance, no invalid-credentials response for infrastructure failure and no stale-result publication.
Inspect the actual unit before claiming production behavior; the repository template is not effective-state evidence.

### D11: bounded final persistence with useful progress

Affected paths: `internal/game/shutdown.go`, cluster shutdown, store flush and `cmd/server/main.go`.

1. Close admission across channels and settle accepted critical operations before terminal snapshots.
2. Capture detached final snapshots on the actor; send independent account saves to a bounded drain worker pool.
3. Preserve per-account and shared-aggregate ordering; exclude poisoned or unresolved state from ordinary final writes.
4. Use one measured shutdown budget and report pending, completed, failed and unresolved work.
5. Define the application budget below the service stop deadline, leaving cleanup and reporting margin.
6. Report incomplete shutdown with failure status; a timeout must not certify that remaining players were saved.

Acceptance: representative 50/150/300-account snapshots, populated inventory, slow SQL, held critical writes and a failed save.
Measure total drain duration before selecting budgets or concurrency; the old three-second cadence is not a loss bound.
Verify repeated shutdown calls, no stale final overwrite and recovery of unresolved work after termination.

### P6: memory, tasks and effective resource configuration

1. Record effective service settings, environment and cgroup CPU/memory limits during later authorized VPS inspection.
2. Measure peak RSS, Go-managed memory, stacks, thread count, output bytes and allocation rate under the supported load.
3. Derive `GOMEMLIMIT` below `MemoryMax`, reserving measured non-Go memory and transient allocation headroom.
4. Derive `TasksMax` from operating-system thread counts, including runtime/cgo needs; goroutine count is not thread count.
5. Review CPU quota after actor, hash and allocation profiles; do not assume raising quota fixes serialized work.
6. Validate GC latency, throttling and process limits in a representative Linux cgroup.

Acceptance: no resource-limit termination or unbounded growth during the declared steady and burst loads.
Retain the old limits as unverified template choices until measurements justify replacement values.

## W6 — P7/P4 activation work

Affected paths: `internal/game/spatial.go`, target validity, runtime lookup and movement/teleport callers.

1. Build a result oracle for current target eligibility, activation radius and public/private runtime membership.
2. Put an explicit nil guard and cheap distance rejection before expensive validity/runtime checks where semantics permit.
3. Preserve score validation and runtime-index repair invariants; do not copy the audit prototype without these checks.
4. Cover death, hidden/expired affects, same-cell movement, cell crossings, reconnect and instance transitions.
5. Profile remaining recomputation; add cached eligibility or narrower invalidation only when exact invalidation is defined and measured.
6. Preserve immediate same-cell wake/sleep behavior; do not defer activation merely to obtain a lower movement benchmark.

Acceptance: compare active-mob and target results for the same deterministic traces before and after the change.
Run at least five paired timing samples across sparse/dense players, 600 mobs, stops and teleports.
Report uncertainty and regressions; the prior compact benchmark did not improve with predicate reordering.
Measure full actor load as well as the isolated function; avoid timing comparisons under the race detector.

W6 source status: items 1–6 are implemented. Item 5 needs no cache: each recompute pass reads the player
cells around the moved cell once and memoizes eligibility for that pass only, and each mob still checks
only its own cells. An oracle (20 seeds, in-window and stale positions, full-recompute comparison) matches
the cell scan. Three alternating probe runs at 300 players: movement 112–114 µs before, 59–60 µs after
(about −48%); tick p50 unchanged. Local Windows timings only.

## W7 — P2 autosave without unnecessary input stalls

Affected paths: autosave scheduler, account snapshots, worker sequencing, PostgreSQL projections and item ledger.

1. Measure snapshot CPU, SQL counts, unchanged rows, pool wait and deferred-command duration with populated accounts.
2. Track live generation, submitted generation and committed generation per account and character sidecar.
3. Keep one bounded replaceable pending background snapshot per account; coalesce only superseded unsent work.
4. Serialize actual writes by account revision so an old snapshot cannot commit over a newer critical mutation.
5. Let nonconflicting live work continue while an immutable background snapshot is written; retain critical-action ownership where required.
6. Clear dirty state only for the acknowledged generation; later mutations remain dirty and require another save.
7. Reduce measured unchanged SQL/projection work while preserving atomic account, character and item-ledger consistency.
8. Retain W1 rejection behavior and W4 recovery for ambiguous results; never turn an older snapshot into an automatic retry.

Acceptance: slow autosave concurrent with movement, potion, trade, reward, teleport, disconnect and final shutdown.
Verify latest durable gold/items/position, no duplicate ledger entries, bounded snapshots and reduced command-delay percentiles.
Do not use the unverified 20–30 SQL estimate as a target or assume all unchanged items are currently rewritten.

W7 source status: item 1 measured locally, item 7 implemented. A JSON fingerprint on the actor cost
757 µs per populated account and a DeepEqual comparison 129 µs, so the decision moved to the worker.
A player-state batch identical to what this process last committed skips its transaction; any other
write of the account or sidecar clears the mark first, and at most 19 skips occur in a row.
Items 2–6: the autosave is a background snapshot that holds no gameplay input. A later write of the
same participant parks in the coordinator and starts only after the snapshot is acknowledged, so
writes of one account commit in submission order; one background snapshot per account is pending
at most. A failed snapshot refuses its parked writes before they reach the store. Item 8 keeps W1
rejection and W4 recovery; no snapshot is ever retried automatically.

## W8 — P10 finer instance and guild ownership

Affected paths: instance/guild persistence adapters, shared reservations, runtime lifecycle and schema migration.

1. Define the smallest durable participants: channel/runtime instance, guild, global war/city state and affected accounts.
2. Introduce versioned per-instance and per-guild storage with explicit ownership; retain separate global records where the contract requires them.
3. Replace whole-aggregate reservations with participant keys and deterministic lock ordering for multi-owner transactions.
4. Keep account plus reward/member/treasury mutations atomic; do not split a payment from its reward or membership change.
5. Tick unrelated runtimes during a save. Queue affected lifecycle intentions while reserved objects remain protected.
6. Preserve absolute deadlines and retry teardown only after held kill/reward continuations settle.
7. Migrate singleton payloads through an explicit version/cutover procedure; validate row counts, references and equivalent reconstructed snapshots.
8. Keep an old-format rollback path only while it can represent every new write; otherwise require a recovery-capable compatible binary.

Acceptance: concurrent unrelated rooms/guilds across both channels, same-guild contention and deterministic multi-guild locking.
Hold a reward write across expiry, disconnect and teardown; prove no lost reward, premature removal or deadline reset.
Inject rejection and ambiguous COMMIT; recovery of one runtime must not freeze all instance lifecycle work.

W8 source status: not started. Running skipped lifecycle work inside completion delivery broke the
instance-entry ordering contract and was reverted; per-runtime and per-guild rows need the migration.
Measurement (2026-11-01, local PostgreSQL): a two-channel aggregate of 100 runtimes is 76 KB and writes in
2.6 ms p50 (3.3 ms max), so a reservation lasts milliseconds. `world_instance_tick_skipped_total` divided
by the tick count gives the production share of skipped instance ticks; use it to size R4.

#### W8 design (2026-11-01)

The owner chose to deliver W8 and W9 for failure isolation, after the R1–R3 validations.
The participant map is: 28 instance reservation checks in 21 files, 22 guild mutation checks,
19 guild registry checks, 13 war-mob checks and 8 producers that write instances.

1. Participants. An instance participant is `(channel, runtime ID)`. Each channel also has one
   globals participant: Nightmare party-run counters and the Gamble jackpot and pool. A guild
   participant is the guild ID; each channel's war state, ID allocation and name index form one
   registry participant. Accounts keep their existing keys.
2. Storage (schema v7). `instance_runtime_state(channel, runtime_id)`,
   `instance_channel_state(channel)`, `guild_rows(guild_id)` and `guild_registry_state(singleton)`
   each hold a versioned JSON payload. A write commits only its declared rows; the store
   rejects a write whose rows are not declared.
3. Migration. On the first start of a v7 binary, one transaction splits the singleton rows into
   participant rows. It then rebuilds the aggregates from those rows, requires them to equal the
   originals, and records version 7. The singleton rows stay unchanged as the rollback source.
4. Rollback. A v7 binary keeps the singleton payloads current through a background export that
   runs after each committed participant write. A v6 binary therefore reads at most one export
   interval of staleness, in game-recoverable lifecycle state only, never account economy.
   `update-release.sh` exports before it starts an older binary.
5. Reservations. The queue holds per-participant keys instead of the instance and guild
   holders. Multi-owner writes take keys in sorted order. Checks with a runtime, mob, player or
   guild in scope narrow to that key; the remaining whole-aggregate checks reserve every key.
6. Retained checks compare only the declared participants instead of the whole aggregate.
7. Ticks skip only reserved runtimes and wars; the rest progress during a write.

Deliver W8 in three reviewed steps, each with its own validation: instance storage and
reservations, then guild storage and reservations, then the migration release candidate (R4).

W8 source status (2026-11-01): steps 1 and 2 are implemented.
- Storage: schema v15 adds `instance_runtime_state` and `instance_channel_state`; schema v16 adds
  `guild_rows` and `guild_registry_state`. Each migration runs in one transaction, rebuilds the
  aggregate from the new rows and refuses to finish unless it equals the original. The old
  singleton rows stay and are exported after every committed write; the public guild view and
  an older binary read them. `update-release.sh` removes the v16 and v15 markers before starting a
  v14 binary.
- Instances: transitions, departures, rewards and room-exit writes declare the runtimes and
  globals that differ between their detached before and after states. Reservations, retained
  checks and installs cover only that scope. Rooms tick unless their own runtime is reserved;
  dirt is tracked per runtime and flushed as keyed writes. Entry stays whole-aggregate because
  it reads exclusive groups and other rooms; Gamble and Nightmare kill rewards also stay whole.
- Guilds: calendar changes, membership changes and joins declare their changed guilds and, when
  needed, the registry header. A guild referenced by a changed alliance or war joins the scope.
  Guild creation, treasury collection, kingdom capes and Kefra kills stay whole-registry.
  Commands of a player whose guild and the registry are free proceed during other guilds' writes.
- Evidence: on PostgreSQL, one runtime committed while another runtime's row was locked; keyed
  rows leave undeclared rows untouched; eight runtimes wrote concurrently without conflict;
  both migrations rebuilt equal aggregates and refused a second run.

## W9 — P1 channel execution and final panic isolation

Target: separately supervised channel processes with one gameplay actor per process.
This is a backend coordination change, not a native-client protocol change.
It provides CPU scheduling separation and a process boundary for fatal channel failures.
Do not close P1 by only increasing CPU quota or launching concurrent Worlds against shared Go maps.

1. Inventory every shared account, character-name, guild, party, transfer, staff and event owner before the split.
2. Replace shared map pointers with explicit versioned coordination over the existing authoritative database and durable operation contract.
3. Give account ownership a fenced generation; reject stale process/session writes and recover expired owners through W4.
4. Deliver cross-channel events through an idempotent transactional outbox; treat database notifications only as wake hints.
5. Keep runtime entities channel-local and preserve recipient-world locale selection for every cross-channel notification.
6. Run each channel's listener, loop, health and worker budgets independently; allocate CPU/memory for their combined VPS footprint.
7. Preserve a single-process compatibility mode only while its shared contracts remain equivalent.
8. Cut over one tested candidate configuration at a time during an authorized release, with active shared-owner recovery available.

Acceptance: concurrent login for one account, channel transfer, cross-channel guild operations and globally unique names.
Test both citizenship/language directions, shared events, reconnect, repeated outbox delivery and coordinator/database loss.
Saturate one channel and measure the other's admission and command latency.
Terminate or panic one channel process and prove the other keeps its existing sessions and new admission.
This package cannot close until missing shared-ownership contracts are implemented and tested.

P1 measurement (2026-11-01, local): one channel's full cycle, which the other channel's commands wait
behind on the shared goroutine, is 2/6/12 ms p50 and 7/11/22 ms p99 at 50/150/300 active players. Latency
alone does not require the split at that scale; fatal-failure isolation (D5) still does.

#### W9 delivery phases (2026-11-01)

The owner chose W9 for availability: a fatal failure of one channel must not disconnect the other.
A Go fatal error (concurrent map write, out of memory) ends the whole process, so only separate
processes isolate it. Each phase is a reviewed release; the single-process mode stays the default
until phase 5 passes.

1. Account ownership in PostgreSQL. A lease row per signed-in account names the channel and
   process generation; login claims it in one statement, logout and settlement release it, and a
   process start ends the leases of its previous generation. Login stays single across processes.
2. Durable channel tickets. Transfers write a single-use ticket with an expiry; the target channel
   consumes it in the same transaction that claims the account lease.
3. Shared guild state. Keyed guild writes (W8) carry the row versions they read; a stale version is
   a certified rejection. Each process refreshes changed guild rows from a change notification and
   on version conflicts.
4. Cross-channel messages. A transactional outbox carries global chat, whispers, guild chat,
   sanction application and staff kick or teleport to the owning channel; delivery is idempotent
   and notifications are only wake-up hints.
5. Process split. One systemd unit per channel with its own listener, actor, coordinator and
   resource budget. Acceptance: one account cannot sign in on both channels, transfers, guild
   operations across channels, both languages, and ending one channel process keeps the other
   channel's sessions and new connections.

#### W9 phase 1 status — account leases (2026-11-01, source only)

Schema v17 adds `game_processes(process, started_at, heartbeat_at)` and
`account_leases(account_key, process, channel, session, claimed_at)`. Older binaries ignore both
tables; the rollback script resets the v17 marker.

- Login: the entry-read worker claims the lease before it reloads the account. A lease of another
  live process refuses the login with the native "already playing" reply. A failed claim asks the
  player to retry. A lease of a process without a heartbeat for 30 s is taken over.
- Release: the lease ends with the account claim, but only after the account's critical writes
  settle. A quarantined account keeps its lease until verified recovery. A refused or cancelled
  admission ends the lease that its worker claimed.
- Liveness: each process renews its heartbeat every 10 s. Each minute the first channel removes
  this process's leases for accounts that it no longer holds and that are older than 30 s.
- Restart: a process that fenced the previous game instances (W4) removes every other process and
  its leases, so reconnecting players are not refused. A complete shutdown releases its leases; an
  incomplete drain keeps them until they are stale.
- Write fence: in a registered game process, every account write locks the accounts' lease rows
  in its transaction. A lease of another live process rejects the write with a certified rejection
  (W1 then closes the participant). The shared lock makes a takeover wait for the write, so the new
  owner reads the committed state. Tools, panels and the player API do not register and are not fenced.
- Metrics: `world_account_lease_total` (`claimed`, `claim_refused`, `claim_failed`,
  `release_failed`, `reconcile_failed`, `heartbeat_failed`) and `postgres_account_lease_fenced_total`.

With one process the behavior is unchanged: the in-memory claim still decides first.

Validation: store tests on PostgreSQL (single owner across processes, stale-session release,
takeover of a stopped process, one winner of four concurrent claims, reconcile, exclusive restart,
write fence by a live owner and not by a stopped one)
and game tests (refusal, claim failure, release after settled writes, cancelled reload). A two-process
PostgreSQL test refuses the account on process B while process A holds it, and admits it after A ends
the session.

Pending before phase 5:

- Offline writes: an account without a lease is written without a fence. Before two processes
  can write, a process must lease an offline account it changes, or route the change to its owner
  (phase 4).
- Replacement across processes: a login on another process is refused, not replaced. Phase 4 delivers
  the kick message that a replacement needs.
- The W4 startup fence and the exclusive registration end every other game process. Phase 5 must
  scope both to the process's channels.

#### W9 phase 2 status — durable channel tickets (2026-11-01, source only)

Schema v18 adds `channel_tickets(token_hash, account_key, channel, slot, expires_at, consumed_by,
consumed_session)` with one pending ticket per account. Only the SHA-256 hash of the token is stored.
The cluster mode keeps its in-memory tickets; durable tickets apply to a World without a cluster that
names its other channels with `WithRemoteChannels` (phase 5 configuration).

- Source: the actor makes the token; a worker stores the ticket. The actor then rechecks the player
  and submits the final account and character write. The client receives the token only after that
  write commits, so the target cannot read the account earlier. A player who leaves during the issue,
  a rejected final write or a closed source cancels the ticket; an uncancelled ticket expires in 30 s.
- Target: the login worker checks the ticket instead of the password. The lease claim consumes the
  ticket in the same transaction and takes the lease from the source even while the source is alive.
  The player enters the ticket's character slot. A replayed, expired or other-channel ticket is refused
  with the native "transfer expired" panel.
- While a ticket waits, a plain login of the account is refused with "try again shortly", and a second
  transfer of the player reports "busy".
- Metrics: `world_account_lease_total` adds `ticket_claimed` and `ticket_refused`.

Validation: store tests on PostgreSQL (lease move with a live source, replay, other account and
channel, expiry, cancel, one consumer of four) and game tests (cancel when the player leaves or the
final write fails, busy on a pending ticket). A two-process PostgreSQL test moves a player from
channel 1 to channel 2 with the committed gold, refuses a plain login during the transfer, refuses
the replayed token and keeps the target's lease after the source's late release.

#### W9 phase 3 status — versioned guild rows (2026-11-01, source only)

No schema change: the W8 guild rows already hold a version. `WithGuildRowVersions` enables the
contract for a World without a cluster (phase 5 configuration); the cluster mode keeps one registry
in one process and does not use it.

- Boot: the World reads every guild row with its version and the registry header with its version
  in the same rows, so no version is newer than its content.
- Write: each guild write names its rows with the version that this process read: a positive version
  must match, -1 expects no row (a new guild, or a guild absent on both sides), and 0 leaves the header
  unchecked when it was never read. A whole-registry write names every known guild. A changed row is
  a certified rejection (`ErrGuildRowConflict`); the producer's rejection path runs as for any other
  rejection. A commit advances the local versions.
- Notification: with `EnableGuildRowSync`, a committed guild write sends `wyd_guild_rows` with the
  process name and the guild IDs. `ListenGuildRows` ignores its own process and reports the IDs; after
  a lost connection it reconnects and reloads every row.
- Reload: a notification or a rejected write reads the rows off the actor. A newer row replaces the
  local guild, a removed row removes it, and a row with a write in flight is reloaded after that write.
- Metrics: `world_guild_sync_total` (`conflicts`, `installed`, `removed`).

Validation: store tests on PostgreSQL (create once, stale update, stale and current removal, absent
copy of a created guild, row states with versions, notification to another process and not to the
writer) and a two-process PostgreSQL test: process A commits a change that reaches process B, a write
of B from its stale copy is rejected without changing the row, B reloads the row, and B's retry
commits and reaches A.

Pending before phase 5:

- Guild name uniqueness and the display of connected members are resolved in phase 5.

#### W9 phase 4 status — channel messages (2026-11-01, source only)

Schema v19 adds `channel_messages(id, source, target, kind, payload, created_at)`. `WithChannelOutbox`
enables it for a World without a cluster (phase 5 configuration); without it nothing is sent.

- Send: the actor queues a message without waiting (capacity 1,024); one worker stores batches of up
  to 100 in one transaction and sends `wyd_channel_messages` as a wake-up hint. A full queue or a
  failed store drops the batch and counts it.
- Receive: one reader per process starts at the database time of its start, reads on each hint and
  every second, and delivers each message ID once. It also reads the last 10 s again, so a message that
  commits after a newer one still arrives. Messages older than ten minutes are removed.
- Kinds: global chat (`=`), citizenship chat (`@@`), guild chat (`-`), whisper and letter (`!`) to a
  character that is not on this channel, staff broadcast notices, and sanction changes. The receiver of
  a sanction change reads the stored sanction state and applies it to its players; a lost message is
  repaired at the next login, which reads the state too.
- Metric: `world_channel_outbox_total` (`queued`, `published`, `delivered`, `dropped_full`,
  `dropped_store`, `invalid`, `unknown_kind`).

Validation: a store test on PostgreSQL (target channel filter, own messages skipped, late-commit window,
new reader start, cleanup) and a two-process PostgreSQL test: a whisper, a guild line, a global line and a
staff notice from channel 1 reach the right players of channel 2 once, and a player of another guild
receives no private line.

Staff overview, presence, kick and teleport, and the whisper offline notice, use requests with replies
on the same outbox (phase 5 status).

#### W9 phase 5 status — channel processes (2026-11-01, source and local acceptance)

- Configuration: `channel_peers` (for example `2` in the channel 1 file) makes the process serve only
  `channel_id`. It excludes `channel2_enabled`. The process then enables account leases, durable
  tickets for the peers, versioned guild rows with notifications, and the channel outbox.
- Fence: a channel process is named `wydgo-game/ch<N>/<id>`. Its W4 startup fence and its exclusive
  registration end only the previous process of its channel and a previous whole-server process; a
  whole-server process still ends every game process.
- Names: character names were already unique in `character_names`. Schema v20 makes guild names unique
  in `guild_rows` with the same rule as the server (trimmed ASCII, case-insensitive).
- Service: `tools/deployment/vps/wyd-game@.service` runs channel `%i` from `/etc/wyd-go/server-ch%i.txt`
  with 800 MB and one CPU each. The release scripts do not install it.
- Staff: the panel process adds the other processes' players to its overview and account presence,
  through requests on the channel outbox with a 3 s reply bound. A kick or teleport whose session is not
  on the panel's channel goes to the other processes; the process that holds the session checks UID,
  account and session, runs the command, keeps the receipt and returns the result.
- Whisper: a whisper to a character that is not on this channel is offered to the other processes; when
  none delivers it, the sender receives the native "not connected" notice.
- Guilds: invitation, expulsion and promotion act only on members connected to the same channel, as
  with one native TMSrv per channel; a member on another channel gets the existing "not connected" or
  "online members only" notice, and no process writes an account it does not hold. A reloaded guild
  repairs the guild fields of connected members and sends their new guild identity.

Local acceptance (2026-11-01, `wydgo_test`): two processes with 10 bots each. Killing the channel 1
process left channel 2 with its 10 sessions, and channel 2 admitted 5 new bots during the outage.
The restarted channel 1 readmitted its bots in 3 s and refused 2 logins of accounts held by channel 2.
Store tests also show that a restarted channel 1 ends only channel 1's old backends and leases.

Not done (cutover requirements):

- Publication and cutover are not authorized. The cutover needs a release with schema v15–v20, two
  configuration files, the unit template, a proxy route from the staff panel to one channel process,
  and an authorized restart that stops `wyd-game.service` first.
- VPS measurements of two processes (memory, CPU, database connections) are pending.

#### W9 item 1 — shared ownership inventory (2026-10-31)

`ChannelCluster.attach` shares these maps between both Worlds on one goroutine. Each needs a
cross-process owner before the split:

| Shared state | Current owner | Needed contract |
| --- | --- | --- |
| `accountSessions` | One map, one actor | Fenced account ownership generation in PostgreSQL |
| `charNames` | One in-memory index | Unique name constraint plus a reserved-name read on creation |
| `guilds` registry | One live registry, singleton row | Per-guild rows (W8) and versioned writes |
| `authSlots`, `hashSlots`, auth and chat rate maps | Process memory | Per-process limits, or a shared limiter if global limits must hold |
| `staffOps` bans, mutes, sanction versions | Process memory, loaded from PostgreSQL | Sanction version in PostgreSQL plus a change notification |
| Channel tickets and transfers | `ChannelCluster` maps | Durable single-use tickets with expiry |
| Instance snapshot | `ChannelCluster.instances`, singleton row | Per-runtime rows (W8) |
| Critical persistence owner and queue | One coordinator | One coordinator per process over the same fences in PostgreSQL |

Cross-world calls that become messages: guild identity and member delivery (`guild.go`), global
chat and whisper audiences (`commands.go`), sanction application on every channel (`staff_ops.go`),
staff teleport and kick by owner (`teleport_control.go`, `kick_control.go`), NPC definition polling
(`npc_definitions.go`), containment across channels (`critical_persistence_containment.go`) and
shutdown of every channel (`shutdown.go`). `playerWorld` and `globalPlayers` currently scan both
Worlds directly; each caller needs a delivery path through the transactional outbox (item 4).

## G — existing gameplay reports

Use the exact scenario and evidence in the [bug register](../bug-register.md#reports-and-acceptance-checks).
Reconcile the candidate, installed binary, published patch and active server before changing already-correct source.
The table defines remaining work; it does not certify any new gameplay result.

| ID | Remaining work | Closure evidence |
| --- | --- | --- |
| BUG-001 | Capture one concrete missing HIT/MISS result across server packet, target lookup and floating-text rendering; fix only the reproduced boundary | Attacker, defender and nearby observer receive correct results without duplicate feedback or changed damage |
| BUG-002 | Test the exact gate candidate, distant movement click, view exit/reentry, relogin and legitimate war close; investigate only a remaining mismatch | Gate state, rendering and collision agree with authoritative open/closed state |
| BUG-003 | Test a new active-version Hell Gate run: entry, selected route/Liches, portals, defenders, Dionys, shop, reconnect, expiry and reward rejection/retry | The complete encounter succeeds/fails under its configured rules, stays reachable and cannot duplicate rewards |
| BUG-004 | Keep owner-accepted Mana Control behavior closed; reopen only for a new concrete HP/MP counterexample | No speculative damage rebalance |
| BUG-005 | Keep owner-accepted casting/absorption behavior closed; reopen only for a named new resource sequence | No speculative mana-cost change |
| BUG-006 | Reconcile candidate 16 with the effect trace; verify local preview, scene-owned billboard lifetime and assets before another correction | Caster and observers see the complete effect once, with one authoritative damage/resource application |
| BUG-007 | Test initial and later group entry in Nightmare N/M/A, plus solo tickets while socially grouped | Immediate two-way visibility and party identification without movement or a periodic refresh |
| BUG-008 | Reconnect leader/member with changed, full, dissolved and expired parties/runtimes | Restore current eligible party/HUD/protector state without resetting the deadline or consuming a new ticket |
| BUG-009 | Complete every Magic Chamber tier/room, including concurrent runs at identical coordinates and final-room reconnect | No ghost/missing targets, cross-runtime interaction, stalled completion or leaked rewards |
| BUG-010 | Reconcile the summary with the detailed native floor-effect evidence; confirm synthetic-generator removal and identify the actual random trigger/assets | Only the proven native effect appears at the correct location and lifecycle; do not invent random timing |
| BUG-011 | Approach Compounder and each affected guard with two players; introduce an eligible hostile monster and repeat after reconnect | Passive Compounder, monster-only guard combat, no player damage or guard-kill rewards |

For BUG-010, explicit supplied index/data findings do not prove an unidentified native floor effect.
If native behavior remains unknown, keep that acceptance item open and identify the next smallest static query.
Follow the native research skills only when that work starts; do not rerun the entire corpus.

Client-source corrections require the current client skill, focused tests and an affected-target build with `-NoDeploy`.
Published client changes require a new mandatory revision, coordinated server minimum and patch verification before server activation.
Client installation, execution and publication remain separate authorized stages.

## Validation and delivery gates

### Per work package

1. Add a regression that reproduces the actual reported boundary; preserve the existing dirty work.
2. Run the affected flow, packages and consumers once after the change stabilizes.
3. For persistence/concurrency, cover rejection, rollback, repetition, late results, disconnect, database behavior and race checks.
4. Run full `go test ./...` and `go vet ./...` for shared integration after the combined candidate stabilizes.
5. Run real network and PostgreSQL fault tests for their affected packages; do not substitute mock success.
6. Record `STATICALLY VERIFIED`, `AUTOMATED TESTED` and `CLIENT-TESTED` separately in the bug register.

Keep generated logs, fixtures and profiles in ignored root `build/<task>/`.
Use a disposable database and controlled connection-fault proxy for destructive failure tests.
Collect dependency vulnerability results separately; a vulnerability scan does not reproduce these defects.

### Candidate acceptance matrix

| Scenario | Required result |
| --- | --- |
| One certified account-write failure | Cancel only that action; session remains usable and another account enters both channels |
| Database delay/outage | Responsive actor, bounded workers/retries, truthful temporary login response and no global maintenance flag |
| COMMIT acknowledgement loss | Exact outcome recovery without duplicate effects or unconditional account unlock |
| Dense teleport plus slow reader | Complete ordered scene, bounded output memory, healthy sessions remain connected |
| Stale old socket plus repeated new login | One current account owner after verified settlement |
| Instance/guild save held across expiry | Unrelated owners progress; reserved objects remain until safe settlement |
| Handler/tick panic | Truthful health, no poisoned save and supervised recovery; final W9 isolation preserves the other channel |
| Shutdown under supported peak load | Drain within measured limits, or explicit incomplete result with recoverable pending identities |
| PT/EN cross-channel delivery | Correct recipient language, valid placeholders and unchanged wire fields |

Set numerical latency, memory and stale-session SLOs after the repeatable W0 baseline.
Record baseline, candidate target, hardware/quota and acceptance workload before evaluating improvement.
Do not invent VPS latency guarantees from local p99 tick measurements.

### Release candidates

| Candidate | Contents | Additional prerequisite |
| --- | --- | --- |
| R1 | W0, W1 and W2, with verified isolated corrections from W3 | Full shared-integration checks and real certified-rejection tests |
| R2 | Complete W3/W4 and recovery-dependent W5 changes | TCP/COMMIT loss, restart-safe recovery and terminal-drain acceptance |
| R3 | W6/W7 | Paired performance evidence and stale-snapshot regression matrix |
| R4 | W8 | Versioned migration/cutover and compatible recovery/rollback |
| R5 | W9 | Cross-channel coordination, independent-process fault and combined-resource acceptance |
| Client/gameplay | Only required remaining G corrections | Exact installed candidate, mandatory patch contract and gameplay acceptance |

Build a concrete candidate and recovery/rollback procedure before requesting any release activation.
Use the release skill for later SSH, backup, publication and restart work.
Preserve runtime data/configuration and verify effective units, active hashes, schema, both channels and restart counts after activation.
An uploaded archive or readiness 200 alone does not certify successful gameplay admission.

This planning task changes documentation only. No product test, service inspection or gameplay execution is claimed by it.
