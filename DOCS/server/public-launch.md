# Public portal and native VPS deployment

This document is an append-only deployment ledger: initial setup and older
feature descriptions reflect their recorded dates. Use [current state](../current-state.md)
for the latest verified release and [website feature status](website-feature-status.md)
for the reconciled implementation matrix. Hidden Valley access is documented
separately; its current server/data implementation is included in the latest release.

## Scope and release gates

The public origin is `https://wyd.forjago.com`. The Spaceship A record `wyd`
points to `46.224.58.209` with a five-minute TTL. This is the same VPS as
Forjago/app-td; that application's Docker database and Caddy host remain
unchanged. WYD uses native Go processes and its own PostgreSQL 18 cluster
listening only on `127.0.0.1:5433`.

This batch is `MODERNIZACAO_COMPATIVEL`: the public HTTP API is new, but game
account payloads, password hashing, the native credential limits and wire
packets remain unchanged. No client or historical binary is patched here.
The initial database was created empty; no local accounts, characters, guild
registry, staff credentials or test accounts were imported. This is an initial
installation decision, not a claim that the live database is still empty.

The game listener is public on `0.0.0.0:8281` (ufw admits TCP 8281 only), and
the distributed `serverlist.bin` points every channel at `46.224.58.209`. The
VPS never serves the game client: outbound traffic is a cost, so the full
client is distributed through an external Google Drive link; the VPS serves
only the small incremental patches of the [launcher](../client/launcher.md).
The download page links that package through `clientRelease` in
`web/portal/src/data/public-content.ts`.
The site theme reuses 7.48 NewUI textures exported by
`tools/client-assets/export_portal_ui.py`.
Do not describe this deployment as a global client/server release: package
licensing and content review, the outstanding English migration and
critical-persistence recovery gates, and an actual remote game session remain
open. Automated checks do not mean `CLIENT_TESTED`.

## Website and HTTP contract v1

Static Astro routes include `/`, `/register/`, `/player/`, `/ranking/`,
`/download/`, `/play/` and its guides, `/news/` and its articles,
`/changelog/`, `/roadmap/`, and `/client-manifest.json`.
`/SrvCont.htm` is the static channel-population file named by
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
| POST `/api/player/v1/accounts` | CSRF session and server-validated Turnstile token; create an empty game account, no automatic login |
| POST `/api/player/v1/login` | Same CSRF session; native password verification, cookie and CSRF rotation |
| POST `/api/player/v1/logout` | CSRF session; revoke the cookie server-side |
| GET `/api/player/v1/profile` | Authenticated cookie; own account only, ignoring client-supplied account selectors |
| GET `/api/player/v1/sessions` | Authenticated cookie; own active website sessions, independent display IDs and timestamps |
| POST `/api/player/v1/sessions/revoke` | Authenticated cookie and CSRF; revoke one owned display ID, 404 for unknown or another account's ID |
| POST `/api/player/v1/sessions/revoke-all` | Authenticated cookie and CSRF; revoke all current website sessions of this account, including this session |
| GET `/api/player/v1/ranking` | Public, at most 100 persisted characters, cached for 30 seconds |
| GET `/healthz` | Process health |
| GET `/readyz` | One-second database reachability check |

Registration accepts 4–12 ASCII letters/digits for the account name and 8–10
printable ASCII password characters without spaces. The legacy wire limits
are explicit UI constraints, not a claim that short passwords are ideal.
Use a unique password. There is no email verification, recovery, payment,
account editing, staff permission, or gameplay mutation in this API.

Profiles expose username, saved premium balance, account creation and last
persisted timestamps, and character slot/name/class, internal zero-based level
and evolution. The UI displays level plus one. A single parameterized SQL
projection returns at most four characters without loading credentials, item
identities, inventory or cargo into the public profile result.
Ranking exposes character metadata only and orders by evolution, level,
experience and stable name/account keys. These are persisted snapshots, not
online presence or live combat state. No hashes, item identities, equipment,
cargo or private account names are returned in the ranking.

JSON responses carry `version: 1`. Creation returns 201, bad JSON 400,
unauthenticated profile/bad credentials 401, host/origin/CSRF or human-verification failure 403,
duplicate name 409, wrong content type 415, invalid registration 422, rate
limit 429, and backend/capacity failures 503. No failed write is reported as
successful; database details are not returned. Registration POST bodies are
limited to 4,096 bytes, including the at-most-2,048-byte `turnstileToken`;
other POST bodies remain limited to 1,024 bytes. Unknown fields and trailing
JSON are rejected. Lost responses are
not automatically retried by the UI: check login before repeating a creation.

## Security and lifecycle

### Current authentication hardening (2026-10-12)

`MODERNIZACAO_COMPATIVEL`: HTTP authentication is hardened without changing
native 7.48 packets or password derivation. This section supersedes the
process-local-only quotas and registration-only challenge described in the
historical deployment records below.

- Player and staff login require server-validated Turnstile tokens bound to
  the exact hostname and actions `login` and `staff_login`; registration uses
  `register`. Tokens cannot cross these actions. Missing/rejected tokens return
  403; missing protection or verifier/database outages return 503, before
  password lookup. HTTPS staff authentication cannot opt out of protection.
- Durable PostgreSQL quotas survive service restarts and are shared across
  processes. Each login scope admits 600 attempts globally, 100 per IPv4 or
  IPv6 /64 network, and 40 per account per hour. Registration admits five per
  network per hour, ten per day, and global limits of 30/minute, 300/hour and
  1,000/day. These are database-clock-aligned fixed windows; earlier in-memory
  limits remain additional burst/work controls. Rejections roll back all quota
  reservations, return 429 and provide `Retry-After`.
- Five registration attempts are a network limit, not verified ownership of
  five accounts per human. Shared NATs share quotas, and invalid attempts count.
- Published staff account creation requires a live authorized staff session
  and the six-digit PIN, and shares the public registration quota. Anonymous
  PIN-only bootstrap is restricted to explicit local HTTP development.
- New web-created passwords require 8–10 printable ASCII characters. Existing
  shorter passwords still authenticate; the native ten-character limit remains
  a constraint, not a claim of modern password strength.
- Schema version 6 adds only `web_auth_limits`. The player role may read/write
  these security counters but still cannot update/delete accounts, read game
  tables, execute DDL or assume the owner role. Parameterized SQL, strict JSON,
  CSRF, exact Origin/Host checks, secure cookies and bounded password work remain
  in effect. Identifier keys are hashed; secrets and challenge tokens are not
  logged.

Both root-only mode-0600 authentication environment files require the same
Turnstile secret. The installed `/usr/local/sbin/wyd-set-turnstile-secret`
reads a replacement from stdin, validates both paths and preserves unrelated
settings. For rotation on the VPS:

```bash
read -r -s -p 'New Turnstile secret: ' wyd_turnstile_secret
printf '\n'
printf '%s\n' "$wyd_turnstile_secret" | sudo python3 /usr/local/sbin/wyd-set-turnstile-secret
unset wyd_turnstile_secret
# Restart only after confirming there are no game players connected.
sudo systemctl restart wyd-game wyd-player
```

The release updater takes a database backup, checks active game connections
before a necessary restart, verifies both API readiness endpoints and retains
the previous release/configuration. Rollback from the additive version-6
migration to a version-5 binary removes only its version marker, not account
data or the security-counter table. Unknown migrations are never rewound.

Validation: full Go tests and vet passed; focused final authentication tests
and race tests passed. PostgreSQL tests executed against an isolated VPS
fixture proved concurrent admission, restart persistence, expiry recovery,
real player-role grants and rejection of forged game-state writes. The
fixture was removed afterward. Portal check/build and ten frontend tests
passed; five POSIX secret-installer tests and shell syntax/invalid-option
checks passed. Successful real player/staff sign-in after these new challenges
still requires an operator test; static/build checks do not prove that flow.

This is application hardening, not a penetration-test certification or a
volumetric-DDoS guarantee. Cloudflare HTTP proxy/WAF is not active on the
current direct-origin hostname. Game TCP protection and signed/authenticated
legacy-launcher updates remain separate deployment concerns.

The first publication attempt detected an established game
connection at 17:40 UTC and restored `/srv/wyd-go/releases/20261012T161218Z`
without restarting either service. Game PID 803621 and player API PID 918134
remained unchanged, schema remained version 5, and the account count remained
three. Both authentication environments retained mode 0600. WYD readiness and
the shared `forjago.com` site returned 200; no new warning journal entries
were present. The launcher feed hash was unchanged. The prepared candidate
`/srv/wyd-go/releases/20261012T174033Z` was not activated. A database backup
was taken before the attempt. No live account
or character data was removed; only the isolated validation database was
dropped after its tests.

Publication completed at 18:30 UTC in
`/srv/wyd-go/releases/20261012T183045Z`. The operator authorized a restart
even if players were connected, but the immediate connection check found no
established game connections, so the normal guarded updater sufficed. The
database backup completed successfully before publication. Game PID 944314
and player API PID 944364 started at 18:30:47 and 18:30:48 UTC respectively;
Caddy reloaded and the backup timer remains active. Schema migrated to 6;
the account count remained three. Actual player-role privileges permit
security-counter writes but deny account UPDATE/DELETE. Both environment
files remain root-owned mode 0600 and the installed rotation helper mode 0700.

