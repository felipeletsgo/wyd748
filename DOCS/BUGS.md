# Additional player reports

These are player reports and feature requests, not confirmed defects or
implemented changes. See the [gameplay bug register](bug-register.md) for
triaged issues, evidence and acceptance checks.

- Experience messages: allow hiding XP-gain messages to keep chat readable.
- Automatic equipment: support Ctrl + right-click to equip inventory items.
- Blood Foema damage and mana: the owner confirmed the current behavior is
  correct; no change is needed.
- Divine Judgment: damage and HP consumption occur, but the visual effect and
  animation appear to be missing.
- Black Foema Lightning: reported damage is about 300; verify the actual skill,
  equipment and target before changing balance.
- Noatun gate: the gate sometimes closes unexpectedly and blocks the return to
  town.
- Magic Chamber entrance: randomly appearing colored floor runes from the native
  client are missing.
- Greater and Lesser Exchange Stones: character renaming reportedly fails.
- Guild leader medal: hiding the medal reportedly fails; the reported native
  command is `guiloff` or `"`. Verify the command before changing dispatch.
- Adamantite: players report being able to upgrade an (E) set to (LE), which
  should not be permitted according to this report. Confirm item identifiers
  and the authoritative upgrade rules.
- Offline shop honor: the requested owner rule is one honor point per ten
  minutes of shop time, with a letter saying "You received 1 honor." The
  current implementation and its persistence status are recorded below; this
  entry is not approval of any additional reward contract.
- Celestial conversion: Health, Sephira and Pancake potion buffs disappear.
  Players request account-bound buffs shared by all characters on the account.
- Huntress Cargo Guard skill: the skill reportedly does not open storage.

## Status — 2026-10-20 (patches 10037–10038 published; gameplay pending)

| Report | Status |
| --- | --- |
| Experience messages | Already available: typing `exp` in chat toggles the XP chat lines (`FieldChatSubmitPolicy`). |
| Ctrl + right-click equip | Already implemented (`ItemShortcutPolicy`, Action::Equipment). |
| Huntress Backpack (85) | Implemented: the skill opens Cargo locally from anywhere; the server accepts Cargo moves and gold without a banker for a living Huntress who learned it (`hasBackpack`). The server no longer treats 85 as the W2PP gold shield. |
| Coin Armor (86) | Gold cost (Trade x 100) moved from 85 to 86; defense Trade/10 per the 7.48 skill help; C.C. rebuffs 86 instead of 85. |
| Enchant Frost (74) with claws | Owner rule: rejected with a claw on client and server; Hide in Shadow (75) and Another Soul (79) no longer require a bow. |
| Celestial conversion buffs | Item buffs (potions, food, Sephira) survive Celestial creation and form changes; skill buffs stay with their form. Account-wide buffs are a new mechanic awaiting approval. |
| Guild leader medal (`guildoff`, `"`) | Already implemented as in W2PP (`guildoff`/`guildon` mask guild and rank for self and observers); needs a reproduction. |
| Greater/Lesser Exchange Stone (774/775) | Implemented from the 7.59 DBSRV (`FUN_00471210`): with 774 and 775 in `Equip[13]` of two characters of the account, the next login swaps their names and removes both stones. Guild members must leave their guild first. |
| Adamantite (E) → (LE) | Owner rule implemented: only D sets (uniques 8/17/27/37) can become Legendary; E sets (10/20/30/40) are rejected without consuming the ore. |
| Blood Foema | Owner confirmed the current behavior is correct; no correction needed. |
| Magic Chamber floor runes | Native Rune_Slate objects lit per run, plus the 7.59 rune bonus boss (colors 1, 2 and 3 summon MCN_CruelZak/MCM_Gargoyle/MCA_Rujeper after room 4); see BUG-010. |
| Divine Judgment | See [bug register](bug-register.md); identify the native visual trigger and assets before changing the client. |
| Black Foema Lightning, Noatun gate | Lightning Storm tick damage and the client's local gate close are corrected in source; automated checks passed, but gameplay acceptance remains pending. |
| Offline shop honor | Owner rule implemented: an open ghost shop earns its owner 1 honor every 10 minutes, with the letter "You received 1 honor." The counter is persisted before the schedule advances or the letter is sent; a failed save rolls the counter back and retries on the next tick. Source and automated tests only. |

