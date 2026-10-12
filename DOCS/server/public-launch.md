# Public portal and native VPS deployment

## Scope and release gates

The public origin is `https://wyd.forjago.com`. The Spaceship A record `wyd`
points to `46.224.58.209` with a five-minute TTL. This is the same VPS as
Forjago/app-td; that application's Docker database and Caddy host remain
unchanged. WYD uses native Go processes and its own PostgreSQL 18 cluster
listening only on `127.0.0.1:5433`.

This batch is `MODERNIZACAO_COMPATIVEL`: the public HTTP API is new, but game
account payloads, password hashing, the native credential limits and wire
packets remain unchanged. No client or historical binary is patched here.
The database starts empty; no local accounts, characters, guild registry,
staff credentials or test accounts are imported.

The game listener is public on `0.0.0.0:8281` (ufw admits TCP 8281 only), and
the distributed `serverlist.bin` points every channel at `46.224.58.209`. The
VPS never serves the game client: outbound traffic is a cost, so the full
client is distributed through an external Google Drive link; the VPS serves
only the small incremental patches of the [launcher](../client/launcher.md).
The download page links that package (`clientUrl` in `download.astro`).
The site theme reuses 7.48 NewUI textures exported by
`tools/client-assets/export_portal_ui.py`.
Do not describe this deployment as a global client/server release: package
licensing and content review, the outstanding English migration and
critical-persistence recovery gates, and an actual remote game session remain
open. Automated checks do not mean `CLIENT_TESTED`.

## Website and HTTP contract v1

Static Astro routes are `/`, `/register/`, `/player/`, `/ranking/` and
`/download/`; `/SrvCont.htm` is the static channel-population file named by
`serverlist.bin`. Caddy rejects `/debug/*` and proxies `/api/player/*` to the
player service. The staff panel is published at `https://wyd.forjago.com/admin/`:
Caddy proxies `/admin` and `/api/v1/*` to the game process's integrated panel,
which listens only on `127.0.0.1:8082` (`web_admin_origin`,
`web_admin_trusted_proxies`). Sign-in needs the game password plus the PIN
`WYD_ADMIN_PIN`, exactly six ASCII digits (leading zeroes are valid), generated
once in root-only `/etc/wyd-go/game.env`
(`admin_access_pin_env`); capabilities come from the versioned
`tools/deployment/vps/staff.json`, installed as `/etc/wyd-go/staff.json`
(at most 9 entries per account). Sessions use a `__Host-` Secure cookie, CSRF
tokens and exact Host/Origin checks; cross-origin POSTs are rejected. `/patch/*` (launcher patches and the `Guilds.txt` export) is
served on plain HTTP as well, without the HTTPS redirect, because the 7.4x
launcher has no TLS.

The public pages and staff panel share `wyd-theme.css`: charcoal surfaces,
muted gold accents, modern typography and existing 7.48 logo/stone textures.
The anonymous staff page contains only sign-in and a public registration link;
staff navigation and administrative account creation remain hidden until
authentication. Hiding controls is presentation only: existing API permission,
PIN, CSRF, origin and session checks remain authoritative. Public player
registration does not require the staff PIN.

| Method and path | Authorization and response |
| --- | --- |
| GET `/api/player/v1/session` | Anonymous session bootstrap or current identity; CSRF token, version 1 |
| POST `/api/player/v1/accounts` | Anonymous/authenticated CSRF session; create an empty game account, no automatic login |
| POST `/api/player/v1/login` | Same CSRF session; native password verification, cookie and CSRF rotation |
| POST `/api/player/v1/logout` | CSRF session; revoke the cookie server-side |
| GET `/api/player/v1/profile` | Authenticated cookie; own account only, ignoring client-supplied account selectors |
| GET `/api/player/v1/ranking` | Public, at most 100 persisted characters, cached for 30 seconds |
| GET `/healthz` | Process health |
| GET `/readyz` | One-second database reachability check |

Registration accepts 4–12 ASCII letters/digits for the account name and 4–10
printable ASCII password characters without spaces. The legacy wire limits
are explicit UI constraints, not a claim that short passwords are ideal.
Use a unique password. There is no email verification, recovery, payment,
account editing, staff permission, or gameplay mutation in this API.

Profiles expose username, saved premium balance, and character slot/name/class,
internal zero-based level and evolution. The UI displays level plus one.
Ranking exposes character metadata only and orders by evolution, level,
experience and stable name/account keys. These are persisted snapshots, not
online presence or live combat state. No hashes, item identities, equipment,
cargo or private account names are returned in the ranking.

