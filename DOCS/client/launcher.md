# WYD launcher and client patches

## Published patch 7.48.009 (2026-10-27)

The revision-8 transport gate rejected the 160-byte categorized catalog before
Premium Nell could open. Revision **9** delegates validation to the existing
versioned parser. The regression failed at three dispatcher assertions before
the correction and passes after it. Wire formats, catalog data, purchases and
EN/PT shop controls are unchanged (`MODERNIZACAO_COMPATIVEL`).

The Release build and client gates passed (3,177,017 architecture checks,
281 socket checks, 200 crash checks and 58 launcher checks). The Go wire package
passes with identity **0x02EC0009** and minimum revision **9**. Portal check
reports zero errors/warnings/hints; all 91 portal tests pass and the build
contains 3,299 pages. The new release article includes English and Portuguese.

- Feed: **48009**, mandatory update; sole ZIP entry `WYD.exe`.
- ZIP: **947,437 bytes**, SHA-256
  `7A257F2E46A2D83D8A29B09C8C0B24897BA92E1A42ABA62D17C656685305B07D`.
- Executable: **2,071,552 bytes**, SHA-256
  `00688A969E0C73CDBF23229E0B78A34CC0142A91755A095A7051F80593759A07`.
- Server/site archive: **25,953,043 bytes**, SHA-256
  `BCDE199881EB2FC1DC343A47D06BAFAFC0D6BF1F08D3C8F300E8CC613CB111F0`.

All 7,797 installed payload files and both running executable hashes match the
package. All 1,157 runtime data files and eleven private settings were preserved.
After public HTTP/HTTPS patch verification, the owner authorized disconnecting
three game connections and restarting the game/API. Release
`/srv/wyd-go/releases/20261027T031959Z` is active; the game/API started at
03:20:02/03 UTC on 2026-10-27. Three accounts were saved with zero failures and
the backup succeeded. Both readiness checks return 200; both channels reject
revision 8 and admit revision 9 to credential validation using nonexistent
account probes. These probes do not authenticate a real player. The log reader
retains its prior process; automatic restart counts remain zero.

The public manifest, homepage, download, changelog, News and release article
advertise 7.48.009. Their HTTP URLs redirect to HTTPS; final responses are 200.
The launcher feed and ZIP return 200 directly over HTTP and HTTPS. All 3,214
recorded source inputs remained unchanged after activation. The existing
thirteen disabled population groups and 169 unresolved staff references persist.
Rollback is `/srv/wyd-go/releases/20261026T235538Z`. No VPS reboot was performed.
Source provenance, public verification, active release and cleanup are recorded
in `.agents/research/release-748009.json`. Real embedded-client opening, closing
and purchases remain pending.

## Published patch 7.48.008 (2026-10-26)

The official-style embedded Premium Nell and catalog categories are published
and verified over public HTTP and HTTPS. The owner explicitly authorized
disconnection and game/API restart. After patch verification, the server
activated `/srv/wyd-go/releases/20261026T235538Z` at 23:55:41 UTC. This coordinated
release uses feed **48008**, compiled identity **0x02EC0008** and server minimum
revision **8**; the old client cannot consume the v4 categorized catalog.

- ZIP: **947,181 bytes**, SHA-256
  `CA2980EBF0D9F9F5A277510A1CD71CA25B539E7A3010F56C2F7D5B27F656EA69`.
- Sole entry `WYD.exe`: **2,071,040 bytes**, SHA-256
  `15C41E8A790C5BEDE9F8EB3F126E67F016556E86405C023F579F41AAF17AD4B2`.
- Server/site archive: **25,951,944 bytes**, SHA-256
  `78427F6432C59D041136FFFAE78CD183C9555F5D5DDFE81C3AA790E1132B61D5`.
  All **7,795** installed files and both running executables match the package.
  All **1,157** runtime data files and eleven private configuration files were
  preserved. Rollback: `/srv/wyd-go/releases/20261026T222350Z`.

Backup succeeded; shutdown persisted three accounts with zero failures. Both
readiness endpoints and public TCP channels passed, with zero automatic
restarts. The log reader retained its existing process. Both channels reject
revision 7 with the update requirement and admit revision 8 to credential
validation in synthetic nonexistent-account probes. These probes do not log
in a real player. The public site, manifest, News, download page and changelog
advertise 7.48.008 over HTTP and HTTPS. The pre-existing thirteen disabled
population groups and 169 unresolved staff content references remain unchanged.

The canonical client gates/build, focused Go wire tests, nine page tests,
portal check/build and all 91 portal tests passed. The page and category
controls support EN/PT; the surrounding pre-existing staff editor remains
English-only. Actual embedded display, focus, close/reopen and purchases remain
pending client acceptance. The local runtime executable was not replaced.
Source inputs, hashes and verification are recorded in
`.agents/research/release-748008.json`. The 3,213 recorded source inputs were
verified unchanged after activation. Cleanup status and retained paths are
recorded in that receipt.

## Published patch 7.48.007 (2026-10-26)

The mandatory Premium Nell host correction was published and verified over
public HTTP and HTTPS before activating
`/srv/wyd-go/releases/20261026T222350Z` at 22:23:53 UTC. Feed **48007**,
compiled identity **0x02EC0007**, server minimum revision **7**. The site,
manifest and release article advertise the same version.

- ZIP: **893,260 bytes**, SHA-256
  `FF11AA275B35183EB853E05808153002FECA76B9923B60567C0CC461EB5DD292`.
- Sole entry `WYD.exe`: **2,010,112 bytes**, SHA-256
  `B4E5540A5F509983716CCC66680E887837729C0808AE78D667DCBE7200F19CE6`.
- Server/site archive: **25,949,475 bytes**, SHA-256
  `AC1956BD65727EC29A74563ED66C15D663747472BC7E6EFB062CEB4F560343FB`.
  All **7,793** installed files matched, including **1,157** runtime data files.

The owner authorized disconnecting the three active game connections and
restarting the game/API. Backup succeeded; shutdown persisted three accounts
with zero failures. Both readiness endpoints and TCP channels passed, private
configuration was preserved and automatic restart counts remained zero. The
log reader was not restarted. Rollback: `20261026T195612Z`.

Synthetic nonexistent-account probes on both channels reject revision 6 with
the update requirement and admit revision 7 to credential validation. These
probes do not authenticate a player or establish gameplay acceptance.

