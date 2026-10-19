# Guild wars: tower, cities, and guild fame

The server implementation is classified as `MODERNIZACAO_COMPATIVEL`. The
calendar and contest rules were compared with the user-specified W2PP
reference (`CWarTower`, timers, and city results). The 2026-10-19 city settlement
corrections additionally use the supplied server 7.59 decompilation, but
neither provenance establishes native 7.48 parity. The active client/server
pair uses existing packet envelopes. Explicit guild-fame bids use the existing
chat command; no amount field was invented in the native NPC packets. Existing
medals replace reference reward 4906 by explicit project choice. The native NPC confirmation flow was verified separately
against the 7.48 client. The server remains authoritative.

Status: statically verified and covered by automated tests; not
`CLIENT_TESTED`. The real-client integration gate remains open.

## Guild kingdom and cape membership

The guild registry's kingdom is authoritative for its members. Recruitment
requires the recruit's cape kingdom to match the guild, including neutral
guilds. Ordinary members and subleaders cannot change kingdom through the
kingdom NPCs, including conversion to a white cape at Kingdom Broker.

A leader may change kingdom using the existing NPC payment rules. The guild
kingdom and affected characters on connected in-world accounts are saved as
one transaction before confirmation and appearance publication. This includes
inactive characters on those accounts. Existing cape family, refinement,
effects and UID are preserved; empty slots are not granted free capes.
Offline and character-selection accounts reconcile their existing cape from
the durable guild registry on character entry, before appearance publication.

Rejected saves restore prior state without publishing the new colors.
Critical persistence also fences repeated requests and includes reciprocal
trade participants when required; an unknown result follows containment
rather than reporting success or retrying the economic action.

AUTOMATED TESTED: recruitment, ordinary-member/subleader rejection, leader
realm changes, cape families/colors, connected-account propagation, entry
repair, rejection/rollback, held writes, unknown outcomes and trade closure.
The full Go suite, `go vet ./...` and focused guild-kingdom race tests passed
on 2026-10-15. These changes are local only and not CLIENT-TESTED.

## Default schedule

The configured timezone is explicitly `America/Sao_Paulo`, independent of
the Windows system timezone.

| Event | Time | Action |
| --- | --- | --- |
| Tower, Monday to Friday | 20:55 and 20:59 | Five-minute and one-minute warnings |
| Tower | 21:00 | Create the tower for the castle's kingdom and announce the start |
| Each capture | During combat | Persist the capturing kingdom/guild and medal claims; restore tower HP; recall only opposing kingdoms |
| Tower | 21:55 and 21:59 | Five-minute and one-minute closing warnings |
| Tower | 22:00 | Settle conditional guild fame and medal claims, remove the tower, recall opponents |
| Cities | Saturday 00:00 through 23:59:59 | Explicit cumulative guild-fame bids |
| Guild fame | Sunday 00:00 | Close bidding and reset unspent guild fame; retain accepted bids |
| Cities, Sunday | 19:55 and 19:59 | Five-minute and one-minute warnings |
| Cities | 20:00 | Announce the event and its stages |
| Cities | 20:03 | Summon players to four arenas and begin protected preparation |
| Cities | 20:05 | One-minute combat warning |
| Cities | 20:06 | Enable combat for 25 minutes |
| Cities | 20:26 and 20:30 | Five-minute and one-minute closing warnings |
| Cities | 20:31 | Resolve all four cities, persist ownership, fame, defense counts and castle-owner reconciliation, and clear the arenas |

The server must be running to publish warnings. After a restart, it publishes
only warnings still relevant to the current window; it does not replay old
countdowns or provide a clock synchronized with the client.

Announcements appear in the log and chat. The native announcement panel holds
95 text bytes and replaces a notice after about four seconds. Messages are
split into parts shown five seconds apart, with a volatile queue limited to
64 parts. Chat history receives all parts immediately; a burst can drop only
excess panel presentations. A process crash does not guarantee redelivery.
Economic results do not depend on this queue.

## Tower war

- `/tower` admits a living kingdom 7/8 guild member in Erion's public space during
  combat. Entry is at `(2450,1855)`.
