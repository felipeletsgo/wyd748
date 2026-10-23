# Bilingual channels and shared client plan

Status: implementation plan with the shared-server product model and the
owner's guild departure and war isolation policies confirmed on 2026-10-23.
This document authorizes no runtime change,
publication, restart, or data migration. The requested product direction is
Channel 1 for Brazilian Portuguese players and Channel 2 for English-speaking
global players. The owner subsequently selected one client installation with
`EN` and `PT` resource folders, a launcher language selector, and a separate
`lang.txt` preference file instead of changing `config.txt`.

Client implementation update (2026-10-23): the shared resource selector,
EN/PT text packs and language selector in the operational native launcher
are implemented, locally installed and automatically validated. The native
launcher retains the website's existing update feed. See
[multilingual client](client/multilingual-client.md) for the
contract, exact artifacts, coverage and remaining gameplay validation. Channel
and server-language work below remains planned.

## Confirmed product and proposed implementation

| Property | Channel 1 | Channel 2 |
| --- | --- | --- |
| Public label | Brazil | Global |
| Game messages and NPC dialogue | Brazilian Portuguese (`pt-BR`) | English (`en`) |
| Client distribution | One shared bilingual client | The same shared bilingual client |
| Gameplay implementation | Shared server code | Shared server code |
| Balance, items, recipes and rewards | Same canonical definitions | Same canonical definitions |
| Maps, monsters and online population | Separate live world | Separate live world |

Distribute one executable, one launcher, one full-client download and one
patch feed from the same source tree and reproducible build pipeline. Include
both language resource sets in every complete installation. The launcher
selects English or Brazilian Portuguese and saves the choice in `lang.txt`.
Language changes select resources; they do not copy translated files over
shared gameplay data or require another installation.

Both channels belong to the same server. Accounts, characters, cargo, currency,
items and guild membership are shared; changing channels does not create a
different character or require an economic transfer. One account has one
active gameplay session across both channels.

Players may choose citizenship 1 or 2, or remain without citizenship (0), and
play in either channel regardless of citizenship. Keep the selected citizenship
when changing channels. Citizenship benefits and NPC requirements remain
separate from channel admission: for example, a citizenship-specific Kibita exchange must not become
a general restriction on entering the other channel.

The shared client can access both channels in either language. The selected
language determines the local interface, not access rights or citizenship.
Keep channel selection independent from the language preference. Server text
follows the destination channel's configured language; switching channels
does not silently replace the selected client language. Player-authored
chat is preserved as written.

### Shared installation and language preference

The proposed installed layout is:

```text
WYD.exe
WYDLauncher.exe
config.txt
lang.txt
EN/
  UI/
  ...localized resources...
PT/
  UI/
  ...localized resources...
...shared gameplay resources...
```

- `lang.txt` contains one token, `EN` or `PT`, with an optional trailing line
  ending. `EN` maps to `en`; `PT` maps to Brazilian Portuguese (`pt-BR`). This
  is a local preference, not a server/channel identifier or build revision.
- Resolve the file and resource roots relative to the installation directory,
  independent of the process working directory. Read the preference before
  loading any localized resource, including initial errors and login UI.
- Implemented compatibility behavior: a missing, malformed or oversized
  preference uses English without invalidating existing settings. Never
  interpolate arbitrary file text into paths.
- Save changes atomically and preserve the previous value on write failure.
  The launcher must report an unsuccessful save before starting the game with
  a different language than the player selected. A direct game launch reads
  the same file without requiring the launcher to run first.
- Apply language changes on the next game launch. Do not modify the running
  game's resource selection. Reopening the launcher restores the saved choice.
- Keep `config.txt`, its `[VERSION]` update marker, the server endpoint files
  and the selected channel independent. Patches must preserve `lang.txt`.
- Store localized presentation resources under `EN` and `PT`, preserving their
  relative loader paths. Shared maps, models, sounds and gameplay definitions
  remain shared unless an audited presentation dependency needs localization.