The canonical client gates/build, focused Go wire tests, six page tests and
portal check/build plus 89 tests passed. An isolated corrected WebView host
regression was blocked before execution by automatic approval review
(`blocked by policy`, without a detailed reason). Opening/closing Premium Nell,
real purchases, focus behavior and both locales remain pending in the updated
game. See [the correction](../protocol/premium-shop.md#host-navigation-correction--client-748007).
Exact inputs, verification and cleanup are recorded in
`.agents/research/release-748007.json`. The local runtime was not replaced.
Remote temporary files were removed. Automatic approval review blocked local
cleanup before execution; the receipt lists the three retained staging paths.
The repository layout check reports preexisting runtime logs, dumps and the
local client ZIP outside build roots; unrelated artifacts were preserved.

## Published patch 7.48.006 (2026-10-26)

The mandatory patch was published and verified over public HTTP and HTTPS
before activating `/srv/wyd-go/releases/20261026T195612Z` at 19:56:15 UTC.
Feed **48006**, compiled identity **0x02EC0006**, server minimum revision **6**.
The site, manifest and release article advertise the same version.

- ZIP: **892,013 bytes**, SHA-256
  `E39B127D23CF2FD4A9E20BC1D64040E73BB533F7DE42DD94F4E1D04AE9E3337A`.
- Sole entry `WYD.exe`: **2,008,576 bytes**, SHA-256
  `6DAB02915D38F7E25DA4A072FE182B9265CF50E1E22EFA097FF282C02BEB239F`.
  The page is embedded and the x86 WebView2 loader is linked statically.
  Eight dependencies from the preceding patch were verified unchanged.
- Server/site archive: **25,948,478 bytes**, SHA-256
  `3E325B84B8359A8D8FC199AE640F06FF3E0F500A61A61A37B2334557A3A140F8`.
  All **7,791** installed files matched, including **1,157** runtime data files.

The owner authorized the game/API restart. Both readiness endpoints and game
ports passed; private configuration was preserved. Backup succeeded and the
services had zero automatic restarts. The log reader was not restarted.
Rollback: `/srv/wyd-go/releases/20261026T064741Z`.

Canonical client gates/build, the full fresh Go suite, vet, focused race tests,
six page tests, portal check/build and all 89 portal tests passed. The local
runtime executable later matched the published hash and revision-6 player login
was observed. These checks do not establish successful Premium Nell gameplay:
the owner reported web startup failure despite an installed runtime. An isolated
Win32 reproduction confirmed that the host cancels its own embedded navigation;
see [the host diagnosis](../protocol/premium-shop.md#published-v3-host-diagnosis-2026-10-26).
No correction of that defect is included in this published patch.

Exact inputs, publication checks, recovered SSH timeout and diagnostic evidence
are in `.agents/research/release-748006.json`.
Remote staging was removed. Automatic approval review rejected local cleanup
before execution (`blocked by policy`); no alternate deletion was attempted.
The receipt lists the retained staging, verification and diagnostic paths.

## Published patch 7.48.005 (2026-10-26)

The owner explicitly authorized publication and immediate game/Player API
restart. The patch was published and verified over public HTTP and HTTPS
before server activation. The site, manifest and launcher now advertise
7.48.005. The durable receipt is `.agents/research/release-748005.json`.

The mandatory client uses feed **48005**, compiled identity
**0x02EC0005** and minimum server revision **5**. The ZIP has nine entries:
`WYD.exe`, `serverlist.bin`, `sn.bin`, and base/EN/PT copies of `UI/command.txt`
and `UI/strdef.bin`. Channel endpoints are unchanged. The local runtime
executable was not replaced.

- Client ZIP: **881,705 bytes**, SHA-256
  `2D3665651A8C085988852D25A54288A5B049F282A4D0A081A80DB01815BCA7BB`.
- Client executable: SHA-256
  `B0ADA8F60BEA7A82A3F3D1F86810959A0FBF2A3685D93A5C28A1F9CEC5B3C476`.
- Server/site archive: **25,823,040 bytes**, **14,388 entries**, SHA-256
  `0d478056de94d2183014293e583e6f8ac28bc39279bb54df0af6f3eac7e08436`.
- Linux game executable: SHA-256
  `994EDFC21A6E4EEA1EE04910ED13E6962F0729B3DF79A8FEF569234F9D272397`.

Remote inspection verified all **1,157** installed runtime data files and the
running executables against the packaged inputs. Public HTTP/HTTPS downloads
matched the ZIP and all nine entry hashes. The runtime data include the owner's
NPCGener cleanup and the three Perzen services. Live configuration, units and
private authentication environments retained their previous hashes.

The atomic updater activated `/srv/wyd-go/releases/20261026T061812Z` at
06:18 UTC (03:18 America/Sao_Paulo). Game PID **1635034** and Player API PID
**1635046** use the expected new executables. The unchanged log reader retained
PID **1304661**. Both readiness endpoints and TCP 8281/8282 passed; no automatic
restarts or new failure diagnostics appeared in the bounded post-start journal.
Backup succeeded before activation; shutdown reported zero accounts and zero
failures. Rollback release: `/srv/wyd-go/releases/20261026T051850Z`.

Synthetic login probes on both public game ports verified that revision 4
receives the update-required message and revision 5 reaches credential
validation. These used random nonexistent accounts and did not authenticate
a real player. The 20 disabled channel-2 source groups and 169 unresolved staff
index references match the previous startup; this release does not resolve
those existing content gaps.

The full Go suite, `go vet`, 19 locale tests, canonical client build/checks,
portal check/build and 89 portal tests passed. This is automated validation;
actual launcher installation, Gryphon travel and Perzen exchanges remain
pending. Neither automated checks nor protocol probes establish CLIENT_TESTED.

Automatic approval review rejected cleanup of the local staging web trees with
`blocked by policy`. No deletion was retried. The local deployment staging
remains preserved. Both remote staging directories were removed by their
successful publishers. The exact retained paths and the initial, subsequently
resolved portal-copy race are recorded in the receipt. An initial installed
inventory aggregate mismatch was a path-ordering difference in the verifier;
direct comparison found zero differing files.

A separate cleanup attempt for the new local verification files was also
rejected with `blocked by policy`; no alternative deletion was attempted.
Retained files are `build/release-748005/public-http.zip`, `public-https.zip`,
`installed-data-inventory.json` and `verification-inputs.json` in that same
directory. These local cleanup limits do not affect the verified live release.

## Published patch 7.48.004 (2026-10-25)

The patch was published and verified over public HTTP and HTTPS before matching
server activation. The owner explicitly superseded the earlier no-disconnection
instruction and authorized disconnecting the two active sockets. Shutdown
persisted two accounts with zero failures. Game and Player API were restarted;
the VPS itself was not rebooted. External access to channel 2 was verified
after the authorized provider-firewall change described below.

Feed **48004**, compiled identity **0x02EC0004**, minimum server revision **4**.
This is an `EXTENSAO_COORDENADA`: two listeners share the authoritative actor,
account reservation and durable state, with channel-local populations and
one-use migration tickets. Existing 7.48 selector/migration boundary evidence
is reused; native 7.59 population data does not establish 7.48 gameplay parity.
Specialized NPC services documented as incomplete remain incomplete.

Source HEAD `40706d99b7715167d7aa4a2c484704f7e91c2c8b` plus the working tree;
no commit or push. Client built through `tmproject/Build-Client.ps1 -NoDeploy`.
The local runtime executable and player settings are unchanged. Exact dirty
input hashes and archive receipts are recorded in
`.agents/research/channel2-release-748004.json`;
the patch uses the established explicit working-tree entry allowlist.

- ZIP: `build/patch/patch-48004.zip`, **879,243 bytes**, SHA-256
  `9C45AF6CB21874F87389976407D016D528BC6FDC8CA8F22F4DD52F54D691C329`.
- Exactly six entries: `WYD.exe`, `serverlist.bin`, `sn.bin`,
  `UI/strdef.bin`, `EN/UI/strdef.bin`, `PT/UI/strdef.bin`. Entry names, lengths
  and SHA-256 values were read back from the ZIP and matched the allowlist.
- `WYD.exe`: **1,939,456 bytes**, SHA-256
  `AB06F70B1CDAF6E52B7DA3A6F1156B56B277AD44C0B01669074DED92BE05FD1F`.
- `serverlist.bin`: **7,040 bytes**, SHA-256
  `D95A75FF82C8FBB8722C4B7DC0CFE40971D89045D55C28959D561C96BFEF4133`.
  First group uses `46.224.58.209:8281` and `46.224.58.209:8282`; the default
  8281 port is implicit in its encoded endpoint.
- `sn.bin`: **143 bytes**, SHA-256
  `2CBB82ED4D49BDFCEF1AD1E56B040E7BF709F2B772E393059A6A7F173CA1C165`.
  Regenerated and verified byte-identical to the existing WYD-Go/Featured
  group/order table. Channels occupy server-list slots, not separate SN groups.
- Base/EN messages: **57,860 bytes** each, SHA-256
  `8F642FF0E04D84A57CA92F1D6534BA0796BB2B8D271C14DFFBEE336B550E5777`.
  PT: **57,860 bytes**, SHA-256
  `7E9E17CD4ADC4F5207F0D29DD6A69EEC0162622BDAA72A19C99DAA5E34CDFBDF`.
  These include the updated NPC service messages; locale selection and English
  fallback are preserved. All 19 locale tests passed, including checksums,
  fixed IDs and printf placeholders. Website news remains English fallback;
  the portal does not yet provide complete Portuguese localization.

Server/site archive: **25,823,918 bytes**, **14,384 entries**, SHA-256
`6D2D1BB5F69260DFAB1131B89C548BBF2D1DD6AC0B4458BF15FE1C16C93CE12B`.
The explicit additional-data allowlist contains **409** untracked runtime
files, all confirmed in the archive. No account snapshots are packaged.
Linux game executable SHA-256:
`21023C507470096431F9FF7D42EB8D1F4D303FBCD924461D920799DFB090E06C`;
Player API: `2680EB1EE7AEE1981BB11B3864C686EEDDC04688C67FBB82D83AF3219E1D65C3`;
log reader: `16D8A9EA3E01B037EE846747CC4EFCF9853432BBD2A0E68D51BBD66904D2227D`.
Nine installed runtime configuration/unit files match the staged copies.
The staged `server.txt` preserves every live setting and changes only the five
channel keys. Private authentication environments are preserved.

Validation: full Go suite and `go vet ./...` passed; the release-line assertion
was updated from 7.48.003 and its wire tests rerun successfully. Client gates
passed **3,176,734** architecture checks, **281** socket checks, **200** crash
checks and **58** launcher checks. Portal check/build and **89** tests passed.
Stored database-fixture results were cached, not a new live integration run.
Actual launcher installation, transfers in both directions, NPC interactions
and visual confirmation remain pending. The layout validator still reports
pre-existing runtime logs/archive/dumps outside sanctioned output roots; those
unrelated files were preserved. An inventory update encountered a Windows
mapped-file lock; the read-only inventory check reported only those existing
artifacts. No blocked cleanup was retried.

Activation started **2026-10-25 20:38:53 UTC** (17:38:53 America/Sao_Paulo).
Active release: `/srv/wyd-go/releases/20261025T203853Z`; rollback reference:
`/srv/wyd-go/releases/20261024T210842Z`. The active game PID **1539385** and
Player API PID **1539402** resolve to the new release and their `/proc/.../exe`
SHA-256 values match the candidates above. Both readiness endpoints passed.
`game.env` and `player.env` hashes match the pre-activation values exactly.
At the first resource check the game used **152,903,680 bytes** and had zero
automatic restarts.

The live feed preserved 48001, 48002 and 48003 and appended 48004. Public HTTP
and HTTPS downloads matched the ZIP hash and all six entry hashes. The public
manifest, home, Download, changelog and News route `/news/client-7-48-004/`
advertise 7.48.004. Both remote `/tmp` staging directories were removed by the
established activation/publisher scripts.

Production loaded **727** channel-2 template bindings and **6,233** source
groups. Its initial world summary reported **5,918** generators and **8,989**
mobs after production event/reservation filtering; this is the observed VPS
count, not the isolated boot-test count. Channel 1 loaded separately. The
official generator and population JSON hashes match the local validated inputs.
The previously documented 21 invalid groups remain disabled, and the staff
content index reports 169 unresolved references; publication does not resolve
those content gaps or complete every specialized service.

Both TCP 8281 and 8282 listen in the same game process. UFW permits both ports
for IPv4 and IPv6. The initial external channel-2 probe timed out. With the
owner's authenticated Hetzner Console and explicit authorization, the applied
`forjago-public-services` firewall (11796824) on server 169421488,
`46.224.58.209`, was inspected: its five inbound rules allowed ICMP and TCP
22, 80, 443 and 8281. A sixth inbound rule now allows TCP 8282 from Any IPv4
and Any IPv6. The Console confirmed the rule was added and fully applied;
the five existing rules and unrestricted outbound policy were preserved.
No service restart was needed for this change. Public IPv4 channel-2 protocol
verification passed at **20:47:01 UTC**. IPv6 connectivity was not tested.

Production protocol probes at **20:41:10 UTC** (channel 1, public endpoint) and
**20:42:48 UTC** (channel 2, loopback on VPS) reject compiled revision 3 with
the update-required message and admit revision 4 to credential validation,
which rejects a random nonexistent account. The public channel-2 probe at
**20:47:01 UTC** confirmed the same behavior after the firewall change.
These probes authenticate no real
account and do not prove actual launcher use or successful gameplay transfers.
Durable input/archive/probe metadata is recorded in
`.agents/research/channel2-release-748004.json`.

Cleanup of this build's completed crash-test folder was rejected by automatic
approval review with `blocked by policy`:
`tmproject/build/tests/Release/crash-capture-tests-18580-85565468`.
It remains untouched; no alternate cleanup method was attempted.

After the successful provider-firewall verification, automatic execution review
also rejected the staging cleanup command before execution (`blocked by policy`).
The remaining paths are `build/deploy/wyd-deploy-20261025-748004/`,
`build/release-748004/{verify-http.zip,verify-https.zip,sn.bin,server.candidate.txt,server.pre.txt}`
and `C:/Users/xereca/AppData/Local/Temp/wyd-release-748004-probe.py`.
No alternate deletion was attempted. The firewall proof image is retained at
`build/release-748004/hetzner-channel2-firewall.jpg`.

## Published patch 7.48.003 (2026-10-24)

Feed **48003**, compiled identity **0x02EC0003**, minimum server revision **3**.
The mandatory patch corrects the raster-shrinking regression: only opted-in
Character/Skill captions receive original-size ellipsis on overflow. Other
labels and numeric fields preserve their previous rendering. This is
`MODERNIZACAO_COMPATIVEL`; existing native boundary evidence is reused.

Source HEAD `40706d99b7715167d7aa4a2c484704f7e91c2c8b` plus the explicitly
selected working-tree inputs below; no commit or push. Built through
`tmproject/Build-Client.ps1 -NoDeploy` into
`tmproject/build/TMProject748/Release/WYD.exe`. The ZIP uses the established
working-tree executable allowlist, rather than misrepresenting a committed
resource diff. No local runtime installation occurred.

- ZIP: **862,788 bytes**, SHA-256
  `492DC2A390DEA1EBFB93CE52FF9F5A579F6D896EE4D72223CBAFEACF82741CAC`.
- Only entry: `WYD.exe`, **1,938,944 bytes**, SHA-256
  `882F11E6CBA65E6B290DA0C8DF7920485982F5A9F1E49005D31823D76881BBE6`.
- No assets, launcher executable/DLL, account data, `config.txt`,
  `ClientSettings.txt` or `lang.txt` are included. Both language trees and
  shared assets retain the 7.48.001 baseline plus 48002 resource corrections.

Fresh HTTP and HTTPS downloads matched the ZIP and executable hashes before
server activation. Both public feeds preserved 48001 and 48002 and appended
`48003 patch-48003.zip`. The live manifest matches these hashes and sizes;
home, Download, changelog and the [announcement](https://wyd.forjago.com/news/client-7-48-003/)
returned 200 and advertised 7.48.003 after activation. The native launcher and
its language extension remain unchanged; full-client baseline remains 7.48.001.
The owner's multilingual policy now explicitly supports EN/PT product content
while keeping engineering instructions and documentation in English.

Server/site archive: **25,434,568 bytes**, SHA-256
`4A6463A663B8AF0B5AD0231E649AA68F63A73176C855F76E16EE5321FEDF7E6D`.
Its 13,971 entries were inspected; private state/settings are excluded.
Release `/srv/wyd-go/releases/20261024T210842Z` activated at 21:08:45 UTC.
Game and Player API were restarted; the unchanged log-reader process continues
from the previous release with the same executable hash as the new archive.
Both readiness checks passed, services are active and no error-priority
Game/API journal entries appeared. Configuration and private environment
identities were preserved. Backup/rollback and actual process hashes are in
[current state](../current-state.md#release-748003--2026-10-24-published-and-activated).
Remote deployment staging was removed; no host reboot occurred.

At 21:11:04 UTC, production probes rejected `0x02EC0002` with the update-required
message before authentication and admitted `0x02EC0003` to credential validation
with the expected invalid-credentials response for a random nonexistent account.
No real account or character was authenticated. Actual launcher installation,
game rendering, title usability and gameplay remain pending, not `CLIENT_TESTED`.
The installed local runtime remains 7.48.002 with SHA-256
`80CA48E77A8F83535401B7FE1415E33C9DE873E04F55C0313C844DF7B6A66209`.

Validation passed: 3,179,319 client architecture checks/static assertions,
281 socket, 200 crash and 58 launcher checks; full Go suite and vet; portal
check/build and all 89 tests. PostgreSQL fixture and race coverage were not
run. This does not establish complete server or website localization.
Repository layout validation still reports preserved pre-existing runtime
artifacts outside build roots, all excluded from the archive. Automatic
execution review rejected deletion of
`tmproject/build/tests/Release/crash-capture-tests-1516-9365828/`; no alternate
cleanup was attempted. The two Python environments lack PyYAML, so skill
validation could not execute; unchanged metadata and policy edits were
reviewed manually.

<details>
<summary>Selected dirty build inputs at packaging time</summary>

All other build inputs come from the stated HEAD. These selected inputs include
previously published changes needed to reproduce the current working tree;
the client ZIP itself contains only the executable above.

| Input | SHA-256 |
| --- | --- |
| `tmproject/client748/EN/UI/command.txt` | `FC07616C20FAF3E76CCB84CC6F06AA89255A1D89065DDD052B57B7FAD0E9C683` |
| `tmproject/client748/PT/UI/command.txt` | `722855F8D4A87EC7F7D6385C533ACD13F531CE6DA5A61606B7F3BD35C814D2BB` |
| `tmproject/client748/UI/command.txt` | `FC07616C20FAF3E76CCB84CC6F06AA89255A1D89065DDD052B57B7FAD0E9C683` |
| `tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp` | `D7C219AB851B8CF85B63104B9671C53B0210278600CEAD720B8BE431A54305C5` |
| `tmproject/TMProject748/internal/app/scenes/TMFieldSceneUI.cpp` | `36FBEA44200E5E4842327A1FD6353F07467593257E2A2523292DCCBFF4A52CC5` |
| `tmproject/TMProject748/internal/app/scenes/TMScene.h` | `592F97CA521EC72F2854C7C35BB6ED3163A3E68262CA31B854F60A5FC122E9EA` |
| `tmproject/TMProject748/internal/app/scenes/TMSceneResources.cpp` | `1768DAD4D1DB0A616E94EEED3BE1F664FA1E01DB4C7AA7160D5396596EC687C3` |
| `tmproject/TMProject748/internal/application/FieldChatSubmitPolicy.h` | `3751E4C5044E10AE05420AD18A368DC2FF90109FD859833C7BDCFF427CBDB455` |
| `tmproject/TMProject748/internal/core/QuestSubjectLevel.h` | `1A72F1002CBAA6346645B4E9518C1F83F34F5339C70CC3C9F1C7EE5F14BBB5A0` |
| `tmproject/TMProject748/internal/platform/windows/FontGdiResources.h` | `DADB506F2997DF7603928053432C6738CC256E62F44046BADE04F9AF5DE50179` |
| `tmproject/TMProject748/internal/platform/windows/ResourceTextGdi.h` | `1EAB18D5B8C066448E1BD7AFB82E007A1A92C7EBC21FF0F248C74D3C4DA6E7A3` |
| `tmproject/TMProject748/internal/platform/windows/SingleLineTextLayout.h` | `E75EF16CC6C92A95506A8D4FF1B48E28B447467E216B0087E61D56F5F5D6B437` |
| `tmproject/TMProject748/internal/platform/windows/TMFont2.cpp` | `BF362349E6949787438DCB14E09542B3C57E96E5DBC6DC388241334CC54BD098` |
| `tmproject/TMProject748/internal/platform/windows/TMFont2.h` | `75C85E204FE63734B2A9485CD13E71629DC9DB3415DB575F05750697A8FF7AA1` |
| `tmproject/TMProject748/internal/ui/controls/SText.cpp` | `D0DE78E99D2B2C6059EA17C5E0EE25A5230F1438D95E1FFF1BA4B10FBBFBE386` |
| `tmproject/TMProject748/internal/ui/FieldCaptionLayout.h` | `C4B44B74F177A8DABD4A02B113240A40E3AC5E61B3F3AD40443BC302134CCDC2` |
| `tmproject/TMProject748/internal/ui/SGridTooltip.cpp` | `52C5A8EA3AE9C2B9795C8651CC948BD5BC76991AA94384BAF269534C2799F2A5` |
| `tmproject/TMProject748/internal/wire/ClientBuildContract.h` | `0FE7F1A14115B89F70334D6FB01A60BE3F5FD9817EAF649C63C37F3AAA881F1B` |
| `tmproject/TMProject748/tests/ArchitectureTests.vcxproj` | `094A2B1DE8E5C36B36F9978AE7B55B5C2D26ECE5963E59F079B9A4073DDEFC77` |
| `tmproject/TMProject748/tests/LocaleBoundaryTests.cpp` | `E16355B62318695C38D4DCCBA358799A5D261F7585A5803A107C92BFE9765380` |
| `tmproject/TMProject748/tests/PacketViewTests.cpp` | `35F64DFFF1DD4F61C769E7BA457918412ECE714E7E6C96E1F31837E1E4D7AA86` |
| `tmproject/TMProject748/tests/SceneDisconnectContractTests.cpp` | `E60CE15C88729E13A8EA28362F2599F040C09D7EA58E5914FD0B8C8FB0A3EA3A` |
| `tmproject/TMProject748/tests/SingleLineTextLayoutTests.cpp` | `37944ABC3AF076168167F8AA42440435E032E5A6A6AE8B34F79E79E8043C6FD0` |
| `tmproject/TMProject748/TMProject748.vcxproj` | `F4DCD9DBFA0014588ECD6FDCE3AF403B66CB19C42C178590AFE924BA4FE38117` |
| `tools/client-assets/client_locale_overrides.json` | `886BC485E09133BD78AC57FC2605BB73B429EB8327458391E8154B663C2D6BFA` |
| `tools/client-assets/test_client_locales.py` | `6E02B9F9F08D5CF53B712FBEC71EF6B07E2AEF36BB8EDE6957BC14BB27FE27AA` |
| `wydgo748/internal/game/client_command_admission_test.go` | `21F24BE1168E0BF4B7EDBA39E709D79A75ABF1EBC36A7E4EBB383676734E4835` |
| `wydgo748/internal/game/client_command_admission.go` | `8AE306563C9B820BB3843E161905D7F6BA5193EFF38A615AD6DAF2FD20BE808A` |
| `wydgo748/internal/game/colosseum_combat_test.go` | `AE4AB47D71DAF677FBE62D9C6BE2530850645A9B81733A36427FD6568145205F` |
| `wydgo748/internal/game/colosseum_control_test.go` | `87AA8059A88AF848CC384584A2D05834599320DDC7BD99745CBE3BCE1F1E1B5C` |
| `wydgo748/internal/game/colosseum_notices.go` | `9C2BBD35B214CCF0A19EC1E9B9D5BA9708680FA36324478D75BCC29045AFC5C0` |
| `wydgo748/internal/game/colosseum_test.go` | `5E7CF3DA34DAEAEEB3D448F8E0F0A938B5C0C07638580F8FCEFDB5A816E80FE9` |
| `wydgo748/internal/game/colosseum.go` | `53012D82D7A0FC8A649BBCC4642FAA51EB0BDFEC1722398DB5DFF9785B431676` |
| `wydgo748/internal/game/commands.go` | `B51F64302DE9957A1635EE2EAA9F0260488F95258ED6A9D993612D423AD2DE21` |
| `wydgo748/internal/game/mob_combat.go` | `DB1A047DC6E3667DF4621406A7E987F42BCF9D81247A7F8E47FEB4FAAFC61952` |
| `wydgo748/internal/wire/client_build_test.go` | `0F46B1D8B8723DEDF05C17A44BBAE139B901AE6C843B91B89FF70B27A75AA790` |
| `wydgo748/internal/wire/client_build.go` | `DC2014864D26C56E31F4E2CF8F6F61F0F9EA84A342FA6A2A4E64456385B7445B` |
| `wydgo748/web/portal/src/content/news/client-7-48-002.md` | `077A4CE8822A0DE67911637F17A4C271E4A9099CE08915B7E69AD03C758D0065` |
| `wydgo748/web/portal/src/content/news/client-7-48-003.md` | `615B56897EC0EAB23E2DCDC47E5E70D9C17CA4523B43976D32366FFFA50D4FE9` |
| `wydgo748/web/portal/src/data/public-content.ts` | `65FB0FF03008095B719BDE0683DDB0195980B5671B5D2049D4276E6CCF1DAA68` |
| `wydgo748/web/portal/src/pages/download.astro` | `91BF8EF360F3B6E85C5A8B0AF53D61BE6BBA3CC641239608A265EE2C894447FC` |
| `wydgo748/web/portal/tests/news-content.test.mjs` | `AACA0266C3218B8417CED8E7705F660EA2111475AC8B144C99BF57AFF3AFC12F` |
| `wydgo748/web/portal/tests/public-content.test.mjs` | `A99052CAE1FAB67FA2282851DFEC726407BDF1D4469BE90D9F4E19C58D2BD785` |

</details>

## Published patch 7.48.002 (2026-10-24)

Feed **48002**, compiled identity **0x02EC0002**, minimum server revision **2**.
Published and verified before server activation. Every client patch is mandatory;
the compiled identity, rather than the editable configuration counter, controls
login admission. The native launcher and its language DLL are unchanged.
Install full client 7.48.001 first if using the retired 100xx release line.

Source: HEAD `40706d99b7715167d7aa4a2c484704f7e91c2c8b` plus the explicitly
selected working-tree client, server and portal changes. No commit or push was
performed. The package uses the established working-tree artifact allowlist;
`Build-ClientPatch.ps1` would only represent committed resource inputs.
Unrelated research, reference clients and local configuration are excluded.

- ZIP: **864,100 bytes**, SHA-256
  `AB2BC312460D30C9415129D32FF8FF35DC08EDF0E9E1D3296524DAE6BB7732B0`.
- `WYD.exe`: **1,937,408 bytes**, SHA-256
  `80CA48E77A8F83535401B7FE1415E33C9DE873E04F55C0313C844DF7B6A66209`.
  Built from `tmproject/Build-Client.ps1 -NoDeploy`; candidate path
  `tmproject/build/TMProject748/Release/WYD.exe`.
- `UI/command.txt` and `EN/UI/command.txt`: 1,959 bytes each, SHA-256
  `FC07616C20FAF3E76CCB84CC6F06AA89255A1D89065DDD052B57B7FAD0E9C683`.
- `PT/UI/command.txt`: 1,759 bytes, SHA-256
  `722855F8D4A87EC7F7D6385C533ACD13F531CE6DA5A61606B7F3BD35C814D2BB`.

Those are the ZIP's only four entries. No shared assets, launcher binaries,
`lang.txt`, `config.txt`, `ClientSettings.txt` or account state are included.
Fresh HTTP and HTTPS downloads matched the ZIP and all four entry hashes.
Both feeds preserve the 48001 baseline and append `48002 patch-48002.zip`.
The live manifest, home, Download, News and changelog advertise 7.48.002;
the full-client link remains the owner's 7.48.001 baseline with unverified
archive size/hash. [Public announcement](https://wyd.forjago.com/news/client-7-48-002/).

At 19:55:24 UTC, two production protocol probes verified the gate: identity
`0x02EC0001` received the update-required message before authentication;
`0x02EC0002` reached authentication and received the expected invalid-credentials
response for a random nonexistent account. No account was created or character
authenticated. This proves admission routing, not a real-client login or the
launcher installation flow. Actual rendering, both languages and gameplay
acceptance remain pending. See [active server receipt](../current-state.md).

<details>
<summary>Working-tree source provenance (41 SHA-256 inputs)</summary>

These hashes identify the selected changed build inputs on top of the HEAD
recorded above. They exclude local runtime configuration and unrelated work.

| Input | SHA-256 |
| --- | --- |
| `tmproject/client748/EN/UI/command.txt` | `FC07616C20FAF3E76CCB84CC6F06AA89255A1D89065DDD052B57B7FAD0E9C683` |
| `tmproject/client748/PT/UI/command.txt` | `722855F8D4A87EC7F7D6385C533ACD13F531CE6DA5A61606B7F3BD35C814D2BB` |
| `tmproject/client748/UI/command.txt` | `FC07616C20FAF3E76CCB84CC6F06AA89255A1D89065DDD052B57B7FAD0E9C683` |
| `tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp` | `D7C219AB851B8CF85B63104B9671C53B0210278600CEAD720B8BE431A54305C5` |
| `tmproject/TMProject748/internal/app/scenes/TMFieldSceneUI.cpp` | `36FBEA44200E5E4842327A1FD6353F07467593257E2A2523292DCCBFF4A52CC5` |
| `tmproject/TMProject748/internal/app/scenes/TMScene.h` | `592F97CA521EC72F2854C7C35BB6ED3163A3E68262CA31B854F60A5FC122E9EA` |
| `tmproject/TMProject748/internal/app/scenes/TMSceneResources.cpp` | `1768DAD4D1DB0A616E94EEED3BE1F664FA1E01DB4C7AA7160D5396596EC687C3` |
| `tmproject/TMProject748/internal/application/FieldChatSubmitPolicy.h` | `3751E4C5044E10AE05420AD18A368DC2FF90109FD859833C7BDCFF427CBDB455` |
| `tmproject/TMProject748/internal/core/QuestSubjectLevel.h` | `1A72F1002CBAA6346645B4E9518C1F83F34F5339C70CC3C9F1C7EE5F14BBB5A0` |
| `tmproject/TMProject748/internal/platform/windows/FontGdiResources.h` | `DADB506F2997DF7603928053432C6738CC256E62F44046BADE04F9AF5DE50179` |
| `tmproject/TMProject748/internal/platform/windows/ResourceTextGdi.h` | `1EAB18D5B8C066448E1BD7AFB82E007A1A92C7EBC21FF0F248C74D3C4DA6E7A3` |
| `tmproject/TMProject748/internal/platform/windows/SingleLineTextLayout.h` | `DA3DCCCCB96D5831965A54DB41DA6AA7465BDAF764067E2D2FB799FABD5E4224` |
| `tmproject/TMProject748/internal/platform/windows/TMFont2.cpp` | `292E39B5024672D2EF18DDB92200A1F737F4E4E7F4E851F6D556D919CBA56A1E` |
| `tmproject/TMProject748/internal/platform/windows/TMFont2.h` | `F699F0ACE27AAC356D26FC7E85476062034A4EDC7AFEDDDD6E86B450316C6A1E` |
| `tmproject/TMProject748/internal/ui/controls/SText.cpp` | `BF72473C5438F0B57CF1555B73798FA6BBEC1762B0A2C49C310126B051903EDC` |
| `tmproject/TMProject748/internal/ui/FieldCaptionLayout.h` | `4135CE4F4E8A1AEBEAA703CB42138F69FEC6B5D86B855F2C3FAD6A987B2B1A51` |
| `tmproject/TMProject748/internal/ui/SGridTooltip.cpp` | `52C5A8EA3AE9C2B9795C8651CC948BD5BC76991AA94384BAF269534C2799F2A5` |
| `tmproject/TMProject748/internal/wire/ClientBuildContract.h` | `1274E2F35F7EE714A45F3AC04040A4DA4EB79A5F92C544E3479356A8C9D6A7AD` |
| `tmproject/TMProject748/tests/ArchitectureTests.vcxproj` | `094A2B1DE8E5C36B36F9978AE7B55B5C2D26ECE5963E59F079B9A4073DDEFC77` |
| `tmproject/TMProject748/tests/LocaleBoundaryTests.cpp` | `E16355B62318695C38D4DCCBA358799A5D261F7585A5803A107C92BFE9765380` |
| `tmproject/TMProject748/tests/PacketViewTests.cpp` | `35F64DFFF1DD4F61C769E7BA457918412ECE714E7E6C96E1F31837E1E4D7AA86` |
| `tmproject/TMProject748/tests/SingleLineTextLayoutTests.cpp` | `6CF9663576363AD3AF3A2BBCFBA620B5601901E382A2526E79F2CDA8B7B98CB9` |
| `tmproject/TMProject748/TMProject748.vcxproj` | `F4DCD9DBFA0014588ECD6FDCE3AF403B66CB19C42C178590AFE924BA4FE38117` |
| `tools/client-assets/client_locale_overrides.json` | `886BC485E09133BD78AC57FC2605BB73B429EB8327458391E8154B663C2D6BFA` |
| `tools/client-assets/test_client_locales.py` | `6E02B9F9F08D5CF53B712FBEC71EF6B07E2AEF36BB8EDE6957BC14BB27FE27AA` |
| `wydgo748/internal/game/client_command_admission_test.go` | `21F24BE1168E0BF4B7EDBA39E709D79A75ABF1EBC36A7E4EBB383676734E4835` |
| `wydgo748/internal/game/client_command_admission.go` | `8AE306563C9B820BB3843E161905D7F6BA5193EFF38A615AD6DAF2FD20BE808A` |
| `wydgo748/internal/game/colosseum_combat_test.go` | `AE4AB47D71DAF677FBE62D9C6BE2530850645A9B81733A36427FD6568145205F` |
| `wydgo748/internal/game/colosseum_control_test.go` | `87AA8059A88AF848CC384584A2D05834599320DDC7BD99745CBE3BCE1F1E1B5C` |
| `wydgo748/internal/game/colosseum_notices.go` | `9C2BBD35B214CCF0A19EC1E9B9D5BA9708680FA36324478D75BCC29045AFC5C0` |
| `wydgo748/internal/game/colosseum_test.go` | `5E7CF3DA34DAEAEEB3D448F8E0F0A938B5C0C07638580F8FCEFDB5A816E80FE9` |
| `wydgo748/internal/game/colosseum.go` | `53012D82D7A0FC8A649BBCC4642FAA51EB0BDFEC1722398DB5DFF9785B431676` |
| `wydgo748/internal/game/commands.go` | `B51F64302DE9957A1635EE2EAA9F0260488F95258ED6A9D993612D423AD2DE21` |
| `wydgo748/internal/game/mob_combat.go` | `DB1A047DC6E3667DF4621406A7E987F42BCF9D81247A7F8E47FEB4FAAFC61952` |
| `wydgo748/internal/wire/client_build_test.go` | `9F8515AD3119EAA393E8D32B906CCE1B62C06709C5B105A8B41F0704C3B28634` |
| `wydgo748/internal/wire/client_build.go` | `0923B4B2E2D6BC8D5449A0A48E6D6464561BBAE46A06870D68A5025D9139E61B` |
| `wydgo748/web/portal/src/content/news/client-7-48-002.md` | `077A4CE8822A0DE67911637F17A4C271E4A9099CE08915B7E69AD03C758D0065` |
| `wydgo748/web/portal/src/data/public-content.ts` | `C5ACF64FD6AE6ECFAA29C6FF3D2FF28AD9F56BFE640C9FB6F6D1C9E6332D4619` |
| `wydgo748/web/portal/src/pages/download.astro` | `91BF8EF360F3B6E85C5A8B0AF53D61BE6BBA3CC641239608A265EE2C894447FC` |
| `wydgo748/web/portal/tests/news-content.test.mjs` | `592E282C6E5E80BD2C41961F04405888055412DB0F7ED0AE73AA9429F66222BB` |
| `wydgo748/web/portal/tests/public-content.test.mjs` | `B6977AD7576D943491EC17E6B6AE14D94997C1102ABE5D9F677E992CB3CA2B44` |

</details>

## Full client 7.48.001 (2026-10-23, published)

Starts [release line 7.48](#release-line-748) and retires patches 10001-10045.
`[VERSION] 48001`; game identity `0x02EC0001`; launcher SHA-256
`85C9C648B389BE91CA348351590FCBEFA6752A4D4E849AFBBCB1F054CA12FDE2` with
`WYDLauncherLanguage.dll` `003134AB…6BF59`; `WYD.exe` `D29873E2…76B7BB3A`.
Distributed through the owner's Drive link (archive not downloaded or hashed
here). The live feed is the 48001 baseline; the 100xx list and ZIPs are archived
in `/srv/wyd-go/patch-archive/20261023T203833Z/`. Activation receipt:
[current state](../current-state.md).

### Full client contents

Runtime files are only what `WYD.exe`, `WYDLauncher.exe` and its DLL read.
Removed on 2026-10-23 after checking the built executables' import tables and
path strings, the game source, tools and tests: the unimported `D3D8.dll` +
`dgVoodoo.conf` wrapper (the game imports `d3d9.dll`), `JPGLib.dll`, the
`JProtectDll.dll`/`jlock10x` anti-cheat files, `curse.bin` (the curse filter is
a no-op), empty `extraitem.bin`, eight `Env/*.map` files, stray copies under
`mesh/` and `UI/` (AttributeMap, ItemList, SkillData, cdata, Field*.dat, texture
lists, a `MeshTextureList` backup), unused text lists, camera paths other than
`CreCamAction`, `SelCamAction1-3` and `DemoCamAction4`, the never-entered demo
and ending scenes, `TimeTable.bin`, `TOTOGame.csv` and `cursor3.cur` (cursors
are embedded). Build-only inputs live in `tools/client-assets/inputs/`.

Kept deliberately: `InitItem.bin` (asserted as shipped by the asset tests),
the unused scene layouts `UI/FieldScene.bin`, `LoginScene*.bin`,
`SelCharScene.bin` and `SelServerScene.bin` (the locale builder reads them for
`UI/captions.txt`, whose keys only match these files and never the loaded
`*2.bin` layouts), `Launcher.bmp`/`.hat` and `sound.txt` (launcher).
Do not ship `lang.txt` (defaults to English), logs, `dumps/` or `update.dat`.

## Published patch 10045 (2026-10-23)

Client build revision **3**; the activated server requires revision >= 3.
The owner authorized commit/push, publication and the controlled restart.
Source commit `41e192ad`; portal receipt commit `cd91c527`; both pushed.
Executable-only ZIP, packaged as 10044.

- ZIP: 852,533 bytes, SHA-256
  `A95C70FA2875F5CA9820076E12468A8617E6D338AE1E8A929A6082C8177B31C0`.
- `WYD.exe`: 1,919,488 bytes, SHA-256
  `1FCC782FBB7F95B2F619A55D8A566DBFA8453FB65EA95D40F1D96F2D944FDF0F`;
  it embeds build identity `0x57470003` and no revision-2 identity.
- HTTP/HTTPS feeds list 10045 and both downloads match; the public manifest,
  News article and changelog advertise 10045. Published before the server
  activation.
- Revision-2 gate evidence from the 10044 server: a revision-1 client was
  rejected before authentication at 05:53:57 UTC and a revision-2 client
  logged in at 05:54:11 UTC.

## Published patch 10044 (2026-10-23)

Client build revision **2**; the activated server requires revision >= 2.
The owner authorized commit/push, publication and the controlled restart.
Source commit `271a75d4`; portal receipt commit `613a5cce`; both pushed. No
client asset changed after `4dcbeed3`, so the ZIP holds only the validated
Release executable (the asset builder refuses an empty diff); it was created
with the same .NET ZIP entry method used to add the executable to 10043.

- ZIP: 852,632 bytes, SHA-256
  `5EA50C20CD9455FF37108E933AC0D2A64562C0A608676E47D51097C23A04136D`.
- `WYD.exe`: 1,918,976 bytes, SHA-256
  `53AE596043FDCEE439821E549DC827C556C4CF07E646A2FF584D52957668CEDA`;
  it embeds build identity `0x57470002` and no revision-1 identity.
- HTTP/HTTPS feeds list 10044 and both downloads match; the public
  manifest, News article and changelog advertise 10044. Published before the
  server activation.

## Published patch 10043 (2026-10-23)

First mandatory patch: client build revision **1**; the activated server
requires revision >= 1. The owner authorized commit/push, publication and a
controlled stop of the three online players. Source commit `4dcbeed3`; portal
receipt commit `78bb9bd4`; both pushed. Assets from Git between `379dcd93`
and `4dcbeed3`; the validated Release executable was added explicitly.

- ZIP: 913,376 bytes, SHA-256
  `98F54183D7361484026CE8F4213568886FC76D0549E24A7A96980E1AE16C2FFD`.
- `WYD.exe`: 1,918,976 bytes, SHA-256
  `7FE98A22B51E2EA8864FF31483186866075B648F81C554592218AD523E55DC64`;
  it embeds build identity `0x57470001`.
- `ItemList.bin`: 910,004 bytes, SHA-256
  `81AD6789B9AB0B4607E5F76EF9772676A6DC9FDDC79790EB1EC31CDA4E53EDFC`.
- `itemhelp.dat`: 81,701 bytes, SHA-256
  `47C97E7FCD119C4D8B5C2E549AF80C5CFE1ACBBF4741055C050D96459C633FB3`.
- HTTP/HTTPS feeds list 10043 and both downloads match. The public manifest,
  home, News article and changelog advertise 10043.
- Published before the server activation, as the gate requires. First
  production logins: an old client (build `0x00000000`) was rejected before
  authentication at 01:24:17 UTC, and a current client logged in with build
  revision 1 at 01:24:37 UTC.

## Published patch 10042 (2026-10-22)

The owner authorized committing/pushing the staff panel (P1, stages 0–3) and
Big Cube batch to `main`, VPS publication and a service restart (no machine
reboot). Source commit `379dcd93`; portal receipt commit `0f4b8b71`. The
patch builder took client assets from Git between `1faf8aca` and `379dcd93`;
the validated Release executable was added explicitly. No private settings,
launcher or local version marker are included.

- ZIP: 863,580 bytes, SHA-256
  `F91D11AA93292AFE38DBD2092443C72E7A536CBB57292D556FD6B9DCC50FE7DA`.
- `WYD.exe`: 1,917,952 bytes, SHA-256
  `7F8C10E433E97526525FBDE42DA5458C93F31359CCC686CB01D35E89EDE033A9`
  (built after the last client product-source edit; gates recorded in
  [current state](../current-state.md#local-source--2026-10-21-big-cube-ticket-and-return-portal)).
- `Env/AttributeMap.dat`: 1,048,580 bytes, SHA-256
  `3D56C0E1EA4AC286EF74D9A7AB764B02AF77964D904F9A3D21AD0B83444465F4`.
- Both public HTTP/HTTPS feeds append 10042; fresh HTTP and HTTPS downloads
  match the ZIP hash. The public manifest, home, News article and changelog
  advertise 10042.
- Active server release: `/srv/wyd-go/releases/20261022T180448Z`. See
  [current state](../current-state.md) for activation and rollback.

Close the game and run `WYDLauncher.exe` to install the coordinated patch:
the game server now sends the private Big Cube question packet `0x7F12`,
which earlier clients do not display. Not `CLIENT_TESTED`: ticket
confirmation, the question panel, O/X resolution and the return portal still
need real-client acceptance.

## Published patch 10041 (2026-10-21)

The owner authorized committing/pushing the audited October 19-21 batch to
`main`, VPS publication and required player disconnection. Source commit
`6746e784` includes the required wide-item sidecar; portal receipt commit
`60c2b618` advertises the matching update. The patch builder took assets from
Git between `7ff2cfc94344c757db05525f60fe6be18a77d537` and `6746e784`; the
exact validated Release executable was added explicitly. No private settings,
launcher or local version marker are included. Earlier dependencies remain in
the cumulative feed.

- ZIP: 900,489 bytes, SHA-256
  `4CF0535F04C58362478F4927D50717FD6D1140B4262439CBB357DC8790BB0628`.
- `WYD.exe`: 1,916,928 bytes, SHA-256
  `64C9BDDEEC0EE6E6197B63A0CA915B10BE1AEA8F1C6B03A1F9D3EC47428C3F82`.
- `ItemList.bin`: 910,004 bytes, SHA-256
  `B674BFD2992E9B38A8C81A5CE7CEDB11FE5CE37E64093BB14D22B9DBE9EBCA0F`.
- `ItemListWide.csv`: 225 bytes, SHA-256
  `4E777CF0C799525527226B0D8E39446AD70927C52FD044F2CB81C4E880E6837B`.
  Git packaging uses Windows CRLF; the working copy's LF-only byte hash
  differs without changing the parsed sidecar contract.
- Both public HTTP/HTTPS feeds append 10041. Fresh HTTP/HTTPS ZIP downloads
  match, and the HTTPS archive contains exactly the three entries/hashes
  above. The public manifest and website home, download, News/article and
  changelog advertise 10041 with the matching receipt.
- Active server release: `/srv/wyd-go/releases/20261021T190239Z`. Authorized
  controlled shutdown saved one account with zero failures before activation.
  Game/API running executable hashes, readiness and listener checks passed.
  See [current state](../current-state.md) for provenance, rollback, backup
  and the recovered transient SSH timeout.
- Reused client Release/architecture/socket/crash/launcher gates and full Go
  tests with real isolated PostgreSQL coverage. Portal check/build and all 60
  tests passed again. Publication does not validate native damage formulas or
  actual gameplay, and does not establish `CLIENT_TESTED`.

Close the game and run `WYDLauncher.exe` to install the coordinated patch.
The local installed executable was not replaced by this publication. Native
formula conflicts, wide progress-bar end-to-end rendering and real-client
acceptance remain open. No global language/release-readiness claim is made.

`tmproject/client748/WYDLauncher.exe` is the OnGame 7.47 launcher adapted to
this client. It downloads incremental patches from the VPS, keeps
`config.txt` and `guilds.txt` current and starts `WYD.exe`. The full client
is distributed outside the VPS (Google Drive) so that its outbound traffic
stays limited to small patches.

Classification: `PARIDADE_NATIVA` for the patch protocol (traced in the
native 7.47 launcher, compatible with the 7.48 `config.txt`, `Music.txt` and
`sound.txt` formats); the host, translations and resolution table are a
`MODERNIZACAO_COMPATIVEL` adaptation. Game wire, assets and `WYD.exe` are
unchanged.

## Published patch 48012 (2026-10-30)

Mandatory client 7.48.012 contains `WYD.exe`, root, EN and PT `UI/strdef.bin` and
`UI/command.txt`, and `SkillData.bin`. The committed ZIP is 938,967 bytes,
SHA-256 `065D2BB04C2692390AEBCBB24DE6DDA551BED004526E8D82DF7ED298994FEB9B`. HTTP/HTTPS downloads matched the ZIP and all eight
entry hashes; the feed kept every earlier entry and appended 48012 once.

Client revision and server minimum are both 12. Server release
`20261030T043606Z` was activated after patch verification and a
database backup. Both channels reject revision 11 and allow revision 12.
Rollback is `/srv/wyd-go/releases/20261029T203701Z`. Gameplay acceptance remains pending.
See the [release receipt](../../.agents/research/release-748012.json).

## Published patch 48011 (2026-10-29)

Mandatory client 7.48.011 contains `WYD.exe`, `skillhelp.dat`,
`EN/skillhelp.dat` and `PT/skillhelp.dat`. The working-tree ZIP is 929,053 bytes,
SHA-256 `362682032FCCEA7C3A6F96CE4F80DAF146D047CE4407D5335AE10EE8DAB71806`.
Fresh HTTP/HTTPS downloads matched the ZIP and all four entry hashes.
The feed retains every earlier entry and appends patch 48011 once.

Client revision and server minimum are both 11. Server release
`20261029T161155Z` was activated after patch verification and a database backup.
Both channels reject revision 10 and allow revision 11 through version admission.
The site manifest, download page and news post identify 7.48.011.
Actual gameplay acceptance remains pending. See the
[release receipt](../../.agents/research/release-748011.json).

## Building the launcher

```powershell
python -I tools/client-assets/build_wyd_launcher.py
```

The script reads the local-only reference `references/OnGame/WYD/WYDLauncher.exe`
(SHA-256 `6cd8b16f…44da75`), refuses any other input, and writes the adapted
executable plus the neutral `Launcher.bmp`/`Launcher.hat` artwork to
`tmproject/client748/`. The reference is never modified. Every patch checks
the original bytes first:

| Change | Detail |
| --- | --- |
| Update host | `update.htm` and file base URL become `http://wyd.forjago.com/patch/` |
| FTP fallback | A failed HTTP download fell back to FTP at the third-party `80.77.12.45`; now `127.0.0.1` (fails closed) |
| Language | Every Portuguese message, button, dialog label and the window title (`WYD 7.48`) is English |
| Resolutions | The settings radios list this client's 11-entry `[RES]` table (`NewApp.cpp` `stResList`) |
| Classic UI | The loader forced the classic UI for RES 1 and 6 (both 640x480 in the OnGame table); only RES 1 now |
| DirectX help | The Korean help URL opens `https://wyd.forjago.com/download/` |

Known residue: the MFC framework string tables (internal error messages)
remain Korean; they are not Portuguese and appear only on MFC failures.

## Traced protocol (Ghidra, native 7.47)

1. On start, the launcher reads `config.txt` (`[VERSION]` must be non-zero),
   `Music.txt` and `sound.txt`, normalizes values and rewrites them on exit.
2. It reads `update.htm` with one 8 KiB `InternetReadFile`, stores it as
   `update.dat` and requires the first line `[WYD]`.
3. `guilds.txt` is always downloaded first. The 7.48 client reads
   `./Guilds.txt` to show guild names; no packet carries them.
4. Each line is `<version> <file> [<file>...]` (`NOTIC w h url` shows a
   notice). The first line's version must not exceed the local version, or
   the launcher asks for a new full client. Files of every line whose version
   is greater than `[VERSION]` are queued; versions below 1000 are multiplied
   by ten.
5. If the newest version is in another ten-thousand range than the local one
   (`version / 10000` differs), the launcher asks for a new full client.
6. Each file is fetched over plain HTTP on port 80 (`InternetConnect`,
   no TLS) from the base URL. ZIPs are extracted over the client folder,
   creating folders, then deleted; `[VERSION]` advances after each patch.
7. **Start** copies `serverlist.<N>` over `serverlist.bin` if it exists (N is
   the server combo index, both entries are `WYD-Go`; no such file ships) and
   runs `WYD.EXE` from the launcher folder.

The launcher cannot delete files, replace itself while running, or verify
downloads beyond HTTP and ZIP CRCs. Patches travel over plain HTTP.

## Server feed

Caddy serves `/patch/` on both `http://` (no redirect, for the launcher) and
`https://wyd.forjago.com`:

- `/patch/update.htm` and `/patch/patch-<version>.zip` from `/srv/wyd-go/patch`;
- `/patch/guilds.txt` from the game server's `Guilds.txt` export in
  `/var/lib/wyd-go-public` (`guilds_txt` in `server.txt`).

The client base version is `[VERSION] 48001` in the distributed
`config.txt`. The baseline list is `tools/deployment/vps/update.htm`.

## Release line 7.48

Traced in the operational launcher (pinned SHA-256 `A60DCA35…E2E901`, Ghidra
headless, 2026-10-23): the status text at `0x439ED8` is formatted at
`0x403F37` with `[VERSION] / 1000` and `[VERSION] % 1000`; the range check at
`0x4039D8` divides the local and newest versions by 10000 (`0x68DB8BAD`,
shift 12) and asks for a full client when they differ; `0x402946` rejects a
first feed line newer than the local version. The launcher keeps
`[VERSION]` in an int. The game's `WYD748_LoadClientConfig` used to bound every
key to `short`, so 48001 rejected the whole `config.txt` and the client fell
back to full-screen defaults; `VERSION` is now an int (> 0) while every other
key keeps the short range, with regression tests. The multilingual launcher build
changes only that format to `Ready to play.  (Version 7.%d.%03d)`, so
`[VERSION] 48001` shows **7.48.001**. All other native code and data stay
byte-identical.

The full-client reset retires every 100xx patch. The feed baseline is
`[WYD]` + `48001 notice.txt`: an old client (10045, range 1) sees a newer
first line and a different range and is told to download the full client.
Patches of this line are 48002..49999 (7.48.002..7.49.999), enforced by
`Build-ClientPatch.ps1`, `Publish-ClientPatch.ps1` and `vps/publish-patch.sh`;
build the first one with `-From` the commit that produced the 7.48.001 full
client. Each patch also raises the compiled admission revision
([contract v2](../login-dispatch.md)).

Activating the reset on the VPS is a one-time, authorized operation after the
full client is downloadable: upload `vps/update.htm` and `vps/reset-patch-feed.sh`
and run `sudo bash reset-patch-feed.sh <uploaded update.htm>`. It moves the
current list and every `patch-*.zip` to `/srv/wyd-go/patch-archive/<UTC>/`
(nothing is deleted) and installs the baseline list last. Then activate the
server release carrying contract v2 and deploy the portal.

## Publishing a patch

A patch is the set of client files added or changed between two commits:

```powershell
$version = 48005 # Use the live maximum plus one; keep the value in 48002..49999.
pwsh -NoProfile -File tools/deployment/Build-ClientPatch.ps1 -From <released commit> -To HEAD -Version $version
pwsh -NoProfile -File tools/deployment/Publish-ClientPatch.ps1 -Version $version -SshTarget <user@host> -IdentityFile <key>
```

The current release line accepts versions from `48002` through `49999`.
The example uses `48005` because the recorded feed already contains `48004`.
Read the live feed before each publication and use its maximum plus one.

`Build-ClientPatch.ps1` takes bytes from Git with `core.autocrlf=true`
pinned (a fresh Windows clone), skips `config.txt`, `ClientSettings.txt` and
`lang.txt`, refuses `WYDLauncher.exe` and `WYDLauncherLanguage.dll` (the running
launcher cannot replace itself or its loaded extension), and reports deletions,
which players keep. `Publish-ClientPatch.ps1` appends `<version>
patch-<version>.zip` to the live list; `vps/publish-patch.sh` tests the ZIP,
installs it, keeps the previous list as `.update.htm.previous` and replaces
`update.htm` last. A launcher change requires a new full client.

The local multilingual candidate preserves this native updater and adds a
source-built language selector to its main dialog. Build and install with
`pwsh -NoProfile -File tools/client-assets/Build-NativeLauncherLanguage.ps1`.
Distribute `WYDLauncher.exe` and `WYDLauncherLanguage.dll` together. See the
[multilingual client contract](multilingual-client.md) for pinned provenance,
the `lang.txt` contract, automated coverage and remaining visual validation.
This local candidate has not been published.

Every client patch is a mandatory update. The game server rejects, before
authentication, any client whose compiled build revision is below its minimum
([client build admission](../login-dispatch.md)). Each patch therefore raises
`client_build::Revision` and the server's `MinimumClientBuildRevision`
together, one above the last published revision, and ships with the server
release that enforces it: publish and verify the patch first, then activate
the server. Record the revision in the patch receipt. Retired v1 line: patches
up to 10042 predate the gate; 10043, 10044 and 10045 carried revisions 1-3.
Release line 7.48: the 7.48.001 full client carries revision 1, so patch 48002
carries revision 2.

Every published patch or player-visible server release also updates the
website: the `clientRelease` receipt and a `changelog` entry in
`wydgo748/web/portal/src/data/public-content.ts`, and a player-facing news
post as a new file `wydgo748/web/portal/src/content/news/<slug>.md`. A post
has `number` (the next one after the highest existing), `slug` (same as the
file name), `date`, `category`, `title` and `summary` front-matter lines,
then plain-text paragraphs separated by blank lines; markup and HTML are not
interpreted, and a malformed post fails the build. `tests/news-content.test.mjs`
checks every post, and `tests/public-content.test.mjs` fails when News does not
mention the latest published patch. Build the portal, run its tests and
deploy it with the release.

## Validation state

- Patcher: every original string, code byte and dialog verified against the
  7.47 image; output dialogs contain no non-ASCII text; code and data
  sections are byte-identical apart from the listed patches (automated).
- Feed: `http://wyd.forjago.com/patch/update.htm` and `/patch/guilds.txt`
  return 200 without redirect; other HTTP paths redirect to HTTPS.
- Patch builder: a dry run produced a valid ZIP whose entries match the
  client tree.
- Not yet `CLIENT_TESTED`: the launcher window, settings dialog, a real
  patch download and **Start** need a person to run them.

The following `100xx` sections are historical receipts from the retired v1
line. They preserve provenance and do not define current commands, ranges, or
release procedures.

## Published patch 10001 (2026-10-11)

The production feed now lists `10001 patch-10001.zip` after the `10000`
base entry. This patch contains only the tested `WYD.exe`, including the
female bow body-reflection correction. It does not replace the distributed
launcher, player settings, or runtime assets. The source launcher remains
incompatible with this legacy feed and was not deployed.

The executable is an ignored build artifact, so this publication used an
explicit single-file ZIP from the validated local runtime rather than the
Git-revision patch builder. `Publish-ClientPatch.ps1 -Version 10001` uploaded
the archive and replaced the manifest last; the previous manifest remains
at `/srv/wyd-go/patch/.update.htm.previous`. The game service was not restarted.

- ZIP size: 810,500 bytes.
- ZIP SHA-256: `DC9F3CC1E2F68110923CFE79A4AFE7BE8D442D78711E6A4F15FBDF0897FD821C`.
- `WYD.exe` SHA-256: `81E824FC6392645B68D0B81E3B3FA17CD62AED40933878D9F87A296F7E34DCB8`.
- Public HTTP manifest returned 200 without a redirect and contained the new
  entry. A fresh HTTP download matched both hashes and contained only `WYD.exe`.
- Validation: `BUILD_AND_DEPLOY_VERIFIED` locally; patch publication and
  public download verified. Launcher extraction and in-game orientation remain
  pending human verification, not `CLIENT_TESTED`.

Players must close the game before opening `WYDLauncher.exe`. A successful
update advances `[VERSION]` in `config.txt` to `10001`; do not edit this value
manually to bypass downloading the patch. Future patches must use a higher
version. The legacy HTTP/ZIP-CRC transport limitations above still apply.
This client patch does not resolve the separately investigated server-side
critical-persistence session isolation incident.

## Published patch 10002 (2026-10-12)

The feed lists `10002 patch-10002.zip`. Like 10001 it contains only
`WYD.exe`, built from the working tree with Sephira special skills, slot-13
familiars, the costume-slot extension, costume expiry tooltips and the
Inventory X cascade ([port record](port-748.md#wyd-go-extensions-costume-slot-expiry-display-timed-horses-2026-10-12)).
It requires server release `20261012T024132Z`, deployed first.

- ZIP size: 811,012 bytes.
- ZIP SHA-256: `5FDB89F86EE9CBD943C02A59C28252A36EE570259136B255EE3D994AD36808FE`.
- `WYD.exe` SHA-256: `F84E27ED9CA0C4E6A0C01CCB877214F98E14C592C367574A34394451EC591FE0`.
- The public manifest returned 200 without a redirect; a fresh HTTP download
  matched the ZIP hash and contained only `WYD.exe`.
- Pending: launcher extraction and in-game checks by a person
  (not `CLIENT_TESTED`).

The earlier source launcher in `tmproject/WYDLauncher/` (content-addressed
HTTPS updates) is superseded by this adaptation.

## Published patch 10003 (2026-10-12)

The production feed now appends `10003 patch-10003.zip`. Server release
`20261012T041400Z` was installed first. The patch includes the audited client
executable with the remaining-time fairy tooltip and restored countdown HUD.

All 20 tracked runtime assets changed from `aa4d0855` to `ce411059` were
included, together with the ignored, validated `WYD.exe` build: 13 music files,
server/group lists, launcher resources/configuration, the corrected sound
table and its new `mguardatt.wav` dependency. Matching the local full bundle
does not prove that older player installations contain these assets; the
incremental patch therefore delivers them too. All 21 entries match the
audited source/build bytes. No settings or launcher executable are included.

- ZIP size: 26,989,810 bytes.
- ZIP SHA-256: `D54C7A40486FBB0F3A7E76D6518FDC45E462CCD1C10E02F60950E946F9CC825E`.
- `WYD.exe` SHA-256: `2A4869B828AD2BAC25657E7A160C574121D761C9380E8BE0333C8134C95519F1`.
- `sound/soundlist.txt` SHA-256: `274AB767F2E978F34E0E50306D7B232379740015DD1A7B0DA4912D3951D4E795`.
- The public HTTP manifest returned 200 without redirect and contained 10003.
  A fresh public download matched the complete ZIP hash and all 21 entries;
  the executable, table and new sound dependency were separately hashed.
- Publication uses a serialized, append-only manifest update after ZIP
  validation/installation. The previous manifest remains recoverable. This
  prevents concurrent publishers from losing an entry; it does not add TLS,
  signatures or authenticity to the legacy transport.
- Validation: `AUTOMATED TESTED`; built artifact and public publication
  verified. Local runtime installation was deliberately not performed.
  Launcher extraction and gameplay remain pending human validation, not
  `CLIENT_TESTED`.

Close the game and run `WYDLauncher.exe` to receive 10003. The launcher cannot
remove the obsolete `project.exe` from older installations; launch through
`WYDLauncher.exe`/`WYD.exe`, not that stale executable. The full client remains
hosted outside the VPS; this feed serves incremental updates only.

## Published patch 10004 (2026-10-12)

The production feed appends `10004 patch-10004.zip`, containing only the
validated `WYD.exe` with the [weather lifecycle correction](performance-and-visuals-plan.md).
Rain and snow activation is mutually exclusive; clear weather disables the
particles, regional transitions preserve the last server weather, and the
heavy snow layer no longer controls the base layer's sound.

The executable was built from `2f2381f9` plus the uncommitted weather changes.
This receipt identifies the delivered artifact, not a committed source release.
No settings, launcher executable or assets are replaced by this patch. It
requires no game-server restart and was not installed into the local runtime.

- ZIP size: 811,389 bytes.
- ZIP SHA-256: `4BB3836863247FB000ACE88A6735CF1F46067E94B1393F7CDD7ABC4F0FC59374`.
- `WYD.exe` size: 1,847,296 bytes.
- `WYD.exe` SHA-256: `0BAE2073F9D5CA6220C0642ABC89A9D06E7784E516E1F0D4A0E32125DD79E0F0`.
- The public HTTP feed and archive returned 200 without redirects. A fresh
  download matched the ZIP hash and contained exactly one root `WYD.exe`
  whose size and hash matched the validated build.
- Validation: `AUTOMATED TESTED`; the Release build passed 1,200,822
  architecture checks, 231 socket checks and 56 launcher checks. Public
  publication/download are verified. Actual launcher extraction and weather
  behavior in game remain pending human validation, not `CLIENT_TESTED`.

Close the game and open `WYDLauncher.exe` to receive the update. A successful
update advances `[VERSION]` in `config.txt` to `10004`; do not change it manually
to bypass downloading. Existing earlier patches remain available in the feed.
The legacy HTTP transport limitations documented above remain unchanged.

## Published patch 10005 (2026-10-13)

The append-only production feed now ends with `10005 patch-10005.zip`.
The package contains exactly one root `WYD.exe`, using the previously validated
Release candidate for persistent buff visual corrections. Skill Amp's ground
aura uses the recorded native scale `1.5` and ground offset `0.4`; other proven
buff parameters, particle timing, weapon emissions and Slow Slash tint are
corrected. No gameplay, wire, assets, player settings or launcher changes are
included. The source is `2f2381f9` plus the uncommitted client changes, not a
new committed source release.

- ZIP size: 811,565 bytes.
- ZIP SHA-256: `222FF65C13BFD9DF1F5A9556A060DD1BE7FF59688DCE6CBF5F892601ABC7A89A`.
- `WYD.exe` size: 1,848,320 bytes.
- `WYD.exe` SHA-256: `55C4A3D2E19404CC4E2ED9F529E7407CF8209B00EE7BD08FB4F44569BCEAF977`.
- Existing build evidence: 1,200,844 architecture checks, 231 socket checks,
  56 launcher checks and costume/skin-shader checks passed. The unchanged
  candidate was hash-verified rather than rebuilt for publication.
- Publication used an explicit single-file ZIP and
  `Publish-ClientPatch.ps1 -Version 10005`; prior patches remain available.
- Fresh public HTTP feed and HTTPS ZIP downloads returned 200. The downloaded
  archive contained only `WYD.exe`; both ZIP and executable hashes matched.
- Website source receipt and regression assertions now describe 10005.
  `npm run build` and the two `public-content.test.mjs` tests passed. Only
  `index.html`, `download/index.html`, `changelog/index.html` and
  `client-manifest.json` were replaced in the existing static website;
  all referenced shared asset filenames already existed on the VPS.
- Public home, download, changelog and JSON receipt returned 200 with version
  10005. Rollback pages are retained at
  `/srv/wyd-go/backups/site-patch-10005-20261013/previous-pages.tar.gz`.
- Game PID `1175700` and start time `2026-10-13 18:10:20 UTC` were unchanged
  across publication. No game/API restart or local runtime installation was
  performed. SSH upload encountered a transient timeout; publication resumed
  only after a TCP probe confirmed port 22 was reachable again.

Validation: `STATICALLY VERIFIED`, `AUTOMATED TESTED`, build and public
download/deployment verified. Actual launcher extraction and in-game comparison
remain pending, not `CLIENT_TESTED`. Full buff composites and runtime emission
performance are not certified by this patch.

Close the game and open `WYDLauncher.exe` to receive 10005. Do not manually
advance `[VERSION]` in `config.txt`, because that would skip the download.

## Published patch 10006 (2026-10-13)

The production feed ends with `10006 patch-10006.zip`, containing the Release
`WYD.exe` and `ItemList.bin` for the
[imported costume and mount presentation](port-748.md#imported-costume-and-mount-presentation-2026-10-13):
costume and KR mount icons, the KR mount class mask, the mount speed tooltip
and the LavaGolem/LavaGrub replacement looks. The matching server catalog was
deployed in release `20261013T202540Z`. The source is `2f2381f9` plus
uncommitted changes.

- ZIP: 859,565 bytes, SHA-256 `F486634EBDFD70D30C8C0B21C6B6DD13D8F097F4685BC44BF0CC5D65A84DB797`.
- `WYD.exe`: 1,848,320 bytes, SHA-256 `6739E415AFB3B5AAF5D7F9813F02253DEDD6C2545F1BF7E3626548234F9245D3`.
- `ItemList.bin`: 910,004 bytes, SHA-256 `905138CD5E1634F35798E8DE0E27CD655EAAA45B3CF5013257CDB128E6E54620`.
- Evidence: 1,200,850 architecture checks, 231 socket checks and 56 launcher
  checks passed; the public HTTP feed returned 200 and a fresh download matched
  the ZIP hash. Launcher extraction and the in-game result are pending, not
  `CLIENT_TESTED`.

## Published patch 10007 (2026-10-13)

The feed ends with `10007 patch-10007.zip`, containing one root `WYD.exe` with
the native -45 degree minimap orientation and the native UI2 scrollbar sets
423..426 ([port record](port-748.md#ui2-vertical-scrollbar-2026-10-13)). The
source is `2f2381f9` plus uncommitted changes.

- ZIP: 811,864 bytes, SHA-256 `D8E8F2612DAD0BFC7E8BC3478C512732484DEF551644D4C8A6A70F668C5193F4`.
- `WYD.exe`: 1,848,832 bytes, SHA-256 `DF8B961699234153D01A91A153152ACE696F2A1102134CCBD129A9C7C1ECB10C`.
- Evidence: 1,200,852 architecture checks, 231 socket checks and 56 launcher
  checks passed; the public feed returned 200 and a fresh download matched the
  ZIP hash. Launcher extraction and the in-game result are pending.

## Published patch 10008 (2026-10-14)

The feed ends with `10008 patch-10008.zip`, containing only
`Env/AttributeMap.dat`. It reopens the city-war arena's central corridor: the
KR/OnGame attribute map marked cells x 136..243, y 188..191 with the
cannot-go bit `0x02`, an invisible wall the native 7.48 map did not have.
Only that bit was cleared in those 27 cells, in both the client map and
`wydgo748/data/maps/AttributeMap.dat`; the arena's outer walls and the 4-byte
client trailer are unchanged, and the two map bodies remain identical.

- ZIP: 10,960 bytes, SHA-256 `328EA9AEC023D334E6A015F5139DB337B7F5844F25C75492CFFF938CC426572D`.
- `Env/AttributeMap.dat`: 1,048,580 bytes, SHA-256 `8A5CC851360D0A1F8209CB13CD0E6A5B092B54E0982FADCC8406DAB8E83F152F`.
- Evidence: Go `TestCityWarArenaCentralCorridorIsOpen`, the full Go suite and
  the client asset tests (client/server attribute equality) passed; the public
  feed returned 200 and a fresh download matched the ZIP hash. In-game walking
  across y 191/192 is pending.

## Published patches 10009 and 10010 (2026-10-14)

Both contain one root `WYD.exe` ([port record](port-748.md#terrain-borders-and-map-objects-2026-10-14)).

| Patch | ZIP bytes | ZIP SHA-256 | `WYD.exe` SHA-256 |
| --- | --- | --- | --- |
| 10009 | 811,991 | `7735DF38349C4FCDF38C07BAC97F9CDE4251E74DD852079CCF67AF02EC329C81` | `255B3036AC12DB2E35CCFB35D8C1DC676F3AC7F97220F94E45D8F4B803B4F3C5` |
| 10010 | 812,476 | `5B4926D6B579962E4F50F4E5A3F7EC46C2E308F9CAD8C68D81B21E2A86572699` | `090FB07A19D43963C10BD938D5975C100BE44E6D5E84DB314688BC719EF78010` |

10009 derives terrain attachment for the 26 KR-only regions; 10010 re-seats map
objects and gate collision after a height-window rebuild. Both were verified
by public download; in-game confirmation is pending.

## Published patch 10011 (2026-10-14)

The feed ends with `10011 patch-10011.zip`. The archive contains only root
`WYD.exe`, built from the current source for citizenship cape emblems and
selection/world metadata refresh. The matching server/site release is
`20261014T011828Z`; the website receipt advertises the same version.

- ZIP: 812,786 bytes, SHA-256 `47544012DAD39F08BAB466A458157F36309FEC8282A62290B148A41007AE7688`.
- `WYD.exe`: 1,849,856 bytes, SHA-256 `FDE1DF8F88CE7D1C7099BE044235F24E6C2C846F40143BD27CDF1AF2B6DC6D10`.
- Reused unchanged build evidence: 1,200,858 architecture checks, 231 socket
  checks, 56 launcher checks, costumes and shaders passed.
- `Publish-ClientPatch.ps1` appended exactly one entry; previous patches were
  preserved. Fresh HTTP feed/HTTPS ZIP and website receipt returned 200, and
  the downloaded ZIP and enclosed executable hashes matched.

No player settings or local runtime executable were overwritten. Close the game
and open the launcher to apply the patch; do not edit the version manually.
Actual launcher extraction, Arch creation, citizenship purchase, owner/observer
cape visuals and selection/relogin remain pending, not `CLIENT_TESTED`.

## Published patch 10012 (2026-10-13, America/Sao_Paulo)

The feed now ends with `10012 patch-10012.zip`, containing only root `WYD.exe`.
It publishes the [cape and character presentation repairs](../../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md):
native mantle texture registration, kingdom icon normalization, guild-directory
names and class-aware skill requirements. Persistent random symbols are already
served by release `20261014T020943Z-cape-emblems`; real citizenship is unchanged.

- ZIP: 818,787 bytes, SHA-256 `E5E051264833A2AB232770B6178EE23BD2F5E453177653EB4D711CA157D2C3B9`.
- `WYD.exe`: 1,860,608 bytes, SHA-256 `F358A2711A2AF7C54EB044F06DFF5B9DDDF457ED14F3F4303D4D4D1B87152319`.
- Unchanged candidate/build evidence reused: 1,205,501 architecture checks,
  231 socket checks, 56 launcher checks, costumes and shaders passed.
- `Publish-ClientPatch.ps1 -Version 10012` installed the ZIP before replacing
  the feed, retaining earlier patches and `.update.htm.previous`.
- Fresh public download contained only `WYD.exe` and matched both hashes.
  Legacy plain-HTTP feed, ZIP and guild endpoints returned 200 without redirects.
- Website build and two public-content tests passed. Only home, download,
  changelog and JSON receipt were updated; deployed shared assets were verified.
  Previous pages: `/srv/wyd-go/backups/site-patch-10012-20261014T021647Z/previous-pages.tar.gz`.
- Public pages and receipt advertise 10012. Game PID `1262340` and API PID
  `1262342`, both started at 02:09:44 UTC, remained unchanged; readiness passed.

No game/API restart, local runtime overwrite, Git commit or push was performed.
Publication/download are verified; launcher extraction and actual cape, kingdom,
guild and Foema tooltip visuals remain pending, not `CLIENT_TESTED`. Close the
game and open `WYDLauncher.exe`; do not manually advance `config.txt` version.

## Published patch 10013 (2026-10-14)

The feed now ends with `10013 patch-10013.zip`, containing only root `WYD.exe`.
It shows Critical in tenths of a percent up to 100.0% (the
[critical scale extension](../../.agents/research/client748/flows/ui/score-hud-shortcut-counters-exp.md))
and carries the stack-overflow-safe crash capture with module offsets.
It must stay paired with server release `20261014T040646Z`.

- ZIP: 820,714 bytes, SHA-256 `686C033A260EAFE13C946F3BE378C8576BBAD1A5A8C48C057D077892E095693C`.
- `WYD.exe`: 1,863,168 bytes, SHA-256 `D8E8403CA724520A8C73FB53235DBB41FF357AE961C1C7452FA57D2FB800F19B`.
- Release build: 1,205,502 architecture, 231 socket, 83 crash-capture and
  56 launcher checks passed.
- Fresh public download contained only `WYD.exe` and matched both hashes.

Not `CLIENT_TESTED`: the Character panel value and launcher extraction remain
manual checks.

## Published patch 10014 (2026-10-14)

The feed now ends with `10014 patch-10014.zip`, containing only root `WYD.exe`.
Enlarging the chat with `+` now resizes the UI2 scroll bar: the down arrow
follows the new bottom and track set 426 stretches to the full height.

- ZIP: 820,823 bytes, SHA-256 `25DE3C3C75C0E7274463F07F3BFC2FF7F638A9F75F051F2CB8204CC67FA6E68F`.
- `WYD.exe`: 1,863,680 bytes, SHA-256 `96B3EAB114C00AA1A9B2B3D0A737B0E883B325BF47ECA74F160A78D97C49A630`.
- Release build: 1,205,503 architecture, 231 socket, 83 crash-capture and
  56 launcher checks passed. Fresh public download matched both hashes.
- Site receipt advertises 10014 through release `20261014T041157Z` (server
  binaries unchanged; services restarted with zero game connections).

Not `CLIENT_TESTED`: the enlarged chat scroll bar remains a manual check.

## Published patch 10015 (2026-10-14)

The feed ends with `10015 patch-10015.zip`: `WYD.exe` (KR/7.48 rider offsets,
new mount visual rows) and `ItemList.bin` (40 new KR mount rows). Paired with
server release `20261014T045628Z` (catalog, ShopKRMt02..04 and website News).

- ZIP: 869,881 bytes, SHA-256 `72CF50DAB1EA3E282AB305643B804E477E7CED1BA4CCEB7CD7078DD3E0301705`.
- `WYD.exe`: 1,865,216 bytes, SHA-256 `B5A197CA363ABFEE902C31D4D8A8E4608641350A32A3038C7CF963B177C51C22`.
- `ItemList.bin`: 910,004 bytes, SHA-256 `C8DFE313EA11561A61F211CEE07E36942A9737A62B4413434AF047E9F3E3AB29`.
- Release build: 1,205,544 architecture, 231 socket, 83 crash-capture and
  56 launcher checks passed. Fresh public download matched all three hashes.

Not `CLIENT_TESTED`: rider height and the new mount models remain manual checks.

## Published patch 10016 (2026-10-14)

The feed ends with `10016 patch-10016.zip`: `WYD.exe` (costume texture
registration, native costume delegation, KR sets 4187/4188) and `ItemList.bin`
(eight costume rows). Paired with server release `20261014T083646Z`
(refinement lock, catalog, ShopCostum06 and website News).

- ZIP: 870,547 bytes, SHA-256 `07E79E677FCD4C9DE3DEB2CEE8B1CBC0AB7AAB1325B06DF475B3C2A2C6D9D733`.
- `WYD.exe`: 1,867,776 bytes, SHA-256 `F088B4486F5B838A86259F0112255F42E39B6726D3ECB2F53E9C207EA311C358`.
- `ItemList.bin`: SHA-256 `7494F56ECAC6CB17066D6A3B28F3F75B44E8D6A443B6DF7380F6B615984CB3C1`.
- Release build: 1,208,413 architecture, 231 socket, 83 crash-capture and
  56 launcher checks passed; costume table check 143/129/176. Fresh public
  download matched all three hashes.

Not `CLIENT_TESTED`: costume textures and the new sets remain manual checks.

## Published patch 10017 (2026-10-14)

The feed ends with `10017 patch-10017.zip`, containing only `WYD.exe`:
minimap header label and native compact coordinates, and Wolf/Bear form
animations. Paired with server release `20261014T160122Z`.

- ZIP: 822,162 bytes, SHA-256 `8EB7AF54943E7581A8310861B2DDAB49E1BBC8F4DFF1DEAFE97EA68444BDA576`.
- `WYD.exe`: 1,867,776 bytes, SHA-256 `7B002D014C54F0EA42195162503CC174C03A3D9A24478531ACCABA0F9A42C395`.
- Release build: 1,208,415 architecture, 231 socket, 83 crash-capture and
  56 launcher checks passed. Fresh public download matched both hashes.

Not `CLIENT_TESTED`: minimap header and transformation animations remain
manual checks.

## Published patch 10018 (2026-10-14)

The feed ends with `10018 patch-10018.zip`, containing only `WYD.exe`: the
local `/fps` toggle for FPS and TCP round-trip time under the minimap.

- ZIP: 822,869 bytes, SHA-256 `BA739C5E981B8AC4353A89E9FCB766F203087DDA00672A1D26A1418A0DA71B67`.
- `WYD.exe`: 1,869,824 bytes, SHA-256 `329D725EF4FBF435A3C3ECD3410CE3D04EA124F2FFF76DD906B386F3AB70C43C`.
- Release build: 1,208,416 architecture, 231 socket, 83 crash-capture and
  56 launcher checks passed. Fresh public download matched both hashes.

Not `CLIENT_TESTED`.

## Published patch 10019 (2026-10-14)

The feed ends with `10019 patch-10019.zip`, containing only `WYD.exe`: cape
refinement glow, open Kefra gate floor restoration and the v1 boss-state mirror
contract. It is paired with server release `20261014T181536Z`; use the launcher
before validating the coordinated mirror behavior.

- ZIP: 823,540 bytes, SHA-256 `C290EAA6B6311D8240B0C5B396C3F488F5109324A06D90938FC94463EABC56CA`.
- `WYD.exe`: 1,870,848 bytes, SHA-256 `3BDD1F7E340AB7AD541485C526CF0AF10635179BC856A5F8349106E88AC3B706`.
- Fresh public download matched both hashes and the website manifest; News,
  changelog and home page returned 200. All previous feed entries were retained.
- The unchanged client candidate reused its Release validation: 1,208,581
  architecture, 231 socket, 83 crash-capture and 56 launcher checks, costume
  gates 143/129/176 and four shader variants. The shared-fixture follow-up
  passed 1,208,582 architecture checks.

This is an explicit-byte distribution from the uncommitted working tree at
`2f2381f9a8e77f23eb07bb10b326df76568d3bdb`, not a committed source release.
The Git-diff patch builder cannot include this uncommitted candidate; packaging
used the exact validated executable without committing generated binaries or
other people's changes. No local runtime executable was overwritten.
Not `CLIENT_TESTED`: cape brightness, doorway movement and boss alive/dead/reset
visual acceptance remain pending. Publication does not certify the global
English migration or overall release readiness.

## Published patch 10020 (2026-10-14)

The feed now ends with `10020 patch-10020.zip`, containing only `WYD.exe`.
It registers the missing Uxmal prompt-table block `(2364,3924)` for the hall
mirror, retaining the existing 700 gold fee and server-side defeat check.
There is no wire or asset change. Server release `20261014T184657Z` separately
deploys the requested temporary 1 HP profile and global +100% combat EXP while
Kefra remains defeated; it does not clear the already persisted kill.

- ZIP: 823,488 bytes, SHA-256 `AEF03DE00BA5E05127CEF380BD562E7CAD1D0CE4BB3E5E4D0EC0715ABE372784`.
- `WYD.exe`: 1,870,848 bytes, SHA-256 `E291656AF11620EA1535C25F9381D19EF53A220E74ECDBB4B99650C1613428E6`.
- A fresh HTTPS download matched both hashes and contained exactly `WYD.exe`.
  The legacy plain-HTTP feed and ZIP also returned 200 without redirects;
  the 823,488-byte HTTP download matched the same ZIP hash.
  The feed retained all earlier entries; the manifest, News, home, download
  and changelog returned 200 with the new receipt. Portal build/check and all
  60 tests passed.
- The unchanged candidate reuses the successful incremental Release gates:
  1,208,652 architecture, 231 socket, 83 crash-capture, 56 launcher checks,
  costume checks and four shader variants. The table regression reproduced
  the missing-row failure before the repair and passed afterward.

As with 10019, packaging distributes the exact validated bytes from the
uncommitted working tree at HEAD `2f2381f9a8e77f23eb07bb10b326df76568d3bdb`.
The Git-revision patch builder cannot represent the ignored candidate, so this
is not a committed source release. No generated binary was committed and the
local runtime executable was not replaced. Actual passage use on 10020 remains
pending; this is not `CLIENT_TESTED` or global release-readiness certification.

## Published patch 10021 (2026-10-14)

The feed now ends with `10021 patch-10021.zip`, containing only `WYD.exe`.
It keeps an item over the FieldScene2 receptacle in occupied player-trade
slots instead of an opaque black backdrop, and adds two diagnostics lines
(`npc click` and `mix npc click`) that record head, Merchant, ground chunk and
the six artisan panel pointers. After updating to 10021 the user reported
that the craft NPC windows open again.

- ZIP: 823,460 bytes, SHA-256 `CA90400CF7506C4DC3FDCEF39742E8944A5FC122220296D386683F5BFF2CE201`.
- `WYD.exe`: SHA-256 `9C1F5EFAE99DA4026315937FDD87F47D0139452005D7CD9AE3856CA9E56C2FE9`.
- A fresh plain-HTTP download matched both hashes and contained exactly
  `WYD.exe`; the feed retained every earlier entry.
- Incremental Release build only; the architecture test executables were not
  rerun for this patch. Portal build and all 60 tests passed.

The server part was initially staged as release `wyd-deploy-20261014-test-shops-10021`
(SHA-256 `7185f7d5dfa4c91eaac27891ab84383c4836979a1e5e0d40513107f0c8f2669c`)
and deferred while players were connected. It restricts dye to armor
set pieces, routes `@` chat to the whole channel and reports `@@` without
citizenship, adds the `ShopArchSet`, `ShopArchWpn` and `ShopAmulet` test
NPCs, and sells 120-unit Ori/Lac stacks in `ShopTest2`. Not `CLIENT_TESTED`.
These changes and the later Lac follow-up are now deployed in server release
`20261014T223717Z` (see the publication below).

## Published patches 10022 and 10023 (2026-10-14)

Patch 10022 was already public when the owner requested publication of the
remaining work. Its ZIP is 824,071 bytes, SHA-256
`F81305DF34C715A2330BBFC60CA4CCDE643256F7CECC9C01E6B3A574D96549AD`;
its executable SHA-256 is
`B70A299EE966FB77B068B2E9843D01E2D65795407FEECFA2D1B43A932BED4E29`.
It was not overwritten or republished.

The feed now ends at `10023 patch-10023.zip`, containing exactly `WYD.exe`:
the native Aylin preview combinations/refinement setter and temporary preview
clearing; Diamond +10 UI, native Garnet glow, and refined imported weapon
variants 2889–2901. Fixed-glow models 2906–2909 remain unchanged. The reported
item-drop crash repair is explicitly excluded.

- ZIP: 824,039 bytes, SHA-256 `E7053B354487ABA9DE959D73BAC21AEDC8196EA31A8D532950D7174B89167340`.
- Executable SHA-256: `18B87EE8B32F6333BD0838C46E0BED85F0FD2B172A7C2AF61804EED054ECC418`.
- Fresh plain-HTTP download matched both hashes and the manifest. All feed
  entries 10000–10023 were retained; public News and readiness returned 200.
- Unchanged candidate validation was reused: 1,208,967 architecture checks,
  231 socket, 83 crash-capture, 56 launcher checks and incremental Release.
  Portal build and all 60 tests passed for the updated publication metadata.

The exact validated executable was packaged from the uncommitted tree at
`2f2381f9a8e77f23eb07bb10b326df76568d3bdb`, not from committed Git revisions.
No source commit, local runtime replacement or game launch occurred. The
installed local executable remains 10021 until the owner uses the launcher.
This shipment is not `CLIENT_TESTED` or certification of global release readiness.

## Published patch 10024 (2026-10-14)

The owner authorized the pending batch and player disconnections. Patch 10024
contains only `WYD.exe`: native mount-level glow, player trumpet chat separate
from server top notices, party Leave/Escape controls, Mystic Amulet equipment
and shared item-icon fitting. Server release `20261014T234237Z` supplies mount
magic amplification, normal Lac +10 earrings, tower guards/position and 49
mortal D/E test weapons with +28 magic or +63 physical damage.

ZIP: 824,185 bytes, SHA-256
`24F3AFBEA36E1E8754D6520E69EE8B1468B007B9DFFF095ECD882C1D981D2E09`.
Executable SHA-256:
`DD357D0EACD58882FAF872FCE645BDF9A93B196C6853DAD8D1521DF1A15E76F6`.
The validated uncommitted build was packaged directly, not through the
Git-revision patch builder. No source commit or local runtime replacement
occurred. Unchanged client checks were reused. The site was rebuilt and all
60 tests passed. Public plain-HTTP ZIP and embedded executable hashes, manifest,
News and append-only feed through 10024 were verified. A transient SSH timeout
was recovered by resuming the existing remote staging; earlier patches remain
unchanged. No game was launched; in-game confirmation remains pending. The
item-drop stack overflow is not fixed by this patch.

## Published patch 10025 (2026-10-14)

The feed ends with `10025 patch-10025.zip`, containing only `WYD.exe`. 3D item
icons keep render block 1 alpha blending and alpha test, as WYD.exe 7.48
FUN_0042E8A1 does, and unrefined icons use `Render(0, offset)` as in
FUN_004BBCB5; shining items such as the Glittering Stone were drawn opaque.

- ZIP: 824,258 bytes, SHA-256 `85B1F24F818268937259FEE6DFBC29B291F21445226945C978F204B6E7944291`.
- `WYD.exe`: 1,871,872 bytes, SHA-256 `192417E70B559B0E24013DD33BABA976926CCC1936F20F0AB32B5E7FABE69FEF`.
- A fresh HTTPS download matched both hashes and contained exactly `WYD.exe`.
- Release client gates passed (architecture, socket, crash-capture, launcher).
  Portal build and all 60 tests passed.
- Website: only the six changed public pages (home, manifest, download, News,
  the new article and changelog) were installed into the live release, with a
  backup at `/srv/wyd-go/backups/site-before-10025-20261015T015821Z`. No
  service restarted. Not `CLIENT_TESTED`.

## Published patch 10026 (2026-10-15)

The feed ends with `10026 patch-10026.zip`, containing only `WYD.exe`: guild
marks downloaded off the render thread from
`http://wyd.forjago.com/patch/guildmark/` (textures created on the render
thread, bounded BMP header, marks for visible characters from CreateMob), C.C.
rebuff inside the last 24 seconds of a buff, and mob scale reading the native
signed-short Con.

- ZIP: 825,170 bytes, SHA-256 `33A113999984554405BD592E504FCAD684B186D4E1135E08FE323655DC2D9E87`.
- `WYD.exe`: 1,873,920 bytes, SHA-256 `12C38CFBF29CE900044EA6B2E4F74916BB2B906F2FDBFF9EA27E45BA66BDE583`.
- A fresh HTTPS download matched both hashes and contained exactly `WYD.exe`.
- Release client gates passed (architecture 1,209,017 checks, socket,
  crash-capture, launcher). Portal build, `astro check` and 60 tests passed.
  Paired with server release `20261015T023211Z`. Not `CLIENT_TESTED`.

## Published patch 10027 (2026-10-15)

Client-only publication, explicitly without a server deployment or restart.
The 25-file allowlist contains `WYD.exe` and 24 restored meshes for eight
ShopCostum06 costumes. It includes the inventory grade-texture lookup and
fixed inventory-lighting profile, retired-actor CreateMob reentry and weapon
animation refresh after dual-to-single equipment changes. Server-side Ehre
and private-instance HUD fixes are excluded and remain pending.

- ZIP: 1,116,039 bytes, SHA-256 `A8768BFC2956F6546ADE2B9E9640BEFD006AE4FE4953CEB4FCA111A8BE3C0DF6`.
- `WYD.exe`: 1,875,968 bytes, SHA-256 `F3E40F97FD9F3B47EFECE890CFB117E0486B93B76C5F99BB4E13134BE4DF8855`.
- The unchanged, validated candidate was packaged explicitly from the dirty
  working tree at HEAD `fa1c853fa9d279719acd4aa0c3518a550505134f`, not from
  committed From/To revisions. Personal `config.txt` and build-only costume
  audit metadata were excluded. No commit or local runtime replacement occurred.
- `Publish-ClientPatch.ps1 -Version 10027` appended the feed while retaining
  older patches and its previous-feed rollback. Remote ZIP SHA-256, fresh
  HTTP/HTTPS ZIP hashes and all 25 embedded file hashes matched the sources.
- Reused candidate gates: ArchitectureTests 1,209,240 checks and incremental
  Release/x86 build; unchanged socket, crash-capture, launcher and shader gates.
  The receipt's two public-content tests and Astro build passed.
- Only home, download, changelog, manifest, News and the new article were
  installed in the existing static site. All three referenced shared assets
  already matched the VPS. Public pages returned 200 with 10027; the manifest
  matched the ZIP receipt. Previous pages are retained at
  `/srv/wyd-go/backups/site-before-10027-20261015/previous-pages.tar.gz`.
- Current remains `/srv/wyd-go/releases/20261015T023211Z`. Game PID `1510633`
  and API PID `1510645` retained start time `2026-10-15 02:32:14 UTC`.
  No service reload/restart or player disconnection was performed.

The owner then tested 10027 and reported that icons still lack brightness,
with Glihtning Stone visible only as sparse dark/gold pixels. This is negative
visual acceptance, not a successful `CLIENT_TESTED` result. The reported icon
defect remains open; publication and passing automated checks do not close it.

## Published patch 10028 (2026-10-15)

Client-only follow-up to the owner's negative 10027 icon test. The constructor
now selects the native item halo (including white/effect 273 for item 3140),
and the existing halo renderer uses the native sine pulse, unchanged alpha
and full-width UV. No server changes or asset replacements are included.

- ZIP: 826,278 bytes, SHA-256 `D093974C2CE408AAD0E3737F27FEF48EB19025C5026DEB7FC3BE831A9F9A187E`.
- `WYD.exe`: 1,876,480 bytes, SHA-256 `75868D55FA061A5256F8A213785D2B6A81EF563BCDD26D161FE4D5BFB99CA2FD`.
- Explicit single-file packaging from the tested incremental output in the
  dirty working tree at HEAD `fa1c853fa9d279719acd4aa0c3518a550505134f`,
  not committed From/To revisions. No commit or local runtime replacement.
- `Publish-ClientPatch.ps1 -Version 10028` appended the feed, preserving old
  patches and the previous-feed rollback. Fresh HTTP and HTTPS downloads
  matched ZIP and embedded executable hashes; feeds returned 200 with 10028.
- ArchitectureTests 1,221,246 checks; socket 231, crash capture 83, launcher
  56, shader/costume gates and incremental Release/x86 build passed.
- Current remains release `20261015T023211Z`; game PID 1510633 and API PID
  1510645 retained their 02:32:14 UTC start time after publication.
- No server restart, disconnection, account write or website update.
  The static website still advertises the previous 10027 receipt.
- CLIENT-TESTED pending: request an updated in-game inventory screenshot.

## Published patch 10029 (2026-10-15)

The feed ends with `10029 patch-10029.zip`: `WYD.exe` plus the six female
Skeleton Set meshes `mesh/ch020190.msh`..`ch020690.msh` restored from the
read-only KR reference. Client changes: Samaritan self-only, C.C. rebuff
continues past a buff that does not cast, KR Carnival Mount fallback visual,
earring gem highlight, guild-mark border contract, and the elite cape icon
colors prepared earlier.

- ZIP: 895,630 bytes, SHA-256 `8DEDE4BA296A9756A1B954C85464283AE5838E5065369A681AD51B0A5E8E3C70`.
- `WYD.exe`: 1,876,480 bytes, SHA-256 `21F67FBC625D6A52868D57ECAE35372AA97382E5D6209D9D428E43B15E51CFB6`.
- A fresh HTTPS download matched both hashes and contained 7 entries.
- `Build-Client.ps1 -NoDeploy`: ArchitectureTests 1,227,750 checks, socket 231,
  crash capture 83, launcher 56. Paired with server release
  `20261015T215714Z`. Not `CLIENT_TESTED`.

## Published patch 10031 (2026-10-16)

Paired with server/site release `20261016T032951Z`. This explicit two-file
patch contains the validated executable and required, rewrapped skill corpus;
unchanged assets remain in previous cumulative patches. It excludes personal
configuration and launcher binaries. Source is the dirty working tree at
HEAD `fa1c853fa9d279719acd4aa0c3518a550505134f`, not a committed release.
No generated executable was committed and no Git index changes were made.

- ZIP: 832,800 bytes, SHA-256
  `38D1B4C3B069A3F19F61E6280EF697FA53BE0E6631FA1617DAD59F1CDD1B03B2`.
- `WYD.exe`: 1,881,088 bytes, SHA-256
  `2F75CA77281CCF21F899A6879DBB1A353C6FBE69A98A2F24C52D6E9309A51F1A`.
- `skillhelp.dat`: 19,309 bytes, SHA-256
  `1BDD96ADC246CF2A71097301EA1C5152D24F5109A43A557A22B0C3335E3DDA24`.
  All 102 IDs and description prose match 10030; 72 records only rewrap lines.
- `/tab`, `/tab2`..`/tab9` and `/snd` use the paired client/server contracts.
  Samaritan and Critical Armor use their respective skill icons. Previous
  skill-window, menu and C.C. fixes remain included in the cumulative update.
- `Publish-ClientPatch.ps1` appended 10031. Both public HTTP/HTTPS feeds and
  downloads returned 200; ZIP and embedded hashes match. The remote ZIP hash
  matches, the prior feed is preserved byte-for-byte as the retained prefix,
  and patch 10030 is unchanged. The site manifest, home and News advertise 10031.
- The server and player API restarted at `2026-10-16 03:29:54 UTC`; Caddy
  restarted one second later. Running binary hashes, readiness, backup and
  rollback checks passed. See [current state](../current-state.md).
- Reused the unchanged Release/x86 build and Go/vet evidence. Linux release
  cross-build passed, as did portal checks, 60 existing tests and the two
  receipt tests after the updated static build. No local game was launched
  or runtime executable replaced. CLIENT-TESTED remains pending.

Use `WYDLauncher.exe` with the game closed. Nightmare, Backpack and existing
English-migration debt are not declared complete by this publication.

## Published patch 10032 (2026-10-16)

Nightmare counter-label update paired with authoritative server lifecycle
corrections. Explicit working-tree payload: only `WYD.exe`; earlier assets
remain supplied by the cumulative launcher feed. Source HEAD on `main`:
`de87cd8789bef3165babbdf41fe1f86bc449106a`, with the pending Nightmare changes.
No commit, push or local runtime replacement was performed.

- ZIP: 829,102 bytes, SHA-256
  `60EB65FCD22EF96CE47337897999F409A927D4BA22365D2229616CB11191B411`.
- `WYD.exe`: 1,881,088 bytes, SHA-256
  `E96B8514B8B5CEC3A6C7D832DE8E0FE3AA11F4A390816A74C96A34285CCB5A36`.
  Candidate: `tmproject/build/TMProject748/Release/WYD.exe`.
- `Publish-ClientPatch.ps1` appended 10032 without altering earlier entries.
  Public HTTP/HTTPS feeds and ZIP downloads returned 200; ZIP and embedded
  executable hashes match. Patch 10031 is unchanged and the previous feed
  remains exactly the new feed's retained prefix.
- Site manifest, home and News advertise 10032. Active server/site release
  `20261016T183715Z` includes all 21 new protector templates. Game/API restarted
  at `2026-10-16 18:37:18 UTC`; actual running hashes and readiness passed.
  Backup and rollback references are recorded in
  [current state](../current-state.md).
- Reused the unchanged Release/x86 candidate and its architecture/socket/
  crash-capture/launcher gates. Fresh full Go tests, vet, Linux cross-build,
  portal build, 60 portal tests and portal checks passed. Actual in-game
  validation remains pending; publication is not `CLIENT_TESTED` evidence.
- Upload staging was removed. Local artifact cleanup was blocked before
  execution; exact leftovers are recorded in current state. Active delivery
  and rollback verification succeeded independently of cleanup.

Use `WYDLauncher.exe` with the client closed. The counter now reports protected
NPC survivors rather than a quota of hostile monsters. The server separates
four-minute admission from 15-minute combat, continuously respawns common
monsters, applies the approved EXP loss per dead protector and spawns Arcane
Lugefer once with five minutes remaining. Exact native population and loot
parity are not certified by this delivery.

## Published patch 10030 (2026-10-15)

Client-only publication, with no server deployment or restart. The explicit
35-file allowlist contains the validated `WYD.exe`, required `skillhelp.dat`,
`UI/UITextureSetList.txt`, `Costumes-KR.json`, `Mounts-KR.json` and 30 restored
costume meshes. Some costume dependencies were already published and are
included again unchanged. Personal configuration and launcher files are absent.

The executable includes the 102 available native skill descriptions faithfully
localized to English, separate prose/metadata controls and bounded tooltip
placement, the corrected appearance-update transformation-effect guard and
the pending C.C. client logic. The atlas definitions restore all sidebar button
states. Skill IDs without native prose still retain calculated captions rather
than fabricated descriptions. Server-authoritative guild kingdom, seal and
weapon promotion changes are not deployed by this client patch.

- ZIP: 1,216,419 bytes; SHA-256
  `F08898C442024C754C5CE0DE8D82EA374917711A1A331340549F471CCD4E3461`.
- `WYD.exe`: 1,880,576 bytes; SHA-256
  `3F38F354E4108FDE1C24BE15E456DF2B77D9536429E16DC13C8CAF2B192A5429`.
- `skillhelp.dat`: SHA-256
  `D614DA42CE36B2C2CA61DEC6BF4DF964D047C28A4A3B618D53695C26FC967629`.
- Packaging reused the unchanged incremental output and an explicit runtime
  allowlist in the dirty tree at HEAD `fa1c853fa9d279719acd4aa0c3518a550505134f`.
  ZIP bytes came directly from those files, not committed From/To revisions;
  `Build-ClientPatch.ps1` was therefore not used. No commit, push, index change
  or local runtime executable replacement occurred.
- `Publish-ClientPatch.ps1 -Version 10030` appended the production feed.
  All 35 local archive entries matched their sources. Fresh HTTP and HTTPS
  feeds and ZIPs returned 200; both downloads matched the ZIP hash and all
  embedded file hashes. Remote ZIP size/hash matched. The previous-feed
  rollback matches the old prefix, and patch 10029 remains unchanged.
- Reused gates: ArchitectureTests 1,228,509 checks, socket 231, crash capture
  83, launcher 56, shader/costume validation and Release/x86 build. No new
  product build was needed because its inputs had not changed.
- Current remains release `20261015T215714Z`. Game PID 1694828 and player API
  PID 1694841 retained their `2026-10-15 21:57:17 UTC` start time; readiness
  returned `ready`. No service restart/reload or player disconnection.
- Remote staging was removed. Local cleanup was rejected before execution;
  `build/patch/patch-10030.zip`, `build/patch/verify-10030-http.zip` and
  `build/patch/verify-10030-https.zip` remain in the sanctioned ignored root.
  Existing build output and previous patches were preserved. No static website
  receipt update was made.

CLIENT-TESTED remains pending for description content/layout, uninterrupted
buff visuals, legitimate transformations and the updated C.C. behavior.
Use `WYDLauncher.exe` with the client closed to install the executable and its
required skill asset together. Existing Portuguese item-resource migration
debt remains outside this scoped publication; this is not global release
certification.

## Published patch 10034 (2026-10-17)

Explicit working-tree publication from dirty `main` at
`de87cd8789bef3165babbdf41fe1f86bc449106a`: only `WYD.exe`, 1,881,088 bytes,
SHA-256 `4183973219F67EC0F22D29276E42541DD9793FA1EB0A54BADFF7919938F60C41`.
The canonical validated build was reused; interface resources are unchanged
from 10033. No private client settings, launcher, version marker or other assets
are included. Earlier dependencies remain in the cumulative feed.

The executable restores the native generic transport prompt for portal tiles
without a named client table entry, with bounded table access. The paired
Hell Gate reconstruction is active in server release `20261017T221343Z`, after
authorized game/API restart. It uses twenty minutes total, two-minute entry,
400,000 HP bosses and a 10,000,000 completion EXP reward; these event settings
are project choices rather than proven historical native constants.

ZIP: 829,693 bytes, SHA-256
`03D044C9C65A56F9A3005DF0836859EC5EEB1043AD5CD340FBFE8D830FE9A6F3`.
Public HTTP/HTTPS feeds return 200 with the prior feed preserved and only 10034
appended. A fresh advertised HTTPS ZIP and its sole executable entry match.
The website manifest, home, News, article and changelog advertise 10034.
Full Go tests/vet, Linux cross-build, portal build, all 60 tests and Astro checks
passed. Local installation and in-game confirmation remain pending.

Use `WYDLauncher.exe` with the game closed. See
[current state](../current-state.md#previous-release--2026-10-17-hell-gate-route-and-defense-patch-10034)
for active-process hashes, rollback, backup and cleanup. This is not
`CLIENT_TESTED` evidence or global release certification.

## Published patch 10033 (2026-10-17)

Paired client/server publication from dirty `main` at
`de87cd8789bef3165babbdf41fe1f86bc449106a`. The owner authorized service
restart and player disconnection, not a host reboot, local installation,
commit or push. The explicit payload contains only `WYD.exe`,
`UI/interface.txt` and `UI/interface1.txt`; earlier assets remain available
through the cumulative launcher feed.

- ZIP: 830,698 bytes, SHA-256
  `FC8CB13743F3F1A94D76497C6D1A9B625D643B54C983EE964F1E3878766FB777`.
- `WYD.exe`: 1,881,600 bytes, SHA-256
  `D6CC42D093FE240127F5096FA133965C225AB410D133AC79B8A6094AD8A7FA8D`.
- `UI/interface.txt`: SHA-256
  `407857BFE8CD47BBA5CA2F1379CCF84AC38E0865A26F38423D6A74DFC5FE9B93`.
- `UI/interface1.txt`: SHA-256
  `EC260EDFAED70727771A3C9443D3D063B6BA24DA1846110297FC98D727007468`.
- Double-hit numbers share their trajectory and speed, with a 120 ms
  second-number delay. The server guarantees double hit at 201% attack speed
  and above, not at exactly 200%; the 900 ms action interval remains intact.
  Manual physical attacks continue against their target until death or
  interruption. Number-row 1-0 selects skills, with Shift+1-0 assignment;
  F1-F10 does not select them. Helper down-arrow placement and chat
  PageUp/PageDown scrolling are corrected; other Helper gaps remain open.
- `/saldo` and `/balance` return one private top notification with Donate,
  Fame, Honor, event passes and Kefra entries. Gold, CP and medals are absent.
  `/medals` and `/medalhas` separately report medal balances. Alias names
  accept both languages; displayed text remains English.
- The prior feed is preserved byte-for-byte, with only 10033 appended.
  Public HTTP/HTTPS feeds returned 200, and a fresh HTTPS ZIP download and
  all embedded hashes match. Patch 10032 is unchanged. Site manifest,
  home, News, the new article and changelog advertise 10033.
- Active release: `/srv/wyd-go/releases/20261017T041938Z`. Game/API both
  restarted at `2026-10-17 04:19:41 UTC`; running binary hashes, all 7,334
  payload checksums, readiness and listener checks passed. Backup and
  rollback references, the transient SSH timeout and configuration-preservation
  evidence are recorded in [current state](../current-state.md).
- Fresh complete Go tests, vet, Linux cross-build, portal build, all 60 portal
  tests and Astro checks passed. Four optional server integration tests were
  explicitly skipped. Unchanged client build evidence was reused. Neither
  publication nor these checks establish actual in-game validation.

Use `WYDLauncher.exe` with the client closed. No local runtime was replaced.
Remote staging was removed; local publication output cleanup is tracked in
current state. Existing unrelated gameplay and language-migration debt remain
open; this is not global release certification or `CLIENT_TESTED` evidence.

## Published patch 10035 (2026-10-18)

This working-tree release publishes client stability safeguards, not a claim
that all historical crashes have been reproduced or fixed. It preserves the
wire contract and uses an explicit executable-only patch allowlist; both UI
interface resources already match patch 10033. Cumulative earlier assets are
still required and remain available in the existing feed.

- ZIP: `patch-10035.zip`, 846,770 bytes, SHA-256
  `8172AFA041A234E8C01BD956DEB36591062E70EEE8A99B0A3FFCC62B24C77E2A`.
- Sole entry: `WYD.exe`, 1,907,712 bytes, SHA-256
  `251CF754409F8AE4C0251C9FDD88EDA14C6799E78A377BD0665B6E858FDAFAB0`.
- Matching Release PDB: SHA-256
  `D29C437505BE2CC34A35B474CBDA38032270DBE30F1DA12A4EA69A1D9255252F`.
  Symbols are retained locally, not distributed in the patch.
- Release build, architecture, socket and crash-capture gates passed. Fresh
  public HTTP/HTTPS feeds return 200; the advertised ZIP and executable hashes
  match. Website manifest, home, download, News/article and changelog advertise
  10035. No private settings or local account state are in the patch.
- Server release `/srv/wyd-go/releases/20261018T152826Z` was activated. The owner
  authorized game/API restart and player disconnection. Running binary paths,
  all 7,346 payload checksums, readiness, backup and configuration-preservation
  checks passed. The [activation receipt](../current-state.md#previous-release--2026-10-18-client-stability-safeguards-patch-10035)
  records exact hashes, remaining gameplay validation and blocked local cleanup.
  The subsequent [server-only Hell Gate release](../current-state.md#active-release--2026-10-18-historical-hell-gate-version-3)
  supersedes its server process without changing patch 10035 or the public site.

Close the client and run `WYDLauncher.exe` to install this patch. Publication
does not replace an already running client or the local installed executable.
Mana Control/right-click and item-drop reproductions, heap-corruption root cause
and remaining resource/lifecycle gaps still need actual gameplay evidence.