- The reward/combat arena spans reference bounds `(2476,1852)..(2545,1918)`, with the dedicated tower near
  `(2507,1873)`, the Tower of Thor anchor in the user-supplied DestinyServer
  TMSRV/run generator reference. The correction stays within the installed
  arena; this historical data is not claimed as native-binary parity.
- The installed `Torre` template supplies its appearance, HP, and defense.
  The tower has no merchant behavior, AI, experience, drops, or quest progress.
  Reference generator 1075 is not reused because it names a different monster
  in the active data.
- Each round starts from the kingdom of the castle owner, with no capturing
  guild. Only the opposite kingdom can damage the tower. Allied guilds cannot
  fight each other through the tower-war PvP exception. Periodic damage
  rechecks the attacker, kingdom, arena, runtime and schedule.
- Capturing persists ownership before announcing it. A failure restores HP
  and the previous owner. The tower regains HP and affects after capture, and
  only opposing kingdoms return to Erion. The winning side remains in place.
- The capturing guild receives **100 guild fame once per round only if the
  final kingdom differs from the initial kingdom**. Each defending-kingdom
  territory owner loses `1000 / max(divisor, 1)` fame, floored at zero, once
  per owned territory (four cities and the castle). The divisor starts at 1
  and increases on a defending-kingdom recapture. It survives unchanged-realm
  rounds and resets to 1 only after a changed-realm settlement.
- Every living, guilded winning-kingdom player in the public arena at capture
  or completion receives one independently drawn medal: green `4055`, gold
  `4051`, or red `4053`, each with probability **1/3**. Capture and completion
  are separate reward events. No individual fame or gold is awarded. The
  green catalog entry retains legacy identifier `Greed_Medal` (mesh 2745);
  gold/red meshes are 2743/2741. Actual client color presentation remains an
  in-game validation gate. The future redemption NPC is not implemented and
  has no invented recipes or prices.
- Medal color and stable account/character identity are persisted with the
  event. Delivery removes the claim and inserts the uniquely identified item
  in one account/guild transaction. Offline players, full inventories and
  active trades leave the claim pending; failed saves neither consume the
  claim nor reroll its color. Critical writes publish only after acknowledgement.
- Restart recovery restores the owner and deadline. It recreates a still-active
  tower at full HP or completes a pending expired-round reward without
  duplication.

Ordinary generator loading and respawning exclude Tower of Thor generators
inside Noatum `(1036,1672)..(1144,1764)`. W2PP creates those towers explicitly
during the Sunday castle battle; event-owned does not mean permanent. The
guard remains active on Sunday because the full Noatum battle controller is
not implemented here. It does not suppress merchants or the Erion tower.
Generator identity and region are checked instead of assuming historical
generator indices are portable. Focused tests exercise installed fixtures,
Sunday suppression, unrelated generators and the corrected Erion anchor.

War movement gates, combat exceptions, tower targeting and arena cleanup apply
only to the public gameplay space. Private instance participants at the same
coordinates are neither enrolled nor ejected by public wars. War exceptions
cannot authorize cross-runtime combat or attacks on a private tower actor.

## City registration and combat

`/citywar` reports all four cities, owner/challenger IDs, bids, and defense
counts in chat. `/citywar <1..4> <total guild fame>` bids for Armia, Azran,
Erion, or Nippleheim respectively. A numbered command without an amount only
returns usage; it cannot spend fame implicitly.

Registration requires a living canonical leader of a kingdom 7/8 guild who
is a citizen of the channel and is in the contested city's public space on
Saturday. There is no minimum or fixed fee. The highest cumulative bid wins,
with the first challenger retaining a tie (explicit project policy). Only
the increase over the guild's previous bid on that city is charged; repeating
the same total is free and decreasing it is rejected. Losing bids are spent.
A guild may divide its available fame among multiple cities, including when
it already owns another city; it cannot challenge its own city.

At Sunday 00:00 in the configured timezone, unspent guild fame resets once.
Accepted bids and challengers remain for Sunday's battle. Earned fame after
the reset belongs to the new week. Downtime is caught up at the next tick.
The first upgrade tick records the current Sunday as a baseline without
retroactively erasing an existing balance. Legacy fixed-fee applications are
cancelled and their 100-fame fees refunded before migration/reset; no legacy
treasury is converted into fame.