This layout supersedes the earlier proposal for separate language downloads,
installation folders, launcher identities and feeds. Server-generated text
continues to follow the channel locale specified above; making all server
text follow each player's preference would require a separate session-locale
contract and is not implied by adding `lang.txt`.

In-game channel switching is part of the initial feature scope. The user must
not need to close the client or manually log out and reenter credentials.
The native migration flow may reconnect internally. Logout/relogin remains
a recovery and test path rather than the normal channel-switch experience.
Do not add nationality, IP, client-language or citizenship restrictions to
channel admission. War participation has separate restrictions below.

### Owner-confirmed citizenship and war policies

| Operation | Required behavior |
| --- | --- |
| Enter or switch channels | Allow citizenship 0, 1 or 2 in either channel; preserve citizenship. |
| Use `getout` while in any guild | Reject before changing any state, for ordinary members and leaders alike. Tell the player to leave the guild first. |
| Complete an eligible `getout` without a guild | Persist citizenship 0 until explicit registration; reconnecting or switching channels must not assign citizenship. |
| Register again | Use the destination channel's explicit citizenship registration flow and its validated requirements. |
| Participate in a guild war | The guild's citizenship must match the war's channel. Apply this to City War, Tower War and every other guild-war entry point. |
| Participate as a character | Require a current guild membership and nonzero character citizenship matching the eligible guild, alongside the event's other requirements. |

The canonical English departure rejection is
`Leave your guild before leaving citizenship.` Store it under a message key
and add the approved Portuguese translation during localization. The guild
check takes precedence over other `getout` rejections so every guild member
receives this instruction. A rejected command changes neither gold, individual
fame, citizenship nor guild state.

A Channel 2 guild cannot bid, enter as a participant, attack objectives,
defend or receive participation rewards in a Channel 1 war, even if it has
enough fame for the historical foreign-guild exception. Its members may still
visit Channel 1 for ordinary gameplay. Apply the same rule symmetrically.

### Evidence and deliberate differences

The supplied official-server decompilation is labeled **7.59**. It is the
authority for the historical server findings below, not proof of native 7.48
parity. W2PP is not the authority for this plan. Static paths establish
conditions and state transitions, not successful gameplay or persistence.

| Recovered boundary | Finding and implementation consequence |
| --- | --- |
| TMSRVIN `0x4DFCE0` / `0x505E60` | Personal citizenship is separate from the hosting channel; keep the fields independent. |
| TMSRVIN guild record; DBSRVIN `0x471210` | Guild citizenship is separately persisted, distinct from kingdom and fame. Its updates must reach both worlds through one authority. |
| TMSRVIN `0x519CA0`, `getout` branch | Historical departure has day/city restrictions, removes half of individual fame rounded down, and has leader-specific guild resets. The owner's any-guild rejection replaces the guild departure paths; do not port the leader resets. |
| TMSRVIN `0x555600` | Historical login can inherit guild citizenship or automatically assign a mortal's hosting channel. Do not reproduce automatic assignment that would undo an explicit departure to 0. |
| TMSRVIN `0x512800` / `0x5656F0` | City-war bidding has a foreign-citizenship fame branch; participant selection checks character/guild citizenship. The owner replaces the foreign exception with strict event-channel eligibility. |
| Core.dll `0x100248E0` | The inspected Thor Tower attack hook checks guild presence and personal citizenship against the hosting channel. This is not proof of every tower entry or reward check. Add the owner's guild-channel rule consistently across those operations. |
| TMSRVIN score/drop/EXP paths | The current Go citizenship drop formulas were not confirmed. No generic citizenship EXP or drop percentage is established by this research. |

The existing [server research map](../.agents/research/rea-server-759-runtime-map.md)
indexes the native evidence. Preserve the independent
[7.48 citizenship and cape contract](../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md):
kingdom, cape emblem and citizenship must not become interchangeable fields.
The detailed local investigation is retained in
`build/rea-analysis/results/official-citizenship-system-20261023.md`; this plan
records the decisions needed without depending on that ignored output alone.