Live HTTPS probes returned 200 for home, registration, player, staff, ranking,
downloads and readiness, and the shared `forjago.com` site remained available.
Authentication pages have their expected challenge actions and CSP. Requests
with valid CSRF but no challenge token returned 403 for both login APIs;
anonymous staff account creation returned 401. These probes used no real
credentials and created no account. No post-restart warning-level entries
were present in the game/API journal. The launcher feed hash is unchanged:
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`.
Only the exact temporary deployment staging directory was removed by the
updater; previous release/configuration backups remain available. The existing
game binaries, assets and public pages were reused as the baseline and only
the tested server/API binaries and built portal were overlaid.

Published artifacts: server SHA-256
`d959039d61125f2e156bbc50922e15dffbc1c4156f739a7285b1bac8516b6a69`,
player API SHA-256
`0c08cdc01476394a5273f9ecf3082e31a44918ce490468a18a1dc85f727d22f0`,
portal archive SHA-256
`5e3f8ec481c53f69ced533a9005550ea130d51938131e01f010533eaf1d97c8d`.
The live server/API hashes match these tested artifacts. Portal archive
identity was verified before unpacking; successful real challenge/sign-in
remains an operator acceptance gate rather than an automated-test claim.

### Turnstile registration verification (2026-10-12)

This hardening is `MODERNIZACAO_COMPATIVEL`: game packets, account schema,
credentials and password derivation are unchanged. The registration form
loads Cloudflare's explicit Turnstile widget with action `register`, English
text and the existing dark theme. Its public site key is bundled into the
registration page; `PUBLIC_TURNSTILE_SITE_KEY` can override it at build time.
The token stays in browser memory, is cleared after each submission, and is
invalidated on widget expiry, timeout or error. Failed script loading/rendering is
bounded to 15 seconds and keeps submission disabled. CSP allows the challenge
script and iframe only on registration paths, without inline/eval allowances.

The API requires `WYD_TURNSTILE_SECRET` at startup. It is stored only in
root-owned mode-0600 `/etc/wyd-go/player.env`, never in source, static output,
staging archives, command arguments or diagnostic logs. Install or rotate it
using `tools/deployment/vps/set-turnstile-secret.py`, which reads stdin and
atomically preserves the other environment settings. The initial installer
now requires that input before starting the player API. On the VPS, the helper
is available in `/tmp/wyd-deploy-20261012-turnstile/vps/`; future release
packages carry it in their `config/` directory. For an interactive VPS shell:

```bash
read -r -s -p 'New Turnstile secret: ' wyd_turnstile_secret
printf '\n'
printf '%s\n' "$wyd_turnstile_secret" | sudo python3 /tmp/wyd-deploy-20261012-turnstile/vps/set-turnstile-secret.py
unset wyd_turnstile_secret
sudo systemctl restart wyd-player
```

Do not paste keys into command literals, chat, Git or deployment archives.
Revoking/rotating the provider secret requires updating this environment and
restarting only the player API; API sessions and in-memory quotas then reset.
Restart is not an abuse-limit bypass procedure. The operator should rotate
any key disclosed through an unprotected channel. No secret is recorded in
this document or project memory.

Admission/CSRF checks precede verification. Invalid tokens return 403 before
password work or account lookup; provider/configuration failures return 503,
never bypassing verification. Siteverify uses a fixed HTTPS endpoint, trusted
peer IP, a three-second deadline, bounded response body and no redirects.
Success must match `wyd.forjago.com` and action `register`; Cloudflare handles
expiry/reuse. One in-flight verification leaves password slots free for login.
Five attempts per network per hour still apply, including verification failures;
this is not a verified five-account-per-person ownership cap or a volumetric
DDoS guarantee. Registration cannot work offline when the provider is down.

Published release: `/srv/wyd-go/releases/20261012T160139Z`. Player API SHA-256:
`9570e0b91fc8081ffe97c4b605b903eedeb82bf20a8e0c2d836174368145e2c9`.
The previous release was cloned and only public web content and the API were
replaced; game files were compared unchanged. Old hashed web assets remain
available for cached pages. Configuration rollback backups are root-only in
`/var/backups/wyd-go/config-20261012T160139Z-turnstile`. Game PID 803621 and its
04:14:01 UTC start time did not change. Only `wyd-player` restarted; Caddy
reloaded. No live account was created, and no database, client or launcher feed
was changed.

Validation is `AUTOMATED TESTED` and deployment-verified: portal check/build
passed with zero diagnostics; nine frontend tests passed; Go player-portal and
account tests passed with race detection, and focused `go vet` passed. Linux
registration/verification tests and POSIX secret-installer tests passed on the
VPS. Windows skips POSIX permission/symlink assertions rather than claiming
Unix permission evidence. Shell syntax and configuration checks passed.
Live probes with no account credentials rejected missing and invalid tokens
with 403. Home, registration, player panel, ranking, downloads and readiness
returned 200; CSP was inspected on live routes; game/API/Caddy are active and
the post-update API warning journal was empty. Existing Caddy forwarded-header
and plain-HTTP-listener warnings remain, without a validation error.

Commands: `go test -race -count=1 ./internal/playerportal ./cmd/player-api`,
`go test -race -count=1 ./internal/account`, focused `go vet`, `npm run check`,
`npm run build`, `npm test`, `test_turnstile_secret.py`, `bash -n`, Caddy
validation and live HTTP/hash/process checks, repository layout validation
and scoped `git diff --check`. The dedicated database integration fixture was
unavailable locally; schema/persistence were not modified. At this deployment,
a successful real challenge and account creation still needed a browser test;
the challenge-only gate was subsequently completed as recorded below. Passing
checks are not `CLIENT_TESTED` or a global security/release certification.

Registration initialization hotfix: `/srv/wyd-go/releases/20261012T161218Z`.
The real asynchronous provider script rejects `turnstile.ready()` (code 3857),
so the initial form failed before rendering its widget. The loader now calls
`render()` directly from the script's `onload` handler, preserving its bounded
timeout, fail-closed behavior and token lifecycle. The frontend fixture now
rejects `ready()` just like the live async provider; all nine tests pass.
Astro check/build passed. Public web archive SHA-256:
`b9bab25b58d0816935c239c0c1e5d4041e06299ff86add0e63722212f041457e`.
The atomic static-web deployment compared every non-web file unchanged and
preserved previous hashed assets. Neither service restarted (game PID 803621,
player API PID 918134); the launcher feed checksum was unchanged. Reloading
the live registration page showed the real widget's `Success!` state and an
enabled Create button. The original readiness warning did not recur; opaque
provider-frame console messages were present despite the successful widget.
No token was logged or submitted and no live account was created. Full account
creation remains an operator gate. No files were removed. Commands for this
hotfix: `npm run check`, `npm run build`, `npm test`, `bash -n`, guarded web-only
deployment, browser inspection, repository layout validation and scoped
`git diff --check`.

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

### Registration flood controls (2026-10-12)

This internal hardening is `MODERNIZACAO_COMPATIVEL`: native credentials,
account payloads, database privileges and game packets remain unchanged.
Registration admission is checked after Host/Origin/content-type/session/CSRF
validation but **before** JSON parsing, password derivation or database access.
All admitted attempts count, including malformed JSON, invalid credentials,
duplicate names, capacity rejection and backend failures.

| Scope | Admitted registration attempts |
| --- | --- |
| One IPv4 address or IPv6 /64 network | 5 per hour and 10 per 24 hours |
| Entire player API | 30 per minute, 300 per hour and 1,000 per 24 hours |

These are fixed windows starting with the first admitted attempt, not rolling
windows or calendar periods. Quotas are charged atomically across all scopes;
rejected requests do not charge another quota or extend the lockout. A 429
response includes `Retry-After` in whole seconds, rounded up to the longest
blocking window. Other public rate limits return a 60-second retry hint.
Changing usernames, cookies or IPv6 addresses inside a /64 does not bypass
registration limits. IPv4-mapped IPv6 addresses share the IPv4 quota. Shared
NATs and IPv6 /64s share limits; legitimate groups may therefore be limited.

The five-attempt hourly allowance is a network abuse control, not a lifetime
five-account cap per person. Invalid attempts also consume it; another network
or a later window can admit more accounts. Enforcing a per-person ownership
cap would require a verified identity and a separate account-ownership policy.

Registration has at most one in-flight operation within the two shared
password-work slots and does not consume the login global hash quota. Login
still shares public read/session capacity: this does not guarantee availability
under a distributed HTTP flood. Anonymous session bootstrap, read and login
limits also aggregate IPv6 /64s; anonymous cookies remain bound to the exact
IP. Caddy must keep overwriting X-Forwarded-For with the actual peer. Direct
public forwarding headers cannot override quota identity. Only GET health or
readiness checks directly from literal loopback without X-Forwarded-For bypass
public quotas; proxied/public probes do not receive this exemption.

The registration limiter has a separate 4,096-bucket cap and fails closed when
full, without evicting active quotas. Expired registration/read buckets are
swept at most once per minute instead of scanning the full map per request.
Both quota stores are process-local: restarting the API resets quotas and
sessions; multiple replicas would need a shared admission store. Operators
must not restart the service repeatedly to clear limits during an attack.

These controls bound application work and account creation, **not** volumetric
DDoS traffic before it reaches the VPS. Server-validated Turnstile is now
required, as documented below; bots rotating networks still encounter global
quotas. Before broad public promotion, review the VPS
provider's upstream DDoS protection. Do not enable a DNS HTTP proxy blindly:
the same hostname carries native game TCP and the plain-HTTP legacy launcher,
and proxying also changes trusted client-IP handling. Neither CSRF nor the
browser's disabled submit button is a bot challenge.

Validation: `AUTOMATED TESTED` with
`go test -race -count=1 -v ./internal/playerportal ./cmd/player-api`, the
unchanged account-service race tests, and focused `go vet`. Thirteen tests
cover network canonicalization, hour/day/global expiry, atomic concurrency,
bounded storage, malformed bodies, session/name rotation, proxy identity,
IPv6 rotation, CSRF rejection, login work capacity, local readiness and
amortized sweeping and five successful registrations followed by rejection of
the sixth. In the 60-request HTTP concurrency test, five requests
reached JSON validation and 55 received 429; no account was created. The
dedicated PostgreSQL fixture was unavailable locally and its integration test
was skipped, not treated as database evidence; persistence/schema code did not
change. The selected registration tests also passed in the Linux/amd64 test
executable on the VPS.

Initial deployment: `/srv/wyd-go/releases/20261012T151717Z`. Packaging reused
the prior release only after its complete 627-file game/web SHA-256 manifest
matched the deployed baseline, then replaced only `game/wyd-player-api`.
The API SHA-256 is
`ebc858712f4e81a1f1178243b055be23b165e72a4fb6eea10dc7fa2cdf7271b7`.
The normal checksum/rollback updater restarted only `wyd-player`; game PID
803621 and its 04:14:01 UTC start time remained unchanged. A live loopback
probe using a reserved documentation IP and empty JSON received
422/422/422/429 with `Retry-After: 3600`, without submitting an account payload.
The public home, registration, player panel, ranking and readiness paths
returned 200 afterward; the new API warning journal was empty. Existing Caddy
forwarded-header/plain-HTTP warnings remain; configuration validation passed.
No client, launcher feed, game binary, live account or database was modified.
At that initial release, human verification and upstream DDoS review were
pending. The later Turnstile release below adds verification, not a global
security or release certification.

Hourly-limit update (2026-10-12): `MODERNIZACAO_COMPATIVEL`, increasing the
network allowance from three to five without changing daily/global quotas,
game protocols or persistence. The focused player-portal/API race suite and
vet passed; the database fixture remained unavailable and was not counted as
evidence. The selected Linux/amd64 tests passed on the VPS, including five
successful in-memory registrations and rejection of the sixth. The current
release is `/srv/wyd-go/releases/20261012T152558Z`; API SHA-256:
`99e18c6b120ddc22ac140e4abdad20ce45cb8910ab157ff67360fc1349b69cdd`.
The checksum-verified API-only deployment copied the existing release,
verified unchanged game/web files and preserved the previous release for
rollback. Only `wyd-player` restarted; game PID 803621 remained unchanged.
A live empty-JSON probe on a reserved documentation IP returned five 422s
then 429 with `Retry-After: 3600`; it created no live account. No client,
launcher, database or infrastructure configuration changed.

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

## Website plan implementation — October 12, 2026

The phased backlog is [WYD web platform](../WYD-WEB-PLATAFORM.md), especially
the public pages, account/security sections and phase 1 in section 161.
This batch implements the
public MVP and website-session controls, not every later phase in that plan.
Classification: `MODERNIZACAO_COMPATIVEL`; game packets and account mutation
ownership are unchanged. The historical plan still contains Portuguese text;
it is untranslated documentation debt, not an English migration completion.

| Planned area | Implemented scope and remaining boundary |
| --- | --- |
| Public entry and information | Home navigation, getting-started, server information, classes, account/testing guidelines and FAQ. Unverified rates and hardware requirements are not invented. |
| Editorial pages | Versioned news index/articles, changelog and roadmap. Static content lives in `public-content.ts`; no runtime CMS or raw HTML input. |
| Downloads | Full-package link and installation/repair guidance; incremental patch version, size and SHA-256; a JSON download receipt. The full-package size/hash and launcher version remain unknown and are clearly marked. |
| Account overview | Own-account creation date, last persisted snapshot timestamp, saved character summaries and balance. No gameplay writes. |
| Account security | List active website sessions and revoke one or all owned sessions. This is not game disconnection, staff-session revocation or a device inventory. |
| Ranking | Existing persisted character ranking retained; read time and cache delay are explicit. New guild, event and competitive ranking projections are not implemented. |

The download receipt is `/client-manifest.json`, version 1. It does not replace
the legacy launcher's `/patch/update.htm`, update its version counter, or imply
signed-update verification. Its published patch-10004 ZIP size (811389 bytes)
and SHA-256 were checked against the VPS artifact. The complete package has a
separate external URL and no claimed checksum until that artifact is verified.

Session endpoints and the private profile are specified in `api/openapi.yaml`
(document version 0.5.0, response version 1); generated TypeScript types are
used by the UI. Session display IDs are random values independent from bearer
cookies and CSRF tokens. Listing discloses no IP address, user agent or tokens.
Reads and revocations recheck authentication while holding the session lock.
POSTs retain same-origin, content-type, size, strict JSON and CSRF checks.
Revocation of an already removed or foreign ID returns the same 404 response.
Self/all revocation expires the browser cookie; a lost POST response is shown
as unconfirmed, never as a rollback. Stale requests cannot restore private UI
after sign-out or an account switch. These sessions are process-local and an
API restart invalidates them; a later successful login can create a new one.

Remaining backlog is explicit: password change/recovery needs an authoritative
account command and a verified recovery identity; expanded databases need
validated drop, crafting and NPC relationships; events and additional rankings need server projections;
guild, support and marketplace need their permission/lifecycle contracts;
payments and rewards need idempotent authoritative delivery. These are not
placeholders presented as working features, and this batch does not close the
entire platform plan.

Validation:

- `go test -count=1 ./internal/playerportal ./cmd/player-api` and matching
  `go vet` passed; `CGO_ENABLED=1 go test -race -count=1
  ./internal/playerportal` passed. Tests cover ownership, CSRF, expiration,
  replay, restart invalidation, malformed payloads and concurrent revocation.
- `TestPlayerDatabaseIntegration` passed against PostgreSQL 18 on the VPS in
  the isolated `wyd_portal_validation` database with the production role
  separation. This exercised profile projections/timestamps and existing
  registration/ranking flows without writing real accounts. The temporary
  database was dropped and its absence was verified.
- `npm run generate`, `npm run check` (40 files, zero errors/warnings/hints),
  `npm run build` (17 pages) and `npm test` (17 tests) passed. Browser checks
  inspected the public home/download presentation at desktop and 390-pixel
  widths. A static preview cannot prove authenticated live UI behavior.
- Source/build validation is complete for this batch; publication of this
  website/API candidate and authenticated live-session UI verification remain
  pending. No game binary, launcher feed or live account was changed by this
  batch. No product files were removed.

## Website catalogue and ranking — October 13, 2026

Classification: `MODERNIZACAO_COMPATIVEL`. This continues the public MVP above;
it does not alter game packets or introduce gameplay writes from the website.

- `/database/` contains 3,093 items and 103 skills from the current server
  tables. Search by name or exact ID, class filters, 20-row pagination and
  stable detail URLs use a compact anonymous index. Query parameters retain
  the current filters; navigation back restores them. Item effects are
  collapsed by default. The presentation reuses the modernized 7.48 theme.
- `cmd/web-export` reuses the authoritative table loader, checks book/skill
  bindings and publishes an explicit static-field allowlist. Four source
  SHA-256 hashes, schema version 1 and a deterministic content hash identify
  the snapshot. Export fails on inconsistent inputs without replacing the
  previous artifact. There are no database credentials, account projections,
  absolute paths or timestamps in the catalogue. Generated JSON is ignored
  by Git and regenerated by portal dev, check and build commands.
- `/database/catalog.json` is the full public snapshot;
  `/database/index.json` contains only IDs, names and skill classes for search.
  HTML and search data must have the same content hash. A failed or mismatched
  search index leaves static links intact and reports a generic unavailable
  message; names are rendered as text, never executable HTML.
- Base item values and static effects are not rolled item bonuses or shop
  prices. Base skill mana/cooldowns can be overridden by skill-specific rules.
  Drop sources, crafting, equipment eligibility, trade restrictions and
  runtime instance values remain outside this projection. ASCII validation
  alone does not prove an English migration of all source data.
- Ranking filters name, class and evolution within the published top 100,
  preserving global positions. Pagination, refresh and failures do not
  expose private errors or retain a stale ranking as a successful refresh.
  Download repair and website sessions use collapsed secondary sections.

Validation for the integrated website batch:

- Focused Go tests passed for `internal/webcatalog`, `internal/data`,
  `internal/playerportal` and `cmd/player-api`; matching catalogue/player API
  vet checks passed. Existing website-session race and isolated PostgreSQL
  integration results above remain valid; no catalogue export opens a database.
- Portal check: 54 files, zero errors/warnings/hints. Build: 3,216 pages.
  Portal tests: 29 passed, none skipped, covering catalogue input identity,
  search failures, safe rendering, ranking lifecycle, all generated links and
  public-page CSP compatibility.
- Browser checks exercised exact ID search, Foema filtering, pagination,
  skill/book navigation and expandable static effects. Detail pages had no
  horizontal overflow at a verified 390-pixel viewport. Authenticated
  live-session UI verification remains a manual gate; no real account is
  created or modified for these checks.
- Publication receipt and live service checks are recorded below. This is
  not full-platform completion, native-client visual
  proof or global security certification. No product files were removed.

Published release: `/srv/wyd-go/releases/20261013T061221Z`, built from
`wyd-deploy-20261013-website-catalog`. Archive SHA-256:
`35b63035e1188e5028fe839f32dbe16c87f714a7d122b025e2e7fe65b605c770`.
The live game, player API and public catalogue file hashes match the packaged
builds. Catalogue content hash:
`f25456b656d8abd86b01a0bab21657ef75f2e64d382033c5387324c18294d734`.

Backup completed before installation. The updater's established-connection
gate allowed the game restart; the player API also restarted. All three
services are active, 8281 listens and both loopback readiness endpoints return
200. Public catalogue, guides, news, changelog, roadmap, download, ranking and
account entry pages return 200. The live ranking API returns response version
1 and its browser filter preserves global positions. No game/player warning
journal entries appeared after restart. Caddy retained its existing redundant
forwarded-header and separate HTTP-listener warnings; validation succeeded.
The launcher feed SHA-256 is unchanged; website publication requires no client
patch. Website sessions are process-local and were invalidated by the API
restart. Existing game accounts and character data were not reset.

Commands: focused Go tests/vet listed above; `npm run check`, `npm run build`,
`npm test`, `Build-VpsRelease.ps1`, `update-release.sh`, service/HTTP/hash/journal
checks, `Test-RepositoryLayout.ps1` and scoped `git diff --check`. The local
preview and temporary browser viewport were cleaned up after validation.

## Website quest catalogue — October 13, 2026

Classification: `MODERNIZACAO_COMPATIVEL`. The public database now includes
18 configured NPC quests at `/database/quests/`, with name/ID search,
requirements, consumption, rewards, prerequisites and item cross-links.
Secondary item lists and next quests are collapsed by default, preserving
the minimal modernized 7.48 presentation.

- Schema version 2 adds the quest projection and hashes six inputs, including
  `quests.json` and `quest_zones.json`. The exporter reuses both authoritative
  loaders, verifies public item references and rejects prerequisite cycles
  and unsupported counter bindings. The catalogue remains read-only and
  contains no account data or player progress.
- Internal zero-based levels become displayed levels. Gold requirements are
  shown as costs; configured EXP is identified as a base reward. Citizenship,
  equipped-item refinement and Kefra recharge/entry consumption preserve the
  server's configured semantics. Configured quest-zone destinations identify
  the global reset, not a personal ten-minute duration.
- This is a configuration snapshot, not a guarantee that an interaction will
  succeed. Runtime validation, inventory capacity and character limits still
  apply. NPC locations, monster drops and crafting are not inferred from these
  quest definitions; their separate public projections remain pending.

Validation:

- `go test -count=1 ./internal/webcatalog ./internal/data` and
  `go vet ./internal/webcatalog ./cmd/web-export` passed. Tests cover real
  quest projections, default quantities, displayed level bounds, citizenship,
  refinement, recharge, invalid references and prerequisite cycles.
- Portal check: 56 files, zero errors/warnings/hints. Build: 3,235 pages.
  `npm test`: 31 passed, none skipped. Generated quest routes, search-index
  identity, prerequisites and reverse item links are covered.
- Browser checks verified exact ID/name search, item cross-links, prerequisite
  and next-quest navigation, initially collapsed secondary lists and no
  horizontal overflow at 390 pixels. Published search and Gravedigger details
  were also exercised. No authenticated account or native client was modified
  for these checks.

Published release: `/srv/wyd-go/releases/20261013T062822Z`, packaged as
`wyd-deploy-20261013-website-quests`. Archive SHA-256:
`a33a656b2ba2e83be3c21a863654f19bcf00b49ed38d8aade8a878f3bb41b5dc`.
All six source hashes match the packaged runtime data. Catalogue content hash:
`b4bb23acfdaf81e4e3e7cbca3c0c6a5d2af511cb11d049f9fc1e05a2f15fd3f4`.
The live catalogue file hash matches the package:
`218b81e8faca7530d370bbb83954847189d893c35af4146bc94bd89b52f93c97`.

Backup succeeded before publication. No established game connections were
present; the updater's connection gate allowed game/API restarts because the
bundled website changed. Game/API executable hashes are unchanged from the
preceding release. All three services are active, 8281 listens and both
loopback readiness checks return 200. Quest list/detail, item and catalogue
endpoints return 200. No game/API warning journal entries appeared after
restart. Existing Caddy warnings are unchanged. The launcher feed hash is
unchanged; no client patch is required. API restart invalidates process-local
website sessions; existing game account and character data were not reset.

Commands: the focused tests/vet and portal checks above;
`Build-VpsRelease.ps1`, packaged-source verification, `update-release.sh`,
service/HTTP/hash/journal checks, `Test-RepositoryLayout.ps1` and scoped
`git diff --check`. No product files were removed. Remaining platform features
are not represented as completed by this batch.

## Private character details — October 13, 2026

Classification: `MODERNIZACAO_COMPATIVEL`. Selecting a character in the
account panel opens a read-only saved-state projection: class, evolution,
displayed level, EXP, gold, CP and fifteen equipment slots. Details remain
hidden until a selection. Equipment names link to existing item pages;
unknown names have a text-only item-ID fallback. The Database navigation
entry remains removed as requested; catalogue routes are not deleted.

`GET /api/player/v1/characters/{slot}` accepts only selectors 0–3 belonging
to the authenticated website account. Query-string account names are ignored.
One SQL statement reads the projection and account update timestamp together.
The response is version 1, explicitly identifies a persisted snapshot and
uses `Cache-Control: no-store`. Internal item/character identities, credentials,
inventory, cargo and effective combat calculations are excluded. Equipment
effects preserve the six protocol bytes without interpreting them as combat
attributes. The API loads item names from the packaged authoritative tables;
missing tables fail startup rather than presenting fabricated names.

Selection generations and session checks discard superseded replies. New
requests, errors and logout clear private details. The UI never edits game
state and does not label saved base attributes as live combat statistics.
The minimal presentation retains the existing modernized 7.48 textures.

Validation:

- `go test ./internal/playerportal ./cmd/player-api` and matching `go vet`
  passed. `go test -race ./internal/playerportal` passed. The server imports
  this package for staff verification; its focused tests and vet also passed.
- Ownership tests cover anonymous requests, malformed selectors, other-account
  attempts, hidden identities, generic dependency errors and mutation rejection.
- Local database integration was skipped because its dedicated fixture was
  unavailable locally. The same integration executable subsequently passed on
  the VPS in `wyd_portal_validation`, exercising real PostgreSQL projections,
  restricted privileges, cancellation, empty/missing slots, equipment effects,
  CP and unsigned gold values above signed 32-bit range. An initial fixture
  failure correctly rejected a copied cross-account item UID; the fixture was
  corrected to give the second account no equipment and this gate was rerun.
- The production backup restored successfully into `wyd_restore_validation`.
  Both disposable databases were removed by the validation cleanup. No test
  account was inserted into the production database.
- Portal check: 57 files, zero errors/warnings/hints. Build: 3,235 pages.
  `npm test`: 35 passed, none skipped. Four new browser-controller tests cover
  safe text/links, stale replies, logout/account changes and malformed equipment.
- Browser interaction with an isolated local HTTP fixture verified selection,
  refresh, fifteen equipment slots and clearing all private details on logout.
  At 390 pixels the document did not overflow horizontally. The screenshot at
  `build/website-validation/character-panel-mobile.png` uses fictional data,
  not a real production account. The preview process, temporary viewport and
  verification tabs were closed/reset. Authenticated production interaction
  remains a manual gate; no native client validation was attempted.

Published release: `/srv/wyd-go/releases/20261013T065025Z`, packaged as
`wyd-deploy-20261013-character-details`. Archive SHA-256:
`03a489e9f3a2425e86c279d56303e3306a5ef52fce2703e4d7ee616036a60af7`.
Live executable hashes match the package:

- Game: `53588e8b36bcdd7f996e142f1b58daf23d452e096eb6d649aa63ebeb92e3944a`.
- Player API: `e7be983492a43e4dd69ad19865df2ec5517ebe2fc6cffdf5b3ed6107925ab00b`.

The installer checked for active game connections before restarting game/API
services; the gate allowed the restart. Backup and readiness checks passed.
Game, API and Caddy are active; 8281 listens and both loopback readiness checks
return 200. The published account page returns 200; anonymous character reads
return 401. No game/API warning journal entries appeared after publication.
Caddy retained its existing forwarding-header/HTTP-listener warnings, with a
successful configuration validation. The launcher feed hash remains
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`;
no client patch is needed. The API restart invalidated process-local website
sessions. Existing account and character data were not reset.

