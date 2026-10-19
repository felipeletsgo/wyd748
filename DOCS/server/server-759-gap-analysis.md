# Server 7.59 mapping and WYD-Go gap analysis

## Scope and evidence boundary

Initial investigation: 2026-10-18; expanded review: 2026-10-19. Source baseline: Git HEAD
`b0e8cfa547ce107a71fd559fa319737820f1316f`, plus the existing uncommitted
working-tree changes inspected during this audit. HEAD alone does not reproduce
this snapshot. The initial investigation made no gameplay changes. The
2026-10-19 war correction described under G04 adds local source and tests;
no installation, publication, VPS connection, service restart, or player
disconnection was performed.

This is a partial, evidence-backed map of the supplied historical **server
7.59**, not a declaration of native client 7.48 parity or a complete audit of
every system. Findings describe the local source, not the active VPS release.
Adopting a 7.59 feature requires a separate decision about the 7.48 client
boundary, authoritative state, persistence, and failures. None of the missing
features below is classified as `PARIDADE_NATIVA` by historical origin alone.

Reference notation used below:

- `N:<line>` means the local-only, read-only file
  `references/decompilacao server/descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c`.
- Go paths are relative to `wydgo748/internal/`, unless stated otherwise.
  Runtime configuration/catalog paths beginning with `data/` are relative to
  `wydgo748/`; Go loader/test paths use the explicit `internal/data/` prefix.
- The existing corpus receipt in [Rune Track](rune-track.md) records SHA-256
  `F8C064D7B3E73600B244A743A1D3CC3F6E4DB66F2B44364367A2E1E825E664CD`.
  This audit reused that receipt; it did not perform a new immutable-corpus hash.
- The C file is approximately 4.2 MB and contains 1,445 function-section
  delimiters. That number includes runtime/library material and is **not** a
  count of understood gameplay functions or a coverage percentage.
- Companion `DestinyServer-main/TMSRV/run`, `TMSRV2/run`, and `DBSRV` files
  provide historical runtime/data context, not a complete authoritative source
  tree. Private account data was not needed for this investigation.

Hex-Rays weak declarations do not establish recovered function bodies. Compiler
runtime material and decompilation failures exist in the corpus. In particular,
the callees listed under unresolved work must not be filled with invented rules.

## Findings, ordered by action priority

### G01 — Player challenge acceptance stops at notification

**CONFIRMED — P2, incomplete implemented flow.**

Invariant: accepting a valid gameplay challenge must either create its
authoritative confrontation state or explicitly reject an unavailable feature;
an acceptance message alone is not a started confrontation.

- Reference: `N:63702`, opcode `0x39F`. On a valid acceptance, `N:63735`
  invokes `sub_4B23D8(mode, accepter, inviter)` after checking the invitation
  and participant state. That callee's body was not recovered in this file.
- Current implementation: `game/protocol_misc.go:17` creates the invitation;
  `game/protocol_misc.go:55` accepts it and consumes its TTL/context. Lines
  78-87 only send the two acceptance panels. Searches for the challenge state
  writers found no subsequent confrontation controller.
- Trigger: two nearby players send a valid request and acceptance, including
  modes advertised as 5 versus 5, 10 versus 10, or all versus all.
- Result: both receive acceptance, but no confrontation state, participant
  assignment, start transition, or completion lifecycle is created by this flow.
- Test boundary: `game/protocol_misc_test.go:55` verifies consumption,
  notification counts, replay, and expiration. It does not assert a started
  match. Its passing result does not close this gap.
- Next investigation: resolve the historical callee and the target 7.48 client
  contract before specifying teams, destinations, timing, or rewards. None of
  those details can be inferred from the acceptance labels.

This is not the separate city registration handshake `0x28E/0x28F`.

### G02 — Four historical crafting routes have no Go implementation

**CONFIRMED — P2 content gaps; client availability is not yet established.**