## Current implementation boundaries

- [Server configuration](../wydgo748/internal/data/server_config.go) provides
  one listener and a `ChannelID`, but
  [server startup](../wydgo748/cmd/server/main.go) creates one `World`.
- [Account admission](../wydgo748/internal/game/handlers.go) claims accounts
  in a map owned by that `World`. It does not establish exclusive ownership
  between independent worlds or processes.
- [PostgreSQL fences](../wydgo748/internal/store/postgres_fence.go) use
  in-process synchronization. The advisory lock in
  [PostgreSQL startup](../wydgo748/internal/store/postgres.go) protects schema
  initialization, not online account ownership.
- The [schema](../wydgo748/internal/store/postgres_schema.sql) stores guild and
  instance aggregates as singleton rows. Two independent snapshots can
  overwrite each other even when individual database transactions are valid.
- The [guild model](../wydgo748/internal/model/guild.go) has kingdom and fame
  but no separate guild citizenship field. A world-level character check alone
  cannot enforce the owner's guild-channel rule.
- The client already selects channels through `sn.bin` and `serverlist.bin`.
  Initial login and migration currently use the fixed `TM_CONNECTION_PORT`
  value, 8281; assigning another listener port alone is insufficient.
- The [server command handler](../wydgo748/internal/game/commands.go) rejects
  `srv`. The client has a migration receiver in
  [TMFieldSceneSessionPackets.cpp](../tmproject/TMProject748/internal/app/scenes/TMFieldSceneSessionPackets.cpp),
  but that does not establish an implemented server transfer protocol.
- [TMPaths.h](../tmproject/TMProject748/internal/platform/windows/TMPaths.h)
  identifies UI strings, message tables, item-name overlays, help and skill
  files. [WYD748Assets.cpp](../tmproject/TMProject748/internal/core/WYD748Assets.cpp)
  validates their legacy formats. Additional text exists directly in C++ and
  in server responses and content files.
- `WYD748_LoadClientConfig` currently rejects unknown keys in `config.txt`.
  The selected separate `lang.txt` design avoids extending that schema or
  depending on the historical launcher's configuration rewrite behavior.
- The [launcher pipeline](client/launcher.md) currently publishes one feed.
  [Build-ClientPatch.ps1](../tools/deployment/Build-ClientPatch.ps1) packages a
  single runtime tree and excludes the launcher itself: replacing a running
  launcher requires a separate bootstrap or full-client distribution path.

These findings support feasibility, but the existing configuration is not a
safe two-channel deployment on its own.

## Phase 0: define the contracts

1. Encode the confirmed shared-server model in contracts and tests: both
   client languages can enter both channels, citizenship is independent of the current
   channel, and the same account and character state follow the player.
2. Amend the English-only repository policy for approved player-facing
   `pt-BR` and `en` resources. Keep code identifiers, comments, diagnostics,
   internal staff tools and technical documentation in English. Update the
   [language migration record](language-migration.md) to distinguish missing
   translations from text in an intentionally supported language.
3. Adopt the ownership matrix below and define mutation ordering. Do not
   duplicate global rewards or overwrite shared state through two local caches.
4. Specify event time zones explicitly. Preserve current schedules until a
   separate schedule decision is made; the language project must not silently
   move Kibita's historical exchange window.
5. Encode the owner-confirmed policies above, preserving citizenship during
   transfer and separating NPC/war eligibility from channel admission. Audit
   existing channel-dependent rules, including Kibita and client PK
   presentation. Language selection must not introduce balance changes.

