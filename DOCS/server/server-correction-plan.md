# Server correction plan

Updated: 2026-11-01. Status: W0–W7 active in release `20261101T050356Z` (R1–R3); W8 and W9 wait for production measurements; G waits for client tests.
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
| W0 | Done | Real network load test on the VPS candidate |
| W1 | Done | Activation; COMMIT loss moves to W4 |
| W2 | Done | VPS measurement of pool size and hash slots |
| W3 | Done | Real TCP arrival, slow-reader and blackhole tests; client check of progressive scenes |
| W4 | Done in source | VPS COMMIT-loss and crash acceptance; shared objects recover by restart |
| W5 | Done in source | VPS resource measurements (P6); systemd restart-budget test |
| W6 | Done | Production confirmation of the movement cost on the VPS |
| W7 | Done in source | Slow-autosave matrix on the VPS; command-delay percentiles under load (acceptance) |
| W8 | Measured; source not started | Per-runtime and per-guild rows with a versioned migration (R4), justified by the VPS skip rate |
| W9 | Items 1 and P1 measurement done | Cross-process ownership, outbox and process supervision (items 2–8, R5) |
| G | Blocked | Needs the installed candidate and the owner's client tests |

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
