# Current project state

## Local NPC catalog migration — 2026-10-27, not activated

The general population now uses one `data/NPCGener.txt` for both channels.
It preserves the former channel 2 baseline and retains 396 imported groups,
including test and quest content. The owner selected the former channel 2
definitions for 40 co-located replacement groups. The result has 5,501 source
groups, with 5,485 enabled by the loader. Trainer quests retain their IDs and
rewards under `Treinador_I`, `Treinador_II`, and `Treinador_III`.
`NPCGener2.txt` and the `npcgener2` configuration key were removed locally.
Distinct legacy spawns and existing overlaps remain for manual owner review.
The owner removed DeathWorm group 5134 at `(2137,2052)` because its coordinates were incorrect.

Both channels now load a common `data/npcs/` catalog based on the approved
channel 2 templates. Required quest, instance, and event variants remain available.
The consolidated `channel2_population.json` input and overlay loader were removed.
PostgreSQL overrides remain channel-scoped, with independent mutable World data.
Identical Reimers templates were consolidated into the spawned lowercase identity
at `(2468, 1996)`; its stock and spawn remain unchanged. See
[live NPC definition management](protocol/npc-definitions.md) for the source and
database contract. These local changes have not been deployed or activated.

## Release 7.48.008 — 2026-10-26, published and activated

The official-style Premium Nell layout and catalog categories are packaged as
mandatory patch 48008, with compiled identity `0x02EC0008` and server minimum
revision 8. Public HTTP/HTTPS feed and ZIP verification preceded activation of
`/srv/wyd-go/releases/20261026T235538Z` at 23:55:41 UTC. All 7,795 installed
files and both running executables match the package. All 1,157 runtime data
files and eleven private configuration files were preserved.