AUTOMATED TESTED (2026-10-20): `go vet ./...`,
`go test -count=1 -p 1 ./...`, the portal's 60 tests, and
`Build-Client.ps1 -NoDeploy` passed after the boss-data corrections
(3,282,438 architecture checks). The candidate and public patch 10038
executable SHA-256 is
`B2417172F15E968E26A6F6B1BB8451F2CE7125476000CE3295079EE01FF54A27`.
The server is active at `/srv/wyd-go/releases/20261020T230245Z`.
Local installation and actual gameplay/`CLIENT_TESTED` remain pending.

## Follow-up — 2026-10-20

- Repliction descriptions: all ten distributed 7.48 `itemhelp.dat` records
  (4016–4025) contain GBK Chinese text that the current client renders as
  corrupted characters. The client now replaces only these records at load
  time with English descriptions of the current server behavior: matching
  grade, armor-only use, replacement of both bonuses, preservation of
  refinement, and the normal/premium +6/+9 limits. The original asset is
  unchanged. Architecture regression checks cover all ten headings, both
  limits, and tooltip line widths. Source only; visual acceptance pending.
- Rake pants 30 damage/7 critical: the owner's estimated item is Rake pants
  +10, but no item capture is available and the reported 25.3%/15.4% values
  match a +11 multiplier in the current formulas, not +10. The owner thinks
  this may already be corrected. Do not change refinement or critical
  arithmetic without a fresh reproduction identifying the actual item,
  refinement, and option bytes.
- Grade-E Repliction appearance: the distributed 7.48 `ItemList.bin` maps
  item 4020 to the grade-D mesh 2837; item 4025 already uses the distinct
  grade-E mesh 2860 (`smel05.msa`). The client loader now maps 4020 to 2860
  when it encounters the old row, and the server catalog agrees. The loader
  regression test covers D, E, and permanent E. This is a coordinated source
  correction; the shipped binary asset is unchanged, and the rebuilt client
  and in-game appearance still need validation before publication.
- Glory Ankh critical at +9: the server now applies the client-source
  position-mask (`Pos & 0xF00`) effective refinement promotion to item
  attributes without changing stored refinement. The catalog's base 5% now
  becomes 10% at +9 instead of 9.5%; ordinary equipment keeps its existing
  calculation. `TestAccessoryPositionRefinementBonus` covers +8, +9, +10,
  +15, and a non-accessory control. This is supported by the current client
  source and the reported expectation; native 7.48 runtime validation and
  in-game acceptance remain pending. Source only, not published.
- Tiny weapon option transfer: the fixed 100% chance was replaced in server
  source with the donor-ID/refinement table recovered from the bundled 7.59
  `TMSRVIN.exe` decompilation. Opcode `0x3C0` calls the `0x4B24C3`
  jump thunk, which targets the table reader at `0x4DB0C0`; the same handler
  tests `rand() % 100 <= threshold`. The binary SHA-256 is
  `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642`.
  Effective chances are 26/38/43%, 30/42/48%, and 34/46/54% for the three
  donor-ID tiers at +9/+10/+11 or higher, respectively. The effective values
  include the native `rand() % 100 <= threshold` comparison. Unsupported
  donors and donors below +9 are rejected; the target and catalyst must be
  exactly +9. `TestTinyChanceFromServer759Table`,
  `TestTinyRecipeUsesDonorTierAndRefinement`, the Tiny persistence tests, and
  `go test -count=1 ./internal/game` passed. This is a requested adaptation
  from later-server evidence, not proven native 7.48 parity. It is source-only,
  unpublished, and pending in-game verification.
