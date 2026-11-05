# Staff operations: sanctions, notices, maintenance and audit

Classification: `MODERNIZACAO_COMPATIVEL`. These controls reuse existing 7.48
boundaries (the 0x101 top notice, the 0x334 whisper line, login rejection
through the top notice and the authoritative disconnect path). No opcode,
packet layout or client resource changes.

All endpoints live in the panel embedded in `tm.exe`
(`wydgo748/internal/webadmin/staff_ops.go`), require a staff session, CSRF for
mutations, and re-check the staff file and session immediately before
execution. The standalone Web API cannot use them. The contract is in
`wydgo748/api/openapi.yaml`.

## Capabilities

Add these to the staff file (`web_admin_staff`, `/etc/wyd-go/staff.json` on
the VPS) to grant each operation. Staff must sign in again after a change.
A staff entry may hold up to 32 capabilities. The tracked local
(`wydgo748/data/config/staff.json`) and VPS template
(`tools/deployment/vps/staff.json`) grant `content.npc.shop` to their
maintainer accounts. Add it only to other authorized accounts. The installed
`/etc/wyd-go/staff.json` remains separately managed.

| Capability | Grants |
| --- | --- |
| `moderation.account.ban` | Read sanction history; apply and revoke bans |
| `moderation.account.mute` | Read sanction history; apply and revoke chat mutes |
| `server.notice` | Broadcast/schedule notices; message one player |
| `server.shutdown` | Schedule and cancel maintenance shutdown or restart |
| `staff.audit.read` | Read the persisted audit history |
| `content.npc.shop` | Edit active NPC JSON definitions per channel; see [NPC definition management](../protocol/npc-definitions.md) |
| `game.persistence.recovery` | List quarantined critical writes and request an audited exact-outcome check; see [server operations](operations.md#critical-persistence-operation). No staff file grants it by default |

## Sanctions (ban and mute)

- Stored in PostgreSQL table `account_sanctions` (schema v9). Rows are an
  append-only history: revocation stamps `revoked_at`/`revoked_by`, expiry is
  computed from `expires_at`. Duration is 1 minute to 1 year, or permanent.
- `requestId` makes a retry idempotent. A different body under the same ID is
  `operation_conflict`.
- The row is written before any live effect. The World then receives the
  account's current active state:
  - a ban closes every session of the account through the normal
    disconnect/persistence path and blocks login until expiry or revocation;
  - a mute blocks player-authored text visible to others: local, whisper and
    letters, party, guild, kingdom (`@@`), channel (`@`), global, and setting
    a new tab or status label. Clearing a label (empty `/tab` or `/snd`) stays
    allowed, so an offensive label can always be removed. Balance/medal
    queries stay available.
  - each live refresh reserves a generation on the World loop before reading
    storage, scoped to the normalized account. A superseded result is discarded,
    including when a revocation removed every active sanction. It cannot
    resurrect a revoked ban/mute or remove a newer sanction.
- `live` in the result: `applied` (online sessions updated), `offline`
  (applies at next login) or `pending` (the World did not confirm; the row
  still governs the next login; revoke/apply again to refresh).
- Login reads the active state in the authentication goroutine, never on the
  World loop. If that read fails, the login is refused ("Login temporarily
  unavailable") rather than admitting a possibly banned account. If a live
  refresh starts after authentication began, its old result is also refused:
  the player must retry with a fresh snapshot instead of restoring a revoked
  mute or bypassing an in-flight sanction change.
- Accounts are identified by the canonical lowercase name (1–12 `[a-z0-9]`).

## Notices and staff messages

- Text is printable ASCII only, 1–94 bytes, without leading or trailing spaces:
  the 0x101 and 0x334 text fields hold 95 bytes and the 7.48 client renders a
  legacy code page.
- Each delivery sends the top notice plus a `[Staff]` chat line to every player
  in the world. A notice can start now or after up to 24 hours and repeat up to
  100 times every 1 minute to 24 hours. At most 32 notices are scheduled.
- A World tick (500 ms) sends at most one scheduled broadcast: the maintenance
  countdown first, then the earliest due notice. Other due notices wait for
  the next tick, so simultaneous schedules are spread instead of stacked.
  Immediate notices sent from the panel are delivered by their own command.
- Scheduled notices and operation receipts are process-local and lost on
  restart. Receipts have separate budgets per operation kind (notice, notice
  cancel, player message, shutdown), 1,024 each, so heavy notice use can
  never block scheduling a shutdown. A full kind evicts its receipts older
  than 24 hours; a retry within 24 hours always replays instead of repeating.
- A player message targets one session with the same `moderationEpoch` /
  UID / account / session envelope as kick.

## Scheduled maintenance

- Delay 1 minute to 24 hours, mode `shutdown` or `restart`, reason required.
  Only one schedule exists at a time; cancel it before scheduling another.
- Players are warned at scheduling and when the countdown crosses 60, 30, 15,
  10, 5, 3, 2 and 1 minutes and 30 and 10 seconds. New logins are refused in
  the final minute. The cutoff and final-shutdown state are checked both when
  authentication starts and when its result returns, before reserving an
  account session or loading character-selection state.
- At zero the process follows the SIGTERM path: the panel stops, the World
  persists every player and the process exits. Exit code `0` for shutdown,
  `75` for restart. The VPS unit uses `Restart=on-failure`, so `75` restarts
  the service and `0` leaves it stopped. A process started by hand does not
  restart itself.
- An unsuccessful final drain exits `1`, exactly like SIGTERM.
- Once the countdown has reached zero it cannot be cancelled.
- Every start of the integrated panel writes a `server.start` audit row with
  process ID, channel, Go version, VCS revision/time/modified flag (when the
  binary was built from a Git checkout) and the executable's SHA-256. A
  restart is confirmed when a `server.start` row follows the
  `server.shutdown` schedule; filter the audit history by `server.start`.

## Audit history

- Every panel audit call is written to the JSON log on stderr. Calls with
  consequences are also queued (1024 entries) to PostgreSQL table
  `staff_audit` through the game writer store, because the panel database role
  may be read-only: every mutation, denial and failure, logins and the boot
  receipt. Successful read-only requests (`GET`/`HEAD` with result
  `succeeded`, such as overview refreshes) stay in stderr only, so polling
  never turns into database writes. A full queue or write failure logs a
  warning; the stderr record remains authoritative.
- Entries hold actor, action, result, request ID, client IP, method, path and
  up to 32 detail attributes (512 bytes each). Passwords, PINs and tokens are
  never passed to the audit call.
- `GET /api/v1/staff/audit` pages newest first by id (`before` keyset), with
  optional exact `actor` and `action` filters.
- Retention is 365 days. The audit writer deletes older rows itself, oldest
  first, in at most 20 batches of 500 per run: first run 10 minutes after
  start, then every 24 hours, or every hour while a backlog remains. A failed
  run waits for the next daily run.

## Load history and protection

- The game records loop lag in fixed buckets (`world_loop_lag_buckets`, the
  same `<=1ms` … `<=1s`, `+Inf` labels as tick duration), so p95/p99 can be
  derived between two readings instead of only last/max.
- The integrated panel samples process counters every 10 seconds
  (`control.ReadRuntime`: atomic `expvar` values and `runtime/metrics`, no
  World command, no stop-the-world) into a 24-hour in-memory ring of 8,640
  points, lost on restart. Each point holds online players, mobs, queue depth,
  loop-lag/tick/queue-age p99 of the interval (bucket upper bound; `1000001`
  means above one second), commands per second, budget overruns, panics,
  PostgreSQL queue depth, failures and transaction time, heap, goroutines and
  GC cycles.
- Each point also holds diagnostic groups. `transport` has session closes by
  first cause, refused connections, output packet/byte rates, output queue
  p99/maximum fill (25/50/75/99/100 %) and queue-wait p99. `pool` has the game
  store pool connections, mean acquisition time, empty-pool waits and
  cancellations. `critical` has pending, queued, running, quarantined and
  deferred work, the oldest pending age, execution p99, outcomes and admission
  refusals. `channels` has each channel's players, queue, lag, tick, command,
  queue-age p99 and actor heartbeat age. Process gauges sum both channels; each
  channel keeps its own previous tick, so cluster ticks no longer hide lag.
- `GET /api/v1/staff/metrics?window=15m|1h|6h|24h` (`server.status`) returns
  at most 360 points: longer windows merge groups keeping peaks and gauge
  maxima, summing interval counts and averaging the command rate. The Overview
  draws twelve charts from it, including disconnects, output queue overflows and
  wait, pool wait, pending critical writes and actor heartbeat age; there is no
  automatic refresh.
- Circuit breaker: when the latest interval has loop-lag p99 above 100 ms,
  queue-age p99 above 250 ms or a PostgreSQL write queue above 1,024, optional
  World reads are refused with `503 temporarily_unavailable` and
  `Retry-After: 10`. Optional means overview search, filters or a page after
  the first, and live presence in the account directory (shown as
  unavailable). The default first overview page is always served, because it
  is how staff see the problem.

## Player filters and boss phase

- The overview accepts `minLevel`, `maxLevel`, `class`, `guild`, and an area
  (`x`, `y`, `radius` up to 256 cells, Chebyshev distance). Filters run in the
  same World pass as the search; `online` stays the unfiltered count. Invalid
  values or partial areas return `400 invalid_query`. The standalone Control
  API accepts the same parameters but its client does not forward them.
- Boss status includes `phase`: the live behaviour phase of an alive boss
  (1 = initial), 0 when dead. The Bosses panel shows it next to the mob ID.

## Account page and saved items

- `moderation.account.ip` reveals IP addresses in login history;
  `operations.logs.read` reads server logs and backup receipts. The tracked
  staff files grant both to the maintainer account.
- Schema v10 adds `account_logins`: one row per admitted game login (account,
  client IP, channel, time). The World only enqueues the record (256 slots,
  never blocking; overflow counts in `world_login_history_dropped_total`) and
  one goroutine writes it. Rows older than 90 days are deleted by the panel's
  audit writer in the same bounded batches as the audit retention.
- The account page opens from the directory ("Open account") or the online
  list ("Account"): characters from the directory row, the last 50 logins
  (`GET /api/v1/staff/accounts/{account}/logins`, `moderation.player.search`;
  IPs only with `moderation.account.ip`) and the last 50 persisted staff
  actions naming the account (`.../actions`, `staff.audit.read`, through an
  expression index on the target account). The Sanctions section loads the
  same account.
- The player inspector loads saved items on request:
  `GET /api/v1/staff/players/{uid}/storage?account=` returns the persisted
  equipment, carry inventory (UI slots only) and account Cargo with catalog
  names, through the panel's read-only pool. Live state may differ until the
  next save; nothing can be edited.

## Server logs and backup status

- A separate `wyd-log-reader` service (`cmd/log-reader`) owns journal access.
  `wyd-log-reader.socket` creates `/run/wyd-log-reader/socket` owned by
  `wyd-log-reader:wyd-game`, mode 0660; the service runs as its own user in
  `systemd-journal`, with no network (`PrivateNetwork`, `AF_UNIX` only),
  64 MiB, 20% CPU, idle I/O and a read-only filesystem. It accepts only the
  `wyd-game` user (kernel peer credentials) and at most two requests at once.
  The game process gains no journal permission.
- Sources are a fixed map (game, player, backup → their units). The helper
  runs `journalctl` with a fixed argument list, newest first, scanning at
  most 20× the page size (5,000 lines), filters the substring itself, strips
  escape and control characters and cuts messages at 2,048 bytes. A reply is
  at most 512 KiB of encoded JSON: only matching lines count, measured after
  escaping, and a page that reaches the limit ends early with a cursor right
  after the last line sent. The helper reads each request before answering,
  including refusals. Windows are up to 7 days, pages up to 500 lines with an
  opaque cursor; a rotated cursor returns `410 cursor_expired`.
- Release: every log reader step in `update-release.sh` (user, receipt
  directory, units, socket start, restart) only prints a warning on failure;
  it never rolls back the game release.
- `GET /api/v1/staff/logs` and `GET /api/v1/staff/backups` need
  `operations.logs.read`; `server.status` does not grant logs. Each log read
  is persisted in the audit with source, window, line count and whether a
  filter was used, never the content or the search text.
- `backup.sh` writes `/var/lib/wyd-go-backup/status.json` (time, file name,
  size, SHA-256) and `test-database.sh` writes `restore.json` after the
  release restore test. The panel shows both; dumps cannot be downloaded.
- Without `web_admin_log_reader_socket` (local Windows), the views report
  that the service runs only on the VPS.

## World map, instances and content index

- **Content index** (`internal/staffcontent`), built once at server start
  from the same loaded files as the World, plus the generators the World
  reserved at boot, and never changed afterwards: which NPC sells an item,
  which monster drops it, which quest requires, consumes or rewards it, and
  where each template spawns (`NPCGener` index, leader, follower or both,
  first segment, maximum). The drop chance is the effective "1 in N" the
  World rolls for that monster before the killer's drop bonus: the
  `droprate.json` slot value adjusted by the shared rule
  `model.DropRateForSlot` (level bands, slots 8–10 forced to 4, slot 11
  always, cap 32,000); a slot whose effective rate is zero drops on every
  kill. The base value is shown when it differs. Generators reserved for an
  item instance or the Colosseum are labeled as such: the World skips them
  before the template check, so they never spawn in the open world. Search by
  name, item or template detail, and a list of unresolved references: "error"
  when an enabled, unreserved generator has no template (the boot would
  fail), "warning" for reserved or disabled generators without a template and
  for drops, shop slots or quest items whose ID is missing from the item
  table. Served only by
  `GET /api/v1/staff/content/{search,items/{id},mobs/{name},issues}`
  (`server.status`), never bundled into the panel's static files, because the
  release copies those files into the public site.
- **Cell rules** `GET /api/v1/staff/world/zone?x=&y=` (`server.status`) from
  static data only: height, raw attribute byte with the three bits the server
  relies on named (0x02 blocked, 0x10 client teleport activation, 0x80 newbie
  field), portals whose 4×4 source square contains the cell, quest zones and
  clear areas, instance areas.
- **Terrain image** `GET /api/v1/staff/world/terrain`: the attribute map as a
  1024×1024 PNG (about 48 KiB), rendered on first request and cached.
- **Live map** `GET /api/v1/staff/world/map?x=&y=&radius=`
  (`moderation.player.search`): players and monsters within a square of up to
  128 cells around the point, from the spatial index, capped at 200 players
  and 400 monsters, 12 served reads a minute per staff account (a read the
  World does not serve, such as a busy reply, is not counted).
- **Instances** `GET /api/v1/staff/world/instances` (`server.status`): every
  active item instance (Water, Magic Chamber, Cube, Nightmare, Hell Gate and
  other ticket or event rooms) with members, monsters alive, stage, state and
  deadlines. The Colosseum keeps its own section.
- The live map and instance list share the overview's single World read slot
  and are optional reads: they pause while the circuit breaker is open.
- Resetting either panel invalidates its pending requests. Replies from the
  previous staff session cannot repopulate player/account names, coordinates
  or instance members, overwrite new-session errors, or release the new
  session's in-flight request guard. Terrain image callbacks use the same fence.

## Case notes

- Schema v11 adds `account_notes`: append-only staff notes on the account
  page (3–2,000 characters of plain text, up to 10 `BUG-NNN` references to
  the gameplay bug register). A correction is a new note.
- Reading needs `moderation.player.search`; writing needs
  `moderation.account.ban` or `moderation.account.mute`, checked with the
  session again right before the write, so a revocation during the request
  is honored. The audit records the note ID, length and bug references, never
  the text.

## Site news and banner

- News posts are Markdown files in `web/portal/src/content/news/<slug>.md`;
  the format and release step are described in the
  [launcher and patch guide](../client/launcher.md). Paragraphs are rendered
  as escaped text, so HTML in a post fails the build instead of reaching
  players.
- Building the portal with `PUBLIC_SITE_BANNER` set shows that text (up to
  200 characters, escaped) at the top of every public page, for example during
  maintenance. The game is not affected.

Next steps for the panel are planned in the [staff panel roadmap](#implementation-plan).

## Validation state

- Automated: Go unit tests (World, HTTP, PostgreSQL store including the v8→v9
  upgrade on a temporary cluster), full `go test ./...` and `go vet ./...`,
  portal `astro check`, `node --test` and a static build.
- Regression coverage: delayed, out-of-order ban/mute refreshes and account
  isolation; revocation during pending authentication; maintenance cutoff at
  authentication completion; and old-session map/instance success/error replies
  while a new session request remains pending.
- Pending: real-client acceptance (login rejection text, mute notice, top
  notice and `[Staff]` line rendering, countdown during play), a browser run of
  the panel against `tm.exe`, and a supervised restart on the VPS.

<a id="implementation-plan"></a>
## Staff panel implementation plan and history

This is the dated planning record. Its baseline measurements and remaining-work claims are historical. Use the operational sections above and current release receipts for delivery status.

<a id="implementation-plan-staff-panel-roadmap"></a>

Single implementation plan for the embedded staff panel (`/admin`). It merges:

- the staff operations delivered on 2026-10-21 ([staff operations](staff-operations.md));
- the remaining items of the first panel review (economy, support, world
  controls, observability);
- the external MMO panel research (features F01–F36, kept in the maintainer's
  Notion workspace), after checking its claims against the current tree.

This document is a plan. Nothing below is implemented unless the baseline
section says so.

<a id="implementation-plan-baseline-after-2026-10-21"></a>
### Baseline after 2026-10-21

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

<a id="implementation-plan-performance-model"></a>
### Performance model

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

<a id="implementation-plan-stage-gates"></a>
### Stage gates

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

<a id="implementation-plan-work-packages"></a>
### Work packages

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

<a id="implementation-plan-reuse-of-existing-features"></a>
#### Reuse of existing features

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

<a id="implementation-plan-panel-navigation"></a>
#### Panel navigation

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

<a id="implementation-plan-wp1--close-p1-stage-0"></a>
#### WP1 — close P1 (stage 0)

- Grant the five new capabilities, rebuild the panel, real-client acceptance
  of ban/mute/notice/countdown and one restart on the VPS.
- Deliver at most one staff broadcast per tick; countdown notices first, other
  due notices move to the next tick.

Status 2026-10-22: capabilities granted in the local and VPS-template staff
files; broadcast cap implemented and automated-tested. Pending: panel
rebuild, real-client acceptance and the VPS release/restart.

<a id="implementation-plan-wp2--audit-hygiene-stage-0"></a>
#### WP2 — audit hygiene (stage 0)

- Persist only mutations, denials and failures in `staff_audit`; read-only
  successes (overview, directory, snapshots, audit reads) stay in stderr.
  Required before anything polls.
- Batched retention (default 365 days) run by the panel writer off-peak.
- At startup the server writes one `server.start` row with build version and
  hash. A scheduled restart is confirmed when that row follows the
  `server.shutdown` row, without a separate readiness service.

Status 2026-10-22: implemented and automated-tested, including PostgreSQL
pruning on a temporary cluster. Details in
[staff operations](#audit-history). Pending: first VPS
start to confirm the boot receipt there.

<a id="implementation-plan-wp3--metrics-and-protection-stage-1"></a>
#### WP3 — metrics and protection (stage 1)

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
Details in [staff operations](#load-history-and-protection).

<a id="implementation-plan-wp4--world-read-model-stages-1-and-3"></a>
#### WP4 — World read model (stages 1 and 3)

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

<a id="implementation-plan-wp5--operations-helper-stage-2-authorized"></a>
#### WP5 — operations helper (stage 2, authorized)

`wyd-log-reader` service on a Unix socket, fixed allowlist (game, player and
backup units, plus the backup receipt file), 200 default/500 maximum lines,
cursor paging, `operations.logs.read`. Shows logs (F36) and backup status:
time, size, checksum, last restore test, with no download or restore (F19).
Installed through `wyd-release`; the game process gains no journal
permission.

Status 2026-10-22: implemented and automated-tested (protocol, filtering,
peer check, receipts, Linux cross-build); release scripts install the
socket, service and user. Pending: the VPS release and a real journald read
there. Details in [staff operations](#server-logs-and-backup-status).

<a id="implementation-plan-wp6--account-investigation-stages-2-and-3"></a>
#### WP6 — account investigation (stages 2 and 3)

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

<a id="implementation-plan-wp7--build-time-export-stage-3"></a>
#### WP7 — build-time export (stage 3)

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

<a id="implementation-plan-wp8--event-outbox-stage-4-measured"></a>
#### WP8 — event outbox (stage 4, measured)

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

<a id="implementation-plan-wp9--typed-commands-v2-stage-4-after-wp8"></a>
#### WP9 — typed commands v2 (stage 4, after WP8)

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

<a id="implementation-plan-wp10--local-toolchain-stage-5"></a>
#### WP10 — local toolchain (stage 5)

Command-line tools run on the maintainer's machine, sharing the server loaders
and rules:

- data editors that produce a reviewable diff for `data/` (F10, F25, F30),
  validated by the WP7 checker;
- drop, craft and combat simulators with a fixed seed (F11, F14, F16);
- `cmd/loadtest` scenarios against a local disposable database (F22, F33);
- nightly analytics rollups from WP8 events, read by the panel (F21).

<a id="implementation-plan-order"></a>
### Order

| Stage | Packages | Starts when |
| --- | --- | --- |
| 0 | WP1, WP2 | Now |
| 1 | WP3, WP4 (player filters, boss phase) | Stage 0 released |
| 2 | WP5, WP6 (account page) | Lag histogram available |
| 3 | WP4 (map, instances, zones), WP6 (cases), WP7 | Stage 2 released |
| 4 | WP8, then WP9 | Local load baseline recorded |
| 5 | WP10 | Independent; any time after WP7 |

<a id="implementation-plan-traceability"></a>
### Traceability

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

<a id="implementation-plan-decisions-2026-10-22"></a>
### Decisions (2026-10-22)

1. Staff authentication stays account password + PIN; no TOTP (F09 rejected).
2. Site content is Markdown in Git, published by build and release (F20).
3. No staging environment. The server is an open beta, so the usual flow
   stays: test on the maintainer's machine, release, then watch the VPS
   metrics. Load comparisons and editors/simulators (stage 5) run on that
   machine when needed. Linux-only parts (journald for F36, systemd restart)
   are validated on the VPS during release.
4. The `wyd-log-reader` helper service is authorized for the VPS (F36).
