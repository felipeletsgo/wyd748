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
- Offline shop honor: request one honor point per ten minutes of shop time,
  with a letter saying "You received 1 honor." This is a requested mechanic,
  not an approved reward contract.
- Celestial conversion: Health, Sephira and Pancake potion buffs disappear.
  Players request account-bound buffs shared by all characters on the account.
- Huntress Cargo Guard skill: the skill reportedly does not open storage.

## Status — 2026-10-20 (source only; not published)

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
| Magic Chamber floor runes | Owner-defined client generator implemented: entering the entrance hall projects random colored runes on the floor (BUG-010). Build and policy tests passed; visual acceptance pending. |
| Divine Judgment | See [bug register](bug-register.md); identify the native visual trigger and assets before changing the client. |
| Black Foema Lightning, Noatun gate | Lightning Storm tick damage and the client's local gate close are corrected in source; automated checks passed, but gameplay acceptance remains pending. |
| Offline shop honor | Owner rule implemented: an open ghost shop earns its owner 1 honor every 10 minutes, with the letter "You received 1 honor." |

AUTOMATED TESTED (2026-10-20): `go vet ./...` and `go test -count=1 ./...`
passed before the latest catalog mesh adjustment; focused
`go test -count=1 ./internal/data ./internal/game` passed afterward.
`Build-Client.ps1 -NoDeploy` passed after the latest client changes
(3,282,421 architecture checks); current candidate WYD.exe SHA-256
`B8FA01A4DA17660D59CD1A43F33B951D626F34536E0258831005A57E85A4B8D5`.
Installation, CLIENT-TESTED, and publication are pending.

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