Changed areas: character API/projection and tests, API startup catalogue loading,
OpenAPI/generated types, account controller/page/style and documentation. No
product file was removed. Commands include the focused gates above,
`Build-VpsRelease.ps1`, isolated database/restore validation, `update-release.sh`,
service/HTTP/hash/journal checks, layout validation and scoped diff checks.
Effective combat attributes, inventory/cargo views and other platform-plan
features remain separate pending work; this batch does not complete the plan.

## Private Inventory and Cargo — October 13, 2026

Classification: `MODERNIZACAO_COMPATIVEL`. This batch adds the read-only
storage views from platform-plan section 69. Inventory is available after
selecting a character; Cargo belongs to the authenticated account. Both
sections are collapsed initially and fetch only when opened. The existing
modernized 7.48 presentation is reused without a new navigation entry.
Database remains absent from the main menu; existing item links still work.

`GET /api/player/v1/characters/{slot}/inventory` accepts character selectors
0–3. `GET /api/player/v1/cargo` has no owner selector. Both derive ownership
from the authenticated session, ignore query-string account names, return
version 1 persisted snapshots and use `Cache-Control: no-store`. A single
SQL statement returns each allowlisted collection and its account timestamp.
Internal UIDs, credentials, item timestamps and reserved ABI cells are
excluded inside PostgreSQL. Inventory exposes 63 usable slots; Cargo exposes
120 visible slots and unsigned 32-bit gold. API slot selectors are zero-based;
the UI labels them starting at one and renders occupied cells only.

