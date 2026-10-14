---
id: score-hud-shortcut-counters-exp
title: Score refresh projects every HUD and Character field
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-06
---

# Score refresh projects every HUD and Character field

## Question

Which value and format does the native 7.48 score refresh write into each
HUD, Character, Skill, inventory and gamble control, and where did the source
projector differ?

## Evidence boundary

- Executable/hash: `references/client748/WYD.exe`, SHA-256 in front matter;
  format strings read from the PE image at `0x005b7bc4..0x005b7ce0`; message
  indices derived from table base `0x00a34178` (128-byte rows) and checked in
  `client748/UI/strdef.bin`.
- Ghidra project/corpus: `tmproject/build/native-research/WYD748.gpr`;
  headless read-only instruction listings of `FUN_004431e4`, binder
  `FUN_00435b13` and patch thunks `0x013c0c60..0x013c0fa9`; exports
  `FUN_004431e4.c`, `FUN_005225c0.c`, `FUN_004fff0b.c`, `FUN_004fa8a1.c`.
- Assets: `tmproject/client748/UI/FieldScene2.bin` walked with the
  `.agents/research/ui-matrix.py` record sizes.
- Current source: `TMFieldScene::UpdateCompatScoreUI`,
  `internal/ui/NativeScoreHud.h`.
- Server: none; every value comes from the authoritative score snapshot.

## Native 7.48 flow

### Observable entry

- Event: `FUN_004431e4(flags)` (UpdateScoreUI) after score, item, gold or
  shortcut changes.
- Preconditions and initial state: controls bound by `FUN_00435b13`; global
  UI mode `DAT_005b892c == 2` (forced by `FUN_0055bc0a`).
- Observable output: the field table below.

### Callers

Shortcut uses `FUN_0044effc`, `FUN_0044f46b`, `FUN_0044f88f` (flag `0x10`;
see [special-potion-shortcut](special-potion-shortcut.md)) and score/item
packet consumers.

### Main function

Score base `S = ObjectManager + 0x6ec` (STRUCT_MOB). Thunks at `0x013c0xxx`
return a 32-bit sidecar when `[0x013c1f8c] != 0`, else the 7.48 short:
`0x44` attack, `0x42` defense, `0x48/0x4a` max HP/MP, `0x4c/0x4e` HP/MP,
`0x50..0x56` STR/INT/DEX/CON, `0x2e0/0x2e2/0x2e4` status/master/skill points.
The binder stores 1028 twice (`+0x2793c` and `+0x27944`), so the MP write
wins. Flag `0x10` returns after the potion counters.

### Callees

`FUN_0058f078` (sprintf), SText `+0x80` SetText / `+0x84` SetTextColor /
`+0x60` SetVisible, progress `+0x8c` max / `+0x88` current,
`FUN_00557aff` skill damage, `FUN_0054eb20` mob ability, `FUN_00434315`
learned skill, `FUN_004ff388` second class.

### Outputs and errors

Missing controls are skipped by null checks; no packet or authoritative
state change. The function also rewrites `STRUCT_MOB.Class` from the face
when `Equip[0] < 40` (not ported; see Gaps).

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| score refresh | Field bound | `FUN_004431e4` | all fields updated | none | null controls skipped |
| refresh, cancel state | `m_cCancel == 1` | same | HP/MP counters show X | none | none |
| refresh flag 0x10 | shortcut use | same | bars, 1028/1092/1094, counters | none | returns early |

### Vtables, vptrs, and receivers

Receiver is the Field scene; controls come from container virtual `+0x48`.

### Ownership

Controls belong to the resource container; the scene borrows them.

### Partial failure

N/A: presentation pass with per-control null checks.

### Cleanup and teardown

N/A: no allocation or retained state.

### Shutdown

N/A: no state outlives the Field scene.

### Logout and relogin

N/A: the next binding and refresh repopulate every field.

## Wire, ABI, and resources