The native collector flow is `0x28E` (client to server, 16 bytes) -> `0x18D`
(server to client, exact 12-byte `MSG_STANDARD`) -> `0x28F` (client to server,
20 bytes). On `0x28E`, the server validates collector visibility, range,
city, and current owner; it stores a 30-second context for the NPC, city,
and owner guild before opening native selection. A valid collector
click (`onUseNPC`) can also prepare this context. `0x28F` consumes it once.
Replay, expiry, owner change, or interaction with another NPC invalidates
confirmation. Confirmation never withdraws gold or automatically spends fame:
it returns the explicit bid command. An invalid option or an absent,
incompatible, or out-of-range collector also consumes the old request; the
player must interact again. Guild and leadership are re-resolved from the
canonical record at confirmation, not trusted from the packet or context.

Collectors proven in the active data are Balmus -> Armia (`Merchant=6`,
`Level=0`, `2107,2144`), Kara -> Azran (`Level=1`, `2538,1714`), and Empis ->
Erion (`Level=2`, `2466,1986`). No Nippleheim collector exists in the current
data; none was fabricated. `/citywar` remains an equivalent administrative
and playable route subject to authoritative validation.

Each participating guild may summon up to 26 living characters from the
city's public space who are not using Ghost Shop. Alphabetical character
name breaks selection ties. Erion permits only mortal characters. Cities
without challengers summon nobody. Summoning cancels trades.

Preparation prevents crossing the arena center and blocks PvP. During
combat, only opposing participants in the same arena may attack. Teleport,
death, disconnect, leaving the arena, or changing guild removes
participation; relogging does not grant another slot. Ordinary CP/experience
death penalties do not apply. Direct, area, and periodic damage all use war
authorization.

The server publishes the opposing guild through `WarInfo 0x3A8/24B` at
combat start and clears the snapshot on exit, death, and completion, reusing
the [native contract](../.agents/research/client748/flows/transport/war-info-contract.md).
Preparation barriers are server-side: the installed data has no gates in
these arenas, so no visual object was invented.

## Results, economy, and persistence

- City points are the sum of valid survivors' levels, plus 399 for each
  non-mortal. A tie preserves the owner; an attack must exceed the defense.
- Conquest changes the owner and clears its defense count. A successful
  defense increments the count up to four. Without a challenger, ownership
  remains unchanged; an unowned city with no scoring attack stays unowned.
- A conquest awards **50 guild fame** to the attacker and removes up to
  **50 guild fame** from the previous owner, with a floor of zero. A defense
  does not award fame. Settlement does not create gold or items.
- Noatum ownership is reconciled after all four city results in the same
  transaction. The castle owner is retained if it matches any city owner;
  otherwise the first nonzero city owner in territory order is selected.
  This preserves the reference's zero-value match: an unowned castle stays
  unowned while any city is unowned. If all cities are empty and the old
  castle owner is nonzero, that owner is retained. This is ownership state,
  **not an implemented Noatum battle controller**.
- Wars do not tax shop transactions, credit gold treasuries, or permit gold
  withdrawals. This is enforced from startup while fame wars are enabled,
  and remains enforced after the fame migration marker exists even if the
  schedule is disabled. Legacy treasury values remain loadable and untouched;
  no conversion, checks, individual fame or gold payout is introduced.
- Player mutation and guild/war records form one logical transaction when
  both are affected. A persistence failure rolls back all affected state.
- `/guildfame` and the established `/famaguild` alias query guild fame.
  Older records start at zero. Implemented fame sources include tower war,
  Kefra, and city conquest; these do not cover every reference fame source.
  Addition rejects overflow, and registration rejects insufficient funds.
- Fame, results, registrations, and completion markers share the guild-record
  transaction in JSON or PostgreSQL. They are not applied twice through the
  reference's redundant paths.
- Failed persistence undoes mutation; rewards are never announced before
  commit. City points freeze during closing persistence attempts so late
  departures cannot change the result.
- A restart during preparation or combat cancels the city round and preserves
  owners. Explicit bids are refunded only before their week's fame reset;
  spent fame cannot reappear in the next week's balance. Failure to summon
  before combat also cancels the round; it does not
  start a smaller round with whichever players reconnect first.
