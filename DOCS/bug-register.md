# Gameplay bug register

Updated: 2026-11-09. This is the report status queue. Use [current state](current-state.md) for the latest recorded release.

## Queue

BUG-012–BUG-015 have source corrections and passing automated checks.
Their corrections were published in patch 48018 and consolidated into baseline 7.48.100.
See current state for the last recorded active release.
New owner gameplay acceptance remains pending. See the [release receipt](../.agents/research/release-748018.json).
BUG-016–BUG-024 corrections were published in mandatory patch 48101 and activated in release 7.48.101. Build and server regression checks passed; owner gameplay acceptance remains pending. See the [November 8 release receipt](../.agents/research/release-748101-20261108.json) and November 7 correction evidence below.
BUG-025 and BUG-027–BUG-028 were introduced in mandatory patch 48102. The owner's screenshots rejected its label occlusion and layout. Patch 48103 and release 7.48.103 now include the follow-up corrections for BUG-025 and BUG-029–BUG-030. Automated, publication and admission checks passed. The owner approved the installed local candidate; detailed scenario coverage and post-activation gameplay remain unrecorded. See the [current activation receipt](../.agents/research/release-748103-20261108.json) and [dated 102 evidence](../.agents/research/release-748102-20261108.json).
Published revision 104 corrects BUG-026 with the Quest centering calculation. Build and automated checks passed. Actual client acceptance remains pending; see the [Help update](client/ui-windows.md#help-commands-and-centering--2026-11-08).
The original eleven reports remain closed. On 2026-11-01 the owner confirmed BUG-001 to
BUG-011 fixed in the client (`CLIENT-TESTED` by the owner's report; no session
log or capture is attached). Reopen an ID only for a new concrete reproduction.

Priority is provisional: P1 blocks an encounter or loses essential session
state; P2 affects feedback, presentation, or behavior requiring diagnosis.
The rows are grouped by affected flow, not permission to abandon work already
in progress. No publication or restart is authorized by this register.

| ID | Priority | Issue | Current status |
| --- | --- | --- | --- |
| BUG-012 | P1 | FelipeTr Unicorn injury while the HUD shows high HP and C.C. is set to 50% | Published in 7.48.018; included in baseline 7.48.100: publish intermediate mount HP to its owner; update C.C., bars and tooltip without rebuilding appearance. Client acceptance pending. Exact death sequence remains unlogged. |
| BUG-013 | P1 | Crash returning from field to server selection | Published in 7.48.018: detach retired-scene geometry, discard terminal lists without traversal and avoid dereferencing borrowed focus during destruction. Local client installed; repeated-transition acceptance pending. |
| BUG-014 | P1 | October 30 client stack overflow | Published in 7.48.018: exact patch-48016 image, RTTI and saved stack prove recursive TMScene destruction. No-reentry and bounded collector corrections pass regression checks. Client acceptance pending. |
| BUG-015 | P2 | Mount food appears to decrease before one equipped hour | Published in 7.48.018; included in baseline 7.48.100: an accepted slot-14 move clears the previous mount's deadline, including moves between ticks. Same-slot and rejected requests preserve the clock. One-hour interval was already correct; timed client acceptance pending. |
| BUG-016 | P2 | Character creation label overlaps the nickname field | Source corrected: place the localized name caption above the nickname input, within the creation panel. EN/PT visual acceptance pending. |
| BUG-017 | P2 | Party invite gives no feedback when the target already has a party | Source corrected: remove the silent client party-flag rejection and let the server return its existing localized already-in-party message. Gameplay acceptance pending. |
| BUG-018 | P2 | Same-party health bar uses dark green like the local player's | Source corrected: restore the confirmed roster flag after observer creation and InitObject. Existing frame rendering resolves the dark green party color. Reciprocal view acceptance pending. |
| BUG-019 | P2 | Same-party characters are not visible on the minimap | Source corrected: restore the yellow party minimap marker when the visible actor is recreated. No new tracking outside visibility was added. Gameplay acceptance pending. |
| BUG-020 | P2 | Buying a skill also triggers world movement | Source corrected: capture skill and confirmation hit bounds before dispatch; consume the event even if confirmation hides the panel. Gameplay acceptance pending. |
| BUG-021 | P2 | Slot machine malfunction | Source corrected: clip reel symbols to the three-row viewport and separate the localized bet label and amount. Existing authoritative five-line payout and failure tests pass. Visual acceptance pending. |
| BUG-022 | P2 | Level quests do not charge their entry tickets | Owner clarified on 2026-11-07: charge all five level-quest entrances. Require and consume one respective ticket, IDs 4038–4042, using supplied native 7.59 quest scripts. Save failures restore payment and position. Gameplay acceptance pending. |
| BUG-023 | P2 | Gold Angel does not group collected stackable items | Owner clarified: only the equipped Gold Angel (3915) groups collected stackable items. Source corrected for ordinary, boss and global drops, plus ground pickup. Preserve quantities, effects and residual stacks; inactive fairies do not group. Gameplay acceptance pending. |
| BUG-024 | P2 | Gold coin drops do not auto-convert to currency | Shipped in 7.48.101. Direct monster coin drops use the same catalog and instance coin effects as ground pickup. Credit gold inside the kill transaction. Preserve the coin on the ground at the gold cap. Gameplay acceptance pending. |
| BUG-025 | P2 | NPC, player and ground-item labels overlap the drop-list window | Reopened after owner screenshots of 7.48.102. Revision 103 shipped gates for both actor render paths and hides existing labels on first open. Build and automated checks passed. The owner approved the local 103 candidate. Post-activation gameplay is unrecorded. |
| BUG-026 | P2 | H (Help) table opens off-center | Revision 104 centers the complete resource root like Quest. Build and automated checks passed. Revision 104 is published. Gameplay acceptance remains pending. See the [Help update](client/ui-windows.md#help-commands-and-centering--2026-11-08). |
| BUG-027 | P2 | Drop-list bonus and search mode are unclear | Published in 7.48.102: label base chance, chance with bonus, and the live personal bonus. Mode-specific prompts, no-result guidance, and disabled page controls clarify search state. Build and EN/PT locale checks passed; visual acceptance pending. |
| BUG-028 | P2 | Drop list includes NPC files with no active spawn source | The correction shipped in 7.48.102 and carried forward into 7.48.103. Index enabled generators, encounters, instances, and boss-summon sources. Exclude orphan templates from normal and global-event rows. Go tests and vet passed. Gameplay acceptance pending. |
| BUG-029 | P2 | Drop-list frame overlaps HUD; footer text and Close leave the artwork | Owner screenshots confirmed the defect in 102. Revision 103 shipped a 620x360 frame, actual HUD bounds, measured text limits, and the existing artwork's Close position. Build and automated checks passed. The owner approved the local 103 candidate. EN/PT and resolution coverage is unrecorded. |
| BUG-030 | P2 | Drop-list close/reopen can retain hidden focus or accept an old reply | Revision 103 releases focus in both keyboard modes and keeps request IDs across resets. Whitespace normalization agrees between localized item matching and the server. Focused automated checks passed. The owner approved the local 103 candidate. Specific lifecycle coverage is unrecorded. |
| BUG-031 | P2 | Fire Tiger mount lacks its native flames | Revision 107 shipped the mount effects. Seven animated billboards attach to the Fire Tiger's bones for riders and observers. Gameplay acceptance remains pending. See the [Fire Tiger trace](../.agents/research/client748/flows/ui/mount-species-effects.md). |
| BUG-032 | P2 | An incoming monster hit can display stale player MP | Confirmed in client source. The hit can briefly display maximum MP before the authoritative snapshot arrives. Timing in the real client is pending. |
| BUG-001 | P2 | HIT/MISS feedback missing for the attacker | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-002 | P1 | Noatun gate closes unexpectedly and blocks return | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-003 | P1 | Hell Gate has no visible monsters after entry | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-004 | P2 | Blood Foema incoming damage reduction appears incorrect | Closed: owner confirmed current behavior is correct |
| BUG-005 | P2 | Blood Foema mana consumption/control appears inconsistent | Closed: owner confirmed current behavior is correct |
| BUG-006 | P2 | Divine Judgment deals damage without its visual effect | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-007 | P2 | Nightmare party entry and identification are not synchronized | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-008 | P1 | Nightmare reconnect loses party and encounter HUD state | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-009 | P1 | Magic Chamber monsters disappear or remain untargetable | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-010 | P2 | Magic Chamber entrance floor runes are missing | Closed: owner confirmed fixed in the client, 2026-11-01 |
| BUG-011 | P1 | Compounder and city guards attack players | Closed: owner confirmed fixed in the client, 2026-11-01 |

Historical static lead — BUG-018/019 (2026-11-03; superseded by the November 7 source correction below): `TMHuman` initializes `m_bParty` to zero
(`tmproject/TMProject748/internal/game/entities/TMHuman.cpp:2576`). The observer
CreateMob path materializes a new human without resolving the party roster
(`tmproject/TMProject748/internal/app/scenes/TMFieldSceneWorld.cpp:457-536`).
`OnPacketAddParty` sets `m_bParty` and creates
the minimap marker only when `GetHumanByID` already finds that human
(`tmproject/TMProject748/internal/app/scenes/TMFieldSceneSocial.cpp:286-291`).
Server view entry sends CreateMob, HP and
Stop (`wydgo748/internal/game/visibility.go:193-201`); party roster entries use
separate 0x37D PartyMember packets (`wydgo748/internal/game/party.go:182-197`).
If the roster arrives before the actor exists, its party flag and marker are
not reapplied when CreateMob later creates the actor. This can explain the
asymmetric view reported by the owner; the exact runtime sequence and native
7.48 behavior remain unverified.

## November 7 correction evidence

Classification: `MODERNIZACAO_COMPATIVEL`. Existing wire formats and resource IDs remain intact.
Admission advances both source contracts from published revision 100 to candidate 101.
The candidate is not installed, published or active. No game was launched.

- BUG-016: indexed `SelCharScene2.bin` controls 5665/4626 share y=330. Separate their rows in the source initializer.
- BUG-017: `OnPgtPartyButton` silently discarded known party targets. The authoritative server already validates membership and supplies EN/PT text.
- BUG-018/019: retain the roster across actor removal. Reapply its confirmed membership and existing minimap color after `InitObject`.
- BUG-020: preserve the pre-dispatch hit, because confirmation can hide itself during the callback.
- BUG-021: `SReelPanel` exposed symbols at negative y and below row three. Clip texture and destination bounds together. Preserve the existing server payout system.
- BUG-022: owner-selected entry payments use native server scripts `13_350Quest__Gravekeeper.c` through `17_350Quest_Guard__.c` under `references/decompilacao server/dlls/TMSRV OFICIAL WYD BR ORIGINAL/DestinyServer-main/TMSRV2/run/Quest/`. Require and consume 4038/4039/4040/4041/4042 for Gravedigger/Gardener/Scout/Guard__/Envoy respectively. Other configured quest payments remain in force; do not invent tickets for free services.
- BUG-023: owner-selected Gold Angel rule extends the existing quantity contract. Server 7.59 `sub_507C60` confirms singleton powder handling, but its 120-unit cap and fairy eligibility differ. Keep this project's 255-unit stack contract and the owner's gold-only requirement; do not claim native 7.48 parity.
- BUG-024: use the existing `ground_gold` rule and `volatileGoldValue`; no coin denomination is inferred from its name.

Validation: client build with `-NoDeploy` and its test gates passed. Focused tests cover
paid-entry commit/rejection, gold-cap fallback, singleton powder grouping, stack residuals,
full inventories, detached ground-pickup commit/repetition, party, boss, global-drop and gamble flows.
The first full-suite run rejected missing MercZakum/MercCabuncle placements.
The owner confirmed these NPC removals were intentional. Update the population
check to validate retained positions and duplicate limits, without requiring removed services.
The final `go test ./...`, focused race checks, `go vet ./...`, and the server build passed.
The full-suite run used the configured isolated `WYD_TEST_POSTGRES_URL`.
Correction-only build SHA-256: `78E96DAADE29A8497FA51695BE7F476F6D5F79D1215901B6B23E01D130550C48`.
This dated build is superseded by the [drop-search candidate](protocol/drop-list.md#implementation-and-check-results), which includes these corrections.
Task logs are in ignored `build/bugs-016-024-*.log` and `tmproject/build/bugs-016-024-build.log`.

Owner acceptance: check EN/PT creation and bet labels, reciprocal party view reentry,
join/leave and relogin, minimap marker removal, skill confirmation without movement,
slot outcomes against credited gold, all five paid entries, Gold Angel collection,
and direct coin drops with an empty or full inventory.

## Disconnect and performance audit — 2026-10-31

This section reviews the owner's D1–D5, D8–D11, P1–P4/P6–P8 and P10 reports
against the working tree at HEAD `e0ae4abc`, including pending persistence changes. Keep
these report IDs distinct from the gameplay BUG IDs above. These are failure
mechanisms and capacity limits, not a ranking of measured production causes.
This audit did not inspect the VPS or change product code, settings or services.
The reported transient connection timeout is not attributed to these findings.

Execution order, dependencies and per-item acceptance are in the
[server correction plan](server/server-correction-plan.md). This register remains the status queue.

### Accepted correction backlog

P1 means availability or retained-session risk; P2 means latency, capacity or
recovery work. These priorities do not certify occurrence in production.

| Report ID | Priority | Verified finding | Pending correction |
| --- | --- | --- | --- |
| D1 | P1 | A certified autosave rejection closes the participant and retains the account fence | Closed (owner-confirmed 2026-11-01): active since R1 (release 20261101T050356Z) with automated retry, duplicate, concurrency, disconnect and unknown-COMMIT tests; no rejection in VPS play or in the simulation of 600 players in two channel processes (build/w9-load-20261101/); owner approved |
| D2 | P1 | Pre-COMMIT timeout cleanup uses the expired operation context and can report an unknown outcome | Closed (owner-confirmed 2026-11-01): active since R1 with exact XID reconciliation and the closed-connection fix (hotfix 20261101T064639Z); real-TCP COMMIT loss before, during and after COMMIT and a crash under load passed on local PostgreSQL; 0 indeterminate writes in VPS play and simulation; owner approved |
| D3 | P1 | The default 256-packet output queue closes on overflow without a cause-specific diagnostic | Closed (owner-confirmed 2026-11-01): active since R1 (W0 causes, W3 batching, byte bound, deferred scene); real TCP accepted on local slow links (0 server closes, peak 77/256) and VPS play over a 250 ms link (peak 0–1/256); the 600-player simulation peaked at 195/256 with no overflow close |
| D4 | P2 | A blackholed socket can retain account ownership until transport detection and actor cleanup finish | Closed (owner-confirmed 2026-11-01): active since R1 (keepalive, TCP_USER_TIMEOUT, bounded wait for a settling owner); loopback blackhole and VPS Linux 6.8 kernel tests passed; owner approved |
| D5 | P1 | Handler panic leaves one World in maintenance while readiness can stay 200; tick panic can terminate the shared process | Closed (owner-confirmed 2026-11-01): tick recovery and readiness active since R1 (W5); channel processes active since R5 (release 20261101T165857Z); on the VPS, killing channel 2 left channel 1 on the same process and admitting new logins, and systemd restarted channel 2 |
| D8 | P1 | The configured critical-write deadline also bounds account reads; initial authentication storage errors use the invalid-credentials panel | Closed (owner-confirmed 2026-11-01): active since R1; simulation of 600 players in two channel processes (build/w9-load-20261101/) with 8 database connections per process: critical write p99 at most 25 ms, oldest pending write at most 17 ms, no admission rejection |
| D9 | P2 | Four shared authentication slots reject excess concurrent logins; password hashing competes for process CPU | Closed (owner-confirmed 2026-11-01): active since R1; simulation of 600 players in two channel processes (build/w9-load-20261101/): 600 logins in 60 s waited at most 1 ms for a password-hash slot, 0 busy refusals |
| D10 | P2 | The tracked game unit requires the PostgreSQL instance; explicit database stop/restart propagates to the game | Closed (owner-confirmed 2026-11-01): active since R1 (`Wants=` unit, restart limit, store recovery from W5); owner approved |
| D11 | P2 | Terminal player saves are sequential across the shared actor, within one 20-second process shutdown budget | Closed (owner-confirmed 2026-11-01): active since R1 (bounded final-save workers); 300 populated accounts drain in 191 ms in the W5 simulation; VPS shutdowns drained without failure |
| P1 | P2 | Both channels share one serialized gameplay actor | Closed (owner-confirmed 2026-11-01): each channel runs its own actor in its own process since R5 (release 20261101T165857Z); the 600-player simulation of two processes kept loop lag p99 at most 10 ms per channel |
| P2 | P2 | Autosave submits a full account snapshot and fences that account during persistence settlement | Closed (owner-confirmed 2026-11-01): active since R1 (W7); simulation of 600 players in two channel processes (build/w9-load-20261101/): 26,838 critical commits with p99 at most 25 ms and 25,137 unchanged autosaves skipped without SQL |
| P3 | P2 | The final login account reload and character-state entry read run on the shared gameplay actor | Closed (owner-confirmed 2026-11-01): active since R1 (W2); simulation of 600 players in two channel processes (build/w9-load-20261101/): 600 entry reloads off the actor with loop lag p99 at most 10 ms; owner approved |
| P6 metrics | P2 | Staff metrics omit pool statistics and detailed critical-write state; transport closes lack cause counters | Closed (owner-confirmed 2026-11-01): active since R1 (W0) in the staff Runtime metrics; transport close causes, pool and critical-write state observed in the simulation; owner approved |
| P7 / P4 | P2 | Mob activation repeats expensive target and runtime checks before exact distance rejection on movement | Closed (owner-confirmed 2026-11-01): active since R1 (W6); simulation of 600 players in two channel processes (build/w9-load-20261101/) at 300 players per channel: loop lag p99 at most 10 ms (max 33 ms), tick p99 at most 50 ms |
| P8 | P2 | Several routine PvP, missed-attack and pickup logs bypass gameplay summary mode on the actor | Closed (owner-confirmed 2026-11-01): active since release 20261101T050356Z; VPS play on 2026-11-01 logged one GAMEPLAY summary per minute (skills, kills, drops) and no per-event line |
| P10 | P2 | Guild and instance snapshots each use a singleton row and shared reservation; an instance reservation skips instance lifecycle work | Closed (owner-confirmed 2026-11-01): W8 active in R4 (release 20261101T145133Z); per-runtime and per-guild rows proven by PostgreSQL tests; 0 skipped instance ticks in the simulation of 600 players in two channel processes (build/w9-load-20261101/) |

## Historical diagnosis and correction evidence

The following sections preserve the diagnosis and implementation sequence. Pending, open and source-only statements apply to their recorded dates. The status tables above contain the later owner-confirmed decisions.

### Evidence and acceptance boundaries

**D1 — certified autosave rejection.**
[queueAccountAutoSave](../wydgo748/internal/game/critical_persistence_autosave.go)
uses `submitCriticalWrite` without a rejection callback. The
[owner](../wydgo748/internal/game/critical_persistence_owner.go) therefore
retains even an adapter-certified rejection, and production containment closes
the affected session. An audit overlay reproduced
`sessionClosed=true accountFenced=true quarantined=1`.

Accounts rotate across six 500 ms buckets, giving a nominal three-second
per-account cadence. The next eligible bucket can retry a fresh snapshot;
the next global slice need not contain that account. Do not simply ignore all
errors: settle only certified rejection with current bindings, preserve live
state, and retain uncertain outcomes. Acceptance must cover fresh-state retry,
duplicate completions, concurrent commands, disconnect and unknown COMMIT.

**D2 — deadline and outcome classification.**
[certifiedTransactionAttempt](../wydgo748/internal/store/postgres_critical_transaction.go)
passes the same context to the callback, rollback and deferred cleanup. The
overlay reproduced `rollback outcome unknown: context deadline exceeded`,
with zero COMMIT calls and both rollback attempts using an expired context.
The configured 500 ms is the total operation budget, not a guaranteed time
spent in PostgreSQL or proof that COMMIT was attempted.

Use a separate bounded cleanup context. Prove that a pre-COMMIT callback cannot
commit independently or publish external effects before certifying rejection;
dispose of an unusable connection. A transport error during COMMIT requires a
different recovery path. PostgreSQL's
[pg_xact_status(xid8)](https://www.postgresql.org/docs/18/functions-info.html#FUNCTIONS-PG-SNAPSHOT)
can help reconcile a known transaction, but needs the correct top-level XID
for the exact attempt. It may return `in progress` or `NULL`; neither permits
unlock. A confirmed result still needs identity checks, durable-state reload
and coordinated account/shared-object cleanup before releasing fences.
Test real connection loss before, during and after COMMIT, late results,
repetition, crash/restart and later writes. The audit deadline probe uses a
context-aware test transaction; it is not PostgreSQL fault-injection evidence.

**D3 — output pressure and diagnostics.**
[Session.Send and writeLoop](../wydgo748/internal/net/session.go) close on queue
overflow, write error, short write or deadline-setup failure. These branches
do not log the initiating cause; the reader may subsequently log closure.
The default queue size comes from
[listener.go](../wydgo748/internal/net/listener.go), with 256 also selected in
the tracked deployment configuration. The overlay filled 256 slots and
confirmed closure on packet 257.

The expansion's arrival probe was rerun with the current map and generators:
50, 100 and 200 nearby synthetic players produced 220, 370 and 670 packets
for the arriving session. The probe uses a 65,536-packet queue without a TCP
writer. These counts confirm an oversized burst, not the queue depth with a
concurrent writer or a guaranteed disconnection threshold. Spawn randomness
also changes the counts slightly. Ranking this as the main production cause
still requires close-cause telemetry and real network load evidence.

The [October 26 investigation](../.agents/research/release-history.md#individual-disconnect-investigation--2026-10-26-channel-2-teleport)
already reproduced overflow during a 664-packet visibility transition with a
healthy loopback reader. Its historical population is not the current world.
War, Hell Gate and slow-link triggers remain plausible scenarios to test,
not separately reproduced causes. The writer calls `Conn.Write` per packet;
an exact syscall count has not been measured. Increasing capacity alone is
not a validated remedy. Acceptance must retain packet order and complete
entity create/update/removal, including large teleports and slow receivers.

**D4 — delayed dead-peer detection.** Authenticated reads have no idle
deadline. Go enables TCP keepalive by default, and the writer has a ten-second
write deadline; neither proves immediate detection while small writes still
fit the kernel buffer. No explicit `TCP_USER_TIMEOUT` is configured. Linux
[TCP documentation](https://man7.org/linux/man-pages/man7/tcp.7.html)
describes this option's bound for unacknowledged or unsent data. Exact detection
time and the VPS TCP settings were not measured in this audit.

[claimAccountSession](../wydgo748/internal/game/handlers.go) retains account
ownership until actor disconnect cleanup; an existing test requires this even
when `Session.IsClosed()` is true. Network client-slot pruning is separate
from account ownership. Do not replace an existing session merely because a
new socket requests login. Any takeover must authenticate first, fence the old
owner, settle persistence/trade/instance cleanup and reject uncertain outcomes.
Test abrupt loss, network blackholes, slow legitimate links and repeated login.

**D5 — panic and readiness.**
[safeHandle](../wydgo748/internal/game/world.go) recovers command-handler panic,
poisons online accounts in that World and sets `shuttingDown`. The process
stays alive, so `Restart=on-failure` does not act. The
[readiness callback](../wydgo748/cmd/server/web_admin.go) only checks database
ping; it can therefore report success while gameplay admission is closed.
`ChannelCluster.run` calls `tick` without a panic recovery boundary, so an
unhandled tick panic terminates the process that hosts both channels.
The existing handler-panic test passed in the focused race run; tick process
termination is statically verified, not reproduced by crashing a live service.
Acceptance must check both channels, actor heartbeat, readiness failure,
partial-state suppression and the selected recovery policy. Do not resume a
partially mutated actor simply by recovering the panic.

**P1–P3 — actor and persistence latency.**
[ChannelCluster.run](../wydgo748/internal/game/channel_cluster.go) serializes
both channels' command batches and ticks. Networking, authentication, HTTP,
persistence workers and Go runtime work can use other threads; saying the
whole server uses one core is inaccurate. `CPUQuota=150%` is a process CPU
ceiling, not a way to parallelize this actor. Shared account/guild ownership
must remain valid under any proposed channel split.

Autosave clones the account and character-state snapshot before worker
persistence. Reserved player commands wait until actor acknowledgement.
[PostgreSQL persistence](../wydgo748/internal/store/postgres.go) writes account
JSON, character/name projections and an incremental item ledger. It does not
rewrite every unchanged item. The claimed 20–30 sequential SQL statements and
periodic visible stutters were not measured here; query count depends on
character count, changed items and projection state. Acceptance needs SQL
counts and command-latency percentiles with realistic populated accounts.

Initial account loading and password verification already run in an
authentication goroutine. P3 concerns the later `onLoginResult` account reload
after session reservation and
[loadCharStateInto](../wydgo748/internal/game/charstate.go) during character
entry. These synchronous reads block the shared actor. Worker conversion must
preserve the fresh reload that prevents stale authentication snapshots from
restoring pre-trade items or gold.

### Expansion review and acceptance boundaries

**D8 — read deadline and login error mapping.**
[main.go](../wydgo748/cmd/server/main.go) passes the critical timeout into the
whole PostgreSQL store. The tracked defaults select 500 ms and eight pool
connections. [LoadAccount](../wydgo748/internal/store/postgres.go) uses that
same context for account reads, including pool acquisition and queries.
This is not a separate 500 ms allowance for each query. Four persistence
workers and four authentication slots can contend with other store users;
eight connections alone do not prove exhaustion.

An injected `context deadline exceeded` in the initial authentication result
reproduced the invalid-credentials panel. A later account reload and sanction
check already have different failure panels. Acceptance must cover each phase,
pool wait, database delay, retry and fresh-state admission. Pool increases and
longer read deadlines require measurements, not fixed untested values.

**D9 — authentication admission and CPU.**
[password.go](../wydgo748/internal/account/password.go) creates hashes with
600,000 PBKDF2-SHA256 iterations. Verification uses the stored iteration count.
Unknown valid account names also run a hash to reduce account enumeration.
[onLogin](../wydgo748/internal/game/handlers.go) rejects excess work when its
four shared slots are occupied; authentication already runs outside the actor.
The slots also cover account reads and sanction checks, not only hashing.
A local verification benchmark reproduced 132.8–133.5 ms per hash across three
runs. This is local elapsed time, not VPS throughput or a measured attack cost.

[Operational limits](../wydgo748/internal/game/operational.go) already enforce
30 attempts per IP/minute and ten per account/minute with bounded key storage.
A burst can receive the busy panel; its production duration and CPU impact
remain unmeasured. Distributed attempts remain a capacity risk, not a proven
production attack. The report's ten-IP saturation claim is not established:
at 133 ms/hash, 300 attempts/minute consume about 0.67 CPU core on average,
before other work. Do not weaken password hashing or add unbounded queues.

**D10 — database service dependency.**
The tracked [unit](../tools/deployment/vps/wyd-game.service) has both `After=`
and `Requires=postgresql@18-main.service`. Explicit database stop/restart
propagates to the game according to the
[systemd dependency contract](https://www.freedesktop.org/software/systemd/man/latest/systemd.unit.html#Requires=).
`Requires=` does not propagate every unexpected database exit as `BindsTo=`
would. This is a template finding, not a verified effective VPS configuration.
Changing the dependency requires tested persistence-outage handling; merely
keeping the process alive does not resolve D1 or D2.

**D11 — final-save throughput.**
[persistShutdownState](../wydgo748/internal/game/shutdown.go) saves eligible
account/character pairs one at a time, after accepted critical writes settle.
The cluster shares a deadline, and the process exits after the configured
20-second shutdown wait fails. The service's 30-second stop limit does not
extend that application budget. Unsaved terminal state is possible if total
drain time exceeds it. No safe player count or maximum loss window was measured;
the three-second autosave cadence does not bound the age of the last successful
save. Preserve pending-write ordering and uncertain outcomes in any new drain.

**P7 / P4 — activation hot path and measurements.**
The supplied 600-mob same-cell benchmark reproduced 450.5 microseconds per
`updatePlayerSpatial` call, with zero allocations, on an i5-12500H. It measures
activation maintenance, not a full movement handler or a visibility full scan.
The load probe reproduced a 300-player movement average of 327.2 microseconds,
with tick p50 6.65 ms and p99 21.93 ms. Movement runs before the measured tick;
these tick percentiles exclude that movement time, database work and real TCP.

A separate CPU profile attributed 52.1% cumulative samples to
`hasNearbyPlayerInGameplaySpace` in the load probe. The supplied predicate-order
overlay reduced the 300-player movement average to 161.0 microseconds in a
separate run. However, the compact 600-mob benchmark rose to 498.9 microseconds;
the improvement is workload-dependent. These are nondeterministic spawn/map
runs, not identical-state timing guarantees or VPS measurements.

The overlay changes only the predicate order in
[spatial.go](../wydgo748/internal/game/spatial.go). Existing spatial boundary
and target-selection tests passed. Preserve an explicit nil guard: the prototype
reads `p.X` before `validMobTargetAt`, which previously rejected nil entries.
Target score validation and runtime-index lookup also have observable failure
or cache-repair behavior. Further acceptance must cover dead/hidden targets,
expired affects, public/private runtimes and index invariants before adoption.

**P8 — synchronous routine logging.**
[handlers.go](../wydgo748/internal/game/handlers.go) directly logs ordinary
PvP hits, misses and pickups; [skill_pvp.go](../wydgo748/internal/game/skill_pvp.go)
also bypasses [gameplay summary mode](../wydgo748/internal/game/gameplay_logging.go).
These standard logger writes occur on the actor. They can add output latency
and log volume even when gameplay logging is quiet. Actual stalls or lost VPS
logs were not measured. Journald can drop a service's excess messages under
its [rate-limit settings](https://www.freedesktop.org/software/systemd/man/latest/journald.conf.html#RateLimitIntervalSec=);
the effective limit also depends on overrides and available journal disk space.

**P10 — aggregate reservation scope.**
[PostgreSQL storage](../wydgo748/internal/store/postgres.go) writes complete
JSON snapshots to singleton `guild_state` and `instance_state` rows.
The [shared coordinator](../wydgo748/internal/game/critical_persistence.go)
admits one write per corresponding aggregate through actor acknowledgement.
`criticalInstanceReserved` gates `tickItemInstances` and party-return processing
in [World.tick](../wydgo748/internal/game/world.go); competing aggregate writes
return busy. Other gameplay ticks continue. The reservation can span several
ticks, not necessarily one, and a failed aggregate fence can retain it.
Absolute deadlines can be checked when lifecycle processing resumes; skipped
processing does not imply every timer is extended. Test both channels and
unrelated rooms, slow writes, rejection, unknown outcomes and concurrent guild
or account settlement before splitting aggregates.

**P6 metrics — corrected scope.**
[ReadRuntime](../wydgo748/internal/control/runtime.go) already supplies staff
with lag, tick, command queue, transaction duration and async-store counters.
Some critical counters also exist in expvar, including quarantine and deferred
commands. The missing work is pool statistics, detailed critical queue/outcome
and latency coverage, staff exposure, and cause-specific transport counters.
This audit does not independently certify the deployed panel or sampler.

### Claims awaiting evidence or rejected as stated

| Report ID | Review | Required next evidence |
| --- | --- | --- |
| P4 | The expansion supplied the missing benchmark; the local cost is now reproduced and tracked with P7. Ordinary movement already uses spatial edge deltas | Keep activation maintenance distinct from visibility refresh; measure full movement and teleport paths separately |
| P6 | Tracked deployment sets `MemoryMax=1200M`, `CPUQuota=150%` and `TasksMax=128`; no explicit `GOMEMLIMIT` is present in tracked configuration. Private environment/drop-ins may differ. No memory/thread exhaustion was demonstrated | Inspect effective VPS settings, peak RSS/Go memory, thread counts and load before choosing limits |
| P6 metrics | The claim that metrics are invisible without `debug_address` is false. The authenticated staff sampler uses `control.ReadRuntime`, atomic counters and `runtime/metrics` independently | Closed: active since R1 (W0) in the staff Runtime metrics; transport close causes, pool and critical-write state observed in the simulation; owner approved |

### Audit validation

`STATICALLY VERIFIED`: the paths above and tracked configuration; no new wire,
economy or gameplay rule. Existing unrelated dirty work was preserved.
`AUTOMATED TESTED`: focused `go test -race -count=1` passed in game, net and
store. Temporary overlays reproduced D1, D2 and D3; existing outcome/retry,
writer, closed-session and handler-panic cases passed. Probes and logs remain
under ignored `build/disconnect-report-audit-20261031/`; product source was
not edited. The report's full-suite, vet and 23-package claim was not rerun or
independently certified here. No real database fault injection, VPS setting
inspection or client execution was performed. Fixes remain pending.

The expansion's supplied arrival, load and spatial benchmark probes were rerun
with temporary overlays. Logs, CPU profile and review probes remain under
ignored `build/server-audit-review-20261031/`. The supplied report's full-suite,
vet and broad race claims remain external results; its named full-test logs
were not present in the supplied artifact directory. No disposable database,
network load run, vulnerability download or client heartbeat test was added.
Focused race tests passed in game and store, including the injected login
failure, shared context budget, admission limits, instance serialization,
shutdown deadline and spatial boundaries. Account and control had no tests
matching that focused selection; their passing command status adds no coverage.
The password benchmark ran separately without the race detector.

### W0 diagnostics — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Packets, client revision, gameplay
rules, persisted formats and the staff authentication contract are unchanged.
The [correction plan](server/server-correction-plan.md#w0--diagnostics-and-reproducible-acceptance)
defines the scope. Nothing was published, installed or restarted.

- Transport: each session records its first close cause once. Reader, writer,
  overflow, admission and explicit server closes share the record. The
  [operations guide](server/operations.md#diagnose-a-disconnect) lists causes.
  Output rates, queue fill at enqueue, queue wait and refusals are published.
- Persistence: the game store pool, critical queue state, oldest pending age,
  worker outcomes and admission refusals are published. Panic, containment,
  maintenance and shutdown closes use their own causes.
- Actor: each channel publishes heartbeat age and lag, tick, command and
  queue-age distributions. The staff endpoint and Overview expose these groups.
- New defect fixed: both cluster channels shared one previous-tick time and
  overwrote the player, mob, queue and deferred gauges. Staff saw only the last
  ticked channel, and half the lag samples were near zero. Each channel now
  keeps its own values; process gauges are sums.
- P8: PvP hits and skills, misses, pickups, Illusion and summons use summary
  mode; verbose mode still writes each line. Player ground drops, admission
  rejections, invalid packets and persistence failures remain unconditional.
- Probes: `TestLoadProbe*` runs with `WYD_LOAD_PROBE=1`, a fixed seed, clock
  and tie order. Two runs queued 75, 225, 375 and 675 arrival packets for 0,
  50, 100 and 200 nearby players. These are actor counts, not TCP evidence.

`STATICALLY VERIFIED`: closed label sets, no account or session labels,
unchanged wire builders, and one close record per served session.
`AUTOMATED TESTED`: full `go test ./...` and `go vet ./...` with local PostgreSQL;
race runs for net, control, store, webadmin, game and cmd/server. The game race
run needs `-timeout 30m`; it took 605 s and exceeds the 10-minute default.
New tests cover each close cause, first-cause precedence, write timeout, pool
lifetime, critical outcomes, cluster lag and gauges, summary logging and charts.
`CLIENT-TESTED`: none. Pending: real network load test with a disposable
database, VPS inspection, bounded operation-ID records (W0 item 7) and activation.

### W1 certified rejection — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Packets, client revision and the
durable formats are unchanged. Nothing was published, installed or restarted.

- D2: rollback uses its own one-second cleanup context, never the expired
  operation context. COMMIT is the only durable boundary: a failure before it,
  including a failed rollback, is a certified rejection because callbacks use
  only their transaction. pgx discards the broken connection. A COMMIT error
  that pgconn marks as unsent is also a rejection. A COMMIT transport error
  stays unknown for W4. The total operation deadline still spans all retries.
- D1: autosave settles a certified rejection without closing the session.
  Live RAM stays current and the next eligible slice writes a fresh snapshot.
  Consecutive rejections wait 0, 6, 12, 24 and then 30 seconds per account.
- Producers: shops, Hell Store, cargo gold and items, skill learning, trade,
  equipment, personal shop, Lindy, advanced composition, character entry and
  channel transfer now resend the authoritative view and an existing EN/PT
  failure message. Trade closes both offers. CP recovery, egg incubation and
  timed expiry keep their state and retry later without a message.
- Unknown outcomes keep the existing containment and fences.

`STATICALLY VERIFIED`: every detached producer installs state only after
COMMIT; rejection continuations restore nothing and publish no success.
`AUTOMATED TESTED`: real PostgreSQL faults before COMMIT (callback error,
deadline during SQL, terminated backend, expired admission and exhausted
serialization retries) leave no row, no held lock and a clean pool. Game tests
cover autosave backoff and fresh retry, trade, cargo, merchant sale, Hell Store
and CP rejection. The updated equipment test keeps unknown outcomes contained.
Race runs passed for store and the critical game tests.
`CLIENT-TESTED`: none. Pending: W4 COMMIT-loss reconciliation and activation.

### W2 login and entry capacity — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Packets, client revision, password
strength and rate limits are unchanged. Nothing was published or restarted.

- D8: account and state reads use `database_read_timeout_ms` (default 3000),
  including pool acquisition, apart from the 500 ms critical write deadline.
  Read timeouts count in `postgres_read_failures_total`. Only invalid
  credentials or a missing account show the password error; storage failures
  show the temporary-unavailable message in the channel language.
- P3: the final account reload and the character sidecar read run on bounded
  workers. The actor keeps the account claim and pending state, then installs
  a result only for the same session, claim, sanction version and selection.
  Disconnect, shutdown and stale results release or ignore the read once.
  Repeated entry requests start no second read. Production enables this with
  `WithAsyncEntryReads`; direct handler tests run the same path inline.
- D9: `auth_hash_concurrency` now bounds PBKDF2 work only. Account and sanction
  reads run without a hash slot. A waiting attempt stops after two seconds or
  when its session closes and shows the busy message. Up to 16 attempts, or
  four per hash slot, may be in flight. The value 4 still awaits VPS tuning.
- Found and fixed: `account.Authenticate` used `os.IsNotExist`, which does not
  match the wrapped PostgreSQL not-found error. Missing accounts skipped the
  dummy hash, so response time revealed whether an account exists. The staff
  persistent-player read had the same check and reported a generic error.

`STATICALLY VERIFIED`: unchanged hash parameters and enumeration-equal work
for missing accounts; one claim release path for every reload outcome.
`AUTOMATED TESTED`: read budget with a held pool and an exhausted pool on
PostgreSQL; PT and EN login messages; async reload and entry with disconnect,
sanction change, replaced character, removed player and duplicate requests;
hash slot held only for hashing, refusal and session-close cancellation.
`CLIENT-TESTED`: none. Pending: VPS measurement of pool size and hash slots.

### W3 output pressure — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Packet contents, order and client
revision are unchanged; the 256-packet capacity stays the default.

- The writer joins packets already queued into one socket write of at most
  64 KiB. It never waits to fill a batch. `net_output_writes_total` counts
  socket writes next to `net_output_packets_total`.
- Each session also bounds retained output at 512 KiB; overflow by bytes or
  packets still closes with `output_overflow`.
- A full view refresh creates an entity only while more than a quarter of the
  receiver's queue is free. Otherwise the entity stays unpublished and the
  next tick refreshes the view again. Removals and updates are never deferred,
  and updates only reach created entities. A teleport recomputes the view,
  so stale creations are dropped. `world_scene_deferred_total` counts views
  that waited.
- Probe with the 256-packet queue and a writer that drains between ticks:
  0, 50, 100 and 200 nearby players completed in 0, 1, 1 and 3 ticks with a
  peak of 75, 192, 192 and 216 queued packets and no closure. Before W3 the
  200-player arrival queued 675 packets at once.

`STATICALLY VERIFIED`: removal and update paths unchanged; each direction of
a player pair is created independently. A failed enqueue still finalizes
before closing; no retryable payload is kept (the session closes).
`AUTOMATED TESTED`: coalesced writes keep order and frames over TCP; the byte
bound closes and releases retained bytes; a 120-mob arrival on a 64-packet
queue completes over ticks without an update to an uncreated entity; a
teleport drops pending creations and removes published ones.
`CLIENT-TESTED`: none. Pending: real TCP arrival, slow reader, blackhole and
guild-war traffic tests, and client confirmation of progressive scenes.

### W4 COMMIT reconciliation and stale sessions — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Packets, client revision and the
schema are unchanged. Nothing was published or restarted.

- D2: each certified attempt first reads `pg_current_xact_id()`. When COMMIT
  may have reached PostgreSQL but its reply was lost, the worker asks
  `pg_xact_status` for that exact transaction for up to three seconds before
  delivering the result. `committed` becomes success and `aborted` a certified
  rejection; `in progress`, NULL and read errors keep the unknown outcome and
  its containment. `postgres_commit_reconciled_total` counts each result.
- D4: game sockets use keepalive after 15 seconds idle, probing every five
  seconds up to four times, and `TCP_USER_TIMEOUT` of 30 seconds on Linux.
  Keys: `tcp_keepalive_idle_seconds`, `tcp_keepalive_interval_seconds`,
  `tcp_keepalive_count` and `tcp_user_timeout_seconds` (0 disables it).
  Before, Go and Linux defaults allowed about 150 seconds for an idle dead peer
  and about 15 minutes with unacknowledged data.
- D4: when the account's previous transport is already closed but its writes
  are still settling, a new login gets the temporary-unavailable message
  instead of "already playing". The duplicate-session policy is unchanged.

Not done: durable intent records and startup recovery for a process crash
during an unresolved write (items 3, 4 and 10). After a restart every account
and aggregate reloads from PostgreSQL, so only a COMMIT still running in the
old backend at that moment remains a narrow risk.

`STATICALLY VERIFIED`: an unresolved status never releases fences; the unknown
cause is quoted, not wrapped, so it cannot hide a reconciled rejection.
`AUTOMATED TESTED`: on PostgreSQL, a lost acknowledgement after a real commit
returns success and keeps the row; after a real abort it returns a certified
rejection with no row; an open transaction stays unknown. TCP options compile
for Linux and report "unsupported" on Windows; settling and live previous
sessions get distinct login answers.
`CLIENT-TESTED`: none. Pending: Linux kernel check of TCP_USER_TIMEOUT,
cable-loss and blackhole tests, durable crash recovery.

### W5 health and service lifecycle — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Packets and client revision are
unchanged. The effective VPS unit was not changed; only the tracked template.

- D5: a tick panic is now recovered like a handler panic. That World enters
  maintenance, poisons its RAM against saves and closes its sessions with the
  `panic` cause; the other channel keeps running. Before, a tick panic ended
  the process and both channels. `/readyz` now also returns 503 when any live
  channel is in maintenance or shutdown, or its actor heartbeat is older than
  five seconds. `/healthz` stays a process liveness check.
- D10: the game unit template uses `Wants=` and `After=` for PostgreSQL and
  stops retrying after five failed starts in five minutes. A fault proxy test
  drops every database connection: reads fail, writes are certified
  rejections, and the pool serves again when PostgreSQL returns.
- D11: final account saves are prepared on the actor as detached snapshots
  and written by bounded workers (`critical_persistence_workers`). Errors are
  still collected per account and reported as an incomplete shutdown.
  Locally, 24 saves of 10 ms took 255 ms sequentially and 63 ms on four workers.
- P6: no limit changed; `GOMEMLIMIT`, `TasksMax` and CPU quota still need VPS
  measurements.

`STATICALLY VERIFIED`: poisoned accounts are excluded before any final write;
readiness reads only atomics and never waits for World.
`AUTOMATED TESTED`: contained tick panic with the panic close cause; readiness
for live, stalled and maintenance channels; database outage and recovery through
a TCP proxy; bounded terminal-save concurrency.
`CLIENT-TESTED`: none. Pending: systemd restart budget and the real database
restart on a disposable service, populated-account drain measurement.

### W6 activation cost — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. No target rule, activation radius,
runtime boundary or packet changed.

- Mob activation and nearest-target search now reject by exact distance (and,
  for targets, by the current best candidate) before the validity and runtime
  checks. The runtime lookup scans every item instance for a player outside
  instances; its index update is a cache recomputed from authoritative
  membership, so calling it less often does not change any result.
- Oracle: 20 seeded worlds with dead, hidden and instanced players, 200 queries
  each, match the previous filter order for activation and for nearest-target
  selection with and without an acceptance filter.
- Paired probe, five alternating runs at 300 players on the official map:
  movement cost 174–175 µs with the previous order, 115–118 µs with the new
  one (about −33%). Tick p50 stayed near 4 ms; tick p99 varied from 13 to 16 ms
  in both orders, with no consistent change. Local Windows timings only.

`AUTOMATED TESTED`: the oracle and the existing spatial and combat suites.
`CLIENT-TESTED`: none. Not started: cached eligibility or narrower invalidation,
which needs a defined invalidation contract first.

### W4–W9 follow-up — source, 2026-10-31

Classification: `MODERNIZACAO_COMPATIVEL`. Nothing was published or restarted.

- W4: the game store names its connections `wydgo-game/<instance>`. At startup
  it ends every backend of a previous game instance in the same database and
  waits until PostgreSQL removes them. An open transaction of the old process
  aborts; a finished COMMIT is visible before any account is read. Tools and
  panels use other names and are never ended.
- W4: an account-only quarantine whose store result names its transaction is
  checked every ten seconds with `pg_xact_status`. `committed` or `aborted`
  releases its account and character fences (`world_critical_quarantine_released_total`);
  the discarded RAM is never saved and the player signs in to the stored state.
  `in progress`, unknown and read errors keep every fence. Shared objects,
  guilds and instances stay fenced until restart.
- W5: with the production option a contained handler or tick panic requests
  one supervised restart (exit 75, the staff restart path). The unit template
  stops after five failed starts in five minutes.
- W7: measured on a populated account, a JSON fingerprint on the actor took
  757 µs and a snapshot DeepEqual 129 µs, too costly for the shared actor. The
  worker now skips a player-state batch identical to what this process last
  committed (`postgres_unchanged_writes_skipped_total`). Any other write of the
  account or sidecar clears the mark first; after 19 skips one write happens.
- W7 (items 2–6): the autosave is a background snapshot. It keeps the
  persistence boundary (disconnect, login, final save) but no longer defers
  the player's packets or stops mobs and kills against that player. A later
  critical write of a participant parks behind the snapshot and goes to a
  worker only after the snapshot is acknowledged (`world_critical_parked_total`),
  so writes of one account commit in submission order. A failed snapshot
  refuses its parked writes as certified rejections that never reached the
  store; a parked write that settles hands its fences back to the failed
  snapshot. A never-sent write that ends in containment is released at once;
  a released quarantine keeps the failed fence of another unresolved one.
  Identity changes during the snapshot (leaving the world, character change)
  no longer isolate the player.
- W8: not changed. Running skipped instance work inside completion delivery
  broke the instance-entry ordering test and was reverted.
- W9: the shared-ownership inventory is in the
  [correction plan](server/server-correction-plan.md#w9-item-1--shared-ownership-inventory-2026-10-31).
- G: blocked on the installed candidate and the owner's client tests.

`AUTOMATED TESTED`: on PostgreSQL, a new instance aborts the previous
instance's open transaction and leaves an unrelated pool backend alive; an
unchanged batch is skipped, a changed one and one after another write are
written, and the skip bound forces a write. Game tests cover one restart
request for two panicking Worlds, and a quarantine
kept while in progress and released for committed and aborted outcomes.
`CLIENT-TESTED`: none.

### W4/W5/W7 completion — 2026-11-01, active

Classification: `MODERNIZACAO_COMPATIVEL`. W0–W7 are active in release
`20261101T050356Z` (owner-authorized publication and restart, no player
online); see [current state](current-state.md).

- W4 item 10: staff with the new `game.persistence.recovery` capability read
  `GET /api/v1/staff/persistence/quarantine` (ID, channel, accounts, shared,
  checkable, last status) and request `POST` checks with a reason; the panel
  has a Quarantine section. The check uses the periodic final-status rule and
  is audited. The capability is not granted to any staff account by default.
- W4 items 1–4: recorded as satisfied by the per-attempt XID, PostgreSQL's
  commit status and the startup fence; see the
  [decision](server/critical-persistence-plan.md#w4-durable-record-decision-2026-11-01).
- W4/D4 items 4–6: a login behind a closed previous owner waits up to five
  seconds and then claims the account (`world_login_replacement_total`). An
  open previous session still gets the duplicate-login reply; a quarantined
  account gets the temporary-unavailable reply instead of "already playing".
- W5/D11: measured on the local PostgreSQL with populated accounts (see the
  correction plan). A first probe with items lacking UIDs produced
  serialization failures under four workers: each save created new item
  identities. Live items always carry a UID, and the realistic probe passed.
- W7: background autosave and parked writes, as recorded above.
- P1 measurement: the load probe records each channel's full cycle (moves
  plus tick), which is the wait it adds to the other channel on the shared
  goroutine: 2/6/12 ms p50 and 7/11/22 ms p99 at 50/150/300 players.
- P10 measurement: a two-channel instance aggregate of 10/50/100 runtimes
  (8 members each) is 7.7/37.9/75.7 KB and writes in 0.7/1.5/2.6 ms p50 on
  the local PostgreSQL. The reservation therefore lasts milliseconds; the new
  counter gives the production skip rate before W8 changes the schema.
- W6 item 5: one neighborhood read per recompute pass replaces one cell-map
  scan per mob. Oracle tests match the old scan; three paired probe runs at
  300 players moved from 112–114 µs to 59–60 µs per step.

`AUTOMATED TESTED`: queue parking, multiple predecessors, closed admission,
failed-snapshot refusal and fence hand-back; a trade parked behind a partner
autosave; the double fault; identity change during autosave; PostgreSQL row-lock
ordering (durable gold equals the newer write); staff status/recheck in the World
and over HTTP (capability, CSRF, revocation, audit); login wait, release, timeout
and quarantine replies; loopback TCP slow reader (1,500 ordered packets) and a
reader that stops reading (closed with `write_timeout`), five race repetitions.
On the VPS (Linux 6.8.0, temporary directory removed afterwards) the same TCP
tests and the kernel `TCP_USER_TIMEOUT` check passed.
`CLIENT-TESTED`: none.

### COMMIT-loss acceptance and closed-connection fix — 2026-11-01

Classification: `MODERNIZACAO_COMPATIVEL`. Found by the new real-TCP acceptance
test, after release `20261101T050356Z` activated W1.

- A proxy between the game store and PostgreSQL recognizes the frontend
  COMMIT message and drops the connection before or after PostgreSQL receives
  it. World, coordinator, store reconciliation and quarantine run unmodified.
- Defect: when the connection closed after COMMIT was sent, pgconn returned
  its "conn closed" lock error, which claims to be safe to retry. W1 trusted
  it and certified a rejection although PostgreSQL had committed. A producer
  with a rejection continuation could then undo committed effects in memory.
- Fix: "conn closed" no longer certifies an unsent COMMIT. The worker asks
  PostgreSQL for the exact transaction status: `committed` is success,
  `aborted` is a rejection, anything else stays unknown and quarantined.

`AUTOMATED TESTED` (local PostgreSQL, real TCP): an acknowledgement lost after
COMMIT reconciles as committed with no rejection; a COMMIT that never arrives
becomes a rejection and the next autosave writes the retained state; a lost
acknowledgement with the database unreachable quarantines the account by XID
and releases it on `committed`, and the durable state is the committed one.
A store test fails without the fix. Game-only hotfix release `20261101T064639Z` is active since
2026-11-01 06:46 UTC (owner-authorized restart, no player online; schema unchanged at v14).

Local acceptance run (`build/acceptance-20261101/acceptance-summary.md`): the
real server with production queue and persistence limits ran 60 direct bots
and 20 bots behind a 150 ms, 16 KB/s link for three minutes with no session
closed by the server, a peak output queue of 77/256 and 1,298 committed
critical writes with none rejected or unknown. A forced process kill under
load left no open transaction; all 80 accounts loaded and validated, and all
80 signed in again after the restart.

### VPS observation — 2026-11-01 08:23 UTC, read-only

Active hotfix release `20261101T064639Z` since 06:46:41 UTC: 0 restarts, 160 MB resident memory,
load average 0.05, two idle game backends. Since activation the journal shows no panic, no
indeterminate or quarantined write, no output overflow, an output queue high water of 1/256, and only
the four post-activation admission checks (revision 16 rejected twice, two credential checks). No
player entered the world after activation; the previous 14 days had 182 character-list deliveries.

A second sample at 14:44 UTC (7 h 58 min active) is unchanged: 0 restarts, 164 MB, no panic or failed
write, three handshake closes from unauthenticated connections, and still no player in the world. With
no player online, the owner then authorized R4, which is active since 14:51 UTC (release
`20261101T145133Z`, schema v20, no panic or error at start).

The pending VPS measurements of D2, D3, D8, D9, D11, P2, P6 and P7/P4 need real player traffic.
Repeat this read-only inspection after players return; it needs no restart.

### VPS measurement with the owner online — 2026-11-01 15:01–15:05 UTC, read-only

R4 release `20261101T145133Z`, one player (the owner) in the world over a 250 ms round trip, eight
30-second samples (evidence: `build/r4-20261101/live-sample.log`):

| Measure | Result |
| --- | --- |
| Game process | 0.6–0.9% CPU, 166–176 MB resident, 0 restarts |
| PostgreSQL | 1.5–1.8 commits/s, 0 rollbacks; 5–10 account and 1–6 character-state row writes per 30 s |
| Transport | Send queue at most 1 KB; output queue high water 0–1/256; no overflow or timeout close |
| Ownership | One account lease (the owner's); process heartbeat age 3–6 s |
| Journal since R4 | 1 login, 0 panics, 0 indeterminate, quarantined or failed critical writes; one GAMEPLAY summary per minute |

What one player proves: R4 serves play with the W1–W9 write paths, leases and per-row writes, with
no failure and a small footprint. What it cannot prove: pool sizing (D8), hash concurrency (D9),
shutdown drain with players (D11), autosave latency percentiles under load (P2), activation cost at
many players (P7/P4) and the instance skip rate (P10); these need more players. Loop lag, tick, pool
and critical-queue values are in the staff panel's Runtime metrics, which need a staff session; this
inspection used none.

### Load simulation — 2026-11-01, two channel processes, 600 players

The owner accepted simulation as the load evidence: more than a few players cannot be gathered for a
VPS test. Run: R4 code (commit `a88fff03`), two processes in channel-process mode on local
PostgreSQL with the production limits (8 database connections, 4 critical workers, 4 hash slots,
256-packet output queue, 400 connections per process; only the per-IP limits were raised because all
bots share 127.0.0.1). 300 bots per channel connected in 60 s and moved and attacked for 4 minutes
(about 70,000 attacks per channel). Evidence: `build/w9-load-20261101/`.

| Measure | Channel 1 | Channel 2 |
| --- | --- | --- |
| Players in world | 300 | 300 |
| Loop lag p50 / p99 / max | ≤1 ms / ≤5 ms / 33 ms | ≤1 ms / ≤10 ms / 14.5 ms |
| Tick p50 / p99 | ≤5 ms / ≤50 ms | ≤10 ms / ≤50 ms |
| Critical writes committed / p99 / oldest pending | 13,666 / ≤25 ms / 17 ms | 13,172 / ≤10 ms / 5 ms |
| Unchanged autosaves skipped | 12,797 | 12,340 |
| Hash-slot wait / busy refusals | ≤1 ms / 0 | ≤1 ms / 0 |
| Output queue high water / overflow closes | 195/256 / 0 | 194/256 / 0 |
| Indeterminate writes / panics | 0 / 0 | 0 / 0 |

Channel 2 had one certified rejection at start: both processes wrote the guild registry header at the
same time, the version check refused the second write, and channel 2 reloaded the row (W9 phase 3
working as designed). Seven bot read timeouts and all 600 closes happened at the planned end of the
run, from the bots; the server logged no timeout or overflow close.

## Reports and acceptance checks

### BUG-001 — Attacker HIT/MISS feedback

- Report: the defender sees hit/miss feedback, but the attacker does not see it
  correctly.
- Findings: authoritative physical and skill result builders already clear
  `FlagLocal`. The client preserves negative MISS sentinels when reading wide
  damage and permits floating feedback for the local attacker and target.
  The server view helpers include an eligible attacker who has the target in
  visibility; the same result packet is sent to other eligible observers.
  Neither a local-flag change nor a speculative broadcast is a supported fix.
- Re-checked 2026-10-20: player-versus-mob and PvP misses go to the whole view
  (attacker included) as `0x39D/52` with `FlagLocal=0` and `Dam[0]=-3`; skill
  misses use the same sentinel. The client's `ApplyTargetedAttackHits` draws
  "miss" for a local attacker when `FlagLocal=0`. Monster misses against a
  player are sent as zero damage and draw no text by design. No static defect
  remains; a capture of one failing case (attack type, skill, target type,
  attacker/target positions) is needed.
- Next check: follow one actual result packet through delivery, target lookup,
  floating-text creation and rendering on both participants.
- Acceptance: attacker and target see the correct result for hits and misses,
  without duplicates or changing damage. Check a nearby observer separately
  against the established visibility policy.

### BUG-002 — Noatun gate state and collision

- Report: the gate closes unexpectedly and prevents normal return to town.
- Findings: no automatic close writer was found in the inspected server
  lifecycle. Castle key IDs 15/16 initialize open; client creation and
  incremental updates use different state encodings. The native 7.48 gate
  frame also closes locally when the *movement target* is over 33 tiles away,
  even if the player and gate are still adjacent. WYD-Go item visibility follows
  actual position at 32 tiles, so it need not resend an object closed this way.
  The client source now leaves gate state and collision to server packets and
  visibility removal. This is an intentional `MODERNIZACAO_COMPATIVEL`, not a
  native-parity claim. See the [gate evidence record](../.agents/research/client748/flows/gameplay/gate-target-distance-closure.md).
- Validation: `Build-Client.ps1 -NoDeploy` passed, including the source contract
  check; candidate SHA-256 is
  `A4C78CFA74786F6A9573B5887A09D73DDE37888873BB4A80E3A1F6AF3C40A5FC`.
  The installed `tmproject/client748/WYD.exe` is still a different build
  (`B20510B20D34651D206C72340B96D2B7314D56B19072AC25678D37183889DE3D`).
- Next check: in the exact installed candidate test a distant click while
  beside an open gate, actual view exit
  and reentry, relogin, and a legitimate war-related close. If it still fails,
  capture server gate state, client state, packets, and collision mask.
- Acceptance: the gate's presentation and passability agree with authoritative
  state, including after leaving/reentering view and reconnecting. Intended
  war-related closures must remain intact.

### BUG-003 — Hell Gate population visibility

- Report: killing the entry spider produces a remaining-monster counter, but
  searching the entrance finds no monsters and the internal portals do nothing.
- Confirmed causes: the entry and arena are disconnected terrain components;
  walkable spawn coordinates alone did not establish reachability. The client
  also ignored portal tiles absent from its named portal table. The former
  encounter skipped corridor bosses, used four minutes for both admission and
  combat, and finished by clearing a wave around `Aki` instead of defending Dionys.
- Native boundary: the 7.48 `FUN_004776c3` uses generic message row 43 for an
  unknown portal index; confirmation `FUN_004640e5` sends the ordinary 16-byte
  `0x290` request with zero reserved data. The client now follows that prompt
  path with bounded table reads. See the [portal evidence record](../.agents/research/client748/flows/transport/hell-gate-portals.md).
  This bounded prompt change is `PARIDADE_NATIVA`, with compatibility-safe
  guards. It does not establish native parity for the whole event.
- Encounter research: the [official Hell Zone guide](https://feedback.kersef.com/pt-br/knowledge-bases/21/articles/46666-hell-zone)
  describes preparation, three corridor bosses, four rooms, a selected Lich and
  protecting Dionys until the deadline. The official KR notices
  [1071](https://www.wyd2.co.kr/news/news_view.asp?cIdx=1071&nK=notice) and
  [1160](https://www.wyd2.co.kr/news/news_view.asp?cIdx=1160&nK=notice) address
  opening times and Dionys health, not a complete historical 7.48 encounter.
  None establishes the exact timers, populations, HP or reward used here.
- Explicit owner choices: twenty minutes total, including two minutes of
  preparation and eighteen active minutes; 10,000,000 completion EXP per eligible
  connected participant, subject to normal progression caps. Existing minute-0/30
  scheduling and ticket IDs 3328/3329 remain compatible. The guide's individual
  admission requirement is not proof that the existing party-ticket contract
  should be removed; that contract is preserved, not claimed as native parity.
- Published version-2 server reconstruction (`MODERNIZACAO_COMPATIVEL`, unchanged wire):
  version-2 configuration, three server-selected corridor bosses, four rooms of
  nine guards, one Lich per cleared room, and a single server-selected correct
  Lich. An incorrect boss/Lich does not permanently lock the remaining route.
  Three authoritative entrance portals become usable after the correct corridor
  boss dies. Destinations are reachable arena tiles and cannot cross instances.
  Dionys must remain alive until the absolute deadline; thirty attackers are
  replenished when cleared, without healing him. His death fails the encounter.
- Dedicated data: fifteen instance-only `NPCGener/HellGate.txt` blocks and five
  definitions: `HG_Spider`, `HG_Corridor`, `HG_Skeleton`, `HG_Lich`, `HG_Dionys`.
  Generator selectors, bounded placement, wire-name limits and native terrain
  reachability are checked. Occupying Dionys's anchor relocates him nearby
  without overlapping a player or crossing a wall. Persisted loader keys and
  `tipo` values are retained for compatibility.
- Balance is a project choice, not a native constant: the owner subsequently
  requested 400,000 HP for the entry spider, every corridor boss and every
  Lich; both maximum and initial HP now use that value. The owner's 10,000
  average magical damage means damage per cast, not DPS. The owner clarified a rotation of
  different spells every 900 milliseconds, not repeated use of one skill
  every six seconds. With an immediate first cast, each boss requires forty
  casts and approximately 35.1 seconds. This estimate does not change server
  attack intervals or individual skill cooldowns.
  Faster spell rotation, actual mana, resistance and movement can change those
  times. The worst random selection requires eight bosses including the entry
  spider (three corridors and four Liches), approximately 280.8 seconds of
  boss damage under that rotation, excluding guards and travel. This is not a
  measured FelipeTr gameplay run. Friendly Dionys has
  150,000 HP and 200 defense so defending him is not an AFK-safe minimum-damage
  exercise; contact pressure and travel still require gameplay validation.
- Persistence: protector HP survives restoration without healing/resurrection.
  EXP and the reward marker are committed together in synchronous and detached
  critical-persistence paths; failures roll back and retries cannot duplicate
  the reward. The committed marker survives the post-deadline exit grace.
  Completion EXP does not refill empty HP/MP. Reconnect and instance isolation
  retain the existing authoritative membership contracts.
- Delivery: source and focused automated checks; client candidate built with
  `Build-Client.ps1 -NoDeploy`. No local runtime installation, gameplay launch,
  commit or push was performed. Subsequently published as patch 10034 and
  activated in server release `20261017T221343Z` with authorized game/API
  restart. All 7,344 active payload hashes, executable identities, readiness
  and public delivery checks passed. Full Go tests/vet and portal build,
  60 tests and Astro checks passed; gameplay acceptance remains pending.
- Earlier integration validation: `go test ./internal/game ./internal/data -count=1` passed
  (10.563s / 2.306s); scoped `go vet` passed. These include reachable terrain,
  phase transitions, protector death/restore, deadline reward/rollback,
  occupied portals, instance isolation and detached persistence. The full-contact
  defense simulation applies the production formula to thirty installed
  Skeletons: deterministic first hit 114 and estimated survival 66 seconds.
  It deliberately excludes travel, line of sight and player intervention.
  The client build passed architecture (1,240,477 checks), socket receive (231),
  crash capture (83) and launcher (56) gates. Candidate path and hash are in
  [current state](../.agents/research/release-history.md#previous-release--2026-10-17-hell-gate-route-and-defense-patch-10034).
- HP/route follow-up: `go test ./internal/game ./internal/data -run '^TestHellGate' -count=1 -v`
  passed (game 0.622s, data 0.163s); output is in
  `build/hell-gate/route-hp-tests.log`. All three configured destinations belong
  to the same traversable arena component. Installed-data request tests cover
  each entrance portal during both populated room and protection phases,
  unoccupied arrival, unchanged membership/price and reachability of all actors.
  Both maximum and initial boss HP are checked; friendly Dionys is unchanged.
- Acceptance still pending: use the exact candidate in game, enter through the
  scheduled ticket flow, traverse all phases/portals, and test owner/observers,
  party visibility, reconnect, Dionys pressure, failure and deadline reward.
  Verify practical solo clear time and mana using FelipeTr's magical setup.

#### Historical correction — 2026-10-18, version 3

- Authority and scope: the owner supplied historical server material under
  `references/decompilacao server/`. The 7.59 folder contains
  `descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c`, SHA-256
  `F8C064D7B3E73600B244A743A1D3CC3F6E4DB66F2B44364367A2E1E825E664CD`.
  The version attribution comes from the supplied collection; it is not an
  independently matched 7.48 server binary. The supplied 6.13 material contains
  a symbol map and explanatory text, not enough implementation to reconstruct
  Hell Gate. This is not evidence that 6.13 lacked the event.
- Proven 7.59 sequence: `sub_578700` at `0x00578700` creates entry generator
  334, selects one of three corridor bosses (335–337), then creates room guards
  (342–377). It selects `first = rand() % 4` and
  `second = (first + rand() % 3 + 1) % 4`: two distinct rooms, including adjacent
  pairs, not one Lich or an always-opposite pair. The death progression inside
  `0x0057F530` releases each room's Lich (338–341) after its nine guard groups
  clear. Both selected Liches must die before Dionys (333) and the defense phase
  begin. Unselected Lich deaths do not permanently lock the event.
- Proven portal boundary: the historical `0x290` handler normalizes coordinates
  with `& 0xFFFC`. At X 1756, only the selected source at Y 3616, 3644 or 3672
  is accepted after the correct corridor boss dies. The other entrances stay
  closed. The existing 7.48 client request and prompt evidence above still
  apply; no new packet, client executable or asset is required.
- Implemented adaptation: installed instance data now selects version 3. It
  restores the distinct random Lich pair and selected entrance, consumes invalid
  instance requests before any global portal fallback, and retains exact runtime
  membership. Replayed deaths cannot count twice. Snapshots record the encounter
  version, selected corridor, pair and completed Liches. Unversioned published
  timed snapshots and explicit version-2 snapshots resume with their old rules;
  a restart neither rerolls their selection nor silently upgrades them. Invalid
  masks, invalid corridor selections and version-3-to-2 configuration downgrades
  are rejected before rebuilding the encounter.
- Historical populations, requested in the owner's follow-up: the bundled
  `DestinyServer-main/TMSRV/run/NPCGener.txt` supplies six corridor escorts
  (390–395), 36 four-member room groups (342–377, 144 guards), three mage escorts
  per Lich (338–341), twelve individual defense generators (378–389) and another
  24 ten-member groups (6023–6046), totaling 252 defense attackers. Leaders and
  followers retain their historical equipment distinctions. Dedicated template
  names retain their conservative 12-byte naming budget within the 16-byte NPC
  field (15 text bytes plus NUL). Spawn anchors are preserved with a
  bounded four-tile placement range; exact original dispersal is not claimed.
- Objective AI: version-3 defense attackers remain awake without nearby players,
  ignore the ordinary home leash, and march toward the live same-runtime Dionys
  using a cached terrain-aware reverse route. Dynamic occupancy delays movement
  rather than allowing passage through another actor. Each casualty queues its
  own original spawn anchor and NPC for replacement on the next event tick;
  temporary capacity/occupancy failure retries once per tick. Surviving mobs,
  protector HP and the absolute deadline are not reset. No replacement occurs
  after expiry or protector death. This cadence is an explicit bounded
  adaptation, not evidence for an exact historical generator interval.
- Dionys shop: head 51 restores the existing native 7.48 panel and 24-byte
  0x2BD request. Same-instance membership, visibility/range, protection phase,
  live tracked protector and deadline are authoritative purchase requirements.
  The supplied 7.59 functions and rate tables provide 20 weapons, 40 armor
  items, 14 accessories and their random options. The public merchant retains
  its earlier reconstruction. Gold/item changes and success packets use the
  critical persistence path; duplicate pending requests cannot reroll. See the
  [store contract](protocol/hell-gate-store.md) for exact evidence and limits.
- Lich visuals: `HG_Lich` now has exactly the face item of the existing
  `KR_Sand_Lich.json` definition: 4088 with effects `[43,7,0,0,0,0]`. The active
  item catalog gives class 57, mapped by `BASE_DefineSkinMeshType` to skeleton
  type 12; `mesh/BoneAni4.txt` selects `mesh/rc01`. All 15 existing `rc01`
  skeleton, animation, mesh and texture resources match the supplied
  `references/WYD_KR 7.70+/mesh/` files byte-for-byte. No historical binary was
  patched and no runtime asset was replaced. This verifies resource selection,
  not the rendered result; visual acceptance remains pending.
- Classification: `EXTENSAO_COORDENADA`, the requested historical 7.59 behavior
  adapted over the unchanged 7.48 wire contract. This is not `PARIDADE_NATIVA`
  for the complete event and does not supersede explicit owner choices.
- Preserved project settings: twenty minutes total (two admission, eighteen
  active), minute-0/30 scheduling, ticket IDs 3328/3329 and party admission,
  configured boss/protector HP and defense, existing purchase prices and the
  10,000,000 completion-EXP policy. The initial publication reused ordinary
  skeleton score/loot; the local follow-up below replaces those six templates.
  Existing deadline, failure, reward persistence and isolation checks remain in
  force. Published version-2 snapshots retain thirty attackers and their prior
  replenishment/puzzle behavior; new version-3 runs use the extended population.
- Historical differences not imported: 7.59 starts active phases around minute
  4, sends 1,500 seconds, and recalls players near minute 29 of each half-hour.
  The bundled generator/rate files have not been independently matched to the
  decompiled binary and are used as the owner-requested secondary 7.59 data.
  Their historical origin does not establish a native-7.48 server equivalent.
  Exact historical schedule, boss balance, occupancy fallback and replacement
  interval remain outside this adaptation. Infantry balance, formation offsets
  and Dionys sales are addressed by the local follow-up below.
- Validation: complete `go test ./... -count=1` and `go vet ./...` passed.
  Focused Hell Gate tests cover all twelve ordered pair draws, all selected
  entrances/phases, installed-terrain reachability, duplicate deaths, progress
  serialization, version-2 upgrade preservation and invalid restoration state,
  configuration budgets and empty groups, all 252 defense origins, full march
  to protector damage, occupancy waiting, replacements and terminal states.
  Store tests cover weight boundaries, deterministic options, valid catalog
  IDs, lifecycle/runtime refusals, save failures, detached proposals and
  deduplication. The full suite initially caught six oversized NPC names; these
  were shortened and the complete suite then passed without relaxing the limit.
  Logs are in `build/hell-gate-historical/`. Subsequently published with owner
  authorization in server release `20261018T171412Z`, with game/API restart,
  all 7,352 active payload hashes and readiness verified. No gameplay run,
  client build, commit or push was performed for this correction.
- Remaining acceptance: start a new version-3 encounter and verify entry,
  selected entrance, both selected Liches, observers/party, reconnect, Dionys
  survival/failure, approaching/replacement mobs, shop purchases in each
  category, the rendered Sand Lich and the existing deadline reward with the
  active release. The follow-up below has separate automated coverage and is
  now published, but has not been tested in the client.

#### Infantry, formation and Dionys sale follow-up — 2026-10-18

- Delivery: published in release `20261018T234132Z`, after explicit owner
  authorization for publication/restart. All 7,385 active payload hashes,
  running game/API executables, readiness and private configuration preservation
  were verified. Release `20261018T171412Z` predates this follow-up.
  Classification remains `EXTENSAO_COORDENADA`: historical 7.59 adaptation over
  unchanged 7.48 packets, not complete native-7.48 event parity.
- Six event-only templates now import the bundled `TMSRV/run/npc` binary data,
  including score, EXP, gold, equipment, Carry holes/options and skill bars.
  Ordinary field monsters and restored version-2 `HG_Skeleton` are unchanged.
  `cmd/runetrack759 -hell-gate` provides the repeatable importer; default
  Rune Track import behavior is unchanged.

  | Event template | Historical source | Level | HP | Attack | Defense | EXP | Gold |
  | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
  | HG_CorrSkel | Skeleton_ | 350 | 26,000 | 850 | 1,600 | 5,000 | 500 |
  | HG_SkelArch | SkeletonArcher | 399 | 24,000 | 3,600 | 1,820 | 5,500 | 500 |
  | HG_SkelMage | SkeletonMage | 399 | 32,000 | 3,600 | 1,850 | 5,500 | 500 |
  | HG_SkelWar | Guer_Caveira | 110 | 8,100 | 468 | 1,236 | 1,500 | 500 |
  | HG_SkelSpear | SpearSkeleton | 399 | 21,000 | 3,410 | 1,940 | 5,500 | 500 |
  | HG_SkelGuard | EsqGuerreiro | 399 | 22,000 | 3,600 | 1,700 | 10,900 | 0 |

- Version-3 owned `HG_SkelMage` uses the historical single-roll 50/35/15 skill
  windows and imported bar `[32,33,32,255]`. Damage uses monster Attack against
  defense and compressed elemental resistance, not player spell scaling or an
  added magic-attack score. Existing hit accuracy, melee reach, attack interval,
  terrain and runtime isolation remain in force. Players and the live tracked
  Dionys receive the supported skill packet. This is not full historical AI.
- Inline groups now use the twelve historical formation-zero offsets from
  `sub_514310` and `byte_63C300/301`. Leader/follower groups anchor followers on
  the actual placed leader. Separate follower-only groups retain their declared
  anchor. Occupied tiles still use the project's bounded placement fallback.
  Replacements retain each casualty's original formation anchor, not its last
  combat position. Group counts and the 252-actor defense budget are unchanged.
- Dionys and the public head-51 store accept the existing 0x37A sale request
  without an ordinary shop session. Full-width slot validation, authoritative
  interaction/lifecycle checks and existing critical persistence remain in
  force. Only Dionys applies the historical quarter-price adjustment. See the
  [store contract](protocol/hell-gate-store.md) for thresholds and economic limits.
- Preserved choices and gaps: event schedule/duration, calibrated boss/protector
  HP/defense, shop prices and completion reward are unchanged. Other boss stats
  are not imported. Exact historical generator-call/replacement cadence remains
  unproven; deadline-bounded next-tick replacement is still a project adaptation.
  A full historical balance/AI or gameplay-parity claim would be premature.
- Validation: complete `go test ./... -count=1` and `go vet ./...` passed after
  production changes. A subsequent test-only addition passed focused combat and
  sale tests. Coverage includes six data goldens, independence from field/v2
  monsters, Carry/options, formation anchors, caster selection/damage,
  player/protector targeting, member isolation, deadline expiry and sales/S2.
  Pending: actual client traversal/combat/purchases/sales. Publication and
  game/API restart are verified separately in the current-state receipt.
- Build/repository checks: `go build` for `./cmd/server` passed. The temporary
  candidate at `build/hell-gate759/wyd-server.exe` had SHA-256
  `A88B6B9E4A28CBA695966FD3D9B482112ADF28E13ADF6EB1F7F45D1B389D3478`;
  no installed runtime was replaced. Cleanup was blocked by execution policy;
  the candidate remains at that exact path.
  `git diff --check` passed. The layout validator remains blocked by preexisting
  `tmproject/TMProject748/build/`, a stack-overflow audit Markdown file under
  `tmproject/client748/dumps/`, and `crash-monitor-p31348.log` in that directory.
  Those unrelated files were preserved. Cleanup of the seven temporary JSON
  files under `build/hell-gate759/infantry/` was also blocked. The cleanup was
  not retried through another mechanism; source identities are recorded below.
- Source identity: `TMSRVIN.c` uses the SHA-256 recorded above. Binary NPC
  SHA-256 values, from the supplied historical `TMSRV/run/npc` directory:

  | Source | SHA-256 |
  | --- | --- |
  | Skeleton_ | `56b7d998bc04873662e16dc954d4feb73abbceed6ce32c8a3e98a96587042972` |
  | SkeletonArcher | `480f9de65a9e90bb23031af7a4bf55416774905408c053ce07308f4965525106` |
  | SkeletonMage | `a0183083eaf730b8adeb1c78ce740babda893d44093658961ee7b9b31aad2fcc` |
  | Guer_Caveira | `d5087d7f502e35fb896da8fc5385dbdbc8c91259ff1e603500f306d2102a846e` |
  | SpearSkeleton | `4b102e0109a30bdc3fb3708344c31f5f87775c61e56c62a08dfa43d82a58822f` |
  | EsqGuerreiro | `affa16d2802ba19b478e3d21434293bd06f328cc29bdcb1ead8e6b2de47e8db7` |

### BUG-004 — Blood Foema incoming damage

- Report: incoming attacks appear to deal full damage despite expected
  resistance/reduction.
- Findings: Mana Control (affect 18) currently absorbs up to 90% of incoming
  damage into MP at 1:1, after other mitigation; an MP deficit spills into HP.
  This is an existing project rule, not a proposed rebalance. Existing tests
  cover physical, magical, multi-hit, boss, summon and poison damage.
- Owner decision (2026-10-20): current Blood Foema behavior is correct; no
  correction or additional reproduction is requested for this report.
- Reopen only for a new concrete counterexample with the exact skill/affect,
  MP, HP, attacker and damage type.
- Acceptance: observed HP/MP changes and displayed damage match the established
  rule with full, partial and empty MP. Do not invent new balance values.

### BUG-005 — Blood Foema mana behavior

- Report: mana consumption/control appears inconsistent; the specific cast and
  resource sequence have not been identified.
- Findings: Mana Control does not discount casting costs. Holy Judgment's HP
  sacrifice bypasses incoming-damage absorption by design in the current rule.
- Owner decision (2026-10-20): current Blood Foema behavior is correct; no
  correction or additional reproduction is requested for this report.
- Reopen only for a new concrete counterexample with a named skill, active
  affects and before/after HP/MP values.
- Acceptance: casting costs and incoming-damage absorption each follow their
  own validated contract without duplicate resource deductions.

### BUG-032 — Stale player MP on incoming monster hits

- Report (2026-11-09). A level-400 Foema with Mana Control regained MP rapidly
  during Tauron combat with zero potions in the HUD.
- `AUTOMATED TESTED`. The saved equipment and the active release's item catalog
  reproduced 10,107 maximum MP. Mithril Robe Gloves +9 contributed 54 MP
  regeneration points. The [server pulse](../wydgo748/internal/game/regen.go)
  restores up to 4,558 MP every three seconds with those gloves, or 10 without them.
- Controlled hits of 461 and 500 after other mitigation consumed 414 and 450 MP.
  The server sent both deductions in its HP/MP snapshots. The next regeneration
  pulse restored the missing 864 MP. Keep BUG-004 and BUG-005 closed.
- `STATICALLY VERIFIED`. The [client snapshot handler](../tmproject/TMProject748/internal/game/entities/TMHumanPackets.cpp)
  stores maximum MP in `m_nReqMP`. The [incoming hit handler](../tmproject/TMProject748/internal/app/scenes/TMFieldSceneCombatPackets.cpp)
  can copy that value over current MP and refresh the HUD before the next snapshot.
  This also applies without Mana Control.
- Acceptance. Display each confirmed MP deduction until a later authoritative
  update changes it. Verify successive hits between pulses in the real client.

### BUG-006 — Divine Judgment visual effect

- Report: damage and HP cost occur, but the corresponding animation/effect is
  absent.
- Findings: skill 30 already creates `TMSkillJudgement`. Local requests dispatch
  a `FlagLocal=1` preview; authoritative local results suppress duplicate cast
  visuals. The native 7.48 attack dispatcher (`FUN_00489a3e`) allocates the
  corresponding effect for skill 30, and its constructor (`FUN_004e6590`)
  confirms the same 300 ms controller lifetime and base texture 124. The
  result gate and controller lifetime alone do not explain the missing effect.
- Next check: trace the reproduced local request/preview and effect ownership,
  lifetime and assets against native 7.48 evidence before changing dispatch.
- Acceptance: the proper effect plays through for the caster and observers,
  with one authoritative damage/resource application and no unrelated flash
  interrupting the cast.

### BUG-007 — Nightmare party state on entry

- Report: a character entering while already in a party temporarily appears
  ungrouped; green party-member identification is missing until a delayed update.
- Additional requirement: a Group Nightmare ticket must transport the eligible
  party together into the same runtime, preserving social membership. Individual
  tickets must not transport other party members.
- Expected: party identity is available as members become visible on entry.
- Cause: joining an already-open shared zone refreshed each player's view
  before all party spatial positions were updated, retained previous movement,
  and omitted the party roster refresh after actor publication.
- Implemented: shared-zone joins now use the same post-commit batch relocation
  as initial entry: clear previous movement, register every position, reconcile
  visibility, then publish party identity. Existing eligibility, shared-tier
  runtime, schedule, capacity and ticket rules remain unchanged. Classification:
  `MODERNIZACAO_COMPATIVEL`; no client packet change.
- Validation: the new regression reproduced stale movement on shared entry in
  Normal, Mystic and Arcane before the fix and passed afterward. Tests cover
  initial and later party admission, immediate bidirectional visibility and
  party roster delivery, unchanged deadline, solo tickets, and critical
  persistence commit/rejection without premature movement or publication.
- Delivery: source and automated validation only; no publication or restart.
  Reconnection and encounter HUD recovery remain separately tracked in BUG-008.
- Acceptance: enter N/M/A with an existing party and verify immediate party
  identification on both sides, without movement or a periodic refresh; repeat
  with another party entering an already-open zone in the installed client.

### BUG-008 — Nightmare reconnect synchronization

- Report: after a disconnect, the character returns inside the encounter, but
  party information, protected-NPC information and remaining time are missing.
- Expected: reconnection restores current authoritative party membership and
  encounter HUD state; it must not reset the timer or revive protected NPCs.
- Confirmed causes: character entry reattached the stable UID to its runtime
  without replaying the encounter HUD; disconnect removed social party
  membership without retaining a return identity.
- Implemented (`MODERNIZACAO_COMPATIVEL`): replay the existing timer/protected-NPC
  packets after EnterWorld and actor visibility. Restore the original party by
  stable character UID and exact live runtime/party identity when a slot is
  available; never evict another member or reuse a recycled actor ID. A
  disconnected leader does not displace the promoted current leader. Explicit
  party dissolution cancels restoration. Persistence acknowledgement precedes
  disconnect party cleanup; reserved party mutations defer restoration to World.
- Entry semantics remain distinct: a group ticket requires the leader, consumes
  only the leader's ticket and transports eligible members together; a solo
  ticket transports and charges only its user. Recovery charges neither.
- Automated coverage: leader/member reconnect with two/three-person parties,
  admission/combat deadlines, current protected-NPC losses, mutual actor
  visibility and party identity replay, full/dissolved parties, expired/replaced
  runtimes, recycled actor IDs, an entirely disconnected party, solo recovery,
  leader-only ticket consumption and critical disconnect acknowledgement.
  Run `go test ./internal/game -run '^TestNightmare(Reconnect|GroupTicket)' -count=1`.
- Scope: social party return hints are process-local, like parties themselves;
  this does not add party persistence across a server process restart.
- Delivery: source and automated checks only; no client build, publication,
  service restart or in-game confirmation in this change.
- Acceptance: after reconnect, both the returning player and connected members
  see correct party state; the returning player sees the actual remaining time
  and current protected-NPC state without waiting for a kill or moving. Also
  check expired instances and a party changed while the player was disconnected.

### BUG-009 — Magic Chamber disappearing and ghost monsters

- Reports: monsters appear late, disappear/reappear, sometimes never return even
  after movement, or exist only visually and cannot be attacked. All Carta
  difficulties and rooms were previously reported affected. The latest Carta N
  report includes monsters disappearing permanently in the final room, blocking
  completion.
- Previous partial correction: explicit room centers/bounds and full visibility
  reconciliation at logical room edges were published in server release
  `20261017T145456Z`; focused regressions passed. This is not proof that the
  reported encounter now completes. The latest report's exact running client
  and server versions have not been verified.
- Latest request: a party must enter and remain in one execution; individual
  entry must not pull other party members into that execution.
- Implemented: Magic Chamber now allocates an independent runtime identity per
  run, like private Water executions, instead of reusing the configuration ID
  or rejecting unrelated runs through a global occupancy lock. Exact runtime
  ownership still gates visibility, collision and targeting. This does not make
  Magic Chamber resumable after disconnect or change its ticket party policy:
  default tickets accept a solo player or the leader's party; an explicitly
  solo rule admits only its user, including a nonleader already in a party.
- Relocation: after an entry commit or room transition, all members' spatial
  indices advance before any visibility reconciliation. Old air/published
  movement is cleared so actor creation cannot replay a previous-room route.
  The existing party roster is replayed after visibility creation to restore
  immediate party identification. Critical persistence publishes nothing before
  the commit acknowledgement. No new client packet is introduced.
- Classification: `MODERNIZACAO_COMPATIVEL`; internal authoritative isolation
  and publication ordering, not a claim of new native 7.48 parity evidence.
- Validation: concurrent parties at identical room coordinates remain isolated,
  preserve mutual visibility through a room transition and target only their
  own monsters. Solo admission while socially grouped stays separate; commit-
  acknowledged group and solo entry paths are covered. The complete
  `internal/game`, `internal/data` and `internal/model` test packages passed.
  The obsolete data assertion requiring a global Magic Chamber lock was updated
  to require independent executions while retaining all four rooms/final boss.
- Delivery: source and data only; no candidate build, publication, restart or
  in-game execution. The earlier disappearing-monster report remains open until
  the original encounter is verified in gameplay; ground markings are BUG-010.
- Next check: verify artifacts/version and reproduce the original room flow;
  compare live server actors, membership/bounds, visibility delivery and client
  actor lifecycle. Distinguish invisible live mobs from stale client-only actors.
- Acceptance: complete every configured room and the final room in Carta N/M/A
  without an unattackable live remainder. Include stationary viewing, movement,
  room transitions, repeated entry, and party participants; visibility repair
  must not create duplicate actors, rewards or kills.

### BUG-010 — Magic Chamber ground markings

- Report: colored runes appear at random on the floor near the Magic Chamber
  entrance in the native client, but are missing from the current client.
- Findings (2026-10-20, static): the historical secret-room ticket is accepted
  only at `(1045..1047, 1689..1691)` (7.59 `TMSRVIN.c`), so the entrance is in
  `Field0813`; the rooms are in `Field0628`. Every object type near the entrance
  (503 braziers, 1596 at the exact entrance tile, 1585–1647, 1697, 658) and in
  the rooms (6/7 butterflies and bats, 532, 1520, 1540–1542 wall glows) takes
  the same branch in the native loader `FUN_00541065` and in
  `TMObjectContainer::Load`. The native field frame (`FUN_004776c3`) only hides
  a control in zone (6,28) and spawns ambient particles elsewhere; the
  environment-effect consumer `FUN_00489618` knows only effect 32. No random
  floor-rune generator was found in the decompiled corpus.
- Correction (2026-10-21, owner capture): the runes are the Rune_Slate
  objects (items 1733–1736, meshes 989–992, `EF_GROUND` 6), already in the
  native 7.48 `InitItem` rows 20–35 and in `data/init_items.csv`: one 2x2
  block per room at (786,3676), (844,3676), (844,3620) and (786,3634). The
  7.48 client (`TMGate`, special case for meshes 989–992) sinks a slate below
  the floor while open and shows it with its glow while closed/locked. WYD-Go
  created them open, so they never appeared. The 7.59 TMSRV rolls a color 1–4
  when each room starts (`sub_561F90`) and shows in every block cell k as room
  k's rune (item 1732+color, state 3) (`sub_561E80`), so up to four runes of
  random colors appear. Implemented per run (`magic_chamber_runes.go`): the
  rune is rolled once per room when the stage starts, members see their run's
  lit slates (locked, height 0, no collision), and other viewers keep the open
  fixtures; already-visible slates are re-sent after a roll. Test:
  `TestMagicChamberRuneSlatesFollowTheViewersRun`.
- Rune bonus boss (owner request, 2026-10-21; 7.59 `sub_561F90`, generators
  97/123/140): the leader's Voucher_Courage (1732, slot 10) limits the roll to
  colors 1–3; without it the Arcane chamber (ticket 1731) always rolls 4 and
  Normal/Mystic roll 1–4. When the fourth room and its completion boss are
  cleared and the runes include colors 1, 2 and 3, `runeBonusSpawns` summons
  one bonus boss at (806,3621) and the run finishes when it dies. Mobs, with
  the 7.59 appearance and the chamber's EXP-only rules (1 damage, no drops),
  3x the room-4 boss HP and 2x its EXP: Normal `MCN_CruelZak` (CruelZakum,
  3,000 HP, 400,000 EXP), Mystic `MCM_Gargoyle` (WiseGagoilKing, 12,000 HP,
  600,000 EXP), Arcane `MCA_Rujeper` (RujeperKing, 30,000 HP, 1,000,000 EXP).
  The inert 7.59 HP marker item 1936 was removed from their equipment. Tests:
  `TestMagicChamberRuneBonusBossFollowsTheCompletionBoss`,
  `TestMagicChamberWithoutTheThreeRunesFinishesAfterTheBoss`,
  `TestMagicChamberRuneRollRules`, `TestMagicChamberRuneBonusBossesAreConfigured`.
  Run state is process-local: a restart forgets the rolled runes.
- Native mechanism (2026-10-20, Ghidra over the whole binary): the only users
  of the rune textures `pattern01..04` (effect rows 91–94) are `TMSkillFlash`
  (`FUN_004e3610`), the per-character affect auras (`FUN_00506f9d`) and
  `TMSkillTownPortal` (`FrameMove` `FUN_004eeb0d`, texture 94, 700 ms, 1.5).
  The teleport handler (`FUN_005284a0`) creates one `TMSkillTownPortal`
  (`FUN_004ee730`) at each arriving character's destination before `Warp`.
  The runes seen at the Carta N/M/A arrival are these one-second arrival
  portals, one per member, at the members' spread positions; that is why
  their number and placement vary. WYD-Go already sends `0x366` Effect=1 for
  every Carta entry and room move (`publishInstanceMemberRelocation`, ticket
  entry), and `OnPacketIllusion` creates the same portal; source and native
  match, including the pre-`Warp` height lookup.
- Correction: the owner-requested hall generator shipped in patch 10037
  (`EntranceRunesPolicy`, Field0813) is not native. It used wrong texture
  rows (82–85, counted names instead of the 264-byte rows) and drew white
  squares. It is removed in source; the next client patch must ship the
  removal.
- Next check: identify the native 7.48 floor effect and its random trigger,
  assets, position and lifetime. Keep this separate from monster visibility unless a
  common cause is demonstrated.
- Acceptance: the colored runes appear at the native entrance location with
  the proven random timing and are removed at the proper lifecycle boundary.

### BUG-011 — Friendly Compounder and city guards

- Report: Compounder attacks nearby players; city guards must attack monsters,
  never players.
- Cause: Compounder, Guard and Guard_ definitions were classified as ordinary
  hostile monsters, so they entered player-targeting AI.
- Implemented: Compounder is a passive NPC. Guard and Guard_ are friendly NPCs
  with an explicit `city_guard` combat role, not a role inferred from their
  names or appearance. Their server AI targets only live public hostile
  monsters within the existing acquisition, attack and home-leash limits.
  Player summons, private-instance actors, friendly NPCs and guild-war towers
  are excluded. Ordinary player-damage boundaries also reject friendly NPCs,
  even if a stale target survives. Guard kills grant no player loot or EXP.
- Compatibility: legacy NPC JSON keys and persisted type values are retained;
  only the three specific definitions change classification. Monster balance,
  appearance and other guard-named merchants/dungeon monsters are unchanged.
  Classification: `MODERNIZACAO_COMPATIVEL`.
- Validation: actual definitions load as friendly roles; tests verify guard
  acquisition/monster damage, player immunity through direct stale-target
  calls, excluded targets, passive Compounder and guard activation without
  nearby players. The complete game/data/model test packages passed.
- Delivery: source and data only, not published or gameplay-tested.
- Acceptance: approach Compounder and every affected city guard with two
  players, including after reconnect; neither player takes NPC damage. Bring
  an eligible hostile monster into guard range and observe pursuit, damage,
  death and return to guard duty without player rewards.

## Validation and closure

The preceding diagnosis ran existing focused Go tests for Hell Gate progression,
restore, collision isolation and entry-window lifecycle; Mana Control resource
splits, depletion, casting costs, combat paths, mounts and lethal cases; and
physical double/critical MISS encoding. They passed but did not reproduce or
close these player reports. No gameplay fixes or new build were produced by
that diagnostic batch.

For each issue, record source changes, focused regression results, candidate and
installed artifacts, publication/activation, and actual gameplay evidence
separately when available. A deployment or successful build alone cannot close
a visual or encounter-completion report. Keep unresolved items open and update
this file instead of creating another parallel bug list.

<a id="additional-player-reports"></a>
## Additional reports and correction history — October 2026

Merged from the former additional-report list. Statements about pending work and delivery describe their recorded dates. They do not reopen owner-accepted BUG IDs or establish current status for the other reports.

<a id="additional-player-reports-additional-player-reports"></a>

This dated appendix retains additional requests and correction evidence. BUG-001–BUG-011 use the status table at the start of this register.
The feature rows below retain their own dates and do not assert current delivery.

- Party request stops the C.C. macro (reported 2026-10-23, pending
  reproduction): with the macro attacking, receiving a party request makes
  the character stop attacking. Static review of the client request handler,
  the `-938` notice panel, the macro loop and the server invite state found
  no cause (see [current state](current-state.md)). Next step: reproduce
  with `WYD748_TRACE_PACKETS=1` set before starting `WYDLauncher.exe` and
  compare the outgoing attack opcodes in `client-debug.log` before and after
  the incoming `0x37F`.

<a id="additional-player-reports-status--2026-10-20-patches-1003710038-published-gameplay-pending"></a>
### Status — 2026-10-20 (patches 10037–10038 published; gameplay pending)

| Report | Status |
| --- | --- |
| Experience messages | Already available: typing `exp` in chat toggles the XP chat lines (`FieldChatSubmitPolicy`). |
| Ctrl + right-click equip | Already implemented (`ItemShortcutPolicy`, Action::Equipment). |
| Huntress Backpack (85) | Implemented: the skill opens Cargo locally from anywhere; the server accepts Cargo moves and gold without a banker for a living Huntress who learned it (`hasBackpack`). The server no longer treats 85 as the W2PP gold shield. |
| Coin Armor (86) | Gold cost (Trade x 100) moved from 85 to 86; defense Trade/10 per the 7.48 skill help; C.C. rebuffs 86 instead of 85. |
| Enchant Frost (74) with claws | Owner rule: rejected with a claw on client and server; Hide in Shadow (75) and Another Soul (79) no longer require a bow. |
| Celestial conversion buffs | Item buffs (potions, food, Sephira) survive Celestial creation and form changes; skill buffs stay with their form. Account-wide buffs are a new mechanic awaiting approval. |
| Guild leader medal (`guildoff`, `"`) | Already implemented as in W2PP (`guildoff`/`guildon` mask guild and rank for self and observers); needs a reproduction. |
| Greater/Lesser Exchange Stone (774/775) | Implemented from the 7.59 DBSRV (`FUN_00471210`): with 774 and 775 in `Equip[13]` of two characters of the account, the next login swaps their names and removes both stones. Guild members must leave their guild first. |
| Adamantite (E) → (LE) | Owner rule implemented: only D sets (uniques 8/17/27/37) can become Legendary; E sets (10/20/30/40) are rejected without consuming the ore. |
| Blood Foema | Owner confirmed the current behavior is correct; no correction needed. |
| Magic Chamber floor runes | Native Rune_Slate objects lit per run, plus the 7.59 rune bonus boss (colors 1, 2 and 3 summon MCN_CruelZak/MCM_Gargoyle/MCA_Rujeper after room 4); see BUG-010. |
| Divine Judgment | See [bug register](bug-register.md); identify the native visual trigger and assets before changing the client. |
| Black Foema Lightning, Noatun gate | Lightning Storm tick damage and the client's local gate close are corrected in source; automated checks passed, but gameplay acceptance remains pending. |
| Offline shop honor | Owner rule implemented: an open ghost shop earns its owner 1 honor every 10 minutes, with the letter "You received 1 honor." The counter is persisted before the schedule advances or the letter is sent; a failed save rolls the counter back and retries on the next tick. Source and automated tests only. |

AUTOMATED TESTED (2026-10-20): `go vet ./...`,
`go test -count=1 -p 1 ./...`, the portal's 60 tests, and
`Build-Client.ps1 -NoDeploy` passed after the boss-data corrections
(3,282,438 architecture checks). The candidate and public patch 10038
executable SHA-256 is
`B2417172F15E968E26A6F6B1BB8451F2CE7125476000CE3295079EE01FF54A27`.
The server is active at `/srv/wyd-go/releases/20261020T230245Z`.
Local installation and actual gameplay/`CLIENT_TESTED` remain pending.

<a id="additional-player-reports-follow-up--2026-10-20"></a>
### Follow-up — 2026-10-20

- Repliction descriptions: all ten distributed 7.48 `itemhelp.dat` records
  (4016–4025) contain GBK Chinese text that the current client renders as
  corrupted characters. The client now replaces only these records at load
  time with English descriptions of the current server behavior: matching
  grade, armor-only use, replacement of both bonuses, preservation of
  refinement, and the normal/premium +6/+9 limits. The original asset is
  unchanged. Architecture regression checks cover all ten headings, both
  limits, and tooltip line widths. Source only; visual acceptance pending.
- Rake pants 30 damage/7 critical: the owner's estimated item is Rake pants
  +10, but no item capture is available and the reported 25.3%/15.4% values
  match a +11 multiplier in the current formulas, not +10. The owner thinks
  this may already be corrected. Do not change refinement or critical
  arithmetic without a fresh reproduction identifying the actual item,
  refinement, and option bytes.
- Grade-E Repliction appearance: the distributed 7.48 `ItemList.bin` maps
  item 4020 to the grade-D mesh 2837; item 4025 already uses the distinct
  grade-E mesh 2860 (`smel05.msa`). The client loader now maps 4020 to 2860
  when it encounters the old row, and the server catalog agrees. The loader
  regression test covers D, E, and permanent E. This is a coordinated source
  correction; the shipped binary asset is unchanged, and the rebuilt client
  and in-game appearance still need validation before publication.
- Glory Ankh critical at +9: the server now applies the client-source
  position-mask (`Pos & 0xF00`) effective refinement promotion to item
  attributes without changing stored refinement. The catalog's base 5% now
  becomes 10% at +9 instead of 9.5%; ordinary equipment keeps its existing
  calculation. `TestAccessoryPositionRefinementBonus` covers +8, +9, +10,
  +15, and a non-accessory control. This is supported by the current client
  source and the reported expectation; native 7.48 runtime validation and
  in-game acceptance remain pending. Source only, not published.
- Tiny weapon option transfer: the fixed 100% chance was replaced in server
  source with the donor-ID/refinement table recovered from the bundled 7.59
  `TMSRVIN.exe` decompilation. Opcode `0x3C0` calls the `0x4B24C3`
  jump thunk, which targets the table reader at `0x4DB0C0`; the same handler
  tests `rand() % 100 <= threshold`. The binary SHA-256 is
  `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642`.
  Effective chances are 26/38/43%, 30/42/48%, and 34/46/54% for the three
  donor-ID tiers at +9/+10/+11 or higher, respectively. The effective values
  include the native `rand() % 100 <= threshold` comparison. Unsupported
  donors and donors below +9 are rejected; the target and catalyst must be
  exactly +9. `TestTinyChanceFromServer759Table`,
  `TestTinyRecipeUsesDonorTierAndRefinement`, the Tiny persistence tests, and
  `go test -count=1 ./internal/game` passed. This is a requested adaptation
  from later-server evidence, not proven native 7.48 parity. It is source-only,
  unpublished, and pending in-game verification.
- Eighth skill fee (owner correction): Mortal 5,000,000 gold; Arch
  50,000,000 gold; Celestial/SubCelestial 100,000,000 gold. The server now
  validates and deducts the fee by evolution for any branch's eighth skill.
  `TestUltimateSkillCostByEvolution` covers insufficient and exact balances;
  `go test -count=1 ./...` passed. Source only; not published or
  tested in game.
- Master capes (Celestial/SubCelestial): generic refining powder is rejected;
  their dedicated Odin composition remains available. Focused normal and
  critical-persistence tests passed. Source only.
- Capture lineage (owner correction): the fourth mastery grants neither HP nor
  defense. The earlier 2 HP/0.5 defense per point (and the interim 3 HP/1
  defense) was the reported bug and is removed; TK magic still uses the higher
  of the second and fourth masteries. `TestFourthMasteryAddsNoHPOrDefense`.
  Source only.
- Uxmal position: the NPC generator moved from (1057,1717) to the Rune Track
  entrance (3297,1693), as in the 7.59 `NPCGener.txt` generator 5661.
  `TestUxmalStandsAtTheRuneTrackEntrance` checks the position and walkability.
  Source only.
- Rune Track admission correction (2026-10-21): an entry with no eligible
  party member now remains in `entry` instead of advancing to `preparation`.
  `TestRuneTrackEmptyEntryStaysPending` covers the fail-closed path. Source
  only.
- Limit test item (owner request, 2026-10-20): `Limit_Test_Amulet` (6015)
  reuses the Crystal Amulet (563) appearance, has no level requirement, is
  untradeable (`EF_NOTRADE`) and adds 100000 STR/INT/DEX/CON in the server
  catalog. ShopAmulet sells it for 1,000 gold. Test:
  `TestLimitTestAmuletAddsAttributesAndIsSold`. Remove it after the limit tests.
- 32-bit limits (`EXTENSAO_COORDENADA`): the client catalog effect value is now
  32-bit in memory (`STRUCT_STATICEFFECT`, runtime `STRUCT_ITEMLIST` 204 bytes)
  while `ItemList.bin` keeps the native 140-byte rows. `ItemListWide.csv`
  (`index,effect id,value`) carries every server catalog value outside 16 bits;
  `TestItemListWideMatchesServerCatalog` keeps both sides equal, and the
  client loader rejects malformed files atomically. The client build and patch
  builder also fail closed when this required sidecar is absent. Audit: the
  server score is
  35 uint32 fields (cap 2,000,000,000), HP/MP travel in 32-bit `SetHpMp`, and
  real damage in the 32-bit DMGX tail. The client's last 16-bit narrowings were
  removed: local pending HP (`m_nReqHP`, which truncated HP above 65,535 and
  capped heals at 100,000) and the big-HP-pool `short` cast. The floating hit
  number no longer disappears above 1,000,000. Heals now carry their real
  32-bit amount in the DMGX slot (`wideTailValue`, client `ReadHeal`) instead of
  the 32,767 prefix word. EXP is uint32 on both sides with no server cap; the
  gain text no longer passes a 64-bit value to an `int` format. Remaining
  16-bit projections are native display packets only: party HP (`/100` above
  32,000, ratio preserved) and the Spirit's Seal capsule's stored base stats
  (32,767).
- World bosses moved (owner request, 2026-10-20): Black Archmage (3910,3418),
  Guardian of the Ruins (3806,4019), Demon Lord (731,3173), Ancient
  Necromancer (2083,347). Four Armia NPCs at y=2095 (ArchmageGate 2093,
  GuardianGate 2096, DemonGate 2104, NecroGate 2107) clone the Gravedigger
  confirmation window and run free repeatable quests 19–22 that teleport to
  the entrances (3893,3418), (3792,4006), (711,3169) and (2087,328). The boss
  files' comments are translated. Test: `TestBossGatesTeleportToTheBossEntrances`.
- Gate audit (2026-10-21). `init_items.csv` row = 7.59 `g_pInitItem` index - 1.
  Startup rule here and in W2PP: KEYID 1–14 locked, everything else open.
  | Rows | Gate (item, KEYID) | Key / controller | Expected (W2PP, 7.59) | Current |
  | --- | --- | --- | --- | --- |
  | 46–48 | Training field gates (458/459/461, 2–4) | First/Second/Final_Gate_Key 451–453 | locked; key opens; relocks | fixed: relocks 10 s after opening (owner rule), never on a player in the mask |
  | 49–52 | Orc castle gates (462–464/468, 5–8) | Orc castle keys 465–467/469 | same | fixed: 10 s relock |
  | 53–61 | Zakum castle gates (757–761, 10–14) | Dragon_Emblem 751, Phoenix_Emblem 752/754–756 | same | fixed: 10 s relock |
  | 45 | South_Gate (457, 1) at (64,64) | no key | locked map-corner dummy | locked (correct) |
  | 0–11 | City-war arena gates (471, 9), 3 per city | `SetArenaDoor` (city 0–3: Armia, Azran, Erion, Nippleheim) | locked; open while the battle runs | fixed: open from +6 to +31 min of city-war combat, locked otherwise |
  | 12–13 | Colosseum Steel_Window (472, 9) | `SetColoseumDoor`, Battle Royale | open at startup, locked by the event | fixed: open at startup; the Colosseum locks them from minute 5 to 17 (no Battle Royale event yet) |
  | 14–18 | Colosseum gates (471, 9) | `SetColoseumDoor2`, Colosseum event | locked, opened by the event | fixed: Colosseum opens them with wave 1 at minute 6 and locks them at minute 17 |
  | 35–40 | OX quiz gates (471, 9) | 7.59 `sub_561C10` | locked, driven by the OX event | fixed: staff panel "OX arena" (`game.event.ox`) locks/opens them |
  | 41–44 | Noatum castle gates (773/800, 15/16) | `SetCastleDoor`, castle war | open; locked during the castle war | always open (no castle battle) |
  | 66–70 | Victory's_Tower (3145) | 7.59 init (`EF_GROUND` 8) | 7.59 locked; W2PP open | open (sunk) — confirm 7.48 |
  | 71–72 | Gate_Of_Time (4101, 19) | 7.59 init + `sub_578CE0` | 7.59 locked; not in W2PP | open — confirm 7.48 |
  | 73–74 | Slot machines (4102/4103, 20) | `gamble.go` | open, interactive | working |
  | 19–34 | Rune slates (1733–1736, 17) | Magic Chamber | per-run lit slates | working |
  | — | PvP door (4143 First_Gate, 21) at (2123,2143) | 7.59 GM `pkgate` (1 locks, 0 opens) | open; closed only by a GM | fixed: server-only fixture, staff panel "PvP door" (`game.world.pvp-door`) |
- Colosseum (7.59 TMSRV `sub_567780`, 2026-10-21): every day at
  EventHour+1/+2/+3 (`colosseum_event_hour`, default 17, guild-war timezone),
  which gives N/M/A starts at 18:05/19:05/20:05 by default.
  Colosseum N (mortals only), M (no Celestials) and A (one Elixir_Notice 3170
  each from the 60 scanned carry slots, `_NN_Need_Elixer` and recall
  otherwise; a pending or rejected charge is retried every tick) run from
  `data/NPCGener/Coliseu{N,M,A}.txt` with the 16 `COL_*` monsters. Minute 5
  locks the steel windows and filters the arena, waves spawn at minutes 6, 9,
  11, 13 and 15 (3–5 GenerateMob calls per label). Owner rule (2026-10-23):
  the inner gates open in the same minute-6 transition, before wave 1 spawns.
  Minute 17 moves players out to (2615,1727) (the dead are
  revived and their corpse removed, through the critical persistence
  coordinator in production), reopens the windows, locks the gates and
  deletes the event mobs. N/M keep recalling ineligible classes that walk in,
  skipping players with a pending write. The base NPCGener arena spawns
  (MinuteGenerate -1) are reserved for the event; the generator files load
  only while `colosseum_enabled` is true. Project choice: a server that misses
  minutes 5–6 skips that hour instead of firing every missed wave at once. The
  daily reset runs before the tier guard, so the completed stage cannot leak
  into the next day's first scheduled tier. The staff panel can advance a
  selected tier with `game.event.colosseum`; the same admission, elixir,
  wave and exit rules run for 12 minutes from the early start. An early start
  replaces that tier's scheduled session for the same local day in the current
  process. Day claims and operation receipts are not durable across restarts.
  Active events, pending player exits, claimed tiers and passed schedules block
  a new start; epoch/revision checks and receipts fence stale or repeated
  commands. Owner rule (2026-10-23): all event-generator monsters in N/M/A
  deal at most 1 damage per physical or spell hit, before player absorption;
  zero-damage results remain zero. Other monsters sharing those templates
  retain their damage. PvP retains the existing PK-mode requirement. These
  internal server adjustments are `MODERNIZACAO_COMPATIVEL`, not a native
  7.48 parity claim, and require no client change. Tests: `TestColosseum*`.
  Source and automated tests only; deployment and gameplay validation pending.
- OX arena (7.59 `quizmode`, `quizdoor`, `quizresultx`, `quizresulto`): staff
  panel "OX arena" (`game.event.ox`). "Start OX event" enables quiz mode and
  opens all six gates in one World command. Starting an already enabled event
  does not reopen gates locked for an active question. OX has no automatic
  schedule. With quiz mode on the Entrance_Ticket
  1737 is spent and teleports to the `quizArena` (1340,1492) of its
  instances.json rule 51; with quiz mode off it keeps this
  project's Big Cube ticket (instances.json rule 51) instead of the 7.59
  `_NN_Only_quiz_time` reply. Results move the losing side to (1341,1465) and
  revive the dead there. Tests: `TestOX*`, `TestOXEventHTTPAuthCSRFAndRevocation`.
- Elixir_Notice (3170) shared volatile 91 with Mount_Recovery_Potion, so using
  it raised an equipped mount to level 100. It is now `no_direct_use` (server
  rule and client route `Interaction`).
- Throwing Knife (94): the current skill catalog and client/server cooldown
  paths both use five seconds. The reported +9 glove interaction still needs
  native 7.48 verification; no cooldown change was made.
- Earring +10: the current server source uses a 16% success chance. The +11
  chance remains unverified and must not be treated as a native value.
- Mana Control: the owner keeps the 90% MP absorption rule.
- Lightning Storm (37): each 8-second tick now hits as a Thunderbolt (33) cast
  by the owner (W2PP ProcessAffect Type 22), instead of the bare affect value
  (about 300). Test: `TestLightningStormTickHitsAsThunderbolt`.
- Native client checks (7.48 Ghidra): the `guildoff` control 299 and the
  `'`/`"` key match the native (`FUN_004662c5`, `FUN_0045288b`). Holy
  Judgement (30) uses the native TMSkillJudgement type 0, constructor,
  destructor and visual gate (`FUN_00489a3e`, `FUN_004e6590`, `FUN_004e74b5`,
  `FUN_004e754b`); no static defect found, an in-game capture is needed.
- Noatun gate lead: `TMGate::FrameMove` marks a gate closed and unauthorized
  beyond 33 tiles and applies closed collision until the server resends it;
  server item view range is 32. The 7.48 native export confirms that local
  behavior. Source correction removes only the target-distance close/delete;
  authoritative gate updates and visibility removal remain intact. The
  `-NoDeploy` client build and source contract test passed; the candidate
  `tmproject/build/TMProject748/Release/WYD.exe` has SHA-256
  `A4C78CFA74786F6A9573B5887A09D73DDE37888873BB4A80E3A1F6AF3C40A5FC`.
  It was not installed or tested in game; see the [gate evidence record](../.agents/research/client748/flows/gameplay/gate-target-distance-closure.md).


### November 2 mount and crash evidence

Correction receipt: `tmproject/build/mount-crash-20261102/REPORT.md`.
The original November 2 EXE/PDB pair is preserved beside the report.
The October 30 image was recovered from `build/patch/patch-48016.zip`.
Its RSDS, timestamp and image size match the dump. Static calls and the saved
TMScene vtable identify recursive retirement without substituting current symbols.
The new Release client is installed locally. Go tests, vet and focused race checks pass.
The server correction is built locally; no account change or server restart occurred.
Food points decrease hourly. C.C. can use inventory feed more often to heal damage;
inventory feed consumption and the food-point timer are different operations.