| ID | FS2 | Native value / format | Source before | Decision |
| --- | --- | --- | --- | --- |
| 1169 / 1158 | progress | HP max/current | same | keep |
| 1170 / 1159 | progress | MP max/current | 1159 never fed | port |
| 1028, 1092, 1094 | text | MP, HP, MP `%d` | same | keep |
| 1026/1025/1028/1027 | text | HP/MaxHP/MP/MaxMP (`FUN_005225c0`) | same | keep |
| 1042 / 1043 | text | Carry HP/MP totals or `X` | never written | port |
| 1031 / 1032 | text | `%u / %u`, 1032 hidden | both written, overlapping | port |
| 1061 / 1062 | text | EXP / next threshold | 1062 = EXP | port |
| 1376, 1168 | text, panel | C.POINT msg 304, red at 80%; hold panel | same | keep |
| 1064, 5741 | text | Level + 1 | same | keep |
| 1090, 1156, 572 | text | status / master / skill points | same | keep |
| 370, 1041 | text | `%10d`, comma flag | `%I64d`, no comma | port |
| 6408 | text | gold minus bet `%10d`, comma | never written | port |
| 1835, 812 | text | cargo gold `%10d` (comma on 1835 only) | never written here | port |
| 1057 | text | `[%s]`, slashed zeros | plain name | port |
| 5752 | text | guild name grey, or empty white | never written | port |
| 1059 / 1066 | text | class / transformed class; `Monster` if face >= 40 | no Monster case | port |
| 1072..1081 | text | STR/INT/DEX/CON | same | keep |
| 1138..1153 | text | `%3d/%-3d` mastery / cap | `%3d/%3d` | port |
| 5776..5779 | text | msg 242 + class*4 | same | keep |
| 5788..5790 | text | msg 107/110/113/133 + n by class | never written ("Skill" stayed baked) | port |
| 1171..1174 | progress, panels | EXP quarter bar and lamps | same | keep |
| 1029, 1096 | text | attack | same | keep |
| 1030, 1104 | text | skill damage 0..103, green heal / blue damage | 1030 only, no color, 0..247 | port |
| 1106 | text | `(int)TMHuman::m_fMaxSpeed` | raw AttackRun nibble | port |
| 1108, 1040 | text | current defense | 1040 = base+abilities | port |
| 1033 | text | base defense + abilities 3, 53 | current defense | port |
| 1110 | text | attack speed `%d%%` | same | keep |
| 1112 | text | critical byte << 1 as `%d.%d%%` | per-mille, cap 100% | extension |
| 1120..1123 | text | resist holy/thunder/fire/ice | same | keep |
| 1377, 266..268 | panel, text | kingdom mantle | mantle only | keep (desc texts absent from scope) |

1044, 1157, 1923..1925, 5730, 5731 are absent from FieldScene2.

## Current mapping

### Buildable source

`UpdateCompatScoreUI` implements every `port` row; the formats are in
`native_score_hud` (`FormatExpRange`, `FormatShortcutCount`,
`FormatCritical`, `FormatMastery`, `FormatCharacterName`).

### WYD-Go

Native 7.48 shows the panel as `(Critical & 0xFF) * 2 / 10` percent (max 51%)
and the item tooltip as `EF_CRITICAL / 10` percent. On 2026-10-14 the user
required one scale with a 100% maximum: an item showing 9.6% must add 9.6%.
`EXTENSAO_COORDENADA` (server and client, wire layout unchanged): Critical is
now kept in tenths of a percent. The server adds item values unchanged, caps
at 1000 and rolls `intn(1000) < Critical`; the client panel prints
`Critical / 10` capped at 100.0%. Skill/buff bonuses authored in 0.2% points
were doubled so their displayed percentages stay the same. Server and client
must be deployed together: an old client would misread the new values.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Field values/formats | table above | table above | 7.59 IDs | score snapshot | port `port` rows |
| 1028 double binding | MP wins | MP written last | N/A | N/A | keep |

## Decisions

`PARIDADE_NATIVA` presentation. EXP and gold keep 64-bit source values and
print with `%lld`, identical inside the 7.48 range. Quick-potion families
follow the native lists even where the Q/W use path accepts 3477/3472.

## Gaps

- The native `Class` rewrite from the face and the skill-grid rebuild
  (`flags & 1`) are outside this projection; the grids belong to
  `UpdateCompatLearnedSkillUI`.
- Native `FUN_004fa8a1` refreshes `m_fMaxSpeed` inside the score refresh; the
  source shows the speed already computed by `TMHuman::SetSpeed`.
- `DAT_005d042c` (no-guild text) is runtime data; an empty string is used.
- Server critical aggregation and roll use the panel scale (see WYD-Go).

## Validation

- Research: listings, exports, PE strings, strdef rows, FieldScene2 walk.
- Automation: `tests/NativeScoreHudTests.cpp`; ArchitectureTests 1,144,424
  checks PASS; SocketReceiveTests 231 PASS.
- Real client: pending on candidate SHA-256
  `5A4CFE72690469263A97E490129D2AD946EE9A9C140DB5895DF7DBAD11290906`.