- Guild dissolution removes its tower, castle, city, and registration
  references in the existing transaction so a reused guild ID cannot inherit
  ownership.
- Use the existing single guild record. Running independent `World` instances
  against it is unsupported; multi-server coordination was not added.

### Server 7.59 settlement evidence and remaining differences

The local-only reference is
`references/decompilacao server/descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c`,
with corpus SHA-256
`F8C064D7B3E73600B244A743A1D3CC3F6E4DB66F2B44364367A2E1E825E664CD`.
The existing receipt was reused rather than rehashing the unchanged corpus.
Line numbers below are one-based:

- `sub_566BE0`, lines 72089-72093: conquest calls with `+50` and `-50`.
- `sub_588340`, lines 85724-85749: the fame adjustment body clamps a
  nonpositive result to zero.
- `sub_566BE0`, lines 72134-72168: castle-owner retention and first-city
  fallback after city settlement, including the zero-value comparison.

The existing four-defense cap remains a project limit; the 7.59 result routine
increments its historical counter without this cap. Guild-fame-only bidding,
weekly reset, medals, restart cancellation and deterministic summoning order are not
newly established as 7.59 parity by this settlement correction. Castle battle
calendar, doors and breach actors, event towers, seal capture, recovery and
native client presentation remain separate work; see
[G04](server/server-759-gap-analysis.md#g04--full-noatum-castle-battle-controller-is-absent).

Focused automated coverage includes conquest/defense/ties, fame floor,
overflow and storage rollback, frozen-score retry, completion replay,
castle-owner ordering and JSON round-trip, guild dissolution, critical writes
held before acknowledgement, certified rejection/retry, and public/private
arena isolation. These tests do not constitute real-client execution.

Historical settlement-only validation on 2026-10-19 passed: `go test ./...`, `go vet ./...`,
the focused settlement/isolation tests, and the following focused race run
with `CGO_ENABLED=1`:

```powershell
go test -race ./internal/model ./internal/game -run 'Test(CityConquestReference|CastleCityOwner|CastleOwnership|ScheduledCityConquest|ScheduledWarPrivateSpaceIsolation|CastleOwnerPruned)' -count=1
```

The server candidate was built successfully from `wydgo748/`:

```powershell
go build -o C:\Users\xereca\Documents\PROJETOS\wyd-go\build\war-settlement-20261019\wyd-server.exe ./cmd/server
```

Its size was 25,810,944 bytes and its SHA-256 was
`ACF259E147BB08E5362428CBE7CC2A4D26EAD179791A73DF0CAFEFC30449EC2F`.
Execution policy blocked cleanup, so that exact candidate and its directory
remain. Cleanup was not retried through another mechanism. It was not
installed, started, or published. No client build or gameplay validation was
performed.
The global layout and whitespace gates still fail on unrelated existing
materials listed in the [gap audit](server/server-759-gap-analysis.md#validation-performed-and-delivery-state);
the changed tracked files pass the scoped whitespace check.

## Server 7.59 full war comparison (2026-10-19)

This section preserves the **pre-implementation audit snapshot**. Its
"pre-change" column and missing-behavior descriptions are historical, not
the current implementation described above. The current economy intentionally
overrides reference gold/individual fame and kingdom bid slots with the
user-approved guild-fame bidding and equal-chance medals.

This investigation compared the then-current implementation with the supplied
server decompilation identified above. It changed no gameplay code and made
no deployment. `N` below means a one-based line in that reference. Server
7.59 behavior is not, by itself, proof of a native 7.48 client contract.
Where the dump exposes only a jump alias, a semantically matching body does
not resolve that alias automatically.

There are three distinct contests: city arenas, the Erion/Endless Tower
kingdom contest, and the Noatum castle battle with event-owned towers.
Implementing one does not implement the others.

| Boundary | Pre-change implementation | Recovered server 7.59 behavior |
| --- | --- | --- |
| City calendar | Saturday registration; Sunday combat at 20:06-20:21 | Configurable `GuildDay` and guild hour, default hour 20; preparation after minute 2, combat after minute 5, settlement after minute 30 |
| City bids | One local highest-fame bid; first tie wins; every accepted registration costs 100 fame | Kingdom-specific bid slots, ownership/citizenship/channel checks, and conditional fee; equal fame is not rejected by the recovered comparison |
| Erion tower | Monday-Friday 21:05-21:35; owner and target restrictions by guild | Monday-Friday, approximately 21:00-22:00; defending and capturing kingdom plus capturing guild |
| Tower capture | Restore tower and eject all participants | Reward qualifying members of the capturing kingdom; recall opponents, not the winning side |
| Tower completion | Final guild receives 100 guild fame | Qualifying winning-kingdom participants receive character fame and an item; guild fame and treasury effects additionally depend on a kingdom change |
| Noatum castle | Owner reconciliation and reserved generators only | Separate phase controller, event towers, doors/breach, seal capture, and settlement |

### City registration, calendar, and collection

The city scheduler is in `sub_567780`, `N:72605-72670`; initialization is at
`N:73922-73928`. Registration opens on the weekday preceding `GuildDay`.
The default guild hour is assigned at `N:10692`; `guildday` and `guildhour`
commands are at `N:30417-30434`. The reference's configured weekday was not
established as Sunday. At the default hour the recovered transitions are
approximately 20:03 preparation, 20:06 combat, and 20:31 settlement. The
reference also invokes arena-door state changes; the current virtual barriers
do not establish parity for those actors or their presentation.

The recovered registration body `sub_512800`, `N:43543-43720`, adds important
conditions beyond the current bid model:

- The character must belong to kingdom 7 or 8. The body tests guild rank
  values 0 and 1; their correspondence to the current canonical leader
  representation must be resolved rather than copied as numeric ranks.
- It searches ownership across ten channels and five territories, not just
  four local city records. Citizenship and guild home-channel checks affect
  eligibility. A foreign citizen needs 1,000 guild fame; a local applicant
  needs 100 when the city already has an owner.
- The 100-fame fee is conditional on an existing city owner. An empty city
  therefore does not justify the current unconditional minimum and fee.
  The bid snapshots fame before the charge; an owned-city losing bid can
  still incur the charge.
- It maintains two kingdom bid slots per territory and branches on the
  owner's kingdom and existing opposing bid. This is not equivalent to one
  global maximum. The fame rejection uses a strict less-than comparison,
  rather than establishing the current first-registration tie rule.

The `sub_4B2090` guard and jump-alias linkage remain unresolved. The current
one-registration-per-week replay protection and restart refund policy must
not be removed merely because equivalent code was not found in this body.

Collector dispatch at `N:48924` and confirmation at `N:49100-49114` also
distinguish registration, status, and treasury collection by event phase.
The recovered withdrawal branch is gated to the fight day and an eligible
owner/leader; the current rule permits collection on every non-registration
day. Variable tax policy and all treasury branches need their own trace before
changing the current fixed 10% rule. The fifth territory includes castle
dependencies; it is not permission to add a fifth ordinary city arena.

### Tower kingdoms, fame, rewards, and treasury

The tower scheduler is `sub_567780`, `N:72673-72841`; capture is at
`N:74610-74651`. The reference starts from the defending kingdom rather than
clearing ownership to an unowned guild each day. Attack paths at
`N:53925-53935` and `N:81503-81506` reject attacks from that kingdom. The
current guild-only ownership rule would incorrectly allow another guild of
the same kingdom to attack. The implementation now uses kingdom ownership.
The defending realm is reconciled from castle ownership (`N:72157`), not
the previous day's tower winner. The divisor starts at 1 (`N:10450`),
increments on defending-realm recapture (`N:74649-74650`), and resets only
on changed-realm completion (`N:72764`).

Capture records both kingdom and capturing guild. Living, guilded characters
inside `(2476,1852)..(2545,1918)` on the capturing side receive item `4906`
and 100 character fame. Opponents are recalled. At completion, qualifying
members of the winning kingdom receive item `4906` and 300 character fame.
These are character rewards, not additions to guild fame. The reference's
`setfame` command at `N:29680-29686` independently identifies the character
fame helper used here.

If the winning kingdom differs from the initial defending kingdom, the
reference additionally:

- Awards 100 guild fame to the capturing guild, with a kingdom-specific
  fallback when no capturing guild is available.
- Debits defending-kingdom city-owner guilds by integer
  `1000 / max(dword_646F04, 1)`. Capture can increment that counter; it is not
  safe to replace the divisor with a count of city owners.
- Transfers a computed share of the global treasury through a home-channel
  database message and updates kingdom balance/SAP state. This is not a
  demonstrated direct gold payout to each participant.

Those conditional effects are absent from the current single-guild reward.
The scheduler also adds 300 to a tower score field under a minute-modulo
condition; the exact field and invocation cadence still need resolution.
The historical generator index `1079` cannot be copied into active data
without checking identity and all stats. No native entry NPC was established
by this trace; `/tower` remains a project entry route.

Item `4906` was not found in the active server `itemlist.csv` or
`Itemname.csv`. Its identity, 7.48 client assets, representation, inventory
behavior, and use/redemption rules must be established before granting it.
This finding does not establish absence from every client asset file. The
user subsequently replaced 4906 with existing medal IDs 4055/4051/4053 at
equal probabilities; no 4906 reward is granted by the implementation.

### NPCs and the 7.48 confirmation boundary

The active data provides Balmus, Kara, and Empis with the collector contracts
listed above. It does not provide a Nippleheim collector. Freyja's gold wager
is a separate system, not the city guild-fame registration mechanism.
Automated confirmation coverage is not proof that any of these NPC windows
have been exercised in a running client.

The reference confirmation dispatcher admits merchant values 6 through 9.
The current 7.48 client checks its quest branch for values 8 and 9 before its
collector branch. Therefore copying those reference merchant types would not
automatically open the collector flow. Resolve the actual NPC definitions and
7.48 dispatch contract before changing merchant roles.

The existing native evidence remains reusable:
[challenge confirmation contract](../.agents/research/client748/flows/transport/challenge-confirm-request.md).
It establishes the `0x28E -> 0x18D -> 0x28F` packet flow, not all registration
economics, all merchant variants, or real-client gameplay validation.

### Other fame and implementation dependencies

Guild fame and character fame must remain separate durable balances. The
current character balance is `SpecialCoins["fame"]`; guild fame belongs to
the guild record. Existing character-fame consumers, including progression
and crafting, cannot be redirected to guild fame.

A further exchange is recovered in `sub_4F6D60`, `N:35945-35981`, under
selector 50: a kingdom 7/8 guild leader with rank 9, a nonzero guild, and
starting guild fame at most 1,000,000 can exchange the first eligible carry
item. Item `4010` supplies 1,000 guild fame; `4011` supplies 10,000, and the
item is consumed. No equivalent guild-fame exchange was located in the
current inspected routes. The meaning and incoming dispatch of selector 50
remain unresolved: it is not proof of an NPC with `Merchant=50` or an item
with `Volatile=50`. Active silver items use volatile 185. Do not invent an
NPC or move this exchange onto Freyja on that evidence.

Citizenship/kingdom lifecycle is also economic state. The reference exit
route at `N:52096-52157` checks war day and cross-channel ownership, reduces
character fame, and can reset a departing leader's guild fame and update its
home channel. Current local guild persistence is not a substitute for the
reference's database/global territory coordination.

Implementation should proceed through these dependencies:

1. Define authoritative kingdom, citizenship, home-channel, territory,
   capturing-guild and two-kingdom bid state, including migration of current
   guild-only tower records. Resolve rank and helper ambiguities first.
2. Resolve the reward item and NPC/client dispatch boundaries. Retain
   server-side range, context, ownership, replay, and runtime checks.
3. Implement city calendar/bids and the kingdom tower lifecycle separately,
   including admission, allied targeting, death/CP exceptions, recall,
   successive captures, ownership persistence, and interrupted recovery.
4. Commit participant inventory/character fame, guild fame, territory state,
   treasury and completion atomically. Cover rejection, unknown outcome,
   retry, disconnect/reconnect, offline guilds, and duplicate settlement.
5. Define a real multi-channel authority or explicitly scope the feature to
   a single channel. Independent `World` instances sharing a file do not
   implement the reference's ten-channel ownership checks or treasury relay.
6. Complete Noatum as a separate controller; see
   [G04](server/server-759-gap-analysis.md#g04--full-noatum-castle-battle-controller-is-absent).
   The recovered seal counter and `+650/-650` fame are not implemented by
   city-owner reconciliation or ordinary tower generation.

Focused validation on 2026-10-19 passed for `internal/model` and
`internal/game`: registration/bid/replay, collector prompt and confirmation,
expiry and owner/NPC changes, invalid collectors, city/tower lifecycle,
summoning and combat gates, tax/treasury boundaries, rollback and critical
persistence, including the PostgreSQL treasury commit/rejection fixture.
These tests validate the current implementation, not the missing reference
behavior. No client execution, publication, or VPS restart was performed.

## Configuration and implementation

Optional server configuration keys:

```ini
guild_wars_enabled=true
guild_wars_timezone=America/Sao_Paulo
tower_war_hour=21
city_war_hour=20
```

Hours are 0..23; the default offsets are listed above. Configuration is
read at startup without changing the machine clock. Disabling guild wars
turns off scheduling, registration, and war combat/movement gates; it does
not remove existing ownership. Gold war economics remain disabled once the
fame migration marker exists.
Re-enabling uses the recovery rules above.

The main implementation is `wydgo748/internal/game/guild_wars.go`, with
models in `wydgo748/internal/model/guild_wars.go`. Relevant tests include
`guild_scheduled_wars_test.go`, `guild_wars_test.go`, persistence and
configuration tests, `WarInfo`, and `guild_city_contract_test.go`. Combat,
movement, teleport, guild, economy, and tick integration reuse the
authoritative `World` without new goroutines.

## Guild-fame implementation validation (2026-10-19)

This receipt covers the current guild-fame bidding, weekly reset, corrected
tower lifecycle and medal delivery batch, not just the earlier settlement
change or the historical comparison snapshot above.

- `go test ./...`: passed, including all `internal/game` tests. The initial
  failure used a staff-teleport fixture outside the corrected tower arena;
  moving that fixture inside the arena restored its intended rejection case.
- `go vet ./...`: passed.
- `CGO_ENABLED=1 go test -race ./internal/model ./internal/game -run
  'Test.*(Tower|CityWar|WeeklyGuildFame|WarMedal|FameWar|ScheduledWar)' -count=1`:
  passed for both packages. This is focused race coverage, not a full-suite
  race result.
- Automated coverage includes cumulative and multi-city bids, zero-balance
  registration, weekly reset/replay, realm captures and divisor accounting,
  castle inclusion in the five-territory fame penalty, equal medal choices,
  offline/full-inventory/save-failure delivery, critical acknowledgement and
  rejection, and atomic finish/reward persistence without duplicate rewards.
- Native Windows server build: passed with
  `go build -o C:/Users/xereca/Documents/PROJETOS/wyd-go/build/guild-fame-wars-20261019/wyd-server.exe ./cmd/server`.
  Candidate size: 25,821,184 bytes. SHA-256:
  `206247EDE8DDC2F02ECA5ECB4D66FB5C60739AF0F779F21218AB4E3769A78045`.
  This candidate was built, not installed or started; disposable output is
  removed after recording its identity.
- Repository-layout validation still reports pre-existing client audit,
  nested build and crash-monitor leftovers outside sanctioned build roots.
  Those unrelated files were not removed by this task. This is not a clean
  repository-layout or global-release approval.

No live PostgreSQL deployment, client execution, patch publication, VPS
upload, player disconnection or server restart was performed for this batch.
The persisted model/critical-write tests do not replace live database or
in-game validation. Noatum's full controller, unresolved reference dispatches
and the future medal-redemption NPC remain separate gaps.

## Remaining validation

Run the built 7.48 client with opposing guilds to verify warnings and
colors, targeting, arrival and barriers, death, skills and periodic damage,
successive kingdom captures, conditional guild fame, medal appearance and
deferred delivery, explicit multi-city bidding, midnight reset, collector
guidance, relogin, and restart. A build or simulated clock does not replace
this gate. RvR/Akelonia-Hekalotia and the full Noatum Castle battle are outside
the tower/city war scope described here; castle-owner reconciliation alone
does not implement that battle.
