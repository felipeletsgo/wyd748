# Current project state

Updated 2026-11-09 (São Paulo). Both channels require client revision 108.

## Active release 7.48.108

Release `20261109T231129Z` runs on both game channels and the player API.
The launcher feed appends `48108 patch-48108.zip`. The ten-file patch
contains the executable and base, EN, and PT equipment and tooltip resources.

This release corrects progression, Celestial unlocks, equipment bonuses,
Ancient weapons, PvP, and weather volume. The portal displays the new
SubCelestial eligibility requirement and the patch notes.

Both channels reject revision 107 and send revision 108 to credential validation.
Verified the patch, portal, executable hashes, readiness, and database backup.
Gameplay and real-client launcher extraction remain pending.
The rollback release is `20261109T160924Z`.
See the [release receipt](../.agents/research/release-748108-20261109.json).

The source build targets revision 109. The public launcher feed and live server
admission use revision 108 until a new release is published and activated.

## Previous release 7.48.107

Release `20261109T160924Z` ran on both game channels before 7.48.108. The launcher feed ends with
`48107 patch-48107.zip`. The patch contains `WYD.exe` and `sound/soundlist.txt`.

The release restores summon and mount attack effects, ground-item pickup by
right click, skill and world effects, and item drop and ambient sounds.
It corrects inventory icons for 36 capes.

Both channels reject revision 106 and send revision 107 to credential validation.
Verified the public patch, portal, running executables, readiness, and database backup.
Post-activation gameplay and cape visual checks remain pending.
The rollback release is `20261109T040919Z`.
See the [release receipt](../.agents/research/release-748107-20261109.json).

## Previous release 7.48.106

Release `20261109T040919Z` ran on both game channels before 7.48.107. The native launcher
feed appends `48106 patch-48106.zip`. The portal advertises 7.48.106.
Both channels reject revision 105 and send revision 106 to credential validation.

The large Corrupted Crystal at (1104, 1711) releases five medium crystals.
After all five break, four small crystals spawn around each former medium.
The complete encounter drops 60 separate Laktorerium Powder units.

Verified the public patch, both running executables, readiness, and the database
backup. In-game visual acceptance remains pending. The rollback release is
`20261109T011640Z`. See the [release receipt](../.agents/research/release-748106-20261109.json).

## Previous release 7.48.105

Release `20261109T011640Z` ran on both game channels before 7.48.106. The native launcher
feed appends `48105 patch-48105.zip`. The portal advertises 7.48.105.
Both channels reject revision 104 and send revision 105 to credential validation.

The Corrupted Crystal at (1104, 1711) now has 100,000,000 HP and a persistent
purple Lightning Storm aura. The four guardians and the Crystal Warden each
have 10,000,000 HP. These HP overrides apply only to the encounter.

Verified the public patch, both running executables, readiness, and the database
backup. Actual client acceptance remains pending. The rollback release is
`20261109T004923Z`. See the [release receipt](../.agents/research/release-748105-20261108.json).

## Previous release 7.48.104

Release `20261109T004923Z` ran on both game channels before 7.48.105. The launcher feed appends
`48104 patch-48104.zip`. The public manifest, download page and changelog show
7.48.104. Both channels reject revision 103 with a localized update notice.
Revision 104 reaches credential validation in synthetic probes.

Each channel introduced the Corrupted Crystal at (1104, 1711) with 10,000,000 HP.
The patch includes the crystal mesh, crack textures, shatter effect, purple
Thunderbolt, Help centering and commands, and expanded camera distance.

Verified all 21 public ZIP entries, both running executable hashes, readiness
and the fresh database backup. The player API and log reader retained their
process IDs. Actual client acceptance remains pending. The rollback release is
`20261108T172846Z`. See the [release receipt](../.agents/research/release-748104-20261108.json).

## Previous BM summon activation