| State | Ownership and persistence contract |
| --- | --- |
| Account admission and handoff | Global coordinator; one owner/generation across both worlds. |
| Characters, citizenship, cargo, inventory, currency and progression | Shared authoritative persistence; only the active owner may write. |
| Guild identity, membership, leadership, citizenship, kingdom, fame and alliances | One global guild authority; versioned mutations and cache invalidation in both worlds. |
| City owners, challenges, bids, war progress, tower ownership and taxes | Channel-scoped records, including `(channel, city)` identity and event-specific keys. A guild remains one global entity. |
| Movement, monsters, drops, shops, bosses and instances | Channel-owned live state; persistent instance/event records include channel identity. |
| Rewards | Issued by the owning event/world through authoritative persistence; deduplicate using channel, event occurrence and recipient identities. |
| Names, sanctions, donation fulfillment and staff account actions | Global authority; channel-filtered presentation where appropriate. |
| Schedules and notices | Explicit channel scope and time zone; retain existing schedule values until separately approved. |
| Language | Server text follows the receiving channel; client UI follows the local `lang.txt` preference. Neither determines citizenship. |

Deliverable: state ownership matrix, locale/resource contract, endpoint
contract and acceptance cases. Classify functional deltas individually;
localization and a new endpoint contract are coordinated extensions where
they cross client/server boundaries. Do not label new behavior native parity.

## Phase 1: coordinate two live worlds

Start with a design spike for one native Go process containing two isolated
world loops and a shared coordinator. This avoids introducing distributed
ownership solely to offer two channels. Confirm that world construction,
metrics, timers, shutdown and administration support this topology before
adopting it. Do not simply instantiate a second `World` and share pointers.

- Move account admission and pending-write reservations into a shared,
  thread-safe authority. Ownership includes a generation token and remains
  held until persistence acknowledgement and disconnect cleanup finish.
- Share the persistence service while giving shared aggregates one owner.
  Scope channel-local instance state by channel, migrate existing state to
  Channel 1, and preserve global character/item identities.
- Route channel listeners to their own worlds. Add a versioned endpoint
  resource supporting host and port, or prove an equivalent compatible
  routing design. Apply the same destination resolution to login and in-game
  migration; retain bounded legacy parsing. Reuse native 7.48 evidence and
  research only uncovered boundaries.
- Make population, readiness, notices, staff actions and audit records
  channel-aware. Coordinate global account limits, name claims, sanctions,
  donation delivery, guild changes and exports across the two worlds.
- Prove safe disconnect/relogin and crash recovery before enabling a shared
  account in the second channel.
- Implement in-game switching with authenticated, short-lived, single-use
  destination-bound tickets and the existing native migration lifecycle.
  Reuse the recorded
  [7.48 scene-rebuild contract](../.agents/research/client748/flows/lifecycle/field-scene-rebuild-after-server-move.md)
  and investigate only the remaining server and ownership boundaries.
- Hold source ownership until outstanding critical writes are acknowledged.
  Permit exactly one destination claim, reload committed authoritative state,
  and fence delayed source writes. An interrupted handoff must resolve to one
  owner with a recoverable login path, never two writable sessions.
- Preserve citizenship, character/item identity, inventory, cargo, balances,
  progression and absolute effect-expiry timestamps. Define destination spawn,
  instance exit and transient-state cleanup from supported gameplay contracts;
  do not invent transfer charges, timers, rewards or citizenship changes.

If process isolation is required instead, design durable account ownership
with fencing, shared-state concurrency control and failure recovery before
running two writers against the same database. That is a larger alternative,
not a configuration shortcut.

## Phase 2: implement citizenship, guild authority and war isolation

### Persist and migrate explicit state

- Add independently validated guild citizenship values 0, 1 and 2 to the
  authoritative guild model and persistence format. Version the aggregate
  and migrate supported storage backends before either world writes it.
  Never infer citizenship from the kingdom or cape emblem.
- Prepare a read-only migration report for existing guilds, leaders, members,
  city ownership and challenger registrations. Use consistent persisted
  evidence to map legacy channel-owned records to Channel 1. Conflicting or
  missing guild citizenship requires explicit reconciliation; do not guess
  from a member's current hosting channel or first login.
