---
id: transknight-native-buffs
title: TransKnight Critical Armor, Combat Sensibility, and Berserk score effects
subsystem: combat
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-29
---

# TransKnight native buffs

## Question

What do 7.48 TK skills 9 (Berserk), 14 (Combat Sensibility), and 15
(Critical Armor) change? The official TMSRVIN 7.59 server has no answer:
its skill 9 is Two-Hand Master, and its Critical Armor buff only sets a flag.

## Evidence boundary

- Executable/hash: native `WYD.exe`, hash above.
- Ghidra project/corpus: `tmproject/build/native-research/WYD748.gpr`;
  `decompiled/FUN_0054fdb9.c` (local score builder) and
  `decompiled/FUN_00557aff.c` (skill damage).
- Assets: `references/client748/WYD 7.48 PORTUGUES/itemhelp.dat`, SHA-256
  `79C42A01DE8E62A9839193DBE30AB028EFAEC09FF6DFA4AD0D44AD6391D8EF3F`,
  records 5009, 5014, and 5015. 7.48 `SkillData.csv`: Berserk affect 8,
  Critical Armor affect 50 (not passive), Combat Sensibility passive.
- Current source: `wydgo748/internal/game/affects.go`, `combat.go`,
  `skills.go`, `skill_passives.go`.
- Server: TMSRVIN `sub_4BB4C0` and `sub_4CE1C0` give contradictory 7.59 rules
  (bit 9 Two-Hand Master; Critical Armor learned passive). W2PP
  `_MSG_Attack.cpp:258-268` (secondary) agrees with help 5014.

## Native 7.48 flow

### Observable entry

- Event: a buff affect is active, or a TK with skill 14 lands a physical hit.
- Preconditions and initial state: affect Level holds the caster mastery (Trans).
- Observable output: attack speed, defense, critical, and minimum damage.

### Callers

The client score builder runs on every local score refresh. The server owns
the authoritative score and damage.

### Main function

`FUN_0054fdb9`, affect loop: type 8 adds `Level/10 + 10` to the attack speed
accumulator, stored as tens in the high nibble at `+0x47`. No case exists for
type 50 or 24; the client does not preview Critical Armor or Rescue.
No class-0 learned passive exists in this builder (LearnedSkill at `+0x2DC`).

### Callees

`FUN_0054eb20` (item abilities). No callee reads bit 14.

### Outputs and errors

Help 5009: "Velocidade: 110% + (Trans/10)%". Help 5014: "Dano Mínimo:
+(Trans/20)%", passive and random. Help 5015: temporary; "Bônus em Crítico:
3% +(Trans/50)%", "Bônus em Defesa: 7% +(Trans/50)%".

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| Berserk active | affect 8 | score recalc | attack speed + Level/10 + 10 | none | expiry restores |
| Critical Armor active | affect 31, visual 24 | score recalc | defense x(107 + Level/50)%, critical +(30 + 10*(Level/50)) per mille | none | expiry restores |
| Physical hit | TK, bit 14 | `hitDamageWithRNG` | combat = Special[2]/20, max 15 | none | others use 0 |

### Vtables, vptrs, and receivers

N/A: plain score arithmetic.

### Ownership

The server owns the score; the client preview is advisory.

### Partial failure

N/A: no I/O or mutation outside the recalculated runtime score.

### Cleanup and teardown

Expired affects drop out on the next recalculation.

### Shutdown

N/A.

### Logout and relogin

Persisted Critical Armor stays type 31 with visual 24; legacy type 50 is
normalized on load (`charstate.go`).

## Wire, ABI, and resources

No packet, persisted value, or resource changes. The wire codec already
identifies Critical Armor as type 31 with visual 24.

## Current mapping

### Buildable source

No client change.

### WYD-Go

`affects.go` case 8 and the visual-24 branch of case 31; `combat.go` passes
`skillDamageMastery` to normal hits; `skill_passives.go` drops the 7.59
Critical Armor passive.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Berserk attack speed | `FUN_0054fdb9` case 8, help 5009 | no case 8 | unchanged | +Level/10 + 10 | PARIDADE_NATIVA |
| Critical Armor buff | help 5015, affect 50 not passive | Coin Armor formula, 7.59 passive | unchanged | help formula, no passive | PARIDADE_NATIVA |
| Combat Sensibility | help 5014, W2PP | skills only; hits used Special[0] | unchanged | Special[2]/20 on hits and skills | PARIDADE_NATIVA |

## Decisions

Use native 7.48 for these three skills; the 7.59 server has no equivalent
contract. Without Combat Sensibility, the hit variance floor is 0 for every
character, as in W2PP. The previous Special[0] floor had no native source.

## Gaps

The native server is absent; the help text is the only Critical Armor formula
source. The native client also differs from TMSRVIN for Growth (bit 15 adds
Level, no +200 stats) and Rescue (help 5003: aggro redirect, no defense).

## Validation

- Research: STATICALLY VERIFIED, decompile and native PT help.
- Automation: AUTOMATED TESTED, `transknight_skills_test.go`.
- Real client: CLIENT-TESTED pending.
