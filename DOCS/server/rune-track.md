# Rune Track: rooms +0 through +6

## Delivery boundary

The authoritative Go server implements the seven-room event from the supplied
7.59 server decompilation and companion runtime data. The active configuration
is `wydgo748/data/instances.json`, Uxmal version `759`. The older version-zero
path remains available for existing configuration fixtures.

Classification: `MODERNIZACAO_COMPATIVEL`. Historical 7.59 gameplay is the
algorithm source, **not** proof of native 7.48 gameplay parity. The existing
7.48 environment-effect envelope has its own independently traced
[native contract](../../.agents/research/client748/flows/transport/environment-effect-contract.md).
No client source, resource, opcode or packet layout was changed.

Status on 2026-10-18: source, automated validation and VPS deployment complete.
After explicit owner authorization to publish and restart, release
`20261018T234132Z` activated at 23:41:35 UTC, disconnecting three established
game connections. Game/API processes, all 7,385 payload hashes, readiness and
unchanged private configuration were verified. See the
[delivery receipt](../current-state.md#active-release--2026-10-18-hell-gate-and-rune-track).
The existing launcher patch remains 10035; no client executable was changed.
Actual client/gameplay validation remains pending. This document does not
declare a global client/server release ready.

## Schedule, admission and lifecycle

- Registration opens at minutes 16–19, 36–39 and 56–59 of the server clock.
  Registration is closed throughout hour 00 and from 23:30 onward.
- The leader clicks Uxmal with ticket `5134` in the first 60 inventory slots.
  The first matching ticket's refinement selects room +0 through +6; the full
  slot is consumed, as in the historical flow. Invalid refinements are rejected.
- A party needs its leader and at least one eligible member, up to the existing
  13-player party limit. Eligible characters must be alive at the entrance
  `(3275,1673)`–`(3318,1712)`, have stable character UIDs and belong to no other
  instance. Duplicate registrations are rejected.
- Registration does not teleport the party immediately. At the next minute
  00, 20 or 40, the server resolves the registered leader's current eligible
  party and moves it into the room. Remain at the entrance until that time.
- Preparation lasts through the first 30 seconds. Combat starts at second 31;
  Amon appears at cycle minute 5. Settlement and cleanup start at minute 15.
- Parties in one room share one exact runtime space and compete over its
  objectives. Different rooms and the public map remain isolated. Arrival
  positions are terrain/collision checked and reserved across every party,
  including rooms whose historical starting coordinates coincide.
- Ownership uses both entity ID and character UID, not coordinates or a reused
  player ID. Registered-member leader transfer updates event leadership.
- Completion does not end the other parties' run. Final cleanup removes the
  event population and returns remaining members to the configured entrance
  near `(3297,1685)` through the existing durable instance-departure path.

The runtime is process-local: registration and encounter progress are not
restart-resumable. Character, ticket, gold, relocation and reward writes use
the existing critical-persistence coordinator. Detached writes keep proposed
state and packets hidden until confirmation; rejection rolls back for retry,
and an unknown outcome retains containment instead of issuing another grant.
This integration does not change the deployment's S2 enablement policy.

## Rooms and objectives

| Room | Party capacity | Generator labels | Objective |
| --- | ---: | --- | --- |
| +0 | 2 | 5662–5714 | Lich selection: killing the matching party Lich succeeds with probability 1/8; failure or the other Lich regenerates it. One winning party closes both Liches. |
| +1 | 3 | 5715–5774 | Tower defense: invaders march toward living towers. At settlement, a party wins only if its tower survived and its leader is alive. |
| +2 | 3 | 5798–5857 | Amon: the boss appears five minutes after cycle start. Arriving invaders heal it by 2,000 HP, capped at 1,280,000. Only party leaders may remain in the central boss area. The killing party wins. |
| +3 | 3 | 5957–5984 | Sulrang: killing Sulrang can reveal one mage when none is active. Mage kills score points. At settlement the highest strictly positive score wins; the first registered party wins an equal-score tie. |
| +4 | 4 | 5858–5907 | Labyrinth: one hidden generator among 5863–5902 opens the route to Shadow. Clearing applicable groups toggles floor danger. The party that defeats Shadow wins. |
| +5 | 3 | 5908–5956 | Balrog: ten portal rectangles use the historical destination probabilities. A portal consumes one full powder slot (`4032`) from the leader and moves that party. The Balrog-killing party wins. |
| +6 | 3 | 5775–5797, 5985–5988 | Golden rabbits: each party collects toward its target. Donations reduce its target from 100 to a minimum of 50. Completing a target can summon the Rabbit King. Its killing party receives runes and the historical gold award; arriving Zakums reduce counters without going below zero. |

The labyrinth uses 34 decoded floor rectangles, not one broad damage area.
An unprotected pulse subtracts 25% of maximum HP, with the historical
nonlethal 1,000-HP floor (bounded by character maximum HP). A registered leader
inside a rectangle protects that rectangle. Nearby event members receive the
existing `0x3A2` effect 32; public-map observers do not.

The +5 portals are configured in the event runtime, not in the general map
teleport list. During combat, every fifteenth event update (approximately
15 seconds), a living party leader inside a portal rectangle can activate
it with powder `4032` in one of the first 60 inventory slots. The destination
draw and party movement follow the room rules; inventory consumption and
movement publish only after persistence succeeds. The +4 hidden objective
instead schedules automatic party movement to Shadow at `(3351, 1335)`;
it does not require portal powder.

Every winning member receives one independent, uniform random draw from the
complete configured rune pool for that room. Draws use replacement: two or
more players may receive the same rune. Neither party registration slot,
member position nor the number of eligible members restricts the pool. A
party with only one, two or three remaining eligible members can draw every
rune in the room's pool; the minimum admission party size is unchanged.

| Room | Possible rune item IDs | Chance per rune per member |
| --- | --- | --- |
| +0 | 5110–5114 | 1/5 |
| +1 | 5112–5117 | 1/6 |
| +2 | 5115–5120 | 1/6 |
| +3 | 5121–5126 | 1/6 |
| +4 | 5124–5130 | 1/7 |
| +5 | 5118–5123 | 1/6 |
| +6 | 5128–5133 | 1/6 |

These pools preserve the union of historically possible rewards in each room,
without adding or removing rune IDs. For rooms below +6, the leader also
receives the next-refinement ticket. The +6 gold award is
`500000 * floor(2 * (sum(collected) + winningTarget) / 3)`, subject to the
existing character gold cap. A completed party cannot receive a second award.

## Independent event population

`wydgo748/data/RuneTrack759.txt` contains all 327 historical generator blocks,
with their labels, routes, timing, group limits and coordinates preserved.
They are owned by the event runtime, not the ordinary public-map scheduler.
`MinuteGenerate` retains the current NPCGener timing unit of 12 seconds.
Spawned groups are staged as a whole and require available walkable tiles;
failed allocation does not publish a partial group.

Thirty new `RT_*.json` templates under `wydgo748/data/npcs/` contain the
historical event EXP, stats, equipment, skills and carry/drop-slot data. They
do not alias or edit the ordinary monsters' definitions. Their independent
carry arrays retain empty slot positions and item effects. For example,
`RT_LichKing` has level 499, defense 5,350, attack 5,600, HP 32,000 and base
EXP reward 8,500 from the supplied 1,024-byte NPC record.
The existing JSON key `tipo` and enum value `monstro` are retained as
established loader/persistence identifiers, not new player-facing text.

Combat, EXP distribution and drop rolls still use the authoritative server's
existing engines. Independent historical inputs do not imply that every
7.59 combat/drop probability formula has been ported or validated in 7.48.

The fourteen event templates with nonempty skill bars now execute them during
combat. One percentile draw selects attack slots 0/1/2 with 50/35/15 percent
windows; an empty selected slot keeps the physical attack, without a reroll.
All imported fourth slots are empty, so no healing behavior is invented.
This selection follows historical `sub_4DECC0`, not player spell semantics:
monster attack versus defense and compressed elemental resistance feed the
existing supported skill-hit packet. Reach, hit accuracy, attack deadlines and
damage application keep the current server contract. Historical accessory
bonuses and complete monster AI/balance parity are not claimed by this fix.
Owned room actors cannot attack before second 31, at or after minute 15,
while settlement is active, or behind an unresolved instance reservation.
Public-world monsters and other instance skill selection are unchanged.
Room +1's historical invaders have empty bars and retain physical tower attacks;
the shared skill path also supports a configured tower attacker without
writing Hell Gate's protected-NPC state.

## Explicit compatibility adaptations

- Historical fixed global arrays and map-coordinate ownership become bounded
  room-local state, exact-runtime membership and stable character references.
  The +4 room retains four parties; +0 retains two, and the others three.
- Explicit product choice: historical `sub_585BC0` already draws randomly,
  but restricts the four-item pool using the party's room slot, not the
  recipient's position in its member list. The requested fairness change
  instead gives every eligible member equal access to the room's entire
  historical reward union. This is not claimed as historical reward parity.
- The historical +6 donation accepts gold before combat, but its Freyja spawn
  occurs at combat initialization. The adaptation makes Freyja available
  during preparation so the existing merchant-9 gold prompt is reachable.
  Only the registered leader may donate while alive and near Freyja. The
  supplied amount must first be affordable, then is capped at 50 million.
  Only the three valid room party indexes are accepted.
- Full-inventory rewards use the existing owned, instance-local ground-item
  fallback instead of silently losing the historical reward. Ground allocation,
  character changes and the party completion marker share the same grant;
  nothing is visible or collectible before persistence confirmation.
- Relocations use valid unoccupied terrain near the historical destination.
  Cleanup uses the configured entrance rather than the historical ID-parity
  scatter. No reconnect entitlement or restart replay is invented.
- Server messages are English. Historical binary names are converted to short
  `RT_` names within the existing 12-byte wire name budget; item IDs, generator
  IDs, equipment effects and refinement bytes remain unchanged.

## Provenance and reproduction

Read-only research root:
`references/decompilacao server/descompilacao tmsrv 7.59/`.

| Input | SHA-256 |
| --- | --- |
| `TMSRVIN/TMSRVIN.c` | `F8C064D7B3E73600B244A743A1D3CC3F6E4DB66F2B44364367A2E1E825E664CD` |
| `DestinyServer-main/TMSRV2/run/TMSRVIN.exe` | `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642` |
| `DestinyServer-main/TMSRV2/run/NPCGener.txt` | `6A5FF788BF9AE2C66D050421A644054C5E277EB874AF3109E4944EFD99C369A8` |
| Active `wydgo748/data/RuneTrack759.txt` | `82C9A738155D0955D08B44861A98DF4FBC30208E5914C7CD3B1C62E8E0F7BD1E` |

Anchors in that decompilation:

- Merchant 72 admission: lines 36167–36360; global cycle: 70453–70485.
- Settlement `sub_5843D0`: 83949–83983; initialization `sub_584530`:
  83992–84040; room entry `sub_5847E0`: 84065–84122.
- Objective death handling `sub_5849B0`: 84144–84260; periodic room logic
  `sub_584F00`: 84280–84500; cleanup `sub_585990` / `sub_585AC0`:
  84536–84588; rewards `sub_585BC0`: 84600–84720.
- Donation through `0x3D1`: 64385–64425; leader transfer: 76287.
- Generator population `sub_514310`: 44232 onward; event ID registration
  `sub_589AE0`: 86504–86520. Floor rectangles come from the executable's
  table at VA `0x646FA8`.

The reproducible converter is `wydgo748/cmd/runetrack759`. Run from
`wydgo748/`, specifying the read-only source and an ignored staging output:

```powershell
go run ./cmd/runetrack759 -source '../references/decompilacao server/descompilacao tmsrv 7.59/DestinyServer-main/TMSRV2/run' -output '../build/rune-track759/import'
```

It extracts only the event labels and their referenced NPCs, rejects an
unexpected NPC binary size or unrepresentable EXP/skills, emits independent
templates and a source-NPC SHA-256 manifest, and does not install them. Review
the staged data before applying a later import; historical input changes
invalidate the current import evidence. Delete disposable staging afterward.

## Automated checks and remaining gameplay checks

Passed on 2026-10-18 from `wydgo748/`:

```text
go test ./...
go vet ./...
```

Coverage includes production data loading, all 327 generators and 30
independent aliases, seven room objectives and rewards, four-party +4
capacity, shared-coordinate arrivals, registration and UID reuse, leader
transfer, donations, floor damage/protection/effect bytes, portal consumption,
objective invader AI, real installed terrain and final cleanup. Inline and
queued persistence cases cover rollback/retry, no publication before commit,
no duplicate rewards and containment for unknown commit outcomes. Converter
tests verify independent binary offsets, inventory holes and overflow failures.
Wire tests verify the literal native effect layout.

Reward tests cover all seven literal room pools, every party slot, member
reordering, one to three eligible recipients, every selectable rune,
independent draws, allowed duplicates and completion without a second draw.

Additional focused verification on 2026-10-18:

```text
go test ./internal/game -run '^TestRuneTrack|^TestStoreGoldRequest' -count=1
```

Combat follow-up: `go test ./...` and `go vet ./...` passed after the skill-bar
dispatcher correction. The focused
`Test(RuneTrack759|HellGate759|RuneTrackObjectiveMarchers)` selection passed
with a fresh run. Coverage checks all 100 percentile outcomes for all fourteen
production caster templates, real-dispatch skill and physical packets,
leader/member publication, public-observer isolation, attack deadlines,
combat-window expiry, missing skills and actor ownership. A configured-copy
tower caster checks objective death without contaminating Hell Gate state;
the production tower invader definitions are not modified for that test.
No client was launched and no deployment is implied by these results.

Portal coverage checks all ten rectangles and all 71 random outcome slots
against the installed terrain, including walkable, distinct party arrivals
and movement packets for both leader and member. Rejection cases cover a
dead leader, member-only activation, missing powder and powder outside the
first 60 slots. The +4 hidden passage test verifies arrival at Shadow and
retry after a persistence failure.

Freyja coverage checks that generator `5775` already exists during +6
preparation, and that the actual NPC publication carries head `51` and
merchant `9`. A 16-byte `0x3D1` confirmation passes through inbound validation
and world dispatch, deducts 50 million gold, lowers the target to 50 and
publishes the committed gold update. Member donations, insufficient funds
and repeated confirmations do not change the balance. The existing rollback
test separately checks persistence failure and retry.

The native gold prompt route is statically traced through `FUN_0045ee28`
(head 51, merchant 9, coin mode 7) and `FUN_004662c5` (16-byte `0x3D1`,
amount at offset 12), and is connected in the active client source. This is
an amount-entry panel used for the room donation, not a separate roulette
window. Static tracing and server tests do not prove that the installed
client displayed or submitted that panel in a live session.

Repository-wide checks have unrelated existing failures: the research
validator reports a missing `Gaps` section in
`.agents/research/client748/flows/combat/persistent-buff-visuals.md` (the new
environment-effect record passes). The layout validator reports
`tmproject/client748/dumps/TMProject748_STACK_OVERFLOW_AUDIT_2026-10-17.md`,
`tmproject/TMProject748/build/` and
`tmproject/client748/dumps/crash-monitor-p31348.log` outside their permitted
locations. These pre-existing files were preserved; the documentation index
was updated successfully. These failures are not gameplay acceptance.

Cleanup limitation: execution policy rejected deletion of
`C:\Users\xereca\Documents\PROJETOS\wyd-go\build\rune-track759\import`.
Its 32 disposable import files remain in the sanctioned ignored build root;
no alternative deletion mechanism was attempted. They are not runtime inputs.

Still pending: launch the exact installed client/server artifacts and play
each room with multiple parties; verify leader/member/public-observer
visibility, incremental spawns/deaths, objective AI paths, merchant-9 donation
prompt, portal and floor visuals, all reward routes, scene transitions,
disconnect/relogin behavior and final exit. Automated tests are not
`CLIENT_TESTED`, and no gameplay acceptance is inferred from them.