- Guild creation, recruitment, leadership changes and offline membership
  changes must all use the shared guild authority. Preserve the recovered
  citizenship equality requirement for recruitment. Document how a founder
  establishes guild citizenship, including citizenship 0, before implementing
  that uncovered creation boundary; do not invent a registration charge or
  automatically grant citizenship to a neutral founder.
- Reconcile incompatible legacy membership before enforcement. Do not use
  login normalization to overwrite an explicitly chosen citizenship or to
  turn a neutral guild into an eligible war guild. Guild citizenship 0 has
  no home-channel war eligibility.

### Make registration and departure durable transitions

- Implement `getout` against authoritative membership, including leaders.
  Serialize it with recruitment, guild departure and leadership changes so
  a stale world cache cannot bypass the guild restriction.
- For a character without a guild, validate citizenship and the applicable
  departure restrictions, then persist citizenship 0 and any retained fame
  penalty atomically before acknowledging success. An idempotent retry must
  not apply the penalty twice. Recompute derived benefits and publish the
  updated character state to the owner and relevant observers.
- Preserve citizenship 0 through disconnect, relogin, restart and channel
  transfer. Require explicit registration to leave that state. The historical
  mortal auto-assignment and leader-driven guild reset are excluded.
- The recovered non-guild departure baseline blocks the configured guild-war
  weekday and leaves `ceil(old individual fame / 2)`, without an explicit
  gold fee or EXP penalty. Record these as the historical baseline requiring
  the project's compatibility decision before implementation; the owner's two
  new rules do not independently approve every other 7.59 rule.
- Implement Kibita registration separately from its same-citizenship Soul
  branch. The recovered registration fee is
  `(stored level + evolution adjustment) * 10000`, with adjustments 0/200/400
  and a zero fee when the calculated amount is below 2,200,000. Verify the
  level representation before reuse. These are registration findings, not
  proof of a Soul gold requirement. Preserve the separately approved
  historical Soul exchange window and absolute expiry on channel transfer.
- Characters with another nonzero citizenship must leave it before registering
  again. A registration request with a guild must preserve guild/character
  citizenship consistency and must not silently migrate the whole guild.
  Close any unproven neutral-guild registration case explicitly.

### Enforce one war predicate at every relevant boundary

Require an eligible global guild with
`guild.citizenship == event.channel`, plus current membership and
`character.citizenship == guild.citizenship != 0` for a participating character.
Retain each war's other validated requirements; channel eligibility does not
replace rank, kingdom, fame, schedule, bid or objective checks.

Use the same policy for registration/bidding, participant selection, war
teleport/summon, reconnect, objective attacks, defense and reward entitlement.
Revalidate stale registrations before an event starts and before a reward is
committed. Administrative registration must not bypass the policy. Enforce
participation at event actions and participant admission without blocking
ordinary channel access or inventing a spectator restriction.

Scope City War and Tower War state, owners, challengers, schedules, tax
collection and rewards to the event channel. Remove the historical foreign
guild fame exception. Define the existing event's behavior for membership
changes and disconnects; prevent a transferred or departed participant from
continuing with a stale eligibility snapshot or receiving duplicate rewards.

### Audit benefits and the remaining citizenship consumers

Create a matrix for citizenship 0, home-channel citizenship and foreign-channel
citizenship covering Kibita/Soul, drop, EXP, individual fame, guild fame,
regional ownership bonuses, citizen chat, recruitment, teleport items,
`relo`, cape conversion and war-related displacement. Recalculate derived
values after citizenship changes and channel arrival, without refreshing
timed effects or applying rewards again.