Quantities come from instance effect 61, not static catalogue defaults.
Mount IDs 2330–2389 reuse that effect for HP and display quantity one.
Refresh, collapse, character selection and logout invalidate old requests;
session checks reject late replies from a previous account. Private DOM
content clears on logout and errors. No endpoint mutates game state, items,
gold, persistence versions or the game-session lifecycle. OpenAPI is 0.7.0.

Validation:

- Focused player-portal/API tests, race tests, server tests and matching vet
  passed. Tests cover ownership, anonymous requests, malformed selectors,
  dependency errors, reserved cells, instance quantities and read-only behavior.
  All effect-byte values were exercised, including mount-range boundaries.
  The final integration gate, `go test -count=1 ./...` followed by
  `go vet ./...`, also passed. Locally unavailable database fixtures remained
  skipped; the dedicated player-portal database gate below supplied real SQL
  evidence for this change.
- The real PostgreSQL integration passed in the disposable
  `wyd_portal_validation` database on the VPS. It exercised restricted access,
  cancellation, absent accounts/characters, empty storage, highest visible
  slots, reserved-cell filtering, saved timestamps and gold 4,000,000,000.
- A production backup restored into `wyd_restore_validation`, containing four
  accounts. Both validation databases were removed by cleanup. No production
  test account was created and existing account data was not reset.
- Portal check: 60 files, zero errors/warnings/hints. Build: 3,235 pages.
  `npm test`: 39 passed, none skipped. Four new storage-controller tests cover
  lazy reads, safe names/links, quantities, stale replies, failures and logout.
- A local browser fixture verified Inventory/Cargo expansion, refresh,
  selection and logout. At 390 pixels there was no horizontal overflow.
  `build/website-validation/storage-panel-mobile.png` records fictional data,
  not a production account. The preview process and temporary browser tab
  were closed, and the viewport override was reset. Authenticated production
  interaction remains a manual gate; no native client test was attempted.

Published release: `/srv/wyd-go/releases/20261013T071137Z`, packaged as
`wyd-deploy-20261013-storage-view`. Archive SHA-256:
`47c8fe4e6fd1d3a97e09c9769bdb7c14f8603a895a44dc156b5033b43bf7cf22`.
Live hashes match the package:

- Game: `0f47408ed26bbdeefdb3a8025947479e4bf6c3eeaaf79779ba5398b27054891e`.
- Player API: `1d8c5a291a6c1bb2fa7508239ea444babfd5e1d0519993dc28ce451606351976`.

The installer checked for game connections before restarting game/API.
Backup, configuration and readiness checks passed. Game, API and Caddy are
active; 8281 listens and both Host-validated loopback readiness checks return
200. The published account page returns 200 with both storage disclosures;
anonymous storage requests return 401 and `no-store`. Game/API warning
journals contain no entries after activation. Caddy retains its existing
forwarding-header/HTTP-listener warnings; configuration validation passed.
The launcher feed remains at patch 10004 with SHA-256
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`.
No client patch is required. The API restart invalidated process-local
website sessions. The upload stage was removed; its local bundle and older
VPS releases remain available for recovery.

Changed areas: storage projection/handlers and tests, backend interface,
OpenAPI/generated types, account storage controller/integration/page and this
delivery record. No product file was removed. Commands include focused
Go/race/vet gates, portal generation/check/build/tests, browser verification,
`Build-VpsRelease.ps1`, isolated database/restore validation,
`update-release.sh`, service/HTTP/hash/journal checks and repository checks.
Other platform-plan features and effective combat attributes remain pending;
this batch does not complete the entire website plan.

## Saved Arch progression advisor — October 13, 2026

Classification: `MODERNIZACAO_COMPATIVEL`. Platform-plan section 70 is partially
implemented against real character snapshots and shared server rules, starting
with the Mortal-to-Arch transition. The compact account section is collapsed
initially, uses the existing modernized 7.48 theme and adds no menu entry.

The character response adds `ascension` in OpenAPI 0.8.0 (response version 1).
It reports applicability, displayed required level, five named checks and
whether all saved requirements are met. The server and website share
`model.EvaluateArchRequirements`: persisted Mortal evolution, internal level
370 (displayed 371), Eternal Stone 1742 in equipment slot 10, class Sefirot
1760–1763 in slot 11 and a free character slot among 0–3. Fame and CP are not
Arch gates in this server and are not invented from the plan's example.
Items held in Inventory or Cargo do not satisfy the equipped-item checks.

The allowlisted SQL projection reads equipment, level, raw evolution, slot
availability and the account timestamp in one statement. Raw evolution and
occupancy context remain inside the adapter; only derived checks are returned.
Ownership, no-store, bounded query timeout and existing API admission limits
are unchanged. The website has no ascension command and changes no account,
item or gold. The existing king interaction keeps its validation order and
transaction/persistence lifecycle. A saved checklist is not live authorization
or a guarantee that character creation and persistence will succeed.

Selection, refresh, failure and logout clear the advisor through the existing
character controller. Non-Mortal characters have no Mortal-to-Arch section.
Malformed advice cannot render a successful checklist; an older API without
the new additive field leaves the previous character view usable.
Celestial advice is covered by the following batch. SubCelestial advice and
other progression paths remain pending.

The real SQL gate also exposed a null character-list regression: a valid saved
account with no characters could break profile and ranking reads. Both SQL
projections now treat missing/JSON-null lists as empty arrays. A disposable
account exercises the empty profile and verifies that ranking still returns
the other accounts' characters. Persisted account data is not rewritten.

Validation: full Go tests and vet passed, followed by focused player-portal
tests/vet and race tests after the SQL correction. Model rule tests and
player-portal ownership/rejection tests passed. Portal check reported 62 files
with zero errors/warnings/hints; build produced 3,235 pages; 43 UI tests passed
with none skipped. Local browser checks used fictional data: collapsed initial
state, character selection, logout clearing and no overflow at 390 pixels.
`build/website-validation/arch-advisor-mobile.png` records that fixture, not a
production account. Authenticated production interaction remains a manual
gate; no native client execution is claimed.

The PostgreSQL integration passed in `wyd_portal_validation`, including
restricted-role projections and the equipped-item/free-slot gates. Backup
restore validation recovered four accounts into `wyd_restore_validation`.
Both disposable databases were removed; no production test account was made.

Published release: `/srv/wyd-go/releases/20261013T074314Z`, package
`wyd-deploy-20261013-arch-advisor-sql`, archive SHA-256
`1f11378d1dbd4599160782e4f710fa25277ec93148d874e23302716541571170`.
Live game/API hashes match the package:

- Game: `227df6c163133a7bccf65b0d488322389d0630e35d7eddaae0c45dda4eca4a1e`.
- Player API: `c770a4047512f4acac6bb0dd9879585c69f4dc79dad841aaa8bd07b1e22a719d`.

The installer checked for established game connections before restarting both
services. Game, player API and Caddy are active; game TCP 8281 listens and both
readiness checks return 200. The HTTPS account page includes the advisor and
has no Database menu entry. Its HTML and published script match local hashes;
anonymous character reads return 401 with `no-store`; public ranking succeeds.
Game/API warning journals contain no entries after activation. Caddy retained
its existing forwarding-header/HTTP-listener warnings and passed validation.
Process-local website sessions were invalidated by the API restart.

Launcher patch 10004 is unchanged, with feed SHA-256
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`;
this website/server batch requires no client patch. The successful upload
stage was removed by the installer; local packages and previous releases are
available for recovery. No product file was removed.