Invariant: if one of these historical recipes is offered as supported content,
its request must reach a validated, transactional server recipe. Presence of
related items or a crafting window is insufficient.

The dispatch in `game/world.go:2269` and the strict allowlist in
`game/security.go:191` contain existing crafting routes, but not these four.
The parser gate at `game/security.go:167` rejects unknown opcodes. Searches of
the crafting handlers, celestial progression, item actions, and alternative
NPC routes found no equivalent implementations for the recipes below.

| Reference route | Recovered behavior | Current gap and unresolved evidence |
| --- | --- | --- |
| `0x2C4`, `N:49613-49779` | Spirit-stone combination; results include 631, 632, and 633, with EXP costs and a distinct failure branch. | No route or equivalent recipe. Ingredient validator `sub_4B25E0` is only a weak declaration, so the complete ingredient catalog remains unresolved. |
| `0x2E2`, `N:51199-51287` | `CombineGod2Item`; checks refinement, catalyst, and secret stones 5334-5337. Success uses `rand() % 100 <= 40`; failure retains the second equipment item after consuming other inputs. | No route or equivalent recipe. Output mapping `sub_4B264E` is unresolved. Related celestial uses of secret stones are not this recipe. |
| `0x2E6`, `N:51418-51545` | `Amunla Stone`; checks class, quest, internal level, EXP, and items 673-677 plus 770; produces item 3464 and charges 1,000,000,000 EXP with level recalculation. | No route or equivalent recipe. The admission threshold stored in `qword_63E580` is unresolved; do not equate it with the known EXP charge. |
| `0x2E9`, `N:51546-51644` | `GodUnseal`; items 677, 677, 678, 678 produce 679 on `rand() % 100 <= 40`; inputs are consumed before the outcome. | No route or equivalent recipe. An in-game 7.48 emitter and recipe availability have not been proven. |

Important details for a future implementation decision:

- For the 100 possible remainder values, `<= 40` accepts **41 outcomes**, not
  40. This describes the historical branch, not a newly approved probability.
- The recovered `0x2C4` branches assign 10 remainder values to 633, 30 to 631,
  30 to 632, and 30 to failure. The zero remainder belongs to the 631 branch.
  Success charges 2,000,000 EXP; the failure branch charges 500,000 EXP and
  preserves the first two inputs after other ingredients have been consumed.
- Adapted client source names `0x2C4` as `MSG_CombineItemLindyAlt_Opcode` in
  `tmproject/TMProject748/internal/core/BasedefPackets.h:123`; the dormant
  combine-mode path in `internal/app/scenes/TMFieldSceneMix.cpp` is a research
  lead, not native 7.48 or actual gameplay proof.
- Existing `game/crafting.go` validates authoritative inventory and duplicate
  slots. Do not reproduce unsafe historical mutation or failure ordering merely
  to copy a reference. A future adaptation needs commit/rollback, replay,
  disconnect, inventory-space, EXP-floor, and publication tests.
- Do not whitelist these packets independently of a proven parser and recipe.
  Rejection currently fails closed; it is not a security defect to preserve.

### G03 — Historical per-channel chat toggles are absent

**CONFIRMED — P2 functional difference.**

Invariant: a supported channel opt-out must update recipient filtering rather
than appear as ordinary nearby chat.

- Reference: the bare `0x333` commands `whisper`, `guildchat`, `partychat`,
  `chatting`, and `kingdomchat` are recognized at `N:48091-48110` and toggle
  per-session fields at `N:48179-48222`.
- These are not just cosmetic acknowledgments: private-message delivery checks
  `dword_61AB43C` at `N:52802`; guild delivery checks `dword_61AB440` at
  `N:52904`; citizen and kingdom deliveries check their fields at `N:52926`
  and `N:52952`. The `partychat` token is labeled **Citizen Chatting** in this
  reference; it must not be silently interpreted as the ordinary party channel.
