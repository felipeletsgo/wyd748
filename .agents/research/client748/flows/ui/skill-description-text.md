---
id: skill-description-text
title: Native skill descriptions and isolated tooltip presentation
subsystem: ui
status: CONTRACT
native_sha256: 8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593
updated: 2026-10-30
---

## Question

Which descriptions actually exist in the native 7.48 runtime, why did some
appear missing, and how can the adapted client show them without calculated
requirements overwriting their text or displacing the learned-skill tooltip?

## Evidence boundary

- Native binary and Ghidra: the hash above, read-only project
  `tmproject/build/native-research/WYD748.gpr`; `FUN_00418828` (grid hover),
  `FUN_00435b13` (field-scene binding), and `FUN_0055bc0a` (startup help loader).
  Existing decompilations and the previous focused caller export were reused.
- Native content: read-only `references/OnGame/WYD/itemhelp.dat`, Windows-1252,
  SHA-256 `A5E49D5755F9081E4D2837B7F6774D8ADFE149D259B7274A3E7D1F1033737195`.
  It has 102 skill records: 5000..5095 and 5097..5102. Many decimal ID lines
  end in whitespace. The earlier strict scan incorrectly classified several
  eighth-skill/passive descriptions as absent.
- Materialized destination: `tmproject/client748/skillhelp.dat`, an ASCII
  English localization of those native descriptions, including formulas,
  effects, conditions and native ARGB colors. SHA-256:
  `D614DA42CE36B2C2CA61DEC6BF4DF964D047C28A4A3B618D53695C26FC967629` (2026-10-15);
  `DD8FA19CB3B882E5C4F65A3544A712F44D29699CBF0ABF112942DB7209D3ACD7` after the
  2026-10-29 implemented-effect updates below.
  This is translated content, not a byte-identical native asset.
- Existing `itemhelp.dat` is unchanged: its historical item text and ANSI
  encoding are not rewritten. The separate skill corpus overrides its
  incomplete skill prose after the original help file loads.
- Server/wire: N/A; explanatory text does not authorize skill learning,
  casting, damage, storage access or resource consumption.
- Secondary source: N/A; no later TMProject establishes native parity.

## Native 7.48 flow

### Callers

`FUN_004209fc` calls grid-hover handler `FUN_00418828`. Scene binding in
`FUN_00435b13` resolves control 6128 and calls the text loader at
`0x0043bfc8` with `UI\\descskillm.txt`.

### Callees

`FUN_00418828` clears twelve parameter controls, then copies nine item-help
rows into rows 2..10 through text/color setters (virtual offsets 0x80/0x84).
The subsequent skill branch applies calculated requirements to overlapping
controls. Merely moving the prose to row two did not isolate these writers.
`FUN_00435b13` uses `FUN_00442dd0` for the apprenticeship explanatory list.

## State and lifecycle

Startup clears the help table, loads item help, then overlays all 102 skill
records. Missing or incomplete skill corpus fails startup with an English
diagnostic instead of silently presenting partial descriptions. The bounded
decoder accepts surrounding ASCII ID whitespace, rejects invalid IDs/colors,
bounds text, and publishes only complete nine-line records.

The scene creates nine non-selectable local prose controls owned by the
existing description panel. Native control IDs and the twelve metadata
controls remain unchanged. Skill hover fills prose separately; layout
compacts nonempty prose followed by computed requirements. It measures width
and height before final cursor anchoring and clamps the origin to the viewport.
This shared path covers apprentice, learned and belt grids. Cached hovers are
repositioned without discarding their text. Leaving a skill clears the prose
and restores the original item-control positions, visibility and panel width.
The parent owns teardown; these controls neither capture input nor own items.
No separate close action is added to the transient hover panel.

## Wire, ABI, and resources

No packets, persisted state, item IDs, existing resource IDs or binary
resources change. The active help table remains nine 32-bit colors and nine
128-byte text buffers per record. The new skill file uses the same decimal-ID
plus nine ARGB-prefixed-line format, with at most 48 text characters per row.
It is a required local runtime dependency of this executable and must ship
with the client patch. The patch builder includes changed/added files beneath
`tmproject/client748/`; publishing only the executable is insufficient.

The nine local prose children and saved layout positions are internal C++
presentation state, not native serialized resource controls or wire fields.