Changed areas: shared Arch rules and king consumer, saved character projection
and profile/ranking null handling, tests, OpenAPI/generated types, advisor and
character-controller integration, account page and this record. Commands
include Go tests/vet/race, portal check/build/tests, browser validation,
`Build-VpsRelease.ps1`, `test-database.sh`, `update-release.sh`, HTTP/hash/service
and journal checks, repository layout validation and `git diff --check`.
Other platform-plan features remain pending; this batch does not complete the
entire website plan. Existing catalogue routes remain available for item links
even though Database is no longer a public navigation entry.

## Saved Celestial progression advisor — October 13, 2026

Classification: `MODERNIZACAO_COMPATIVEL`. Platform-plan section 70 now also
includes a compact Arch-to-Celestial checklist. It uses the existing theme,
starts collapsed and introduces no menu item or website evolution command.
The Database navigation entry remains removed; equipment catalogue links
continue to work.

OpenAPI 0.9.0 adds `celestialAscension` to the version-1 character response.
Four derived checks cover Arch evolution, internal Arch level 355 (displayed
356), saved source Mortal level 399 (displayed 400), and Ideal Stone 5338 in
usable Inventory slots 0–62. Equipment, Cargo and reserved ABI slot 63 do not
satisfy the Inventory gate. The SQL projection uses the authenticated account
and one saved snapshot; raw inventory and cached source level stay private.

Shared model constants replace identical game-operation literals, preserving
rejection order, transformation/trade validation, item consumption and critical
persistence. The checklist is informational, not authorization or a promise
that a live evolution will succeed. Fame, CP and crystals are not invented as
additional requirements. Selection, refresh, failure and logout clear the
advice; malformed responses fail closed and older responses without the
additive field keep the character view usable.

Validation: focused model/game/player-portal tests, full Go tests and vet, and
model/player-portal race tests passed. Portal generation, check and build
passed: 64 checked files, zero errors/warnings/hints, 3,235 built pages and
46 UI tests with none skipped. The PostgreSQL integration passed under the
restricted player role in `wyd_portal_validation`, including level boundaries
and a deliberately populated reserved slot. Backup restoration recovered four
accounts in `wyd_restore_validation`; both disposable databases were removed.
No production test account was created or existing account rewritten.

Local browser checks used fictional characters: initial collapsed state,
three visible requirements, missing Ideal Stone and logout clearing passed.
The observed narrow viewport had equal client/content widths (375 pixels).
The browser could not export a full-page screenshot; no screenshot artifact
is claimed. Authenticated production interaction remains a manual gate.
No native client execution or in-game parity validation is claimed.

Published release: `/srv/wyd-go/releases/20261013T080308Z`, package
`wyd-deploy-20261013-celestial-advisor`, archive SHA-256
`d985f5940b63e1ca1cb586abfecbe8175c91cd8ab2e97e0730d3198a14d73381`.
Live hashes match the local package:

- Game: `6224161bb246f6f10721af4f4c8de7f5fb96913d87352b04431ec01c049bbf66`.
- Player API: `7cd455ca67b6f94e7863a0e2218eb9516d5798c0b161ae78e5e0a13ec72f2aa5`.
- Account HTML: `4a803b5a1d282d3cc7f22089f4352ebd46340c4025b6d00116d6f5e996e00665`.

The installer found no established game connection before restarting the game
and API. Game, API and Caddy are active; TCP 8281 listens and both readiness
checks return 200. Published account HTML includes the advisor and no Database
navigation entry. Anonymous character reads return 401 with `no-store`; public
ranking returns 200. Game/API warning journals contain no entries since
activation. Caddy retains its existing forwarding-header/HTTP-listener
warnings and passed configuration validation. The API restart invalidated
process-local website sessions.

Launcher patch 10004 and its feed hash
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`
are unchanged; this server/website batch needs no client patch. The installer
removed the exact upload stage; the local bundle and previous VPS releases
remain available for recovery. No product file was removed.

Changed areas: shared Celestial requirements and existing game consumers,
character projection and tests, OpenAPI/generated types, advisor/controller
integration, account page and this record. Commands include Go tests/vet/race,
portal generation/check/build/tests, browser checks, `Build-VpsRelease.ps1`,
`test-database.sh`, `update-release.sh`, live HTTP/hash/service/journal checks,
repository layout validation and `git diff --check`. SubCelestial advice and
other platform-plan features remain pending; section 70 is not complete.

## Saved SubCelestial advice and Guilds navigation — 2026-10-13

Classification: `MODERNIZACAO_COMPATIVEL`. The main navigation now includes
Guilds and excludes Play and Database. Existing guide and catalogue URLs are
retained; no product file was removed. The new `/guilds/` page uses the existing
public `/patch/guilds.txt` client export, with name search and 20-name pages.
This is a name directory, not a guild ranking or member-management API. No
account, membership or private guild data is exposed. Invalid, duplicate,
foreign-channel and oversized exports fail closed; unsuccessful refreshes
clear stale names and allow retry. Requests omit credentials. The current
public export is empty, so the page displays `No guilds yet.`.

OpenAPI 0.10.0 adds saved SubCelestial advice to the version-1 character
response. Eight checks cover Celestial evolution, internal level 120 (display
121), character Fame 100, equipped Sefirot 1760–1763, Ideal Stone 5338 in usable
Inventory, no existing alternate, valid appearance and room for the ten-stone
reward. Fame comes from the selected character's saved sidecar, never CP,
another character or an account balance. Consuming a single Ideal Stone may
free its slot; existing reward stacks do not substitute for an empty slot.
The checklist starts collapsed and is invalidated on selection, refresh
failure and logout. It is advice only; authoritative in-game validation and
critical persistence still decide whether creation succeeds.

Schema v7 adds the narrow `player_character_progression` read view. The player
role can SELECT the view but cannot read or mutate the underlying sidecar
table. The view validates the counter format and binds UID to account ownership.
The v6-to-v7 migration does not replay the historical account migration.
Integration tests block account writes during upgrade, compare complete
account rows, exercise repeated v7 startup and reject an unknown future schema
without altering its marker. Deployment rollback retains additive tables/views
but restores the known previous marker before starting an older binary.

Validation: full Go tests and vet passed; store/player-portal race tests passed.
The earlier unchanged model/game race checks remain valid. Portal check passed
with 69 files and zero errors/warnings/hints; build produced 3,236 pages and
all 54 portal tests passed. An initial content test correctly failed against
an old build after an incorrect log-output path prevented building; correcting
that path and rebuilding resolved it. The dedicated PostgreSQL integration
passed, including malformed Fame, ownership isolation, appearance, inventory
capacity, reserved-slot and privilege cases. Backup restoration recovered four
accounts; both disposable validation databases were removed. No production
test account was created or existing account rewritten by the migration.

Published package: `wyd-deploy-20261013-subcelestial-guilds-v7`, archive SHA-256
`18ae042d2b97b564ee4b8847938787d43278f8d487b76b505ef8601c4f283048`, release
`/srv/wyd-go/releases/20261013T154637Z`. Local and live hashes match:

- Game: `76453e554813aa9f9ef5ff112fe26afe3eaed4a13ba751a70bca6f998b8eb824`.
- Player API: `bf5e0099a8e037528da3eca33d0864d7ed32d6b5a9f435b68d55650c87ac490b`.
- Account HTML: `0e29cdf0f80ec56a8a75573fa795b36af0973c7cc2bc7e5b409f991bc03e6bec`.
- Guilds HTML: `5c12a7589c293a9b0ab41904890166042eb24e7eb06f227e50683a7c9b6329c6`.

No established game connection was present at the preflight or installer
restart guard. Game/API restarted; Caddy reloaded. Both readiness probes
return 200, all three services are active and TCP 8281 listens. Live HTTPS
Home/Guilds and the public guild export return 200; the served main navigation
has Guilds and neither Play nor Database. Anonymous character reads return
401 with `no-store`; ranking returns 200. Schema marker is 7 and live
privileges confirm view SELECT allowed and sidecar SELECT denied. Game/API
warning journals contain no entries since activation. Existing Caddy
forwarding-header/HTTP-listener warnings remain; configuration validation passed.

Launcher patch 10004 and feed SHA-256
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0` remain
unchanged: this website/server batch requires no client patch. The exact upload
stage was removed; previous releases and local packages remain recoverable.
Affected files include the shared SubCelestial model and game consumers,
character projection, migration/privilege/deployment scripts, OpenAPI and
generated types, advisors, Guilds page/controller, navigation and their tests.
Commands include Go tests/vet/race, portal check/build/tests,
`Build-VpsRelease.ps1`, `test-database.sh`, `update-release.sh`, live HTTP,
schema/privilege/hash/service/journal checks, layout validation and scoped
`git diff --check`. Authenticated production UI interaction and nonempty live
guild rendering remain manual gates. No native-client visual test is claimed.
Guild membership/leadership management and other platform-plan features remain
pending; neither the whole platform plan nor section 70 is declared complete.

## Saved character Fame display — 2026-10-13

Classification: `MODERNIZACAO_COMPATIVEL`. Platform-plan section 66's Fame
field is now available in the authenticated character details. It is a
character-specific uint32 counter, separate from CP, guild Fame and the
account's premium balance. The existing owner-bound progression view supplies
the value in the same saved snapshot used for SubCelestial advice; this change
adds no schema migration, privilege or game rule. `asOf` still includes the
counter's saved timestamp. Only the selected account's counter is returned;
raw sidecars, inventory, other currencies and internal identities stay private.

