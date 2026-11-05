# Current project state

Updated: 2026-11-02. Activation and admission were verified at 22:07 BRT (2026-11-03 01:07 UTC).

## Recorded active release

Release `20261103T010701Z` runs one game process per channel:

| Service | Channel | Game port | Language |
| --- | --- | --- | --- |
| `wyd-game@1` | 1 | 8281 | Portuguese; staff panel enabled |
| `wyd-game@2` | 2 | 8282 | English |

Both derive their configuration from `/etc/wyd-go/server.txt` through `wyd-channel-configs`.
The server minimum is client revision 100. The full client with native HTTPS
launcher was verified on Google Drive before baseline feed 48100 and server activation.
Schema v20 was activated by R4 (`20261101T145133Z`).

All 1,076 game payload files, both running game hashes, readiness and both channel
admission checks passed. Revisions 18 and 99 are rejected; revision 100 reaches
credential validation in each channel's language. No real account was authenticated.
Backup `wydgo-20261103T010701Z.dump` passed hash and PostgreSQL archive validation.
Only game channels restarted. Player API, log reader, private settings and schema
v20 remain unchanged. See the [release receipt](../.agents/research/release-748100-https.json).
The earlier [R5 isolation evidence](../.agents/research/release-history.md#channel-processes-r5--activated-2026-11-01) remains valid for the process architecture.

## Corrections and acceptance

| Scope | Latest recorded state | Owning document |
| --- | --- | --- |
| W0–W7: diagnostics, persistence, login, output pressure, health and activation cost | Activated in R1–R3; COMMIT closed-connection hotfix activated as `20261101T064639Z` | [Correction plan](server/server-correction-plan.md) |
| W8–W9: participant rows, shared ownership and channel processes | R4 activated schema v15–v20; R5 activated separate channel processes | [Correction plan](server/server-correction-plan.md) |
| BUG-001–BUG-011 | Closed by the owner's client confirmation on 2026-11-01; no session capture is attached | [Bug register](bug-register.md#queue) |
| Load acceptance | Owner accepted a local two-process, 600-player simulation; this is not a 600-player VPS observation | [Load evidence](bug-register.md#load-simulation--2026-11-01-two-channel-processes-600-players) |
| Unknown persistence outcomes | Account-only quarantine can recover from final XID status; shared-object recovery remains restart-based | [Persistence recovery](server/critical-persistence-plan.md#current-gaps-and-acceptance-criteria) |

## Published November 2 corrections

The published client, active server, source and local installation use the
owner-selected baseline 7.48.100. Its external HTTPS-launcher package is verified.
The feed starts at 48100; earlier feeds and patch ZIPs are archived outside the
served tree. Subsequent updates start at 48101. See the
[baseline receipt](../.agents/research/client-baseline-748100-20261102.json).
The owner authorized local installation on November 2 and reported several minutes
of farming without apparent failures. This is partial OWNER-REPORTED CLIENT-TESTED.
The November 2 crash and mount incidents now have tested source corrections.
The mount server corrections are active. New gameplay acceptance remains pending.
See BUG-012–BUG-015 in the [bug register](bug-register.md#queue).
Performance measurements remain pending.
The installed local baseline-100 `tmproject/client748/WYD.exe` has SHA-256
`2287425438FC995D740205BE53280C0931ABA7E5876BEBED39552A28D64223C4`.
The archived revision-18 game retains SHA-256
`F8E4F9A2D73458C0B72072F9EF5F53F110C2EFD0A6D7CC9EB03FB3587DBCC3B8`.

- Client remediation, all twelve later authorized improvements and the empty
  channel-list correction are implemented. Release and Debug gates passed
  during remediation; the final channel-list correction has a passing Release
  gate. Debug predates that final correction.
- The owner reported the previously supplied fifteen manual stages and two
  special scenarios functional. This is `OWNER-REPORTED CLIENT-TESTED`, without
  per-step captures. It does not close unrelated deferred or partial contracts.
- [Private guild areas](guild-wars.md#private-guild-areas-inside-cities) now use
  authoritative city ownership and canonical membership. Full Go tests, vet,
  focused race tests, isolated PostgreSQL and the server build passed. This
  correction is active; targeted gameplay acceptance remains pending.
- The dated client ledger has 271 records: 13 owner deferrals, 19 evidence
  deferrals and 36 partial records. These are not a current active-bug count.
  Reconcile them against later resolutions, acceptance and the November 2 batch.
- The asset gate now passes inside `Build-Client.ps1`: 218 reviewed references,
  zero unclassified gaps, zero missing required assets and zero casing errors.
  One conditional Hardcore mantle mesh remains unavailable.
- Item-use UI now waits for authoritative inventory snapshots. Sale tooltips
  follow current Go payment rules. Versioned sale intentions reject stale
  price/item quotes before persistence. See the [client correction](client/port-748.md#authoritative-item-use-and-merchant-display-2026-11-02).
- The owner requires the [native launcher](client/launcher.md#native-launcher-retained-2026-11-02).
  The replacement launcher, source, tests, build targets and feed tools are removed.
  The native language extension remains supported under `tmproject/NativeLauncher/`.
  The prepared local full client uses the installed native image and includes the
  signed VC++ installer for manual use. Full clients use external hosting; the site
  links the verified Google Drive package for baseline 7.48.100.
  Both full-client VPS downloads and replacement-launcher endpoints are withdrawn.
  Native HTTPS transport is built, tested and installed locally. The new public
  Google Drive download matches the revised archive and is linked from the site.
  Automatic runtime installation and clean-machine acceptance remain pending.
  See the [HTTPS evidence](../.agents/research/native-launcher-https-20261102.json).
  See the [retirement receipt](../.agents/research/native-launcher-retirement-20261102.json).
- The [eight native-source corrections](client/port-748.md#eight-native-source-corrections-2026-11-02)
  cover asynchronous channels, combat prediction, deferred movement, equipment,
  mount health, appearance, emitter cadence, missing-file caching, scene cleanup
  and ground-data integrity. Release build and focused contracts passed.
  The four-second HTTP fixture no longer blocks cache reads. Initial farming
  has an owner report; targeted gameplay, two-client appearance and
  30/60/144 FPS measurements remain pending.

Local receipts remain in ignored `build/tmproject748-remediation-20261101/`,
`build/tmproject748-improvements-20261101/`,
`build/tmproject748-channel-list-20261101/` and
`build/guild-city-areas-20261101/`. The channel-list directory contains
`owner-client-acceptance.json`. These earlier local records alone do not establish VPS activation;
the November 2 release receipt records publication and activation.
The latest correction receipt is `build/client-native-corrections-20261102/REPORT.md`.
The earlier item-use and quoted-sale receipt remains in
`build/client-pending-corrections-20261102/REPORT.md`.
The news draft remains in `build/release-748018/news-draft.md` for owner review; it was not published.

## Rollback and operations

The recorded process-mode rollback is `wyd-channel-processes disable`; it restores `wyd-game.service`.
The previous release reference is `20261101T165857Z`, paired with client revision 17.
It predates coordinated revision-18 contracts; assess compatibility before rollback.
Keep the published revision-18 feed immutable and never reuse its revision.
These are recorded options, not authorization to change services.
Use [server operations](server/operations.md), [public deployment](server/public-launch.md) and the release skill for authorized work.

## Historical records

The [release and implementation history](../.agents/research/release-history.md) retains the previous log, hashes, backups and validation boundaries.
Old source-only and pending statements remain dated evidence; later receipts supersede them.
The [bug register](bug-register.md) owns report status; feature contracts own behavior.
Historical handoffs do not override these records.