JSON responses carry `version: 1`. Creation returns 201, bad JSON 400,
unauthenticated profile/bad credentials 401, host/origin/CSRF mismatch 403,
duplicate name 409, wrong content type 415, invalid registration 422, rate
limit 429, and backend/capacity failures 503. No failed write is reported as
successful; database details are not returned. POST bodies are limited to
1024 bytes with unknown fields and trailing JSON rejected. Lost responses are
not automatically retried by the UI: check login before repeating a creation.

## Security and lifecycle

`__Host-wyd_player` is Secure, HttpOnly, SameSite=Strict and path `/`. Random
256-bit cookie and CSRF values are independent; server storage indexes a hash
of the cookie. Anonymous sessions expire after 20 minutes and are IP-bound;
authenticated sessions expire after eight hours, with a 30-minute idle timeout.
Logout invalidates the server record; restarting the API invalidates all
sessions. There is no localStorage token or CORS allowance.

All requests require the exact public Host. POST requires the exact HTTPS
Origin, JSON content type and constant-time CSRF comparison. Caddy overwrites
X-Forwarded-For with the actual peer; the API trusts only a single IP supplied
by a loopback proxy, never arbitrary external forwarding headers. Rate and
session storage are bounded, as are concurrent password derivations (two).
Read requests, authentication, registration and anonymous bootstrap have
separate per-peer/global limits. No password is logged.

`wyd_game` owns the authoritative database. `wyd_player` has CONNECT, schema
USAGE and SELECT/INSERT on `accounts` only, not UPDATE, DELETE, game tables,
DDL or a grant to assume the owner role. The deployment installs a trigger
which also rejects nonempty/forged registration payloads when this role inserts,
including premium balance, cargo gold, characters, cargo items and unknown
fields. Game-owner writes remain unaffected. Review this SQL whenever the
registration payload or password format changes.

Credentials are generated on the VPS and held in root-only
`/etc/wyd-go/game.env` and `player.env`, never in Git or deployment archives.
Game, player API and PostgreSQL have independent systemd resource limits.
The services have distinct unprivileged OS users, no capabilities, read-only
release trees and no access to home directories. Only the game can write its
own `/var/lib/wyd-go` state/export directory.

## Deployment and operations

Build the portal first from `wydgo748/web/portal` with `npm run check` and
`npm run build`. Package from the repository root:

```powershell
pwsh -NoProfile -File tools/deployment/Build-VpsRelease.ps1 -ReleaseId wyd-deploy-YYYYMMDD
```

This cross-builds Linux/amd64 Go executables and the isolated database test,
packages an explicit static-data allowlist and the public website, and emits
a SHA-256 checksum into the ignored root `build/deploy/` directory. It does
not include local account state, development `server.txt`, staff identities,
native client executables or the staff HTML entry point. It refuses an existing
staging directory; the server's embedded schema is the only game migration path.

Upload the archive, checksum, test executable and `config/` to a uniquely named
`/tmp/wyd-deploy-*` directory. Review `first-install.sh` before invoking it with
sudo and that directory. It is a **first-install script**, not an updater:
it refuses an existing WYD deployment/cluster and never resets a database.
It installs PostgreSQL from the official Ubuntu PGDG repository, creates the
new roles/database, starts the native services, grants player privileges,
backs up the database and validates Caddy before reloading. A copy of the
original Caddyfile is retained; validation failure restores it without reload.

Later releases use `update-release.sh` from the same staging layout:

```sh
sudo bash /tmp/wyd-deploy-<id>/config/update-release.sh /tmp/wyd-deploy-<id>
```

It verifies the checksum, unpacks a new timestamped release with root-owned,
non-writable modes, installs the versioned `server.txt`, units, backup script
and Caddy site, validates Caddy, switches `/srv/wyd-go/current` atomically,
and restarts the game or player API only when their files or configuration
changed. A failed validation or readiness check (player API `/readyz`, active
game unit, listener on 8281) restores the previous release and configuration.
It never touches the database or `/var/lib/wyd-go`. A valid six-digit PIN is
preserved. An incompatible existing PIN stops the deployment unless migration
was explicitly requested:

```sh
sudo bash /tmp/wyd-deploy-<id>/config/update-release.sh /tmp/wyd-deploy-<id> --migrate-admin-pin
```

Migration generates a new cryptographically random six-digit PIN without
printing it. It atomically replaces only the PIN entry, preserves database
credentials, keeps the environment file root-owned and mode 0600, and restarts
the game to reload the PIN. Root-only `game.env.pre-<timestamp>` and staff
configuration backups are restored with the previous release on failure.
Retrieve the new PIN privately on the VPS; never paste it into repository
records, logs or screenshots. A game restart invalidates staff sessions.