OpenAPI 0.11.0 adds optional `fame` to the existing version-1 character response.
New responses always include the value. Older responses omit its display;
the browser never infers it from CP or renders missing data as zero. Malformed,
negative, fractional and overflowing values fail closed and clear private DOM.
Selection, failed refresh and sign-out retain the existing stale-reply guards.
The presentation adds one Fame row, with no extra explanatory panel or menu.

Validation for this batch:

- `go test -count=1 ./internal/playerportal`, `go vet ./internal/playerportal`
  and `CGO_ENABLED=1 go test -race -count=1 ./internal/playerportal`: passed.
- OpenAPI generation, the six focused character controller tests, Astro check
  (69 files, no diagnostics), build (3,236 pages) and all 55 portal tests:
  passed. The earlier full Go suite remains evidence for unchanged packages;
  it was not rerun for this allowlisted read-only field.
- The updated Linux integration binary passed `TestPlayerDatabaseIntegration`
  on the VPS disposable database in 3.24 seconds. Coverage includes distinct
  character Fame, absent counters, owner binding, uint32 maximum, malformed
  saved counters, view-only privileges and migration rejection. Backup restore
  succeeded with four accounts; both disposable databases were removed.
- Repository layout and links passed with 188 documents. Scoped diff whitespace
  and authored English checks passed. No product source or asset was removed.

Published package: `wyd-deploy-20261013-character-fame`, archive SHA-256
`5f191a4fff4594d2d7ddae4936e2798f2f75165fb1f13d423f40903bcdfd6afc`.
Live release: `/srv/wyd-go/releases/20261013T155500Z`. Local and installed
game/API/Account HTML hashes match:

- Game: `6a6263106ea41e546e5518aa673d873886ac451e144ba7d0cd5c966a3831c41e`.
- Player API: `4e83443c647305d61018a2e0c22b73d1d5c147efad03dba6432251bfcba07c11`.
- Account HTML: `4882b9714a6f550a4e9bc7f90c41140e47f4d0e38cfa46b1301820843f2e58ab`.

Preflight and the installer guard found no active game TCP connections before
restart. Game, player API and Caddy are active; both loopback readiness checks
pass. Public Home, Account and Guilds return 200; anonymous character access
returns 401 with `no-store`. The menu retains Guilds and excludes Play and
Database. Game/API warning-level journals contain no entries since publication.
The first post-release probes had a local reserved-variable error and an SSH
SQL quoting error; corrected probes confirmed the navigation and database
cleanup. These probe failures did not affect the release or modify live data.

The installer removed only the exact temporary upload stage; the local package,
database backups and previous release remain recoverable. Launcher feed hash
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`
is unchanged: this web/API feature does not require a client patch. Existing
Caddy advisory warnings about forwarded headers and the HTTP listener remain;
configuration validation passed.

Changed areas: character DTO/projection, OpenAPI/generated types, character
controller, corresponding Go/DOM/database tests and this delivery record.
`STATICALLY VERIFIED`, `AUTOMATED TESTED` and deployment/readiness are verified.
Authenticated production browser presentation remains a manual gate; no
production account was created or edited for testing and no `CLIENT_TESTED`
claim is made. The remaining platform backlog, including authoritative guild
profiles and resolved effective combat score, is still open.

## Saved public guild profiles — 2026-10-13

Classification: `MODERNIZACAO_COMPATIVEL`. The Guilds directory now links to
public saved profiles with kingdom, guild Fame and a searchable, paginated
member/role list. Ally and declared war target appear only when present; a
declaration is not presented as active war or war history. The shared navigation
retains Guilds and excludes Play and Database. This delivers part of platform-plan
section 47, not the complete guild feature set.

Schema 8 adds a security-barrier public projection of the canonical guild
registry. The player API can SELECT that view but cannot SELECT the raw registry.
Private account keys, join timestamps and war payloads are excluded in SQL.
The 8 MiB projection bound rejects oversized snapshots rather than truncating
them. The migration does not rewrite accounts or guilds; unknown creation dates
remain compatible with the native saved model and are not displayed.

Anonymous `GET /api/player/v1/guilds/{name}` returns a version-1 allowlisted
snapshot with its actual saved timestamp. Invalid names return 400, missing
guilds 404 and malformed or unavailable data 503 without internal details.
Concurrent reads share one cancelable load; successful snapshots have a
60-second cache and failures a five-second cooldown without stale fallback.
HTTP responses use `no-store`. The browser validates identities, integer ranges
and member roles, uses text-only rendering, and clears stale data after failures.

Validation for this batch:

- Full `go test -count=1 ./...` and `go vet ./...` passed. Focused store/player
  tests and vet passed after diagnostic translation; player portal race tests
  passed with `CGO_ENABLED=1`.
- OpenAPI generation, all 60 portal tests, Astro check (72 files, no diagnostics)
  and build (3,237 pages) passed. Five profile tests passed again after the final
  strict kingdom validation adjustment.
- VPS disposable PostgreSQL integration passed in 3.33 seconds, including
  public projection privacy, permissions, corruption, cancellation, bounded
  reads, additive v6/v7 upgrades, no-rewrite tripwires and future-version
  rejection. Backup restore succeeded with five accounts. Both disposable
  databases were removed; no production account or guild was created for tests.
- Caddy configuration and deployment shell syntax passed. The installer rejected
  an invalid staging directory before making changes.

Published package: `wyd-deploy-20261013-guild-profiles-final`, archive SHA-256
`3a691986e8d6ccb2f7a1f6969f4d042624f1dbe5e94d17f4fe68cc8c733c9258`.
Live release: `/srv/wyd-go/releases/20261013T161447Z`. Installed game, player
API and both guild HTML artifacts match the local package hashes. Preflight and
the installer guard found no established game TCP connections before restart.
Game, player API and Caddy are active; readiness probes return 200. Public pages
and profile shells return 200 with the expected navigation, CSP and cache policy.
Missing/invalid guild requests return 404/400 with `no-store`; anonymous private
profile access returns 401. Schema marker 8 and view-only privileges are verified.
Game/API warning journals contain no entries since activation. Existing Caddy
configuration advisories remain; configuration validation passed.

The exact temporary upload stage was removed; local packages, backups and
previous releases remain recoverable. No product source or asset was removed.
Launcher feed SHA-256
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`
is unchanged because this web/API batch requires no client patch.

Changed areas: guild projection/migration, player API/cache, OpenAPI/types,
Guilds directory/profile/controller, database/Go/DOM tests, privileges, deployment
routing and this delivery record. `STATICALLY VERIFIED`, `AUTOMATED TESTED` and
deployment/readiness are verified. Nonempty production guild presentation remains
a manual browser gate; no native `CLIENT_TESTED` claim is made. Guild marks,
territories, historical rankings/wars, achievements, membership management and
other platform-plan gaps remain open; saved profiles do not implement write flows
or live member presence.

## Authenticated inactivity and shared VPS security — 2026-10-13

Classification: `MODERNIZACAO_COMPATIVEL`. The ten-minute game disconnect was
an application read deadline, not evidence of a DDoS attack. Successful World
account admission now removes the waiting-for-packet deadline immediately,
including an already parked reader. Authenticated players may remain idle.
The legacy configuration key bounds only the absolute unauthenticated lifetime;
packets cannot extend it. Frame completion still has a deadline from its first
byte, and admission cannot clear an incomplete frame's deadline. Handshake,
flood, connection, outbound queue and TCP keepalive protections remain active.
No client packet, asset or launcher update is needed.

The SSH jail previously used an all-ports UFW ban, which also blocked the website
and game for an IP with failed SSH authentication. The deployed override scopes
bans to the verified OpenSSH profile (TCP 22) and preserves retry/ban limits.
`tools/deployment/vps/harden-ssh-jail.sh` validates the profile and configuration,
reloads only the SSH jail and restores the prior configuration on activation
failure. A controlled documentation-address ban created only an OpenSSH rule;
the address was unbanned immediately. A stubbed invalid-profile test rejected
installation before changes. Key-only, non-root SSH authentication remains in
effect.

Hetzner firewall `forjago-public-services` is applied to server 169421488. Its
five inbound rules allow TCP 22, 80, 443 and 8281 plus ICMP, for IPv4 and IPv6;
other inbound traffic is dropped and outbound access is unchanged. Server
deletion/rebuild protection is enabled. PostgreSQL, staff/player API listeners
and Caddy administration remain loopback-only. These measures do not prove
immunity to denial of service or replace application-level protections.

Validation: full `go test -count=1 ./...` and `go vet ./...` passed. Focused
network/login/admission regression tests also passed under the race detector.
Tests cover idle authenticated packet resumption, unauthenticated expiry,
absolute pre-auth deadlines, partial-frame timeouts and unsuccessful admission.
A Linux baseline build with only this patch reversed exactly matched the
previous installed game hash. Before publishing, every packaged game file
except the server binary, the public website and the player API matched the
existing release. No unrelated pending source or website changes were shipped.

Published package: `wyd-deploy-20261013-idle-security`, archive SHA-256
`dcbdb63b133b9583436ff72f0f16b7ebba6737ede4cd1ecedd56b7d25a9051c0`.
Live release: `/srv/wyd-go/releases/20261013T181017Z`; game SHA-256
`55e1cf92aff35249e4fdd5ed1fea54aea3ec5adcbd8bf7e4cafabd0cbe5d3397`.
Preflight and the installer guard found no established game connections.
The game restarted at 18:10:20 UTC (15:10:20 America/Sao_Paulo); the player API
restarted against the same binary, and Caddy reloaded its unchanged configuration.
The database backup and previous release remain recoverable. Game/player/Caddy
and Fail2ban are active; WYD home/readiness and Forjago HTTPS return 200; the
app-td database remains healthy. Warning journals have no new game/API entries;
shutdown reported zero persistence failures. Existing Caddy advisories remain.
Launcher feed hash remains
`5f31e286fd7bbddf14a6b1367957175cf23424f487ebd17b6fd03ffbe08137f0`.

`STATICALLY VERIFIED`, `AUTOMATED TESTED` and deployment/readiness are verified.
Manual in-game inactivity beyond ten minutes and subsequent movement remain
pending; no `CLIENT_TESTED` claim is made. No product source or asset was removed.
Only the validated temporary upload stage was removed; local packages, backups
and earlier releases are preserved. No Git commit or push was requested here.

## Catalog presentation and Hidden Valley release — 2026-10-13

Release `20261013T202540Z` (package SHA-256
`527486b9a9c17b8ec3793c4e507a439b5c50281b193ca1a709b9c51b3d27e2a3`) was applied
with `update-release.sh` after confirming zero game connections and a
successful `wyd-backup.service` (`wydgo-20261013T202539Z.dump`). It deploys the
whole local server tree, so it also publishes the previously local
[Hidden Valley access](hidden-valley-access.md) with the KR mount and
LavaGolem/LavaGrub catalog rows that pair with launcher patch 10006. The
website receipt advertises 10006. `staff.json` and `server.txt` matched the
live copies before installation.

