# Current project state

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