## Current mapping

- `internal/ui/ItemHelpText.h`: bounded decoder and viewport anchor helper.
- `internal/app/scenes/NewApp.cpp`: original item help plus required skill overlay.
- `client748/skillhelp.dat`: 102 faithfully localized native descriptions.
- `internal/ui/SGridTooltip.cpp`: separate prose and metadata; current-content layout.
- `internal/ui/SGridInput.cpp`: populate hover before final measurement/positioning.
- `internal/app/scenes/TMFieldSceneUI.cpp`: panel-owned prose children, saved
  original item layout, and existing `UI/descskillm.txt` list binding.

## Delta matrix

| Claim | Classification | Evidence and limit |
| --- | --- | --- |
| Descriptions, formulas and effects for the 102 available native records | PARIDADE_NATIVA | Native runtime asset IDs/content; English localization, not identical bytes; real-client validation pending |
| Original non-skill help begins at row two | PARIDADE_NATIVA | `FUN_00418828`; unchanged non-skill resource identity |
| Apprenticeship explanatory list binding | PARIDADE_NATIVA | `FUN_00435b13` -> `FUN_00442dd0`, control 6128 |
| Bounded decoding and whitespace-tolerant decimal IDs | MODERNIZACAO_COMPATIVEL | Existing file/table format and decoder tests |
| Separate prose controls and content-first bounded tooltip layout | MODERNIZACAO_COMPATIVEL | Local presentation fix; not claimed to reproduce exact native geometry |

## Decisions

Use native content rather than the former short paraphrases. Faithfully
translate it to English under the project-wide language requirement; preserve
its formulas even when a formula's behavior has not been independently
revalidated on the server. Do not invent descriptions for missing native
records. Keep calculated current values and passive captions alongside prose,
and preserve the original twelve resource IDs instead of inventing IDs for
additional rows. Do not change skill gameplay or Backpack behavior here.

## Implemented-effect update (2026-10-29)

Classification: `MODERNIZACAO_COMPATIVEL`. The owner asked tooltips to match
the server's implemented effects, not native prose. 26 TK, BM and HT records
changed: 5003, 5007, 5011, 5013, 5014, 5016, 5064-5071, 5077, 5079, 5081-5083,
5086, 5087, 5090, 5091, 5093-5095. Each states the Go formula (official
TMSRVIN or native client, per `DOCS/current-state.md`). Argumentar (5083)
states that it has no effect. PT rows are `skill_help` overrides in
`tools/client-assets/client_locale_overrides.json`; numbers match EN.

A second pass updated FM records 5025, 5027, 5029-5031 and 5041-5047 after
the Foema server alignment, and 5100 (Cancellation blocks potions). The PT
White Magic name is `Magia Sagrada`.
Other classes' damage records keep native prose.

## Gaps

### Huntress regression correction (2026-10-30)

The 2026-10-29 Huntress update above used 7.59 skill-name mapping for 7.48
tree rules. This correction supersedes those eight affected descriptions:
5075, 5077, 5079, 5082, 5083, 5090, 5091, and 5094.
Classification: `MODERNIZACAO_COMPATIVEL`; descriptions match the corrected
Go behavior. This is not a blanket native combat-parity claim.

Static 7.48 evidence remains in `tmproject/build/native-research/decompiled/`:
`FUN_00557aff.c` selects mastery from the original skill index, uses the eighth
skill of that tree, and identifies Rapid Hit as 95. `FUN_0054fdb9.c` uses
Trade for bow/claw attack, bit 15 for its finisher, and bit 7 for Meditation.
The original PT `references/client748/WYD 7.48 PORTUGUES/itemhelp.dat`
places Life Drain and Hunter's Vision in Capture, with kill healing and drops.
The player suggestion to move Hunter's Vision into Trade is not supported.

English overrides now have a durable `skill_help_en` policy beside the PT
`skill_help` records. All eight descriptions have nine rows, at most 42 text
bytes per row, CP1252 encoding, and preserved resource IDs. Canonical help,
EN, and PT are changed together. The source hash guard was updated only after
reviewing these translations. Locale tests cover both policies and formulas.
Actual rendering and server/client gameplay acceptance remain pending.

### Owner-selected Two Weapon Fighting percentage (2026-10-30)