- Current implementation: `game/commands.go:152` handles local chat and only
  the existing bare guild affiliation commands before the normal command/chat
  paths. The command aliases at line 337 contain none of these five toggles.
  Local relay at line 190 and `deliverWhisper` at line 541 have no corresponding
  per-recipient opt-out state. Guild affiliation masking in
  `game/guild_disable.go` is a different feature.
- Trigger: send bare `whisper`, then receive a private message. The word follows
  the local-chat path instead of disabling subsequent private messages.
- Test gap: no end-to-end toggle/filter coverage was identified. A future
  adaptation must cover sender/recipient, privileged bypass policy, lifecycle
  resets, and the citizen-versus-party naming ambiguity. The local-chat flag's
  complete historical consumption path still needs tracing.

### G04 — Full Noatum castle battle controller is absent

**CONFIRMED — P2 content gap, not absence of all guild wars.**

Invariant: event-owned castle generators must be driven by a complete battle
controller rather than ordinary spawn/respawn behavior.

- Reference: scheduler `sub_567780`, `N:72224`, drives the castle rectangle
  `(1036,1672)..(1144,1764)`. The branch at `N:72270-72330` advances phases,
  performs region/door operations, generates event mobs, and invokes an end
  transition. End routine `sub_55EAB0`, `N:68485`, performs castle-region
  operations, publishes state, and removes event mobs. The subsequent trace
  located seal capture in `sub_55EC40`, `N:68529-68805`: the guild leader must
  occupy `(1046,1690)`, belong to the channel's authorized owner list, and
  retain a valid capture context. The counter completes after exceeding 22;
  it is not a 23-second timer. The transition uses `+650/-650` fame calls.
  Door/breach integration and the full native 7.48 client boundary remain
  unresolved, so this evidence is not a complete playable implementation.
- Current implementation: `game/guild_war_generators.go:9-21` reserves the
  Noatum Tower of Thor generators and explicitly excludes an ordinary spawn
  substitute. The complete controller is absent from `game/guild_wars.go` and
  the inspected registration, scheduler, and economy paths.
- Impact: enabling ordinary towers or waiting for Sunday will not supply the
  missing castle battle lifecycle. This is a missing event, not evidence that
  current city/tower registration, treasury, or rewards are generally broken.
- Existing scope is documented in [Guild wars](../guild-wars.md): four city
  wars and the separate tower event are implemented. Historical generator
  indices must not be copied into installed data without identity validation.
- Implemented locally on 2026-10-19: city conquest now applies `+50/-50`
  fame with a zero floor, following `sub_566BE0`, `N:72089-72093`, and the
  adjustment body `sub_588340`, `N:85724-85749`. City settlement also persists
  castle ownership reconciliation from `N:72134-72168`; it retains an owner
  matching any city, otherwise selects the first nonzero city owner. The
  native zero comparison is preserved. Fame, ownership and completion share
  one transaction; rollback, rejection, acknowledgement, retry and replay
  tests cover the change. Public war cleanup, movement and combat exceptions
  now respect private runtime ownership. This closes those settlement and
  isolation gaps, **not G04 as a whole**.
- Next implementation gate: resolve door/breach actors and the 7.48 castle
  state/capture presentation boundary, then implement and test the complete
  battle controller, admission, interruption and reconnect/recovery. The
  existing defense-count cap and tax policy remain separately documented
  differences. Do not synthesize rewards or extend schedules from another
  source.

### G05 — City owners cannot configure the historical tax rate

**CONFIRMED divergence — P3, product/economy decision required.**

Invariant for a configurable-tax feature: an authorized city owner can update
the rate within the approved range and cadence, with durable atomic pricing.

- Reference: bare `guildtax`, `N:48224-48265`, finds an owned territory,
  permits values 5 through 25, and records a per-city change flag. Five slots
  are scanned. The reset cadence and fifth territory's full meaning have not
  been mapped here.
- Current implementation: `game/guild_economy.go:15` fixes the rate at 10;
  `cityShopTax` returns that constant at line 59. No `guildtax` route was found.