Archives built on Windows store directories with mode 0777. Both scripts
normalize release trees to `u=rwX,go=rX`; the first installation predates
that fix and was corrected in place on October 11, 2026.

Operational checks on the VPS:

```sh
systemctl status wyd-game wyd-player postgresql@18-main
curl -f https://wyd.forjago.com/readyz
sudo journalctl -u wyd-game -u wyd-player --since '10 minutes ago'
sudo ss -lntp
sudo systemctl start wyd-backup.service
```

Daily native `pg_dump -Fc` backups run at approximately 04:15 UTC under
`wyd-backup.timer`, stored root-only in `/var/backups/wyd-go/`. A dump is
published only after `pg_restore --list` succeeds. First publication also
tests a complete restore to a dedicated disposable validation database.
There is not yet off-VPS backup replication, an automatic retention policy,
alerting, or a verified ongoing restore schedule. Monitor disk usage and
configure those before opening the game to production players.

`test-database.sh` uses only `wyd_portal_validation` and
`wyd_restore_validation`, refuses existing fixtures, and removes those
temporary databases on exit. It never inserts test accounts in `wydgo`.
The Go integration test refuses any other database name, uses the actual
native store/schema and player-role trigger, and checks shared authentication,
duplicate races, privilege rejection, forged registrations, profiles and
ranking order. Fixture-skipped local tests are not database proof.

## Validation record — October 11, 2026

- Automated: full Go suite and vet passed; PostgreSQL-dependent existing
  store tests were fixture-skipped in that local run.
- Player API: focused session/security tests and race gate passed.
- Portal: Astro check passed with zero errors/warnings/hints; static build passed.
- DNS: record saved in Spaceship; VPS DNS resolver returns `46.224.58.209`.
- Native deployment: PostgreSQL 18.6, `wyd-game`, `wyd-player` and Caddy are
  active. Game startup reports critical persistence enabled. Game TCP 8281,
  player HTTP 8083 and PostgreSQL TCP 5433 listen on loopback only. Existing
  Forjago HTTPS still returns 200 and `favela-td-postgres-1` remains healthy.
- Live SQL-role integration: `TestPlayerDatabaseIntegration` passed on the
  dedicated fixture. Shared game authentication, concurrent duplicate
  registration, forged-payload rejection, denied permissions, own profile and
  ranking order were exercised. This was a normal Linux test binary, not a
  database-connected race build; the separate local race gate skipped the
  absent database fixture.
- Backup: native dump and complete restore passed; the restored snapshot had
  zero accounts. The daily timer is enabled. Both validation databases were
  removed automatically; only `wydgo` remains, with zero accounts.
- HTTPS: all five public pages, readiness and ranking return 200; HTTP redirects
  with 308. Private staff paths return 404, unauthenticated profile returns 401
  and a registration request without CSRF returns 403. The session cookie has
  Secure, HttpOnly and SameSite=Strict; strict CSP and other headers are present.
- Browser: registration/login session bootstrap enabled the forms, empty ranking
  rendered correctly and the unavailable download message appeared. No console
  warnings/errors were observed. The home page at 390-pixel width had no
  horizontal overflow. No account was created in the live database; authenticated
  profile behavior was validated in the isolated database/API tests.
- Installer corrections: allow game-user traversal of the configuration
  directory while keeping credentials root-only; read staged SQL through root
  stdin rather than granting PostgreSQL access to the private staging directory;
  set the uploaded test executable's mode explicitly. The interrupted install
  resumed only its unexecuted tail, preserving the created database/release.
  The first-install guard rejected a second invocation. Caddy's warning about
  explicitly setting X-Forwarded-For is benign; that explicit peer overwrite is
  retained as part of the proxy trust contract.
- Client packaging, public game port and remote in-game flow: pending.

## Validation record — October 11, 2026 (launcher and public port)

- Found and fixed: 615 world-writable paths under the release tree (Windows
  `tar` modes). Only the `current` symlink remains 0777, which is inherent to
  symlinks.
- Game port: `listen_address=0.0.0.0:8281` and ufw `8281/tcp` applied; the
  server restarted cleanly and a TCP connection from an external host succeeded.
- Release `20261011T195812Z` deployed with `update-release.sh`: Caddy
  validated, `/download/` and `/SrvCont.htm` return 200, private paths still
  return 404. Game and player binaries were identical, so neither restarted.
- `serverlist.bin`: three channels set to `46.224.58.209`, two status URLs to
  `https://wyd.forjago.com/SrvCont.htm`; decoded and size-checked (7040 bytes).