The supplied server supports a Kefra-region personal fame award of 20 for
eligible home citizens; the separate guild fame and regional EXP branches
must remain separate. The Go home-channel +10 drop and EXP-balance-derived
drop formula are unresolved against the official evidence. Preserve the
existing implementation during infrastructure work; resolve its authority
or obtain an explicit balance decision before changing or advertising those
benefits. Do not invent a citizenship EXP/drop percentage. This gap blocks
acceptance of that benefit matrix, not independent infrastructure work.

For chat, preserve native citizen-equality and opt-out semantics unless an
explicit contract changes them. Proposed shared-server routing is global
guild chat and whispers through the coordinator, local proximity chat per
world, and citizen chat to the matching citizenship across both worlds.
The citizen relay, including the treatment of citizenship 0, requires an
explicit routing contract: the inspected historical branch only proves
same-process delivery and also matches zero. Label cross-channel relay as
project behavior, not recovered native parity. Render system errors in each
recipient's channel locale; preserve player-authored text.

Deliverable: versioned data migration, authoritative membership and citizenship
transitions, shared war eligibility, channel-scoped event records, and tests
for both channels before localization or public rollout acceptance.

## Phase 3: localize authoritative server text

- Add keyed `pt-BR` and `en` catalogs for panels, errors, notices, NPC dialogue,
  quests and time-sensitive messages. Resolve messages using the receiving
  channel's configured locale, including pre-login errors.
- Preserve packet layouts where text packets already suffice. Keep stable
  IDs, database values, command identifiers, NPC roles and gameplay values
  separate from translated display names.
- Treat command aliases and strings used by parsers as contracts. Translation
  must not break chat commands, NPC selection or crafting input.
- Preserve player-authored chat and names; automatic translation is outside
  this proposal. Implement the approved shared-chat routing contract from
  Phase 2 without translating user content.
- Validate catalog completeness, formatting parameters and encoded byte
  lengths. English fallback can support development, but unresolved required
  Portuguese messages block Portuguese release acceptance.

## Phase 4: implement one bilingual client and launcher

Inventory the active loaders and their overlays before translating. Cover
login, character selection, HUD, inventory, item/skill descriptions, effects,
NPC dialogue, help, confirmation dialogs, errors, launcher and text embedded
in textures. Translate the effective item-name source, including overlays,
rather than changing an earlier table that is overwritten at load time.

Introduce a locale resource layer for hardcoded presentation text and generate
legacy resources from reviewed translation sources. Resolve each localized
path through the selected `EN` or `PT` root before loading any overlay.
Inventory embedded paths inside resource files as well as the top-level
constants. Translate presentation fields through overlays where a binary
table also carries gameplay data; do not introduce different item statistics
or resource IDs between languages.

Implement a small dedicated `lang.txt` reader using the contract above and
initialize locale before asset validation and scene construction. Keep the
existing `config.txt` parser unchanged. The language selector and atomic
preference writer are now installed through a source-built extension to the
operational native launcher, preserving its website update protocol and
original Start handler. Translation of the launcher's remaining presentation
through the same selected locale remains planned.

Preserve resource indexes, checksums, binary widths, format parameters and
non-text item/skill fields. Validate Portuguese accents, font coverage,
keyboard input, truncation and layout in the actual client; do not assume
UTF-8 fits legacy byte fields.

Build one installable package containing both complete locale trees and the
shared assets. Record one executable identity plus separate resource hashes
for `EN` and `PT` in the package manifest. Missing or corrupt required locale
resources must produce a repair path before gameplay instead of silently
mixing individual files from another language. English startup defaults do
not substitute for a complete Portuguese resource set.

## Phase 5: shared distribution and version admission

- Extend build/publish tooling to include both `EN` and `PT` trees in one
  full-client download and one cumulative patch feed. Both locales must stay
  compatible with the same executable and gameplay data revision.
- Verify both locale resource manifests and shared executable/assets during
  updates. Preserve `lang.txt` and other personal settings. An existing
  preference is never reset by a patch; avoid shipping a default preference
  file that overwrites a player's selection.