- Impact: the owner cannot reproduce the historical configurable-rate flow.
  This is **not** a finding that Go has no tax or treasury: normal shops,
  collectors, treasury limits, and atomic rollback already exist. Focused
  arithmetic/rollback tests passed during this audit.
- Decision: retain the fixed rate or explicitly approve owner configuration.
  Do not silently change the economy to 5-25%, import a fifth city, or assume
  a daily reset. Implementation would need persisted rate state, authorization,
  policy bounds/cadence, restart recovery, and shop-price consistency tests.

### G06 — Guild raid / Legend_Verid lifecycle has no mapped counterpart

**PROBABLE content gap — not yet a fully specified implementation task.**

- Reference entry: the volatile-effect `244` branch at `N:59116` checks the
  guild credited with the Khepra kill, `WeekMode`, raid state, and guild rank.
  It creates generators 156-158 and controls an arena door at `N:59165`.
- Reference lifecycle: death handling at `N:75694` calls
  `CRaidQuest::ProcessEnd`; `sub_589310`, `N:86211`, cleans leader/party state,
  relocates participants, deletes the raid generators, and changes event state.
- Go inspection: no equivalent raid controller, Khepra-winning-guild admission,
  or raid-specific lifecycle was found in gameplay code. Active
  `data/volatiles.json` and `data/instances.json` contain neither effect 244
  nor the inspected ticket-ID overrides 1777-1780. Generic instances exist,
  but are not by themselves proof of this guild-gated event.
- Companion data: historical `DestinyServer-main/TMSRV/run/itemlist.csv:1565`
  through 1567 bind Kefra Scrolls 1777-1779 to effect 244. Item 1780 is a
  separate Anubis Scroll with effect 0, not a fourth effect-244 ticket. These
  companion files are not independently proven to match this binary's build.
- Remaining uncertainty: delegated helper bodies, complete admission/settlement,
  and the generic-instance alternative need a focused equivalence audit. Do
  not claim the raid's exact native rewards, population, or finish rules are
  known merely from its generator and cleanup hooks.

### G07 — Voluntary guild rank / leadership handover is absent

**CONFIRMED — P2 content gap.**

Invariant for a supported handover: an authorized member can transfer rank to
the chosen eligible recipient while both retain guild membership. Departure
succession does not implement that operation.

- Reference: the bare `handover` branch at `N:52012-52097` checks the guild
  day, online target, guild/kingdom bindings, relative ranks, and another target
  flag. It charges the caller, clears crown/kingdom-dragon state, swaps rank
  fields, recalculates both scores, and publishes both members' state. The
  branch is not restricted to rank 9; fee constants and the unknown target
  flag need separate interpretation before specifying a recipe for adoption.
- Current implementation: aliases and dispatch in `game/commands.go:337` and
  `game/commands.go:410` expose creation, invitation, acceptance, departure,
  expulsion, and subleader promotion, but no handover. The alternative writer
  `game/guild.go:335` promotes the oldest remaining member only after the
  current leader is removed. `guildCommandSubLeader` at line 562 assigns a
  free subleader rank, not leadership. The critical-persistence departure path
  at `game/critical_persistence_guild_membership.go:178` uses the same removal
  and succession operation, not an independent handover.
- Trigger and impact: a leader wants a specific member to become leader while
  remaining in the guild. No current command performs that transition; leaving
  instead changes membership and chooses the oldest remaining member.
- Test boundary: `game/guild_flow_test.go:74` covers ordinary guild operations;
  line 138 covers departure succession. Neither proves voluntary handover.
- Before implementation: approve fees, rank policy, day/flag restrictions, and
  the 7.48 command boundary. Test atomic updates to both characters and the
  guild registry, crown/observer updates, replay, disconnect, and relogin.

### G08 — Cube admission omits historical class/level gates

**CONFIRMED — P2 admission divergence, plus a different entry rectangle.**

Invariant if historical admission is selected: every ticket use must apply the
approved tier's class/level and location predicates before consumption, whether
opening a Cube or joining an existing shared run.