Classification: `EXTENSAO_COORDENADA`. The owner explicitly replaced the
fixed bow/claw attack bonus with `10 + Trade/20` percent, capped at 26 percent.
This rule supersedes book 5082 in the earlier regression correction. It does
not claim native combat parity.

- USED: preserved PT item help 5082 and native `FUN_0054fdb9`, lines 243..253.
  Both establish the old flat `10 + Trade/5` bonus and the weapon/learned-bit
  boundary. The native finisher branch uses `120 + Trade/5`.
- USED: explicit owner selection of a 10..26 percent attack bonus.
- USED: server `skill_passives.go`, `equipment.go`, `equipment_test.go`,
  `huntress_two_weapon_test.go`, and locale generator/policy tests.
- NOT APPLICABLE: 7.59 balance coefficients. No coefficient from that version
  defines the new owner-selected rule.

The server keeps skill 82, learned bit 10, main-hand Unique 42/43, and Trade
mastery ownership. Clamp effective Trade to 320. Calculate physical attack as
`floor(attack * (2200 + Trade) / 2000)`, then apply the normal score cap.
This retains each mastery point's 0.05 percent contribution without floats.
Apply the passive after equipment/attribute/mount calculation and before
timed affects. Spirit Change and the Capture finisher add no second passive
bonus. The existing ten-point increase to the second weapon's damage share
remains a separate rule.

The passive changes only derived attack. Recalculate from persisted base on
equipment, mastery, learned-skill, scene and login flows. Repetition must not
compound the percentage. Invalid weapon, class or learned bit gives no bonus.
No state is consumed or persisted by this calculation; the existing owner
publishes score snapshots and uses the same attack in physical hits and Rapid
Hit. Existing account persistence, rejection and rollback paths stay intact.

No opcode, ABI, skill identity or persisted value changes. Book 5082 retains
nine CP1252 rows and the existing help overlay. Root/EN/PT `skillhelp.dat` and
the guarded canonical hash change together. Every other help record is
byte-preserved. English remains the fallback. The next publication needs a
mandatory client patch and matching server admission revision under the
release procedure; this implementation does not publish or activate it.

STATICALLY VERIFIED: native flat rule versus the explicit coordinated delta;
unchanged loader, IDs and resource format. AUTOMATED TESTED: percentage
boundaries, eligibility, cap, equipment mastery, repeated recalculation,
runtime reload and Rapid Hit consumption; locale formulas and complete
generation checks. CLIENT-TESTED: pending for the tooltip and gameplay.

The inspected native asset has no prose for 5096, 5103, 5104, or advanced books
5400..5447. Their existing calculated requirements/passive captions remain.
This is complete coverage of the inspected native corpus, not a claim that
every skill implemented by the current game has a native description.
Actual font, clipping, positioning and hover transitions require client testing.
The generated resource compatibility matrix is an older snapshot, not live
loader coverage. Existing Portuguese text in untouched item resources remains
language-migration debt; this local correction is not a global release claim.

## Validation

STATICALLY VERIFIED: native asset identity, matching source/destination sets
of 102 IDs, nine-line records, English-only destination, preserved numeric
formula values, unchanged wire/resource identities and panel ownership.

AUTOMATED TESTED: `Build-Client.ps1 -NoDeploy` passed Release/x86.
ArchitectureTests: 1,228,509 checks; socket: 231; crash capture: 83;
launcher: 56. Added coverage includes whitespace IDs, all 102 descriptions,
Destiny's formula, Backpack's storage prose, isolated production writers,
content-before-layout ordering and viewport anchors at belt/edge positions.
No blocking build errors or warnings appeared in this incremental invocation.

Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`3F38F354E4108FDE1C24BE15E456DF2B77D9536429E16DC13C8CAF2B192A5429`.

Published together as client patch 10030; see the
[launcher receipt](../../../../../DOCS/client/launcher.md#published-patch-10030-2026-10-15).
Fresh HTTP/HTTPS ZIP downloads matched the candidate, required skill asset
and all remaining payload entries. No game server restart, local runtime
replacement, client launch or running-process termination was performed.
Real client validation remains pending: the player must close the client and
run the launcher to load the new executable and skill corpus. Do not promote
this flow to CLIENT_TESTED based on publication, build or static checks.
