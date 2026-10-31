# Monster drop item options

Classification: `MODERNIZACAO_COMPATIVEL` (2026-10-30).
The port reuses the recovered 7.59 tables within the existing 7.48 item format.
Native 7.48 server parity is not established. The grade-5 table guard below
prevents reading unrelated native memory as a value table.
Code: `wydgo748/internal/game/drop_item_bonus.go`, called from `planMobDrops`
in `drops.go`. Tests: `drop_item_bonus_test.go`.

## Defect

Players reported that every monster drop arrived without additional options.
`planMobDrops` copied `NPCDef.Carry[i]` into the inventory without change.
The native server runs the option generator on each carry drop.
4,236 of 9,324 carry items in `data/npcs/` hold `EF_GRADE0`..`EF_GRADE5`
(effects 100..105) in slot 0. The generator consumes that grade.
The Go server delivered the raw grade marker and no options.

## Native evidence

Primary evidence is the supplied 7.59 `TMSRVIN.exe` decompilation
(`references/decompilacao server/descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c`).
W2PP `Server.cpp` `SetItemBonus` is secondary evidence only.

| Item | Native reference |
| --- | --- |
| Option generator `SetItemBonus(item, level, reduced, dropBonus, armorMagic)` | `FUN_005109B0` (`sub_5109B0`), thunk `0x4B1AA0` (REA auxiliary thunk map) |
| Carry drop call: `SetItemBonus(&carry[jj], mob.CurrentScore.Level, 0, killer.DropBonus, 0)` | MobKilled carry loop, `TMSRVIN.c` near line 75021 |
| Killer `DropBonus` (`dword_7D85344[586*killer]`), also added to `g_pDropBonus[jj]` in the rate | `TMSRVIN.c` near line 74855 |
| Slot-0 option effects `dword_63CC44[0..9]` = 4, 54, 11, 12, 13, 14, 7, 8, 9, 10 | `TMSRVIN.c` data near line 6699 |
| Slot-0 range table: 100 ints at `0x63D120` (`dword_63D120`, `dword_63D124[97]`, `dword_63D2A8`, `dword_63D2AC`) | `TMSRVIN.c` data near line 7013 |
| Gate `dword_61AA138` is the `localserver` console flag; production value 0 | `TMSRVIN.c` near lines 32023 and 42042 |

The drop guard applies the generator to carry indexes in `(0, 6500)`.
The native loop also excludes items 5502..5546 (step 4) from the drop.
No 7.48 mob carries those items, so the Go server does not add that guard.

## Recovered roll

Item fields: `ReqLvl`, `nUnique`, and `nPos` from the item list.
Slots: `Eff[0..1]` is slot 0, `Eff[2..3]` the first option, `Eff[4..5]` the second.

1. `dropBonus >= 1000` encodes a grade: `forced = dropBonus % 1000`, bonus 0.
   `extra = clamp(dropBonus / 8, 0, 2)`.
2. Slot 0 `EF_GRADEn` (100..105) gives grade level `n` and grade value `v`; slot 0 is cleared.
   A forced value above 100 overrides the grade level with `forced - 100`.
3. When not reduced and `level >= 210`, `level -= 47`.
   `lv = (level - ReqLvl) / 25`, replaced by the grade level when present.
   `forced = lv >= 4`; clamp `lv` to `0..4` (to `0..5` for grade level 5).
   Reduced calls cap `lv >= 3` at 2.
4. Options roll only when `nPos & 0xFE != 0`, `nPos != 128`, and slot 0 is empty.
   Accessories (position 0/1), shields (128), and items with a preset slot 0 skip it.
5. First option choice: `r = rand()%101`; `lv 0: r%(8-extra)`, `lv 1,2: r%(6-extra)`,
   `lv 3: r%4`, `lv >= 4: r%2`.
   Second option choice: `r = rand()%100`; `lv 0: %8`, `lv 1,2: %6`, `lv 3: %4`, else `%3`.