- Patch `20261011.1` (Git tree of `HEAD` plus the new `serverlist.bin`, 8462
  files, 561.8 MiB) was briefly published under `/patch/` and exercised
  headlessly by the launcher engine, then withdrawn the same day because
  client downloads must not consume VPS traffic. The `/patch/` Caddy route was
  removed (404). When `/patch/` returned for the native launcher feed, the
  549 MB copy was moved out of the served tree to root-only
  `/srv/wyd-go/withdrawn-client-blobs`, which awaits manual deletion.
- Launcher feed: release `20261011T210344Z` added the plain-HTTP `/patch/`
  route, `update.htm` (base version 10000) and the `Guilds.txt` export in
  `/var/lib/wyd-go-public`; the game and player API restarted for the new
  unit and configuration.
- Caddy fix: the unmatched `header Cache-Control "no-cache"` ran after the
  matched one and overwrote `immutable` on `/_astro/*`; the matchers are now
  disjoint and hashed assets are cached for a year.
- Remote login and world entry through the VPS remain pending a person running
  the client.

## Six-digit PIN and shared-theme validation — October 11, 2026

- Delta: `MODERNIZACAO_COMPATIVEL`; no game packet, account password limit,
  persisted player data or launcher contract changes.
- Automated: full Go suite and `go vet ./...` passed. PIN tests cover ASCII
  format, leading zeroes, configuration loading, incorrect credentials,
  registration rejection without account creation and secret-free audit logs.
  Fixture-skipped database paths are not new database evidence.
  `CGO_ENABLED=1 go test -race -count=1 ./internal/webadmin` also passed.
- Portal: Astro check reported zero errors, warnings or hints; static build
  passed. Four Node regression tests cover both masked PIN forms, anonymous
  staff UI, shared theme/resources and unchanged native credential limits.
- Deployment: four isolated Python tests cover six-digit generation, leading
  zeroes, preserving valid PINs/database credentials, explicit migration and
  duplicate-entry rejection. Shell syntax and invalid-option rejection are
  checked on the VPS before applying a release.
- Browser: anonymous staff sign-in and public registration were inspected
  locally at desktop and 390-pixel widths. No horizontal overflow was found;
  logo aspect ratio and keyboard-only skip-link presentation were corrected.
  Local static preview has no API, so its expected service-unavailable message
  is not a backend failure. Authenticated staff visual inspection remains a
  separate manual gate; no live player account is created for visual tests.
- Published release: `/srv/wyd-go/releases/20261011T231600Z`, built from
  `wyd-deploy-20261011-site-pin6`. Archive SHA-256:
  `5d5c8ede0b12f9b8c5adbeb0842173303df15bf71dba8480d33237629a5cd250`.
  The updater explicitly migrated the previous eight-digit staff PIN to a
  newly generated six-digit PIN, retaining root-only configuration backups.
  The live environment file has mode `0600`; the PIN was not printed.
- Live checks: game, player API and Caddy are active; home, staff sign-in,
  registration, player panel, ranking, download and readiness return HTTP 200.
  Registration and staff sign-in bootstrap enabled their forms without
  browser console warnings/errors. Anonymous staff controls remain hidden.
  The game restarted with zero established player connections; the player
  API did not restart. The launcher feed checksum was unchanged.
- Scope: changed PIN validation/configuration, OpenAPI, focused Go tests,
  portal forms/layout/theme/session presentation, deployment migration and
  regression tests, and this document. No files were removed. Unrelated
  existing client/audio edits were preserved; their whitespace warnings are
  not failures of the scoped patch.

Commands used for this batch: `go test -count=1 ./...`, `go vet ./...`, the
web-admin race command above, `npm run generate`, `npm run check`,
`npm run build`, `npm test`,
`py -3 tools/deployment/test_admin_pin.py`, `Build-VpsRelease.ps1`,
`Test-RepositoryLayout.ps1`, scoped `git diff --check`, staged shell syntax
and invalid-option checks, and the explicit `update-release.sh` PIN migration.
Authenticated staff visual inspection and a forced deployment-rollback
exercise were not performed; neither is implied by the passing checks.

## Account recovery release — October 12, 2026

