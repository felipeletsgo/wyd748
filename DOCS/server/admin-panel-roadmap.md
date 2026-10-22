# Staff panel roadmap

Single implementation plan for the embedded staff panel (`/admin`). It merges:

- the staff operations delivered on 2026-10-21 ([staff operations](staff-operations.md));
- the remaining items of the first panel review (economy, support, world
  controls, observability);
- the external MMO panel research (features F01–F36, kept in the maintainer's
  Notion workspace), after checking its claims against the current tree.

This document is a plan. Nothing below is implemented unless the baseline
section says so.

## Baseline after 2026-10-21

Already in the tree (do not rebuild):

| Area | Existing implementation |
| --- | --- |
| Staff auth | Game account password + six-digit PIN, Turnstile on the public origin, capability file re-read per request, CSRF, rate limits |
| Live reads | Overview with one admitted World read at a time (`controlPending`), 50-player pages, account presence, persistent player snapshot |
| Moderation | Kick, city teleport, persistent ban and chat mute with history and revocation, staff message to one session |
| Communication | Broadcast notices now, delayed or repeated (process-local) |
| Operations | Scheduled shutdown/restart with countdown, final-minute login gate, SIGTERM-equivalent drain, exit `75` for `Restart=on-failure` |
| Audit | Structured stderr log plus PostgreSQL `staff_audit` history with a paged viewer |
| Events | Global drop, quiz, bosses, PvP door, OX, Colosseum, premium shop, donate credit |
| Metrics | `expvar` counters/gauges and fixed tick/command duration buckets; loopback-only `/debug` |

Corrections to the research notes:

- PostgreSQL schema is v9 (sanctions and staff audit), not v8.
- Ban/mute, broadcast and scheduled maintenance are no longer gaps. F07, F08
  and F24 only keep their remaining parts below.
- F24 says a panel must never stop the process without an external
  coordinator. The delivered design keeps that intent: the panel only arms a
  World countdown, the drain is the same path as SIGTERM, and systemd decides
  the restart. What is still missing is the post-restart readiness and
  version receipt (stage 2).
- `DOCS/CRAFTING.md` does not exist; crafting rules live in
  `wydgo748/internal/game/crafting*.go`.
- There is no loop-lag histogram yet, only last/max values. The research's
  p95/p99 lag gates cannot be measured until it exists (H5).

## Performance model

These facts decide where each feature may run.

- The World is one actor with a 500 ms tick. Anything it executes delays every
  player. `Session.Send` builds and encrypts on the caller goroutine, so a
  broadcast costs the World O(online players).
- Measured on the development machine: building and encrypting one staff
  notice pair (top notice + chat line) costs about 0.28 µs. One broadcast to
  1,000 players is therefore about 0.3–0.5 ms of World time including channel
  sends; 32 notices due in the same tick would be about 10–15 ms. These are
  development-machine figures, not VPS measurements.
- The panel runs inside `wyd-game.service` (`CPUQuota=150%`,
  `MemoryMax=1200M`). HTTP goroutines do not block the World but consume the
  same CPU quota and garbage collector.
- The panel's read-only PostgreSQL pool has 2 connections; the game pool has
  8. Both share the same server, cache and disk with autosave and critical
  writes.
- Every persisted audit row is a write. Read-only panel requests are audited
  today, so an auto-refreshing page would turn into steady write traffic.

Cost classes used in the decision table:

| Class | Where it runs | Allowed in production |
| --- | --- | --- |
| A | Build time or the browser (static JSON from `cmd/web-export`) | Yes |
| B | Panel HTTP goroutine + bounded PostgreSQL read | Yes, with limits and timeouts |
| C | Bounded World read through the single admission gate | Yes, cached 5–15 s, refused when busy |
| D | World mutation + critical persistence | Yes, rare, typed, idempotent, audited |
| E | Separate process (helper service) or local tools | Helper only; tools run locally |

Rules carried from the research cost audit and kept as non-negotiable:

1. Only the World mutates gameplay; the panel sends typed commands.
2. No SQL, file I/O, journal reads, network calls, simulation or large
   serialization on the World goroutine.
3. Static game data stays in `data/`; editors produce diffs for review and
   deploy, never live catalog mutation.
4. Optional panel reads back off (`429`/`503`) when World lag, queue age or
   persistence backlog exceed thresholds.
5. Load tests, simulators and editors run on the local machine, never on the
   production VPS.

## Stage gates

The server is an open beta without a staging environment (decision 3).
Load comparisons run on the maintainer's machine (local PostgreSQL 18, local
`tm.exe`, a disposable `wydgo_loadtest` database and `cmd/loadtest` as
documented in [operations](operations.md#run-the-load-test)) and are required
only for features with World cost (classes C and D). Their absolute numbers do
not predict the VPS; only the with/without comparison on the same machine is
a gate. No load test runs on the VPS.

A stage is closed only when:

- Go tests, `go vet`, portal check/tests and the affected PostgreSQL
  integration tests pass;
- for class C and D features, the local load comparison (same
  scenario and bot count with and without 1–3 staff sessions using the new
  features, three runs each) shows p99 loop lag within baseline +5 ms, p99
  tick duration within baseline ×1.05, and no sustained growth of command
  queue depth/age or persistence backlog. These thresholds are initial targets
  to calibrate against measured variance, not proven limits;
- after release, VPS loop lag, tick duration and queue age read through the
  loopback metrics show no regression against the previous release;
- real-client acceptance is recorded for any feature a player can see.

## Work packages

The research features overlap: many of them are different screens over the
same piece of infrastructure. The plan is therefore organized in ten work
packages, each built once and reused. A package lists the research features
it absorbs; the traceability table at the end maps every F-number.

| Package | Shared infrastructure | Absorbs | Cost |
| --- | --- | --- | --- |
| WP1 Close P1 | — | P1 acceptance, notice cap | C |
| WP2 Audit hygiene | `staff_audit` writer | audit write filtering, retention, boot receipt (F24 remainder) | B |
| WP3 Metrics and protection | In-process `expvar` sampler | F01, F18, F27, lag histogram, circuit breaker, F35 counters | A + B |
| WP4 World read model | Existing Overview read (single admission gate), extended with optional sections and a shared cache | F02 filters, F04 live layer, F13, F31 live query | C |
| WP5 Operations helper | `wyd-log-reader` service on a Unix socket | F36, F19 | E + B |
| WP6 Account investigation | Panel read-only PostgreSQL pool | F03, F06 inspection, login history, F07/F34 case log, F35 review queue | B |
| WP7 Build-time export | `cmd/web-export` static JSON and Astro build | F15, F29, F31 layers, F04 terrain image, F20/F32 content | A |
| WP8 Event outbox | One append-only event table written in the critical transaction | F05, F26, F28, F21 source data | D + E |
| WP9 Typed commands v2 | Existing receipt/idempotency pattern; world events extend the global drop event | F06 grants, EXP/drop multipliers, instance and war controls, F17 index, PIN re-entry | D |
| WP10 Local toolchain | Server loaders and rules reused by local tools | F10, F25, F30, F11, F14, F16, F22/F33, F21 rollups | E |

Removed as duplicates or rejected: F09 (decision 1), F23 (production bots),
F08 remainder (guild/area notices, no current need), F03 progression edits (no
current need).

### Reuse of existing features

Each package extends what the panel already has instead of adding a parallel
screen or contract:

| Existing feature | Absorbs | Consequence |
| --- | --- | --- |
| Bosses panel (`GET /staff/bosses`: alive, HP/MaxHP, position, spawn, respawn time) | F12 boss monitor | Mostly delivered. Only the current phase (already tracked in `BossRuntime.Phase`) is added to the existing status, in stage 1. |
| Overview cards and diagnostics | F01, F18 charts | Charts render on the existing Overview, not a new dashboard page. |
| Overview read and player list | F02 filters, F04 live layer, F13, F31 query | New sections of the same World read, same gate and cache. |
| Player inspector and persistent snapshot | F06 inspection | Inventory/Cargo added to the inspector, reusing the player portal item presentation. |
| Account directory and Sanctions panel | F03, F07, F34 | The account page is the directory row expanded: characters, sanctions, login history, staff actions, case notes. The standalone Sanctions lookup becomes that page. |
| Audit history viewer | F26 event explorer | One viewer with two sources (staff audit, game events) and the same keyset paging. |
| Premium `donate_ledger` | F05 for premium currency | Stays the authoritative premium ledger; `game_events` does not duplicate it and the explorer reads both. |
| Global drop event control | EXP/drop multipliers | Same process-local event model (epoch, expected event ID, duration, stop); becomes a "World events" panel with item drop, drop rate and EXP modes. |
| Colosseum and other event panels | F13 instance actions | Instance status shows the existing Colosseum status; new instance actions follow the Colosseum command pattern. |
| Maintenance panel and countdown | F24 receipt, F19 backups | Last start/stop receipt and backup status shown in the same panel. |
| Staff message and kick/teleport receipts | WP9 envelope | Every new mutation uses the same epoch/session/operation envelope. |
| `cmd/web-api` (read-only Control API) | — | Receives no new features; staff features live only in the embedded panel. |

### Panel navigation

The sidebar has grown one link per feature. It is regrouped into six areas as
packages land, without changing endpoints:

| Area | Contents |
| --- | --- |
| Overview | Metrics, charts, diagnostics (WP3) |
| Players | Online list, filters, inspector, message, kick, teleport, live map (WP4) |
| Accounts | Directory, account page with sanctions, history, cases (WP6), account creation |
| World events | Global drop and multipliers, quiz, bosses, PvP door, OX, Colosseum, instances, premium shop |
| Operations | Notices, maintenance, logs, backups (WP5) |
| Audit | Staff audit and game events (WP2, WP8) |

### WP1 — close P1 (stage 0)

- Grant the five new capabilities, rebuild the panel, real-client acceptance
  of ban/mute/notice/countdown and one restart on the VPS.
- Deliver at most one staff broadcast per tick; countdown notices first, other
  due notices move to the next tick.

Status 2026-10-22: capabilities granted in the local and VPS-template staff
files; broadcast cap implemented and automated-tested. Pending: panel
rebuild, real-client acceptance and the VPS release/restart.

### WP2 — audit hygiene (stage 0)

- Persist only mutations, denials and failures in `staff_audit`; read-only
  successes (overview, directory, snapshots, audit reads) stay in stderr.
  Required before anything polls.
- Batched retention (default 365 days) run by the panel writer off-peak.
- At startup the server writes one `server.start` row with build version and
  hash. A scheduled restart is confirmed when that row follows the
  `server.shutdown` row, without a separate readiness service.

Status 2026-10-22: implemented and automated-tested, including PostgreSQL
pruning on a temporary cluster. Details in
[staff operations](staff-operations.md#audit-history). Pending: first VPS
start to confirm the boot receipt there.

### WP3 — metrics and protection (stage 1)

- Fixed-bucket loop-lag histogram (`world_loop_lag_buckets`); it is the base
  of every load gate.
- One in-process sampler reads existing `expvar` values every 10 s into a 24 h
  ring buffer (about 8,640 samples × ~20 metrics) and serves dashboard and
  performance charts from that cache (F01, F18). `/debug/pprof` stays
  loopback.
- The same sampler drives the circuit breaker: optional panel reads return
  `503 temporarily_unavailable` while lag, queue age or
  `postgres_async_queue_depth` exceed thresholds.
- New counters are fixed-cardinality and added only by the feature that needs
  them (F27), including movement/packet rejection counters for F35.

Status 2026-10-22: implemented and automated-tested (lag histogram, sampler,
charts, breaker). F35 counters are added later with their review queue.
Details in [staff operations](staff-operations.md#load-history-and-protection).

### WP4 — World read model (stages 1 and 3)

The existing Overview read gains optional sections, keeping its single
admission gate, plus a 5–15 s shared cache. Each section is bounded:

- stage 1: player filters (level, class, guild, area) inside the existing
  overview pass (F02); current boss phase added to the existing bosses status
  (F12);
- stage 3: viewport snapshot from the spatial index with a cell range and
  entity cap (F04 live layer); active instances with members, stage and
  deadlines (F13); per-coordinate rule query (F31).

No global map, push stream or per-mob polling.

Status 2026-10-22 (stage 3): viewport map (up to 128-cell radius, 200
players, 400 monsters, 12 reads a minute per staff), active item-instance
monitor and per-cell rule query implemented and automated-tested. The map and
instance reads share the overview gate and pause under the circuit breaker;
the cell query reads only static data.

Status 2026-10-22: stage 1 part (player filters, boss phase) implemented and
automated-tested. Filters reuse the overview pass; the breaker treats them as
optional reads.

### WP5 — operations helper (stage 2, authorized)

`wyd-log-reader` service on a Unix socket, fixed allowlist (game, player and
backup units, plus the backup receipt file), 200 default/500 maximum lines,
cursor paging, `operations.logs.read`. Shows logs (F36) and backup status:
time, size, checksum, last restore test, with no download or restore (F19).
Installed through `wyd-release`; the game process gains no journal
permission.

Status 2026-10-22: implemented and automated-tested (protocol, filtering,
peer check, receipts, Linux cross-build); release scripts install the
socket, service and user. Pending: the VPS release and a real journald read
there. Details in [staff operations](staff-operations.md#server-logs-and-backup-status).

### WP6 — account investigation (stages 2 and 3)

The account directory row expands into one account page over the read-only
pool, replacing the standalone Sanctions lookup:

- stage 2: characters, sanctions, staff actions targeting the account, login
  history (F03) and, in the existing player inspector, read-only
  Inventory/Cargo/equipment using the player portal item presentation (F06). Login history is
  a new `account_logins` table written off the World loop (one row per
  admitted login, IP visible only with a dedicated capability, 90-day
  retention). Status 2026-10-22: implemented and automated-tested, including
  the v9→v10 upgrade on a temporary PostgreSQL cluster;
- stage 3: case notes on the same page linking sanctions, audit entries,
  `BUG-NNN` records and anomaly counters under review (F07, F34, F35). A
  separate ticket system is not planned; player-submitted tickets would come
  later through the player API.

Status 2026-10-22: stage 3 case notes implemented and automated-tested
(schema v11, append-only, BUG-NNN links). Anomaly counters (F35) are not built
yet, so the page has nothing of that kind to link.

### WP7 — build-time export (stage 3)

Zero World cost:

- item/NPC/quest cross-references (F15) and the dependency validator for
  unresolved references (F29);
- terrain and zone layers rendered once for the map (F04 background, F31);
- news moved from `web/portal/src/data/public-content.ts` to one Markdown
  file per post with validated front matter, rendered without raw HTML; site
  banner as a build flag (F20, F32, decision 2).

Status 2026-10-22: implemented and automated-tested, with one deliberate
change. The cross-reference index, validator and terrain image are built once
when the server starts (`internal/staffcontent`) and served only through the
authenticated panel API, not exported at build time: the release copies the
panel's static assets into the public site, so data bundled into them would
become public. Crafting recipes are not indexed because they are code, not
data files. The news pages build byte-identical to the previous ones. The
real data currently has 175 warnings: 107 monster drops and 68 shop slots
point at item IDs missing from `itemlist.csv`. Review fixes the same day: the
index shows the effective drop rate from the rule the World uses
(`model.DropRateForSlot`) and "every kill" for guaranteed or zero rates;
generators the World reserves for item instances or the Colosseum are
labeled and never reported as boot errors; a generator using one template
as leader and follower is one row; the live-map quota counts only served
reads; and writing a case note re-checks permission right before the write.

### WP8 — event outbox (stage 4, measured)

A single append-only `game_events` table written inside the same critical
transaction as the change it describes: staff grants/removals, trades, NPC
shop and a configurable list of rare drops. Premium currency keeps its
existing `donate_ledger`; `game_events` does not duplicate it. No autosave or
per-tick rows. It serves:

- item and gold history by UID (F05) and the event explorer, as a second
  source in the existing audit viewer next to `staff_audit` and
  `donate_ledger` (F26);
- Discord webhooks read by a worker outside the World with backoff and rate
  limits (F28);
- the source data for analytics rollups (WP10).

Gate: commit latency, WAL growth and critical-write queue impact in a local
load comparison. If it exceeds the budget it stays off.

### WP9 — typed commands v2 (stage 4, after WP8)

All new mutations reuse the existing pattern (typed body, operation ID,
receipt, execution-time re-check, audit) and write their event to WP8:

- grant/remove/restore items and gold, online characters only in the first
  version, full inventory refused (F06);
- EXP and drop-rate multipliers as new modes of the existing global drop
  event (same epoch, expected event ID, duration and stop), shown in one World
  events panel;
- instance cancel with full cleanup and guild war force/cancel, following the
  Colosseum command pattern and only where the system already has a safe
  lifecycle;
- optional PIN re-entry for shutdown, bans and grants (the replacement for
  F09's step-up);
- the panel lists these commands in one action index (F17). No macros, shell
  or scripts.

### WP10 — local toolchain (stage 5)

Command-line tools run on the maintainer's machine, sharing the server loaders
and rules:

- data editors that produce a reviewable diff for `data/` (F10, F25, F30),
  validated by the WP7 checker;
- drop, craft and combat simulators with a fixed seed (F11, F14, F16);
- `cmd/loadtest` scenarios against a local disposable database (F22, F33);
- nightly analytics rollups from WP8 events, read by the panel (F21).

## Order

| Stage | Packages | Starts when |
| --- | --- | --- |
| 0 | WP1, WP2 | Now |
| 1 | WP3, WP4 (player filters, boss phase) | Stage 0 released |
| 2 | WP5, WP6 (account page) | Lag histogram available |
| 3 | WP4 (map, instances, zones), WP6 (cases), WP7 | Stage 2 released |
| 4 | WP8, then WP9 | Local load baseline recorded |
| 5 | WP10 | Independent; any time after WP7 |

## Traceability

| Feature | Package | Feature | Package |
| --- | --- | --- | --- |
| F01 | WP3 | F19 | WP5 |
| F02 | WP4 | F20 | WP7 |
| F03 | WP6 (edits deferred) | F21 | WP8 data, WP10 rollups |
| F04 | WP4 live layer, WP7 terrain | F22 | WP10 |
| F05 | WP8 | F23 | Rejected |
| F06 | WP6 view, WP9 grants | F24 | Done in P1, receipt in WP2 |
| F07 | WP6 | F25 | WP10 |
| F08 | Done in P1, segmentation deferred | F26 | WP8 |
| F09 | Rejected (decision 1) | F27 | WP3 |
| F10 | WP10 | F28 | WP8 |
| F11 | WP10 | F29 | WP7 |
| F12 | Bosses panel, phase field in WP4 | F30 | WP10 |
| F13 | WP4 | F31 | WP7 layers, WP4 query |
| F14 | WP10 | F32 | WP7 |
| F15 | WP7 | F33 | WP10 |
| F16 | WP10 | F34 | WP6 |
| F17 | WP9 | F35 | WP3 counters, WP6 review |
| F18 | WP3 | F36 | WP5 |

## Decisions (2026-10-22)

1. Staff authentication stays account password + PIN; no TOTP (F09 rejected).
2. Site content is Markdown in Git, published by build and release (F20).
3. No staging environment. The server is an open beta, so the usual flow
   stays: test on the maintainer's machine, release, then watch the VPS
   metrics. Load comparisons and editors/simulators (stage 5) run on that
   machine when needed. Linux-only parts (journald for F36, systemd restart)
   are validated on the VPS during release.
4. The `wyd-log-reader` helper service is authorized for the VPS (F36).