6. Option tables (effect, step, grade adjustment); other choices give `EF_UNIQUE` step 0:

   | Position | First option | Second option |
   | --- | --- | --- |
   | 2 helmet | 0: ATTSPEED 3; 1: MAGIC 2 | 0: HP 10; 1: AC 5 (-1) |
   | 4, 8 armor/pants | CRITICAL2 10 (+1); with `armorMagic` 1: AC 5 | 0: MAGIC 2 (-1); 1: DAMAGE 6 (-1); 2: AC 5 (-1) |
   | 16 gloves | ACADD2 5 | 0: MAGIC 2 (-1); 1: DAMAGE 6 (-1); 2: SPECIALALL 3; 3: RESISTALL 3 |
   | 32 boots | DAMAGE2 6 (-1) | 0: MAGIC 2 (-1); 1: SPECIALALL 3 |
   | 64, 192 weapon | 0: ATTSPEED 3 (+1); 1: DAMAGE 9 (-1); 2: SPECIALALL 3 | 0: ATTSPEED 3 (+1); 1: DAMAGE 9 (-1); 2: SPECIALALL 3 (+1) |
   | 64, 192 unique 44/47 | 0: MAGIC 4 (-1); 1: SPECIALALL 3 (+1) | same |

7. First grade from `rand()%100`:
   `lv 0`: <2 → 4, <6 → 3, <24 → 2, <55 → 1, else 0;
   `lv 1`: <4 → 4, <24 → 3, <65 → 2, else 1;
   `lv 2`: <14 → 4, <60 → 3, else 2;
   `lv 3`: <9 → 6, <50 → 5, <90 → 4, else 3;
   `lv 4`: <25 → 6, else 5; `lv 5`: 6.
   `forced` raises it to 4. At `lv >= 3` a weapon DAMAGE/MAGIC option uses adjustment -2.
   Value = `step * (grade + adjustment)` when positive.
   Otherwise boots write the option with value 0; other positions write `EF_UNIQUE rand()%128`.
8. Second grade from a new `rand()%100`:
   `lv 0`: 0 when the first grade roll is below 2, else <6 → 3, <24 → 2, <55 → 1, else 0;
   `lv 1`: <1 → 5, <5 → 4, <24 → 3, <65 → 2, else 1;
   `lv 2`: <2 → 5, <16 → 4, <60 → 3, else 2;
   `lv 3`: <9 → 6, <45 → 5, else 4;
   `lv 4`: <5 → 6, <55 → 5, else 4; `lv 5`: <65 → 5, else 4.
   `forced` raises it to 3; the weapon -2 rule applies; reduced calls cap 5+ at 4.
   A zero total becomes `extra`. Nonpositive totals write `EF_UNIQUE rand()%128`.
   At `lv 0` the native code tests the first grade roll (stack slot `ebp-A4h`).
   This is a debug build, so the slot read is literal; the Go port keeps it.
9. Slot 0, when still empty, from `rand()%100` (halved when reduced).
   Thresholds `(+2, +1, +0, option)` are `(6, 22, 75, 90)` at `lv 0,1` and `(6, 35, 85, 100)` at `lv >= 2`:
   below the first gives `EF_SANC +2`; the next give `+1` and `+0`.
   The option band picks `i = rand()%10`, effect `dword_63CC44[i]`,
   value `min + rand()%(max+1-min)` from the table below. Above 90 gives `EF_UNIQUE rand()%128`.
   A grade value `v > 2` draws `rand()%100` and upgrades `+2`:
   `v 3`: <30 → +3; `v 4`: <10 → +4, <40 → +3; `v 5`: <10 → +5, <30 → +4, <60 → +3;
   `v 6`: <10 → +6, <20 → +5, <40 → +4, <60 → +3;
   `v 7`: <4 → +7, <10 → +6, <20 → +5, <35 → +4, <60 → +3.
10. Item-list static effects then overwrite slots, in list order:
    `EF_SANC`, `EF_AMOUNT`, `EF_WDAY` to slot 0; `EF_INCUBATE` to slot 0 as
    `min(9, value + rand()%4)`; `EF_HOUR` to the first option; `EF_MIN` to the second.
11. Items 447..450, 692..695, and 753 fill each empty slot with `EF_UNIQUE rand()`.

Slot-0 option ranges (`min..max`):

| `lv` | HP | RESISTALL | SPECIAL1 | SPECIAL2 | SPECIAL3 | SPECIAL4 | STR | INT | DEX | CON |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | 10..30 | 2..5 | 3..10 | 2..8 | 2..8 | 2..8 | 2..6 | 2..6 | 2..6 | 2..6 |
| 1 | 20..45 | 4..8 | 7..20 | 5..14 | 5..14 | 5..14 | 4..12 | 4..12 | 4..12 | 4..12 |
| 2, 3, 4 | 30..60 | 5..10 | 10..30 | 8..20 | 8..20 | 8..20 | 6..16 | 6..16 | 6..16 | 6..16 |

## Native option combinations

The two option rolls are independent, so each pair below can occur.
`none` is `EF_UNIQUE` with step 0: no usable option.
Grade A..C is `lv 0..2`, D is `lv 3`, E and E-Arch are `lv 4..5`.
`armorMagic` is 0 for drops, so armor and pants always roll `CRITICAL2` first.