The game (PID `1198636`, started `20:25:44 UTC`) and player API restarted and
are active; `/readyz` is ready, port 8281 listens, the public site returns 200
and the game/player journal had no warnings in the first minutes. Remote
staging was removed. Pending: in-game confirmation of Hidden Valley access,
the icons, a BeastMaster equipping a KR mount and both replaced mobs.

## Nightmare entry and area countdown release — 2026-10-13

Release `20261013T223204Z` (package SHA-256
`d3d73b2fa3c56f5fcfa0851604e9c72c807c65cf6938f16f76d8d8f4aeb95ff6`, game binary
`0a47c9ccf05142cfcf059d7b4f0a0883214f38015daa12e526039f7a496415b5`, player API
`47e92a02d997fa0d3df6df4717bd6143af09d921735cf2f6905eee5c25911b45`) was applied
after zero game connections and a successful backup
(`wydgo-20261013T223126Z.dump`). It carries exact-tile critical instance
reservations (Nightmare first waves), the Game Room clear area with the shared
reset countdown, the Hidden Valley access countdown and the website receipt for
launcher patch 10007. A first invocation piped through a malformed `grep`,
was terminated by SIGPIPE before creating a release and changed nothing; the
second run completed. The game restarted at 22:32:07 UTC; `/readyz` is ready,
8281 listens, the site returns 200 and the journal had no warnings. Full Go
suite, vet and 60 portal tests passed before packaging. Pending: in-game
confirmation of Nightmare entry, the Game Room and Hidden Valley timers.

## Staff /goto release — 2026-10-13