- Eighth skill fee (owner correction): Mortal 5,000,000 gold; Arch
  50,000,000 gold; Celestial/SubCelestial 100,000,000 gold. The server now
  validates and deducts the fee by evolution for any branch's eighth skill.
  `TestUltimateSkillCostByEvolution` covers insufficient and exact balances;
  `go test -count=1 ./...` passed. Source only; not published or
  tested in game.
- Master capes (Celestial/SubCelestial): generic refining powder is rejected;
  their dedicated Odin composition remains available. Focused normal and
  critical-persistence tests passed. Source only.
- Capture lineage (owner correction): the fourth mastery grants neither HP nor
  defense. The earlier 2 HP/0.5 defense per point (and the interim 3 HP/1
  defense) was the reported bug and is removed; TK magic still uses the higher
  of the second and fourth masteries. `TestFourthMasteryAddsNoHPOrDefense`.
  Source only.
- Uxmal position: the NPC generator moved from (1057,1717) to the Rune Track
  entrance (3297,1693), as in the 7.59 `NPCGener.txt` generator 5661.
  `TestUxmalStandsAtTheRuneTrackEntrance` checks the position and walkability.
  Source only.
- Rune Track admission correction (2026-10-21): an entry with no eligible
  party member now remains in `entry` instead of advancing to `preparation`.
  `TestRuneTrackEmptyEntryStaysPending` covers the fail-closed path. Source
  only.
- Limit test item (owner request, 2026-10-20): `Limit_Test_Amulet` (6015)
  reuses the Crystal Amulet (563) appearance, has no level requirement, is
  untradeable (`EF_NOTRADE`) and adds 100000 STR/INT/DEX/CON in the server
  catalog. ShopAmulet sells it for 1,000 gold. Test:
  `TestLimitTestAmuletAddsAttributesAndIsSold`. Remove it after the limit tests.
- 32-bit limits (`EXTENSAO_COORDENADA`): the client catalog effect value is now
  32-bit in memory (`STRUCT_STATICEFFECT`, runtime `STRUCT_ITEMLIST` 204 bytes)
  while `ItemList.bin` keeps the native 140-byte rows. `ItemListWide.csv`
  (`index,effect id,value`) carries every server catalog value outside 16 bits;
  `TestItemListWideMatchesServerCatalog` keeps both sides equal, and the
  client loader rejects malformed files atomically. The client build and patch
  builder also fail closed when this required sidecar is absent. Audit: the
  server score is
  35 uint32 fields (cap 2,000,000,000), HP/MP travel in 32-bit `SetHpMp`, and
  real damage in the 32-bit DMGX tail. The client's last 16-bit narrowings were
  removed: local pending HP (`m_nReqHP`, which truncated HP above 65,535 and
  capped heals at 100,000) and the big-HP-pool `short` cast. The floating hit
  number no longer disappears above 1,000,000. Heals now carry their real
  32-bit amount in the DMGX slot (`wideTailValue`, client `ReadHeal`) instead of
  the 32,767 prefix word. EXP is uint32 on both sides with no server cap; the
  gain text no longer passes a 64-bit value to an `int` format. Remaining
  16-bit projections are native display packets only: party HP (`/100` above
  32,000, ratio preserved) and the Spirit's Seal capsule's stored base stats
  (32,767).
- World bosses moved (owner request, 2026-10-20): Black Archmage (3910,3418),
  Guardian of the Ruins (3806,4019), Demon Lord (731,3173), Ancient
  Necromancer (2083,347). Four Armia NPCs at y=2095 (ArchmageGate 2093,
  GuardianGate 2096, DemonGate 2104, NecroGate 2107) clone the Gravedigger
  confirmation window and run free repeatable quests 19–22 that teleport to
  the entrances (3893,3418), (3792,4006), (711,3169) and (2087,328). The boss
  files' comments are translated. Test: `TestBossGatesTeleportToTheBossEntrances`.
