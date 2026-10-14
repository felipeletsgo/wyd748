# Nonblocking critical persistence

This plan tracks persistence design and recovery acceptance, not deployment
identity. The [current-state record](../current-state.md) separates the live
VPS release from later local changes. S2 is enabled in production; old staging-only
notes must not be used as current activation status.

Status as of 2026-10-07: the producer migration is implemented, enabled for
PostgreSQL, and published in
[`83a94da7`](https://github.com/felipeletsgo/wyd748/commit/83a94da7c531ebdebe2ccc2faedbc35b3bbffd0a).
`cmd/server` passes `WithCriticalPersistence` and
`WithCriticalAccountContainment`. Gameplay commands and ticks no longer write
inline on the World goroutine; see [Activation](#activation-2026-10-06).
The October 7 corrections passed the full Go race suite with the isolated
PostgreSQL fixture. Migration and containment are delivered; verified recovery
of unknown outcomes remains open. The October 6 closure claim does not certify
those recovery requirements. Tracks S2 in the
[source audit](../../.agents/research/source-audit-2026-10-03.md).
Classification: `MODERNIZACAO_COMPATIVEL`; preserve existing packet formats,
authoritative validation, durable transactions and save-before-confirm ordering.
No new native research is required for this internal scheduling boundary.

## Current gaps and acceptance criteria

This is the current recovery queue, not a request to repeat completed producer
migrations. Detailed regression evidence is in the
[October 7 audit](s2-closure-audit-2026-10-07.md#follow-up-correction-account-timing-and-instance-readers).
Operational containment and configuration are documented in
[server operations](operations.md#critical-persistence-operation).

| Gap | Current behavior | Required evidence before closure |
| --- | --- | --- |
| Durable operation identity and outcome reconciliation | Completion IDs and fences are process-local; an uncertain write is not replayed or released | A durable identity/outcome protocol that distinguishes committed, proven rejected and unresolved work; prove repeated reconciliation cannot duplicate rewards or overwrite a later commit |
| Restart-safe containment | Unknown account outcomes retain fences indefinitely in the running process; shared outcomes stop the World. No persisted quarantine/recovery protocol exists | Crash/restart tests with unresolved and late commits, including account, UID and shared-object ownership; prevent new mutations until reconciliation is proven |
| Verified account unlock and bounded recovery | A confirmed commit whose account-local continuation fails is recovered automatically ([October 12](#committed-continuation-recovery-2026-10-12)). Truly unknown outcomes still have no supported unlock command or timer, retain queue capacity and can eventually exhaust admission | An identity-checked, auditable recovery/unlock workflow, tested for concurrent login, repetition and saturation; never reclaim capacity merely because time passed |
| Real transport failure acceptance | The late-commit regression uses an injected adapter outcome followed by a real PostgreSQL write | Reproducible database connection-loss/COMMIT fault injection and crash recovery, checking durable state, publication, fences and terminal-save suppression |

### Committed continuation recovery (2026-10-12)

Incident: after a client disconnect the character stayed online and the
account could not log in again until the server restarted. Cause: when the
adapter confirmed the transaction but the account-local continuation failed
(stale binding, panic, partial apply, closed transport), the World treated
the confirmed commit as an unknown outcome and fenced the account forever.

`recoverCommittedAccountFailure` (`critical_persistence_containment.go`)
now separates that case. It runs in the same actor delivery, before another
command can claim the account, and only when the adapter reported success,
the operation is account-only (no shared World, instance or guild owner), and
every participant still matches its recorded session, account, lifecycle and
character binding. It acknowledges the queue entry, poisons and disconnects
the participants without replaying the callback or saving a stale final
snapshot, and releases their admission slots. Login then reloads durable
state. Ownership is still retained until serialized instance detachment and
trade cleanup are acknowledged. Adapter errors, shared outcomes and replaced
ownership keep the existing containment.

Evidence: `critical_persistence_recovery_test.go` (disconnected, open, panic,
partial-apply and selection continuations; retained instance ownership;
changed session/account/lifecycle/character/disconnect ownership is never
evicted) and `critical_persistence_recovery_postgres_test.go` (final save and
fresh relogin against real PostgreSQL). The full Go race suite with the
PostgreSQL fixture and `go vet ./...` passed. Deployed as release
`20261012T013651Z` ([public launch](public-launch.md#account-recovery-release--october-12-2026)).
Not covered: durable reconciliation of truly unknown outcomes across restarts
(rows above).

Separate validation debt: `TestStartWebAdminPostgres` and optional local JSON
account fixtures skipped in the final batch; they do not invalidate executed S2
PostgreSQL cases or count as passed tests. See the fixture instructions in
[build and integration](../build-and-integration.md#database-and-race-test-gates).
The user's October 7 gameplay report is not an exhaustive per-flow client gate.
Uxmal runtime/floor state is process-local; restart-resumable Uxmal is not
delivered by this migration. Synchronous login/world-entry reads are a separate
latency workstream, not an unfinished critical-save producer.

## Reading the migration history

The integration notes and staged batches below record how the migration was
built. Statements that `NewWorld` is disabled, that an injected owner is required,
or that race tooling is unavailable describe those earlier stages, not the
published implementation. The activation section supersedes producer/staging
status; October 7 corrections supersede timed quarantine release and provisional
instance visibility. Historical passing gates retain their original coverage
and do not close the current recovery queue above.

## Required end state

No critical database or filesystem save may execute on the World goroutine
during normal command/tick processing. World remains the only writer of live
accounts, characters, entities, indexes, trades, guilds and session state.
Workers receive detached persistence snapshots, not live pointers or gameplay
callbacks. Only a completed successful transaction allows its result to become
visible to the client or other players.

The bounded coordinator is only a foundation. Its tests cannot establish
nonblocking gameplay until every relevant producer and consumer is migrated.
Do not enable a handler merely because its own save call has become asynchronous.

## Ownership and states

| State | World owner | Worker |
| --- | --- | --- |
| Validate | Check identity, current resources and lifecycle; determine all affected aggregates/resources | None |
| Reserve | Reserve all participant accounts and shared resources; retain original and proposed state plus session generation | None |
| Submit | Reject without mutation if busy, stopping, unsupported or capacity exhausted; snapshot accepted work | Receive detached aggregate |
| Persist | Continue unrelated commands/ticks; do not mutate reserved participants/resources | Execute existing atomic store operation |
| Complete | Match operation ID and retained session/account generation; commit or restore/reconcile; publish only committed results | Report result exactly once |
| Release | Release reservations only after completion is applied; resume or reject deferred intentions | None |

The coordinator retains account and character-UID fences until explicit acknowledgement by its
owner, not merely until database completion. A completed transaction whose
World continuation has not run is still pending. Capacity includes queued,
running and completed-but-unacknowledged operations. Admission and shutdown
drain are separate: closing admission does not discard accepted work.

UID fences cover every character in each account snapshot and an explicit
sidecar UID, including deleted characters. A UID shared by distinct accounts in
one batch is rejected before admission; an already reserved UID rejects an
overlapping submission. These are persistence ownership fences, not a complete
gameplay reservation system.

## Integration requirements

1. **Coordinator:** bounded worker count and pending count; detached account,
   character-sidecar and instance snapshots; canonical account identity; duplicate participant
   rejection; atomic multi-account capability validation before submission;
   explicit completion/acknowledgement; panic outcome marked indeterminate.
2. **World completion path:** process completion under the same ownership as
   commands/ticks, including while command batches are busy. Retain continuations
   in an owner-only map keyed by operation ID. No callback runs on a worker.
   The staged owner and scheduling hooks now exist: at most eight completions
   per drain and one between session commands. Continuations must not perform
   blocking I/O. Store/continuation failure retains account fences and capacity;
   closures are discarded after delivery, so failures cannot replay a mutation.
   The provisional failure path enters global maintenance and preserves the
   shutdown error. Account-local isolation and reconciliation are still pending.
   `NewWorld` does not create a coordinator. CP recovery and equipped-egg elapsed
   hours now submit detached proposals when a coordinator is injected, applying
   live fields and confirmation only after successful identity-checked delivery.
   A held CP save is exercised through the real World actor loop: unrelated
   regeneration, ticks and overview commands continue without early confirmation.
   Autosave is deliberately postponed in that fixture; this gate does not prove
   nonblocking autosave or economy operations. A separate actual-loop test
   holds a final disconnect account write while unrelated regeneration, ticks
   and overview commands continue. A third actual-loop test holds the subsequent
   instance membership write while unrelated regeneration, ticks and overview
   commands continue, retaining disconnect ownership until acknowledgement.
   These fixtures postpone autosave and do not exercise production queue initialization.
   Cargo deposit/withdrawal now submits detached account proposals and applies
   only committed balances, then publishes the existing three-frame sequence.
   Its actual dispatcher/actor-loop test holds a transfer while unrelated ticks,
   regeneration and overview commands advance. A separate actual-loop unknown
   outcome test verifies maintenance without balance application or publication.
3. **Reservation layer:** cover packet handlers, ticks, movement/combat targets,
   buffs, timed items, CP, rewards, trades, ghost shops, ground objects, guilds
   and administrative commands. Reserve multi-party/shared objects together.
   Account fencing alone is insufficient for another player's attack or pickup.
   Staged guards now reject reserved account/sidecar loads and saves at the
   common World wrappers, and omit reserved players from autosave. Packet
   dispatch and common player movement also stop while a player is reserved,
   including after worker completion but before World acknowledgement. Bounded
   combat guards now reject reserved PvP participants before authorization and
   preflight the complete direct-skill target set before caster costs/effects.
   Mob target retention/chasing and attack deadlines pause for a reserved target;
   the common mob/boss damage boundary rejects it before HP, mana, mount damage,
   death or publication. CP recovery retains reserved deadlines; PvP CP penalties
   preflight every participant. Timed expiry/entry preparation reject reserved
   accounts before item mutation, including inactive characters and Cargo.
   Support selection rejects a reserved explicit target or selected party
   member before caster costs; direct application preflights every participant
   before healing, revival, teleport, cleansing or buffs. Regeneration and player
   affect ticks skip reserved players before HP/MP, deadline or expiry mutation.
   Player-owned effects also pause while their retained owner is reserved,
   before source validity, expiry, damage or rewards; disconnected mob-effect
   owners are checked by stable UID before stale-source cleanup. The direct
   area-effect entry rejects reserved casters. These are bounded guards, not
   complete support/summon or kill-reward operation migrations.
   Summon combat/follow now pauses reserved owners before rebinding, commands,
   movement and attack deadlines. Reserved PvP commands are retained rather
   than treated as invalid targets. Mob commands preflight the killer and all
   eligible party reward recipients before damage or command cleanup; reward
   eligibility and bonuses are unchanged. Direct BM creation and contract
   planning reject reserved owners. Physical/Iron Spear, offensive skills and
   periodic poison/area effects also preflight reward recipients before damage,
   caster costs or stored deadlines. Mount ticks, incubation/item entry points,
   follower reconciliation, hunt EXP, damage absorption and rider death reject
   reserved owners. A mount tick rechecks the fence after submitting egg hours,
   before touching food clocks or followers. These guards do not complete
   trade/shared-resource or administrative coverage. Contract publication still
   follows its synchronous item commit;
   asynchronous producer integration must publish only after acknowledgement.
   Remaining
   direct store callers and complete gameplay families still need migration.
4. **Lifecycle:** disconnect retains the pending account until its operation
   settles; reconnect/login cannot load the old durable snapshot. Session handles
   and player IDs may be reused, so completions must match retained identity and
   generation. Death, character replacement and cancellation cannot publish or
   restore into an unrelated replacement character.
   Account-session admission now rejects a reserved canonical account even if
   its old session ownership has been released. During explicit shutdown,
   disconnect commands close transports but retain aggregates and indexes until
   process exit; they cannot overwrite an accepted write.
   The staged `submitCriticalWrite` API now binds every participant to its exact
   session/player/account/character pointers, canonical account ownership, slot,
   character UID, world indexes and opaque lifecycle token. Character reset
   invalidates the token even when the same pointers and IDs are reused. All
   participants are checked before the producer continuation can publish; an
   identity mismatch retains fences and enters maintenance. A known maintenance
   failure also suppresses later successful publication. Low-level `owner.submit`
   is not a lifecycle-safe API for production producers.
   Normal disconnect now closes the transport but retains pending aggregates
   and ownership until successful owner delivery. Repeated disconnects retain
   one entry; queued packets from a closed transport cannot mutate that entry.
   Eligible deferred cleanup resumes in batches of at most eight and cannot
   remove a replacement player. Unknown outcomes retain ownership until exit.
   The final account/sidecar save now submits a detached snapshot through a
   private identity-bound path accepting only a retained closed transport.
   Ordinary gameplay admission still rejects closed sessions. An atomic
   account/sidecar adapter is required when sidecars are supported; two separate
   writes are never substituted. A full queue retains a disconnect-intent fence
   without installing a lifecycle token, changing state or saving inline.
   Completion/tick drains retry only rejected admission, not accepted writes.
   Permanent admission rejection retains ownership and logs once; it does not
   masquerade as an indeterminate transaction or retry every tick.
   The continuation only marks account persistence successful. Membership
   detachment then runs once on World, preserving resumable Water UID/leader
   identity. Final trade/shop/party/summon/spatial/index cleanup and readmission
   wait for a serialized instance snapshot that includes that detachment.
   A disconnect arriving after an older instance snapshot was admitted cannot
   release ownership on that older acknowledgement. Full admission retains dirty
   state and retries without inline I/O; permanent rejection retains ownership
   and logs once. Unknown outcomes suppress cleanup and terminal overwrites.
   Default production disconnect still uses the synchronous path until the
   remaining migrations permit constructor activation.
   CP-producer tests disconnect during a real held recovery, retain ownership
   through completed-but-unacknowledged work, and verify that resumed final
   persistence contains committed CP before readmission. No recovery packet is
   sent to the closed transport. Unknown outcomes retain ownership and suppress
   final persistence/readmission. The final save now has its own held reservation
   and acknowledgement before cleanup and readmission.
5. **Failure:** a definite rejection rolls back the complete proposed mutation,
   including indexes, reservations and shared resources. An indeterminate commit
   outcome is not an ordinary rollback: poison/fence the affected aggregates and
   reconcile durable state before accepting further mutations. Never retry an
   unknown outcome by repeating an economic intention.
   The existing store interfaces do not certify commit outcome for returned
   errors. The staged coordinator conservatively marks every store error and
   panic indeterminate while retaining the original error for diagnosis. Before
   enabling ordinary failure rollback, add and test an adapter-level outcome
   contract that distinguishes confirmed transaction rejection from unknown
   commit; do not infer certainty from an error string or a timeout.
6. **Autosave:** do not enqueue a precommit or poisoned account snapshot. Keep
   existing PostgreSQL stale-autosave fences; do not rely on worker ordering to
   replace them. Stores without a suitable async/transactional capability must
   reject unsupported operations, not fall back to inline writes.
7. **Shutdown:** stop admission, settle all accepted continuations, then take final
   snapshots and drain store autosaves. Repeated shutdown requests share the
   same drain result. Timeout is incomplete shutdown, not cancellation/rollback
   of a transaction still running. No final snapshot may overwrite pending work.
   This ordering is now integrated for the staged coordinator: World continues
   completion delivery without waiting on workers, then performs the terminal
   synchronous snapshot/flush. Retained callers are bounded at 64; later callers
   receive the cached final result. Unknown outcomes preserve the failure and
   skip final snapshots/flush rather than overwrite uncertain durable state.
   Explicit shutdown also saves queue-rejected account-only disconnects from
   character selection. Disconnect-intent fences no longer block terminal
   snapshots once gameplay has stopped; actual unresolved-write fences remain.
   The production tick path also stops gameplay mutation and autosave after the
   shutdown/maintenance barrier. Held CP tests use the same actor loop as `Run`,
   with injected ticks and a joined test-only stop signal. Shutdown-specific
   tests still exercise actual World methods with held adapters; they do not
   establish migrated economy/shutdown end-to-end behavior.
8. **Guild/instance aggregates:** detached instance-only and atomic
   account/instance forms now exist, guarded by one global instance reservation
   through actor acknowledgement. Every nested UID slice and party-run map is
   detached. Guild and combined account/sidecar/instance forms remain unsupported;
   the latter rejects admission before writing. A separate write cannot be
   presented as part of an atomic account/sidecar/guild operation.

## Migration sequence

Implemented staged instance batch: one global instance-aggregate reservation,
detached instance-only and atomic account/instance forms, and non-confirming
dirty-state flushes. Account/sidecar/instance combinations reject admission until
an adapter supports that exact atomic form. Snapshots serialize through
acknowledgement; an older completion cannot clear a newer dirty-generation
token. Existing inline instance writers reject overlap before writing.
Disconnect detaches membership once on World after account acknowledgement,
then retains account, UID and player ownership until a covering instance
snapshot is acknowledged. Unknown outcomes retain fences and suppress terminal
overwrites. Shutdown waits for accepted work before its explicit terminal
snapshot, which can include a later dirty generation. Flush never falls back
inline after staged shutdown/maintenance. These are durability writer fences,
not a reservation of every shared gameplay resource or member UID. Live dirty
state may continue changing while a flush is pending and is coalesced into a
later snapshot. This does not migrate instance economy/reward proposals or
establish complete shared-resource gameplay reservations.

Current bounded reservation batch: reject reserved participants before PvP
authorization and direct skill mutation; pause a mob's retained target before
chase/attack deadlines, and protect the common mob/boss damage boundary. Freeze
CP recovery deadlines and timed-item account mutations before save attempts.
Preflight every participant in direct multi-target skill and CP operations so
a reserved later target cannot leave an earlier target partially mutated.
Tests must hold actual coordinator writes through worker completion and until
World acknowledgement, then exercise the resumed operation. Support casts now
preflight all selected participants, including a reserved later party member,
before costs or partial application. Regeneration and periodic effects retain
reserved HP/MP and stored deadlines. Stable affect owners are checked before
stale-source cleanup, even when their transport is disconnected; direct area
effects reject reserved casters. Tests prove resumed healing, revival, teleport,
cleansing, buffs, regeneration, poison, area damage and expiry/source cleanup.
Summon combat now preflights party reward recipients before any lethal hit,
retains held PvP commands and freezes owner-linked summon state until World
acknowledgement. Held owners cannot enter BM creation or contract planning.
Ineligible party members and lower-priority reserved fallback commands do not
block unrelated attacks. These guards are tested with actual tick/creation
entry points, not a production queue migration. Physical/Iron Spear, offensive
skills and periodic effects now use the same reward-participant preflight.
Mount consumers retain item/character state, clocks and followers until owner
acknowledgement. The detached CP and egg-hour producers are covered with held,
completed-but-unacknowledged, rejected and unknown saves. CP retains the
450-second interval and packet order. Egg hours retain slot 14, effect 84,
whole-hour charging and fractional progress; admission rejection does not
consume progress or fall back to an inline save. Egg completion clears its clock
inside the identity-checked continuation rather than using a helper that rejects
reserved players. Neither producer is activated by `NewWorld`.
This batch does not authorize queue activation: remaining reward/shared-object
consumers, trade resources and administrative commands still require coverage.
Expiry timestamps remain real-time; skipped timed items are
removed on the next eligible sweep, not granted additional lifetime.
The same absolute-time rule applies to effects: reservations preserve stored
deadlines but do not grant extra buff lifetime or replay every missed tick.

- Implement and independently test the coordinator without enabling gameplay.
- Add World completion scheduling and the reservation/lifecycle layer. Exercise
  slow commits while unrelated sessions and ticks advance.
- Migrate one complete operation family, starting with counters/CP only after
  combat, autosave and disconnect consumers honor reservations. Preserve the
  450-second CP contract and existing packet order.
- Migrate single-account economic operations, then multi-account/shared-resource
  operations (trade, ghost shops, pickup/drop and rewards), then guild/instance
  and shutdown paths. Review every synchronous boundary's callers, not only a
  textual save-call inventory.
- Remove synchronous normal-loop fallbacks only after all consumers are migrated.

## Cargo gold migration batch

Cargo deposit/withdrawal is implemented under the injected coordinator. Preserve the
existing 16-byte requests, banker visibility/distance, living in-world character,
balance caps, and transfer/cargo/character response order. Validate before
admission, then submit a private account snapshot with proposed character and
cargo balances. Only an identity-bound successful World continuation may install
those two fields and publish confirmation. Do not replace the live account or
character with the worker snapshot, which normalizes persisted entry positions.

Admission rejection leaves balances unchanged and never falls back to inline
persistence. A reserved or closed participant cannot start another transfer.
After acknowledgement, an identical request is a new transfer: the native
packet contains no transaction ID. Unknown outcomes retain fences and enter the
existing maintenance path without confirmation, replay, or terminal overwrite.
Disconnect retains ownership until the transfer and final disconnect snapshots
are acknowledged; a committed transfer still updates retained balances without
sending packets to a closed transport. Test both directions, invalid requests,
capacity/stopping/identity rejection, held and completed-unacknowledged work,
disconnect/readmission, shutdown, and actual actor-loop progress. Keep the
constructor disabled until all remaining S2 requirements are met.

The cargo regressions cover both directions, the selected character slot and
unchanged sibling state/identity, exact balance-cap acceptance, domain and
admission rejection, capacity recovery, and reservation before banker-context
mutation. Accepted repetitions are suppressed through acknowledgement; the next
identical post-acknowledgement request commits as a new transfer. Unknown store
errors/panic and changed participant identity never apply proposed balances or
confirm success, replay accepted work, permit readmission or overwrite with a
terminal snapshot. Disconnect tests hold both the transfer and final atomic
account/sidecar write through completed-but-unacknowledged phases; final snapshots
contain committed balances and closed transports receive no confirmation.
Shutdown tests inspect terminal snapshot contents and repeated caller results.

Validation on 2026-10-04: focused cargo/legacy contract tests passed 20 repetitions
(`go test -count=20 -timeout=90s ./internal/game -run
'TestCriticalCargoGold|TestCargoGoldContract'`, 0.499 s). Full Go tests, vet and
native server build passed; details and remaining fixture/toolchain gates are in
the source audit. These tests prove the injected path only, not constructor
activation, other economy families, nonblocking autosave, or executed gameplay.

## Hell Gate purchase migration batch

The existing zero-treasury Hell Gate purchase is migrated under the injected
coordinator, separately from taxed
merchant/ticket purchases: those other callers still require guild transactions.
Keep category prices, item rolls/materialized identity, NPC/session/slot/gold
validation and the `UpdateCarry` then `UpdateEtc` response sequence unchanged.
Submit a detached account containing the charged gold and one granted inventory
item; install only those fields after successful identity-bound acknowledgement.
Do not roll again in a continuation or mutate/publish the live item before commit.

Reject closed/poisoned/reserved participants before trade cancellation or rolling.
If an existing trade counterpart is reserved, leave both trade contexts untouched
and reject the purchase; existing cancellation for eligible counterparts stays at
its original point, including on later invalid requests or save rejection. Queue
rejection does not retain the rolled item or request and never falls back inline.
Accepted repetitions remain fenced until acknowledgement; afterwards the filled
slot rejects a duplicate while another empty slot can receive a new purchase.
Test all categories, unchanged account fields, identity preservation, rejection,
unknown outcomes, trade-counterpart preflight, disconnect/final snapshots and
actual World-loop progress. Keep production activation gated on full S2 closure.

Validation on 2026-10-04: all category/character-slot proposals, held and
completed-unacknowledged repetition, domain/admission rejection, trade-counterpart
reservation, unknown error/panic/identity, committed disconnect snapshots and
terminal shutdown contents passed. Actual dispatcher/actor-loop tests verify
unrelated progress during held persistence and maintenance after an unknown
outcome. Autosave remains deliberately postponed in these fixtures. Focused
Hell Gate/cargo tests passed 20 repetitions (17.990 s); full Go tests, vet and
native server build passed. The initial publication assertion incorrectly
compared a finalized header size with an unfinished builder's zero size; the
corrected assertion checks the finalized length and retains the full payload,
recipient, opcode and response-order checks. Database/race gates remain pending.

## Cargo item swap migration batch

Implemented under the injected owner: storage/inventory and storage/storage swaps, separately from timed
equipment swaps. The existing boundary rejects direct Cargo/equipment moves.
Validate slot ranges, Auto Trade locks, the visible nearby banker target and
reserved trade counterpart before cancelling a trade or planning the swap.
Submit a detached account with only the two item slots exchanged; apply those
two item values and the existing single `0x376` cursor-completion response only
after successful identity-bound delivery. Preserve item UIDs, lifetime metadata,
gold, sibling characters and live account/character identity. Queue rejection
must not fall back to inline saving or retain a rejected request for replay.

Test both directions, occupied and empty destinations, storage/storage moves,
same-slot requests, held/completed-unacknowledged repetition, rejection and
trade reservations, unknown outcomes, disconnect/shutdown snapshots and actual
actor-loop progress. Cargo swaps do not trigger equipment appearance/mount
effects. Equipment uses the separate migration below; neither migration
activates the production coordinator or completes S2.

The Cargo-item batch passed detached-state, complete response-body, queue
saturation, legacy fallback, repeated request, trade reservation, unknown
outcome, disconnect, shutdown and real actor-loop tests. Its final inputs passed
the focused economy suite repeated 20 times, the full Go suite, vet and server
build. This is automated coverage, not PostgreSQL, race or gameplay approval.

## Timed equipment acknowledgement ordering

Prepare equipment moves on a detached account, validate affected slots and
activate item deadlines there. Initialize any fresh equipped mount exactly once
on the proposal before persistence; never reroll its state on delivery. Apply
only the intended item values after a successful identity-bound commit.

An optional actor-owned post-acknowledgement continuation runs synchronously
inside completion delivery, after the persistence reservation is released but
before another command, tick, disconnect or shutdown snapshot can run. It owns
egg-clock reset, score calculation, follower reconciliation and existing packet
publication. This permits guarded mount helpers without weakening reservations
for ordinary callers. Workers never receive either continuation. Rejected,
failed or stale-identity commits must never run the post-acknowledgement phase.
Any failure in that phase enters global maintenance before World yields: the
confirmed write is not rolled back or replayed, and poisoned sessions prevent
old terminal snapshots. Already acknowledged persistence fences are not retained
for this distinct post-commit failure; containment is global, not recovery.

All equipment moves under the staged owner use this path, including moves that
do not activate a deadline, so an ordinary swap cannot bypass durability or
mount planning. The nil-owner production path remains unchanged until S2 closes.

## Implemented expiry and entry-preparation migrations

The injected owner now plans expired equipment, Carry, inactive-character and
Cargo locations on one detached account. It applies the committed removals
before post-acknowledgement score, mount and client effects. Rejected admission
does not remove live items; a later sweep can submit a new intention. Unknown
commit retains ownership and enters maintenance, without an old-state save.
Tests cover all locations and selected-character slots, admission retry, held
writes, lifecycle snapshots and actual World-loop progress/failure. The nil-owner
production path remains synchronous.

Character entry also prepares unsupported/timed equipment changes on a detached
account. Selection and entry publication wait for each required preparation
commit. A deadline that passes during a held write requires another preparation
commit rather than entry with expired equipment. Final identity/slot checks,
disconnect/shutdown snapshots, no-write entry and actual dispatcher tests exist.
The sidecar read remains on World; this batch migrates preparation writes, not
all entry I/O. See `critical_persistence_entry.go` and its tests.

## Single-account merchant, skill and inventory migrations

Ordinary shop purchases, sales and TOTO ticket purchases now use detached
account proposals under the injected owner. Gold and item slots are installed
only after persistence and identity checks; publication retains the existing
wire format and ordering. Zero-treasury paths are covered. A nonzero guild tax
is explicitly rejected in the staged branch until the guild transaction and
shared reservation exist; this is a production-activation blocker, not an
acceptable replacement for the taxed operation. Shop tests cover domain and
admission rejection, held/repeated writes, reserved trade partners, unknown
outcomes, disconnect/shutdown contents and actual World-loop progress.

Skill-master requests through both BuyItem and ApplyBonus prepare gold, learned
skills and shortcut changes privately. They preserve mastery/chain/level and
advanced-evolution requirements, live identity and runtime score. Failed
validation must not recalculate the live character. Tests exercise both packet
paths, ordinary/ultimate learning, selected-character slots, domain/admission
rejection, uncertain commit, lifecycle and actual World scheduling.

Native DeleteItem (`0x2E4`, 20 bytes) and SplitItem (`0x2E5`, 24 bytes) now
prepare one/two visible inventory slots on a detached account. A split creates
one UID before admission and preserves effects, quantity and lifetime metadata;
delivery uses those same values. All changed slots are installed before the
existing `0x182` snapshots are sent. A reserved reciprocal trade partner is
checked before cancellation. Queue rejection leaves inventory intact and may
send only corrective old-state snapshots; it never saves inline.

`critical_persistence_inventory_test.go` covers source slots 0/5/62, all three
amount-effect positions, both selected-character slots, full-account proposal
contents, live identity and held/completed-unacknowledged requests. It also
covers rejection, trade reservation/cancellation order, errors/panics/stale
identity, final atomic account/sidecar disconnect writes, repeated shutdown and
actual dispatcher progress. A valid identical split after acknowledgement is
accepted as a new operation; the legacy wire has no transaction ID. Pending
reservation is not permanent duplicate suppression. The actor fixture postpones
autosave, so its progress assertions do not certify autosave migration.

Validation on 2026-10-04: focused inventory/legacy inventory/skill/shop tests
passed 20 repetitions. The full Go suite, vet and native server build passed
with `WYD_TEST_POSTGRES_URL` removed from the test process. Fixture-dependent
PostgreSQL tests therefore remain skipped, not transaction approval. A supported
CGO/race toolchain is still absent. None of these staged migrations enables
`NewWorld`, proves client execution or closes S2.

## Implemented staged multi-account trade migration

The staged trade path uses one detached `SaveAccounts` operation for both
participants, never two single-account saves. Validate final range/liveness,
offers, inventory capacity, gold and capsule ownership before admission. Prepare
both inventories, balances and account-local seal-ID remapping on private
account copies. Reject missing batch capability, saturation or invalid identity
without economic mutation or synchronous fallback.

Retain reciprocal trade objects and their full offer values through persistence
acknowledgement. Invitation, offer updates, close requests and timeout/general
cancellation must not mutate a reserved participant or its reciprocal trade.
Completion validates the captured contexts before applying either account;
install both economic results before publishing existing Carry/close packets.
Offer-check responses are intention acknowledgements, not economic success.

`critical_persistence_trade_test.go` covers both character slots, bidirectional items/gold and seal
collisions, held/completed-unacknowledged repetition and cancellation, a third
participant, rejection and unknown outcomes, identity/context replacement,
disconnect final snapshots, repeated shutdown and actual World progress. This
batch does not activate production or certify unrelated shared-resource paths.

Validation on 2026-10-04: trade and affected economy/legacy trade tests passed
20 repetitions (12.731 s). The full Go suite, vet and native server build passed.
The test process excluded the unverified inherited PostgreSQL connection;
fixture-dependent database tests skipped. Race validation remains pending.

## Implemented staged ghost-shop purchase migration

Prepare the buyer's inventory/gold and seller's Cargo/Cargo gold on detached
accounts and persist both in one atomic operation. Retain the shop's identity,
registration and complete listing context through acknowledgement. Neither a
second buyer nor owner closure, character reset or disconnect cleanup may
invalidate that context while the operation is pending. Account reservations
cover the shop because its sole owner participates in every purchase.

Install both committed economic results and remove the sold listing before
publishing the existing buyer, seller and observer packets. Shared-shop close
and sold-out cleanup must run only after reservation acknowledgement, without
yielding actor ownership. Closed transports receive no success responses.
Rejection must leave accounts/listings intact without inline saving; uncertain
outcomes must retain ownership and never publish success or overwrite old state.

`critical_persistence_ghost_shop_test.go` exercises selected-character slots,
full detached account contents, competing buyers, owner closure,
held/completed-unacknowledged writes, invalid listings and capacity,
disconnect/shutdown and real dispatcher progress. It also covers a fresh second
purchase after acknowledgement and direct/reciprocal-trade participant fences.
Ghost-shop actor fixtures postpone autosave; the separate autosave actor tests
below exercise the due periodic path, not every combined shop/autosave schedule.
No production activation or unrelated shared-resource certification is implied.

## Implemented staged periodic autosave migration

Under the injected owner, submit periodic account snapshots through the same
bounded coordinator as economic operations. Include the selected character's
sidecar in one atomic write when the adapter supports it; an adapter exposing
sidecars without the atomic form must reject before either write. Never call
the legacy asynchronous queues or synchronous fallback from this branch.

Autosave has no economic success response and installs no old snapshot into
live memory on completion. Bind the retained account/session/character identity
and keep its account and UID fences through World acknowledgement. Existing
gameplay guards pause affected participants while unrelated commands and ticks
continue. Disconnect final saves wait behind an accepted autosave. Unknown
outcomes retain ownership and enter maintenance, without a stale terminal save.

Keep the existing bucket cadence. Busy/full admission skips that account until
its next eligible slice; no rejected snapshot is replayed or saved inline.
`critical_persistence_autosave_test.go` covers all four selected-character slots,
detached nested account and sidecar contents, synchronous/asynchronous
sidecar-only capability rejection, account-only adapters, saturation without
fallback, identity/stopping rejection, held/completed-unacknowledged repetition,
subsequent fresh snapshots, disconnect readmission, repeated shutdown and
unknown error/panic/identity containment. A direct actor-owned mutation probe
checks snapshot isolation and durability-only completion; it does not certify
every gameplay consumer. The real World-loop tests make autosave due and prove
unrelated regeneration/overview/tick progress, reserved Cargo rejection and
fresh post-acknowledgement economy admission. Legacy nil-owner autosave tests
remain green. Production activation still requires all remaining S2 gates.

Validation on 2026-10-04: the ghost-shop focused suite passed (0.053 s), and the
autosave/fence/legacy autosave focused suite passed (0.048 s). Critical and
affected economy/legacy tests passed 20 repetitions (23.983 s). The full Go
suite passed (game 4.231 s, store 0.419 s, webadmin 7.241 s); vet and the native
server build passed. The full-suite child process excluded the unverified
inherited PostgreSQL connection. Database-dependent tests skipped; isolated
PostgreSQL transactions and supported CGO/race validation remain pending.

## Ground pickup migration contract

Ground pickup must reserve both the authenticated account and the exact ground
object before yielding World ownership. The staged handler retains the existing
packet layouts, instance visibility, reach, permanent-object rejection, currency
overflow checks and preferred-slot/first-free-slot behavior. Admission rejection
must leave the item, inventory and gold unchanged, without an inline save or
success response. Workers receive detached accounts, never the ground object.

Keep the object registered and visible while persistence is pending, including
completed-but-unacknowledged writes. Competing pickups and registry removal or
replacement must reject a reserved object. Expiry retains its original deadline
but processes other due objects; it must not spin or block behind reserved loot.
On successful identity-checked delivery, validate the retained object and planned
economic source field, install only the prepared inventory slot or gold balance,
remove the object for observers, and send the existing confirmation sequence.
A closed owner transport suppresses its reply, not the committed removal.

Instance cleanup may release private loot to public visibility without changing
its economic identity; that existing metadata-only transition is allowed while
reserved. Any other object replacement or content/position/deadline change must
fail closed rather than publish a stale operation. Unknown save outcomes retain
both reservations and enter the existing maintenance path, without replay or
rollback. Ground dropping is covered by the separate contract below. Reward
creation and other shared-object producers still require migration before
production activation.

## Ground drop migration contract

Prepare removal from Carry or Cargo on a detached account. Reserve an exact,
unpublished ground object and its ID before asynchronous submission. Keep the
live source intact until successful identity-checked completion. Pending drops
must remain invisible through spawn, view refresh and instance cleanup, and
reject guessed-ID pickups, replacement, removal and expiry. Other due objects
must continue expiring. Queue rejection removes only the unpublished allocation
without saving inline or changing the source. Unknown outcomes retain the
account and hidden object reservations for reconciliation, not rollback/replay.

Completion must check the retained source and object context, clear the source,
release the object's unpublished reservation and publish the existing ground
spawn and drop/slot confirmation sequence. Preserve effects, UID, Carry/Cargo
limits, warehouse proximity, filled-seal restrictions and listing locks. Closed
transports receive no owner confirmation; disconnect and terminal saves must
include the committed source removal. Private-to-public instance metadata
release may be accepted, but no other object mutation may be silently applied.
Reward generation and crafting remain independent producers to migrate.

The ground-drop tests exercise Carry/Cargo boundaries and all four selected
characters, exact detached proposals and packet bodies, held and completed-
unacknowledged writes, guessed-ID pickup, visibility/registry/expiry consumers,
private-to-public metadata release, UID materialization, cannon lookup,
admission/domain rejection, reciprocal reserved trades, unknown outcomes,
disconnect/readmission and repeated shutdown. Real dispatcher/World-loop tests
hold both successful and uncertain writes while unrelated ticks, regeneration,
overview commands and ground expiry advance. The original drop expiry is not
extended by persistence; an already expired committed item expires on the next
sweep. The focused drop/cannon suite passed 100 repetitions (3.446 s); the
critical/ground/drop/Cargo/trade/ghost-shop suite passed 20 repetitions (20.286 s).
Production initialization and PostgreSQL/race gates are still pending.

## Tiny crafting migration contract

Keep the existing recipe, gold cost, eight authoritative ingredient snapshots,
single percentage roll and message-before-result packet order. Plan all item
consumption, refinement and gold changes on a detached account. Do not alter live
inventory/gold, cancel an active trade or publish a crafting result before a
successful actor-owned completion. Admission rejection must not save inline or
fall through into the legacy mutating path.

A valid reciprocal trade is a shared context even though crafting transfers
nothing to its peer. Reserve both accounts in one atomic batch, preserving the
peer's durable state. Retain both exact trade contexts until completion; a busy
peer, stale trade or missing atomic-store capability rejects before mutation.
Apply only the prepared economic fields after verifying retained sources and
identities. Cancel the retained trade and send the existing crafting packets
after acknowledgement, without yielding World ownership. Closed transports
receive no confirmation, but disconnect/terminal snapshots include the committed
craft. Unknown outcomes retain ownership and enter maintenance without rollback
or reroll. Other artisans, sidecar-changing crafts and refinement consumers are
independent migrations; this contract does not activate production S2.

Tiny tests exercise success/failure, extra ingredients, all selected character
slots, detached nested proposals, exact packet order, repeated requests before
and after store completion, reciprocal trades and rejected admission without
inline fallback. Unknown error/panic/stale contexts retain reservations.
Disconnect, repeated shutdown and the actual dispatcher/World loop are covered;
the actor fixtures postpone autosave and do not certify every combined schedule.
The peer comparison uses the same normalized account projection as the retained
snapshot so an unchanged nil capsule collection does not cause false maintenance.
The focused crafting suite passed (0.067 s), and Tiny plus ground pickup/drop
passed 50 repetitions (4.466 s). The full Go suite passed with the inherited
database URL removed; database-dependent tests skip. Vet and the native server
build passed. Isolated PostgreSQL, race and production activation remain pending.

## Remaining S2 closure work

### Inventory-only composition batch contract

Migrate Compositor, Agatha and Aylin using the existing recipe checks, percentage
policy and gold costs. Compositor consumes the target, jewel and four materials
on either outcome and recreates the transformed target only on success. Agatha
always consumes the main item and four materials, consuming the donor only on
success. Aylin always consumes its catalyst/jewels and gold cost, changing the
main/donor only on success. Preserve UIDs, effects and lifecycle fields.

Workers receive only detached account proposals. Apply changed Carry slots and
gold after identity/source validation; send sorted slot snapshots, UpdateEtc,
text and the unchanged composition result only after acknowledgement. Keep
reciprocal trade contexts and both participant reservations through completion;
unlike Tiny, these handlers historically do not cancel a valid active trade,
so successful completion must retain it. Missing atomic capability rejects
before mutation. Unknown outcomes retain reservations and enter maintenance.
Hold/repeated requests, trade consumers, domain/admission rejection, disconnect,
shutdown and actual World progress require tests before this batch is accepted.
Final disconnect cleanup must cancel retained reciprocal trades after final
persistence acknowledgement. Its own disconnect-intent fence must not suppress
that cancellation. A reciprocal participant's accepted write must still finish
before either trade context or participant identity is removed. Cover owner,
peer and simultaneous disconnects, including the interval between the two
final acknowledgements, and revalidate consumed trade offers after composition.

This inventory-only batch does not migrate equipment/sidecar recipes or enable
production S2. The separate Lindy batch below covers its cape and Fame changes.

### Lindy level-unlock batch contract

Keep the existing seven-ingredient recipe, Arch lock checks and cape selection.
Unlocking 355 replaces the cape and changes the account flag without spending
Fame; unlocking 370 retains the cape and atomically spends one Fame point from
the character sidecar. Prepare these changes on detached account/currency
copies. Reserve a reciprocal trade peer and retain the trade, as in the original
handler. The 370 trade case requires an atomic multi-account/sidecar adapter;
missing capability rejects before live mutation, without sequential saves.
Validate retained ingredients, flags, cape, currencies and trade before applying
the committed delta. Recalculate equipment-derived runtime state on World only.
Publish slot snapshots, UpdateEtc, the existing score synchronization, composition
result and unlock message only after acknowledgement. Disconnect and shutdown
must retain the committed flags, cape and Fame. Unknown outcomes retain ownership.

Implemented under the injected coordinator in `critical_persistence_lindy.go`.
`playerStateBatchStore` and PostgreSQL `SavePlayerStateBatch` add a single
transaction for the owner's account/sidecar and the unchanged reserved trade
peer. Account and character-state fences use the existing store lock order;
account/ledger, sidecar and optional guild writes share one serializable
transaction. The coordinator rejects missing capability and cross-account
sidecar ownership before queueing. Production `NewWorld` remains disabled.

Validation on 2026-10-04:

- The staged Compositor/Agatha/Aylin batch and retained-trade disconnect cleanup
  passed its focused 50-repetition gate before Lindy was added. Final cleanup
  ignores its own saved disconnect intent but waits for the peer's accepted
  final write before destroying identities or trade contexts.
- `go test -count=1 -timeout=90s ./internal/game ./internal/store -run
  'TestCriticalLindy|TestPostgresPlayerStateBatch'` passed after correcting test
  fixtures and preserving the existing text-before-result packet sequence.
- The subsequent Lindy lifecycle additions passed
  `go test -count=1 -timeout=90s ./internal/game -run 'TestCriticalLindy'`.
  Coverage includes both limits, all four character slots, reciprocal trade,
  repeated held/unacknowledged requests, malformed/admission rejection,
  unknown error/panic and stale state, actual dispatcher/tick progress,
  owner/peer/simultaneous disconnects, spending the last Fame point and repeated
  shutdown. Final saves retain committed account and sidecar state.
- With `WYD_TEST_POSTGRES_URL` removed from the child process,
  `go test -count=1 -timeout=180s ./...` passed: game 4.494 s, store 0.493 s,
  webadmin 7.210 s. Fixture-dependent database tests were not exercised.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed.
  The output was not executed or deployed. No client inputs changed.
- `tools/repository/Test-RepositoryLayout.ps1` passed without `-UpdateMap`:
  local links and layout are valid, with 175 documents indexed.
  `git diff --check` passed; Git's LF/CRLF warnings are not whitespace failures.

The disposable-PostgreSQL batch commit/ledger-conflict rollback/deletion test
exists but requires an isolated database fixture. No operational database was
used. Race validation still requires a supported CGO toolchain. These are
automated results for staged paths, not production S2 or client-runtime approval.

### Extraction and alchemy migration contract

This internal scheduling change is `MODERNIZACAO_COMPATIVEL`. Under the injected
owner, skill 83 extraction and skill 84 alchemy prepare detached inventory
changes and reserve reciprocal trade participants before storage. No inventory
delta, trade cancellation, or success response is published before durable
completion and actor acknowledgement. Missing batch capability rejects admission
without falling back to inline persistence. Held and completed-but-unacknowledged
requests cannot reroll or move the cooldown.

Extraction preserves the strict `roll < chance` boundary, target UID/lifecycle,
damage conversion, whole catalyst consumption, and item-only success/failure
publication. Alchemy preserves all ten recipes, secondary-skill stack size,
first ingredient UID, optional ingredient consumption, `roll <= chance`, and
the existing item/UpdateEtc/text/result response sequence. Retained recipe inputs
and trade contexts are checked before applying a committed proposal. Unknown
outcomes retain reservations and use the existing conservative containment;
this batch does not supply reconciliation or enable production S2.

Implemented in `crafting_skills.go` and `critical_persistence_crafting.go`,
with staged-path tests in `critical_persistence_skill_crafting_test.go`.
The production nil-owner path remains synchronous until the remaining S2
producers and lifecycle requirements are migrated.

Validation on 2026-10-04:

- The focused `TestCriticalSkillCraft` gate passed after adding all ten alchemy
  recipes, admission rejection and owner/peer/simultaneous disconnect cases.
  It also covers extraction's strict probability boundary, all character slots,
  held and completed-but-unacknowledged replay, retained trades, stale inputs,
  unknown errors/panics and actual World dispatcher/tick responsiveness.
- With `WYD_TEST_POSTGRES_URL` removed from the child process,
  `go test -count=1 -timeout=180s ./...` passed: game 4.631 s, store 0.432 s,
  webadmin 7.289 s. Fixture-dependent database tests were not exercised.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed.
  The ignored executable was not run or deployed. No client inputs changed.
- Isolated PostgreSQL and supported-CGO race validation remain pending.
  These automated staged-path results do not close S2 or approve runtime.
- Repository layout/local links passed with 175 documents indexed, without
  `-UpdateMap`; `git diff --check` passed with line-ending warnings only.

### Bounded validation follow-up: advanced crafting and native consumables

Recorded on 2026-10-05 after the user reported repetitive continuation. This
entry closes the reporting gap for the already-started batch, not S2. No new
producer migration should be inferred from these results.

Classification: `MODERNIZACAO_COMPATIVEL`. Ehre/Odin proposals are implemented in
`critical_persistence_advanced_crafting.go`, with dispatch in
`crafting_advanced.go`. Equipped refinement, Nightmare tickets and mastery reset
proposals are implemented in `critical_persistence_native_consumables.go`, with
dispatch in `consumables.go` and `consumables_native.go`. These paths require an
injected coordinator; production `NewWorld` remains on the synchronous path.

Advanced crafting focused tests passed, including the subsequent owner/peer
trade-disconnect and shutdown tests. Native consumable tests exercise detached
account proposals, reciprocal trade reservations, held-write repetition,
post-ack publication, rejection, unknown/stale outcomes, owner disconnect,
shutdown and continued World ticks. Native trade-peer disconnect is not covered
by this batch. All 32 native detached-commit cases passed after correcting test
fixtures and expected shortcut filtering. The other native focused tests had
passed in the preceding run; this does not replace a green integrated gate.

Validation and limitations:

- An earlier consumable command reported `[no tests to run]`; its success was
  compilation evidence only, not behavioral validation.
- `go test -count=1 -timeout=180s ./...` failed in `internal/game`.
  A diagnostic rerun using JSON output isolated
  `TestLaktoreriumPowderConsumesOnlyCommittedValidAttempts/item_413_last_unit_disappears_after_successful_commit`:
  it still expects the old Portuguese log substring, while the translated
  diagnostic emits `consumed=true remaining=0`. The earlier item/state/save
  assertions pass. At that checkpoint the expectation remained uncorrected;
  integration was not approved. The resumed validation below supersedes that
  failure. No assertion was removed or weakened.
- Other packages passed in the full run, including store (0.427 s) and webadmin
  (7.399 s). `go vet ./...` and the server build passed. The ignored build output
  was not executed or deployed.
- `WYD_TEST_POSTGRES_URL` was removed from child test processes. Isolated
  PostgreSQL and supported-CGO race gates remain pending. No operational data,
  services, installed executable or client runtime was changed.

Do not continue producer-by-producer batches automatically or report S2 as
finished. The outstanding requirements below still determine completion;
unknown-outcome maintenance is containment, not reconciliation.

### Resumed validation and transaction-outcome hardening

Recorded on 2026-10-05. Classification: `MODERNIZACAO_COMPATIVEL`. The current
tree already contained the corrected English log expectation at entry; the
focused native-consumable/advanced-crafting gate passed without changing it.
No producer migration or production activation was added by this follow-up.

Fixed an adapter outcome-classification defect: a callback rejection nested
inside a failed rollback could satisfy `IsWriteRejected`, incorrectly allowing
a serializable retry despite an unknown transaction outcome. Unknown rollback
and commit outcomes now carry a private marker that takes precedence over any
nested rejection, including wrapped and joined errors. Original error causes
remain inspectable. This hardening applies to the migrated certified
`SavePlayerStateBatch` transaction path, not all legacy store adapters.

Added focused transaction tests for successful commit, certified rollback,
failed rollback with nested rejection, lost commit acknowledgement, server-side
serialization/deadlock aborts, retry bounds, pre-callback cancellation/begin
failure and panic cleanup. Unknown outcomes never retry. The real PostgreSQL
ledger-conflict test now requires adapter-certified rejection as well as
unchanged durable account and sidecar state.

Closed the recorded native-consumable trade-disconnect coverage gap with 12
cases: equipped refinement, Nightmare tickets, opportunity reset and balance
reset, each with owner, peer or both sessions disconnecting. They cover repeated
cleanup while a write is held, save-before-consumption/trade cancellation,
publication only to open sessions, final account/sidecar durability, retained
readmission fences until actor acknowledgement and no duplicate final saves.

Validation (`AUTOMATED TESTED` unless noted):

- Focused game/store outcome and trade-disconnect tests passed.
- PostgreSQL 18 ran as a disposable loopback-only fixture on port 55483, without
  connecting to operational databases. The batch commit/ledger-conflict
  rollback/deletion and delayed-autosave account/sidecar fencing tests ran and
  passed, without fixture skips. The fixture was stopped after validation;
  its data and log remain in the ignored server `bin/` directory.
- With `WYD_TEST_POSTGRES_URL` set only to that fixture,
  `go test -count=1 -timeout=180s ./...` passed, including game (5.279 s), store
  (1.267 s) and webadmin (7.147 s).
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed
  (`STATICALLY VERIFIED`). The ignored executable was not run or deployed.
- Repository layout/local links passed (175 indexed documents). Whitespace
  checks passed for all six affected, still-untracked files using no-index
  comparisons; tracked-only `git diff --check` cannot cover those files.
- Supported-CGO race validation remains pending: `CGO_ENABLED=0`, and no
  supported C compiler was available in the checked environment. Ordinary
  concurrency tests and database tests do not substitute for race detection.
- No installed executable, operational service, client input or wire contract
  was changed. There is no `CLIENT-TESTED` evidence from this batch.

S2 remains open. The next implementation decision is still governed by the
requirements below, not by repetition of the now-green validation commands.

### Certified rejection settlement and account adapter

Recorded on 2026-10-05. Classification: `MODERNIZACAO_COMPATIVEL`. This batch
adds a concrete recovery path for adapter-certified rejection, not unknown
outcome reconciliation or default coordinator activation.

The actor-owned coordinator now accepts an explicit rejection continuation.
It runs only when the store certifies that no commit occurred and the result
does not carry an indeterminate-outcome marker. Identity/lifecycle bindings
are checked before the producer checks retained economic/shared state. A
successful rejection continuation settles failure effects under the account
reservations, then acknowledges the operation without running its success or
post-commit callbacks. Failed or panicking settlement retains reservations,
drops callbacks and enters maintenance. Duplicate delivery cannot replay it.
Producers without an explicit rejection continuation keep conservative
containment; this is not generic error-driven unlocking.

Equipped refinement, Nightmare tickets, opportunity reset and balance reset
opt into this recovery. They verify the full retained owner/peer accounts,
trade identity and offers. Certified rejection consumes nothing, cancels no
trade and publishes no gain. An open owner receives its retained used-item
slot and an English failure message. Closed sessions receive no response;
their retained disconnect subsequently saves unchanged account/sidecar state
before releasing readmission. Only a new intention can retry the operation.

The new optional PostgreSQL `SaveCriticalAccounts` adapter uses the existing
atomic account/item-ledger transaction and account autosave fences, with the
certified serializable transaction helper. The coordinator prefers it for
account-only writes and prefers `SavePlayerStateBatch` even for a single
sidecar participant. Batch-only sidecar stores are accepted for that form.
Legacy callers and unsupported-store fallbacks remain unchanged; instance,
guild and combined-form adapters are not covered by this extension. Existing
diagnostics in untouched legacy PostgreSQL/model files remain English-language
migration debt; this batch does not claim a language migration or release.

Validation (`AUTOMATED TESTED` unless noted):

- Seven owner outcome/settlement modes verify certified rejection, unknown
  outcomes, indeterminate markers, missing handlers, callback errors/panics,
  success, held reservations and duplicate non-replay.
- Twenty native-consumable rejection cases cover solo/trade contexts and
  owner/peer/both disconnects, repeated requests, unchanged state, failure-only
  replies, subsequent explicit retry and final saves. Five stale-state,
  lifecycle or maintenance cases prove certification cannot bypass retention
  checks. Six adapter-routing cases cover one/two participants and preferred
  certified account/sidecar adapters.
- The existing disposable PostgreSQL 18 fixture was reused, loopback-only on
  port 55483. New account adapter tests passed for certified invalid admission,
  multi-account item transfer, repeated transfer without ledger duplication,
  foreign-owner conflict rollback and old-autosave fencing for both accounts.
- Eight real native-consumable PostgreSQL scenarios passed: each of the four
  production item-use paths through the held worker boundary, real commit or
  injected detached-proposal ledger conflict, retained actor state until
  acknowledgement, certified failure settlement and a successful new
  intention. The first test-fixture attempt lacked canonical timed-item UIDs;
  assigning valid fixture UIDs fixed admission before these gates passed.
  Actor responsiveness under a held write remains covered by the unchanged
  actual-World-loop tests; the new database test does not replace those tests.
  The fixture was stopped after validation, with no listener remaining on
  port 55483; its ignored data/log were preserved.
- With `WYD_TEST_POSTGRES_URL` set only to that isolated fixture,
  `go test -count=1 -timeout=180s ./...` passed: game 6.019 s, store 1.050 s,
  webadmin 7.328 s. The selected database gates ran without fixture skips.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed
  (`STATICALLY VERIFIED`); the ignored executable was not run or deployed.
- Supported-CGO race validation remains pending for the previously recorded
  missing compiler prerequisite. No client/wire inputs changed and no client
  was executed; this batch has no `CLIENT-TESTED` evidence.

Affected files in this batch (no files removed):

- `wydgo748/internal/game/critical_persistence.go`
- `wydgo748/internal/game/critical_persistence_owner.go`
- `wydgo748/internal/game/critical_persistence_lifecycle.go`
- `wydgo748/internal/game/critical_persistence_native_consumables.go`
- `wydgo748/internal/game/critical_persistence_rejection_test.go`
- `wydgo748/internal/game/critical_persistence_adapter_test.go`
- `wydgo748/internal/game/critical_persistence_postgres_test.go`
- `wydgo748/internal/store/postgres_critical_accounts.go`
- `wydgo748/internal/store/postgres_critical_accounts_test.go`
- `DOCS/server/critical-persistence-plan.md`

Repository layout/local links passed (175 indexed documents). No-index
whitespace checks covered these still-untracked files; tracked-only
`git diff --check` cannot validate their contents.

### Detached Gamble and certified instance aggregate adapter

Recorded on 2026-10-05. Classification: `MODERNIZACAO_COMPATIVEL`. This is a
bounded implementation batch after the requested continuation, not production
activation or full S2 closure. No client, wire or installed runtime changed.

The optional PostgreSQL `SaveCriticalInstanceState` adapter now atomically
persists the instance singleton and zero, one or multiple account participants,
including their item ledger. It uses the existing account autosave fences and
certified serializable transaction helper. Invalid admission is rejected before
writing; callback errors certify rejection only after proven rollback. Lost
commit/rollback acknowledgements retain the existing unknown-outcome policy.
The coordinator prefers this adapter for both aggregate-only and account/instance
forms, while preserving legacy adapters. At that batch's completion, guild and
sidecar/instance combined forms remained unsupported; the subsequent adapter
batch below adds those forms without migrating their production callers.

With an injected coordinator, the actual Gamble handler builds private gold,
silver-bar inventory and jackpot/pool proposals. It performs identity, queue,
adapter and resumable-instance ownership checks before consuming RNG; the worker
receives detached state, never the RNG or live actors. Reciprocal trade accounts
and the shared instance aggregate stay reserved through actor acknowledgement.
Repeated bets, trade cancellation and dirty aggregate flushes cannot overwrite
the accepted proposal while it is held or completed but unacknowledged.

Commit installs the prepared economic fields and publishes the existing Gamble
result/carry/etc packets; trade cancellation occurs only after successful
settlement. Certified rejection validates full retained owner/peer accounts,
trade pointers/offers, aggregate contents and generation before releasing fences.
It consumes no gold, pays no prize, changes no pools and cancels no trade. Only
a new intention can retry; closed transports receive no reply and their retained
disconnects subsequently save the unchanged account/sidecar state. Inventory
capacity rejection never admits a write or changes live gold/items/pools.
Unknown or stale outcomes retain reservations and enter maintenance, not replay
or reconciliation.

Retained aggregate comparisons use the captured admission time. Merely crossing
a hard deadline while the worker is held is not an actor mutation; actual
aggregate changes still invalidate settlement. This does not migrate the other
instance consumers: all of their mutations/orderings must be covered before
enabling the coordinator in production. The default nil-owner Gamble path and
`NewWorld` remain unchanged.

Validation (`AUTOMATED TESTED` unless noted):

- Focused Gamble tests passed for loss, jackpot and silver payouts, solo/trade
  contexts, held and completed-but-unacknowledged duplicates, shared-pool/trade
  fencing, exact post-commit packets, certified rejection, explicit retry,
  owner/peer/both disconnects, unknown/stale context containment, admission
  rejection before RNG, full inventory and clock-only deadline passage.
- The actual World dispatcher/tick test passed under a held Gamble write:
  unrelated regeneration and ticks continued without early confirmation.
  This actor-loop gate uses a held test store, separately from the real database
  gate below; neither is described as a live client test.
- Six adapter-routing cases cover aggregate-only, one-account and two-account
  forms, with certified-only and preferred-over-legacy stores.
- The previously created, isolated PostgreSQL 18 fixture was reused on loopback
  port 55483. Aggregate tests passed for standalone persistence, multi-account
  atomic commit, repetition and full rollback of a foreign-owner item conflict.
  Three actual Gamble-handler database scenarios passed for a loss, silver-bar
  payout and certified ledger-conflict rollback followed by a new intention.
  They verify durable account/pool state, retained live state and reservations
  until acknowledgement, no duplicate bet and no synchronous fallback.
  The first Gamble fixture comparison incorrectly distinguished omitted empty
  JSON collections from empty in-memory collections; comparing the serialized
  contract fixed the assertion before the focused and integrated gates passed.
- With `WYD_TEST_POSTGRES_URL` set only to that isolated fixture,
  `go test -count=1 -p 1 -timeout=180s ./...` passed: game 6.203 s, store
  0.821 s and webadmin 7.105 s. `-p 1` serializes packages sharing the isolated
  instance singleton. The new database gates ran without fixture skips.
  The fixture was stopped afterward; no listener remained on port 55483, and
  its ignored data/log were preserved.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed
  (`STATICALLY VERIFIED`). The ignored executable was not run or deployed.
- Supported-CGO race validation remains pending for the previously recorded
  compiler prerequisite. No client was executed; no `CLIENT-TESTED` evidence
  or language-migration/release approval is claimed.

Affected files in this batch (no files removed):

- `wydgo748/internal/game/critical_persistence.go`
- `wydgo748/internal/game/critical_persistence_adapter_test.go`
- `wydgo748/internal/game/critical_persistence_gamble.go`
- `wydgo748/internal/game/critical_persistence_gamble_test.go`
- `wydgo748/internal/game/critical_persistence_gamble_postgres_test.go`
- `wydgo748/internal/game/gamble.go`
- `wydgo748/internal/game/instance_state.go`
- `wydgo748/internal/store/postgres_critical_instances.go`
- `wydgo748/internal/store/postgres_critical_instances_test.go`
- `DOCS/server/critical-persistence-plan.md`

Repository layout/local links passed (175 indexed documents). The batch's
authored text is English. Whitespace validation includes no-index checks of
the affected files because several are still untracked; a tracked-only diff
cannot cover their contents.

### Certified guild and combined aggregate adapter

The coordinator now accepts detached guild registries and combined guild,
instance, account and optional character-sidecar writes. It clones guild
membership and city-war entries as well as the previously covered nested
account, sidecar and instance collections. Guild and instance singleton
reservations remain held with the participant fences until actor acknowledgement;
receiving a worker result alone does not release them. Stores without the new
optional adapter reject these forms before admission, with no decomposition
into separate legacy writes.

PostgreSQL `SaveCriticalGameState` writes the included account/item ledger,
sidecar upsert or deletion, guild singleton and instance singleton in one
certified serializable transaction. Empty character UID omits the sidecar;
nonempty UID with nil state deletes it. Invalid ownership, versions and
participants are rejected before write admission. The adapter fences older
account and sidecar autosaves without waiting for an unrelated async-worker
barrier. A proven transaction rollback certifies rejection; an ambiguous commit
or rollback acknowledgement keeps the existing unknown-outcome policy.

This adapter batch alone did not migrate production guild producers. The city
treasury producer is now staged as described below; other guild mutations,
shared consumers and gameplay publication still need retained-state checks
and domain ordering before using it. `NewWorld` remains unchanged
and disabled; S2 remains open. No client, wire or installed runtime changed.

Validation:

- `AUTOMATED TESTED`: held-worker coordinator tests cover seven aggregate forms,
  nested snapshot detachment, overlapping guild/instance/account rejection,
  completed-but-unacknowledged reservations, acknowledgement and explicit
  repetition. Unsupported adapters and invalid participants retain no fences
  and perform no writes.
- Real isolated PostgreSQL tests cover ten invalid-admission cases, seven
  atomic forms with repetition, late SQL rejection after every preceding
  aggregate write, sidecar-deletion rollback and foreign-owner ledger conflict.
  Both queued-autosave cases passed: old account snapshots cannot overwrite
  the combined commit, and old sidecars cannot overwrite an upsert or resurrect
  a deletion. No new database gate was skipped.
- With `WYD_TEST_POSTGRES_URL` restricted to the existing isolated loopback
  fixture on port 55483, `go test -count=1 -p 1 -timeout=180s ./...` passed:
  game 6.506 s, store 1.471 s and webadmin 7.042 s. `-p 1` prevents concurrent
  packages from interfering with shared singleton fixtures.
- `STATICALLY VERIFIED`: `go vet ./...` and
  `go build -o bin/tm-check.exe ./cmd/server` passed. The ignored executable
  was not run or deployed. Supported-CGO race validation remains pending for
  the previously recorded compiler prerequisite; no `CLIENT-TESTED` claim.
- The isolated database was stopped after validation; its ignored data/log
  were preserved. Repository layout/local links and whitespace validation
  passed, including no-index checks for the untracked files. Authored text
  in the affected files is English; this is not a global language audit.

Affected files in this adapter batch (no files removed):

- `wydgo748/internal/game/critical_persistence.go`
- `wydgo748/internal/game/critical_persistence_game_state_test.go`
- `wydgo748/internal/store/postgres_critical_game_state.go`
- `wydgo748/internal/store/postgres_critical_game_state_test.go`
- `DOCS/server/critical-persistence-plan.md`

### Detached city treasury collection

Recorded on 2026-10-05. Classification: `MODERNIZACAO_COMPATIVEL`. Under the
injected coordinator, the existing collector handler prepares a detached
account payout and guild treasury debit and submits them through
`SaveCriticalGameState`. The balance split is unchanged: below one billion,
pay character gold; otherwise pay unique-UID cheque item 4011 for whole
billions that fit in player carry slots, retaining the unpaid remainder.
Saturday, leadership, territory ownership, treasury cap, gold capacity,
inventory space and current participant identities are checked before
admission. No packet format, command identifier or installed runtime changes.

The account and guild remain unchanged while the transaction is held or
completed but unacknowledged. Actor acknowledgement verifies retained account,
guild registry identity, deep membership/war state and trade offers before
applying the payout. A reciprocal trade retains both participants and their
unchanged trade context. Only after commit and actor application are the carry,
character and success-panel packets sent. A closed transport receives none.
Certified rejection preserves the original account/treasury, emits one failure
panel and releases fences without automatic replay; a new intention may retry.
Unknown outcomes or changed retained context enter maintenance and keep fences.
That remains containment, not unknown-outcome reconciliation.

The guild singleton reservation also rejects legacy guild saves before pruning
or store I/O, rejects treasury credits and economy/war callbacks before mutation,
and pauses the guild-war scheduler without advancing its next tick. Combat
reward paths using the existing pre-damage participant check are fenced during
the shared guild write, including guild fame and Kefra ownership. This is a
conservative global reward fence, not proof that all remaining shared guild
consumers are migrated. Other guild producers still use synchronous fallbacks;
`NewWorld` still leaves the coordinator disabled and S2 remains open.

Validation:

- `AUTOMATED TESTED`: eight held-write gold/cheque/carry-limit/cap scenarios,
  with and without a reciprocal trade; duplicate admission and completion;
  global guild callback, scheduler and reward fences; certified rejection and
  explicit retry; unknown/panic and stale account, registry, membership,
  treasury, peer, trade, UID and lifecycle contexts; fifteen admission failures.
- Disconnect tests retain account ownership until the payout acknowledgement
  and a subsequent atomic account/sidecar final-save acknowledgement. They
  reject premature readmission, repeated final saves and transport publication.
  The actual `World.run` test advances unrelated regeneration, commands and
  ticks while a treasury transaction is held, then settles exactly one payout.
- Real isolated PostgreSQL producer tests passed for gold, partial cheques and
  foreign-item ownership rejection followed by an explicit retry. They verify
  durable account/treasury atomicity and no live application or publication
  before actor acknowledgement. No database gate was skipped in this batch.
- With `WYD_TEST_POSTGRES_URL` restricted to the existing loopback fixture on
  port 55483, `go test -count=1 -p 1 -timeout=180s ./...` passed: game 6.374 s,
  store 1.266 s and webadmin 7.077 s. The final test-only strengthening enables
  the scheduler and verifies its due tick stays unchanged under the reservation;
  the focused treasury gate passed again without repeating unchanged integration.
- `STATICALLY VERIFIED`: `go vet ./...` and
  `go build -o bin/tm-check.exe ./cmd/server` passed. The ignored executable was
  not run or deployed. Supported-CGO race validation remains pending on the
  previously recorded compiler prerequisite; no `CLIENT-TESTED` claim.
- The isolated PostgreSQL fixture was stopped and port 55483 was verified not
  listening; ignored data/logs were preserved. A startup without the fixture
  port option failed to bind the occupied default port and performed no test
  writes; the corrected start explicitly bound only `127.0.0.1:55483`.
- Affected-file authored text is English, including translated legacy guild
  diagnostics/comments. This does not complete the global language migration.
  Repository layout/local links and whitespace checks passed for this batch.

Affected files (no files removed):

- `wydgo748/internal/game/critical_persistence_guild_economy.go`
- `wydgo748/internal/game/critical_persistence_guild_economy_test.go`
- `wydgo748/internal/game/critical_persistence_guild_economy_postgres_test.go`
- `wydgo748/internal/game/critical_persistence_fences.go`
- `wydgo748/internal/game/guild.go`
- `wydgo748/internal/game/guild_economy.go`
- `wydgo748/internal/game/guild_wars.go`
- `DOCS/server/critical-persistence-plan.md`

### Detached guild creation and early command fences

Recorded on 2026-10-05. Classification: `MODERNIZACAO_COMPATIVEL`. This is a
bounded continuation of the treasury batch, not migration of every guild
producer. Under the injected coordinator, `/create` prepares a private guild
registry and account membership proposal and submits them together through
`SaveCriticalGameState`. Name, membership, ID capacity, canonical session,
account/character identity, store capability and queue admission are checked
before writing. Registry pruning runs only on the private proposal. A reciprocal
trade reserves both accounts and retains both offers without cancelling trade.

Live registry and account state stay unchanged until World acknowledges the
commit. Acknowledgement checks registry identity and deep state, account state,
trade context and participant bindings, then updates membership while preserving
the registry pointer. The existing success panel, identity, score and character
packets follow application, with no packets sent to a closed transport.
Certified rejection leaves membership unchanged and releases fences without
automatic replay; only a new intention retries. Unknown or stale outcomes retain
fences and enter maintenance, not reconciliation.

The remaining guild commands now check guild/account/trade reservations before
mutation. This covers invitations (including cooldown and expiry clearing),
acceptance, leaving, expulsion, promotion, alliances and war declarations or
cancellation. Online targets and leader succession participants are checked as
well as initiators. A save-time rejection alone is insufficient because an
intermediate mutation or registry replacement can invalidate a retained plan.
These guards do not make the other commands nonblocking: their unreserved paths
remain synchronous and the production owner remains disabled in `NewWorld`.

Validation:

- `AUTOMATED TESTED`: detached creation with/without reciprocal trade; retained
  state and exact packet ordering while held and completed-but-unacknowledged;
  duplicate requests; private pruning; certified rejection and explicit retry;
  eight unknown/panic/stale-context cases; eleven admission failures.
- Twelve competing guild command entry points preserve a retained treasury
  aggregate, including invitation/cooldown state. Six target-only reservation
  cases cover invitation, native recruitment, command/native expulsion,
  promotion and leader succession without relying on a global guild fence.
- Disconnect retains ownership until creation and final account/sidecar saves
  are acknowledged; repeated disconnect and premature readmission do not save
  or release state early. The actual `World.run` dispatch test advances unrelated
  regeneration, commands and ticks while creation is held, then creates one guild.
  The fixture uses the existing 108-byte chat contract with an inbound Size
  header; earlier fixture size failures were corrected without a wire change.
- Real isolated PostgreSQL tests passed for account/guild commit and foreign-item
  ledger rejection followed by explicit retry. They check durable membership,
  rank, retained balances/items and registry atomicity before actor publication.
  The existing bounded pool and parameterized fixture cleanup are preserved;
  no database schema or product configuration changed.
- With `WYD_TEST_POSTGRES_URL` explicitly restricted to the loopback fixture on
  port 55483, `go test -count=1 -p 1 -timeout=180s ./...` passed: game 6.889 s,
  store 1.548 s and webadmin 7.060 s. The focused guild/treasury compatibility
  gate and the new PostgreSQL producer gate passed as well.
- `STATICALLY VERIFIED`: `go vet ./...` and
  `go build -o bin/tm-check.exe ./cmd/server` passed; the ignored candidate was
  not run or deployed. Supported-CGO race validation remains pending on the
  recorded compiler prerequisite. No client inputs changed or client tests ran.
- The isolated fixture was stopped; port 55483 was verified not listening and
  ignored data/logs were preserved. Repository layout/local links passed with
  175 indexed documents. Whitespace checks passed, including the untracked
  files. Affected-file text was inspected in English; the language scan's only
  matches were the English word "expulsion", not untranslated legacy text.
  This is not a global language migration audit.

Affected files (no files removed):

- `wydgo748/internal/game/critical_persistence_guild_commands.go`
- `wydgo748/internal/game/critical_persistence_guild_commands_test.go`
- `wydgo748/internal/game/critical_persistence_guild_commands_postgres_test.go`
- `wydgo748/internal/game/critical_persistence_fences.go`
- `wydgo748/internal/game/guild.go`
- `wydgo748/internal/game/guild_protocol.go`
- `DOCS/server/critical-persistence-plan.md`

Next bounded implementation: detached invitation acceptance and native
recruitment, including both affected accounts, rejection settlement and held
write consumer tests. Do not treat the guards above as migration of those
producers or start additional producer families as a completion ritual.

## Activation (2026-10-06)

Classification: `MODERNIZACAO_COMPATIVEL`. This batch finished the migration
and enabled it; it supersedes the outstanding list below, kept for history.

- **Packet deferral instead of loss.** A reserved player's validated packets
  wait in a per-session queue (`critical_persistence_deferral.go`) and replay
  in arrival order on acknowledgement; newer packets queue behind older ones.
  Closed transports never replay. A backlog of 512 packets closes the session.
  Held repetitions are now native: they run after acknowledgement as new
  intentions (tests that assert one settlement discard them explicitly).
- **No inline gameplay writes.** With a coordinator, every legacy synchronous
  wrapper (`saveAccount`, character state, trade, guild, instance and
  multi-account saves) refuses through `syncPersistenceAllowed`, logs the
  operation once and counts `world_sync_persistence_rejected`. Only the
  terminal shutdown snapshot may write inline.
- **Remaining producers migrated.** Kill rewards (single, multi-target, DoT,
  summons, Kefra with guild fame) persist one detached multi-account batch;
  the mob stays dead but unpublished until acknowledgement and is resurrected
  on certified rejection. PvP CP, logout, DelayStart, character
  creation/deletion, quests, chat commands, Golden Shield, Sefirot, Eternal
  Stone, Arch ascension, Carbuncle and Mount masters, Celestial sealing and
  extraction use `queueCriticalLiveMutation`: the legacy code mutates, the
  mutated account becomes the proposal, live state returns to its pre-image
  and only acknowledgement installs and publishes it. Instance transitions
  (room moves, Uxmal start/reward, Big Cube and quiz answers, expiry exit)
  use `commitInstanceMutation`. The October 7 follow-up snapshots the proposed
  accounts, positions and runtime, then restores public RAM before admission
  yields. Confirmed completion installs under identity/preimage checks and
  publishes after acknowledgement; definite rejection preserves the restored
  state. Uxmal floor rewards stay invisible and reserved until settlement.
- **Account-local containment.** An unknown or unverifiable account-only
  outcome poisons and disconnects only its participants and retains their
  fences, disconnect state and queue capacity until verified reconciliation.
  The October 7 follow-up removes one-minute release and timed readmission:
  neither elapsed time nor an account reload proves a lost COMMIT has settled.
  Durable operation reconciliation and cross-process recovery remain pending.
  Guild/instance aggregates, shared ground/mob ownership and post-acknowledgement
  failures enter global maintenance. Shared gameplay ownership was added by the
  October 7 correction independently of the database aggregate type.
- **Activation.** `NewWorld` starts the coordinator last and fails closed when
  the store lacks any certified adapter. Configuration:
  `critical_persistence_workers` (default 4, below `database_max_conns`) and
  `critical_persistence_capacity` (default 1024). The PostgreSQL operation
  budget is `critical_persistence_timeout_ms` (default 500); timeout is not
  proof of rollback or a quarantine release deadline. See
  [the operating limits](operations.md#critical-persistence-operation).

Validation (`AUTOMATED TESTED` unless noted):

- New tests: deferral order/limits/closed sessions, account containment and
  retained quarantine, shared-aggregate global containment, activation
  rejection of uncertified stores, an activated `NewWorld` actor loop with zero
  inline writes, and an activated `NewWorld` over real PostgreSQL (CP producer
  commit, deferred packet replay, logout, shutdown).
- Full suite with the isolated PostgreSQL 18 fixture (loopback port 55483,
  role `s2test`, database `s2_fixture`): all 15 packages passed; the fixture
  was stopped afterwards. `go vet ./...` and `go build ./cmd/server` passed
  (`STATICALLY VERIFIED`; the output was not run or deployed).
- A call-graph reachability scan found no command or tick path to a legacy
  writer that does not first branch to the coordinator; the remaining reports
  were verified false positives (guards via `continue` or caller branches).

### Activation gates (passed on 2026-10-06; not verified recovery)

Independent follow-up on 2026-10-07 found shared-object failure paths in ground
pickup/drop and kill reward containment. The subsequent S2 correction added
identity-checked ground rejection settlement and global containment for
uncertain ground/mob ownership. All 11 original acceptance cases now pass;
35 additional settlement/identity/shutdown cases and 12 real PostgreSQL cases
cover the correction. The passing gates below describe October 6 coverage,
not these additional cases. See the
[S2 closure audit](s2-closure-audit-2026-10-07.md) for correction evidence,
integrated validation, follow-up timing/visibility corrections and the remaining
verified-reconciliation requirement. The follow-up adds an isolated PostgreSQL
delayed-settlement regression; it is not a TCP fault-injection or restart test.

- Race gate: passed on 2026-10-06. MinGW-w64 gcc 16.2 (WinLibs UCRT) was
  installed with user approval; `CGO_ENABLED=1 go test -race -count=1 -p 1
  ./...` with the isolated PostgreSQL fixture passed all 15 packages (game
  60.7 s, webadmin 123.3 s) with zero data races reported.
- Client gate: `CLIENT-TESTED` on 2026-10-06. The user ran the 7.48 client
  against the activated server (PostgreSQL, 4 workers, account containment):
  wrong-password rejection, login, character list, world entry, six NPC shop
  openings, an equipment consumable (tint) and an inventory/equipment swap all
  committed through the coordinator. The server log showed no rejected inline
  write, admission failure, persistence rejection, quarantine or maintenance.
  Kills, potions, trade and logout were covered by automated and PostgreSQL
  tests, not by this client session.
- Language: Portuguese comments, logs and errors in the files touched by this
  batch (`quest.go`, `ascension.go`, `uxmal.go`, `celestial_capsule.go`,
  `mount_master.go`, `spatial.go`, `internal/data/server_config.go`) were
  translated on 2026-10-06. Skill names in `catalog_test.go` stay as
  `SkillData.csv` contract values. Untouched legacy files remain global debt.

Out of S2 scope (follow-up, not a closure gate): synchronous database reads
still run on World at login and world entry (`LoadAccount`, `LoadCharState`,
character-name checks). S2 covers saves only.

### Historical migration requirements (producer status superseded)

This list records the pre-activation migration queue. Its producer and
constructor work is delivered; the requirement for unknown-outcome
reconciliation in item 4 remains open in the current recovery queue above.

1. Migrate the other critical economy producers into detached operation plans,
   including rejection, repetition and save-before-confirm tests. A wrapper that
   queues a save and immediately returns success is not a valid migration.
2. Finish shared-object ordering/consumer coverage for normal disconnect and
   enable it only after the other production paths are safe. Account/sidecar
   writes and covering instance membership flushes are now nonblocking under
   the injected owner; default production disconnect is still synchronous.
3. City treasury collection and `/create` are now staged on detached combined paths above;
   migrate the other guild producers onto the certified combined aggregate
   adapter above; migrate the remaining instance economic operations and shared-object
   reservations for rewards, ground objects, shops, trades and administrative consumers. Existing
   account/instance coordinator tests do not migrate those production callers.
   Gamble is now staged on the detached account/instance path above; the other
   instance consumers are not made safe by that migration.
4. Extend the tested `SavePlayerStateBatch`, `SaveCriticalAccounts`,
   `SaveCriticalInstanceState` and `SaveCriticalGameState`
   definite-rejection/unknown-outcome certification to the remaining required
   store adapters and test reconciliation. Explicit native-consumable and Gamble
   rejection settlement now recovers a proven uncommitted proposal; unknown outcomes and
   producers without such a continuation still enter global maintenance.
   That containment is not unknown-outcome reconciliation.
5. Remove the remaining normal-loop direct-store fallbacks. Periodic autosave
   now uses the coordinator under the injected owner; the default nil-owner
   path remains legacy. Enable the coordinator in `NewWorld` only after the
   other migrations, and validate the complete migrated paths with
   held writes, database transactions and a supported race-test toolchain.

## Validation and completion gates

Coordinator tests must prove detached nested snapshots, queue saturation,
unsupported-store rejection before writing, canonical multi-account fences,
unrelated-account progress, completion-before-ack ordering, error delivery,
indeterminate panic outcomes and drain without discarded accepted work.

Integration tests must prove continued World ticks under a held transaction,
no confirmation before commit, all rejection/rollback fields, repeated requests,
multi-account reservation, competing combat/pickup, disconnect/reconnect and
handle reuse, poisoned/indeterminate state, and shutdown timeout/repetition.
Run PostgreSQL transactions/fencing tests against an isolated fixture and race
tests with a supported CGO toolchain. Test the actual migrated production paths,
not a mock coordinator path alone. Full Go suite/vet are required for the
integrated batch; there is no client rebuild requirement until client inputs or
build dependencies change. Runtime failure gates remain separately recorded.

S2 is closed only when production saves no longer block normal World processing
and all lifecycle/transaction requirements above are verified. Neither this
plan nor green coordinator tests close S2.