Release `20261013T235920Z` (package SHA-256
`1d58c90ab8baf8ed69a74a1bebeb40038c7d50097813ed7d01dc57edf68f197e`, game binary
`7420f0f7cd8483e59dbe645d83fd46f6dd852712f99b430206ed30a71a57766b`) adds the
[staff `/goto` command](operations.md#staff-in-game-commands) and grants
`game.command.goto` to `flptr`. It was applied after zero connections and
backup `wydgo-20261013T235919Z.dump`; the game restarted at 23:59:23 UTC,
`/readyz` is ready and the journal had no warnings. Full Go suite and vet
passed. Releases older than the current and two previous ones were pruned.
Pending: in-game use of `/goto`.

## Arch and citizenship publication — 2026-10-14

At the user's request, the complete current server/site tree was packaged with
`Build-VpsRelease.ps1` and installed by `update-release.sh` as
`/srv/wyd-go/releases/20261014T011828Z`. Source identity remains `2f2381f9`
plus uncommitted changes. The [citizenship contract](../../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md)
records the new Arch sanitation and commit-before-publication behavior.

- Package SHA-256: `ee87322eeeed76096789bae625bedcb09ddc47f0fe273b7de8443c242e7baa8b`.
- Game SHA-256: `f99842302ee3daae0ad3e54e5aa70abd6ef70010ce8d29b4ba813ebcf1b7c5e1`.
- Player API SHA-256: `1b532cb6fac81250596bd97bfea43dcc8db1daad507efc790fd1071dc7d19b8d`.
- Backup: `wydgo-20261014T011828Z.dump`; previous release: `20261014T003714Z`.

Preflight and the installer restart guard found zero established game connections.
Game PID `1253162` and player API PID `1253175` started at 01:18:31 UTC.
Both readiness endpoints returned ready, services are active and TCP 8281 listens.
Public home/download/changelog and Forjago returned 200; the manifest advertises
patch 10011. The fresh patch ZIP and enclosed executable matched their recorded
hashes. No warning-priority game/API journal entries or persistence errors were
observed after publication; the old game shut down with zero save failures.

Validation reused the unchanged server full-suite/vet/race and client build
results, then ran portal check (72 files, zero diagnostics), build and 60 tests.
`test-database.sh` passed against isolated PostgreSQL fixtures, including a backup
restore with five accounts. Schema remains 8; accounts were not reset. Caddy
validation passed with existing nonfatal proxy-header/HTTP-listener warnings.

Remote upload stages and disposable databases were removed by the installers.
Local publication staging and verification downloads were removed after receipts
were recorded. Current incremental client outputs, live patches, backups and
previous releases remain intact. No Git commit/push or client visual test occurred.
Manual Arch/citizenship, gate, Valley and launcher extraction acceptance remains
pending; this publication is not a claim of global release readiness.

## Existing cape-symbol backfill — 2026-10-14

At the user's request, release `20261014T020943Z-cape-emblems` replaced only the
game and player API binaries, copying the preceding release's site/config/data.
It supports persisted `capeEmblem` independently of actual citizenship.

- Game SHA-256: `127628cb6b38fa22c0ff8f6de972d0717fbb1f98535a20c6c1208400c47a8529`.
- Player API SHA-256: `10c055e7b23fb1c65a06d04e7b849af8076e72e33fc1330f22092e3a2a2ae8ed`.
- Previous release: `20261014T011828Z`; backup: `wydgo-20261014T020944Z.dump`.

Two connection checks found no established game sessions. Both services stopped,
the shutdown reported zero save failures, and a fresh backup passed archive-list
validation before the SQL backfill. Both new binaries were selected before the
new JSON field committed. The transactional
[backfill](../../tools/deployment/vps/backfill-cape-emblems.sql) assigned two
characters across two accounts (symbols 5 and 6). Existing valid symbols, neutral
capes and unrelated account/item state are preserved. The second pass changed
zero accounts/characters; a live query found no eligible cape without a valid
symbol. Each changed account version increments once.

Game PID `1262340` and player API PID `1262342` started at 02:09:44 UTC. Both
readiness checks passed, WYD/Forjago returned HTTP 200, and the bounded post-restart
warning journal was empty. Full Go suite/vet and focused race tests passed before
deployment. This does not certify client visuals or a database restore drill.

Old strict JSON readers reject `capeEmblem`: a binary-only rollback is unsafe.
Keep compatible binaries during investigation; a data rollback requires stopped
services and a separately approved restore/removal plan preserving later saves.
At backfill completion launcher remained 10011. The subsequently authorized
[client patch 10012](../client/launcher.md#published-patch-10012-2026-10-13-americasao_paulo)
published the repair candidate and updated only four website receipt pages, with
a backup. Game/API process identities remained unchanged; no service restart
or repeated backfill was needed. Visual acceptance is still pending.

## Critical scale release — 2026-10-14

Release `20261014T040646Z` keeps Critical in tenths of a percent: items add
their tooltip value, the roll is `intn(1000) < Critical` capped at 1000, and
skill/buff bonuses were doubled to keep their displayed percentages. It is
paired with client patch 10013; site receipt advertises 10013.

- Full Go suite and vet passed; portal build and 60 frontend tests passed.
- The installer refused the first attempt because one game connection was
  active and restored the previous release. With the user's authorization the
  game was stopped (`1 accounts persisted, 0 failures`) and the release applied;
  game and player API restarted, readiness returned 200 and the bounded warning
  journal was empty. Game binary SHA-256:
  `479df914c6292c3dfca9f777a37855b901fb874978d3263845f194802a7b6fb8`.
- Previous release `20261014T020943Z-cape-emblems` remains for rollback; do
  not pair the new server with a pre-10013 client.

## KR mount release — 2026-10-14

Release `20261014T045628Z` adds 40 KR mounts to the catalog, ShopKRMt04
(generator 5226) and the News post for patch 10015. Game binary unchanged
(`479df914…`). The first attempt failed readiness and restored the previous
release automatically: `Build-VpsRelease.ps1` packages only files known to
Git, so the new `npcs/ShopKRMt04.json` was missing and the generator had no
template. The file was marked intent-to-add (`git add -N`) and the second
attempt went live with zero game connections; readiness returned 200.

## Costume and refinement release — 2026-10-14

Release `20261014T083646Z` blocks refinement of costumes and familiar-slot
items, adds eight costumes and ShopCostum06 (generator 5227), and publishes the
patch 10016 News post. Zero game connections; game and player API restarted;
readiness 200. Game binary SHA-256
`8c5116d9bf6ec5223c597abf69d64d2684eb64af2f35a183798c22fd48be0220`.
Full Go suite, vet, portal build and 60 frontend tests passed.

## Transformations, Armia birth and kingdom guards — 2026-10-14

Release `20261014T160122Z`: transformation glow from Nature and level, Armia
birth point 2091,2100 ±5, Archs bound to Armia, template hometown Armia, and
kingdom guards that skip their own kingdom. Game binary SHA-256
`4340fc23b431ef0d0a1de0dcacb42f5758ad1b9a3518de8ad55e75d546537798`.

With zero game connections both services were stopped and `wyd-backup.service`
succeeded (16:00:57 UTC) before `tools/deployment/vps/rebind-nippleheim-to-armia.sql`
ran: its self-test passed, 4 characters in 3 accounts were rebound from
Nippleheim to Armia, and a later query found zero Nippleheim-bound
characters. The release then restarted game and player API; readiness 200,
no warning entries. Full Go suite, vet, portal build and 60 tests passed.

## Transformation face-update release — 2026-10-14

Release `20261014T162737Z` re-sends the owner's visual equipment after the
Kibita citizenship and kingdom face updates while a transformation is active,
and publishes the patch 10018 News post. Zero game connections; game and
player API restarted; readiness 200; no warning entries. Game binary SHA-256
`c36ac9b86aaf4618f00a0757ec05060cea195058bf6e607afb26eca7dea002ed`.

## Combat, cape and Kefra release — 2026-10-14

Release `20261014T181536Z` is paired with launcher patch 10019 and its website
manifest, News and changelog. It deploys the corrected attack-speed aggregation
and 900 ms action limit, authoritative Kefra mirror state, and Survivor's
50-visit allowance with mandatory ground-portal balance validation and durable
visit/gold debit. The immediate Survivor teleport spends the first visit,
leaving 49; a spare seal does not bypass a zero ground-portal balance.

The owner authorized disconnecting players and restarting. At 18:15:36 UTC,
shutdown persisted two connected accounts with zero failures. The release
installer took a successful backup before activating the new release; isolated
player PostgreSQL integration and a restore drill had already passed against
disposable databases (seven restored accounts, both databases cleaned). No
production account backfill or manual schema rewrite was performed.

- Game PID `1425880` and player API PID `1425893` started at 18:15:39 UTC,
  with zero automatic restarts at verification. `/proc/1425880/exe` resolves to
  the new release, and TCP 8281 listens.
- Game SHA-256: `fcf93b9c0cde7abd452696cb1bf7598edec255dd1eb8df8eff0c93b1efd0fbc3`.
- Player API SHA-256 (unchanged): `10c055e7b23fb1c65a06d04e7b849af8076e72e33fc1330f22092e3a2a2ae8ed`.
- Release archive: 21,831,226 bytes, SHA-256
  `7887e3da95696b505e412a30be18c4cedb1ec1cd44352fc7a7e92338daf108ae`;
  the remote checksum matched before activation.
- Both readiness endpoints returned 200; game, player API and Caddy were active.
  The bounded game/player warning journal since startup had no entries. Caddy
  validation passed with existing redundant proxy-header warnings.
- Public feed, fresh patch download and contained executable hashes matched;
  the home page, News and changelog returned 200. Portal build and 60 tests
  passed. Prior Go/client evidence is recorded in
  [current state](../current-state.md#published-combat-cape-and-kefra-batch--2026-10-14).

The previous release `20261014T162737Z` and previous patch artifacts remain for
recovery, subject to existing persisted-field compatibility constraints. Remote
release/patch staging was removed by the deployment scripts. SSH briefly timed
out during the first patch upload; the unchanged public feed was checked and
publication resumed after connectivity returned. No security limits were
disabled. Source remains the uncommitted working tree at HEAD
`2f2381f9a8e77f23eb07bb10b326df76568d3bdb`; this is not a committed release tag.
No client visual test was performed and global release readiness is not claimed.

## Kefra passage and EXP follow-up — 2026-10-14

Release `20261014T184657Z` and launcher patch 10020 are live. The client adds
the missing Uxmal activation-table entry; the server adds the owner-requested
global +100% combat EXP while Kefra is defeated and the temporary 1 HP QA
profile. No forced spawn, weekly reset or manual account/database mutation was
performed. Restore both Kefra HP fields to 28000 after QA through the normal
deployment flow.

The owner explicitly authorized disconnecting the sole connected player.
Graceful shutdown at `18:46:57 UTC` saved one account with zero failures.
The installer then completed a database backup and activated the new release.
Game PID `1431886` and player API PID `1431898` started at `18:47:00 UTC` with
zero automatic restarts. Both readiness endpoints returned 200, TCP 8281
listened, and `/proc/1431886/exe` resolved to the new release. The bounded
post-start game/player warning journal was empty. Caddy validated with the
existing redundant-header/HTTP-listener warnings. PostgreSQL retained schema
8 and `wars.kefra.defeated=true`, guild 1, defeat timestamp
`2026-10-14T18:30:36.160110947Z`.

- Game binary SHA-256: `bbfbd469fcffa698bb076ac9b1de6e39dc8b343d02e7fb2602e04164bc67a9c4`.
- Player API SHA-256, unchanged: `10c055e7b23fb1c65a06d04e7b849af8076e72e33fc1330f22092e3a2a2ae8ed`.
- Release archive: 21,832,354 bytes, SHA-256 `fb3bf7be00df580aa31bfab0835df0177398022f142858067d90ee9cae11c82b`.
  The remote checksum matched before activation. Runtime data/configuration
  comparison against the previous package found only `npcs/Kefra.json` changed.
- Public patch: 823,488 bytes, SHA-256 `AEF03DE00BA5E05127CEF380BD562E7CAD1D0CE4BB3E5E4D0EC0715ABE372784`;
  its sole `WYD.exe` is 1,870,848 bytes, SHA-256 `E291656AF11620EA1535C25F9381D19EF53A220E74ECDBB4B99650C1613428E6`.
  A fresh public HTTPS download matched both hashes; the append-only feed,
  website manifest, home, download, News and changelog agreed on 10020.

The previous release `20261014T181536Z` and earlier patch artifacts remain for
rollback. Remote release and patch staging were removed by the deployment
scripts. Linux cross-build, portal build, Astro check (72 files with no
diagnostics) and all 60 portal tests passed. Unchanged Go/client evidence was
reused from [current state](../current-state.md#published-kefra-passage-activation-repair--2026-10-14),
including persistence failures, weekly lifecycle and the red/green table
regression. No schema, player API or persistence contract changed, so the
prior isolated PostgreSQL/restore evidence was not rerun as a publication ritual.

Source remains the uncommitted working tree at HEAD
`2f2381f9a8e77f23eb07bb10b326df76568d3bdb`, not a committed release tag. The
exact validated client bytes were packaged explicitly because the Git-revision
builder cannot include the ignored executable. No local runtime install or
game launch occurred. Actual portal use and the HP/EXP gameplay checks remain
pending; global English migration and release readiness are not certified.
Local publication-stage cleanup was rejected by the execution policy before
execution. `build/deploy/wyd-deploy-20261014-kefra-passage-10020/` and
`build/patch/patch-10020.zip` remain in ignored build roots; no bypass was used.
Remote staging was successfully removed, and published/rollback artifacts were
preserved.

## Pending shop, dye, chat and Lac batch — 2026-10-14 22:37 UTC

The owner requested deployment and restart before crash repair. Existing
validated server/API binaries from `wyd-deploy-20261014-lac-10022` were reused
unchanged, with the portal rebuilt for launcher patch 10023. Release
`20261014T223717Z` is active. It includes armor-only dye, channel chat routing,
the test shops and Lac +10 to +11 at 5%; no crash fix is included.

- Archive: 21,840,421 bytes, SHA-256
  `0e38caab586be772b64c54c3670958343db1f91661b391c8f1064956d5495072`.
- Game SHA-256: `3BC4032585D6B0B418C396436126752418C22FCCFFE664A1DBF0562FE5835108`.
- Player API SHA-256: `30A9974502197B60984CC3870DFB0A147E56A4B3B233349C35791BB404D0CC15`.
- Game PID `1471572` and player API PID `1471585` started at
  `2026-10-14 22:37:21 UTC` (19:37:21 America/Sao_Paulo).
- Backup: `/var/backups/wyd-go/wydgo-20261014T223717Z.dump`, 89,759 bytes,
  verified by the backup script's `pg_restore` listing. Previous release
  `20261014T184657Z` remains available for rollback.

Three active game TCP connections were observed before the authorized restart;
game/player were stopped gracefully before the installer backup and switch.
Remote archive checksum and live executable hashes matched. Both loopback
readiness endpoints, public readiness, public News and the shared ForjaGo home
returned 200. Game, player API and Caddy were active; TCP 8281 listened. The
bounded post-start game/player warning journal had no entries. Caddy validation
passed with its existing redundant-header and HTTP-listener warnings.

Focused game tests passed, including dye/chat/refinement consumers, gem-variant
preservation and actual Lac use success/failure. The portal build and all 60
tests passed. The unchanged client reused its recorded Release gates; a fresh
public patch download verified ZIP/executable hashes and every feed entry
10000–10023. No local client installation or visual validation occurred.

Remote staging created by this publication was removed by the installers.
The older, pre-existing `lac-10022` staging was not removed by this task. Source
remains an uncommitted working tree at
`2f2381f9a8e77f23eb07bb10b326df76568d3bdb`; this is not a committed release
tag, `CLIENT_TESTED`, or global release-readiness certification. The reported
item-drop crash and its preserved dump remain the next implementation task.

Local publication-stage cleanup was rejected by the execution policy before
execution; no bypass was attempted. This task's leftovers are
`build/deploy/wyd-deploy-20261014-ready-10023/`,
`build/patch/patch-10023.zip`, and the verification download directory
`C:/Users/xereca/AppData/Local/Temp/wyd-publish-10023-ea7a6a24cfa643f296dd37eb4b13bc25/`.
They do not affect the live service or preserved rollback artifacts. Repository
layout and local links passed (198 documents); the scoped diff check passed.

## Mount, interface and equipment follow-up — 2026-10-14

The owner explicitly authorized publication and disconnecting all players.
Shutdown at 23:42:36 UTC saved three accounts with zero failures. Package
`wyd-deploy-20261014-mounts-ui-10024` activated release `20261014T234237Z`;
game PID 1482267 started at 23:42:39 UTC and player API PID 1482279 at
23:42:40 UTC, both with zero automatic restarts. Public readiness returned 200
and the post-start warning-level game/player journal was empty.

Archive: 21,846,033 bytes, SHA-256
`e76bf8343a412c461682e09b9747e1bf3a47fb7421daa7751acbb9fba4781264`.
Installed game SHA-256:
`11CF8801358AC935DC69A05FB5D6E6E5E5252043ACFFC7B0C975436A07C8ECBD`.
The player API is unchanged. Backup
`/var/backups/wyd-go/wydgo-20261014T234237Z.dump` is 87,197 bytes and its restore
listing was validated by the installer. Prior release `20261014T223717Z`
remains available for rollback; account and world state were not reset.

The working-tree batch includes mount spell amplification, normal Lac +10
earrings, Noatum ordinary tower suppression, Erion tower placement and updated
mortal D/E test stock. All three changed weapon-shop files matched packaged
and live hashes. The matching client patch is 10024; site receipt, public ZIP
and embedded executable hashes match. Full Go tests/vet and client build/checks
from the preceding implementation were reused; Linux cross-build, portal build
and 60 portal tests passed for this publication. Both remote staging directories
were removed. The temporary SSH timeout was recovered without duplicate feed
entries. This is not CLIENT_TESTED or global release-readiness certification;
the reported item-drop crash and complete Noatum war remain open.

## Guild marks and gameplay release — 2026-10-15

Release `20261015T023211Z` (game binary SHA-256 prefix `620b8ec08c36f8df`) is
paired with launcher patch 10026. With the owner's authorization the game was
stopped first (`1 accounts persisted, 0 failures`); the installer then
restarted the game and player API, validated Caddy and passed readiness.

- Guild marks: player API `-guildmark-dir /var/lib/wyd-go-guildmark`
  (`wyd-player`, 755); Caddy serves `/patch/guildmark/b<group><channel><guild>.bmp`
  from `<guild>.bmp` as `image/bmp`, 404 otherwise. A temporary file proved the
  mapping for two groups and was removed; the directory is empty.
- Server: native support-buff duration, Magic Chamber and Nightmare waves from
  `data/NPCGener/Carta*.txt` and `Nightmare*.txt`, training-field level-35
  return/block, and D/E weapon and armor-set test shops placed in Armia.
- Checks: services active, port 8281 listening, public `/readyz` 200, warning
  journal empty, home/News/player 200, anonymous `/api/player/v1/guild` 401,
  manifest advertises 10026. Remote and local staging removed. Not `CLIENT_TESTED`.

## Player-report release — 2026-10-15

Release `20261015T215714Z` (bundle SHA-256
`1c692d4434f965c6dbdf3ade12dc0fe660e119bc4fdc0486f5ef9633fa8909c6`) is paired
with launcher patch 10029. With the owner's authorization (players
disconnected) the game was stopped first, then the installer restarted the
game and player API, validated Caddy and passed readiness. The remote staging
was removed by the installer; the local staging was deleted.

- Server: Aylin W2PP grade/gem rules with Ancient weapons, second Cythera at
  the level-90 unlock, earring Lac +9→+11 at 16% with breakage, earring gems,
  Golden Shield 100 gold per mastery, Samaritan self-only, bow for skills
  75/79, sub-leader InviteType 1–3, summons following teleports, 5–10 minute
  support buffs, 5–15 minute Soul Limit feeding derived stats, and Con 1 for
  64 monster templates.
- Website: receipt 10029 and News `player-report-fixes-10029` (portal build
  and 60 tests passed). Not CLIENT_TESTED.