Release `20261108T172846Z` introduced the BM adjustment on both game channels. Any learned BM summon skill
creates one creature per learned summon type, up to eight types. Recasts replace
missing types and retain living creatures. See the [summon rules](server/operations.md#bm-learned-summons-2026-11-08).

Both running executables match SHA-256
`79aa1ec6bf9be30ffa257f894ff8b76815c299031f8d85075c2815370a87b30a`.
Only `game/wyd-server` changed across 27,722 installed files. Both channels restarted;
the player API and log reader retained their process IDs. Readiness checks returned
200, ports 8281/8282 listened, and the postflight review found no channel errors or warnings.
Backup `wydgo-20261108T172846Z.dump` passed checksum and archive-list checks.
Configuration, schema v20, runtime data, client feed, and website files were preserved.

The launcher feed stayed at 48103 during that activation. Revision 102 receives the mandatory-update notice;
revision 103 reaches credential validation on both channels. Synthetic probes used
no real account. Gameplay acceptance remains pending. Release `20261108T150126Z`
is retained for rollback. See the [BM activation receipt](../.agents/research/release-bm-summoner-20261108.json).

## Client release 7.48.103 and previous server activation

Release `20261108T150126Z` ran on both game channels before the BM adjustment, with minimum client revision 103. The launcher feed preserves 48100–48102 and adds `48103 patch-48103.zip`. The site manifest, download page and changelog advertise 7.48.103. Full-client downloads remain on the owner's existing Google Drive baseline; existing installations update through the native launcher.

The four-file patch corrects both actor-label paths, first-open occlusion, HUD-safe panel placement, footer and button bounds, text clipping and close/reopen focus. Client and server normalize query whitespace consistently. Drop rates, rewards and wire v1 remain unchanged. The classic monster-detail and icon-grid layout remains a visual reference.

The HTTPS ZIP and all four entry hashes passed verification; ZIP SHA-256 is `5739f3df256a823fd31acacecfc91d7e2fdaf265ef667e337725087f4369703e`. All 1,076 server payload and 6,661 site payload hashes matched. Both game executables then matched SHA-256 `456eaeac80e17ee9b1954d8256935e98e9c1f1555b4c294a5731474ed37b5c58`. Both channels restarted; the API and log reader kept their process IDs. Configuration hashes and schema v20 were preserved. Fresh database backup `wydgo-20261108T150125Z.dump` passed checksum and archive-list checks.

Revision 102 receives the localized mandatory-update notice on both channels; revision 103 reaches credential validation. Synthetic probes used no real account. The owner approved the installed local candidate before publication; no per-scenario capture or post-activation gameplay result was supplied. News remains a review draft. The prior release `/srv/wyd-go/releases/20261108T123541Z` is retained for rollback. See the [activation receipt](../.agents/research/release-748103-20261108.json).

## Open correction and acceptance queue

**BUG-026** affected the revision 103 Help window. Published revision 104 uses the same centering as Quest. Actual client acceptance remains pending.

Published corrections still need targeted game acceptance. This includes the BM learned summons; the 7.48.103 drop-list layout, overlay and close/reopen flows; the earlier mount, party, skill-shop, quest, Gold Angel and coin-drop changes; and the eight client corrections listed in the [client port status](client/port-748.md#eight-native-source-corrections-2026-11-02). Automated checks and publication do not prove every gameplay scenario.

The dated 271-row client audit is not reconciled row by row against later fixes. Its 13 owner deferrals, 19 evidence deferrals and 36 partial records are **not** a current bug count. Until that reconciliation is complete, the queue above cannot establish that BUG-026 is the only unresolved issue across historic audits.

Other recorded limits are follow-up or product work rather than confirmed new player bugs: matched 30/60/144 FPS measurements; the unresolved sound-list ID 220 mapping and runtime playback; one unavailable conditional Hardcore mantle mesh; clean-machine VC++ installation; and shared-object persistence quarantine, which still needs a restart for recovery. See the owning client, asset and [persistence](server/critical-persistence-plan.md) documents.

## Previous active release 7.48.102

Release `20261108T123541Z` ran on both game channels with minimum client revision 102 before 7.48.103 activation. Its launcher feed preserved 48100 and 48101 and added `48102 patch-48102.zip`. The site receipt and changelog advertised 7.48.102. Full-client downloads remained on the owner's existing Google Drive baseline.

The four-file patch attempted to correct drop-search world-label overlap, explains the personal drop bonus, and improves search feedback. The owner's later screenshots still show labels, HUD overlap and footer defects. The server excludes orphan NPC templates from ordinary and global-event results. The classic monster-detail and icon-grid layout remains a visual reference; it was not implemented in this release.

The HTTPS ZIP and all four entry hashes passed verification; ZIP SHA-256 is `81aec6a09b26287dcca7fca832bf2961b01b962d4506858952a1ba6bd4db12eb`. All 1,076 server payload and 6,661 site payload hashes matched. Both running game executables match SHA-256 `eff7a4c9f4ef1589a57a4ba9c5cf9b58a7767d72c89110ed714d4509dcc08ae3`. Both channels restarted; the API and log reader kept their process IDs. Configuration hashes and schema v20 were preserved. The database backup passed checksum and archive-list checks.

Revision 101 receives the localized mandatory-update notice on both channels; revision 102 reaches credential validation. The synthetic probe used no real account. The owner's visual check rejected the drop-list presentation. News remains a review draft. The prior release `/srv/wyd-go/releases/20261108T112940Z` is retained for rollback. See the [activation receipt](../.agents/research/release-748102-20261108.json).

## Revision 103 validation and local approval

The [drop-list follow-up audit](protocol/drop-list.md#november-8-follow-up--published-748103) covers 17 areas. Source admission contracts agree on revision 103. The `-NoDeploy` Release build, client checks, game/wire tests, focused vet, EN/PT locale tests, and layout/link check passed. Portal checks, build and 116 tests passed. The owner authorized local installation on November 8, then approved publication. `tmproject/client748/WYD.exe` matches published SHA-256 `23B40806A698BB143051FAF547FE9E68731372B2DB2AB68D109F5C46F561A8FD`; the previous executable is backed up. See `tmproject/build/drop-list-103/local-install.json`. Detailed EN/PT and resolution coverage remains unrecorded.

## Previous active release 7.48.101

Release `20261108T112940Z` ran on both game channels with minimum client revision 101 before 7.48.102 activation. Its launcher feed listed `48100 notice.txt` and `48101 patch-48101.zip`. The public site advertised 7.48.101 and retained the 7.48.100 full-client download on Google Drive. Existing baseline-100 installations updated through the adapted native launcher.

The game-only release verified all 1,076 server payload hashes. The site release verified all 6,661 files. The client patch contains nine files; its downloaded HTTPS copy matches SHA-256 `8d0b114a64792d936df2ec09c612f84a0cc83f764acc23d44ac06af476b2d170`. Client revisions 100 and 101 were checked on both channels: 100 is rejected with the localized update notice; 101 reaches credential validation. No real account was used.

The PostgreSQL backup `wydgo-20261108T013328Z.dump` passed `pg_restore --list`; SHA-256 `bf30585695b2d6608158409f7153277dc7adc1789751ed23cc75c4808b3a94d0`. The previous release remains the rollback target at `/srv/wyd-go/releases/20261103T010701Z`. Both game channels restarted. The player API and log reader kept their process IDs; all four services are active with zero automatic restarts. Readiness returned HTTP 200 on ports 8082 and 8083, and game listeners 8281 and 8282 are active. Private settings and their hashes remain unchanged.

The update has not been tested in a live game session. Client installation and gameplay acceptance remain pending. The release news is a draft for owner review, not published. See the [activation receipt](../.agents/research/release-748101-20261108.json) and the [release 7.48.101 client changes](protocol/drop-list.md).

## Previous active release — baseline 7.48.100

Release `20261103T010701Z` ran until November 8 with minimum revision 100. It established separate channel processes, the verified Google Drive full-client baseline, and schema v20. Its [dated activation receipt](../.agents/research/release-748100-https.json) remains historical evidence. November 2 client installation and gameplay reports do not establish acceptance of 7.48.101.

## Corrections and acceptance

| Scope | Latest recorded state | Owning document |
| --- | --- | --- |
| W0–W7: diagnostics, persistence, login, output pressure, health and activation cost | Activated in R1–R3; COMMIT closed-connection hotfix activated as `20261101T064639Z` | [Correction plan](server/server-correction-plan.md) |
| W8–W9: participant rows, shared ownership and channel processes | R4 activated schema v15–v20; R5 activated separate channel processes | [Correction plan](server/server-correction-plan.md) |
| BUG-001–BUG-011 | Closed by the owner's client confirmation on 2026-11-01; no session capture is attached | [Bug register](bug-register.md#queue) |
| Load acceptance | Owner accepted a local two-process, 600-player simulation; this is not a 600-player VPS observation | [Load evidence](bug-register.md#load-simulation--2026-11-01-two-channel-processes-600-players) |
| Unknown persistence outcomes | Account-only quarantine can recover from final XID status; shared-object recovery remains restart-based | [Persistence recovery](server/critical-persistence-plan.md#current-gaps-and-acceptance-criteria) |

## November 2 baseline and local-client history

Before November 8, the published client, active server and local installation used the
owner-selected baseline 7.48.100. Its external HTTPS-launcher package was verified.
At that time the feed started at 48100. The 7.48.101 patch is now published. See the
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
The November 2 news draft remains in `build/release-748018/news-draft.md`; it was not published. The 7.48.101 draft is `build/release-748101-20261107/news-draft.md`; it also awaits owner review.

## Rollback and operations

The active release rollback target is `/srv/wyd-go/releases/20261108T150126Z`, recorded in the [BM activation receipt](../.agents/research/release-bm-summoner-20261108.json). It enforces minimum client revision 103. The published patch feed is append-only; do not lower or reuse a revision. Older process-mode rollback notes belong to the dated release history.
These are recorded options, not authorization to change services.
Use [server operations](server/operations.md), [public deployment](server/public-launch.md) and the release skill for authorized work.

## Historical records

The [release and implementation history](../.agents/research/release-history.md) retains the previous log, hashes, backups and validation boundaries.
Old source-only and pending statements remain dated evidence; later receipts supersede them.
The [bug register](bug-register.md) owns report status; feature contracts own behavior.
Historical handoffs do not override these records.
