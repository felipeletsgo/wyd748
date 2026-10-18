# Gameplay bug register

Updated: 2026-10-18. This is the active queue for the current player reports,
not an inventory of every historical defect. Release receipts remain in
[current state](current-state.md). Registration does not mean that a cause has
been reproduced, a fix implemented, or gameplay validated.

Deployment update: release `20261017T164649Z` activates the implemented portions
of BUG-003, BUG-007, BUG-008, BUG-009 and BUG-011 after an explicitly authorized
player disconnect and game-service restart. Full server tests, vet, portal
checks and active-payload verification passed. The subsequent Hell Gate version-2
encounter was published in release `20261017T221343Z` and retained by the recorded
10035 release. The historical version-3 correction below was subsequently
published in server release `20261018T171412Z`, without a new client patch.
Gameplay acceptance remains pending.

## Queue

Priority is provisional: P1 blocks an encounter or loses essential session
state; P2 affects feedback, presentation, or behavior requiring diagnosis.
The rows are grouped by affected flow, not permission to abandon work already
in progress. No publication or restart is authorized by this register.

| ID | Priority | Issue | Current status |
| --- | --- | --- | --- |
| BUG-001 | P2 | HIT/MISS feedback missing for the attacker | Initial diagnosis; no confirmed cause |
| BUG-002 | P1 | Noatun gate closes unexpectedly and blocks return | Initial diagnosis; no confirmed cause |
| BUG-003 | P1 | Hell Gate has no visible monsters after entry | Version-3 historical populations, march, Dionys shop and Sand Lich selection published in server release 20261018T171412Z; gameplay pending |
| BUG-004 | P2 | Blood Foema incoming damage reduction appears incorrect | Existing mitigation covered by tests; reported scenario unverified |
| BUG-005 | P2 | Blood Foema mana consumption/control appears inconsistent | Concrete skill/resource reproduction needed |
| BUG-006 | P2 | Divine Judgment deals damage without its visual effect | Client effect lifecycle investigation started; no fix yet |
| BUG-007 | P2 | Nightmare party entry and identification are not synchronized | Shared-zone party relocation corrected; automated checks passed, gameplay pending |
| BUG-008 | P1 | Nightmare reconnect loses party and encounter HUD state | Server recovery implemented; automated checks passed, gameplay pending |
| BUG-009 | P1 | Magic Chamber monsters disappear or remain untargetable | Party relocation and independent-run isolation corrected in source; automated checks passed, gameplay pending |
| BUG-010 | P2 | Magic Chamber room-transition ground markings are missing | New player report; native presentation not yet identified |
| BUG-011 | P1 | Compounder and city guards attack players | Friendly roles and monster-only guard combat implemented; automated checks passed, gameplay pending |

## Reports and acceptance checks

### BUG-001 — Attacker HIT/MISS feedback

- Report: the defender sees hit/miss feedback, but the attacker does not see it
  correctly.
- Findings: authoritative physical and skill result builders already clear
  `FlagLocal`. The client preserves negative MISS sentinels when reading wide
  damage and permits floating feedback for the local attacker and target.
  A local-flag change alone is not a supported fix.
- Next check: follow one actual result packet through delivery, target lookup,
  floating-text creation and rendering on both participants.
- Acceptance: attacker and target see the correct result for hits and misses,
  without duplicates or changing damage. Check a nearby observer separately
  against the established visibility policy.

### BUG-002 — Noatun gate state and collision

- Report: the gate closes unexpectedly and prevents normal return to town.
- Findings: no automatic close writer was found in the inspected server
  lifecycle. Castle key IDs 15/16 initialize open; client creation and
  incremental updates use different state encodings.
- Next check: capture server gate state, client state and collision mask when
  the failure occurs; include visibility refresh and scene reentry.
- Acceptance: the gate's presentation and passability agree with authoritative
  state, including after leaving/reentering view and reconnecting. Intended
  war-related closures must remain intact.

### BUG-003 — Hell Gate population visibility