The Cube itself is implemented. Active tickets 1772, 4000, and 4001 resolve
through effects 54, 171, and 172 in `data/instances.json:2241-2263`.
The historical entry branches differ as follows:

| Effect / ticket | Historical admission | Reference |
| --- | --- | --- |
| 54 / 1772, Normal | `Is2stClass <= 1` and internal current level `<= 199` | `N:60176-60202` |
| 171 / 4000, Mystic | `Is2stClass <= 1` and internal current level `<= 299` | `N:60203-60226` |
| 172 / 4001, Arcane | `Is2stClass == 1`; no level gate in this branch | `N:60227-60250` |

`Is2stClass` has only a weak declaration at `N:375`, not a recovered body.
Its numeric results above must not yet be translated into named Go evolutions.
The shared historical enable flag also needs interpretation.

- Current data: `data/instances.json:284` defines the shared-entry Cube;
  variants at line 907 inherit it. None sets `allowedEvolutions` or an
  equivalent tier-specific level predicate. The ordinary volatile catalog has
  no competing item override imposing those restrictions.
- Current flow: `instanceAllowsEvolution`, `game/item_instances.go:414`, permits
  an empty allowed list. Opening at line 1416 and shared joining at line 1156
  use generic admission, including that permissive predicate, but no Cube
  tier-specific level gate. The independent critical entry path in
  `game/critical_persistence_instance_entry.go:28` also lacks these Cube gates.
  Nightmare-specific eligibility is not a substitute for Cube eligibility.
- Location: the historical predicate is inclusive `(2368,1728)..(2404,1756)`;
  `data/instances.json:293-299` uses `(2370,1730)..(2410,1759)`. For example,
  `(2408,1758)` satisfies only the Go rectangle and `(2369,1729)` satisfies only
  the historical one. These are predicate counterexamples, not proof that a
  player can traverse every coordinate in the installed terrain.
- Impact: a character rejected by the historical tier rule is not rejected by
  these Go predicates and can enter/consume a ticket if other checks pass.
  This is a source-level divergence, not a reproduced live-client exploit.
- Test boundary: `internal/data/volatiles_test.go:357` checks production Cube
  variants, shared entry, and 25 stages, not admission parity. Generic evolution,
  shared-ticket, rollback, and area tests passed but do not cover this matrix.
- Before implementation: approve historical restrictions versus current access,
  resolve the evolution mapping, and test internal levels 199/200 and 299/300,
  all three tickets, rectangle edges, first entry/shared join, and both legacy
  and critical persistence paths. Rejection must not consume or teleport.

Sharing physical room coordinates across isolated `RuntimeID` instances is not
itself a defect; historical map offsets need not replace valid instance isolation.

### G09 — Scheduled BRItem arena event has no equivalent controller

**CONFIRMED — P2 content gap; full event specification remains unresolved.**

Invariant for a supported scheduled contest: admission, start, participant
restrictions, settlement, and closure belong to one authoritative lifecycle,
with rewards committed at most once.

- Reference: scheduler `sub_567780`, `N:72329-72468`, activates only when
  `BRItem > 0` and the configured event hour matches. Within 20-minute slots,
  minute 0 opens admission, minute 5 starts, minute 13 settles, and minute 14
  or later closes the state. The inspected arena is
  `(2604,1708)..(2648,1744)`. Start checks class by `BrGrid`, adjusts a pool,
  removes parties, and invokes region helpers. Party request/accept branches
  at `N:62785` and `N:62876` separately guard this event.
- Settlement: `N:72412` constructs reward item 4906 with effect 61 quantity
  data; another helper is logged as Fame. The survivor counter advances by
  **10**, not one, and the quantity calculation has conditional bounds. Do not
  infer an approved per-player reward or fame formula from labels alone.
  Region/door and fame helper bodies remain unresolved. A cape-specific death
  branch at `N:74354` also changes the pool while `BrState == 2`.