- Provide a bootstrap/full-download migration for existing launchers, because
  the current patcher cannot replace itself while running. Maintain the
  established feed through the transition to the shared bilingual package.
- Update the public download page with the single bilingual package and
  explain the launcher language selector and access to both channels.
- Raise the compiled client revision and server minimum together to one above
  the latest published revision. Both language resource sets use that same
  gameplay revision. Track content identity separately; a matching executable
  revision alone does not certify translated assets. Define a versioned
  admission extension only if the server must enforce resource content; the
  local preference file does not itself need a new gameplay packet.
- Publish and verify the bilingual package and launcher migration before
  activating the server gate.
  Prepare coordinated recovery without lowering or disabling that gate.

## Phase 6: acceptance and rollout

1. Two-client concurrency: simultaneous login to the same account in different
   channels, pending writes, duplicate character names, character creation,
   trade/cargo writes, disconnect, crash, restart and recovery. No duplicated
   items, stale balances or writes by a former owner.
2. Channel isolation: local movement, combat, drops, shops, bosses and instances
   stay in the intended world. Global guild, donation, sanctions and staff
   operations reach the intended owner exactly once.
3. Citizenship: test 0/1/2 on both channels, without a guild, as a member and
   as a leader. A guild member always receives the leave-guild rejection from
   `getout`, including on the war weekday. Confirm no mutation on rejection,
   atomic fame/citizenship persistence on success, retry safety, and no implicit
   citizenship after relogin, restart or transfer. Race departure against
   recruitment and use offline guild changes to exercise stale-cache handling.
4. Wars: test own-channel, foreign-channel and neutral guilds at each City War
   and Tower War boundary, including high-fame foreign bids, defending owners,
   stale registrations, reconnect and rewards. Confirm independent city/tower
   ownership in both channels and no cross-channel participation or payout.
5. Benefits and consumers: exercise the approved home/foreign/neutral matrix,
   including registration fees, Kibita's two Soul paths, timed expiry, guild
   recruitment, citizen chat, teleports, cape presentation and regional awards.
   Keep unresolved drop/EXP claims out of completed validation.
6. Language: exercise the same installation with English and Portuguese from
   installation through gameplay,
   including NPCs, Kibita, items, skills, failures, accents and long strings.
   Test all four language/channel combinations and citizenship 0, 1 and 2.
   Verify absent, malformed and valid `lang.txt`, direct game launch, paths
   with spaces, launcher reopening, atomic save failure, and a language change
   taking effect on the next launch without changing channel or citizenship.
7. Delivery: clean installation and incremental update of the shared package,
   both locale manifests, mismatched/missing assets, unavailable feed,
   obsolete client rejection, and intact personal settings including
   `lang.txt`. Record the executable, shared assets and both locale hashes.
8. Channel transfer: exercise 1-to-2 and 2-to-1 with citizenship 0, 1 and 2 and
   either language, pending persistence, duplicate/replayed/expired tickets,
   disconnect at each handoff stage, destination failure and restart. Confirm
   that migration neither changes citizenship nor refreshes Soul duration;
   verify source removal, destination creation and observer state.
9. Activate in staging first, then publish the client, activate the compatible
   server and verify both public channel entries. Actual observed gameplay
   is required for `CLIENT_TESTED`; builds and deployment checks are separate.

## Completion criteria

The feature is complete when both channels run safely, one client covers all
required player-facing surfaces in both languages, the launcher saves and
restores the selection through `lang.txt`, the shared patch feed preserves
that preference, version admission works, players can switch channels in-game
with citizenship 0, 1 or 2, guild members cannot use `getout`, guild wars admit
only their own channel's eligible guilds, and the accepted gameplay matrix has
been observed in the exact distributed artifacts. Benefit authority, legacy
data reconciliation and the approved locale-policy change are explicit gates;
record their unresolved status rather than silently choosing defaults.
This plan changes documentation only; no channel or translated package is live.