- Fix: a confirmed database commit whose in-memory continuation fails no
  longer leaves the account locked until restart; see
  [committed continuation recovery](critical-persistence-plan.md#committed-continuation-recovery-2026-10-12).
- Published release: `/srv/wyd-go/releases/20261012T013651Z`, built from
  `wyd-deploy-20261012-account-recovery` (no PIN migration). Archive SHA-256:
  `ead939e8a2ab7c939880863c631b114708527d10dc1b341a97600713f32d9b25`. The
  live `wyd-server` hash matches the local build.
- Applied with zero established connections on 8281. The game restarted; the
  player API did not. Game, player API and Caddy are active, 8281 listens,
  the home page returns HTTP 200, and the game journal shows no warnings
  since the restart.
- Commands: `go vet ./...`, `go test -race -count=1 ./...` with
  `WYD_TEST_POSTGRES_URL`, `Build-VpsRelease.ps1`, `update-release.sh`.
- Pending: an in-game disconnect/relogin check by the operator.

## Costume slot and timed items release — October 12, 2026

- Release `/srv/wyd-go/releases/20261012T024132Z` from
  `wyd-deploy-20261012-costume-slot`. Archive SHA-256:
  `258b79e07151bb4db36ad7d6498da0ed64756c0c245c62fa433f298a588a1a42`.
- Scope: costume-slot extension, expiry display projection and timed-horse
  catalog (`data/mounts.json` `premium`); see the
  [port record](../client/port-748.md#wyd-go-extensions-costume-slot-expiry-display-timed-horses-2026-10-12).
- Applied with zero established connections on 8281. Game and player API
  restarted; game, player API and Caddy are active, 8281 listens, home returns
  200, and the live `wyd-server` hash matches the local build. The host time
  zone is UTC, so tooltip expiry times are shown in UTC.
- Client patch 10002 was published right after ([launcher](../client/launcher.md#published-patch-10002-2026-10-12)).

## Coordinated feature audit and quest timer release — October 12, 2026

- Source commit: `ce411059ae7b312031c3e83201f9a3c5431eb86d` on `main`.
  The accumulated changes were reviewed across critical-persistence recovery,
  costume/familiar slots, timed items and horses, special skills, client
  lifecycle/UI, player-portal authorization, native-launcher packaging and
  deployment. The audit repaired public/private countdown ownership, the
  character-reset flag and four absent sound dependencies; publication now
  serializes writers and requires an append-only feed.
- Release: `/srv/wyd-go/releases/20261012T041400Z`, built from
  `wyd-deploy-20261012-quest-timer-audit`. Archive SHA-256:
  `ffe9498f32ae62927d4a472eab37c392a503f0efc0fe93463db622b28564aa9b`.
  Live game SHA-256:
  `c5b3688be3ca0fcc344fbf143475a3d376804a686d34934fefa947002ca045af`.
  Live player API SHA-256:
  `e5af6ca57afbd8e4200980bfa99c6d6b0cab8399b33bb30e18273ddf48cf78c3`.
  Both live hashes match the packaged Linux builds.
- Backup completed before installation. The staged player database integration
  test passed on PostgreSQL 18 with the production role separation in a
  dedicated temporary database. A fresh backup restored successfully into a
  second temporary database. Neither test wrote live accounts; both temporary
  databases were removed and their absence was checked afterward.
- All staged shell scripts passed `bash -n`. Unknown updater options and an
  invalid publisher staging path were rejected before installation. The
  update preserved the existing six-digit staff PIN and player credentials.
- There were zero established game connections at restart. Game and player API
  restarted; game, player API and Caddy are active, and port 8281 listens.
  Home, registration, player panel, ranking, downloads, staff entry and
  readiness returned HTTP 200. The game warning journal was empty after
  restart. Caddy reported existing redundant-forwarded-header/HTTP-listener
  warnings, but configuration validation succeeded.
- Automated client/server evidence and the repaired full-race-run failure are
  recorded in the [port audit](../client/port-748.md#quest-countdown-and-fairy-duration-audit-2026-10-12).
  This is not executed-client visual validation or a global security/release
  certification. The legacy launcher's unsigned plain-HTTP feed remains a
  known limitation, not corrected by append-only publication.
- Patch 10003 was published after the server update; see the
  [launcher publication](../client/launcher.md#published-patch-10003-2026-10-12).
  The obsolete tracked `project.exe` was removed from Git; generated client
  executables are ignored. Player settings remain uncommitted locally.

Commands: `go test -race -count=1 -timeout=300s ./...`, the isolated JSON game
rerun and focused game/wire race gates, `go vet ./...`, portal generation,
tests/check/build, PIN tests, `Build-Client.ps1 -NoDeploy`, the final
ArchitectureTests build/run, `Build-VpsRelease.ps1`, staged database/restore
tests, `update-release.sh`, `Build-ClientPatch.ps1`,
`Publish-ClientPatch.ps1`, public HTTP/hash checks and repository layout/diff
validation. Manual launcher extraction, fairy duration and quest/instance
countdown checks remain pending; no automated gate is labeled `CLIENT_TESTED`.