| Position | Grade | First option | Second option |
| --- | --- | --- | --- |
| 2 helmet | A..D | ATTSPEED, MAGIC, none | HP, AC, none |
| 2 helmet | E | ATTSPEED, MAGIC | HP, AC, none |
| 4, 8 armor/pants | A..D | CRITICAL2 | MAGIC, DAMAGE, AC, none |
| 4, 8 armor/pants | E | CRITICAL2 | MAGIC, DAMAGE, AC |
| 16 gloves | A..C | ACADD2 | MAGIC, DAMAGE, SPECIALALL, RESISTALL, none |
| 16 gloves | D | ACADD2 | MAGIC, DAMAGE, SPECIALALL, RESISTALL |
| 16 gloves | E | ACADD2 | MAGIC, DAMAGE, SPECIALALL |
| 32 boots | all | DAMAGE2 | MAGIC, SPECIALALL, none |
| 64, 192 weapon | A..D | ATTSPEED, DAMAGE, SPECIALALL, none | ATTSPEED, DAMAGE, SPECIALALL, none |
| 64, 192 weapon | E | ATTSPEED, DAMAGE | ATTSPEED, DAMAGE, SPECIALALL |
| 64, 192 unique 44/47 | A..D | MAGIC, SPECIALALL, none | MAGIC, SPECIALALL, none |
| 64, 192 unique 44/47 | E | MAGIC, SPECIALALL | MAGIC, SPECIALALL, none |

Without a carry grade, `lv` comes from the mob level and `ReqLvl` (step 3).
The same effect can occur in both weapon options; the native switches allow it.
Slot 0 holds `EF_SANC`, one slot-0 option from the range table, or `EF_UNIQUE`.

## Decisions and gaps

- Owner decision (2026-10-31), `MODERNIZACAO_COMPATIVEL`: add explicit
  `EF_GRADE0` to the 16 unmarked Arch set drops in carry slots 24..27 of
  `PerBerserker`, `PerValkyrie`, `PreKalintz_M`, and `PreKalintz_W`.
  The owner also requested grade 0 for the 16 `EF_GRADE4` set drops in
  `PerAmon`, `PerBear`, `PerEnt`, and `PerWolf` (carry slots 0..3).
  All 32 affected drops now use grade 0 regardless of mob level.
- The Go port follows 7.59 where W2PP differs. W2PP adds 1 in the level
  difference, caps it at 3, and has a different range table.
- Grade E-Arch (`EF_GRADE5`, 228 carry items) gives `lv 5`.
  The native slot-0 option band then reads past the 100-int table into
  unrelated data (`0x63D2B0` onward; some bounds give a negative modulus).
  The Go server writes no slot-0 option in that band, which matches the native
  `localserver` outcome. This is the only deviation; owner review is pending.
- Go `math/rand` replaces MSVC `rand()`. Distributions use the same bounds.
  Byte truncation of `rand()` is modeled as `rand()%256`.
- The killer `DropBonus` maps to `dropBonusFor` (citizenship, gems, Huntress mastery).
  Both servers add it to the drop-rate percentage in the same unit.
- The administrative global-drop event is a WYD-Go feature and is unchanged.
- Existing inventory items are not modified. Only new drops roll options.

## Validation

- `AUTOMATED TESTED`: exact scripted rolls for a weapon, gloves with and
  without drop bonus, the sanc upgrade, non-qualifying items, range sweeps for
  every grade and position, and deterministic `planMobDrops` with an injected RNG.
  `TestDropItemBonusCombinationsMatchNativeTables` enumerates all option choice
  rolls and requires the exact pair set in the combination table.
- `CLIENT-TESTED`: pending. The 7.48 client already shows these effect codes.

## Audit follow-up

Static review checked `sub_5109B0` and its carry-drop call against the port.
The first and second option switches, grade thresholds, level reduction,
drop-bonus adjustment, refinement upgrades and static-effect overrides agree.
The existing grade-5 guard remains the documented compatibility limit.

Regression checks now count every grade-roll outcome and slot-0 threshold.
They also verify the grade-0 dependency between the two rolls. Integration
checks cover committed item publication, save rollback, template immutability
and rolled effects retained by the full-inventory ground fallback.

The population checks now use enabled mixed groups. The previous candidate
was outside installed terrain and cannot represent a live group. Perzen Arc
retains the owner's coordinates `(2480,1705)`; its stale comment is corrected.
