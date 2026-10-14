# Current project state

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