- Gate audit (2026-10-21). `init_items.csv` row = 7.59 `g_pInitItem` index - 1.
  Startup rule here and in W2PP: KEYID 1–14 locked, everything else open.
  | Rows | Gate (item, KEYID) | Key / controller | Expected (W2PP, 7.59) | Current |
  | --- | --- | --- | --- | --- |
  | 46–48 | Training field gates (458/459/461, 2–4) | First/Second/Final_Gate_Key 451–453 | locked; key opens; relocks | fixed: relocks 10 s after opening (owner rule), never on a player in the mask |
  | 49–52 | Orc castle gates (462–464/468, 5–8) | Orc castle keys 465–467/469 | same | fixed: 10 s relock |
  | 53–61 | Zakum castle gates (757–761, 10–14) | Dragon_Emblem 751, Phoenix_Emblem 752/754–756 | same | fixed: 10 s relock |
  | 45 | South_Gate (457, 1) at (64,64) | no key | locked map-corner dummy | locked (correct) |
  | 0–11 | City-war arena gates (471, 9), 3 per city | `SetArenaDoor` (city 0–3: Armia, Azran, Erion, Nippleheim) | locked; open while the battle runs | fixed: open from +6 to +31 min of city-war combat, locked otherwise |
  | 12–13 | Colosseum Steel_Window (472, 9) | `SetColoseumDoor`, Battle Royale | open at startup, locked by the event | fixed: open at startup; the Colosseum locks them from minute 5 to 17 (no Battle Royale event yet) |
  | 14–18 | Colosseum gates (471, 9) | `SetColoseumDoor2`, Colosseum event | locked, opened by the event | fixed: Colosseum opens them at minute 7 and locks them at minute 17 |
  | 35–40 | OX quiz gates (471, 9) | 7.59 `sub_561C10` | locked, driven by the OX event | fixed: staff panel "OX arena" (`game.event.ox`) locks/opens them |
  | 41–44 | Noatum castle gates (773/800, 15/16) | `SetCastleDoor`, castle war | open; locked during the castle war | always open (no castle battle) |
  | 66–70 | Victory's_Tower (3145) | 7.59 init (`EF_GROUND` 8) | 7.59 locked; W2PP open | open (sunk) — confirm 7.48 |
  | 71–72 | Gate_Of_Time (4101, 19) | 7.59 init + `sub_578CE0` | 7.59 locked; not in W2PP | open — confirm 7.48 |
  | 73–74 | Slot machines (4102/4103, 20) | `gamble.go` | open, interactive | working |
  | 19–34 | Rune slates (1733–1736, 17) | Magic Chamber | per-run lit slates | working |
  | — | PvP door (4143 First_Gate, 21) at (2123,2143) | 7.59 GM `pkgate` (1 locks, 0 opens) | open; closed only by a GM | fixed: server-only fixture, staff panel "PvP door" (`game.world.pvp-door`) |
