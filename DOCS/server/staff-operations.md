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
(`wydgo748/data/staff.json`) and VPS template
(`tools/deployment/vps/staff.json`) files grant all five to the maintainer
account; the installed `/etc/wyd-go/staff.json` changes only with a release.

| Capability | Grants |
| --- | --- |
| `moderation.account.ban` | Read sanction history; apply and revoke bans |
| `moderation.account.mute` | Read sanction history; apply and revoke chat mutes |
| `server.notice` | Broadcast/schedule notices; message one player |
| `server.shutdown` | Schedule and cancel maintenance shutdown or restart |
| `staff.audit.read` | Read the persisted audit history |

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
  - a revocation that arrives a few milliseconds before a login whose
    database read predates it can leave that session muted; repeating the
    revocation pushes the live state again.
- `live` in the result: `applied` (online sessions updated), `offline`
  (applies at next login) or `pending` (the World did not confirm; the row
  still governs the next login; revoke/apply again to refresh).
- Login reads the active state in the authentication goroutine, never on the
  World loop. If that read fails, the login is refused ("Login temporarily
  unavailable") rather than admitting a possibly banned account.
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
  the final minute.
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
- `GET /api/v1/staff/metrics?window=15m|1h|6h|24h` (`server.status`) returns
  at most 360 points: longer windows merge groups keeping peaks and gauge
  maxima, summing interval counts and averaging the command rate. The Overview
  draws six charts from it; there is no automatic refresh.
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

Next steps for the panel are planned in the [staff panel roadmap](admin-panel-roadmap.md).

## Validation state

- Automated: Go unit tests (World, HTTP, PostgreSQL store including the v8→v9
  upgrade on a temporary cluster), full `go test ./...` and `go vet ./...`,
  portal `astro check`, `node --test` and a static build.
- Pending: real-client acceptance (login rejection text, mute notice, top
  notice and `[Staff]` line rendering, countdown during play), a browser run of
  the panel against `tm.exe`, and a supervised restart on the VPS.