- Alternative-controller audit: `game/world.go:1873` schedules the existing
  global events and instance tick. `game/item_instances.go:2653` handles Rune
  Track, Hell Gate, Nightmare, Big Cube, and generic room progression, not this
  contest. Loader validation in `internal/data/volatiles.go:552-652`, the
  production instance templates/rules, guild wars, and active configuration
  have no BRItem state machine or equivalent schedule/settlement. City/tower
  wars use different battle regions and do not establish equivalence.
- Impact: this historical scheduled arena content cannot be run through an
  existing equivalent controller. This does not mean every historical server
  enabled it, nor that existing city wars, Big Cube, or challenges are BRItem.
  Active item catalogs also lack reward 4906, so copying the scheduler alone
  would not supply a valid 7.48 reward contract.
- Test gap and next decision: no equivalent phase, party restriction, death,
  reward-once, or recovery coverage exists. Recover helper/economy semantics,
  decide whether to offer this content, validate installed item/client support,
  and specify persistence/restart behavior before implementation.

### G10 — Party admission does not apply historical level/evolution limits

**CONFIRMED divergence — P3, gameplay policy decision required.**

Invariant if historical party restrictions are adopted: invitation and
acceptance enforce the same effective-level rule, including exemptions, rather
than a blanket 200-level restriction.

- Reference: invitation `0x37F`, `N:62775-62784`, and acceptance
  `0x380/0x3AB`, `N:62862-62870`, use base levels and `Is2stClass` results.
  A result of 2 adds 400 to effective level. A difference greater than 200 is
  rejected only when both effective levels are below 1000 and at least one
  class result is nonzero. Two zero results or either effective level at least
  1000 exempt the pair. Named evolution mapping remains unresolved, as in G08.
- Current implementation: `game/party.go:57-99` validates target identity,
  range, gameplay space, reservations, leadership, and capacity;
  `game/party.go:116-151` validates the invitation, TTL, membership, space,
  and capacity. Neither checks level/evolution admission, and no equivalent
  party admission helper was identified.
- Trigger: two otherwise eligible characters with class result 1 and internal
  levels 0 and 399 fail the historical predicate but are not rejected for
  level by Go. This is a static predicate comparison, not live gameplay proof.
- Impact: current cross-level grouping is more permissive. No recorded product
  decision establishing this as accidental was found; do not automatically
  remove that access or conflate admission with the separate party EXP policy.
- Test gap: packet-envelope, EXP, and membership coverage does not establish
  this historical admission matrix. Decide whether unrestricted grouping is
  intentional; if restoring restrictions, validate evolution mapping and both
  request/accept paths, including a level change while an invitation is open.

## Mapped entry points and subsystem comparison

These anchors make the investigation repeatable without rescanning the entire
corpus. A mapped entry point is not a completely traced call graph.