- Report: killing the entry spider produces a remaining-monster counter, but
  searching the entrance finds no monsters and the internal portals do nothing.
- Confirmed causes: the entry and arena are disconnected terrain components;
  walkable spawn coordinates alone did not establish reachability. The client
  also ignored portal tiles absent from its named portal table. The former
  encounter skipped corridor bosses, used four minutes for both admission and
  combat, and finished by clearing a wave around `Aki` instead of defending Dionys.
- Native boundary: the 7.48 `FUN_004776c3` uses generic message row 43 for an
  unknown portal index; confirmation `FUN_004640e5` sends the ordinary 16-byte
  `0x290` request with zero reserved data. The client now follows that prompt
  path with bounded table reads. See the [portal evidence record](../.agents/research/client748/flows/transport/hell-gate-portals.md).
  This bounded prompt change is `PARIDADE_NATIVA`, with compatibility-safe
  guards. It does not establish native parity for the whole event.
- Encounter research: the [official Hell Zone guide](https://feedback.kersef.com/pt-br/knowledge-bases/21/articles/46666-hell-zone)
  describes preparation, three corridor bosses, four rooms, a selected Lich and
  protecting Dionys until the deadline. The official KR notices
  [1071](https://www.wyd2.co.kr/news/news_view.asp?cIdx=1071&nK=notice) and
  [1160](https://www.wyd2.co.kr/news/news_view.asp?cIdx=1160&nK=notice) address
  opening times and Dionys health, not a complete historical 7.48 encounter.
  None establishes the exact timers, populations, HP or reward used here.
- Explicit owner choices: twenty minutes total, including two minutes of
  preparation and eighteen active minutes; 10,000,000 completion EXP per eligible
  connected participant, subject to normal progression caps. Existing minute-0/30
  scheduling and ticket IDs 3328/3329 remain compatible. The guide's individual
  admission requirement is not proof that the existing party-ticket contract
  should be removed; that contract is preserved, not claimed as native parity.
- Published version-2 server reconstruction (`MODERNIZACAO_COMPATIVEL`, unchanged wire):
  version-2 configuration, three server-selected corridor bosses, four rooms of
  nine guards, one Lich per cleared room, and a single server-selected correct
  Lich. An incorrect boss/Lich does not permanently lock the remaining route.
  Three authoritative entrance portals become usable after the correct corridor
  boss dies. Destinations are reachable arena tiles and cannot cross instances.
  Dionys must remain alive until the absolute deadline; thirty attackers are
  replenished when cleared, without healing him. His death fails the encounter.
- Dedicated data: fifteen instance-only `NPCGener/HellGate.txt` blocks and five
  definitions: `HG_Spider`, `HG_Corridor`, `HG_Skeleton`, `HG_Lich`, `HG_Dionys`.
  Generator selectors, bounded placement, wire-name limits and native terrain
  reachability are checked. Occupying Dionys's anchor relocates him nearby
  without overlapping a player or crossing a wall. Persisted loader keys and
  `tipo` values are retained for compatibility.
- Balance is a project choice, not a native constant: the owner subsequently
  requested 400,000 HP for the entry spider, every corridor boss and every
  Lich; both maximum and initial HP now use that value. The owner's 10,000
  average magical damage means damage per cast, not DPS. The owner clarified a rotation of
  different spells every 900 milliseconds, not repeated use of one skill
  every six seconds. With an immediate first cast, each boss requires forty
  casts and approximately 35.1 seconds. This estimate does not change server
  attack intervals or individual skill cooldowns.
  Faster spell rotation, actual mana, resistance and movement can change those
  times. The worst random selection requires eight bosses including the entry
  spider (three corridors and four Liches), approximately 280.8 seconds of
  boss damage under that rotation, excluding guards and travel. This is not a
  measured FelipeTr gameplay run. Friendly Dionys has
  150,000 HP and 200 defense so defending him is not an AFK-safe minimum-damage
  exercise; contact pressure and travel still require gameplay validation.
- Persistence: protector HP survives restoration without healing/resurrection.
  EXP and the reward marker are committed together in synchronous and detached
  critical-persistence paths; failures roll back and retries cannot duplicate
  the reward. The committed marker survives the post-deadline exit grace.
  Completion EXP does not refill empty HP/MP. Reconnect and instance isolation
  retain the existing authoritative membership contracts.
- Delivery: source and focused automated checks; client candidate built with
  `Build-Client.ps1 -NoDeploy`. No local runtime installation, gameplay launch,
  commit or push was performed. Subsequently published as patch 10034 and
  activated in server release `20261017T221343Z` with authorized game/API
  restart. All 7,344 active payload hashes, executable identities, readiness
  and public delivery checks passed. Full Go tests/vet and portal build,
  60 tests and Astro checks passed; gameplay acceptance remains pending.
- Earlier integration validation: `go test ./internal/game ./internal/data -count=1` passed
  (10.563s / 2.306s); scoped `go vet` passed. These include reachable terrain,
  phase transitions, protector death/restore, deadline reward/rollback,
  occupied portals, instance isolation and detached persistence. The full-contact
  defense simulation applies the production formula to thirty installed
  Skeletons: deterministic first hit 114 and estimated survival 66 seconds.
  It deliberately excludes travel, line of sight and player intervention.
  The client build passed architecture (1,240,477 checks), socket receive (231),
  crash capture (83) and launcher (56) gates. Candidate path and hash are in
  [current state](current-state.md#previous-release--2026-10-17-hell-gate-route-and-defense-patch-10034).
- HP/route follow-up: `go test ./internal/game ./internal/data -run '^TestHellGate' -count=1 -v`
  passed (game 0.622s, data 0.163s); output is in
  `build/hell-gate/route-hp-tests.log`. All three configured destinations belong
  to the same traversable arena component. Installed-data request tests cover
  each entrance portal during both populated room and protection phases,
  unoccupied arrival, unchanged membership/price and reachability of all actors.
  Both maximum and initial boss HP are checked; friendly Dionys is unchanged.
- Acceptance still pending: use the exact candidate in game, enter through the
  scheduled ticket flow, traverse all phases/portals, and test owner/observers,
  party visibility, reconnect, Dionys pressure, failure and deadline reward.
  Verify practical solo clear time and mana using FelipeTr's magical setup.

#### Historical correction — 2026-10-18, version 3

- Authority and scope: the owner supplied historical server material under
  `references/decompilacao server/`. The 7.59 folder contains
  `descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c`, SHA-256
  `F8C064D7B3E73600B244A743A1D3CC3F6E4DB66F2B44364367A2E1E825E664CD`.
  The version attribution comes from the supplied collection; it is not an
  independently matched 7.48 server binary. The supplied 6.13 material contains
  a symbol map and explanatory text, not enough implementation to reconstruct
  Hell Gate. This is not evidence that 6.13 lacked the event.
- Proven 7.59 sequence: `sub_578700` at `0x00578700` creates entry generator
  334, selects one of three corridor bosses (335–337), then creates room guards
  (342–377). It selects `first = rand() % 4` and
  `second = (first + rand() % 3 + 1) % 4`: two distinct rooms, including adjacent
  pairs, not one Lich or an always-opposite pair. The death progression inside
  `0x0057F530` releases each room's Lich (338–341) after its nine guard groups
  clear. Both selected Liches must die before Dionys (333) and the defense phase
  begin. Unselected Lich deaths do not permanently lock the event.
- Proven portal boundary: the historical `0x290` handler normalizes coordinates
  with `& 0xFFFC`. At X 1756, only the selected source at Y 3616, 3644 or 3672
  is accepted after the correct corridor boss dies. The other entrances stay
  closed. The existing 7.48 client request and prompt evidence above still
  apply; no new packet, client executable or asset is required.
- Implemented adaptation: installed instance data now selects version 3. It
  restores the distinct random Lich pair and selected entrance, consumes invalid
  instance requests before any global portal fallback, and retains exact runtime
  membership. Replayed deaths cannot count twice. Snapshots record the encounter
  version, selected corridor, pair and completed Liches. Unversioned published
  timed snapshots and explicit version-2 snapshots resume with their old rules;
  a restart neither rerolls their selection nor silently upgrades them. Invalid
  masks, invalid corridor selections and version-3-to-2 configuration downgrades
  are rejected before rebuilding the encounter.
- Historical populations, requested in the owner's follow-up: the bundled
  `DestinyServer-main/TMSRV/run/NPCGener.txt` supplies six corridor escorts
  (390–395), 36 four-member room groups (342–377, 144 guards), three mage escorts
  per Lich (338–341), twelve individual defense generators (378–389) and another
  24 ten-member groups (6023–6046), totaling 252 defense attackers. Leaders and
  followers retain their historical equipment distinctions. Dedicated template
  names fit the 12-byte wire name limit. Spawn anchors are preserved with a
  bounded four-tile placement range; exact original dispersal is not claimed.
- Objective AI: version-3 defense attackers remain awake without nearby players,
  ignore the ordinary home leash, and march toward the live same-runtime Dionys
  using a cached terrain-aware reverse route. Dynamic occupancy delays movement
  rather than allowing passage through another actor. Each casualty queues its
  own original spawn anchor and NPC for replacement on the next event tick;
  temporary capacity/occupancy failure retries once per tick. Surviving mobs,
  protector HP and the absolute deadline are not reset. No replacement occurs
  after expiry or protector death. This cadence is an explicit bounded
  adaptation, not evidence for an exact historical generator interval.
- Dionys shop: head 51 restores the existing native 7.48 panel and 24-byte
  0x2BD request. Same-instance membership, visibility/range, protection phase,
  live tracked protector and deadline are authoritative purchase requirements.
  The supplied 7.59 functions and rate tables provide 20 weapons, 40 armor
  items, 14 accessories and their random options. The public merchant retains
  its earlier reconstruction. Gold/item changes and success packets use the
  critical persistence path; duplicate pending requests cannot reroll. See the
  [store contract](protocol/hell-gate-store.md) for exact evidence and limits.
- Lich visuals: `HG_Lich` now has exactly the face item of the existing
  `KR_Sand_Lich.json` definition: 4088 with effects `[43,7,0,0,0,0]`. The active
  item catalog gives class 57, mapped by `BASE_DefineSkinMeshType` to skeleton
  type 12; `mesh/BoneAni4.txt` selects `mesh/rc01`. All 15 existing `rc01`
  skeleton, animation, mesh and texture resources match the supplied
  `references/WYD_KR 7.70+/mesh/` files byte-for-byte. No historical binary was
  patched and no runtime asset was replaced. This verifies resource selection,
  not the rendered result; visual acceptance remains pending.
- Classification: `EXTENSAO_COORDENADA`, the requested historical 7.59 behavior
  adapted over the unchanged 7.48 wire contract. This is not `PARIDADE_NATIVA`
  for the complete event and does not supersede explicit owner choices.
- Preserved project settings: twenty minutes total (two admission, eighteen
  active), minute-0/30 scheduling, ticket IDs 3328/3329 and party admission,
  configured boss/protector HP and defense, existing purchase prices and the
  10,000,000 completion-EXP policy. Skeleton variants reuse the project's
  existing skeleton score/loot, not historical caster-specific AI or balance.
  Existing deadline, failure, reward persistence and isolation checks remain in
  force. Published version-2 snapshots retain thirty attackers and their prior
  replenishment/puzzle behavior; new version-3 runs use the extended population.
- Historical differences not imported: 7.59 starts active phases around minute
  4, sends 1,500 seconds, and recalls players near minute 29 of each half-hour.
  The bundled generator/rate files have not been independently matched to the
  decompiled binary and are used as the owner-requested secondary 7.59 data.
  Their historical origin does not establish a native-7.48 server equivalent.
  Exact historical schedule, monster balance, spatial dispersal, replacement
  interval and selling items back to Dionys remain outside this adaptation.
- Validation: complete `go test ./... -count=1` and `go vet ./...` passed.
  Focused Hell Gate tests cover all twelve ordered pair draws, all selected
  entrances/phases, installed-terrain reachability, duplicate deaths, progress
  serialization, version-2 upgrade preservation and invalid restoration state,
  configuration budgets and empty groups, all 252 defense origins, full march
  to protector damage, occupancy waiting, replacements and terminal states.
  Store tests cover weight boundaries, deterministic options, valid catalog
  IDs, lifecycle/runtime refusals, save failures, detached proposals and
  deduplication. The full suite initially caught six oversized NPC names; these
  were shortened and the complete suite then passed without relaxing the limit.
  Logs are in `build/hell-gate-historical/`. Subsequently published with owner
  authorization in server release `20261018T171412Z`, with game/API restart,
  all 7,352 active payload hashes and readiness verified. No gameplay run,
  client build, commit or push was performed for this correction.
- Remaining acceptance: start a new version-3 encounter and verify entry,
  selected entrance, both selected Liches, observers/party, reconnect, Dionys
  survival/failure, approaching/replacement mobs, shop purchases in each
  category, the rendered Sand Lich and the existing deadline reward with the
  active release. Historical caster AI/balance is not covered by these tests.

### BUG-004 — Blood Foema incoming damage

- Report: incoming attacks appear to deal full damage despite expected
  resistance/reduction.
- Findings: Mana Control (affect 18) currently absorbs up to 90% of incoming
  damage into MP at 1:1, after other mitigation; an MP deficit spills into HP.
  This is an existing project rule, not a proposed rebalance. Existing tests
  cover physical, magical, multi-hit, boss, summon and poison damage.
- Next check: reproduce with the exact skill/affect, MP, HP, attacker and damage
  type; distinguish an absent/expired buff from incorrect mitigation.
- Acceptance: observed HP/MP changes and displayed damage match the established
  rule with full, partial and empty MP. Do not invent new balance values.

### BUG-005 — Blood Foema mana behavior

- Report: mana consumption/control appears inconsistent; the specific cast and
  resource sequence have not been identified.
- Findings: Mana Control does not discount casting costs. Holy Judgment's HP
  sacrifice bypasses incoming-damage absorption by design in the current rule.
- Next check: obtain or reproduce a named skill, active affects, and before/after
  HP/MP values, including any Combat Control involvement.
- Acceptance: casting costs and incoming-damage absorption each follow their
  own validated contract without duplicate resource deductions.

### BUG-006 — Divine Judgment visual effect

- Report: damage and HP cost occur, but the corresponding animation/effect is
  absent.
- Findings: skill 30 already creates `TMSkillJudgement`. Local requests dispatch
  a `FlagLocal=1` preview; authoritative local results suppress duplicate cast
  visuals. The result gate alone does not explain the missing effect.
- Next check: trace the reproduced local request/preview and effect ownership,
  lifetime and assets against native 7.48 evidence before changing dispatch.
- Acceptance: the proper effect plays through for the caster and observers,
  with one authoritative damage/resource application and no unrelated flash
  interrupting the cast.

### BUG-007 — Nightmare party state on entry

- Report: a character entering while already in a party temporarily appears
  ungrouped; green party-member identification is missing until a delayed update.
- Additional requirement: a Group Nightmare ticket must transport the eligible
  party together into the same runtime, preserving social membership. Individual
  tickets must not transport other party members.
- Expected: party identity is available as members become visible on entry.
- Cause: joining an already-open shared zone refreshed each player's view
  before all party spatial positions were updated, retained previous movement,
  and omitted the party roster refresh after actor publication.
- Implemented: shared-zone joins now use the same post-commit batch relocation
  as initial entry: clear previous movement, register every position, reconcile
  visibility, then publish party identity. Existing eligibility, shared-tier
  runtime, schedule, capacity and ticket rules remain unchanged. Classification:
  `MODERNIZACAO_COMPATIVEL`; no client packet change.
- Validation: the new regression reproduced stale movement on shared entry in
  Normal, Mystic and Arcane before the fix and passed afterward. Tests cover
  initial and later party admission, immediate bidirectional visibility and
  party roster delivery, unchanged deadline, solo tickets, and critical
  persistence commit/rejection without premature movement or publication.
- Delivery: source and automated validation only; no publication or restart.
  Reconnection and encounter HUD recovery remain separately tracked in BUG-008.
- Acceptance: enter N/M/A with an existing party and verify immediate party
  identification on both sides, without movement or a periodic refresh; repeat
  with another party entering an already-open zone in the installed client.

### BUG-008 — Nightmare reconnect synchronization

- Report: after a disconnect, the character returns inside the encounter, but
  party information, protected-NPC information and remaining time are missing.
- Expected: reconnection restores current authoritative party membership and
  encounter HUD state; it must not reset the timer or revive protected NPCs.
- Confirmed causes: character entry reattached the stable UID to its runtime
  without replaying the encounter HUD; disconnect removed social party
  membership without retaining a return identity.
- Implemented (`MODERNIZACAO_COMPATIVEL`): replay the existing timer/protected-NPC
  packets after EnterWorld and actor visibility. Restore the original party by
  stable character UID and exact live runtime/party identity when a slot is
  available; never evict another member or reuse a recycled actor ID. A
  disconnected leader does not displace the promoted current leader. Explicit
  party dissolution cancels restoration. Persistence acknowledgement precedes
  disconnect party cleanup; reserved party mutations defer restoration to World.
- Entry semantics remain distinct: a group ticket requires the leader, consumes
  only the leader's ticket and transports eligible members together; a solo
  ticket transports and charges only its user. Recovery charges neither.
- Automated coverage: leader/member reconnect with two/three-person parties,
  admission/combat deadlines, current protected-NPC losses, mutual actor
  visibility and party identity replay, full/dissolved parties, expired/replaced
  runtimes, recycled actor IDs, an entirely disconnected party, solo recovery,
  leader-only ticket consumption and critical disconnect acknowledgement.
  Run `go test ./internal/game -run '^TestNightmare(Reconnect|GroupTicket)' -count=1`.
- Scope: social party return hints are process-local, like parties themselves;
  this does not add party persistence across a server process restart.
- Delivery: source and automated checks only; no client build, publication,
  service restart or in-game confirmation in this change.
- Acceptance: after reconnect, both the returning player and connected members
  see correct party state; the returning player sees the actual remaining time
  and current protected-NPC state without waiting for a kill or moving. Also
  check expired instances and a party changed while the player was disconnected.

### BUG-009 — Magic Chamber disappearing and ghost monsters

- Reports: monsters appear late, disappear/reappear, sometimes never return even
  after movement, or exist only visually and cannot be attacked. All Carta
  difficulties and rooms were previously reported affected. The latest Carta N
  report includes monsters disappearing permanently in the final room, blocking
  completion.
- Previous partial correction: explicit room centers/bounds and full visibility
  reconciliation at logical room edges were published in server release
  `20261017T145456Z`; focused regressions passed. This is not proof that the
  reported encounter now completes. The latest report's exact running client
  and server versions have not been verified.
- Latest request: a party must enter and remain in one execution; individual
  entry must not pull other party members into that execution.
- Implemented: Magic Chamber now allocates an independent runtime identity per
  run, like private Water executions, instead of reusing the configuration ID
  or rejecting unrelated runs through a global occupancy lock. Exact runtime
  ownership still gates visibility, collision and targeting. This does not make
  Magic Chamber resumable after disconnect or change its ticket party policy:
  default tickets accept a solo player or the leader's party; an explicitly
  solo rule admits only its user, including a nonleader already in a party.
- Relocation: after an entry commit or room transition, all members' spatial
  indices advance before any visibility reconciliation. Old air/published
  movement is cleared so actor creation cannot replay a previous-room route.
  The existing party roster is replayed after visibility creation to restore
  immediate party identification. Critical persistence publishes nothing before
  the commit acknowledgement. No new client packet is introduced.
- Classification: `MODERNIZACAO_COMPATIVEL`; internal authoritative isolation
  and publication ordering, not a claim of new native 7.48 parity evidence.
- Validation: concurrent parties at identical room coordinates remain isolated,
  preserve mutual visibility through a room transition and target only their
  own monsters. Solo admission while socially grouped stays separate; commit-
  acknowledged group and solo entry paths are covered. The complete
  `internal/game`, `internal/data` and `internal/model` test packages passed.
  The obsolete data assertion requiring a global Magic Chamber lock was updated
  to require independent executions while retaining all four rooms/final boss.
- Delivery: source and data only; no candidate build, publication, restart or
  in-game execution. The earlier disappearing-monster report remains open until
  the original encounter is verified in gameplay; ground markings are BUG-010.
- Next check: verify artifacts/version and reproduce the original room flow;
  compare live server actors, membership/bounds, visibility delivery and client
  actor lifecycle. Distinguish invisible live mobs from stale client-only actors.
- Acceptance: complete every configured room and the final room in Carta N/M/A
  without an unattackable live remainder. Include stationary viewing, movement,
  room transitions, repeated entry, and party participants; visibility repair
  must not create duplicate actors, rewards or kills.

### BUG-010 — Magic Chamber ground markings

- Report: ground runes/markings expected during passage between rooms are not
  shown. The player is unsure whether these objects are specifically runes.
- Next check: identify the native 7.48 transition object/effect and its trigger,
  assets and lifetime. Keep this separate from monster visibility unless a
  common cause is demonstrated.
- Acceptance: the proven native transition presentation appears at the correct
  location and time, and is removed at the proper lifecycle boundary.

### BUG-011 — Friendly Compounder and city guards

- Report: Compounder attacks nearby players; city guards must attack monsters,
  never players.
- Cause: Compounder, Guard and Guard_ definitions were classified as ordinary
  hostile monsters, so they entered player-targeting AI.
- Implemented: Compounder is a passive NPC. Guard and Guard_ are friendly NPCs
  with an explicit `city_guard` combat role, not a role inferred from their
  names or appearance. Their server AI targets only live public hostile
  monsters within the existing acquisition, attack and home-leash limits.
  Player summons, private-instance actors, friendly NPCs and guild-war towers
  are excluded. Ordinary player-damage boundaries also reject friendly NPCs,
  even if a stale target survives. Guard kills grant no player loot or EXP.
- Compatibility: legacy NPC JSON keys and persisted type values are retained;
  only the three specific definitions change classification. Monster balance,
  appearance and other guard-named merchants/dungeon monsters are unchanged.
  Classification: `MODERNIZACAO_COMPATIVEL`.
- Validation: actual definitions load as friendly roles; tests verify guard
  acquisition/monster damage, player immunity through direct stale-target
  calls, excluded targets, passive Compounder and guard activation without
  nearby players. The complete game/data/model test packages passed.
- Delivery: source and data only, not published or gameplay-tested.
- Acceptance: approach Compounder and every affected city guard with two
  players, including after reconnect; neither player takes NPC damage. Bring
  an eligible hostile monster into guard range and observe pursuit, damage,
  death and return to guard duty without player rewards.

## Validation and closure

The preceding diagnosis ran existing focused Go tests for Hell Gate progression,
restore, collision isolation and entry-window lifecycle; Mana Control resource
splits, depletion, casting costs, combat paths, mounts and lethal cases; and
physical double/critical MISS encoding. They passed but did not reproduce or
close these player reports. No gameplay fixes or new build were produced by
that diagnostic batch.

For each issue, record source changes, focused regression results, candidate and
installed artifacts, publication/activation, and actual gameplay evidence
separately when available. A deployment or successful build alone cannot close
a visual or encounter-completion report. Keep unresolved items open and update
this file instead of creating another parallel bug list.