- Colosseum (7.59 TMSRV `sub_567780`, 2026-10-21): every day at
  EventHour+1/+2/+3 (`colosseum_event_hour`, default 17, guild-war timezone),
  which gives N/M/A starts at 18:05/19:05/20:05 by default.
  Colosseum N (mortals only), M (no Celestials) and A (one Elixir_Notice 3170
  each from the 60 scanned carry slots, `_NN_Need_Elixer` and recall
  otherwise; a pending or rejected charge is retried every tick) run from
  `data/NPCGener/Coliseu{N,M,A}.txt` with the 16 `COL_*` monsters. Minute 5
  locks the steel windows and filters the arena, waves spawn at minutes 6, 9,
  11, 13 and 15 (3–5 GenerateMob calls per label), the inner gates open at
  minute 7, and minute 17 moves players out to (2615,1727) (the dead are
  revived and their corpse removed, through the critical persistence
  coordinator in production), reopens the windows, locks the gates and
  deletes the event mobs. N/M keep recalling ineligible classes that walk in,
  skipping players with a pending write. The base NPCGener arena spawns
  (MinuteGenerate -1) are reserved for the event; the generator files load
  only while `colosseum_enabled` is true. Project choice: a server that misses
  minutes 5–6 skips that hour instead of firing every missed wave at once. The
  daily reset runs before the tier guard, so the completed stage cannot leak
  into the next day's first scheduled tier. The staff panel can advance a
  selected tier with `game.event.colosseum`; the same admission, elixir,
  wave and exit rules run for 12 minutes from the early start. An early start
  replaces that tier's scheduled session for the same local day in the current
  process. Day claims and operation receipts are not durable across restarts.
  Active events, pending player exits, claimed tiers and passed schedules block
  a new start; epoch/revision checks and receipts fence stale or repeated
  commands. Tests: `TestColosseum*`. Source and automated tests only.
- OX arena (7.59 `quizmode`, `quizdoor`, `quizresultx`, `quizresulto`): staff
  panel "OX arena" (`game.event.ox`). "Start OX event" enables quiz mode and
  opens all six gates in one World command. Starting an already enabled event
  does not reopen gates locked for an active question. OX has no automatic
  schedule. With quiz mode on the Entrance_Ticket
  1737 is spent and teleports to the `quizArena` (1340,1492) of its
  instances.json rule 51; with quiz mode off it keeps this
  project's Big Cube ticket (instances.json rule 51) instead of the 7.59
  `_NN_Only_quiz_time` reply. Results move the losing side to (1341,1465) and
  revive the dead there. Tests: `TestOX*`, `TestOXEventHTTPAuthCSRFAndRevocation`.
- Elixir_Notice (3170) shared volatile 91 with Mount_Recovery_Potion, so using
  it raised an equipped mount to level 100. It is now `no_direct_use` (server
  rule and client route `Interaction`).
- Throwing Knife (94): the current skill catalog and client/server cooldown
  paths both use five seconds. The reported +9 glove interaction still needs
  native 7.48 verification; no cooldown change was made.
- Earring +10: the current server source uses a 16% success chance. The +11
  chance remains unverified and must not be treated as a native value.
- Mana Control: the owner keeps the 90% MP absorption rule.
- Lightning Storm (37): each 8-second tick now hits as a Thunderbolt (33) cast
  by the owner (W2PP ProcessAffect Type 22), instead of the bare affect value
  (about 300). Test: `TestLightningStormTickHitsAsThunderbolt`.
- Native client checks (7.48 Ghidra): the `guildoff` control 299 and the
  `'`/`"` key match the native (`FUN_004662c5`, `FUN_0045288b`). Holy
  Judgement (30) uses the native TMSkillJudgement type 0, constructor,
  destructor and visual gate (`FUN_00489a3e`, `FUN_004e6590`, `FUN_004e74b5`,
  `FUN_004e754b`); no static defect found, an in-game capture is needed.
- Noatun gate lead: `TMGate::FrameMove` marks a gate closed and unauthorized
  beyond 33 tiles and applies closed collision until the server resends it;
  server item view range is 32. The 7.48 native export confirms that local
  behavior. Source correction removes only the target-distance close/delete;
  authoritative gate updates and visibility removal remain intact. The
  `-NoDeploy` client build and source contract test passed; the candidate
  `tmproject/build/TMProject748/Release/WYD.exe` has SHA-256
  `A4C78CFA74786F6A9573B5887A09D73DDE37888873BB4A80E3A1F6AF3C40A5FC`.
  It was not installed or tested in game; see the [gate evidence record](../.agents/research/client748/flows/gameplay/gate-target-distance-closure.md).