| Subsystem / entry | Historical anchor | Go counterpart / result |
| --- | --- | --- |
| Client request dispatch | `sub_519CA0`, `N:46128`; principal switch starts near `N:48065` | `game/world.go`, `game/security.go`; strict allowlist and authoritative handlers; selected routes inspected, not exhaustive parity. |
| NPC generation | `sub_514310`, `N:44232` | NPC generators and event-owned population paths; ordinary generators must not replace missing event controllers. |
| World scheduling | `sub_567780`, `N:72224` | Guild-war and event ticks; castle branch missing, city/tower controllers present. |
| Death and event cleanup | `sub_56C860`, `N:74141` | Combat/death and instance hooks; raid-specific equivalence remains open. |
| Player challenge | `0x39F`, `N:63702` | Invitation implemented, accepted-match transition missing: G01. |
| Crafting | `0x2C4`, `0x2E2`, `0x2E6`, `0x2E9` | Four missing recipes/routes: G02. Existing Lindy, Ehre, Odin, extraction, alchemy, and other recipes are not counted as absent. |
| Chat preferences | `0x333`, `N:48091`; delivery near `N:52802` | Missing channel toggles: G03. Existing chat and guild masking are distinct. |
| Noatum castle | `sub_55EAB0`, `N:68485`; scheduler castle phases | Full controller missing: G04. |
| Territory economy | `guildtax`, `N:48224` | Fixed 10% rate with treasury: G05, policy difference. |
| Guild raid | Effect 244; `sub_589310`, `N:86211` | Probable missing lifecycle: G06. |
| Guild handover | Bare command, `N:52012-52097` | Voluntary rank/leadership transfer absent; departure succession is different: G07. |
| Cube tier admission | Effects 54/171/172, `N:60176-60250` | Cube content exists, but class/level predicates and entry rectangle differ: G08. |
| BRItem scheduled arena | Scheduler, `N:72329-72468` | Missing configured contest lifecycle; helper/economy details unresolved: G09. |
| Party eligibility | `0x37F`, `N:62775`; `0x380/0x3AB`, `N:62862` | Historical effective-level restriction absent; adoption requires a policy decision: G10. |
| Hell Gate waves | `sub_578700`, `N:78898` | Existing `game/hell_gate.go`, `game/hell_gate_population.go`, and instance lifecycle; reuse the current event evidence, not older pre-adaptation reports. No new defect was demonstrated by this mapping pass. |
| Hell Gate item store | `sub_4E3D50`, `N:27456`; `4E4160`, `4E46C0`, `4E4AA0`, `4E4B60` | `game/hell_store_759.go`; [store contract](../protocol/hell-gate-store.md) records the historical pools and separate native 7.48 UI evidence. No fresh gameplay claim. |
| Rune Track admission | `sub_5847E0`, `N:84065` | [Rune Track](rune-track.md), `game/rune_track.go`; existing implementation/evidence reused. |
| Rune Track death, tick, cleanup | `5849B0`, `N:84134`; `584F00`, `N:84283`; `585990/585AC0`, `N:84540/84565` | Existing event objectives, independent monsters, rooms +0 through +6 and lifecycle; prior documented tests do not substitute for real traversal. |
| Rune Track settlement/rewards | `5843D0`, `N:83950`; `585BC0`, `N:84601` | Uniform per-member selection from each room's complete pool is an intentional project adaptation. The historical random lookup's dependence on party position must not be reintroduced. |
| Quiz | `0x2C7`, `N:49780` | Versioned `0x7F10/0x7F11` extension in `wire/quiz.go`, with token, deadline, and reward-once checks. Legacy rejection is **NOT A DEFECT**. |
| Channel switching | Go scope inspection only; historical route not resolved in this pass | `game/commands.go:376` explicitly reports unavailable: no channel coordinator or `0x52A` ticket. Known architecture limitation, not a newly proven 7.59 regression. |

## Unresolved research queue

1. Recover bodies or independent evidence for `sub_4B23D8`, `sub_4B25E0`,
   and `sub_4B264E`: challenge start, spirit-stone ingredients, God2 output.
2. Finish the castle helper graph and raid entry/death/end graph, including
   persistence, disconnects, observers, and restart outcomes.
3. Validate 7.48 client emitters/UI and installed item/NPC data before proposing
   any newly exposed crafting or event contract.
4. Resolve `Is2stClass` numeric-to-evolution mapping, handover fee/flag semantics,
   and BRItem region/fame helpers. Decide Cube access and party admission policy
   separately from recovering the historical algorithms.
5. Audit combat formulas, drop/EXP tables, all shop recipes, DB-server handlers,
   and all quest branches separately. Their presence in the corpus is not
   evidence that they were exhaustively compared here.
6. Trace broader territory modes, channel migration, and remaining dispatch
   cases independently; they are leads, not confirmed additional defects.

## Alternatives checked and exclusions

- Cubes are not missing merely because their tickets have no per-item volatile
  override: effect-based instance rules already implement all three variants.
  G08 concerns admission, not absence of the event.