The owner explicitly authorized disconnection and game/API restart. Shutdown
persisted three accounts with zero failures; backup succeeded. Both readiness
endpoints and game channels passed, with zero automatic restarts. The public
site, News, changelog and manifest advertise 7.48.008. Both channels reject
revision 7 and admit revision 8 to credential validation in synthetic probes.
Build, wire/page tests and portal checks passed; actual embedded shop lifecycle
and purchases remain pending gameplay acceptance. Pre-existing population and
content-index warnings remain unchanged. See the
[release receipt](client/launcher.md#published-patch-748008-2026-10-26).

## Release 7.48.007 — 2026-10-26, published and activated

The Premium Nell embedded-navigation correction is published as mandatory
patch 48007. After public HTTP/HTTPS verification, the game and API activated
`/srv/wyd-go/releases/20261026T222350Z` at 22:23:53 UTC with the owner's explicit
authorization to disconnect the three active connections. Backup succeeded;
shutdown persisted three accounts with zero failures. Both readiness endpoints
and channels passed. All 7,793 installed files matched, private configuration
was preserved and automatic restart counts remained zero.

The launcher feed, public manifest and website advertise 7.48.007. Both channels
reject revision 6 and accept revision 7 as far as credential validation in
synthetic nonexistent-account probes. The local runtime executable was not
replaced. The [release receipt](client/launcher.md#published-patch-748007-2026-10-26)
records hashes, validation and rollback. The
[host correction](protocol/premium-shop.md#host-navigation-correction--client-748007)
preserves the v3 purchase contract. Actual shop opening, purchases and lifecycle
acceptance in the corrected client remain pending; this is not `CLIENT_TESTED`.

## Release 7.48.006 and Premium Nell diagnosis — 2026-10-26

Mandatory patch 48006 was published and verified before activating
`/srv/wyd-go/releases/20261026T195612Z`. Game and API restarted with owner
authorization; both readiness endpoints and channels passed. All 7,791 installed
files matched the archive and private configuration was preserved. The
[release receipt](client/launcher.md#published-patch-748006-2026-10-26) records
the exact hashes, validation and rollback reference.

The owner's installed client matches 7.48.006, and revision-6 login is recorded.
Premium Nell web startup nevertheless fails. An isolated x86 host using the
actual compiled resource reproduced the cause: `NavigateToString` emits a
`data:text/html;...` navigation on the installed WebView2 runtime, but the host
only permits `about:blank`, cancelling its own page. Allowing that initial
navigation in the diagnostic copy reached the page's `ready` message. The
production source and installed client were not changed by this diagnosis.
The correction and actual purchases remain pending; publication is not
`CLIENT_TESTED` acceptance. See the
[host diagnosis](protocol/premium-shop.md#published-v3-host-diagnosis-2026-10-26).

## Molar entrance server update — 2026-10-26, activated

The game now runs `/srv/wyd-go/releases/20261026T064741Z`, with executable
SHA-256 `f735e212f890bc02ad85130d97df7fe4f86b455516cef6d2afc91672f334dd2c`.
The owner requested the missing generator before publication and authorized
the restart. Channel 2 now has one friendly `Gargoyle_Watcher` at `(787,4058)`,
matching channel 1. Entry and Molar use accept displayed Mortal levels 201-205.
The existing destination `(817,4062)`, Molar drops and one-time refinement remain.
The Molar rectangle `(793,4046)-(827,4080)` now uses the shared quest area
guard: unauthorized stays, invalid levels and the shared reset return players
to town. NPC admission grants authorization only after persistence commits.
The rectangle comes from the secondary W2PP reset reference, adopted as an
explicit project choice without a new native 7.48 parity claim.

Focused tests cover both populations, walkable entrance/destination, enabled
Molar drop groups, handler routing, level boundaries and persistence settlement.
Area tests cover wall jumps, relogin, authorized entry, countdown and reset.
All 1,157 installed data files and the running executable match the package.
Both readiness endpoints and public TCP channels passed; backup succeeded,
shutdown reported zero failures and the game has zero automatic restarts.
The owner requested another restart at 2026-10-26 06:58 UTC. All recorded
server/client/website inputs were unchanged, and the running binary plus all
1,157 installed data files matched the existing release. After a successful
backup, only the game restarted (PID 1642569); both readiness endpoints and
public channels passed again. No additional package was published.
Player API/log-reader process identities and private settings are unchanged.
The previous release remains available for rollback. Client 7.48.005 is unchanged.
Actual NPC interaction and quest completion in the client remain pending;
the generic quest still has incomplete PT localization. Exact provenance and
verification are in the `serverUpdates` entry of
`.agents/research/release-748005.json`.

## Release 7.48.005 — 2026-10-26, published and activated

The mandatory client patch was published and verified over public HTTP/HTTPS
before activating `/srv/wyd-go/releases/20261026T061812Z`. The owner explicitly
authorized the game and Player API restart. Game PID 1635034 and API PID 1635046
use the expected new binaries; both readiness endpoints and public channels
passed verification. The site and manifest now advertise 7.48.005. All 1,157
installed data files match the package, including the owner's NPCGener cleanup
and the three Perzen services. Runtime/private configuration hashes are
unchanged. Backup succeeded, shutdown reported zero failures, and automatic
restart counts remain zero. The previous release is retained for rollback.

See the [published release receipt](client/launcher.md#published-patch-748005-2026-10-26)
and `.agents/research/release-748005.json` for exact hashes, source inputs,
public delivery, protocol probes and cleanup limits. No real player account
was authenticated by the probes. Actual launcher installation, Gryphon travel
and all Perzen exchanges still require gameplay acceptance. Existing disabled
source groups and unresolved staff index references match the previous startup.

## Individual disconnect investigation — 2026-10-26, channel 2 teleport

Read-only production inspection identified session 7 (`FelipeTr`) closing at
04:21:31 UTC (01:21:31 America/Sao_Paulo), immediately after staff `/goto` from
`(2244,1333)` to `(2467,2007)` on TCP 8282. The reader reports
`use of closed network connection`, consistent with a local socket close.
There was no accompanying critical persistence, maintenance, security or
invalid-outbound-packet diagnostic. The service remained active as PID 1604105,
with zero automatic restarts, on the hotfix release documented below. Session 8
authenticated eight seconds later and entered channel 1 at 04:22:00 UTC.

The deployed configuration has `session_queue_capacity=256`. `Session.Send`
closes the socket when its outbound queue is full, without logging that cause.
The writer also closes on deadline setup, write error or short write without
logging the underlying result. Therefore the reader message alone cannot
distinguish queue overflow from a writer failure; it does not establish a
client crash or an external network outage.

A preliminary Go overlay probe used a reduced world and a paused writer.
A follow-up replaced it with the full `NewWorld` constructor, the channel-2
population, terrain, initial map items, item-instance reservations and enabled
Colosseum reservations. Seed 759 produced 5,918 generators and 8,996 mobs at
boot, including 624 visible mobs at `(2244,1333)`. The generator count matches
the live channel-2 summary. This is a deterministic boot fixture, not a replay
of the production world's subsequent movement, deaths, replenishment, players,
shops or configured boss encounters.

Of those 624 visible mobs, 561 use template `Cobra_`. Both official reference
files (`DestinyServer-main/TMSRV/run/NPCGener.txt` and
`DestinyServer-main/TMSRV2/run/NPCGener.txt`) and the then-installed `NPCGener2.txt`
contained the same 561 enabled `Cobra_` records starting at `(2240,1324)`.
The concentration is inherited from the reference data; this comparison does
not establish how the native executable treats those records at runtime.

Refreshing visibility at `(2467,2007)` emitted 664 packets totaling 22,808 bytes:
624 mob removals, 39 mob creations and one ground-item creation, verified by
decrypting and classifying the collected packets. Temporary instrumentation distinguished queue
overflow from writer errors. With a real loopback TCP connection, a concurrent
writer and a peer continuously reading, all 20 trials at capacity 256 closed
through the queue-overflow branch; none reported a writer error as the first
close cause. At capacity 2,048, none of 20 trials closed. A control that excluded
only the 561 `Cobra_` IDs from the origin visibility snapshot emitted 103 packets
and stayed open at capacity 256. Neither control changed runtime data files.

This reproduces the disconnect mechanism with a healthy local receiver,
without relying on the preliminary paused-writer assumption. The visibility
refresh publishes the entire transition synchronously, while `Session.Send`
closes immediately if the writer cannot drain the bounded queue fast enough.
Queue overflow during this teleport is therefore strongly supported. The exact
historical close cause remains unrecorded, and the capacity control is a
diagnostic result, not a validated production sizing recommendation.

At investigation time, the local population inputs matched the active VPS files by SHA-256:
`NPCGener2.txt` =
`5200d07243c0dbb232b000a42f7412ee600bd6d815ae4c80840a23cb7189bf98`;
`channel2_population.json` =
`107f511b41ed4575ce90ba61661b066523bf36a793ba0a31177e5057677caebb`.
This is a historical snapshot hash. The later local catalog migration replaced
the consolidated file with individual templates in `data/npcs/`.
All three focused follow-up runs passed. Their tests and transport instrumentation
existed only in a temporary Go overlay; product source was not changed.
No gameplay correction, configuration change, publication or restart was
performed for this diagnosis. Follow-up work should record outbound close
causes and handle large visibility transitions without blocking the world
actor, losing entity updates or silently overflowing its session queue.

### Local population cleanup — 2026-10-26

After the owner's manual cleanup, 541 `Cobra_` generator sections remained at
`(2240,1324)` in `NPCGener2.txt`, plus one in `NPCGener.txt`. All 542 sections
were removed from the local files. Every retained section was preserved byte
for byte, including the 21 `Cobra_` sections elsewhere in `NPCGener.txt`.
Neither file now has a `Cobra_` starting point within 32 tiles of that location.
Templates and reference data were not changed.

The four focused population resolution, channel-2 world boot, official monster
bootstrap and mixed-group capacity tests passed. The channel-2 world boot
created 5,154 generators and 8,448 mobs. This validates local loading, not live
client acceptance or a general fix for outbound queue overflow. The initial
cleanup was local; the owner subsequently authorized publication and restart.

Resulting local SHA-256 values:
`NPCGener.txt` =
`467683cfa88aabba8b57d6da114335c8cb582e86b0625c8f9fe4aea909054cde`;
`NPCGener2.txt` =
`044f93d55db96cc51f4aeb8417881cf89fe33afe9785a50b8aacaef94110a7cc`.

At 05:18:52 UTC (02:18:52 America/Sao_Paulo), the game service restarted on
`/srv/wyd-go/releases/20261026T051850Z`, PID 1624402. The existing game-only
updater activated a snapshot of the installed release with exactly two changed
data files, the NPCGener files above; all 1,157 data paths were retained. Source
HEAD remains `40706d99b7715167d7aa4a2c484704f7e91c2c8b` with working-tree
population inputs. No commit, push, client patch or executable change occurred.
The 14,067,875-byte release archive SHA-256 was
`06fc092d9a75788c85b3216a2412d06fa0e8937437b9c278eedb6980cb35bd08`.
The running executable still hashes to
`9b23eb9619eaef2bebbfc2d93cda601ab848ac6cd331dc682eaa2b0c445a3d2a`.

Backup completed successfully before activation. Graceful shutdown persisted
one account with zero failures. Rollback remains available at
`/srv/wyd-go/releases/20261026T031523Z`. All `/etc/wyd-go` file hashes were
unchanged; Player API and log-reader PIDs were preserved. By 05:19:15 UTC,
both readiness endpoints passed, both game ports were externally reachable,
automatic restarts remained zero, and no critical containment, fatal or panic
diagnostic appeared in the bounded post-start journal. Inspection of the active
files confirmed zero regional `Cobra_` sections and the 21 sections elsewhere
preserved in channel 1. Remote staging was removed. Actual gameplay acceptance
and the separate transport queue correction remain pending.

## Production incident — 2026-10-25, persistence containment

Read-only SSH inspection confirmed that the former game release
`/srv/wyd-go/releases/20261025T203853Z` entered global maintenance at
22:55:01 UTC (19:55:01 America/Sao_Paulo). The journal reports:
`CRITICAL: persistence completion 4810 failed; entering maintenance: critical persistence: participant identity changed`.
All three connected sessions closed in the same second. Completion 4811 then
reported `maintenance prevents publication`, a subsequent containment error.
Later connection attempts on both TCP 8281 and 8282 closed after initialization.
Listening ports and an active service therefore do not establish gameplay
availability.

At the first inspection, the game process was PID 1539385, started at 20:38:56 UTC,
with `NRestarts=0`; PostgreSQL was also active. The running executable SHA-256
was `21023c507470096431f9ff7d42eb8d1f4d303fbcd924461d920799dfb090e06c`.
The immediate disconnect cause is confirmed persistence containment, rather
than a process exit. The generic identity error does not identify the failing
producer or the mismatched state. Opening a shop six seconds earlier is only
temporal context, not proof that the shop caused the failure.

An intervening restart at 23:15:21 UTC started PID 1565428 on the same release
and executable hash. At 23:15:24 UTC, completion 1 produced the same identity
error and global maintenance, before any player login. Both channels remained
unavailable for gameplay despite the active process.

Local diagnostic tests reproduced a deterministic defect in
`queueScheduledGuildChange`: the proposal mutates `Wars`, leaving the active
entry in `ChannelWars` stale. After applying that proposal, the post-acknowledgment
identity check compares it against `criticalGuildSnapshot()`, whose `Clone()`
refreshes that entry. `reflect.DeepEqual` consequently rejects the server's own
committed change and invokes global containment. The no-channel control passed;
channel 1, channel 2 and the automatic guild-economy producer reproduced the
failure. Tests asserting normal operation initially failed with the same
identity error, establishing a reproduction before the correction.

City-war scheduling provides a more specific production lead. The original
failure at 19:55:01 local time falls in the five-minute announcement window for
the 20:00 city war. A read-only PostgreSQL query found guild-state version 114,
updated at exactly 23:15:24.427472 UTC, with channel-1 city phase `waiting`,
start time `2026-10-25T20:00:00-03:00`, and notice mask 5 (five-minute warning
and start marker). Separate scheduler reproductions at 19:55:01 and 20:15:24
produced notice transitions 0 to 1 and 1 to 5, respectively, followed by the
same maintenance error. This strongly implicates city-war announcement writes;
the production journal does not name the producer, so the exact original
completion 4810 remains an inference. The current guild aggregate was committed;
this does not establish the durable outcome of every operation from the first
incident.

The owner subsequently authorized correction, publication and restart.
`queueScheduledGuildChange` now clones the changed proposal before submission,
normalizing its active channel entry exactly as the persistence worker does.
This is `MODERNIZACAO_COMPATIVEL`; the client contract and required revision 4
are unchanged. Regression tests cover both channels, completion from the other
channel, certified rejection, real identity divergence, city notices and the
weekly fame reset. The reproductions pass after the correction; real divergence
still invokes containment. The complete Go suite and `go vet ./...` passed.
Race and PostgreSQL fixture tests were not enabled.

Recovery inspection found the affected account write committed at the incident
timestamp, matching account identities, zero character-index ownership
mismatches, and no other active database transaction. Completion 4811's sole
`maintenance prevents publication` error is emitted after a successful store
result; it does not report an uncertain database write. No economic operation
was replayed, no account data was overwritten, and no containment check was
bypassed. The exact original producer remains inferred as described above.

At 2026-10-26 03:15:25 UTC (00:15:25 America/Sao_Paulo), the game service was
restarted on `/srv/wyd-go/releases/20261026T031523Z`, PID 1604105, executable
SHA-256 `9b23eb9619eaef2bebbfc2d93cda601ab848ac6cd331dc682eaa2b0c445a3d2a`.
The package includes the local manual NPC removals, direct NPC-name corrections,
service classification and Silver Dragon handler. The replacement data tree
does not retain old templates omitted from the package. The existing backup
service completed before activation; private configuration, account state,
Player API, log reader and website were preserved. The previous release remains
available for rollback. Exact inputs and results are in the `serverHotfix`
entry of `.agents/research/channel2-release-748004.json`.

Both listeners and readiness endpoints passed. External protocol probes on
8281 and 8282 rejected revision 3 and reached credential validation with
revision 4, using nonexistent accounts. The startup city-war interruption
committed guild-state version 115 at 03:15:27.783540 UTC, with phase `finished`,
without maintenance. At 03:18:22 UTC the process remained active with zero
automatic restarts and no new critical persistence error. Existing disabled
terrain groups and unresolved staff-index references remain separate gaps.
Actual client gameplay and full NPC acceptance remain pending. No client patch,
commit or push was performed for this hotfix.

Remote staging was removed. Local cleanup was rejected by execution policy
before execution; the task staging and two scratch scripts remain at the exact
paths recorded in the receipt. No alternate deletion was attempted.

## Release 7.48.004 — 2026-10-25, published and activated; both channels externally reachable

The owner authorized publication of channel 2 and the matching launcher assets.
Patch 48004 and the Linux server/site archive are published and validated; see
the [release receipt](client/launcher.md#published-patch-748004-2026-10-25).
Paired client/server revision 4 adds the second endpoint on TCP 8282, the
official `NPCGener2.txt` population and adapted 7.48-compatible templates.
The six-entry patch includes the executable, server list, SN group names and
base/EN/PT NPC message tables. Local runtime installation is unchanged.

The full Go suite, vet, client build gates, 19 locale tests and 89 portal tests
passed. Additional untracked runtime data is now accepted only through an
explicit packaging allowlist; all 409 selected files were verified in the
archive. The staged configuration preserves the VPS's current settings.
The owner explicitly authorized disconnecting the two active sockets; shutdown
persisted two accounts with zero failures. The initial active release was
`/srv/wyd-go/releases/20261025T203853Z`; game and Player API were restarted.
Public HTTP/HTTPS feeds, ZIP entries, site receipt, both readiness endpoints and
running executable hashes were verified. Local runtime installation is unchanged.

Both channels listen and their compiled admission gates work: revision 3 is
rejected, revision 4 reaches credential validation for a nonexistent account.
Both channels are externally reachable over IPv4. The owner authorized adding
inbound TCP 8282 in the authenticated Hetzner Console. The applied firewall
now has six inbound rules; ICMP and TCP 22/80/443/8281 and the outbound policy
were preserved. Public channel-2 protocol probes passed at 20:47:01 UTC,
rejecting revision 3 and reaching credential validation with revision 4.
IPv6 connectivity was not tested. Actual client transfers and full
NPC service acceptance remain pending. The production initial channel-2 world
summary was 5,918 generators and 8,989 mobs, with 727 template bindings; 21
invalid source groups remain disabled and the staff index has 169 unresolved
references. The completed crash-test output folder recorded in the receipt
could not be removed because automatic approval review blocked cleanup.
Post-verification staging cleanup was also rejected before execution. Exact
remaining paths are recorded in the release receipt; no alternate deletion
was attempted. Publication and external reachability remain verified.

## Release 7.48.003 — 2026-10-24, published and activated

The owner authorized publication and restart of the caption correction. Patch
48003 was published and verified through public HTTP and HTTPS before server
activation. The native launcher feed, public manifest, home, Download, changelog
and News now advertise 7.48.003. See the [patch receipt](client/launcher.md#published-patch-748003-2026-10-24).

Paired client/server revision 3 is active on HEAD `40706d99` plus selected
working-tree changes, without commit/push. The client gate passed 3,179,319
architecture checks, 281 socket checks, 200 crash checks and 58 launcher checks.
The full Go suite, `go vet ./...`, portal check/build and all 89 portal tests
passed. Database fixture and race coverage were not enabled; actual rendering
and launcher installation remain pending. The portal test baseline initially
referenced 7.48.002; after correcting its input path and updating the baseline,
the complete portal test rerun passed.

- Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, 1,938,944 bytes,
  SHA-256 `882F11E6CBA65E6B290DA0C8DF7920485982F5A9F1E49005D31823D76881BBE6`.
- Patch: `build/patch/patch-48003.zip`, 862,788 bytes, exactly one entry
  `WYD.exe`, whose hash matches the candidate. ZIP SHA-256
  `492DC2A390DEA1EBFB93CE52FF9F5A579F6D896EE4D72223CBAFEACF82741CAC`.
- Server/site archive: `build/deploy/wyd-deploy-20261024-748003/release.tar.gz`,
  25,434,568 bytes, 13,971 inspected entries, SHA-256
  `4A6463A663B8AF0B5AD0231E649AA68F63A73176C855F76E16EE5321FEDF7E6D`.
  Required executables, manifest and News page are present; private accounts,
  environment files and client preference/configuration paths are excluded.
- Exact selected input hashes are recorded in the patch receipt; verification
  logs remain in the sanctioned ignored build root.

Active release: `/srv/wyd-go/releases/20261024T210842Z`. Game and Player API
restarted at 21:08:45 UTC; the listener was ready at 21:08:46 UTC. Running
executable hashes match the inspected archive:

| Component | PID at verification | SHA-256 |
| --- | --- | --- |
| Game | 1317427 | `F8D654E23E64802D750BCAFEDAFADC969D0F01C35EBC6BC24B7D4FE1C2449D06` |
| Player API | 1317440 | `7374B707ED41EAB030418B04CEB452D650FC76E8AE10006BD0027FE7FB50B89C` |
| Log reader, unchanged process on the previous release | 1304661 | `16D8A9EA3E01B037EE846747CC4EFCF9853432BBD2A0E68D51BBD66904D2227D` |

Both readiness endpoints returned ready; all three services are active. No
error-priority Game/API journal entries appeared after activation. No established
game connections were present before the stop. Shutdown persisted zero accounts
with zero failures and `Result=success`. All nine configuration/unit files
matched production before activation; private game/player environment hashes
were unchanged afterward. No host reboot or security-policy changes occurred.
Backup `wydgo-20261024T210842Z.dump`: 186,110 bytes, SHA-256
`C8EF596F854D2DE76882AD72F69DD852A994AAB8D91557FF024439E4520AF69E`.
Remote deployment staging was removed. Rollback reference:
`/srv/wyd-go/releases/20261024T195426Z` (minimum revision 2; accepts revision 3).

At 21:11:04 UTC, production protocol probes rejected revision 2 before
authentication and admitted revision 3 to credential validation using random
nonexistent accounts. No account was created or character authenticated.
Actual launcher installation and game rendering remain pending. The installed
local runtime remains 7.48.002, SHA-256
`80CA48E77A8F83535401B7FE1415E33C9DE873E04F55C0313C844DF7B6A66209`.

The owner resolved the language policy conflict: the product supports English
and Brazilian Portuguese. `AGENTS.md` and the affected skills now preserve
multilingual product content and English engineering text. Existing client
localization and the native launcher remain intact; this does not certify
complete server or website localization. Repository layout validation still
reports pre-existing runtime ZIP/log/dump artifacts outside sanctioned build
roots; those files were preserved and excluded from delivery. Skill validation
could not run because both available Python environments lack PyYAML; unchanged
skill metadata and the focused instruction diff were inspected manually.

Automatic execution review rejected cleanup of the new crash-test evidence
directory `tmproject/build/tests/Release/crash-capture-tests-1516-9365828/`.
No alternative deletion was attempted; this directory remains locally.
The final cleanup command was also rejected as `blocked by policy` before
execution. No alternate method was attempted. Additional exact leftovers:
`build/deploy/wyd-deploy-20261024-748003/`,
`build/release-748003/verify-http.zip`,
`build/release-748003/verify-https.zip`, and these files under
`C:/Users/xereca/AppData/Local/Temp/`:
`wyd-release-748003-preflight.sh`, `wyd-release-748003-activate.sh`,
`wyd-release-748003-probe.py`, `wyd-release-748003-receipt.py`.
The published patch and current incremental client build are retained.

## Local caption regression fix — 2026-10-24, subsequently published above

The owner reported unreadable letters after 7.48.002. The local correction
removes automatic raster shrinking and bounds only confirmed Character/Skill
captions, using original-size glyphs and ellipsis for real overflow. Unrelated
labels and numeric values retain their existing layout. The full NoDeploy
build gate passed; actual game rendering remains pending, not `CLIENT_TESTED`.
See [cause, coverage and artifact hashes](client/multilingual-client.md#caption-legibility-regression-october-24).
That initial local check did not install or publish its candidate; the paired
revision-3 rebuild and publication are recorded above.

## Release 7.48.002 — 2026-10-24, published and activated

The owner authorized client/server/site publication, restart and player
disconnection. The native launcher patch was published and independently
verified first; see [patch receipt](client/launcher.md#published-patch-748002-2026-10-24).
Admission requires release prefix `0x02EC` and revision >= **2**. News explains
mandatory updates and the full-client 7.48.001 baseline. No launcher binary or
language-preference file was replaced.

Source HEAD `40706d99b7715167d7aa4a2c484704f7e91c2c8b` plus selected working-tree
changes (`vcs.modified=true`); no commit/push. Includes caption fitting, explicit
Windows-1252 GDI text boundaries, bounded quest subject loading, corrected command
help and reserved-command handling, the Colosseum minute-6 gates/first wave and
one-damage monster cap, and the committed DEX score rule below. Citizenship
departure, handover and legacy war commands remain unavailable. This deployment
does not implement the separate channel plan.

Release `/srv/wyd-go/releases/20261024T195426Z` is active. Archive SHA-256:
`3FD9D7402BD836C3D6D84260EAA615E2D910381210D0A1680BD6BDE4EE56CE84`.
Actual running executable paths and hashes match staging:

| Component | PID at verification | SHA-256 |
| --- | --- | --- |
| Game | 1304575 | `7BE3E58C051842BC18F0584C044371B71AEA70185F0070F6812512A6BFC80192` |
| Player API | 1304588 | `7374B707ED41EAB030418B04CEB452D650FC76E8AE10006BD0027FE7FB50B89C` |
| Log reader | 1304661 | `16D8A9EA3E01B037EE846747CC4EFCF9853432BBD2A0E68D51BBD66904D2227D` |

All nine staged private configuration/unit files matched production before the
stop. The game had zero connections at 19:54:25 UTC; the previously observed
player had disconnected at 19:48:27 UTC. Graceful shutdown persisted zero
accounts with zero failures and `Result=success`. Game/API restarted at 19:54:28
UTC; the game listener was ready at 19:54:29 UTC. No machine reboot. Schema
remains 11. Backup `wydgo-20261024T195425Z.dump`: 185,993 bytes, SHA-256
`36B0B2B5874EAFC7BAB4560C1A02CE65F93E36D3CB0193EB433785279DA6F6A6`.
Rollback reference: `/srv/wyd-go/releases/20261023T203847Z` (minimum revision 1,
same release prefix; accepts the new revision-2 client). Do not lower the gate
in the new source to roll back.

Both readiness endpoints returned ready; all three services are active; no
error-priority game/API journal entries appeared after activation. Public
HTTP/HTTPS feed, ZIP and manifest, plus News, Download, home and changelog were
verified. Production protocol probes rejected revision 1 and admitted revision
2 to credential validation; no real character login was attempted. Existing
Caddy redundant-forwarded-header warnings and the staff index's 173 unresolved
references remain outside this release's acceptance coverage.

Validation: client gate/build (3,176,611 architecture assertions/checks, 281
socket, 200 crash and 58 launcher checks); all Go packages passed across the full
run and a focused wire rerun after updating its hardcoded baseline assertion;
`go vet ./...`; portal check/build and 89 tests. PostgreSQL-dependent fixture
and race coverage were not enabled. The previous locale suite/generator results
remain valid for unchanged resource inputs. Not `CLIENT_TESTED`: visual text
fitting, non-Western Windows ACP, actual launcher update/login and Colosseum
gameplay remain pending. Installed local `client748/WYD.exe` is still the
7.48.001 executable; publication does not install it locally.

The initial server upload and subsequent SSH diagnostic timed out before
authentication; access recovered with the same account/key. No credential or
security-policy change was made. A filtered fail2ban log query returned no event
for the current source in that file; this does not establish a global absence
of network blocking. The activation succeeded; a trailing CR on the wrapper's
final cleanup-trap command produced a post-activation exit status 1. Independent
verification confirmed the selected release, process hashes, readiness, backup
and removed remote staging; the activation was not repeated.

Remote temporary staging was removed. Local cleanup was rejected by execution
policy before the command ran; no alternative deletion method was attempted.
The following task-created paths remain and can be removed manually. The
published patch ZIP and current incremental client build are retained.

- `build/deploy/wyd-deploy-20261024-748002/`
- `build/release-748002/verify-http.zip`
- `build/release-748002/verify-https.zip`
- `tmproject/build/tests/Release/crash-capture-tests-13816-4580921/`

Full build, transfer and activation failure diagnostics remain in the sanctioned
`build/release-748002/` output directory. The 41 selected changed source/resource
input hashes are preserved in the launcher receipt.

## Player DEX score rule — 2026-10-23, activated in 7.48.002

Owner-requested: player physical attack now uses `STR/3 + DEX/8` and DEX adds
no defense, following the official 7.59 score builder
([7.59 analysis](<../references/decompilacao server/dlls/rea-analysis/maps/server-759-gap-analysis.md#player-score-dex-damage-and-defense-adopted-2026-10-23>)).
Committed in `40706d99`; full Go suite and vet passed. Activated by the release
above. Actual in-game score observation remains pending.

## Full client 7.48.001 — 2026-10-23, published and activated

The owner reset the client version for the multilingual full client. Handshake
contract v2 ([client build admission](login-dispatch.md)): the client sends
`0x02EC0001` (7.48.001) and the server requires prefix `0x02EC`, revision >= 1;
every retired `0x5747xxxx` client is refused. `config.txt` is `[VERSION] 48001`
and the launcher shows 7.48.001 ([release line](client/launcher.md#release-line-748)).
The site now has one changelog entry and one News post for 7.48.001 (only the
account-protection announcement is kept); the download receipt (version 2)
has no patch and links the owner's Drive full client
`https://drive.google.com/file/d/1ouCjq-A8JOnHRXpO0m8fEhRm2NuGgmFk/view`.
The owner chose to publish without downloading it: its size, SHA-256 and the
`WYD.exe` it contains were not verified and remain null on the site.

Installed locally in `tmproject/client748/`: `WYD.exe` SHA-256
`D29873E2DFA5528E432A164CC007D6DFE729A20D31C3902F3986542D76B7BB3A` (embeds
`0x02EC0001` twice, no `0x57470003`), `WYDLauncher.exe`
`85C9C648B389BE91CA348351590FCBEFA6752A4D4E849AFBBCB1F054CA12FDE2`,
unchanged `WYDLauncherLanguage.dll`
`003134ABA581FD9802FD0DDFA8D2048D7EA834E14043ED6B8878377E8666BF59`.
Automated: ArchitectureTests 3,171,532 checks, SocketReceiveTests 281,
CrashCaptureTests 200, LauncherTests 58, seven native-launcher tests, full Go
suite and vet, portal build/check and 89 portal tests. Not `CLIENT_TESTED`:
launcher status text, old client rejection, new client login and both languages
need a real run.

Runtime cleanup for distribution ([full client contents](client/launcher.md#full-client-contents)):
build-only inputs moved to `tools/client-assets/inputs/` (`skin-shaders/shader1..8.bin`,
`Costumes-KR.json`, `Mounts-KR.json`); 53 tracked files that neither `WYD.exe`,
the launcher nor any tool or test reads, plus local logs, `update.dat`,
`SkillData.csv` and a crash log, were moved to the ignored quarantine
`build/client-cleanup/client748/` because policy blocked direct deletion.
The tracked removals appear as deletions in Git. Delete the quarantine once the
cleaned client is confirmed in a real run.

Publication (owner-authorized, including the game restart), from HEAD
`76626439` with the working tree (`vcs.modified`; includes the already active
Kibita server delta); nothing committed. Staged `server.txt`, `staff.json`,
Caddy site and six units matched the live files; no player was connected.
`reset-patch-feed.sh` archived the old list and 45 patch ZIPs to
`/srv/wyd-go/patch-archive/20261023T203833Z/` and installed `48001 notice.txt`.
`update-release.sh` activated `/srv/wyd-go/releases/20261023T203847Z` (archive
SHA-256 `61A05401B2EE7E3CAE87F7E70E308772507185C4A3FD74CE9CC411D5AE09F6D8`),
restarting game and player API. Running game `/proc/<pid>/exe` SHA-256
`8675CEEF83E6C47D9B0F34F6870B7B2DCEB584BCB6227515B8CB916209033B01`
(contains `0x02EC0001`, no `0x57470003`); player API unchanged
`01066C11…488110F`. Both readiness endpoints `ready`, no error-priority journal
entries, port 8281 reachable. Pre-activation backup
`wydgo-20261023T203847Z.dump`. HTTP and HTTPS `update.htm` return the 48001
baseline; `patch-10045.zip` and `patch-10001.zip` return 404; the manifest,
Download, News and changelog show 7.48.001 only. Rollback reference
`/srv/wyd-go/releases/20261023T173911Z` pairs only with the archived 10045 feed
and revision-3 clients. Pending: a real 7.48.001 login and an old-client
rejection observed in game; not `CLIENT_TESTED`.

## Release — 2026-10-23, Kibita Mortal Soul

The owner authorized publication and restart. Server-only working-tree release
from HEAD `76626439342037f92b72518b94745ea4c795fc74`, with
`vcs.modified=true`; the complete functional delta is the five Go files and
hashes recorded in [Kibita Soul](server/kibita-soul.md#production-delivery).
No commit or push was performed. The unrelated local client configuration and
crash dumps were excluded. Client packets/assets are unchanged: patch **10045**
and compiled admission revision **3** remain aligned on both sides.

Release `/srv/wyd-go/releases/20261023T173911Z` is active. Rollback reference:
`/srv/wyd-go/releases/20261023T153545Z`, with the same client revision.
Archive: 25,486,009 bytes, SHA-256
`AC7DEF60F16AB9747EC04771259EB2C12D668889C4ED02B60B352161B0007BAA`.
Running game SHA-256
`143902826F5B74A1F57CC3334A13F9640873558F954EFA971ACC591232609CCB`;
player API `01066C11702176D1A5A4500E547D90FA88508AD5F89C5B45245B3906C488110F`;
log reader `49121210861B93E187768266BD995941F8C634B813A939D5B2CFDB4ED2157342`.
All three `/proc/<pid>/exe` paths and hashes match the candidate. Nine private
configuration/unit files matched staging before activation.

No players were connected. Graceful shutdown at **17:39:15 UTC** persisted
zero accounts with zero failures; the game listened at **17:39:16 UTC**.
Pre-activation backup: `wydgo-20261023T173911Z.dump`, 179,595 bytes, SHA-256
`6619BC7AD2760295EE6518B9457A6389D8F2BD62E86D46FB034E2E5C1B131B20`.
Schema remains 11. Both readiness endpoints returned `ready`; the public game
port was reachable; no error-priority game/API journal entries appeared during
activation. Public HTTP/HTTPS launcher feeds and the manifest still advertise
10045. Remote staging was removed. Execution policy rejected the local recursive
cleanup, so `build/deploy/wyd-deploy-20261023-kibita-soul/` remains with the
candidate/archive/configuration and source-hash receipt. Deletion was not retried
through another command or API.

Validation reuses the full Go suite/vet and final game regressions recorded in
the feature document; portal check passed with zero diagnostics and its build
generated 3,328 pages. Actual Kibita exchanges, skill UI, casting, observer
appearance and live relogin remain pending; this is not `CLIENT_TESTED`.

## Release — 2026-10-23, patch 10045 (ankh tint, second cause) and revision 3

Patch 10044 (`53AE5960`, confirmed installed) still drew the inventory ankhs
grey. The table lookup was correct, but the UI pass leaves texture stage 0 at
`COLOROP = SELECTARG1` (texture only, `SetRenderStateBlock(0)`), so the vertex
tint was discarded; the world pass that draws ground ankhs modulates by vertex
color. The 737..739 branch in `RenderDevicePrimitives.cpp` now sets
`MODULATE(TEXTURE, DIFFUSE)` for that draw and restores the cached stage
values; `AnkhInventoryTintTests` checks the order.

Commits `41e192ad` (source, revision 3 on both sides) and `cd91c527` (portal
receipt), pushed. Gates: full `go test ./... -count=1`, vet, handshake
cross-check at 3, `Build-Client.ps1 -NoDeploy` (ArchitectureTests 3,171,485,
socket 281, crash 200, launcher 56), portal build, 89 tests, `astro check`.
Patch 10045 published and verified first
([launcher receipt](client/launcher.md#published-patch-10045-2026-10-23)).
Archive SHA-256
`D09760D0A696318B0FACD13CBFE8C1FCA2E5EC3BB271489EB173DE54F5AD43E0`; game
`01B7CC40408081C3F5B8DF29ADA7B00AE23673ADBA8CBD08007D4CEECD1A0D8D`, player
API `1026925B2C6AC0F364DD64D1B6AC5247A9269319066BF86157D701F1D749DA60`, log
reader `55513EB6B11555883649E1CE263D2C8B5A2AE9A50FE8EFF54E2689F7E14FBE78`,
revision `cd91c527`. All nine private configuration files matched staging.
No players online; the controlled stop at 15:35:45 UTC persisted zero
accounts with zero failures. Release `/srv/wyd-go/releases/20261023T153545Z`
is live (game listening 15:35:49 UTC); rollback reference
`/srv/wyd-go/releases/20261023T053602Z` (pairs with 10044, revision 2).
Readiness 200/`ready`, schema 11, no error-priority journal entries, remote
staging removed. Pending: visual confirmation of the inventory ankh colors
and a first revision-3 login; not `CLIENT_TESTED`.

## Release — 2026-10-23, patch 10044 and client build revision 2

Commits `271a75d4` (source: ankh inventory tint, Growth auto-buff, CON-free
camera zoom, revision 2 on both sides) and `613a5cce` (portal receipt),
pushed. Patch 10044 published and verified first
([launcher receipt](client/launcher.md#published-patch-10044-2026-10-23)).

Gates: `go test ./... -count=1` and `go vet ./...` (no persistence change;
the PostgreSQL run of 10043 still covers the unchanged store), handshake
cross-check with both constants at 2, `Build-Client.ps1 -NoDeploy`
(ArchitectureTests 3,171,484, socket 281, crash 200, launcher 56), portal
build, 89 tests and `astro check`.

Archive SHA-256
`87D4100A4FF258C6F502FDDC1ED84FDB051F86180C27E1ECA1CADD8A7CBD4C18`. Game
`F893ADCB2F65BAFA13E22BF4F23E1F2C4857E80404394DF555590F0424C5CE9D`, player
API `2B286B1C68DFD2714A8CD1CB0105988697D42A9A8CF48F5871A38BD6EA137B4E`, log
reader `62B2E5EE307C079B9EA59ED0EBF494235790B1B2A5E5ED5B02232487762B7781`,
revision `613a5cce` (`vcs.modified=true`: excluded local files only). All
nine private configuration files matched staging. The first archive attempt
failed because the build script ran from Git Bash, where `tar` resolved to
GNU tar (it reads `C:` as a remote host); the PowerShell run uses Windows
`tar`. The incomplete staging folder was removed.

No players were online. The controlled stop at 05:36:02 UTC persisted zero
accounts with zero failures (`Result=success`); release
`/srv/wyd-go/releases/20261023T053602Z` went live and the game listened at
05:36:06 UTC. Rollback reference: `/srv/wyd-go/releases/20261023T012326Z`
(pairs with patch 10043, revision 1). Running executables match the hashes
above; staff and player readiness 200/`ready`; schema 11; no error-priority
journal entries; remote staging removed. The gate held in production: a
revision-1 client was rejected at 05:53:57 UTC and a revision-2 client
logged in at 05:54:11 UTC. Pending: in-game acceptance of the three fixes;
not `CLIENT_TESTED`.

## Release — 2026-10-23, patch 10043 and mandatory client build gate

Commits `4dcbeed3` (source) and `78bb9bd4` (portal receipt), pushed to `main`;
local `tmproject/client748/config.txt` and the client crash dumps excluded.
Patch 10043 was published and verified first
([launcher receipt](client/launcher.md#published-patch-10043-2026-10-23)).

Gates: full `go test ./... -count=1` with a temporary PostgreSQL 18 cluster
(79 store tests ran), `go vet ./...`, race tests for wire/net and
login/admission/fairy/Water/summon/Lactolerium paths, `Build-Client.ps1
-NoDeploy` (ArchitectureTests 3,171,479, socket 281, crash 200, launcher 56),
portal build, 89 tests and `astro check`.

Archive SHA-256
`3FEAFA5B042828B3D414D1D2F1FEBBEAA0152828B9FAF28832AE31A17D3B305B`. Game
`FDB370E82A7FD6522CB3B165B9544B839603F887D449FCD4332C3C205772801C`, player
API `F98295E4AEFF90B5CDF4A9B8064EAC1DDB8BDE977EFE40B67755606CA4CFA783`, log
reader `3C9E612BCD5739B0CB9E40DF22B07DD5E4C0F0AF486E98632ADA7DD4174C7599`,
revision `78bb9bd4` with `vcs.modified=true` (excluded local files only). All
nine private server, staff, Caddy and unit files matched staging.

The first updater run found three online players, refused the restart and
restored the active release unchanged. With the owner's authorization, the
controlled stop at 01:23:25 UTC persisted three accounts with zero failures
(`Result=success`). Release `/srv/wyd-go/releases/20261023T012326Z` went
live; the game listened at 01:23:30 UTC. Rollback reference:
`/srv/wyd-go/releases/20261022T222536Z` (a rollback must pair it with a
client that does not require the gate). Running executables match the
hashes above; staff and player readiness 200/`ready`; schema 11; no
error-priority journal entries; remote staging removed.

Gate in production: build `0x00000000` rejected before authentication at
01:24:17 UTC; a revision-1 client logged in at 01:24:37 UTC. Pending:
in-game acceptance of the batch contents (sizes, ankh colors, fairy/Water,
weapons, mount HUD); not `CLIENT_TESTED`.

## Local source — 2026-10-23, ankh inventory colors and Lactolerium 100%

- Ankh inventory color (`PARIDADE_NATIVA` fix, 2026-10-23, after patch
  10043 showed the ankhs still uncolored): the inventory renderer keeps
  meshes 737..739 out of the ordinary item path and, as native
  `FUN_0042e8a1`/`FUN_004f59b4`, tints every vertex with
  `{0xFFFF4400, 0xFF3366FF, 0xFF00FF00}[mesh - 737]` (Justice MP red-orange,
  Eternity HP blue, Glory critical green). The port wrote the decompiled stack
  offset (`n3DObjIndex - 2964`) instead of the table lookup, so inventory
  ankhs lost their color; ground items already used the table. Fixed in
  `RenderDevicePrimitives.cpp`; `AnkhInventoryTintTests` guards both
  renderers. The earlier halo entries in `native_inventory_glow::Background`
  were never reached for these meshes and were removed, keeping the native
  halo catalog exact. Client gate passed (ArchitectureTests 3,171,482);
  candidate `WYD.exe` SHA-256
  `C5703F6E0D844C20D3B87001EB1EB0207369428F257A7404859D665BAAEFD1D4`. Not
  committed or published; needs the next patch (build revision 2).
- C.C. auto-buff (2026-10-23): TK Growth (skill 13) was missing from
  `cc_mode::IsAutomaticBuff`, although the same policy already gave it a
  mastery slot. Growth applies affect 14 (server "Possessed"), which
  `g_AffectSkillType[14]` maps back to skill 13, so the existing rebuff
  timing recognizes it. It is now in the list; `CCModePolicyTests` pins the
  set. Client gate passed (ArchitectureTests 3,171,482); candidate `WYD.exe`
  SHA-256 `203C8CECE4CC4A93AD39AD3FB8959618FDD84E9300DE678714B5D81868966314`
  (also contains the ankh fix). Not committed or published.
- Camera zoom with high CON (2026-10-23, `MODERNIZACAO_COMPATIVEL`): the
  wheel's near limit in `EventTranslator::CameraEventData` still added native
  `CON * 0.00019` to 1.2 (2.5 mounted). With the 100,000-CON test amulet it
  reached about 20.2, above the 20.0 maximum, so the camera could not zoom in.
  Patch 10040 had removed CON from `TMCamera` but missed this limit. It no
  longer reads CON, matching the fixed player size; `FieldViewContractTests`
  guards it. Client gate passed (ArchitectureTests 3,171,484); candidate
  `WYD.exe` SHA-256
  `4B552FCE7E855EB4CE312DF1DA8745624712C9E3D8040C6EB3FE889A56D3DBCA`
  (also contains the ankh and Growth fixes). Not committed or published.
- Reported, not reproduced (2026-10-23): attacking stops when a party request
  arrives. Static review found no dialog, focus change or attack-state reset:
  the client `OnPacketREQParty` only fills the party list, opens the party
  panel and writes chat lines; the parameterized `-938` notice is a
  non-modal panel; the server stores only `InviteFrom/InviteUntil`, which no
  combat path reads; the C.C. loop has no party gate. Needs reproduction
  details before a change. The owner confirmed the C.C. macro was attacking;
  the server log has no macro-attack records. Pending a trace reproduction
  ([additional player reports](BUGS.md)).
- Lactolerium 100% (`EXTENSAO_COORDENADA`, new catalog item): item **3022**,
  free on both sides, uses the Fury Stone (3020) mesh/texture `2874.8`; 3020
  is unchanged. It carries Lac's `EF_VOLATILE 5`, so the native client already
  offers drop-on-equipment refinement, and price 900. The server rule
  (`volatiles.json` item 3022: `refineMax 9`, `refineTenChance 100`, new
  `refineChance 100`) allows exactly what Lac allows and succeeds on every
  step, by the owner's choice: up to +9, +10 to +11, and the equipped earring
  +9 to +10 and +10 to +11, which therefore never breaks. `refineChance` is
  validated 0..100 on refine rules only; ordinary Lac keeps the native rates.
  Files: server `itemlist.csv`, `Itemname.csv`, `volatiles.json`, test shop
  `ShopVolGen2`; client `ItemList.bin` record 3022 (SHA-256
  `81AD6789B9AB0B4607E5F76EF9772676A6DC9FDDC79790EB1EC31CDA4E53EDFC`) and an
  `itemhelp.dat` tooltip (SHA-256
  `47C97E7FCD119C4D8B5C2E549AF80C5CFE1ACBBF4741055C050D96459C633FB3`). No
  `Itemname.bin` record is needed: the overlay applies only to listed IDs.
  The test shop `ShopTest2` sells it beside Lac (slot after 413, stack of
  120, like Ori and Lac); a coverage test requires all three. No player shop,
  drop or event gives it yet.
- Validation: full `go test ./... -count=1`, focused vet, Lac/earring/+10
  regressions and new `perfect_lactolerium_test.go` (worst roll on an earring
  still succeeds and consumes the stone); `Build-Client.ps1 -NoDeploy`
  (ArchitectureTests 3,171,479, socket 281, crash 200, launcher 56).
  Candidate `WYD.exe` SHA-256
  `7FE98A22B51E2EA8864FF31483186866075B648F81C554592218AD523E55DC64`, from the
  shared tree with other uncommitted client work. Not published,
  not `CLIENT_TESTED`. The client patch needs the next build revision and the
  matching server release (see the client build admission rule).
- Observed, not changed: the 2007 `Itemname.bin` overlays numeric labels
  (for example `2874.8` for Fury Stone 3020, and 33 other items such as
  649/650 and 1901..1912) onto the item list names.

## Local follow-up — 2026-10-22, monster CON and maximum visual size

Implemented and automated-tested locally, not installed, published or restarted.
This is `MODERNIZACAO_COMPATIVEL`: an explicit project presentation/data policy,
not a claim that native 7.48 imposed these limits. Existing packets are unchanged.

- Monster visual scale cannot exceed `(5000 / 2000 + 1) * 0.89999998`, including
  bosses and race-25 multipliers. Player scaling remains unchanged. Historical
  signed-short CON interpretation is preserved before applying the ceiling.
- Twenty ordinary Colosseum, Hell Gate and Rune Track templates above 1000 CON
  now use 100. Templates at or below 1000 are unchanged. Five event objective
  templates retain their CON and are explicitly tagged `combatRole: "boss"`:
  `COL_Lugefer` (1300), `RT_Balrog` (2000), `RT_BunnyKing` (3000), `RT_MageBoss`
  (1500), and `RT_Valkyrie` (2500). The tag documents the exception; it introduces
  no new AI. Colosseum A generator 106 is the final encounter; Rune Track kill
  transitions 5798/5908/5776 and 5957..5964 identify the other objectives.
  Pack/wave monsters such as `RT_LichKing`, `RT_Sulrang` and `COL_Talos` are not
  exempt merely because they lead a group or have a boss-like name.
- The Auto Trade clone copied its owner's complete score despite using a monster
  ID and Carbunkle face. It now takes CON from the Carbunkle appearance template
  (currently 6), or 0 when unavailable, preserving all other copied score fields.
  Owner score is untouched. Existing shop visibility snapshots carry that same
  corrected clone score to the owner and observers. This is a source-confirmed
  cause of oversized Carbunkle shop clones, not proof of the reported live entity.
- 2026-10-23: server companions named `Name^` (BM and contract summons, the
  mount hatchling) keep the base visual scale. A BM summon copies its caster's
  CON (`skill_summons.go`), so high-CON casters produced giant summons. The
  client rule (`monster_visual_scale::IsSummonName`, applied first in
  `SetCharHeight`, with the native whole-character `^` check) changes size
  only; the authoritative summon score is unchanged. Client gate passed
  (ArchitectureTests 3,171,478 checks); candidate
  `tmproject/build/TMProject748/Release/WYD.exe` SHA-256
  `AA6352EA90E0305B061B2E0323BC68CD29CA7A8C020805AB8ED21C3B1A324218`, built
  from the shared working tree with other uncommitted client changes. Not
  installed, published or `CLIENT_TESTED`.
- The four configured world-boss Lua CON values remain 5000. Boss overrides are
  not normalized, including larger configured values; only their client scale
  is capped. A catalog regression test checks all ordinary monster templates
  and the exact twenty corrections/five preserved exceptions.

Validation: full `go test ./... -count=1`, `go vet ./...`, focused monster/clone/
boss/Rune Track tests and `Build-Client.ps1 -NoDeploy` passed. Client architecture
tests passed 3,171,450 checks, including all 65,536 legacy word CON values, wide
values, invalid floats and repeated race multipliers; socket/crash/launcher
checks passed 281/198/56. Candidate `tmproject/build/TMProject748/Release/WYD.exe`
SHA-256: `886B33B3F7CEE864FA6D7F6730BAE295E47CD69510F229D9F9657028D2081CD5`.
Installed `tmproject/client748/WYD.exe` remains
`7F8C10E433E97526525FBDE42DA5458C93F31359CCC686CB01D35E89EDE033A9`.
Actual visual acceptance remains pending, not `CLIENT_TESTED`. Changed text is
English; established JSON keys and persisted identifiers are preserved.
`git diff --check` passed. Cleanup of the new crash-test fixture was blocked by
execution policy; no alternate deletion was attempted. Leftover:
`tmproject/build/tests/Release/crash-capture-tests-29476-1058925578/`.

## Local follow-up — 2026-10-22, Water cycling, fairy lifetime, hand-slot fit and mount HUD

Implemented locally; not installed, published or restarted. These changes are
`MODERNIZACAO_COMPATIVEL`: existing item/instance packets are unchanged, fairy
pause metadata is server-only, and the equipped weapon fit changes presentation
without changing inventory occupancy. The requested behavior is not claimed
as newly proven native parity.

- Silver Angel now consumes an existing matching first-level scroll after a
  successfully completed terminal Water room has durably exited. It preserves
  party admission, exact item UID checks and the normal entry transaction;
  timeout/death, unequipped/expired/paused fairy, or missing scroll stops cycling.
  No replacement scroll is granted and no periodic retry is introduced.
  Room closure clears derived membership indexes before another durable entry.
- Production N/M/A bosses return to `(1966,1768)`, outside the former external
  Room 1 entry rectangle starting at `Y=1772`. Only the three Room 1 external
  entry rectangles now start at `Y=1768`; boss-platform chain requirements and
  the existing return coordinates remain unchanged. A production-table test
  covers all three terminal exits against their first-scroll entry rules.
- Unequipping a functional fairy freezes its remaining lifetime and tooltip.
  Re-equipping shifts its expiry by the paused interval. Optional `pausedUnix`
  follows the UID through account JSON, inventory and Cargo. Entry migrates
  still-live stored fairies; it does not resurrect already expired items or
  refund unknown past consumption. Equipped fairies still consume lifetime
  while offline; costumes and premium mounts retain absolute expiry.
- Equipped left/right hand meshes fit the full hand receptacle rather than a
  Carry cell. Reinsertion into Carry restores the smaller fit; authored mesh
  scale is never exceeded. Source-level layout tests and the Win32 build passed;
  actual equipped weapon appearance remains unverified in the game.
- The selected-skill HUD and skill tooltip now include the equipped timed
  premium/KR mount's exact magic percentage, after amplification and 5/4,
  before passives. The server already applied this bonus to actual spell
  damage; its formula and packets are unchanged. Ordinary adult mounts are
  not multiplied again, and physical/healing/fixed skills retain their paths.
  The client reads live slot 14 on each projection; server equipment updates
  own expiry/removal, with no client-side timer or cached bonus.

Water is not strictly native-equivalent: the existing 27 room/boss definitions
use dedicated templates with project HP/EXP, disabled NPC loot and a cap of 1
on incoming monster hits before player absorption. These balance choices were
not changed. No complete native 7.48 Water contract was established in this
batch; the 7.59 runtime map does not establish 7.48 parity.

The reported premium/KR magic-damage failure was not reproduced on the server.
Production server catalog tests cover all 10 premium and 15 KR mounts with
TK, FM and BM magic skills, verifying the exact mount percentage against
unmounted skill damage. The client projection omission was reproduced in
source and corrected in `BASE_GetSkillDamage`. Client tests exercise all
10 premium and 99 KR production definitions, exact percentage rounding,
invalid lifetimes, ordinary-mount exclusion and wide multiplication.
This does not establish full client/server formula parity or in-game acceptance.

Validation: full `go test ./... -count=1`, `go vet ./...`, focused race tests
for fairy pause/critical equipment/Water restart/mounts/account snapshots, and
`tmproject/Build-Client.ps1 -NoDeploy` passed. Optional live PostgreSQL fixture
tests were not enabled; the snapshot test checks JSON persistence preparation.
Client candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`FF9445EB8B0AC9DFB92B61AF1AD32CFFF255531B5672F51696B368FAE28D4CB8`.
The final client build passed 3,105,911 architecture checks, 281 socket checks,
198 crash-capture checks and 56 launcher checks. The final full Go suite and
focused fairy/Water/mount/score packet regressions passed. Installed local
`tmproject/client748/WYD.exe` remains SHA-256
`7F8C10E433E97526525FBDE42DA5458C93F31359CCC686CB01D35E89EDE033A9`.
The installed client and active VPS release below were not changed. This is
source/build validation, not `CLIENT_TESTED` or global release readiness.

Repository checks: `git diff --check` passed. The layout gate still reports
only the preexisting client crash-monitor directory/logs noted in the release
receipt below. The research-record validator accepted `mount-level-glow.md`
but returned exit 1 for an unrelated, unchanged record:
`.agents/research/client748/flows/combat/persistent-buff-visuals.md` lacks the
required `Gaps` section. No unrelated evidence record was edited.
A cleanup attempt for this run's crash-test output was blocked
by execution policy; no alternate deletion was attempted. Leftover:
`tmproject/build/tests/Release/crash-capture-tests-23544-1054885390/`.
The final client validation created another crash-test fixture directory;
cleanup was also blocked by execution policy, without an alternate attempt:
`tmproject/build/tests/Release/crash-capture-tests-38460-1056697562/`.
Incremental client build output is retained. Changed-file language inspection
found only preserved `Montaria` identifiers/JSON keys in the account contract;
these were not renamed because that would change persisted/API compatibility.

## Release — 2026-10-22, Duel Letter runes and all local server/panel fixes

The owner authorized publishing all local corrections and restarting the
server, not just the rune fix. Built the working tree at
`8273c4e8bdc3b463491a99814e3c24c85d729208` with `vcs.modified=true`, including
the untracked O/X runtime source. This batch includes first-room Duel Letter
rune initialization after the durable entry acknowledgment, O/X lifecycle,
admission, rewards and movement guards, teleport/arena callbacks, stale login
and sanction-result fencing, and panel event/instance/map corrections.
No commit or push was performed. Private client configuration was excluded;
no client patch was published, and the launcher feed remains at patch 10042.

Archive SHA-256
`EEC8D290C56F5DC0D8F0E0850BB319BE55D181AE845447D1808F6846FBDD03C0`
(25,455,220 bytes), verified again on the VPS. Running executable hashes:
game `CF11F9E44F6E2BC5C8899F841FB8BB241F2ACCA9208B68DA77BC21A48B825BE3`,
player API `E1721449DAABD7A1AA04456A5AF09A37985DC848BD7A77887D42754441C800A5`,
log reader `66ACFA084E6E05E8FD94CC3EA9E54C865ED5074AB01824FAA44DF80D5B3F5BB4`.
The archive contained no account files or private runtime configuration.
Server, staff, Caddy and service configurations matched staging before
activation; authentication environment files remained byte-identical.

Active release: `/srv/wyd-go/releases/20261022T222536Z`. Rollback reference:
`/srv/wyd-go/releases/20261022T180448Z`. Zero game connections before the
controlled restart at 22:25:39 UTC (19:25:39 America/Sao_Paulo). The previous
process logged zero account-save failures and `shutdown complete`. Game,
player API and log reader run from the new release with the hashes above.
The pre-update database backup succeeded; receipt:
`wydgo-20261022T222536Z.dump`, 178,159 bytes, SHA-256
`216FE9F373B1EFF618642CA6BA5C5FCCE331FB23A842FC34E4D7D196D10707FD`.

Validation: full `go test ./... -count=1` and `go vet ./...`; focused race
tests for Magic Chamber, O/X persistence/security and login/sanction ordering;
portal checks, tests and production build (3,324 pages); Linux cross-build.
Optional PostgreSQL fixture tests were not enabled in this run; the live
database remains at schema 11. Post-activation staff/player readiness returned
200/`ready`, game port 8281 listened, public portal returned HTTP 200, and
deployed portal/admin entry-point hashes matched staging. Startup logged the
expected revision and executable hash; no error-priority journal entries
were present for the three services during the activation check.

`git diff --check` passed. The repository layout gate remains blocked only by
preexisting `tmproject/client748/dumps/` and its crash-monitor logs; these were
preserved. Remote task staging was removed. Local cleanup was blocked by the
execution policy; the package, extracted candidates and configuration copies
remain in `build/deploy/wyd-deploy-20261022-local-fixes/`. No alternate deletion
method was attempted. Incremental portal build output is retained.
Pending: actual first-room rune visibility and
O/X/teleport/panel acceptance in the running client/browser. Publication and
readiness do not establish `CLIENT_TESTED` or complete language migration.

## Release — 2026-10-22, staff panel stages 0–3, Big Cube and patch 10042

The owner authorized commit/push to `main`, VPS publication and a service
restart (not a machine reboot). Source commit `379dcd93`, portal receipt
commit `0f4b8b71`; both pushed to GitHub `main`. Local
`tmproject/client748/config.txt` remains excluded. Client patch: see
[launcher patch 10042](client/launcher.md#published-patch-10042-2026-10-22).

Archive SHA-256
`DCC1EC8C3F64A08C31C9E29B78E8B857CF89768A16A3CA37D58BCBD4A57FE6C3`, checked
on the host before activation. Game binary
`4B7A19D0F96DDB3B39FECC6DB3A108D60CC4141273DAA9436D39701EC9855B86`, player
API `A2967E413137C33FFB9E43D438D942AF2E76F1A16871C9E436C3D25FE0EAAA02`, log
reader `DC64DD149625812F6F3B39D7DDE552EE58D9E44961A55DA08D06E7924330ED65`;
all embed revision `0f4b8b71` with `vcs.modified=true` (only the excluded
client configuration and a research note were dirty). Live server, unit and
Caddy files matched staging except the new `web_admin_log_reader_socket`
line; staff membership was unchanged, adding the seven panel capabilities.

Zero game connections before activation. The updater ran the existing backup
(`Result=success`), then the controlled stop at 18:04:51 UTC logged zero
persisted accounts, zero failures and `shutdown complete`. Release
`/srv/wyd-go/releases/20261022T180448Z` went live with game and player API
restarted; the game listened on 8281 at 18:04:52 UTC. Rollback reference:
`/srv/wyd-go/releases/20261021T190239Z` (schema 8 marker restore is built
into the updater).

Verified after activation: running `/proc/<pid>/exe` paths are inside the
new release with the hashes above; schema 11; staff and player readiness
return 200/`ready`; a `server.start` audit row is persisted; the
`wyd-log-reader` socket (`wyd-log-reader:wyd-game 0660`) served a real
journal read to `wyd-game` and refused `wyd-player`; no game, player API or
log-reader journal entries at priority error or above. The first backup
receipt (`status.json`) appears with the next timer run, because the backup
during the update used the previous script. Remote staging was removed.

Pending: authenticated browser acceptance of the panel, in-game acceptance of
moderation/notices/maintenance and the Big Cube flow (not `CLIENT_TESTED`).

Follow-up 18:18 UTC: `game.event.colosseum` was added to `flptr` in the
staff template and the local staff file, and installed as
`/etc/wyd-go/staff.json` (`root:wyd-game 0640`; previous file kept as
`staff.json.pre-20261022T181843Z`). Membership unchanged; only that
capability added. The game reads the file per check, so no restart was
needed; staff sessions sign in again because the capability set changed.

## Local source — 2026-10-22, staff panel P1 and stages 0–3

The staff panel adds persistent bans and chat mutes, scheduled staff notices
and private staff messages, scheduled maintenance shutdown/restart, a
persisted audit history, runtime charts with a load circuit breaker, account
login/action history and storage views, server logs and backup receipts
through the `wyd-log-reader` helper, a live map, an instance list, a static
content index and case notes. Plan and per-package status:
[admin panel roadmap](server/admin-panel-roadmap.md); behavior and
capabilities: [staff operations](server/staff-operations.md). Classification:
`MODERNIZACAO_COMPATIVEL`; notices, whispers and disconnects reuse existing
wire messages, and no client change is required.

Source API version `0.14.0`; schema `11` (v9 sanctions and staff audit, v10
account logins, v11 case notes), applied by the game binary at start. The
VPS update script installs the log-reader socket and user, and its rollback
restores the schema 8 marker. Site news moved to Markdown files; the
release post is `big-cube-staff-notices-10042` (patch 10042).

Final audit 2026-10-22: full `go test ./... -count=1` and `go vet ./...`;
store, player API, webadmin and server tests against a temporary PostgreSQL
18 cluster, including a v11 → v8 marker rollback and re-upgrade; race tests
on staff operations, log reader and control; Linux cross-build; portal
build, tests and `astro check`; repository layout gate. Pending: VPS
activation, the log reader against the real journal, and authenticated
browser and in-game acceptance. No commit, push or publication yet.

## Local source — 2026-10-21, Colosseum notices and wave countdown

Colosseum now announces scheduled sessions globally at five minutes and one
minute before entry closes, using the configured guild-war timezone. Notices
are deduplicated per tier and local day. A late tick announces only the current
remaining time, not expired warnings. An already claimed tier, active run or
pending durable exits suppress advance notices.

Both scheduled and staff-triggered starts announce that the entrance gates
have closed. The first wave announces combat globally; subsequent waves notify
only public-world players in the arena. Notices also remain in chat, rather
than relying solely on the transient notice panel. Manual starts remain
immediate, without a new admission delay or an extra scheduled session.

Arena participants receive the existing `0x3A1` HUD countdown to each next
wave: offsets +1, +4, +6, +8 and +10 minutes from event start. The client
decrements locally; the server sends a new deadline only on a wave transition
or for a newly observed participant. Leaving, finishing, disabling the event
or an overdue wave retires the countdown. Cleanup does not overwrite a newer
private-instance or public-area timer. The quest reset/guard does not own this
countdown. Pending Elixir persistence cannot invent a later wave deadline.

Classification: `MODERNIZACAO_COMPATIVEL`, preserving the existing notice/chat
and timer wire contracts, admission, persistence, rewards and scheduler. No new
packet, client binary or native-parity claim. Focused Colosseum tests and the
unchanged client ArchitectureTests executable (3,105,777 checks) passed.
Full `go test ./... -count=1`, `go vet ./...`, `go build ./...`, focused
Colosseum/public-area timer race tests and `git diff --check` also passed.
The layout gate still fails only on the preexisting
`tmproject/client748/dumps/` and `crash-monitor-p26020.log`; both are preserved.
These are local source changes, not a VPS release or actual in-game HUD
acceptance; not `CLIENT_TESTED`. No installation, restart, disconnection,
commit or push was performed.

## Local source — 2026-10-21, regular Cube question visibility

Regular, Mystic and Arcane Cube now use the private timed instance-question
panel already implemented for Big Cube. All 24 questions before the final
combat room have ten-second configured and authoritative answer windows.
The question no longer uses the shared notice panel, so actions and EXP
notices cannot replace it. Stale kill timestamps are refreshed before opening
the window. Answer persistence closes every participant's question before
publishing results; failed commits publish neither close nor rewards.

This continues `EXTENSAO_COORDENADA` v1, with no new opcode or client change.
The [existing contract](protocol/instance-question.md) now documents both
Cube lifecycles. Validation passed: full `go test ./... -count=1`,
`go vet ./...`, `go build ./...`, focused race tests for Cube/Big Cube,
atomic answer publication, data configuration and wire, plus the existing
client ArchitectureTests executable (3,105,777 checks). Client question state
and packet tests exercise expiry at 10,000 ms; gameplay visibility during
actions and EXP is source-reviewed, not actual in-game acceptance.

The unchanged client candidate and its hash are recorded below. These changes
are local source only: no client installation, publication, commit, push,
restart or player disconnection. Real in-game visibility remains pending;
not `CLIENT_TESTED`.

## Local source — 2026-10-21, Big Cube ticket and return portal

Entrance Ticket admission now accepts the owner's inclusive rectangle
X1308..1375 / Y1444..1743. The portal at (1342,1466) returns to Noatum
(1058,1725), using the existing route's 700-gold tariff. Client and server
attribute maps enable that portal's 4x4 block with one byte changed in each
map (`0x04` to `0x14`); other bytes, including the client trailer, are unchanged.

The client confirms the ticket before sending its use intention. Big Cube
questions start only after persisted admission and use a dedicated private
panel for the authoritative ten-second answer window, independent of modal
confirmations and ordinary notices. See the versioned
[instance question contract](protocol/instance-question.md).

Validation passed: full `go test ./... -count=1`, `go vet ./...`,
`go build ./...`, focused race tests for Big Cube/admission/wire/portal,
terrain synchronization and `Build-Client.ps1 -Configuration Release -NoDeploy`.
Client gates: ArchitectureTests 3,105,777 checks, SocketReceiveTests 281,
CrashCaptureTests 198 and LauncherTests 56, all passed. `git diff --check`
passed. The documentation map was refreshed; the layout gate still fails
only on the preexisting client crash-monitor file/directory noted below.

Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`7F8C10E433E97526525FBDE42DA5458C93F31359CCC686CB01D35E89EDE033A9`.
Client Env attribute SHA-256:
`3D56C0E1EA4AC286EF74D9A7AB764B02AF77964D904F9A3D21AD0B83444465F4`.
Server attribute SHA-256:
`A5238811FE70330DC02A0F2B5D9BDB063A3DD1FB37CBD247F969A95B3AC73358`.
These sources/assets and candidate are local, not an installed runtime or
published client/server release. No commit, push, player disconnection or
restart was performed. Actual in-game confirmation, movement during the
question, O/X resolution and portal use remain pending; not `CLIENT_TESTED`.

## Local source — 2026-10-21, event schedule and staff controls

Colosseum defaults now start N/M/A at 18:05/19:05/20:05 in the configured
guild-war timezone. The admin panel can advance a selected tier, retaining
the same 12-minute lifecycle and authoritative admission/persistence rules.
An early start claims that tier's scheduled session for the same local day
in the current process; claims and command receipts do not survive restart.
The OX panel adds a one-click start that enables quiz entry and opens its
six gates, without reopening an already active question on a repeated start.

API version is now `0.13.0`. Details and capability requirements are in
[website feature status](server/website-feature-status.md#colosseum-and-ox-controls--2026-10-21).
No VPS activation, player disconnection, commit or push was requested for
this batch. Automated validation does not establish authenticated browser or
in-game acceptance; the deployment receipts below describe earlier code.

Validation: `go test ./... -count=1`, `go vet ./...`, `go build ./...`, portal
type generation, `npm run check` (80 files, zero diagnostics), `npm test`
(65 passed), `npm run build` and focused `go test -race` for the Colosseum/OX
game, control and webadmin tests passed. The repository layout check remains
failed on the unrelated client crash-monitor output
`tmproject/client748/dumps/crash-monitor-p26020.log` and its directory;
this task did not create or remove them.

## Release — 2026-10-21, audit batch and patch 10041

The owner authorized commits/push to `main`, VPS activation and necessary
player disconnection. Source commit `6746e784` and portal receipt commit
`60c2b618` are pushed to GitHub `main`. The required `ItemListWide.csv` is
now tracked. Local `tmproject/client748/config.txt` remains excluded.

The Linux archive passed its remote SHA-256 check before activation.
Archive SHA-256:
`D8AD95D1ABA052D1D2905C24FC341A031D17F1C9CE98AD87C9F47B5189166669`.
The candidate game binary has SHA-256
`D506103DE1C05F8E3D143439B28A61966784544552FF360DFCF1FFCD353DA5D6`;
the player API has SHA-256
`4940149C382C194D749C1C3E81F1B7A2868DC9FD57A32371AD183D17718BA610`.
Both embed revision `60c2b618193c10de220973aaa0c560bd4137f791` with
`vcs.modified=true`: the excluded local client configuration was dirty, not
the Go sources. Existing private server/Caddy/systemd settings were copied
into staging; existing staff membership/capabilities were preserved, adding
only the two audited capabilities. The backup service completed successfully.

One game connection was present before the authorized controlled stop at
18:59:20 UTC. Shutdown logged one persisted account, zero failures and
`shutdown complete`; systemd reported `Result=success`, `ExecMainStatus=0`.
No game connections remained. The previous/rollback release is
`/srv/wyd-go/releases/20261020T235014Z`.

After upload, the patch publication SSH connection timed out before
authentication. TCP diagnostics found port 22 timing out while HTTPS remained
reachable. At 19:01 UTC the feed still ended at 10040 and patch 10041 returned
HTTP 404. After a connection-free backoff, established SSH access recovered;
no credential, SSH policy or firewall workaround was used. The publisher
installed the ZIP before appending 10041, then the established release updater
activated `/srv/wyd-go/releases/20261021T190239Z` at 19:02:42 UTC, restarting
the game and player API. The game was unavailable from its controlled stop
until the new listener started at 19:02:43 UTC.

Both services are active. Their running `/proc/<pid>/exe` paths resolve inside
the active release, and both executable hashes match the candidates above.
Both loopback readiness endpoints return `ready`, TCP 8281 listens, and the
post-activation game/API error journal has no entries. Startup confirms
PostgreSQL persistence, four critical-write workers, four boss encounters and
the game listener, without an admission-containment error. Readiness and startup
are not a login or gameplay test. The backup service again reports success.
The updater and publisher removed both remote staging directories.

The published three-entry client ZIP is 900,489 bytes, SHA-256
`4CF0535F04C58362478F4927D50717FD6D1140B4262439CBB357DC8790BB0628`.
It contains the audited `WYD.exe`, `ItemList.bin`, and `ItemListWide.csv`,
without private settings. Fresh HTTP and HTTPS downloads match the ZIP hash;
the HTTPS archive's three entry hashes match the candidate/asset allowlist.
Both public feeds end at 10041. The public manifest reports the correct ZIP
size/hash and executable hash, and home/download/News/changelog/article return
HTTP 200 with patch 10041. Portal check/build and all 60 tests passed again.
The local installed runtime was not replaced. Native formula conflicts,
real-client acceptance and global English migration remain pending; publication
does not establish `CLIENT_TESTED` or global release readiness.

Remote staging was removed automatically. Local cleanup was rejected by
execution policy before execution and was not retried. Exact retained output:
`build/deploy/wyd-deploy-20261021-audit-10041/`,
`build/patch/patch-10041.zip`, `build/release-10041/verify-10041-http.zip`,
and `build/release-10041/verify-10041-https.zip`.
An unrelated stopped client's 51-byte crash-monitor log was preserved by moving
its otherwise empty dumps directory to
`build/release-10041/local-client-dumps-p29464/`; layout validation then passed.
The earlier blocked audit-cluster artifacts below were not touched.

## Source corrections — 2026-10-21, generated-code audit

The [final audit record](../.agents/research/native-combat-audit-2026-10-21.md)
covers the October 19-21 changes and the working-tree batch. Colosseum reset
and admission, Rune Track eligibility, ghost-shop honor durability and required
`ItemListWide.csv` dependency checks were corrected. The final review also
fixed critical-write entry/exit handling, skipped-hour finalization, overlapping
OX sides, boss shield overflow, duplicate boss registration, wide combat HP
projection, and authoritative snapshots leaving active big HP/MP pools stale.

The full Go suite passed with isolated PostgreSQL enabled; dedicated transaction
and World activation tests confirmed execution rather than conditional skips.
Vet/build, focused regressions, portal check (77 files, zero diagnostics), all
60 portal tests, and client Release build passed. Client checks: 3,105,526
architecture, 281 socket, 198 crash capture and 56 launcher. Candidate:
`tmproject/build/TMProject748/Release/WYD.exe`, 1,916,928 bytes, SHA-256
`64C9BDDEEC0EE6E6197B63A0CA915B10BE1AEA8F1C6B03A1F9D3EC47428C3F82`.

At the original review boundary only source and automated-test validation had
occurred. The release above subsequently committed the required sidecar and
published the validated artifacts, without local installation or gameplay
execution. Native 7.48 formula conflicts and real-client acceptance remain
pending; no global language/release-readiness claim is made. The stopped test
PostgreSQL cluster and log remain at `build/final-audit-20261021/` because cleanup
was blocked by policy; see the audit record for exact paths.

## Release — 2026-10-20, player camera independent of CON (patch 10040)

The owner reported that patch 10039 restored normal player model size, but
the camera still tracked the character as if the 100,000-CON amulet had made
them giant. `TMCamera::GetCameraPos` still added player CON to mounted minimum
distance and mounted/unmounted camera height. Those three additions are removed;
mount-type distances and seated offsets remain. The source contract now guards
the camera against CON coupling alongside the existing player/monster scale
checks. `Build-Client.ps1 -NoDeploy` passed (architecture 3,282,440 checks,
socket 281, crash capture 198, launcher 56); the candidate executable at
`tmproject/build/TMProject748/Release/WYD.exe` has SHA-256
`81E6662D8867268E7ADE81FCDE14C8B264DBE9D72026C96B8BCFE90732689B88`.
The launcher feed now lists patch 10040. Public HTTPS download of its sole
`WYD.exe` entry matched the candidate SHA-256 above; the ZIP is 851,211 bytes
with SHA-256
`76905B3E7258F97CEE6EE4C0B0504A2951A5A77D8B21F02715EB75816F4F3DEC`.
The static website manifest, home, News and changelog advertise patch 10040.
The portal build and all 60 tests passed. A website-only VPS release at
`/srv/wyd-go/releases/20261020T235014Z` changed six public files; its game
and service configuration matched the previous release, and the updater
reported no game or player API restart. The active game executable hash remains
`3BE39B5503C1238EC286B45B88BD175A59EDFEDA8496923297F022D89361FB74`;
both services remain active. The installed local `tmproject/client748/WYD.exe`
remains patch 10039 with SHA-256
`40A4D94DDFE14F3A873945BEBF0463D39B1979F97D4230B456DE222C06CDFAF9`.
Publication is verified, but this is not `CLIENT_TESTED`: gameplay acceptance
must compare the camera with and without the amulet, mounted and unmounted,
after the launcher installs patch 10040.

## Release — 2026-10-20, player scale and world boss CON 5000

Client patch 10039 is published in the launcher feed and available publicly as
`/patch/patch-10039.zip`. The archive is 851,253 bytes with SHA-256
`82E56AAD4E6936C038F4305E89B6D21A003CBA3938F28963E3398B48DA08A672`;
its sole `WYD.exe` entry has SHA-256
`40A4D94DDFE14F3A873945BEBF0463D39B1979F97D4230B456DE222C06CDFAF9`.
The client change keeps player height independent of CON while preserving
monster scaling. The client build and automated checks passed, but the local
installed executable was not replaced and actual gameplay acceptance remains
pending.

The owner authorized a game restart and possible player disconnections. Four
established connections were present before the game service was stopped. The
server release changed only the four boss Lua files relative to the previous
active game's files, setting each encounter CON to 5000. The established
updater activated `/srv/wyd-go/releases/20261020T233645Z`, restarting the game
and player API. The final website-only release, archive SHA-256
`CCA31AC5A60CB7FCD944651143CBBD8E27B5E6FEC1E8EC528E1729723A0CA5D0`,
activated `/srv/wyd-go/releases/20261020T234007Z` without restarting either
service; only three public website files differed from the preceding release.

The active game process still matches the deployed binary SHA-256
`3BE39B5503C1238EC286B45B88BD175A59EDFEDA8496923297F022D89361FB74`.
All four installed boss Lua files contain CON 5000; game and player API are
active, both loopback readiness checks return HTTP 200, TCP 8281 listens, and
the game error journal has no entries since activation. The public manifest
reports patch 10039 and server update `world-bosses-con5000-10039`; its news
page and launcher feed return the updated content. The portal build and all 60
tests passed. In-game visual and boss-encounter validation remain pending;
deployment checks do not establish `CLIENT_TESTED`.

## Release — 2026-10-20, world boss CON hotfix

The owner requested immediate correction of the previous server release. The
four boss encounters now use CON 10000 without changing their KR preview NPCs.
The hotfix reused the 10038 release payload and replaced only the game binary
and four boss Lua files. The local full Go suite and vet had passed before
packaging. The archive SHA-256 was
`D11875BE0FEC796F6A92B6E61D71F1EE67C119E931C383EE263412ACFE30C069`;
the upload passed the remote checksum and archive-entry checks. The previous
release was `/srv/wyd-go/releases/20261020T230245Z`.

The owner had authorized disconnecting players for the game restart. Two
established game connections were present. The game service was stopped before
the established updater activated `/srv/wyd-go/releases/20261020T232346Z`,
then both game and player API restarted. The active game binary SHA-256 is
`3BE39B5503C1238EC286B45B88BD175A59EDFEDA8496923297F022D89361FB74`.
The running game process executable matches that binary hash. All four
installed boss Lua files contain `con = 10000`; both services are active, both
loopback readiness checks return HTTP 200, TCP 8281 listens, the post-activation
game error journal shows no entries, and remote staging was removed by the
updater. Actual in-game visual
confirmation remains pending; this deployment is not `CLIENT_TESTED`.

Local staging remains at `build/deploy/wyd-deploy-20261020-boss-con-hotfix/`.
An earlier failed relative-path command left generated output under
`wydgo748/build/deploy/wyd-deploy-20261020-boss-con-hotfix/`; cleanup was
blocked by execution policy and was not retried.

## Previous local follow-up — 2026-10-20, world boss scale

The owner confirmed that CON 10000 was intentional for the four configured
world bosses: DeathWorm, LegendBerial, Murmy, and Anubis. The local source now
supports a per-encounter `stats.con` override and sets it to 10000 in all four
boss Lua definitions. The corresponding KR catalog NPCs retain CON 1 because
NPCGener also spawns them as one-mob preview groups. The existing client scale
formula uses the transmitted CON, so no client source change is required for
this correction. The boss catalog is loaded only at game-server startup; Lua
edits are not hot-reloaded. The subsequent hotfix above built, published, and
activated this correction; validation in a running client remains pending.

## Release — 2026-10-20, world bosses, wide values and patch 10038

The owner authorized activating the pending VPS release, including game and
player API restarts. The client patch 10038 was already listed in the public
feed. Its public ZIP and local archive both have SHA-256
`A0012C6E5215A6693B3DBE1001EA2B465B34290675F6F87C0891B415834AF571`
(900,401 bytes); a fresh `Build-Client.ps1 -NoDeploy` produced the same `WYD.exe`
as the patch, SHA-256
`B2417172F15E968E26A6F6B1BB8451F2CE7125476000CE3295079EE01FF54A27`.
The local installed client was not replaced. Client architecture, socket,
crash-capture and launcher tests passed; Go `go vet ./...` and
`go test -count=1 -p 1 ./...` passed, as did the portal's 60 tests.

The first prepared server archive was not activated: fresh tests exposed three
conflicts with existing boss-data expectations. The owner's deliberate NPC swaps
for different monster faces were preserved: DeathWorm, LegendBerial, Murmy and
Anubis remain the four boss base NPCs. Encounter names and stable IDs were
restored to match the four Armia gates and public announcement; two NPC
references were changed from filenames to catalog names. DeathWorm's CON was
changed from 10000 back to 1 to satisfy the existing base-NPC test. The owner
subsequently confirmed that 10000 was intentional for the boss itself; the local
follow-up above keeps that value on boss encounters without changing preview
NPCs. The focused boss/preview/reference tests and full Go suite passed at
the time of this release.
The corrected archive SHA-256 is
`4576233701806CFDCC1762A8959FA0F71EA8062AD2BD3CB713A3E4D5960C1679`.
Remote checksum and archive checks passed, private configuration matched the
existing VPS files, and no game connections were established before activation.

The updater backed up the database and activated
`/srv/wyd-go/releases/20261020T230245Z` (rollback
`/srv/wyd-go/releases/20261020T201006Z`), restarting game and player API.
The live binaries match the built payload:

- Game: `90E0E29AA9EECDD7CEE5379FB662B1A0813DC4F0D6590AFC1F02671D5D0562A6`.
- Player API: `95741B48E0D7BD1BE35D92727BAA0357B868A13BF1B606EE407BC905C77CBF4C`.

Both services are active and return HTTP 200 on loopback readiness, TCP 8281
listens, and the post-activation error journal is empty. The public manifest
reports patch 10038 and the release news returns HTTP 200. No actual client
login or gameplay acceptance was performed; `CLIENT_TESTED` remains pending.
The updater removed its remote staging directory, and the obsolete remote
preparation directory was removed separately. Local cleanup was blocked by
execution policy before deletion. The two ignored staging directories remain
at `build/deploy/wyd-deploy-20261020-patch10038` and
`build/deploy/wyd-deploy-20261020-patch10038-r2`; the latest ignored crash-test
output remains at
`tmproject/build/tests/Release/crash-capture-tests-34424-882294218`.

## Release — 2026-10-20, skills, items and patch 10037

The owner authorized publishing the pending updates, the launcher patch and a
game-server restart. `go vet ./...`, `go test -count=1 ./...`, the client build
with architecture checks (3,282,468) and the portal's 60 tests passed. No
gameplay validation was performed. Issue status: [player reports](BUGS.md) and
[bug register](bug-register.md).

Patch 10037 contains only `WYD.exe` (SHA-256
`0254CEA3D2C9D8E6AD3579C07701D132DC6A3F2704A99D9628456BD1829A0D90`). The ZIP
is 849,103 bytes, SHA-256
`B41A987968CF478B1FAAF8330BF695D68A3ADA528C70FB219292096097B3AB32`. The HTTPS
and HTTP feeds list 10036 and 10037; the ZIP downloaded through the public URL
and its entry matched both hashes. Client settings were excluded.

The first server activation (`20261020T200827Z`) failed readiness because
`Build-VpsRelease.ps1` omitted `data/RuneTrack759.txt`; the updater restored
`20261019T050509Z` automatically. The script now packages that runtime file.
The rebuilt archive (SHA-256
`b2c38672b75030aefa022e3062e205fb8d1814eb566f2c66b88ca2e84f24e9a3`) has the
same data inventory as the previous release plus the changed files. Private
server, staff, Caddy and unit files matched the VPS before activation. With no
established game connections, release `/srv/wyd-go/releases/20261020T201006Z`
went live at 20:10:09 UTC (rollback `20261019T050509Z`); game and player API
restarted and the running executables match the candidates:

- Game: `5c5cc23b3c31cc5f95a98d357604fc9d89d1a42545c778a4c36f1f9294783670`.
- Player API: `08cb2db3d044a724c0f83cd43ae8a4e5abc51e5d06ca2225cad59bb1b8acd21a`.

Game, player API and Caddy are active, TCP 8281 listens, Uxmal loaded seven
rooms, and the post-start journal shows no failure. The public home, news,
changelog and manifest (patch 10037, 2026-10-20) returned 200. Remote staging
was removed. The failed release directory `20261020T200827Z` remains on the
VPS as inert evidence. Actual login and gameplay remain pending.

## Previous release — 2026-10-19, persistence recovery and patch 10036

The owner authorized the current server, portal, pending client fixes and a VPS
reboot. Full Go tests and vet passed, including the configured isolated
PostgreSQL tests; the portal's 60 tests, Astro checks and build passed. Client
build and automated checks passed. No gameplay validation or local client
replacement was performed.

Patch 10036 is published. Its independently downloaded ZIP has SHA-256
`E4F2940353D425B0C6DEE0B7F3FC4C9F8A24305D1950EB03D6ED323DA2F4B49B`;
the four entries are `WYD.exe`, `UI/etc.txt`, `UI/interface.txt` and
`UI/interface1.txt`. The executable SHA-256 is
`B20510B20D34651D206C72340B96D2B7314D56B19072AC25678D37183889DE3D`.
The previous feed entries remain present. Client settings were excluded.

The server/portal archive was uploaded to
`/tmp/wyd-deploy-20261019-persistence-full`. All 7,389 payload hashes and
archive SHA-256 `7DAC09D03A204CF04F3631837F3D73780269010A0E9CAF629921ACBB92AC3091`
passed remote verification. Private server, staff, Caddy and unit configuration
were copied from the VPS into staging and compared before stopping game/API;
no established game connections were present. SSH briefly timed out before
authentication, then recovered without credential or security changes.

The established updater backed up PostgreSQL and activated
`/srv/wyd-go/releases/20261019T050509Z`, with rollback
`/srv/wyd-go/releases/20261018T234132Z`. Game/API restarted at 05:05:12 UTC
(02:05:12 America/Sao_Paulo). Readiness passed, PostgreSQL and Caddy were active,
and the running executable hashes matched the candidates:

- Game: `2AD463F629535B856CD4CF6887B5CA176D15E4D71F78899E03C2A51A6415971F`.
- API: `5ECF69231E409DD1351A160600CAE3770541C09BB228DA2708BC7248093BCA08`.

The authorized whole-VPS reboot completed. The boot ID changed from
`0b093ffb-7520-4810-a584-2580f7fd490f` to
`b1d60b33-191b-45f9-8a72-981d9176a1c8`. Game/API restarted at 05:05:45 UTC
(02:05:45 America/Sao_Paulo), PIDs 1319/1320, with the same verified executable
hashes and active release path. Game, API, Caddy, PostgreSQL 18, Docker,
Fail2ban and SSH are active; systemd lists no failed units. Game TCP 8281
listens, PostgreSQL remains on loopback 5433, and loopback readiness for both
APIs passes. The inspected post-boot game/API journal has no maintenance or
persistence failure. This is bounded startup evidence, not gameplay proof.

Public home, news, changelog, readiness and the client manifest returned 200;
the manifest identifies 10036 and the persistence release. The public HTTP
launcher feed preserves versions through 10036. Private environment, server,
staff, Caddy and all four WYD unit hashes match preflight exactly. The backup
is `/var/backups/wyd-go/wydgo-20261019T050509Z.dump` (139,848 bytes).

The persistence correction and incident evidence are recorded in
[the persistence plan](server/critical-persistence-plan.md#instance-expiry-and-pending-kill-settlement-2026-10-19-local-correction).
Working audit records remain under `build/persistence-release-20261019/`.
The established updater removed remote release staging; the post-boot check
also confirms that patch publication staging is absent. Current local client
incremental build outputs and remote rollback releases are retained.
Actual client login and gameplay remain pending; publication and service
readiness do not establish `CLIENT_TESTED`.

Local cleanup was blocked by execution policy and was not retried. Remaining
task artifacts (repository-relative unless absolute) are:

- `build/deploy/wyd-deploy-20261019-persistence-full/`.
- `build/patch/patch-10036.zip`.
- `build/persistence-release-20261019/public-patch-10036.zip`.
- `tmproject/build/tests/Release/crash-capture-tests-3716-730556234/`.
- `C:\Users\xereca\AppData\Local\Temp\wyd-release-20261019.ps1`.
- `C:\Users\xereca\AppData\Local\Temp\wyd-finalize-20261019.ps1`.

The final repository layout check still fails on existing client dump
documentation/logs and `tmproject/TMProject748/build/`. The broad diff check
also reports whitespace in `TMProject.rc`; unrelated work was preserved.
These local housekeeping findings do not change the verified VPS delivery.

## Previous release — 2026-10-18, Hell Gate and Rune Track

The owner explicitly authorized publication and restart, including the three
established game connections present immediately before shutdown. Release
`/srv/wyd-go/releases/20261018T234132Z` is active; rollback remains
`/srv/wyd-go/releases/20261018T171412Z`. Game/API started at 23:41:35 UTC
(20:41:35 America/Sao_Paulo). No host reboot, unrelated application restart,
local client installation, commit or push occurred.

- Scope: Rune Track rooms +0 through +6, thirty event-only NPC templates,
  327 room generators, independent full-room-pool rune draws, room objectives,
  Balrog/labyrinth portals, Freyja donation handling and imported caster bars;
  Hell Gate infantry, formations, mage combat and Dionys sales. These are the
  historical adaptations and explicit project choices documented in
  [Rune Track](server/rune-track.md) and the
  [Hell Gate follow-up](bug-register.md#infantry-formation-and-dionys-sale-follow-up--2026-10-18),
  not complete native-7.48 event parity.
- Source: dirty `main` at `b0e8cfa547ce107a71fd559fa319737820f1316f`.
  Linux/amd64 candidates were built through `Build-VpsRelease.ps1`, CGO off.
  The 878 Go/module, portal and deployment source inputs were hash-checked
  unchanged before activation. The previous complete Go tests/vet and final
  focused caster tests remain valid; production code was not changed for
  deployment. Fresh portal build, all 60 tests and Astro checks passed
  (zero errors, warnings or hints).
- Packaging used the established builder plus a 124-path supplemental-data
  allowlist, including the Rune Track file and thirty NPC definitions.
  All 7,352 previous payload paths survive; 50 files are new or different.
  All 7,385 extracted payload hashes passed before activation and match the
  complete independently read active tree afterward. Private account state,
  client settings and local crash dumps were excluded.
- Archive: 22,416,446 bytes, SHA-256
  `D5424E523422EAFABC16A8DAD17A0DA92A33A7E5C288C3480A4ADECF026E0A0A`.
  Game PID `2404633`, `/proc` executable SHA-256
  `AA2F935359ADC8DFCB487C8D8A13A4B67BAF9A7FF601928D878428CDEEBCAA87`;
  API PID `2404646`, `/proc` executable SHA-256
  `B217DEF1415F0E20E4ACB1807F1F673C71B5A297DA6FDBB4FE9DF2386FFF152A`.
  Both executable paths identify this release, not the prior checkout.
- The established updater backed up PostgreSQL to
  `/var/backups/wyd-go/wydgo-20261018T234132Z.dump` (138,104 bytes), atomically
  switched the release, restarted game/API and reloaded Caddy. Private
  environments, server/staff/Caddy configuration and all four WYD units match
  their pre-activation backups byte-for-byte. The existing game firewall rule
  was already present. Caddy validation passed with its existing nonblocking
  forwarded-header and HTTP-listener warnings.
- Loopback readiness on 8082/8083, public HTTPS readiness, home, news,
  changelog and manifest all returned 200; TCP 8281 listens. Warning-priority
  game/API journals since startup contain no entries. Startup loaded 691 NPC
  definitions. Remote staging was removed by the updater.
- The website receipt identifies `hell-gate-rune-track-759`; English News and
  changelog describe the server changes and outstanding gameplay acceptance.
  Launcher feed remains at 10035. A fresh public download of patch 10035 has
  SHA-256 `8172AFA041A234E8C01BD956DEB36591062E70EEE8A99B0A3FFCC62B24C77E2A`,
  unchanged. No new client patch or executable was published.

Delivery is `AUTOMATED TESTED` and deployment-verified, not `CLIENT_TESTED` or
global release certification. Actual room traversal, party/observer behavior,
Freyja's prompt, portal visuals, combat, sales and rewards remain pending in
the exact installed client against this release. Existing unrelated repository
layout violations remain unchanged: the stack-overflow audit and crash-monitor
log under `tmproject/client748/dumps/`, and `tmproject/TMProject748/build/`.
`git diff --check` passed. Input manifests and delivery logs are retained in
`build/rune-hg-release/` as the working-tree release audit record.

Cleanup limitation: execution policy rejected deletion of the disposable local
staging directory
`C:\Users\xereca\Documents\PROJETOS\wyd-go\build\deploy\wyd-deploy-20261018-rune-hg-final`.
It remains in the sanctioned ignored build root. No alternative cleanup
mechanism or retry was attempted. This does not affect the verified active
release; remote staging is absent.

## Previous release — 2026-10-18, historical Hell Gate version 3

The owner authorized publication and restart. Server release
`/srv/wyd-go/releases/20261018T171412Z` is active; rollback is
`/srv/wyd-go/releases/20261018T152826Z`. Activation occurred at
17:14:15 UTC (14:14:15 America/Sao_Paulo), with no established game connections.
No host reboot, app-td restart, client installation, commit or push occurred.

- Scope: the historical Hell Gate encounter described below, including the
  selected entrance and two-Lich puzzle, grouped populations, autonomous march
  toward Dionys, individual replacements, Dionys lottery and KR Sand Lich face.
  Start a new encounter to exercise version 3: restored version-2 snapshots
  intentionally keep their original rules instead of being reset or migrated.
- Source: dirty `main` at `de87cd8789bef3165babbdf41fe1f86bc449106a`, compiled
  using Go 1.26.5 for Linux/amd64, CGO disabled. Full Go tests and vet from the
  preceding implementation were reused; source provenance covers 762 Go/module
  inputs in `build/hell-gate-historical/deploy-source-inputs.sha256`.
- Packaging used `Build-VpsRelease.ps1` plus an explicit 93-entry supplemental
  runtime-data allowlist. Every previous payload path is preserved. Exactly ten
  files differ: the game executable, instance configuration, Dionys/Lich
  definitions and six new skeleton definitions. The deployed API binary,
  public website and staff-panel payload remain byte-for-byte unchanged.
- Archive: 22,327,763 bytes, SHA-256
  `37AAD514CC00E2806D904D46894F3672D4ABA25FEF1E2D9F99FA59087792F921`.
  All 7,352 extracted payload checksums passed before activation and again in
  the active release. Private account state and configuration are not included.
- Game PID `2343020`, executable SHA-256
  `9A0DCB56234BF2CA8F89BD63CF2794E01FB76257968447A96470D2AA88E499F6`;
  API PID `2343032`, executable SHA-256
  `4722A4B376642AE6DBE9B0AE753C07EEECF0CBC93F20965670F972555F0C054C`.
  Both started at 17:14:15 UTC and their `/proc` paths identify the new release.
  Both readiness endpoints and public HTTPS readiness passed; TCP 8281 listens.
  Warning-priority game/API journals since activation contain no entries.
- The established updater backed up the database to
  `/var/backups/wyd-go/wydgo-20261018T171412Z.dump` (138,097 bytes), restarted
  game/API and reloaded Caddy. Configuration validation passed with existing
  nonblocking forwarded-header/HTTP-listener warnings. Game/player environment,
  server settings, staff configuration and Caddy configuration hashes are
  unchanged. Remote staging is absent.
- Launcher feed remains at 10035: this server-only change requires no new
  client patch. Existing cumulative Sand Lich assets remain required. No site
  news or client executable was changed by this publication.

Publication and restart are verified, not `CLIENT_TESTED` or global release
certification. In-game encounter, lottery and Lich rendering acceptance remain
pending. Deployment logs, supplemental paths and payload hashes are recorded in
`build/hell-gate-historical/`; durable delivery facts are recorded above.

Execution policy rejected local staging cleanup before execution. No alternate
method or retry was attempted. The exact remaining directory is
`build/deploy/wyd-deploy-20261018-hellgate759/`. Remote staging was removed by
the successful updater; the active release and rollback remain preserved.

## Previous release — 2026-10-18, client stability safeguards, patch 10035

The owner authorized publication and restart, including disconnection of the
one established game connection observed immediately before activation.
Patch 10035 and `/srv/wyd-go/releases/20261018T152826Z` are active. Rollback is
`/srv/wyd-go/releases/20261017T221343Z`. No host reboot, app-td restart, local
client installation, commit or push was performed.

- Source: dirty `main` at `de87cd8789bef3165babbdf41fe1f86bc449106a`.
  This is a working-tree release, not a release of the committed HEAD alone.
  Client safeguards cover bounded packet/text/minimap handling, UI draw-list
  unwinding, serialized socket dispatch, actor retirement and skin-mesh loading.
  These safeguards do not establish the root cause or closure of every crash.
- Release/x86 client and automated gates passed: 3,281,408 architecture checks,
  281 socket checks and 198 crash-capture checks, including the 1,596-mesh corpus.
  Existing Winsock deprecation warnings are nonblocking. Fresh complete
  `go test -count=1 ./...`, `go vet ./...`, Linux cross-build, portal build,
  all 60 portal tests and Astro checks passed. Conditional database fixtures
  were not enabled. The initial portal test failure was the obsolete 10034
  receipt expectation; the updated 10035 receipt test passed.
- Public patch: `patch-10035.zip`, 846,770 bytes, SHA-256
  `8172AFA041A234E8C01BD956DEB36591062E70EEE8A99B0A3FFCC62B24C77E2A`.
  Its sole entry is `WYD.exe`, SHA-256
  `251CF754409F8AE4C0251C9FDD88EDA14C6799E78A377BD0665B6E858FDAFAB0`.
  The matching Release PDB is retained with the incremental build; SHA-256
  `D29C437505BE2CC34A35B474CBDA38032270DBE30F1DA12A4EA69A1D9255252F`.
  Interface resources match published patch 10033; earlier cumulative assets
  remain in the feed. HTTP/HTTPS feed checks and a fresh public ZIP download
  passed. Website manifest, home, download, News/article and changelog match.
- Server archive: 22,299,735 bytes, SHA-256
  `533E6A624234B8CB153D48C69F34109533DFFD47CE7C1D1A462AE2A2BA570D9D`.
  The explicit 87-entry supplemental runtime allowlist was verified against
  the active release inventory. All 685 runtime-data paths were preserved.
  Archive paths exclude private account/settings files; all 7,346 deployed
  payload checksums passed after activation.
- Game PID `2325444` started at `2026-10-18 15:28:29 UTC`; actual executable
  SHA-256 `5404ED74151256C4E803C06D7D2E532884E07D39ACECEE2F9F16B7C3560959E0`.
  API PID `2325457` started at `15:28:30 UTC`; actual executable SHA-256
  `4722A4B376642AE6DBE9B0AE753C07EEECF0CBC93F20965670F972555F0C054C`.
  Both `/proc` paths identify the new release. Both readiness endpoints are
  ready, TCP 8281 listens, and warning-priority journals are empty. The text
  error scan found only the normal critical-persistence worker startup line.
- Backup succeeded: `/var/backups/wyd-go/wydgo-20261018T152825Z.dump`,
  135,743 bytes. Private game/player environment and server configuration
  hashes are unchanged. Caddy validation passed with existing nonblocking
  forwarded-header/HTTP-listener warnings. Remote release/patch staging is absent.

The installed local `tmproject/client748/WYD.exe` remains unchanged at SHA-256
`4183973219F67EC0F22D29276E42541DD9793FA1EB0A54BADFF7919938F60C41`.
Players must close the client and run `WYDLauncher.exe` to receive 10035.
Actual in-game crash reproduction and resource/lifecycle gaps remain pending;
this publication is not `CLIENT_TESTED` or global language/release certification.

Execution policy rejected local cleanup before execution; no alternative method
or retry was attempted. New remaining outputs are
`build/deploy/wyd-deploy-20261018-crash-hardening-10035/`,
`build/patch/patch-10035.zip`,
`build/release-crash-hardening-20261018/public-10035.zip` and
`tmproject/build/tests/Release/crash-capture-tests-27764-681877187/`.
Logs, the supplemental runtime list and payload hashes remain in
`build/release-crash-hardening-20261018/`. Its `prior-release/` EXE/PDB pair
is deliberately retained for diagnosis of the previously tested executable.
Previously blocked cleanup outputs below were not retried.

## Implemented rules — 2026-10-18, historical Hell Gate

The supplied historical 7.59 server decompilation establishes two distinct
random Liches and a single usable entrance selected by the corridor puzzle.
New Hell Gate runs now use encounter version 3 with those rules. Versioned
snapshots retain pair/progress; existing version-2 runs retain their old rules
when restored. Invalid progress and incompatible configuration downgrades fail
closed instead of rerolling the puzzle.

The owner's follow-up extends version 3 with the bundled 7.59 populations:
six corridor escorts, 144 room guards (nine four-member groups per room),
three mage escorts per Lich, and 252 defense attackers. Defense mobs remain
active without a nearby player and follow a terrain-aware route toward Dionys.
Casualties replenish individually from their original generator anchors until
the absolute deadline, without healing Dionys or resetting surviving attackers.

Dionys opens the existing random-item shop through head 51 and the native
0x2BD packet. Historical catalogs, weights and option generation apply only to
instance Dionys; the public merchant is unchanged. Purchases remain restricted
to living, active, same-instance protectors and critical committed persistence.
The Lich template now uses the existing imported KR Sand Lich head 4088,
selecting skeleton type 12 (`mesh/rc01`), not the Dragon Lich head. All 15 local
`rc01` resources match the supplied KR files; no asset was copied or replaced.

Classification: `EXTENSAO_COORDENADA`, adapting historical 7.59 behavior over
the unchanged 7.48 wire contract, not complete native-7.48 parity. Existing
owner-selected timing, HP, reward, prices and party admission remain unchanged.
Historical spawn dispersal and replacement timing are bounded project
adaptations, and skeleton combat balance remains the existing project's.
The supplied 6.13 map alone does not reconstruct its implementation. Evidence,
differences and remaining acceptance are recorded in the
[Hell Gate bug record](bug-register.md#historical-correction--2026-10-18-version-3)
and [store contract](protocol/hell-gate-store.md).

Full Go tests and vet passed; focused tests cover pair selection, portal
authorization/reachability, duplicate deaths, restoration/upgrade safety,
all 252 attack origins, a complete march to damaging Dionys, occupied-route
waiting, individual replacements, shop selection and purchase persistence.
Evidence logs are in `build/hell-gate-historical/`. No new client artifact was
needed or built. This work is now published in server release
`20261018T171412Z` as recorded above; it has not been committed, pushed or tested
in game. Launcher patch 10035 remains unchanged.

## Previous release — 2026-10-17, Hell Gate route and defense, patch 10034

The owner authorized publication, player disconnection and game-service restart.
Three established game connections were present at the preflight check.
Patch 10034 and release `/srv/wyd-go/releases/20261017T221343Z` were activated.
Rollback remains `/srv/wyd-go/releases/20261017T164649Z`. The host was not
rebooted; no local client installation, launch, commit or push was performed.

- Source: dirty `main` at `de87cd8789bef3165babbdf41fe1f86bc449106a`.
  The canonical validated client candidate was reused. Patch 10034 contains
  only `WYD.exe`; both interface resources match patch 10033, and cumulative
  earlier assets remain available.
- Public ZIP: 829,693 bytes; SHA-256
  `03D044C9C65A56F9A3005DF0836859EC5EEB1043AD5CD340FBFE8D830FE9A6F3`.
  A fresh advertised HTTPS download and its sole executable entry match.
  HTTP/HTTPS feeds return 200 and preserve prior entries with only 10034
  appended. Website manifest, home, News, article and changelog advertise 10034.
- Server archive: 22,299,469 bytes; SHA-256
  `DD463B6AEF9244289A9DFB308285218A170B5976E8294513A3F3ADD15EF2F9A1`.
  The tracked-data packager was supplemented with an explicit 87-entry runtime
  allowlist: previously deployed exclusive templates plus five Hell Gate
  definitions and its generator. Every previous payload entry is preserved;
  24 files are new or changed. Archive paths exclude private settings/accounts.
  All 7,344 extracted checksums passed before activation and again in the
  active release afterward.
- Game PID `2163007` started at `2026-10-17 22:13:46 UTC`; actual executable
  SHA-256 `839D3D02D749C9063A48E25102126A287EFFAEF59EF04108D3A756C4E5CB90ED`.
  Player API PID `2163020` started one second later; actual executable SHA-256
  `4722A4B376642AE6DBE9B0AE753C07EEECF0CBC93F20965670F972555F0C054C`.
  Both `/proc` paths identify the new release. Both readiness endpoints respond
  ready, TCP 8281 listens, and warning-priority game/API journals since
  activation contain no entries.
- Existing configuration matched before activation; private game/player
  environment and server configuration hashes remain unchanged. The established
  updater made backup `/var/backups/wyd-go/wydgo-20261017T221343Z.dump`
  (128,888 bytes), validated Caddy and atomically activated the release.
  Existing redundant forwarded-header and HTTP-only-listener Caddy warnings
  are nonblocking. Remote release and patch staging paths are absent.
- Fresh complete `go test -count=1 ./...` and `go vet ./...` passed; this does
  not certify unavailable conditional database/account fixtures. Portal build,
  all 60 tests and Astro checks passed with zero errors, warnings or hints.

Publication and restart are verified. Actual portal confirmation, full encounter
progression and solo balance remain pending; this is neither `CLIENT_TESTED`
nor global release-readiness/language-migration certification.
Local publication cleanup was rejected by execution policy before deletion;
no alternate shell/API or retry was attempted. Exact new leftovers are
`build/deploy/wyd-deploy-20261017-hellgate-10034/`,
`build/patch/patch-10034.zip` and `build/hell-gate/public-10034.zip`.
Task logs also remain in `build/hell-gate/`: `release-go-tests.log`,
`release-go-vet.log`, `release-portal-build.log`, `release-portal-tests.log`,
`release-portal-check.log`, `release-package.log`,
`release-patch-publish.log` and `release-activation.log`.
These ignored local outputs are not operational state; remote staging is absent.
Previously blocked cleanup paths below were not retried.

BUG-003 now has a version-2 encounter reconstruction; details and remaining
gameplay checks are in the [bug register](bug-register.md#bug-003--hell-gate-population-visibility).
The entry and arena are disconnected terrain components. Three server-owned
portal routes and the native generic client prompt address that blockage.
Progression now includes corridor bosses, four rooms/Liches and defending
Dionys until the deadline, rather than ending on a cleared wave.

The owner chose twenty minutes total (two preparation plus eighteen active)
and 10,000,000 completion EXP. The subsequent owner-requested HP increase sets
the entry spider, corridor bosses and Liches to 400,000 HP each; friendly
Dionys remains at 150,000 HP. At 10,000 magical damage per cast, not DPS,
each boss takes forty casts. The owner clarified that spells rotate every
900 milliseconds: the estimate is 35.1 seconds with an immediate first cast,
not 234 seconds repeating a single six-second-cooldown skill. This is a
planning estimate, not actual FelipeTr gameplay evidence or a combat-cadence
change. The requested timer and HP are preserved. These values and the
reconstructed population are project settings, not proven native 7.48 constants.

The native prompt boundary is documented in the
[Hell Gate portal record](../.agents/research/client748/flows/transport/hell-gate-portals.md).
Client compilation through `tmproject/Build-Client.ps1 -NoDeploy` passed.
Candidate: `tmproject/build/TMProject748/Release/WYD.exe`; SHA-256
`4183973219F67EC0F22D29276E42541DD9793FA1EB0A54BADFF7919938F60C41`.
No local runtime installation or client launch was made. The candidate was
subsequently published as patch 10034 with server activation recorded above.
Automated server/data validation is recorded in BUG-003; visual acceptance and
real solo balance remain pending.
The earlier `go test ./internal/game ./internal/data -count=1` and scoped `go vet`
passed. After the 400,000 HP update, focused `TestHellGate*` cases passed in
both packages (game 0.622s, data 0.163s). The route review additionally exercises
all three portals through the real request handler with installed terrain and
populated room/defense phases, preserving membership and free arrival tiles.
The documentation inventory was refreshed earlier. Layout validation remains
blocked solely by the obsolete, task-owned direct-compile output at
`tmproject/TMProject748/build/` (177 files). Its recursive deletion was rejected
by execution policy; no alternate cleanup route was attempted. Canonical
incremental outputs under `tmproject/build/` are preserved. This cleanup limit
does not change the source/test/build result and is not a gameplay claim.

## Active release — 2026-10-17, instance parties and reconnect recovery

The owner explicitly authorized publishing all pending updates, disconnecting
players and restarting game services. Four established game connections were
present before the authorized stop. The host was not rebooted; no credentials,
firewall policy, local client runtime, Git index, commit or remote branch changed.

Source: dirty `main`, HEAD `de87cd8789bef3165babbdf41fe1f86bc449106a`.
This activates the implemented portions of BUG-003, BUG-007, BUG-008, BUG-009
and BUG-011 in the [bug register](bug-register.md). Group Nightmare charges
only the leader and transports eligible members together; individual tickets
remain individual. Reconnection restores the current encounter HUD and rejoins
the original process-local party when it still exists and has room. It does
not add social-party persistence across a server restart. Magic Chamber party
relocation and independent-run isolation, dedicated Hell Gate generators, and
friendly Compounder/city-guard behavior are included. Remaining Hell Gate event
rules, other open reports and in-game acceptance checks are not closed.

Fresh `go test -count=1 ./...` and `go vet ./...` passed. This does not certify
conditional database/account fixtures unavailable to the local run. Portal build,
all 60 tests and Astro checks passed with zero errors, warnings or hints.
The client candidate and both UI resources match published patch 10033 exactly;
no redundant patch, client rebuild, local installation or launch was performed.
The public feed and website receipt remain at 10033. Server news and changelog
now describe this follow-up without claiming full Hell Gate parity.

`Build-VpsRelease.ps1` produced the Linux/amd64 binaries and tracked runtime
payload. An explicit allowlist restored 85 untracked runtime entries: the
previous 81 dedicated NPC templates, three Hell Gate templates and its generator.
Archive comparison preserved every previous payload entry; 18 files are new or
changed, including binaries, instance data, friendly NPC definitions and website
news. Archive paths and all 7,340 extracted payload checksums passed before the
authorized downtime. A redundant local nested website staging copy was excluded
from both the archive and its manifest; it is not part of the deployed payload.

- Archive: 22,259,567 bytes; SHA-256
  `D26D70A035BA5E7D82CA496246F6580263C4030AF9C8DDE8CFC625F6406425EF`.
- Game SHA-256:
  `4B36369936F45DCC4BCFFEC51BDF0851C6855D7C72617EC78FBE0FEEDFE616E4`.
- Player API SHA-256:
  `0A0E3AC870A96CB70FC17430F4AA04D7E362C266DDB3FE14C23CB9B18EBCB2E0`.
- Active release: `/srv/wyd-go/releases/20261017T164649Z`.
  Rollback: `/srv/wyd-go/releases/20261017T145456Z`.
- Game PID `2109897` started at `2026-10-17 16:46:51 UTC`; player API PID
  `2109910` started one second later (`13:46:51`/`13:46:52` America/Sao_Paulo).
  Actual `/proc/<pid>/exe` paths and hashes match the release. All 7,340 active
  payload files match the manifest; both readiness endpoints report `ready`,
  TCP 8281 listens, and warning-priority journals since startup have no entries.
- Established deployment configuration matched before activation; private game
  and player environment hashes are unchanged afterward. The established updater
  completed backup `/var/backups/wyd-go/wydgo-20261017T164649Z.dump`
  (121,682 bytes), atomic activation and Caddy validation. Existing redundant
  forwarded-header and HTTP-only-listener warnings remain nonblocking.
- Public HTTP/HTTPS feeds, client manifest and the new news article return 200.
  The manifest retains patch 10033's original size and hashes.
- Remote `/tmp/wyd-deploy-20261017-party-recovery/` was removed by the updater.
  Local recursive staging cleanup was rejected by execution policy; no alternate
  deletion was attempted. Exact leftovers are
  `build/deploy/wyd-deploy-20261017-party-recovery/`,
  `build/nightmare-release-tests.log`, `build/party-recovery-portal-build.log`
  and `build/party-recovery-portal-check.log`. They are not operational state.

Publication and restart are verified; actual gameplay remains pending. This is
not a global release-readiness, language-migration or `CLIENT_TESTED` claim.

## Active release — 2026-10-17, Magic Chamber room bounds

Classification: `MODERNIZACAO_COMPATIVEL`. No packet, client ABI or native
actor-lifecycle contract changed; existing 7.48 CreateMob/RemoveMob evidence
is reused. The server/data correction is active, but gameplay validation is pending.

The repeated Carta N/M/A report had two reproducible server causes. Room bounds
were centered on the entrance rather than the encounter: for example, room 3
ended at x=867 while generators placed living monsters beyond that boundary.
Combat and visibility rejected a member crossing it. Incremental movement
visibility only examined view-edge strips, leaving actors in the overlapping
view region stale when logical room eligibility changed.

All twelve configured rooms now have explicit `areaX`/`areaY` centers, separate
from unchanged entrance coordinates. Existing templates without these fields
retain their old centers. Membership, combat, movement containment, recovery,
occupancy and generator reservation use the same room geometry. Explicit bounds
also constrain ordinary, detached-admission and completion-wave spawn placement.
Crossing a logical room edge reconciles the full visibility set; ordinary movement retains the
incremental fast path. Population, timers, EXP and loot rules are unchanged.

The regressions failed before the correction: all twelve real rooms could spawn
outside their combat bounds, and a one-tile boundary crossing sent no removals.
They now pass, covering every real spawn/boss tile, member targetability,
non-member isolation, distinct room entrances, clear/transition/final completion,
and repeated leave/reentry without mob movement or population changes. A crowded
detached first wave also stays inside explicit bounds without publishing actors.
`go test ./internal/game ./internal/data ./internal/model -count=1` passed
after the final regression and detached-admission changes.
`go vet ./internal/game ./internal/data ./internal/model` also passed.
The owner authorized upload, then explicitly approved game-service restart and
player disconnection through the follow-up question.
`Build-VpsRelease.ps1` built the Linux/amd64 game/API from dirty `main` at
`de87cd8789bef3165babbdf41fe1f86bc449106a`. The previous release's explicit
81-NPC allowlist was restored with unchanged hashes. Comparison against its
7,334-entry manifest found changes only in `game/wyd-server`,
`game/wyd-player-api` and `game/data/instances.json`; portal and other data
remain byte-identical. Archive paths were checked before upload.

Uploaded staging: `/tmp/wyd-deploy-20261017-carta-bounds/`. Archive:
22,245,477 bytes, SHA-256
`2BC7C31EAED6ABBDD549B3066F53BDEF5F29F197652798A0A97CBA946F56A3AE`.
Game SHA-256:
`48BD0BF791EF9E23FA6047F239F5F75559B8AE70DC7B7FB8C558A480F3A5CB9C`.
Player API SHA-256:
`713D6B9E18DD1E69A0D4962E30081D25052D9B09B74A24A07D9D21922214835D`.
Instances SHA-256:
`EB59C4DBFC071253AC57B3BA5090061DC3A5FB927D1A54E769E2E540DE29AA67`.
Remote archive verification and all 7,334 extracted payload checksums passed.
The temporary remote verification extraction was removed. Activation then used
the established `update-release.sh`; its remote staging was removed successfully.

The active release is `/srv/wyd-go/releases/20261017T145456Z`; rollback remains
`/srv/wyd-go/releases/20261017T041938Z`. Game/API PIDs `2091183`/`2091196`
started at `2026-10-17 14:54:59 UTC` (`11:54:59` America/Sao_Paulo).
Both actual running executable paths/hashes match this release and the values
above; all 7,334 active payload checksums match. Both readiness endpoints returned
`ready`, TCP 8281 is listening, and warning-priority game/API journals since
activation have no entries. Deployment configurations were byte-identical before
activation, and game/player environment hashes are unchanged afterward.
Database backup `/var/backups/wyd-go/wydgo-20261017T145456Z.dump`
(120,238 bytes) completed. Caddy validation passed with the existing nonblocking
forwarded-header and HTTP-listener warnings. The host was not rebooted.
No client patch/build/install, commit or push occurred. An in-game replay is
still required; this is not a global release-readiness or CLIENT-TESTED claim.
Local cleanup of `build/deploy/wyd-deploy-20261017-carta-bounds/` was rejected
by execution policy before deletion. This staging directory remains; no alternate
deletion mechanism was attempted. Remote staging was removed successfully.

### Follow-up report queue

The active reports, initial diagnostic findings and acceptance checks now live
in the [gameplay bug register](bug-register.md), issues BUG-001 through BUG-010.
It includes the original combat/gate/Foema queue, delayed party identification
on Nightmare M entry, party and encounter HUD loss after reconnection, recurring
Magic Chamber monster visibility failures, and missing room-transition ground
markings. The exact native identity of the reported markings remains unverified.

The published Carta bounds correction above is a verified partial correction,
not closure of the recurring gameplay report. The latest report's running
artifacts have not yet been verified. Registration and initial diagnosis add no
source fix, client build, balance change or deployment.

## Published release — 2026-10-17, patch 10033

The owner authorized publication and game/player API restart, including
disconnection of online players. No host reboot, credential replacement,
firewall-policy change, local client installation, commit or push is requested.
This release pairs the double-hit, manual physical autoattack, number-row
skill selection, Helper scrollbar/chat scroll and private balance/medal
commands documented below. Other open gameplay and Helper gaps stay open.

Source: dirty `main` at `de87cd8789bef3165babbdf41fe1f86bc449106a`.
The unchanged, previously compiled client candidate is
`tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`D6CC42D093FE240127F5096FA133965C225AB410D133AC79B8A6094AD8A7FA8D`.
Patch 10033 has an explicit three-entry allowlist: `WYD.exe`,
`UI/interface.txt` and `UI/interface1.txt`. Personal settings, account data
and launcher binaries are excluded. ZIP: 830,698 bytes, SHA-256
`FC8CB13743F3F1A94D76497C6D1A9B625D643B54C983EE964F1E3878766FB777`.
UI hashes, respectively:
`407857BFE8CD47BBA5CA2F1379CCF84AC38E0865A26F38423D6A74DFC5FE9B93`
and `EC260EDFAED70727771A3C9443D3D063B6BA24DA1846110297FC98D727007468`.

`Build-VpsRelease.ps1` cross-built the Linux/amd64 game and player API.
The archive includes the explicit 81-template exclusive-instance allowlist
omitted by the script's tracked-only data inventory, plus the rebuilt portal.
All 7,334 regular payload files have checksums. Archive: 22,244,355 bytes,
SHA-256 `5AD94C424F1EFDD0ED335C99C7DABB517D3718F65A37038F779561346CC2EC69`.
Game SHA-256:
`67F379CC21F99CD196613C849569342CC6B3D71DF34EAD17968989A4492882F5`.
Player API SHA-256:
`2D56FC94E3C882FE7ECB5D38327C9163106007D9F1587E6C2410E981C278F722`.

Fresh `go test -json -count=1 ./...` and `go vet ./...` passed.
Four tests remain explicitly skipped: dedicated auth/player database
integration, web-admin PostgreSQL integration and optional local account
fixtures. This does not certify those skipped paths. The treasury world-loop
test now fixes its clock to an allowed Sunday and re-arms the unrelated
player's regeneration deadline; Saturday admission rules in production are
unchanged. The previously reported weekday-dependent failure is resolved,
not excluded. Portal build, all 60 tests and Astro checks passed (zero
errors, warnings or hints). Layout/link validation passed (206 documents).
Existing unchanged client build evidence below is reused; gameplay remains
pending, not `CLIENT_TESTED`.

Preflight SSH recovered the established account/key and verified live patch
10032 and release `/srv/wyd-go/releases/20261016T232841Z` as the rollback
reference. Server/staff/service/proxy configurations match local deployment
inputs. Patch 10033 is published and release
`/srv/wyd-go/releases/20261017T041938Z` is active. Game PID `1990466` and
player API PID `1990481` both started at `2026-10-17 04:19:41 UTC`
(`01:19:41` America/Sao_Paulo). The host was not rebooted. Actual running
executables resolve to this release and match the binary hashes above;
all 7,334 active payload checksums also match. Both readiness endpoints
returned `ready`, and the public game listener is open on TCP 8281.

The publisher preserved the previous feed byte-for-byte and appended only
`10033 patch-10033.zip`; patch 10032 is unchanged. Public HTTP and HTTPS
feeds returned 200. A fresh HTTPS ZIP download and all three embedded file
hashes match the candidate. The public site manifest matches version, size
and ZIP hash; home, News, the new article and changelog returned 200 and
advertise 10033. No local runtime executable was replaced or launched.

An SSH connection timed out after publication while the game services were
stopped. A bounded TCP probe subsequently confirmed port 22 was reachable;
the established account/key then activated the release successfully without
credential or firewall changes. Server, staff, authentication environments,
service units and proxy configuration hashes remain unchanged. The first
running-process audit needed `sudo` for `/proc/<pid>/exe`; the corrected
read-only check passed without repeating the payload hash audit.

Database backup `/var/backups/wyd-go/wydgo-20261017T041938Z.dump`
(106,196 bytes) completed before activation. The previous release remains
available as the rollback target. Game/API warning-priority journals since
the restart contain no entries. Caddy validation passed with existing,
nonblocking redundant-forwarded-header and HTTP-only-listener warnings.
Remote release and patch staging were removed. Local cleanup was rejected
before execution by the execution policy; no alternate deletion method was
attempted. Exact leftovers are `build/deploy/wyd-deploy-20261017-combat-10033/`,
`build/release-10033/` and `build/patch/patch-10033.zip`. They contain staging,
verification output and diagnostics, not active runtime data. Earlier
cleanup-blocked artifacts were not touched. Publication and activation
verification succeeded independently of this housekeeping limitation.

This is scoped publication evidence, not global language-migration or native
parity certification. Other open issues and actual gameplay validation remain
pending; no new `CLIENT_TESTED` claim is made.

## Local follow-up — 2026-10-17, private balance commands

`MODERNIZACAO_COMPATIVEL`: `/saldo` and `/balance` query balances;
`/medals` and `/medalhas` separately query Game Room medal counters.
All four case-insensitive aliases use the existing chat/whisper dispatcher,
but the response is exactly one private `0x101` top notification, not chat.
There are no pages, delayed sends or additional commands to read a report.
Command names accept both languages; displayed text stays English. Only the
requesting, in-world character may query its own values. Aliases share their
canonical command's rate limit. No new packet or client build is required.

The balance report contains account Donate and character Fame, Honor,
event passes (`Pass`) and Kefra entries (`Kefra`). Per owner direction,
gold, CP and medals are excluded. The medal report always shows Gold, Red
and Blue counters, including zero. These counters currently have grant rules
but no implemented spending flow. Honor has no configured active wallet:
show its legacy counter if present, otherwise `Honor:N/A`. Legacy character
`donate` is not account Donate and is excluded. Other persisted SpecialCoins
keys can contain access-expiry timestamps rather than currencies and are
not exposed as money. Pending premium credits are not available money.
Neither command mutates or reloads live economic state.

Maximum-width `uint32` values produce 83 ASCII bytes for balances and 55 for
medals, below the existing 95-byte notice text limit; values are never clipped.
Focused tests passed for all four aliases through both transport paths,
private single-notice delivery, unchanged economic state, zero/unconfigured
counters, medal/CP/gold separation, invalid arguments (including old page
numbers), inactive sessions, shared alias rate limits, reserved names and
maximum-width encrypted packet round trips.
Repository layout/link validation (206 documents) and `git diff --check` passed.
The earlier broader `go test ./internal/game` gate failed at
`TestCriticalCityTreasuryActualWorldLoopProgress`: `critical store call did not
start` (two-second timeout). That guild-treasury path was not changed here;
its weekday-dependent cause was subsequently fixed in the test fixture
and the complete server gate now passes; see the release receipt above.
Earlier diagnostics are retained in
`build/balance-command/game-tests.jsonl`. That older failed run is not passing
evidence; the fresh complete gate in the publication receipt above passed.
This section records the original source-only phase. The commands are now
published in patch 10033's paired server release; gameplay validation remains
pending. No local installation, commit or push occurred.

## Local follow-up — 2026-10-17, number-row skill controls

The owner chose 1-0 instead of F1-F10 for skills. The temporary native F-key
branch has been removed; number-row selection and Shift+1-0 hovered
assignment retain their existing character routing, focus/mode/alive-state
guards and numpad suppression. Both belts and both damage labels share one
selection implementation. F11 and current C.C./consumable bindings remain
unchanged. Native PageUp/PageDown visible-list scrolling remains corrected.
This control choice is not a claim of native F-key parity. No new packet,
resource ID or server behavior. Helper selection text names the number row;
old keyboard artwork and `UI/etc.txt` still need a separate content correction.

Details: [Helper audit](client/ui-windows.md#helper-shortcut-audit--2026-10-17)
and [native evidence](../.agents/research/client748/flows/ui/field-shortcut-selection-and-chat-scroll.md).
Alternate-mode accelerator registration, Helper text conflicts, Ctrl loot
labels and command-page end-to-end verification remain separate open gaps.
`Build-Client.ps1 -NoDeploy` passed: architecture 1,240,477 checks, socket 231,
crash capture 83, launcher 56 and Release/x86 compilation. Candidate
`tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
`D6CC42D093FE240127F5096FA133965C225AB410D133AC79B8A6094AD8A7FA8D`.
It includes the previous double-hit, manual autoattack and scrollbar changes.
Installed `tmproject/client748/WYD.exe` remains
`1FE4CBA89C18847C3E4C35CC9497B049B9E01DCAB12E58B36152AE7FBFE19F6A`.
No installation, launch, publication, restart, commit or push. Actual gameplay
remains pending; source-contract checks are not `CLIENT_TESTED` evidence.
The flow record validator and repository layout/link checks passed (206
documents); `git diff --check` passed. Cleanup was rejected before execution;
no alternate deletion was attempted. Task leftovers are
`build/hotkey-native-flow.tsv`, `build/hotkey-native-entry.tsv`, their
`*-ghidra.log` files, `build/hotkey-client-build.log`, and
`tmproject/build/tests/Release/crash-capture-tests-26200-552293093/`.
The number-row build also produced
`tmproject/build/tests/Release/crash-capture-tests-19624-553526015/`;
its exact-target cleanup was likewise rejected before execution. No retry
or alternate deletion mechanism was used. The candidate is the current
incremental build output and is intentionally retained.

## Local follow-up — 2026-10-17, Helper scrollbar

The queued Helper placement correction is implemented and compiled. The down
arrow no longer applies UI scale twice during construction; resizing uses the
same native resource offset in physical pixels. Classic and horizontal bars
are unchanged. Details: [UI2 scrollbar](client/port-748.md#ui2-vertical-scrollbar-2026-10-13).
The [Helper shortcut audit](client/ui-windows.md#helper-shortcut-audit--2026-10-17)
identified unbound F1–F10, missing alternate-mode accelerators, the apparent
PageUp/PageDown visibility inversion, and help text conflicts with A/D/F.
F1–F10 and scrolling are addressed by the follow-up above; the other gaps
remain open. Existing C.C./consumable bindings were
not overwritten. Ctrl loot-label behavior and command-page end-to-end routes
remain unverified. No claim is made that every advertised shortcut works.

`Build-Client.ps1 -NoDeploy` passed: architecture 1,240,466 checks, socket 231,
crash capture 83, launcher 56 and Release/x86 compilation. Candidate
`tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
`1A144D0017F001EBE68B814694E796F45AD3ECD8270445A4C8BB31E004FFDA8A`.
It includes the double-hit and manual autoattack source changes below.
The installed runtime was not changed. No launch, deployment, restart,
commit or push occurred; gameplay validation remains pending.
The new crash-test evidence directory is
`tmproject/build/tests/Release/crash-capture-tests-23696-545265781/`.
Previous cleanup was policy-blocked; no alternate deletion was attempted.

## Local follow-up — 2026-10-17, manual physical autoattack

The double-hit follow-up is complete at source/build level. The next queued
task, manual physical continuation, now retains a clicked enemy and retries
through existing click validation without requiring C.C. The client/server
cooldowns and damage formula remain unchanged. Death, removal, transition,
manual cancellation and C.C. handover are covered by the local policy and
source-contract checks. Details: [combat control](client/combat-control.md).

`Build-Client.ps1 -NoDeploy` passed: architecture 1,240,451 checks, socket 231,
crash capture 83, launcher 56 and Release/x86 compilation. Candidate
`tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
`EE8C2DC67CF4BCAE54DC5953871A394CA968C1C218B04399DBC5AA22997F8055`.
Installed runtime remains unchanged at
`1FE4CBA89C18847C3E4C35CC9497B049B9E01DCAB12E58B36152AE7FBFE19F6A`.
No install, launch, deployment, restart, commit or push occurred.
Gameplay validation remains pending; this is not `CLIENT_TESTED`.
Next queued batch: native hotkey audit and Helper scrollbar placement.
Cleanup was rejected before execution; no alternate deletion was attempted.
Generated crash-test evidence remains under `tmproject/build/tests/Release/`:
`crash-capture-tests-18636-544892015/` and
`crash-capture-tests-12948-544945843/`.

## Local follow-up — 2026-10-17 00:40 UTC, double-hit speed and threshold

`MODERNIZACAO_COMPATIVEL`: this completes the owner's double-hit follow-up
before the queued physical autoattack and Helper shortcut/scrollbar work.
Those queued tasks have not been implemented or validated in this batch.

- Both damage numbers now share the first number's cached movement speed,
  lifetime, visibility curve and initial Y position, with the existing 120 ms
  delay between them. Matching lifetime alone had left the constructor's
  cached speed and branch-specific vertical offsets inconsistent. Both
  targeted and ground attack presentation paths use the correction.
- The authoritative server guarantees double hits at **201% and above**.
  Exactly 200% retains its existing 50% chance; lower ten-point probability
  bands are unchanged. An ephemeral, non-serialized character field retains
  exact speed before `AttackRun` quantization, including 201–209%. Score reset
  clears it, recalculation refreshes it, and detached skill-learning commits
  copy it together with the prepared runtime score. No persisted schema,
  score ABI, packet, damage formula or 900 ms physical-action interval changed.
- Local prediction shares the existing HUD speed formula, including item,
  transformation and affect contributions. Observer presentation continues
  to consume the server's double/critical flags. These are project rules,
  not a new claim about native probabilities or native 120 ms timing.
- `Build-Client.ps1 -NoDeploy` passed: architecture 1,240,440 checks, socket
  231, crash capture 83, launcher 56. Known nonblocking `C4996` socket warnings
  remain. Tests cover relative number trajectories, construction delays,
  first-number fallback, probability boundaries and shared HUD inputs.
- Focused Go attack-speed/cadence and critical skill-learning tests passed.
  The broader game/model run passed with only
  `TestCriticalCityTreasuryActualWorldLoopProgress` excluded. That unchanged
  test uses the real clock in UTC and attempted treasury collection on
  Saturday, when the domain rejects it; the unfiltered run therefore failed.
  Initial skill-learning failures exposed the missing exact-speed commit
  copy and were fixed before the final run. Full failure diagnostics remain
  under `build/double-hit-followup/`; this is not an all-tests-green claim.
- Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `D7D87D8615A50271DFA9FB86393DB3280E800FC2B9A46E239A12E835189FA4FF`.
  It supersedes the older candidate below and requires the existing 7.48
  resources plus the updated server for the authoritative 201% rule.
- Installed runtime `tmproject/client748/WYD.exe` was observed at SHA-256
  `1FE4CBA89C18847C3E4C35CC9497B049B9E01DCAB12E58B36152AE7FBFE19F6A`.
  This batch did not replace it. No launch, publication, restart, commit or
  push occurred. Gameplay proof remains pending for owner and observers,
  melee/ranged attacks, normal/critical doubles, speed falling to 200%,
  equip/unequip, affect expiry, scene change and relogin. Not `CLIENT_TESTED`.
- Cleanup was rejected before execution; no alternate deletion was attempted.
  Retained generated output: `build/double-hit-followup/focused-tests.log`,
  `build/double-hit-followup/go-tests-final.jsonl` and
  `tmproject/build/tests/Release/crash-capture-tests-9668-542330859/`.
  Failure diagnostics and the incremental candidate are intentionally retained.

## Operational follow-up — 2026-10-17 00:14 UTC, authorized service restart

The owner requested a restart after the double-hit presentation adjustment.
The existing 900 ms server interval limits separate physical actions, not the
two presentations within a double hit. It was not changed; neither a game
restart nor a host reboot is required to activate a client-only change.

At the owner's request, the existing game and player API were nevertheless
restarted, disconnecting the one established game connection. The pre-stop
database backup succeeded. The game stopped gracefully with `Result=success`
and `ExecMainStatus=0` before being started again. Both services started at
`2026-10-17 00:14:14 UTC`: game PID `1951043`, player API PID `1951046`.
Both readiness endpoints report ready and TCP 8281 is listening. No
warning-priority game/API journal entries appeared during the restart.

Active release remains `/srv/wyd-go/releases/20261016T232841Z`; both running
executable paths and hashes match the prior publication receipt below. No
files were uploaded, no release was activated, and no client patch was
published. Caddy and PostgreSQL were left running; the VPS was not rebooted.
The double-hit candidate below remains local and uninstalled, so this restart
does not deliver its visual changes to players. Gameplay validation remains
pending.

## Local follow-up — 2026-10-16, double-hit presentation

`MODERNIZACAO_COMPATIVEL`: owner-requested presentation adjustment, not a
claim that 120 ms is native 7.48 timing. The targeted and ground attack-number
paths now schedule the second number 120 ms after the first using the same
visibility curve. Previously, longer second-number lifetimes and half-view
windows produced roughly 580–660 ms gaps in ordinary/critical branches.
Double-hit state is assigned before starting the animation, including clearing
stale state on a single attack. The existing double-animation speed takes
precedence over a previous skill's motion count. Physical projectile follow-ups
use the same 120 ms gap instead of the existing 500/700 ms intervals; ordinary
single-projectile timing is unchanged.

The server, packet layout, damage calculation, double-hit probability and
900 ms physical-action interval are unchanged. Existing attack-envelope
evidence was reused; no new native-parity claim is made. Local prediction and
remote actor dispatch retain their existing routing.

- `Build-Client.ps1 -NoDeploy` passed. Architecture tests: 1,229,612 checks;
  socket tests: 231 checks; crash capture: 83 checks; launcher: 56 checks.
  Focused assertions cover normal/critical number lifetimes, construction
  delay, unsigned timestamp arithmetic, missing first numbers, weapon types,
  freeze and previous skill motion counts. They do not certify renderer
  behavior across timer wrap or actual gameplay animation.
- Existing nonblocking `C4996` warnings remain in `CPSock.cpp` for `inet_addr`
  and `WSAAsyncSelect`; no blocking compilation errors occurred.
- Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `1FE4CBA89C18847C3E4C35CC9497B049B9E01DCAB12E58B36152AE7FBFE19F6A`.
  It still depends on the existing 7.48 runtime resources and server.
- Installed runtime `tmproject/client748/WYD.exe` was not replaced; its
  observed SHA-256 is
  `E96B8514B8B5CEC3A6C7D832DE8E0FE3AA11F4A390816A74C96A34285CCB5A36`.
  No client launch, publication, VPS change, restart, commit or push occurred.
- Gameplay confirmation remains pending for owner and observers, melee and
  ranged attacks, critical/noncritical doubles, freeze, single attacks after
  doubles, scene changes and relogin. Automated checks are not `CLIENT_TESTED`.
- Cleanup was rejected before execution. No alternate deletion was attempted.
  Retained task output: `build/double-hit-client-build.log` and
  `tmproject/build/tests/Release/crash-capture-tests-23824-540543968/`.
  The current incremental candidate is intentionally retained for local testing.

## Published follow-up — 2026-10-16, instance leveling balance

The owner authorized publication of all pending changes and a game restart.
Nightmare, Water and Magic Chamber leveling changes below are active in
`/srv/wyd-go/releases/20261016T232841Z`. These are explicit project balance
choices (`MODERNIZACAO_COMPATIVEL`), not certified native monster statistics.
The preceding Nightmare lifecycle and client patch 10032 remain included.

- Source: dirty working tree on `main`, HEAD
  `de87cd8789bef3165babbdf41fe1f86bc449106a`. No commit, push, local client
  replacement or gameplay launch was performed. Personal client settings
  were excluded and preserved.
- `Build-VpsRelease.ps1` cross-built Linux/amd64 game and player API binaries.
  Its tracked runtime-data inventory was supplemented with an explicit
  allowlist of 81 untracked JSON templates: 17 Nightmare monsters, 28 Water
  monsters, 15 Magic Chamber monsters and 21 existing Nightmare protectors.
  All 81 entries were inspected in the resulting archive. Account state and
  private runtime settings were excluded.
- Release archive: 22,236,652 bytes, SHA-256
  `4A3339578D7BC7DD79FFCF10CC832387C377C14DB88283E0D2AC5972AC4238BD`.
  Game SHA-256:
  `B2BF27AC1ED8F59C0B3E29A2F3800EA243C258FE0494EE939D6F8719EF848F2B`.
  Player API SHA-256, unchanged:
  `C6E3836149FDDEAF58E893485CD81FD2DE87FD0C8F7A72520771B5A7FBCBFCB5`.
- Upload archive checksum and all 93 payload checksums matched before
  activation and against the active release afterward. Coverage includes
  both executables, all 81 templates, instances, all six affected generator
  files, the public news article, changelog and client manifest.
- The established installer activated the release atomically after an
  authorized graceful game stop. Game PID `1943006` and player API PID
  `1943018` started at `2026-10-16 23:28:44 UTC`; their `/proc/.../exe`
  targets and hashes match this release. Both readiness endpoints report
  ready and TCP 8281 is listening. Caddy was reloaded, not rebooted.
- Backup `wydgo-20261016T232841Z.dump` succeeded (106,295 bytes). Previous
  release `/srv/wyd-go/releases/20261016T183715Z` remains the rollback target.
  Server/staff/service/proxy configurations matched before activation;
  both private authentication environments remain byte-identical afterward.
- No warning-priority game/API journal entries appeared after startup.
  Caddy validation passed with its existing redundant-header and HTTP-only
  warnings, which do not block this deployment.
- Public HTTPS news and changelog include the leveling update. The manifest
  and public HTTP launcher feed retain patch 10032; no duplicate client patch
  was necessary. New article: `/news/instance-leveling-balance/`.
- Fresh `go test -count=1 ./...` and `go vet ./...` passed. Portal build,
  all 60 portal tests and Astro checks passed (zero errors, warnings or
  hints). No disposable PostgreSQL fixture integration or live gameplay
  proof is claimed. Prior unchanged client build evidence was reused.
- The installer removed remote upload/extraction staging. The post-activation
  verification checksum file was also removed. Local cleanup was rejected
  before execution; no alternate deletion mechanism was attempted. This
  publication's exact retained paths are
  `build/deploy/wyd-deploy-20261016-instance-leveling/`,
  `build/instance-leveling-portal-build.log` and
  `build/instance-leveling-portal-check.log`. Active release and rollback
  artifacts are unaffected.

The active changes still require gameplay confirmation for all three
activities. Publication does not certify unrelated gameplay gaps, the global
English migration or native encounter parity. Earlier blocked cleanup paths
remain untouched; they are listed in the previous publication receipt below.

## Published follow-up — 2026-10-16, Magic Chamber leveling monsters

`MODERNIZACAO_COMPATIVEL`: explicit owner-selected server balance matching
Water, not native monster-statistics parity. Implemented and tested locally;
published with the restart in release `20261016T232841Z` above. Gameplay
validation remains pending.

| Tier | HP | Physical/magic attack | Defense | Base EXP per monster |
| --- | ---: | ---: | ---: | ---: |
| Normal | 1,000 | 1 | 1 | 200,000 |
| Mystic | 4,000 | 1 | 1 | 300,000 |
| Arcane | 10,000 | 1 | 1 | 500,000 |

The three Magic Chamber generator files now reference 15 independent static
definitions with `MCN_`, `MCM_` and `MCA_` prefixes, including each tier's boss.
All four rooms use the same tier balance. Names fit the existing 12-byte field;
appearance and unrelated statistics retain their source values. The original
templates, Water, Nightmare and other maps do not borrow these definitions.
No runtime HP/EXP override is used.

Definitions have zero gold and no carry/drop inventory. The central EXP-only
kill policy also excludes global event drops and special boss loot before
inventory mutation or event reservation. EXP and boss death bookkeeping remain
active. Final physical/spell damage is capped at one before player absorption,
preserving zero-damage outcomes. Existing EXP modifiers and party sharing apply.
Entry rules, four-room progression, populations, three-minute room timers,
ten-second transitions and completion rewards are unchanged; the delayed boss
still uses the fourth room's remaining time.

Validation: `go test ./internal/data ./internal/game -count=1` passed
(data: 2.835 seconds; game: 12.915 seconds). Coverage includes all 15 definitions,
exclusive references, all 12 real-map room spawns and three delayed bosses,
private runtime ownership, physical/spell damage caps, and native/event/boss
loot exclusion with empty/full inventories. Open-world and Cube controls retain
their normal loot and damage. This is automated evidence, not gameplay proof.

## Published follow-up — 2026-10-16, Water leveling monsters

`MODERNIZACAO_COMPATIVEL`: explicit owner-selected server balance, not native
monster-statistics parity. Implemented and tested locally, then published with
the restart in release `20261016T232841Z`. Gameplay validation remains pending.

| Tier | HP | Physical/magic attack | Defense | Base EXP per monster |
| --- | ---: | ---: | ---: | ---: |
| Normal | 1,000 | 1 | 1 | 200,000 |
| Mystic | 4,000 | 1 | 1 | 300,000 |
| Arcane | 10,000 | 1 | 1 | 500,000 |

All 27 Water configurations (eight rooms plus the boss room per tier) now
reference 28 independent static definitions, with `WTN_`, `WTM_` and `WTA_`
prefixes. Bosses and boss-room escorts use the same tier balance and EXP.
The definitions retain their conservative 12-byte naming budget within the
16-byte NPC field (15 text bytes plus NUL), retain source appearance
and unrelated statistics, and have no carry/drop inventory and zero gold.
Other maps and Nightmare do not borrow these Water definitions; their original
shared templates are unchanged. No runtime HP/EXP override is used.

Water now shares Nightmare's central EXP-only kill policy. Native carry, gold,
global event drops and special boss loot are excluded before inventory changes
or event reservations. EXP and boss death bookkeeping remain active. The
shared incoming-damage boundary caps physical and spell hits at one before
player absorption; misses and absorption can still yield zero. Existing EXP
modifiers and party sharing still apply to the configured base rewards.

Room populations, timers, entry rules and completion behavior are unchanged.
In particular, next-room Water Scroll rewards and their full-inventory ground
fallback remain available; these are progression rewards, not monster drops.
Boss rooms retain their reward-free return to room one.

Validation: `go test ./internal/data ./internal/game -count=1` passed
(data: 2.962 seconds; game: 12.387 seconds). Coverage includes all 28 dedicated
definitions and their preserved appearance, all 27 room references and
progression rewards, real-map spawning through the production private-room
path, physical/spell damage caps, and guaranteed native/event/boss loot
exclusion with empty and full inventories. Open-world and Cube controls
retain their normal loot and damage. Water chain and full-inventory reward
tests passed. This is automated server evidence, not gameplay validation.

## Published follow-up — 2026-10-16, Nightmare leveling balance

`MODERNIZACAO_COMPATIVEL`: explicit owner-selected server balance, not native
monster-statistics parity. Implemented and tested locally, then published with
the restart in release `20261016T232841Z`. Gameplay validation remains pending.

| Tier | HP | Physical/magic attack | Defense | Base monster EXP |
| --- | ---: | ---: | ---: | ---: |
| Normal | 2,000 | 1 | 1 | 200,000 |
| Mystic | 5,000 | 1 | 1 | 400,000 |
| Arcane | 10,000 | 1 | 1 | 600,000 |

The values belong to 17 independent static monster definitions in `data/npcs/`,
including Arcane's Lugefer. Their names use the `NMN_`, `NMM_` and `NMA_`
prefixes and retain their conservative 12-byte naming budget within the
16-byte NPC field (15 text bytes plus NUL). The three Nightmare generator
files and both Arcane boss configurations reference these exclusive definitions;
shared NPC templates, monsters outside Nightmare and friendly protectors remain
unchanged. Appearance and unrelated statistics retain their source values.
All 17 dedicated definitions have no carry/drop inventory and zero gold.
There is no runtime HP/EXP template rewrite. Ordinary respawns, restoration and
delayed boss creation all use the dedicated definitions.
Final physical/spell damage is capped at one
before player absorption, so level/attribute bonuses cannot produce stronger
hits; misses and absorption can still yield zero damage. Attacks against
protectors use the same cap without changing protector HP.

Existing EXP modifiers and the approved 10% loss per dead protector still
apply to these base rewards. Nightmare kills grant no items or gold: the shared
kill-reward planner skips native carry, global event drops, gold and special boss
loot before any inventory mutation or event reservation. Boss death bookkeeping
and EXP remain active. All six configurations also have no completion item reward.
Timing, population and entry rules are unchanged. No client or wire change is required.

Validation: `go test ./internal/data ./internal/game -count=1` passed
(data: 2.682 seconds; game: 14.286 seconds), including the no-loot follow-up.
Coverage includes dedicated
definition isolation, preserved appearance, exclusive generator references,
wire-name limits, friendly/outside-instance exclusions, zero/one/large incoming
damage, repeated respawns, restoration and delayed Lugefer balance. Installed-map
tests explicitly cover all six party/solo tier configurations, including inline
item rules: Normal and Mystic each spawn 101 hostiles; Arcane spawns 80 plus its
delayed boss. This is automated server evidence, not gameplay validation.

No-loot follow-up: focused data and game tests passed (0.117 and 0.045 seconds).
They cover all 17 empty drop definitions, six reward-free configurations and
Normal/Mystic/Arcane kills with empty and full inventories. Guaranteed native,
global-event and special boss loot plus gold are excluded without changing EXP;
open-world and non-leveling-instance controls still plan all reward sources,
including the full-inventory ground fallback. Water now follows the additional
no-loot policy documented above. Publication and gameplay validation remain pending.

## Published follow-up — 2026-10-16, Nightmare clocks and protected NPCs

`MODERNIZACAO_COMPATIVEL`: server lifecycle repair using the existing wire
contract, plus a native counter-label correction. Published as patch 10032
and active VPS release `20261016T183715Z`; local runtime installation and
actual gameplay validation remain pending.
The six Normal/Mystic/Arcane party and solo configurations specify a four-minute
admission window. The runtime incorrectly treated its end as the encounter end,
spawned monsters immediately and displayed a monster counter. A late entrant
could therefore receive only one minute before expulsion.

The read-only W2PP reference separates admission from combat:
`Code/TMSrv/_MSG_UseItem.cpp` (Normal/Mystic/Arcane admission paths around
lines 2494–2745) sends the remaining part of 240 seconds;
`Code/TMSrv/ProcessSecMinTimer.cpp` (around lines 1040–1093) starts combat with
`sm.Parm = 900`. Its cleanup schedule is around lines 660–690. This is
secondary server-algorithm evidence, not independent proof of full native
7.48 Nightmare parity.

- Admission remains four minutes: Normal opens at minutes 00/20/40, Mystic
  at 05/25/45 and Arcane at 10/30/50. No monsters spawn during this wait.
- Combat starts when admission closes and lasts 15 minutes. The absolute end
  survives individual respawns and state restoration; late admission, a delayed
  scheduler tick or restart does not grant a fresh combat period.
- The counter reports surviving protected NPCs, not hostile monsters. Normal
  has five protectors; Mystic and Arcane have eight, using the secondary 7.59
  generator/template data. Protectors spawn during admission, can be attacked
  by encounter monsters during combat, and never respawn. Their loss mask is
  persisted and cannot be reset by rejoining or restoring the encounter.
- Each dead protector removes 10% of original monster EXP before party sharing,
  as explicitly approved by the owner. Eight survivors yield 100%; zero of
  eight yield 20%. The
  [official Arcane guide](https://feedback.kersef.com/pt-br/knowledge-bases/21/articles/47597-nightmare-a)
  confirms eight protected NPCs and decreasing EXP, but not the percentage.
  The percentage and imported NPC data are project choices sourced from
  `MacedaoRG/w2Project-SeitTbNao`, not independent 7.48 parity evidence.
  Reproduction roots on its `main` branch: `Projects/Gameserver/GetFunc.cpp`
  (EXP around lines 2458–2540), `Projects/Gameserver/Basedef.cpp`
  (`DoNightmare`, around lines 9239–9368), `Release/TMSRV/NPCGener.txt` and
  the corresponding 808-byte NPC templates under `Release/TMSRV/npc/`.
- The client uses the native `%d Left` format and preserves a visible zero
  while the encounter timer is active. Departure clears the timer before
  clearing the count, hiding both. Existing evidence in
  [instance runtime controls](../.agents/research/client748/flows/ui/instance-runtime-counter-controls.md)
  is reused; native `FUN_00489260` proves the label, not the constructor,
  geometry or complete teardown behavior. No wire or asset format changed.
- Both legacy and critical-persistence admission paths preserve ticket/state
  commit and rejection behavior. Failed combat creation closes the encounter
  without a clear reward.
- Each committed monster death immediately replaces only that monster,
  retaining its type and configured spawn origin. Survivors are not cleared;
  no wave-clear gate or completion reward exists. Respawns continue without a
  quota until the absolute combat deadline and do not resend/reset the clock.
  This uses the W2PP `Code/TMSrv/MobKilled.cpp` Nightmare generator branches
  around lines 1074–1082, plus the owner's explicit behavior requirement.
  The owner explicitly rejected the secondary 7.59 rule that stops selected
  groups after their protector dies: all common groups continue respawning,
  including with zero survivors. Runtime spawn origins are rebuilt with the
  stage after restoration.
- Arcane Lugefer is a separate one-shot boss, not an ordinary respawn group.
  It first appears with five minutes remaining, an explicit owner choice;
  the official guide supplies no exact minute. Its survivor condition is
  interpreted as at least one living protector. Once spawned, it remains
  restorable even if the remaining protectors subsequently die. Its defeat
  is persisted atomically with player rewards through critical persistence;
  rejection leaves both the boss and rewards unchanged. A defeated boss
  never respawns within that encounter.
- Arcane uses four common families: `Tauron_____`, `Adamant_Tauron`,
  `Hell_Spider2` and `Manticora_`. Existing generator locations/counts and NPC
  statistics are reused, with `Adamant_Tauron` serving as the armored Tauron
  counterpart. Exact native template identity, population distribution and
  boss loot parity have not been independently established. This repair is
  not a claim of complete native 7.48 encounter parity.

Validation: the focused Nightmare suites and the complete
`go test ./internal/game ./internal/data -count=1` packages passed
(`game`: 12.053 seconds; `data`: 2.576 seconds). Coverage includes all
three tiers and schedule windows, late entry, hour rollover, absolute expiry,
individual repeated respawns, restoration, critical commit/rejection and
combat-spawn failure, protected-NPC attacks/deaths, loss restoration, all six
installed protector configurations, EXP scaling and detached state isolation.
Repeated death tests exercise both player-reward commits and deaths without a
player, with stationary observers and an unchanged surviving monster.
The installed-map test ran successfully and spawned five protectors during
Normal admission and 101 hostile monsters at combat start.
Both real Arcane configurations spawned eight protectors and 80 common
monsters on valid, distinct terrain tiles, then one Lugefer at the ten-minute
combat mark. Tests also cover no duplicate boss, no boss before its deadline,
late ticks, boss restoration/defeat, zero-protector common respawns, invalid
boss configuration before ticket consumption, and atomic reward commit and
certified rejection. No additional client build was needed for these
server-only follow-up changes.
The preceding `Build-Client.ps1 -NoDeploy` passed: 1,229,517 architecture checks, 231 socket
receive checks, 83 crash-capture checks and 56 launcher checks. Candidate
`tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
`E96B8514B8B5CEC3A6C7D832DE8E0FE3AA11F4A390816A74C96A34285CCB5A36`.
The installed `tmproject/client748/WYD.exe` remains different, SHA-256
`D689BF77899F906629AE93F2C127EC72F3099DBE4A959B73F08B5F45693A05FF`.
Publication receipt:

- Source: dirty working tree on `main`, HEAD
  `de87cd8789bef3165babbdf41fe1f86bc449106a`. No commit, push, local runtime
  replacement or game launch was performed.
- Patch 10032 contains only `WYD.exe`: ZIP 829,102 bytes, SHA-256
  `60EB65FCD22EF96CE47337897999F409A927D4BA22365D2229616CB11191B411`.
  The executable hash is the candidate hash above. Earlier assets remain in
  the cumulative launcher feed; no resource or packet-format change is needed.
- Linux release archive: 22,229,289 bytes, SHA-256
  `238BA87689B09AE19E4263C69E75A55846AE906647C9E7275909226706DBC7F1`.
  The tracked-data packager was supplemented with an explicit allowlist of
  all 21 new protector templates, each referenced by the installed config.
  The archive excludes account state and private runtime settings.
- Active release: `/srv/wyd-go/releases/20261016T183715Z`. Game PID 1895803
  and player API PID 1895815 started at `2026-10-16 18:37:18 UTC`.
  Actual running executable hashes match the packaged binaries:
  game `CD0D86612C27ADD4D97AF40DF98C09B9542118167B8FC2C969C9D1389BF42BF4`;
  API `C6E3836149FDDEAF58E893485CD81FD2DE87FD0C8F7A72520771B5A7FBCBFCB5`.
  Both readiness endpoints report ready and TCP 8281 is listening.
- The first activation safely restored the old release because a game
  connection appeared. The explicitly authorized restart then stopped the
  game before retrying activation. Caddy was reloaded, not rebooted; its
  existing redundant-header/HTTP-only validation warnings are nonblocking.
  No warning-priority game/API journal entries appeared after startup.
- Backup `wydgo-20261016T183715Z.dump` succeeded (102,914 bytes). Previous
  release `20261016T032951Z`, patch 10031 and the prior feed are retained.
  Server/staff configuration hashes are unchanged; private environments and
  account data were preserved by the installer.
- Public HTTP/HTTPS feeds and ZIP downloads return 200. ZIP and embedded EXE
  hashes match; the retained previous feed is exactly the new feed's prefix.
  Site manifest, home and News advertise 10032. Installed instance/generator
  hashes match local sources and all 21 protector templates are present.
- Fresh `go test -count=1 ./...`, `go vet ./...`, portal build, 60 portal
  tests and portal checks passed (zero errors, warnings or hints). Existing
  client build evidence above was reused with unchanged inputs. No database
  fixture integration or live gameplay proof is claimed.
- Remote upload staging was removed by the publishers. Subsequent cleanup
  was blocked before execution; no alternate deletion method was attempted.
  Leftovers: `build/deploy/wyd-deploy-20261016-nightmare-10032/`,
  `build/patch/patch-10032.zip`, `build/patch/verify-10032.zip`,
  `build/patch/verify-10032-http.zip`,
  `build/nightmare-10032-portal-build.log`,
  `build/nightmare-10032-portal-check.log` and the inactive failed-attempt
  release `/srv/wyd-go/releases/20261016T183640Z`. These do not affect the
  active release or its retained production rollback reference.

Actual Nightmare gameplay, full native population/loot parity and launcher
installation remain pending; this is not `CLIENT_TESTED` evidence. Use
`WYDLauncher.exe` with the game closed to install patch 10032.

## Published follow-up — 2026-10-16, patch 10031 and server restart

Published client patch 10031 and deployed server/site release
`/srv/wyd-go/releases/20261016T032951Z` with the owner's permission to
disconnect players. SSH succeeded with the existing `favela-admin` account
and key; no password reset, key replacement or SSH-security change was needed.
The earlier refusal cause remains unconfirmed.

The paired update delivers the tab/SND and distinct buff-icon contracts below,
plus pending authoritative guild-kingdom, Warrior's Seal and off-hand promotion
changes. Previous client assets remain supplied by the cumulative launcher
feed. Nightmare and Backpack gameplay are not completed by this deployment.

- Patch: 832,800 bytes, SHA-256
  `38D1B4C3B069A3F19F61E6280EF697FA53BE0E6631FA1617DAD59F1CDD1B03B2`.
  Explicit two-file payload: `WYD.exe` and `skillhelp.dat`; no player settings
  or launcher binaries. Executable: 1,881,088 bytes, SHA-256
  `2F75CA77281CCF21F899A6879DBB1A353C6FBE69A98A2F24C52D6E9309A51F1A`.
  Skill asset SHA-256:
  `1BDD96ADC246CF2A71097301EA1C5152D24F5109A43A557A22B0C3335E3DDA24`.
  Compared with 10030, 72 skill records have adjusted wrapping; all 102 retain
  identical IDs and prose. All other 33 non-executable dependencies match 10030.
- Linux release archive: 22,202,420 bytes, SHA-256
  `2D1CDCD932E4B672A2173F0C9E4DF051D2F48CDE9FBD5F3D1755AAE369BAF07E`.
  Game binary SHA-256:
  `E8D03DB0DCA52D741309B43A791D2200981F9CA0B9FEC72FB3231B7D13C64F6D`;
  player API:
  `C5426522FA87CEB8C653A43F45B15A1690249EFDF84AAC4A72069F9B592D209A`.
  Both running `/proc` executable hashes match these artifacts.
- Game/API restarted at `2026-10-16 03:29:54 UTC`, PIDs 1748601/1748614;
  Caddy restarted at 03:29:55, PID 1748674. Both readiness endpoints return
  `ready`; all three services are active. The post-restart warning/error
  journal query returned no entries. Caddy validation reported only existing
  redundant forwarded-header and HTTP-listener warnings, not blocking errors.
- Backup `wydgo-20261016T032951Z.dump` is present (99,941 bytes). Previous
  release `20261015T215714Z`, patch 10030 and the exact prior feed remain for
  rollback. The installer retained account/character state. One upload timed
  out before transferring files; a verified retry completed before downtime.
- Public HTTP/HTTPS feeds and ZIP downloads returned 200; ZIP and both entry
  hashes match. The HTTPS manifest, home and News pages advertise 10031 and
  return 200. The remote archive checksum and publication-prefix checks passed.
- Reused the unchanged, previously validated Release/x86 candidate and full
  Go/vet evidence below. Linux cross-build succeeded. Portal checks reported
  zero errors/warnings; 60 tests passed before the receipt update, and both
  affected public-content tests passed after rebuilding all 3,304 static pages.
  STATICALLY VERIFIED and AUTOMATED TESTED; CLIENT-TESTED remains pending.

Packaging uses the explicit artifact allowlist from the dirty working tree at
HEAD `fa1c853fa9d279719acd4aa0c3518a550505134f`, not committed From/To
revisions; `Build-ClientPatch.ps1` was not used. No source commit, push, index
change, local runtime replacement or game launch occurred. This publication
does not certify the unresolved global language migration or gameplay gaps.
Remote patch/release staging was removed by the installers. Local cleanup was
blocked by the execution policy: the release staging directory
`build/deploy/wyd-deploy-20261016-patch10031` and the three patch/verification
ZIPs for 10031 remain under `build/`; no alternate deletion was attempted.
See the
[patch receipt](client/launcher.md#published-patch-10031-2026-10-16).

## Local follow-up — 2026-10-15, distinct Samaritan and Critical Armor icons

Corrected the visual-ID collision: Samaritan now uses skill icon 3, Critical
Armor uses skill icon 15, and Coin Armor remains unchanged. The shared client
table serves the owner, target, party and C.C. buff recognition. Critical Armor
now activates its persistent effect on native visual 36, not Samaritan's 24.
The server retains canonical/persisted affect IDs, bonuses and durations and
normalizes only outgoing snapshots, including legacy raw type 50.
See [buff visual evidence](../.agents/research/client748/flows/combat/persistent-buff-visuals.md).

STATICALLY VERIFIED and AUTOMATED TESTED: focused regressions, full Go suite
and vet passed. Release/x86 build passed with 1,229,509 architecture checks,
231 socket, 83 crash capture and 56 launcher checks. Existing C4996 Winsock
warnings are nonblocking. The candidate is 1,881,088 bytes, SHA-256
`2F75CA77281CCF21F899A6879DBB1A353C6FBE69A98A2F24C52D6E9309A51F1A`.
Both client and server are required; local runtime installation, publication
and gameplay confirmation remain pending at this entry. No game was launched.
Existing tab/SND changes below remain included in this source candidate.

Publication has not occurred. Initial SSH checks used incorrect accounts
(`root` and `admin`), so their `Permission denied (publickey)` results do not
establish that the deployment key is invalid. The exact PowerShell history
identifies `favela-admin@46.224.58.209` as the deployment account. The server
host key still matches known_hosts. Follow-up attempts with the correct account
were closed before authentication (`banner exchange: Connection refused`);
the cause is unconfirmed, including whether failed attempts triggered a
temporary security block. No files were uploaded and no services were restarted.
Recheck the correct account after the connection issue is resolved, without
changing keys or server security settings. Repository layout and research-record
validation also passed.
Cleanup of the completed crash-test output directory
`tmproject/build/tests/Release/crash-capture-tests-16312-462865625` was
blocked by execution policy; the generated test artifacts remain there.

## Local follow-up — 2026-10-15, tab messages and SND status

Fixed slash-command dispatch: `/tab` and `/snd` no longer look up players named
`tab` or `snd`. Native 7.48 chat entry was traced in Ghidra; W2PP supplies the
secondary server reference. `/tab text` sets a label above the character;
`/snd text` saves private presentation text returned by an empty-body
`/PlayerName` lookup, not an automatic reply to ordinary whispers. A command
without text clears its message. Labels reset on character logout/relogin.
Mortal tab creation requires displayed level 70; advanced evolutions are exempt.

Added coordinated tab-color v1: `/tab`, `/tab2`, `/tab3`, `/tab4`, `/tab5`,
`/tab6`, `/tab7`, `/tab8`, `/tab9` select gray, red, green, blue, yellow, cyan,
pink, orange and white respectively. Example: `/tab3 Selling equipment`.
Repeat the text with another command to recolor it. The server owns text/color
and publishes only to the owner and already-visible same-instance observers.
Updates reuse the in-place metadata handler without respawn, movement/vitals
packets or animation resets. Incoming copies and the outgoing temporary tab
buffer are bounded to 25 text bytes plus NUL. SND reserves notice-prefix space,
accepts 87 bytes and cannot invoke panel control syntax through user text.

CreateMob keeps the existing coordinated 328-byte ABI: tab color uses former
padding byte 327, leaving cape/server byte 326 unchanged. Unknown colors use
the previous gray. Trade frames are excluded because 327 overlaps their title.
This is a source-client extension, not a claim about native CreateMob layout.
Both the client and server must be updated for the feature; this is not a
client-only fix. See [chat evidence and contract](../.agents/research/client748/flows/ui/chat-tab-status.md).

STATICALLY VERIFIED and AUTOMATED TESTED: focused Go command/wire tests,
full `go test -count=1 ./...` and `go vet ./...` passed. Release/x86
`Build-Client.ps1 -NoDeploy` passed: architecture 1,229,504 checks, socket 231,
crash capture 83 and launcher 56. An initially missing include was corrected;
the affected-target build also reported existing nonblocking C4996 warnings
for deprecated Winsock APIs. No blocking errors remain.

Candidate `tmproject/build/TMProject748/Release/WYD.exe`: 1,881,088 bytes,
SHA-256 `7464C7B074398D466EEB462AE366A1EE89862CE6CC3211D758DD76E6FBF78162`.
Installed runtime was not replaced. No public client patch, VPS deployment,
server build installation or service restart was performed. CLIENT-TESTED
remains pending; no game process was launched. This follow-up does not close
unrelated pending fixes or existing language-migration debt.

Research validation and layout/local links passed (203 indexed documents).
Scoped whitespace and affected-file language checks passed. The whole-tree
`git diff --check` still reports an unrelated existing blank line at EOF in
`wydgo748/data/NPCGener.txt`; that user-owned change was preserved. Cleanup was
rejected before execution: temporary exports/logs remain in
`build/chat-labels-10031/`, as do test artifacts in
`tmproject/build/tests/Release/crash-capture-tests-21772-460996531/` and
`crash-capture-tests-17108-461060328/`. No broad deletion or retry was attempted;
current incremental build output is retained.

## Published follow-up — 2026-10-15, client-only patch 10030

Published the validated client candidate and its runtime dependencies through
the launcher, without deploying server binaries/data or restarting services.
The 35-file payload contains `WYD.exe`, required `skillhelp.dat`, the sidebar
button atlas definitions, both KR asset manifests and 30 restored costume
meshes. Previously published costume dependencies are intentionally retained
in this explicit allowlist. Player settings and launcher binaries are excluded.

This publishes the native skill descriptions faithfully localized to English,
isolated prose/requirements and bounded tooltip placement, the genuine
face/class-change guard for transformation effects, the pending C.C. client
logic and Character menu states. Server-side guild kingdom, Warrior's Seal
and off-hand promotion changes remain outside this publication. Nightmare and
Backpack gameplay were not implemented by this batch.

- ZIP: 1,216,419 bytes; SHA-256
  `F08898C442024C754C5CE0DE8D82EA374917711A1A331340549F471CCD4E3461`.
- Executable: 1,880,576 bytes; SHA-256
  `3F38F354E4108FDE1C24BE15E456DF2B77D9536429E16DC13C8CAF2B192A5429`.
- Skill asset SHA-256:
  `D614DA42CE36B2C2CA61DEC6BF4DF964D047C28A4A3B618D53695C26FC967629`.
- Reused the unchanged Release/x86 candidate and passing gates recorded below;
  no source changed after that build. All 35 ZIP entries matched their sources.
  Fresh public HTTP and HTTPS feed/ZIP requests returned 200; all downloaded
  entries and ZIP hashes matched. The remote ZIP hash matched as well.
- The feed appends 10030. Its previous-feed rollback is byte-identical to the
  retained prefix, and the 10029 ZIP hash is unchanged. Remote staging was
  removed by the publisher. Local cleanup was rejected before execution:
  `build/patch/patch-10030.zip`, `verify-10030-http.zip` and
  `verify-10030-https.zip` remain in `build/patch/`. Current incremental build
  output remains available.
- Current remains `/srv/wyd-go/releases/20261015T215714Z`. Game PID 1694828
  and player API PID 1694841 retain their `2026-10-15 21:57:17 UTC` start time;
  readiness returned `ready`. No service restart/reload, player disconnection,
  account mutation, website update or local runtime replacement occurred.

Publication is verified; CLIENT-TESTED remains pending. Restart the client
through `WYDLauncher.exe` to receive the patch. This scoped update does not
close the existing language-migration debt or certify a global release.
See the [launcher receipt](client/launcher.md#published-patch-10030-2026-10-15).
Earlier local-only entries below describe the implementation history, not the
current publication status of their client changes.

## Local follow-up — 2026-10-15, unwanted appearance-refresh glow

Restored the native face/class-change guard around the whole transformation
effect block in `TMHuman::OnPacketUpdateEquip`. Previously only the first
particle allocation was conditional; ordinary `0x36B` snapshots always played
sound 158, a bright shade, transformation motion and a level-up effect. These
effects now run only for genuine face/class changes. Equipment, color, mount,
weapon, object and HUD updates remain outside the guard for both the owner
and observed characters. No server or wire change was needed.

Classification: `PARIDADE_NATIVA`, supported by `FUN_0052b116` and its jump
past the effect block to `0x0052b46c`. See
[equipment appearance evidence](../.agents/research/client748/flows/ui/update-equip-visual-contract.md).
STATICALLY VERIFIED and AUTOMATED TESTED: Release/x86 build passed using
`Build-Client.ps1 -NoDeploy`; architecture 1,228,406 checks, socket 231,
crash capture 83 and launcher 56 passed. Thirteen new source-contract checks
protect conditional effects and unconditional state application.

The candidate at `tmproject/build/TMProject748/Release/WYD.exe` also contains
the preceding skill description fix. The installed runtime was not replaced.
No patch publication, VPS deployment or restart. CLIENT-TESTED remains pending
for buff animation continuity and genuine transformations. Nightmare behavior
has not been changed by this follow-up.

## Local follow-up — 2026-10-15, skill description presentation

The initial restoration was incomplete: existing English paraphrases omitted
native formulas, and skill metadata still overwrote prose in shared rows.
Imported all 102 available descriptions from the native OnGame 7.48 asset
into `tmproject/client748/skillhelp.dat`, faithfully localized to English.
The required overlay preserves native effects, conditions and formulas;
`itemhelp.dat` and its historical ANSI encoding remain unchanged. Native IDs
with trailing spaces now load correctly. Skill prose uses panel-owned local
controls separate from the original twelve metadata rows. Apprentice,
learned and belt tooltips are measured after population, compacted and bounded
to the viewport; returning to an item restores its resource layout.

Native content and apprenticeship list binding are `PARIDADE_NATIVA`; bounded
decoding and isolated tooltip layout are `MODERNIZACAO_COMPATIVEL`. See
[skill description evidence](../.agents/research/client748/flows/ui/skill-description-text.md).
The inspected native corpus lacks prose for 5096, 5103, 5104 and 5400..5447;
their existing calculated captions remain. No descriptions were invented.

STATICALLY VERIFIED and AUTOMATED TESTED: Release/x86 build passed with
`Build-Client.ps1 -NoDeploy`; architecture 1,228,509 checks, socket 231,
crash capture 83 and launcher 56 passed. No blocking errors or warnings in
this incremental build. CLIENT-TESTED remains pending. The candidate is
`tmproject/build/TMProject748/Release/WYD.exe`; the installed runtime was not
overwritten. The new executable must ship together with `skillhelp.dat`.
No client patch, VPS deployment or restart was performed. Earlier investigation
logs/export remain in `build/client-skill-descriptions/` after rejected cleanup;
this follow-up created no scratch files in the repository.

## Local follow-up — 2026-10-15, C.C. reference logic and guild kingdoms

The owner's revised C.C. request retains the existing UI and uses the supplied
TMProject rather than the KR client. Buff membership, inactive-belt ordering,
mastery delay groups, summon ordering and independent hunger feeding follow
that reference. Compatibility guards preserve the server's 5..10-minute buff
duration, renew from elapsed affect time, and advance past failed casts without
stalling. See [Combat Control](client/combat-control.md) for the pinned source,
intentional timing adaptation and validation boundaries.

Guild membership now requires matching kingdoms at recruitment. Members and
subleaders cannot independently change kingdom; leader changes propagate to
connected-account characters transactionally, with offline characters repaired
from the guild registry on entry. See [guild kingdom and cape membership](guild-wars.md#guild-kingdom-and-cape-membership).

Both changes are `MODERNIZACAO_COMPATIVEL`. The full Go suite, vet and focused
guild-kingdom race tests passed. The Release/x86 client build and integrated
architecture tests passed using `Build-Client.ps1 -NoDeploy`. STATICALLY
VERIFIED and AUTOMATED TESTED; CLIENT-TESTED remains pending.

No local runtime executable replacement, client patch publication, VPS
deployment or restart was performed. Backpack behavior remains unimplemented
by this follow-up; skill-window prose was addressed in the later entry above.
The task-only cleanup command was rejected by execution policy. Reference
downloads remain in the system temporary directory `wyd-kr-cc-20261015`, and
four `cc-*` verification logs remain in the ignored root `build/` directory.
The incremental client build and native research corpus were preserved.

## Local follow-up — 2026-10-15, unfinished weapon, seal and menu fixes

Completed the three changes left unfinished after patch 10029. These changes
are local only; no patch publication, VPS deployment or restart was performed.
Classification: `MODERNIZACAO_COMPATIVEL`; existing item IDs, equipment slots,
packet layouts and resource IDs are unchanged.

- Removing the main weapon promotes a remaining eligible off-hand weapon from
  slot 7 to slot 6, preserving its UID and effects. Shields remain in slot 7.
  Direct and critical-persistence paths save before confirming the move,
  publish the normal swap followed by both hand snapshots, and refresh the
  appearance. The existing client stance-rebind fix remains in place.
  W2PP `_MSG_TradingItem.cpp:333–350` is a behavioral reference, not independent
  proof of native 7.48 parity.
- Warrior's Seal (4146, volatile 199) grants Mortals at displayed level 355+
  the Elite cape: Hekalotia 3191 (blue), Akeronia 3192 (red), or Adventurer
  3193 (white) without a kingdom/cape. Existing Elite and Hero capes are kept.
  Fame and seal consumption retain their existing behavior; cape replacement
  is included in the same account/character-state save and published only
  after success. W2PP `_MSG_UseItem.cpp:3213–3255` supplies the reference rule.
- Character button 293 in `UI/FieldScene2.bin` references texture set 398.
  Its one-row atlas set disappeared when `SButton` selected state 3. Sets
  398–403 now expose all five button states using their original atlas-134
  rectangles, matching the existing System set 404. No texture, binary layout
  or click binding was replaced. The previously traced menu lifecycle in
  `.agents/research/client748/flows/ui/system-menu-initial-layout.md` remains
  applicable; bounds checks in the renderer are preserved.

AUTOMATED TESTED: focused weapon/seal tests cover ordinary and critical saves,
commit, rejection, unknown outcomes, rollback, held-write repetition and exact
equipment publication order. The consumable settlement matrix covers all
three cape colors. ArchitectureTests passed 1,227,757 checks, including menu
atlas states and the existing dual-to-single/mounted pose tests. Earlier in
this review, `go test -count=1 ./internal/game ./internal/data
./internal/playerportal` also passed before the test-only additions.

STATICALLY VERIFIED and AUTOMATED TESTED; CLIENT-TESTED remains pending for
InkWC's mounted Balmung/Neorion removal, Character selection/hover/close, and
using the seal with each kingdom. The UI asset has no compilation dependency;
no executable rebuild or runtime executable replacement was necessary.
Repository layout/local links and scoped `git diff --check` passed. The
repository-wide whitespace check still flags a pre-existing extra blank line
at EOF in `wydgo748/data/NPCGener.txt`, outside this follow-up's changes.

## Local follow-up — 2026-10-15, skill description tooltip layout (unpublished)

Review of the separate-session `skillhelp.dat` work (102 English native skill
descriptions shown in dedicated tooltip rows):

- `TMFont2::SetText` splits any string over 42 characters into a second line
  inside the same row; 48 rows exceeded that and overlapped. The corpus is
  re-flowed to at most 40 characters per row (still nine rows per record).
- Rows were stacked with `m_Font.m_FontSize`, which is zero before the first
  render, so every line landed on the same band. They now use the resource's
  native row pitch (`m_vecDescParamPos[1].y - [0].y`) and the native text x.
  Order (owner request): name, calculated skill data, then the description,
  centred across the tooltip like the name and data rows.
- The help prefix is RRGGBBAA; reading it as ARGB drew the gold text purple.
  `item_help_748::LegacyColor` converts it to opaque ARGB (`CC9900FF` →
  `0xFFCC9900`), for item and skill help alike.

`Build-Client.ps1` passed (1,229,438 architecture checks); candidate
`WYD.exe` SHA-256
`3FE98CCC25ADD72D223D4F7D75F014BB7E0260B729805C6F9CCBFA8CEDDEE03A` is installed
in `tmproject/client748` when the running game closes, for the
owner's visual check. CLIENT-TESTED and publication pending.

## Local follow-up — 2026-10-15, off-hand weapon and Warrior's Seal (unpublished)

- Removing the main weapon (slot 6) while slot 7 holds a weapon, not a shield,
  moves that weapon to slot 6 (W2PP `_MSG_TradingItem`), on both persistence
  paths. A lone off-hand weapon was drawn through the body for observers.
- Warrior's Seal (4146): a Mortal at level 355+ without an Elite or Hero cape
  receives the Elite cape of its kingdom (3191/3192), or the white Elite of
  Adventurer (3193) without one (W2PP "Selo do Guerreiro"); the cape keeps its
  UID but not refinement/effects. Fame +10 is unchanged.
- The missing Character minibar icon is being handled in a separate session
  (`UITextureSetList.txt`, `NewApp.cpp`, `TMFieldSceneUI.cpp`).

AUTOMATED TESTED: `go vet ./...`, `go test -count=1 ./...` and
`Build-Client.ps1 -NoDeploy` (1,228,509 architecture checks) passed on the
working tree. CLIENT-TESTED and publication pending.

## Published follow-up — 2026-10-15, player-reported batch (patch 10029)

Fixes for the 15-item player report, published as patch 10029 with server
release `20261015T215714Z` (players disconnected with the owner's approval).

- Aylin follows W2PP `GetMatchCombineAilyn`: Ancient grades 5..8 need their own
  gem (2441..2444), other items four equal gems; WYD-Go also requires Ancient
  weapons. Rejections now explain the reason.
- Celestial level-90 unlock grants the second Cythera (same index as the
  equipped one, unrefined); a full inventory rejects the unlock. Existing
  level-90 Celestials did not receive it retroactively.
- Lac on the slot-8 earring: +9→+10 and +10→+11 at 16%, earring destroyed on
  failure (W2PP). Gems 180..183 apply to +10/+11 earrings.
- Golden Shield charges 100 gold per mastery point (W2PP), with a message
  when gold is short.
- Samaritan is a self buff on client and server (owner rule): selected players
  and monsters are ignored. The C.C. rebuff loop moves on when a buff does not
  cast instead of stalling.
- Enchant Ice (75) and Lightning Storm (79) require a bow on the server too.
- Guild InviteType 1..3 recruits directly into Sub1/Sub2/Sub3 (100M).
- Summons follow their owner on teleport; idle summons stay within 2 tiles
  and are placed beside the owner beyond 8 tiles. Combat is unrestricted.
- 64 monster templates with Con above 1000 now use Con 1 (owner request);
  this also fixes the near-invisible Nightmare M Grub swarms.
- Female Skeleton Set (4154): six `ch02xx90.msh` meshes restored from the
  read-only KR reference. KR Carnival Mount (4220): native mesh 19
  (`hs010120/hs010220`) is absent from every reference; it now shows the base
  hs01 horse with +13 glow.
- Support buffs last 5 minutes at mastery 0 up to 10 minutes at mastery 320
  (owner rule, replacing the W2PP AffectTime formula that gave 20–84 minute
  and 32–128 second buffs). Soul Limit lasts 5 minutes for a new Celestial up
  to 15 minutes at Celestial level 199 (owner rule; it lasted 8–32 s).
  Its attribute bonus now feeds the same derived formulas as base points:
  STR/3 + DEX/4 physical attack, DEX/5 defense, the class INT share of magic
  attack, 2 HP per CON and 2 MP per INT; attack speed and evasion read the
  final DEX (previously only STR attack, HP and MP were derived).
- Open: the HT cargo-guard skill and Avenging Spirit (55) with claws need the
  owner's clarification.

AUTOMATED TESTED: `go vet ./...` and `go test -count=1 ./...` passed;
`Build-Client.ps1 -NoDeploy` passed all gates. Candidate WYD.exe 1,876,480
bytes, SHA-256 `21F67FBC625D6A52868D57ECAE35372AA97382E5D6209D9D428E43B15E51CFB6`.
CLIENT-TESTED pending.

## Local follow-up — 2026-10-15, elite cape icon colors

The owner confirmed the Arch 355 report concerns only swapped item icons.
The grid-only override now selects blue for Elite Of Hekalotia (3191), red
for Elite Of Akeronia (3192), and white for Elite Of Adventurer (3193), using
existing textures. Kingdom, world cape appearance, catalog, stats and accounts
are unchanged. Classification: `MODERNIZACAO_COMPATIVEL`.

`Build-Client.ps1 -NoDeploy` passed: ArchitectureTests 1,227,747 checks, socket
231, crash capture 83, launcher 56, costume/shader gates and incremental
Release/x86 build. Candidate `tmproject/build/TMProject748/Release/WYD.exe`:
1,876,480 bytes, SHA-256
`C8F2768D337EA2F99699A19F2D8702E7098A75CD0648DF457161A841EAE24362`.
STATICALLY VERIFIED and AUTOMATED TESTED; CLIENT-TESTED and publication are
pending. No runtime install, server restart, account change, commit or push.
Patch 10028 below does not contain this later correction; patch 10029 does.
See [cape evidence](../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md#elite-cape-grid-icons--2026-10-15).
Research validation, repository layout/local links and scoped whitespace review
passed. The execution policy rejected cleanup of `build/cape-icon-inspection/`
and isolated `crash-capture-tests-20544-441018312` output; no bypass was attempted.

## Follow-up — 2026-10-15, native icon halo patch 10028 published

Supersedes the icon investigation below, not the pending server changes.
Patch 10028 contains only `WYD.exe`: the missing native per-item halo selection
and corrected sine pulse, alpha retention and full-width halo UV. The 10027
lighting fix alone did not resolve the owner's negative visual report.
No mesh, server release, account, service or local runtime was changed by 10028.
The static website receipt remains at 10027; the launcher feed ends at 10028.

AUTOMATED TESTED: ArchitectureTests 1,221,246 checks; socket 231, crash capture
83, launcher 56, shader/costume gates and incremental Release/x86 build passed.
The ZIP is 826,278 bytes, SHA-256
`D093974C2CE408AAD0E3737F27FEF48EB19025C5026DEB7FC3BE831A9F9A187E`.
Executable: 1,876,480 bytes, SHA-256
`75868D55FA061A5256F8A213785D2B6A81EF563BCDD26D161FE4D5BFB99CA2FD`.
Fresh HTTP/HTTPS feeds returned 200 and contained 10028; both downloaded ZIPs
and their sole embedded executable matched. No commit or push was performed.
Game PID 1510633 and API PID 1510645 still started at 02:32:14 UTC, and current
still resolves to release `20261015T023211Z` after publication.
CLIENT-TESTED: pending; do not treat publication as visual acceptance.
See [launcher receipt](client/launcher.md#published-patch-10028-2026-10-15).
Research validation, repository layout/local links (200 documents), scoped
diff whitespace checks and English review passed. Cleanup of generated patch
ZIPs/downloads and temporary site staging was rejected by the execution policy;
those artifacts remain, alongside isolated crash-test evidence. No bypass was
attempted. Published ZIPs, native evidence and incremental outputs are retained.

## Follow-up — 2026-10-15, client patch 10027 published

This section supersedes the working-tree status below, not the historical VPS
receipts. Entry HEAD was `fa1c853fa9d279719acd4aa0c3518a550505134f`, with
existing staged and unstaged changes preserved. Client patch 10027 is published;
the server changes below remain local. No commit, push, VPS restart, local
runtime installation, or agent-initiated game execution was performed.

Patch 10027 contains the executable and 24 restored costume meshes (25 files).
Public HTTP/HTTPS downloads and every embedded file hash matched. The six
public receipt pages were updated without replacing the server release.
Game PID `1510633` and player API PID `1510645` retained their start time
`2026-10-15 02:32:14 UTC`; current remains release `20261015T023211Z`.
See [launcher receipts](client/launcher.md#published-patch-10027-2026-10-15).

The owner subsequently tested patch 10027 and reported that inventory icons,
including Glihtning Stone, still lack their expected brightness. This is
negative visual acceptance: the lighting correction is shipped, but the
reported icon defect is not resolved. Further render/texture investigation is
required; passing source assertions and a build do not override that result.

- Private-instance departure now clears both HUD values after committed recall;
  persistence rejection and unknown outcomes retain the original state. Public
  quest timers keep their separate lifecycle.
- Ehre accepts Spirit Stone F and two Ankhs with the user-selected 50% chance
  per attempt. Both persistence paths share the recipe and rollback behavior.
- Client reentry replaces a previously retired remote actor on a new CreateMob,
  rather than retaining its pending-deletion state. Server tests cover the last
  five living instance monsters and visibility-boundary reentry. This addresses
  a concrete lifecycle defect; it does not prove every reported disappearance
  has the same cause.
- Eight ShopCostum06 costumes now use explicit imported renderers, with 24
  missing meshes restored from the read-only KR reference. Weapon stance changes
  refresh the current animation when changing from two weapons to one.
- Ordinary inventory refinement uses the native grade-to-texture lookup.
  The broader dull-icon investigation found a separate native contract gap:
  icons used world lighting instead of fixed inventory lighting. Native main
  and fill lights, white ambient and transparency cutoffs are now restored,
  including the path used by unrefined Glihtning Stone (3140). No replacement
  assets or extra glow layers were added. In-game visual confirmation is pending.

AUTOMATED TESTED: full `go test -count=1 ./...` and `go vet ./...` passed.
Client ArchitectureTests passed 1,209,240 checks and the incremental Release/x86
solution build passed. Earlier unchanged socket (231), crash capture (83),
launcher (56), manifest and shader gates passed in the same batch. Candidate
`tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
`F3E40F97FD9F3B47EFECE890CFB117E0486B93B76C5F99BB4E13134BE4DF8855`.
CLIENT-TESTED: pending for every changed visual/gameplay flow.
Research-record validation and repository layout/local links passed (200 indexed
documents). Batch-authored text was reviewed in English. Whole-tree
`git diff --check` still reports pre-existing trailing blank lines in
`TMFieldScene.h` and `data/NPCGener.txt`; those unrelated edits were preserved.

Implementation details: [client follow-up](client/port-748.md#local-instance-equipment-and-inventory-follow-up-2026-10-15)
and [server follow-up](server/operations.md#local-instance-and-ehre-follow-up-2026-10-15).

## Historical deployed snapshot

Verified snapshot: 2026-10-14, after the 23:42 UTC restart and patch 10024 publication. This is the continuation entry point, not a
promise of release readiness. Recheck live identity before a later deployment;
do not replay old commands or assume the working tree is the installed release.

## Source, deployment and client identity

The checkout is on `main` at `2f2381f9a8e77f23eb07bb10b326df76568d3bdb`, with
substantial uncommitted client, server, API, website and deployment changes.
Preserve them. The latest deployment is release `20261014T234237Z`; its receipts
below identify the installed artifacts, not a new source commit. The changes
remain uncommitted and client delivery uses explicit launcher patches.

| Surface | Verified identity | What this proves |
| --- | --- | --- |
| VPS current symlink | `/srv/wyd-go/releases/20261014T234237Z` | Mount, interface, equipment, tower and test-shop batch deployed; site advertises 10024 |
| Game process | PID `1482267`, started `2026-10-14 23:42:39 UTC` | Linux binary hash matches the packaged candidate |
| Player API process | PID `1482279`, started `2026-10-14 23:42:40 UTC` | Readiness 200 |
| PostgreSQL | WYD cluster on loopback `5433`, schema `8` | Current schema, not the initial v4/v5/v7 ledger entries |
| Public launcher feed | Ends at `10024 patch-10024.zip` | Latest follow-up batch, retaining all earlier patches |
| Installed local client | Not overwritten by this publication | Run the launcher to receive the new executable |

Artifact SHA-256 receipts:

| Artifact | SHA-256 |
| --- | --- |
| VPS game binary | `11CF8801358AC935DC69A05FB5D6E6E5E5252043ACFFC7B0C975436A07C8ECBD` |
| VPS player API binary | `30A9974502197B60984CC3870DFB0A147E56A4B3B233349C35791BB404D0CC15` |
| Built/published 10011 `WYD.exe` | `FDE1DF8F88CE7D1C7099BE044235F24E6C2C846F40143BD27CDF1AF2B6DC6D10` |
| `patch-10011.zip` | `47544012DAD39F08BAB466A458157F36309FEC8282A62290B148A41007AE7688` |
| Built/published 10012 `WYD.exe` | `F358A2711A2AF7C54EB044F06DFF5B9DDDF457ED14F3F4303D4D4D1B87152319` |
| `patch-10012.zip` | `E5E051264833A2AB232770B6178EE23BD2F5E453177653EB4D711CA157D2C3B9` |
| Built/published 10013 `WYD.exe` | `D8E8403CA724520A8C73FB53235DBB41FF357AE961C1C7452FA57D2FB800F19B` |
| `patch-10013.zip` | `686C033A260EAFE13C946F3BE378C8576BBAD1A5A8C48C057D077892E095693C` |
| Built/published 10014 `WYD.exe` | `96B3EAB114C00AA1A9B2B3D0A737B0E883B325BF47ECA74F160A78D97C49A630` |
| `patch-10014.zip` | `25DE3C3C75C0E7274463F07F3BFC2FF7F638A9F75F051F2CB8204CC67FA6E68F` |
| Built/published 10015 `WYD.exe` | `B5A197CA363ABFEE902C31D4D8A8E4608641350A32A3038C7CF963B177C51C22` |
| `patch-10015.zip` | `72CF50DAB1EA3E282AB305643B804E477E7CED1BA4CCEB7CD7078DD3E0301705` |
| Built/published 10016 `WYD.exe` | `F088B4486F5B838A86259F0112255F42E39B6726D3ECB2F53E9C207EA311C358` |
| `patch-10016.zip` | `07E79E677FCD4C9DE3DEB2CEE8B1CBC0AB7AAB1325B06DF475B3C2A2C6D9D733` |
| Built/published 10017 `WYD.exe` | `7B002D014C54F0EA42195162503CC174C03A3D9A24478531ACCABA0F9A42C395` |
| `patch-10017.zip` | `8EB7AF54943E7581A8310861B2DDAB49E1BBC8F4DFF1DEAFE97EA68444BDA576` |
| Built/published 10018 `WYD.exe` | `329D725EF4FBF435A3C3ECD3410CE3D04EA124F2FFF76DD906B386F3AB70C43C` |
| `patch-10018.zip` | `BA739C5E981B8AC4353A89E9FCB766F203087DDA00672A1D26A1418A0DA71B67` |
| Built/published 10019 `WYD.exe` | `3BDD1F7E340AB7AD541485C526CF0AF10635179BC856A5F8349106E88AC3B706` |
| `patch-10019.zip` | `C290EAA6B6311D8240B0C5B396C3F488F5109324A06D90938FC94463EABC56CA` |
| Built/published 10020 `WYD.exe` | `E291656AF11620EA1535C25F9381D19EF53A220E74ECDBB4B99650C1613428E6` |
| `patch-10020.zip` | `AEF03DE00BA5E05127CEF380BD562E7CAD1D0CE4BB3E5E4D0EC0715ABE372784` |
| Built/published 10023 `WYD.exe` | `18B87EE8B32F6333BD0838C46E0BED85F0FD2B172A7C2AF61804EED054ECC418` |
| `patch-10023.zip` | `E7053B354487ABA9DE959D73BAC21AEDC8196EA31A8D532950D7174B89167340` |
| Built/published 10024 `WYD.exe` | `DD357D0EACD58882FAF872FCE645BDF9A93B196C6853DAD8D1521DF1A15E76F6` |
| `patch-10024.zip` | `24F3AFBEA36E1E8754D6520E69EE8B1468B007B9DFFF095ECD882C1D981D2E09` |

Patch 10006 contains `WYD.exe` and `ItemList.bin`; 10008 updates the attribute map;
10011, 10012, 10019, 10020, 10023 and 10024 contain only `WYD.exe`. Earlier patches remain available and the website
advertises 10024. The full external client download still has no verified size/hash
receipt and must not be described as a certified complete package.

Deployment history, rollback paths and service configuration remain in
[public launch](server/public-launch.md) and [launcher patches](client/launcher.md).

## Published follow-up batch — 2026-10-14, patch 10024

This working-tree follow-up is now deployed; older receipts below remain historical.
Ordinary living adult mount magic now enters MagicAmp before the shared /4
conversion, instead of adding flat displayed damage. Spells and the displayed
score receive the bonus; timed premium mount magic also reaches spells.
All 30 catalog types at levels 1..120, score encoding, repeated recalculation,
death/unequip and premium expiry were tested. Full Go tests and vet passed
before the subsequent test-shop data-only adjustment.

The client batch includes shared inventory/equipment mesh fitting, native
mount glow textures, native green player trumpet chat separated from server
top notices, party Leave and Escape handling, and Mystic Amulet slot 9.
The server also permits equipped +9 earrings to reach +10 with normal Lac
(16% success, existing non-destructive failure), suppresses event-owned
Noatum tower generators and corrects the Erion tower anchor. See the existing
research records for evidence, persistence rejection coverage and limitations.
Release/Win32 Build-Client.ps1 -NoDeploy passed: 1,209,006 architecture checks,
231 socket, 83 crash capture and 56 launcher checks. No client was launched
and the installed runtime was not overwritten; this is not CLIENT_TESTED.

The existing Armia shops now offer 49 unrefined mortal D/E weapon blueprints
with EF_MAGIC 60/+28 for magic-bearing weapons or EF_DAMAGE 2/+63 otherwise:
ShopWeaponsD at (2114,2106), ShopWpnD2 at (2118,2106), and ShopWpnE2 at
(2120,2106). The separate Thron/Arch shop is unchanged. No new item IDs,
prices, purchase protocol or generators were needed. Classification:
MODERNIZACAO_COMPATIVEL, server test-stock configuration. Focused data tests
passed for exact effects, stock counts, catalog coverage and loadable Armia
generators. Existing authoritative purchase and UID allocation are retained.

The owner explicitly authorized publication and disconnecting players. Shutdown
saved three accounts with zero failures. Release `20261014T234237Z` restarted
both services; public readiness and News returned 200, with no warning-level
game/player journal entries after startup. Database backup
`/var/backups/wyd-go/wydgo-20261014T234237Z.dump` is 87,197 bytes; the installer
validated its restore listing. Release `20261014T223717Z` remains for rollback.
The release archive is 21,846,033 bytes, SHA-256
`e76bf8343a412c461682e09b9747e1bf3a47fb7421daa7751acbb9fba4781264`.

Patch 10024 contains only the previously validated executable, packaged directly
from the uncommitted working-tree build (not Git revisions), without rebuilding
or overwriting the local runtime. ZIP size is 824,185 bytes. Remote/public ZIP,
embedded executable, feed and website manifest hashes match the table above.
The portal was rebuilt and all 60 tests passed; unchanged Go/client validation
was reused. A transient SSH timeout interrupted the final patch activation;
the existing uploaded staging was resumed without republishing earlier entries.
Both remote staging directories were removed by the installers. This is not
CLIENT_TESTED or global release-readiness certification. The item-drop stack
overflow remains unresolved, and the complete Noatum war is not implemented.

Local publication cleanup was rejected by the execution policy before execution.
The exact leftovers are `build/deploy/wyd-deploy-20261014-mounts-ui-10024/`,
`build/deploy/verify-10024/` and `build/patch/patch-10024.zip`, all in sanctioned
ignored build roots. No deletion bypass was attempted. Live/rollback artifacts,
incremental build outputs and player state were preserved.

### Acceptance queue after publication

Publication, automated checks and actual gameplay acceptance are separate states.
The following changes are deployed in release `20261014T234237Z` / patch 10024;
their in-game acceptance remains open. Use the launcher before testing the client.

| Request | Delivered behavior | Pending in-game acceptance |
| --- | --- | --- |
| Mount magic | Ordinary mount magic enters amplification; premium bonuses reach spells | Compare displayed score and spell damage mounted/unmounted; check expiry |
| Mount glow | Native texture bands and owner/observer appearance handling | Compare levels 1, 70 and 120 from a second player; native bands are not linear per-level brightness |
| Trumpet and server notices | Green player-name trumpet chat; server notices at the top | Check both paths with a second player and verify no SERVER identity on trumpets |
| Party and Escape | Authoritative Leave request; close-only button; menu dismissal | Leave a party, close without leaving, and dismiss system/player menus with Escape |
| Mystic Amulet | Compatible slot-9 equipment enabled and preserved | Equip, unequip and relog with each supported class |
| Equipment icons | Shared cell fitting for fairy, costume and mount meshes | Compare equipment, inventory and cursor at supported resolutions |
| Earring +10 | Equipped +9 earrings accept normal Lac at 16%; failure consumes powder but retains the earring | Check success/failure, effect retention and relog; ordinary +9 weapons still require Aylin |
| War towers | Noatum event generators suppressed; Erion anchor corrected to (2507,1873) | Check daily-city placement; this does not implement the Sunday Noatum war controller |
| D/E test weapons | 49 unrefined mortal weapons in three existing Armia shops, magic +28 or physical +63 | Purchase representative items and verify effects and unique identities |

Earlier Aylin preview/glow fixes remain published through patch 10023 and are
retained by 10024; four-result previews and gem-specific UI/world effects still
need client acceptance. The reported item-drop stack overflow remains an open
repair, not a completed fix. Preserve its dump and matching 10021 executable.
Kefra still uses the temporary 1 HP test configuration; restore 28,000 HP after
the test session. Do not infer that this publication closes the broader English
migration, full-client provenance, S2 recovery or release-readiness gates.

## Published pending batch before crash repair — 2026-10-14

At the owner's request, the already validated candidate was published as 10023
and the pending server package was activated as `20261014T223717Z`, before any
crash repair. Patch 10022 was already public and was retained unchanged. The
new patch contains the native Aylin combinations/refinement setter, temporary
preview clearing, Diamond +10 UI and Garnet glow follow-up, and refined imported
weapon variants. It does **not** fix the reported item-drop stack overflow.
The dump and installed 10021 executable were preserved; no game was launched.

The pending server/API binaries were reused byte-for-byte: armor-only dye,
channel chat routing, test shops and Lac +10 to +11 at 5%. Focused game tests,
including gem-variant preservation and actual Lac use success/failure, passed.
The portal was rebuilt for 10023 and all 60 tests passed. Unchanged client
evidence was reused: 1,208,967 architecture checks, 231 socket, 83 crash-capture,
56 launcher checks and the incremental Release build. This is automated/build
evidence, not `CLIENT_TESTED` or global release-readiness certification.

Both readiness endpoints and the public readiness URL returned 200; TCP 8281
listened, and the bounded game/player warning journal was empty. Backup
`/var/backups/wyd-go/wydgo-20261014T223717Z.dump` (89,759 bytes) was verified;
release `20261014T184657Z` remains for rollback. Public feed, manifest, ZIP and
embedded executable hashes matched. See the linked deployment receipts for
archive identity and the distinction between source, distribution and gameplay.

## Published Kefra passage activation repair — 2026-10-14

Classification: `MODERNIZACAO_COMPATIVEL`. The owner killed Kefra and saw the
mirror but could not use it. The live game journal records the ADMIN guild's
kill at `18:30:36 UTC`; subsequent local client diagnostics show
`teleport ignored x=2364 y=3924 attr=0x54 motion=1 index=-1`. The client prompt
table lacked this four-unit block. `TMGlobal.h` now registers it as Uxmal with
the existing 700 gold route price. Both attribute maps, the 16-byte `0x290`
request, destination and authoritative boss-defeat admission are unchanged.
See the [contract follow-up](protocol/kefra-portal.md#post-kill-activation-repair--2026-10-14).

The new regression reproduced the missing-entry failure before the fix and
passed afterward. It checks the actual table, unique entry, label/fee, movement
grid, all 16 mirror tiles in both real attribute maps, and the real route file.
Focused Go tests for closed/open passage, charging and weekly reset passed.
`Build-Client.ps1 -NoDeploy` passed: 1,208,652 architecture checks, 231 socket
checks, 83 crash-capture checks, 56 launcher checks, costume and shader gates,
and the incremental Release build. Existing nonfatal Winsock C4996 warnings
remain. This is `STATICALLY VERIFIED` and `AUTOMATED TESTED`, not `CLIENT_TESTED`.

Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, 1,870,848 bytes,
SHA-256 `E291656AF11620EA1535C25F9381D19EF53A220E74ECDBB4B99650C1613428E6`.
It is **published as patch 10020**, but the local runtime retains the published
10019 hash `3BDD1F7E340AB7AD541485C526CF0AF10635179BC856A5F8349106E88AC3B706`.
The initial repair did not write to the VPS or launch the game. Subsequent
owner-authorized publication deployed patch 10020 and server release
`20261014T184657Z`, including the 1 HP / global EXP follow-up below. Actual
confirmation and passage use remain pending on the updated executable.

At `18:46:57 UTC`, shutdown saved one connected account with zero failures.
The installer completed a fresh database backup before switching releases.
Game PID `1431886` and player API PID `1431898` started at `18:47:00 UTC`, with
zero automatic restarts; both readiness endpoints returned 200 and TCP 8281
listened. The bounded post-start game/player warning journal was empty.
PostgreSQL still reports schema 8 and the durable Kefra defeat from
`18:30:36.160110947 UTC`, guild 1; no defeat reset or forced spawn occurred.

The 21,832,354-byte release archive SHA-256 is
`fb3bf7be00df580aa31bfab0835df0177398022f142858067d90ee9cae11c82b`.
Its runtime data/configuration comparison against the prior package found only
`data/npcs/Kefra.json` changed; the player API binary is identical. The game
binary incorporates the pending EXP pipeline change. Reused unchanged Go and
client test evidence is recorded here; Linux cross-build, portal build/check
(72 files, no diagnostics) and all 60 portal tests passed for this publication.
Fresh public ZIP and contained executable hashes matched; the feed ends at
10020, and the website manifest, home, download, News and changelog agree.
The previous release `20261014T181536Z` and earlier patch ZIPs remain available.
Remote release/patch staging was removed by the publication scripts. Source
remains the uncommitted working tree at the HEAD above, not a release tag.

Cleanup limitation: removal of this run's isolated crash-test evidence was
rejected by the execution policy. Only
`tmproject/build/tests/Release/crash-capture-tests-14516-348178203/` remains from
this run; it is inside the sanctioned ignored build root. No deletion bypass
was attempted; incremental build outputs and runtime state were preserved.

Publication cleanup was also rejected by the execution policy before execution.
This task's local leftovers are `build/patch/patch-10020.zip` and
`build/deploy/wyd-deploy-20261014-kefra-passage-10020/` (including its verification
download). They remain in sanctioned ignored build roots. No bypass was
attempted; the published VPS artifacts and rollback release are unaffected.

## Published combat, cape and Kefra batch — 2026-10-14

Subsequent Kefra adjustment, now deployed by release `20261014T184657Z`
(`MODERNIZACAO_COMPATIVEL`, server-only
rules and data with no new wire/client boundary): the owner requested **1 HP for testing**
and confirmed **+100% combat EXP for everyone while Kefra is defeated**.
`data/npcs/Kefra.json` now sets max/current HP to 1 (restore both to **28000**
after QA). The EXP pipeline doubles each receiver's combat reward using the
durable world defeat flag; quests are unchanged. Tuesday 12:00 in the war
timezone and the winning-guild broadcast already existed. Focused tests cover
the real dataset/generator/attack death, distant-player guild notice, global
and party EXP, stacking, saturation, persistence rejection, restart and weekly
reset, including critical-owner acknowledgement/rejection/unknown outcomes.
`go test -count=1 ./internal/game` and `go vet ./internal/game` passed; repository
layout/local links and `git diff --check` passed. Actual in-game validation is
pending. See [operations](server/operations.md) for HP restoration. The following
paragraphs retain the earlier 10019 deployment receipt, not the current identity.

The owner subsequently authorized publication and restart. Patch 10019 and
release `20261014T181536Z` are live; the installed local
`tmproject/client748/WYD.exe` was not overwritten. The source remains an
uncommitted working tree at the HEAD listed above, not a committed release tag.
Two connected accounts were saved with zero failures before the authorized
restart. A fresh backup succeeded; isolated PostgreSQL integration and backup
restoration passed (seven restored accounts). Game/API readiness returned 200,
the bounded post-start warning journal was empty, and the public patch ZIP,
contained executable, launcher feed and website manifest all matched. The
previous release `20261014T162737Z` remains available. Release archive SHA-256:
`7887e3da95696b505e412a30be18c4cedb1ec1cd44352fc7a7e92338daf108ae`.
The portal build and all 60 frontend tests passed after updating its receipt
assertions for 10019. Remote staging was removed by the publication scripts.

- **Attack speed and cadence:** `MODERNIZACAO_COMPATIVEL`. The server now sums
  DEX, equipment and affect speed as percentage-point bonuses, then packs the
  high `AttackRun` nibble once. It no longer treats the character template's
  high nibble as an extra base speed. The implicit base is 100%; 50 DEX gives
  the reported 110% with no speed equipment. Native-compatible double-hit
  probability is clamped from displayed quantized speed minus 150: 110% has
  no chance, 160% has 10%, 200% has 50%, and 250% has 100%. This follows the
  existing `BASE_GetDoubleCritical` formula, not a new 200% minimum. Server
  tests exercise the fixed 900 ms action interval, forged client ticks,
  rejected actions without damage/clock consumption, and the full 1,024-step
  double-hit progression. A legitimate double hit still means two numbers
  within one action; no separate literal "2x" UI removal is claimed.
- **Kefra doorway:** `MODERNIZACAO_COMPATIVEL`. Open permanent gates restore
  their original masked floor rather than stamping height zero. This repairs
  the artificial step from -83 at the reported `(2350,3868)` doorway without
  changing map assets, mask geometry or temporary gates. See the
  [ground-object evidence](../.agents/research/client748/flows/ui/ground-item-state-contract.md#kefra-doorway-regression--2026-10-14).
- **Cape refinement:** `MODERNIZACAO_COMPATIVEL`. Full and compact appearance
  paths derive the glow from slot 15's refinement, not `TotalKill`; unequipping
  clears stale glow. Emblems and citizenship remain independent. See the
  [cape evidence](../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md#cape-refinement-glow-repair--2026-10-14).
- **Kefra hall mirror:** `EXTENSAO_COORDENADA` v1. The model and both effects
  follow the durable boss-defeated flag, default hidden. Entry restores the
  snapshot; committed death opens it and committed weekly reset closes it.
  Failed or unacknowledged writes publish no proposed state. The server's
  existing Uxmal passage admission remains unchanged. Both updated client and
  server are required; see the
  [versioned contract](protocol/kefra-portal.md).
- **Survivor and hall entry:** `MODERNIZACAO_COMPATIVEL`, with the owner's
  explicit 50-entry allowance rather than a native-parity claim. Survivor
  already exchanged Locked Seals; the direct `0x290` hall portal bypassed the
  counter and checked only gold. Every tile in its 4x4 activation block now
  requires an existing entry and spends one per successful visit. Only Survivor
  exchanges item 4127, granting 50 and consuming the first visit through the
  existing NPC teleport (49 remain); a spare seal cannot bypass an empty balance
  at the ground portal. Existing balances are preserved. Gold and the counter
  are saved atomically before moving/publishing, using the existing hometown
  login projection rather than persisting arbitrary world coordinates. Critical
  writes wait for acknowledgement; rejection leaves state unchanged, and an
  unknown outcome retains its containment fence. Relogin restores the balance
  by character UID, not account name. See [operations](server/operations.md).

Validation: `STATICALLY VERIFIED` and `AUTOMATED TESTED`, not `CLIENT_TESTED`.
The integrated `go test -count=1 ./...` run passed all packages except a stale
web-catalog assertion that still expected 100 Survivor entries. After updating
that assertion to 50, the affected package passed its focused rerun; the other
package results remain valid. `go vet ./...` and the admission/recharge race
tests passed. Incremental client Release `-NoDeploy` passed with
1,208,581 architecture checks, 231 socket checks, 83 crash-capture checks,
56 launcher checks, the 143/129/176 costume gates and four skin-shader variants.
After moving the portal frames into the shared Go/C++ fixture corpus, the
affected architecture target was rebuilt and passed 1,208,582 checks. The Go
suite above includes the encrypted transport/fixture roundtrip. Existing
Winsock C4996 warnings remain nonfatal.

Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, 1,870,848 bytes,
SHA-256 `3BDD1F7E340AB7AD541485C526CF0AF10635179BC856A5F8349106E88AC3B706`.
This build artifact is now distributed through the hash-verified launcher patch,
not a committed source release. Windows
prevents the previously prohibited visual testing: real-client movement,
cape brightness, living/dead/reset portal acceptance and the complete 50-entry
Survivor cycle remain pending in game. Do not describe this published batch or the
global English migration as release-ready.

Cleanup limitation: the environment rejected removal of this task's temporary
test outputs. The ignored `build/combat-cape-kefra-validation/` log and the
`tmproject/build/tests/Release/crash-capture-tests-8408-343197109/` and
`crash-capture-tests-13532-344302656/` test dumps remain. Incremental build
outputs and installed/runtime state were preserved.
The publication cleanup was also rejected by the execution policy: this task's
`build/deploy/wyd-deploy-20261014-combat-capes-kefra-10019/` and
`build/patch/patch-10019.zip` remain locally in sanctioned ignored build roots.
Do not republish that version or bypass the deletion restriction. VPS temporary
staging was successfully removed by the deployment scripts.

## Delivered behavior and boundaries

- S2 producers are enabled in production. Confirmed account-local continuation
  failures can disconnect and reload durable state instead of leaving a false
  permanent online lock. Truly unknown outcomes still require containment;
  restart-safe reconciliation is not delivered. See the
  [recovery acceptance queue](server/critical-persistence-plan.md#current-gaps-and-acceptance-criteria).
- Successful game authentication clears the legacy ten-minute waiting-for-packet
  deadline. The configuration key now limits the absolute pre-auth lifetime.
  Handshake, incomplete-frame, flood and queue protections remain active.
  Actual authenticated inactivity beyond ten minutes and resumed movement are
  still a manual game gate.
- Patch 10004 contains exclusive rain/snow selection; 10005 contains the
  persistent buff corrections, including Skill Amp scale `1.5` and ground
  offset `0.4`. Full composites, mounted/hidden/dead lifecycle and runtime
  particle performance remain manual validation work. The
  [buff evidence record](../.agents/research/client748/flows/combat/persistent-buff-visuals.md)
  separates proven constants from open gaps.
- The public website has account registration/login, private saved character
  and storage views, progression advice, ranking, guild profiles and downloads.
  The reconciled [website feature matrix](server/website-feature-status.md)
  supersedes old plan sections that still describe these as absent.
- Play and Database were removed from primary navigation, not deleted as
  addressable guide/catalogue routes. PINs are six ASCII digits, including
  leading zeroes. The game remains authoritative; website reads describe saved
  state and cache time, not guaranteed live presence.
- Published in patch 10006 and release `20261013T202540Z`: costume and KR mount icons, the KR mount class mask and
  the mount speed tooltip were corrected; LavaGolem and LavaGrub use the Aquagolem
  and Purple_Worm looks because their KR meshes are unavailable. See [imported costume and mount presentation](client/port-748.md#imported-costume-and-mount-presentation-2026-10-13).
- Published in release `20261013T223204Z`: critical instance entry reserved 16x16 spatial cells
  instead of exact tiles, so Nightmare first waves (101/81 monsters in radius
  55) failed with "The instance monster area is blocked." Reservations now use
  exact tiles; `TestNightmareSoloOpensWithInstalledData` covers ticket 3390
  with installed terrain and monsters.
- Published in release `20261013T223204Z`: every timed public-map stay now shows the 0x3A1
  countdown on the per-second guard. The Game Room (`clearAreas` in
  `data/quest_zones.json`, rectangle 3570..3965 x 3446..3711) joins the shared
  ten-minute reset and returns its occupants to town, following the W2PP
  `ClearArea` reference rather than recorded native evidence; Hidden Valley
  shows the remaining ticket access. Ticket instances and Uxmal already sent
  their timers.
- Published in patch 10007: the client minimap restores the native -45 degree
  orientation by rotating its texture inside the frame.
- [Hidden Valley ticket-only access](server/hidden-valley-access.md) is now
  implemented and tested locally. It was deployed in release `20261013T202540Z`; in-game validation is pending.
  The installed terrain exists, but it has zero enabled starting monster
  generators inside the protected area. Access does not certify dungeon content.

## Security and operations

Live inspection found game, player API, Caddy and Fail2ban active. Public
listeners are SSH `22`, HTTP `80`, HTTPS `443` and game `8281`; PostgreSQL
`5433`, staff HTTP `8082`, player HTTP `8083` and Caddy administration `2019`
are loopback-only. Readiness requires the configured Host header:

```sh
curl -H 'Host: wyd.forjago.com' http://127.0.0.1:8083/readyz
```

Registration, player login and staff login use fail-closed Turnstile checks;
database-backed registration/login quotas survive process restarts. Staff
sessions require authorization/capabilities and CSRF, not a PIN-only bypass.
No secrets belong in documentation, client packages, commits or diagnostic output.

The SSH jail is scoped to TCP 22 rather than all ports. The previously verified
Hetzner firewall allows the public service ports and ICMP; deletion/rebuild
protection is enabled. This snapshot does not claim a fresh console audit.
Turnstile does not establish that Cloudflare proxies HTTP or protects game TCP,
and none of these controls proves DDoS immunity.

The game/player warning journal had no entries in the inspected
`18:10:20..19:09 UTC` window. This is bounded log evidence, not an all-time
absence-of-error guarantee. The daily backup timer is active around `04:15 UTC`;
off-VPS retention, restore drills and failure alerts remain unverified.

## Validation and next work

### Active client crash-correction queue — 2026-10-18

Follow the [updated correction plan](client/port-748.md#correction-plan-update-2026-10-18)
before resuming crash work. It reconciles the supplied October 18 audit of
committed `de87cd8` with the later local safety batch: minimap copying and
packet/text boundaries first, device/UI recovery and actor/effect lifetime
next, then remaining loader/UI bounds and measured effect pressure. Existing
guards and external crash collection are already implemented locally. The
continuation has now corrected bounded minimap copies/lock cleanup, legacy
packet/text boundaries, `SText` caller mutation and UI draw-list cleanup after
aborted frames. The Release `-NoDeploy` gate passed 3,263,747 architecture,
231 socket, 198 capture and 56 launcher checks, plus costume/shader checks.
The second continuation serializes modal Winsock reentry, clears nested effect
owners on final deletion, scopes ground-item removal to item lookup and protects
local-field actor retirement. Its focused gates passed 3,273,759 architecture
and 281 production socket checks. The prior Release EXE/PDB is preserved for the
user's gameplay test; separate Debug integration compilation passed without
deployment. Its exact EXE/PDB hashes and unchanged Release/runtime identities
are recorded in the plan.
The third continuation bounds `CMesh` palette/payload allocation, validates
positions and indices, rolls back failed GPU uploads and rejects incomplete
bone bindings. Its focused gates passed 3,281,408 architecture checks plus static
assertions and accepted all 1,596 current skin-mesh files. Separate Debug
integration compilation passed; its new EXE/PDB hashes supersede the earlier
Debug receipt and are recorded in the plan. Release and installed runtime
identities remain unchanged. This is not visual/gameplay acceptance or closure
of device recovery and borrowed-resource lifetime work.
The plan records candidate identities and remaining COM failure injection,
application teardown/modal lifetime, resource recovery, scene/actor lifecycle
and P2 coverage gaps. Historical Mana Control, item-drop and heap incidents remain
open. No client installation, active VPS/launcher release or gameplay acceptance
was changed by this continuation.

### Current cape-symbol continuation

The user reported that patch 10011 still showed a white/missing cape, no kingdom,
no guild name, and Foema mastery requirements labelled Magic Sword. Do not treat
the preceding publication receipt as visual acceptance. The client source now
registers missing native citizen-mantle textures, normalizes emblem variants for
the kingdom icon, resolves guild names, and uses class-aware skill requirements.
Incremental Release build passed: 1,205,501 architecture checks, 231 socket
checks, 56 launcher checks, and costume/shader gates. Candidate SHA-256:
`F358A2711A2AF7C54EB044F06DFF5B9DDDF457ED14F3F4303D4D4D1B87152319`.
This candidate is published through launcher patch 10012. Local runtime is not
overwritten and it is not `CLIENT_TESTED`. Fresh ZIP/executable hashes and plain
HTTP launcher endpoints passed. Website build and two public-content tests
passed; only home/download/changelog/JSON receipt were published, retaining
shared assets and previous pages. No game/API restart occurred; PIDs and start
times remain as recorded above. See the [10012 ledger](client/launcher.md#published-patch-10012-2026-10-13-americasao_paulo).

Random symbols 1..10 are persisted separately as `CapeEmblem`; real citizenship
and channel-based gameplay remain unchanged. The first kingdom cape grant and
legacy entry assign only missing/invalid symbols. Critical rejection publishes
nothing; promotion, re-equipping, kingdom changes and reload retain the symbol.
Full Go suite and vet passed; focused race tests cover model/store/game,
including JSON-store reload and independent Kibita citizenship publication.

The requested offline VPS backfill assigned two characters across two accounts
(symbols 5 and 6), preserving neutral capes and existing valid assignments.
Backup: `wydgo-20261014T020944Z.dump`. A second transaction changed zero accounts
and characters; a live query found zero eligible capes without a valid symbol.
Both readiness endpoints passed; WYD and Forjago returned HTTP 200. No warning
entries appeared in the bounded post-restart journal. See the
[contract and remaining visual gates](../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md).
Both new binaries must remain installed: old strict JSON readers reject the new
field. Do not downgrade only the executable after this backfill.

### Latest published continuation: Arch and citizenship

The [citizenship/cape contract](../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md)
now covers own-character face metadata, all selection slots, observer snapshots
and committed Kibita refreshes. New Arch characters have no starting equipment
except their face; visible inventory is cleared and the final four reserved
slots are preserved. Tests cover all four classes and critical-write rejection.
These changes are deployed in release `20261014T011828Z` and client patch 10011.

Full Go suite, vet, focused race tests and an incremental client Release build
passed. Client checks: 1,200,858 architecture, 231 socket, 56 launcher. The new
candidate hash is `FDE1DF8F88CE7D1C7099BE044235F24E6C2C846F40143BD27CDF1AF2B6DC6D10`.
`Build-Client.ps1 -NoDeploy` left the installed runtime untouched. Visual
owner/observer cape checks, selection/relogin and Arch creation remain pending.
This publication rechecked the live services, listeners, readiness, public feed,
downloaded ZIP/executable hashes and website receipt. No warning-priority game/API
journal entries or persistence failures were observed after the restart.
The portal check had zero errors/warnings/hints across 72 files; build and all
60 frontend tests passed. The dedicated player PostgreSQL integration gate and
backup restore into a disposable database passed; this does not substitute for
the pending in-game Arch/citizenship flow or a store-specific S2 database gate.
Backup `wydgo-20261014T011828Z.dump` and previous release
`/srv/wyd-go/releases/20261014T003714Z` remain available for recovery.

Reused unchanged client candidate evidence: 1,200,844 architecture checks,
231 socket checks, 56 launcher checks, 135 costume definitions and four skin
shader programs. Static publication checks do not prove launcher extraction or
in-game visuals. Previous website build evidence covers 37 pages and two
public-content regressions; it is not a new validation of every private UI.

For the current local server batch, `go test -count=1 ./...`, `go vet ./...`,
focused Hidden Valley/timed-access/S2 tests under the race detector and native
server compilation passed. The isolated store PostgreSQL URL was available;
dedicated player-role and staff read-only database fixtures were not configured
for this run. Those conditional skips do not prove their paths.

Priorities for continuation:

1. Test the deployed Hidden Valley client portal and expiry flow.
2. Define validated Valley monster/drop content and resolve the `3910` display
   name versus duration discrepancy without silently changing persisted behavior.
3. Complete manual idle, weather, buff, launcher and nonempty guild profile gates.
4. Close S2 unknown-outcome recovery, full-client package provenance and backup
   restore/alerting acceptance.
5. Continue the [English migration](language-migration.md). Known legacy text
   remains; a global release must not be labelled ready on this snapshot.

The authorized publication restarted the game and player API only after zero
established game connections; Caddy was reloaded. No Git commit/push, historical
binary modification, local runtime install or client visual execution occurred.