- Quest reward items 4117-4121 already have item-specific level, evolution,
  EXP, and gold rules in `data/volatiles.json:1463-1510`. Absence of a generic
  effect-191 branch alone is not evidence that these rewards are absent.
- Historical rebuy opcode `0x3E8` is intentionally removed and rejected by
  `game/security_test.go:130`, not an accidentally unhandled supported feature.
- Rune Track's uniform per-member selection from the full room pool is an
  explicit project choice, not a defect to revert to party-position behavior.

## Validation performed and delivery state

The initial audit updated the documentation inventory. The repository layout
check still failed on three pre-existing paths outside the sanctioned layout:
`tmproject/client748/dumps/TMProject748_STACK_OVERFLOW_AUDIT_2026-10-17.md`,
`tmproject/TMProject748/build/`, and
`tmproject/client748/dumps/crash-monitor-p31348.log`. This audit did not create
or remove them. The 2026-10-19 war correction check also reported
`tmproject/client748/dumps/crash-monitor-p21384.log`, outside this correction's
scope. Layout validation is therefore not reported as passing.

The full working-tree `git diff --check` also reports pre-existing whitespace
in `tmproject/TMProject748/TMProject.rc` at lines 163, 168, 179, and 232.
That unrelated resource file was not changed by this investigation. The report
itself has no trailing-whitespace findings; a no-index comparison with an empty
file returns a difference status, not evidence that the whole tree is clean.

Static inspection followed selected requests through dispatch, runtime state,
alternative handlers, active data, and existing tests. Function addresses and
source lines above are the reproducible anchors; this is not an exhaustive
function-by-function semantic inventory.

Executed from `wydgo748/` on the inspected working tree:

```powershell
go test ./internal/game -run '^(TestPlayerChallenge.*|TestCityTax.*|TestQuiz.*)$' -count=1 -v
```

Result: **PASS**, exit 0, 14 top-level tests selected: three player-challenge,
three city-tax, and eight quiz tests, plus their subtests. These tests establish
their existing behavior, not the missing functionality. Expected injected
failure logs in tax/quiz rollback tests are not production failures.

The expanded review also ran:

```powershell
go test ./internal/data ./internal/game -run '^(TestVolatilesRealConfigDifferentiatesByItemIndex|TestGuildCreateInviteAcceptChatPromoteAndExpel|TestGuildLeavePromotesOldestMember|TestInstanceAdmissionUsesGameplaySpaceAndChecksEvolution|TestCubeSharedEntryConsumesOneTicketPerPlayer|TestCubeSharedEntryRollbackAndStageLock|TestInstanceScheduleAndEntryArea)$' -count=1 -v
```

Result: **PASS**, exit 0, seven top-level tests actually selected: one production
catalog test and six guild/instance tests. These preserve evidence for the
implemented alternatives; none supplies missing handover, historical Cube/party
admission, or BRItem coverage. The earlier 14-test result was reused, not rerun.

The initial source investigation required no new server/client build. The
2026-10-19 G04 correction subsequently implemented city conquest fame,
castle-owner reconciliation, dissolution cleanup and public/private war
isolation. The full Go suite, `go vet`, focused race tests and server candidate
build passed. The [guild-war record](../guild-wars.md#server-759-settlement-evidence-and-remaining-differences)
contains the validation coverage and candidate hash. Execution policy blocked
cleanup of `build/war-settlement-20261019/wyd-server.exe`; the candidate and
its directory remain, without retrying deletion through another mechanism.

No installed executable, published patch, active VPS hash, or in-game flow was
verified. Rune Track portals, Freyja's wager UI, Hell Gate traversal, and the
new findings are **not** newly `CLIENT_TESTED` by this audit. Existing pending
publication and the instruction not to disconnect players remain unchanged.

Delivery: mapping and findings documented; an initial local war settlement and
isolation correction is implemented and automatically validated. G04 remains
open for the full Noatum battle controller and client integration. No
deployment or restart was performed.
