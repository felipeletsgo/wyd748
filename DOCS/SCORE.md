# Canonical score — WYD 7.48+

## Contract

`model.Score` is the only authoritative representation of character and mob
attributes. The same logical layout is declared as `STRUCT_SCORE` in the
TMProject client. Its 35 `uint32` fields have a fixed order and occupy
**140 bytes** on the TMProject client wire.

Order: `Version`, `Level`, `Attack`, `MagicAttack`, `Defense`, `MaxHP`, `MaxMP`,
`CurHP`, `CurMP`, `Str`, `Int`, `Dex`, `Con`, `Accuracy`, `Evasion`, `Parry`,
`Critical`, `Range`, four resistances, `SaveMana`, `MagicAmp`, `RegenHP`,
`RegenMP`, `StatusPts`, `MasterPts`, `SkillPts`, four `Mastery` fields,
`AttackRun`, and `Merchant`.

Persisted values remain capped at 2,000,000,000 so intermediate calculations
and older renderer paths stay within the signed range.

## Authority

- `World` mutates `Score`; the client never sends an authoritative score.
- `RuntimeScore` is only a computed copy of the same `Score` type with buffs
  and debuffs; it is neither another format nor a persistent source.
- PostgreSQL persists `Score` directly in account state. There is no migration
  for old JSON containing `score`: accounts predating this contract must be
  recreated.

## Equipment bonuses from WYD KR

The KR equipment catalog shipped in 7.48.108. See
[release status](current-state.md#gameplay-changes-in-748109) for activation
of the recovery formula below.

Native 7.59 recovery runs every eight seconds. Each pulse restores the area
base plus the total equipment index rounded down to a multiple of four.
The area base is 200 normally, 500 on attribute `0x01`, and 1000 on
attribute `0x20`. Apply `0x20` first when both flags are set. Cap recovery
at the missing HP or MP. The Huntress Hide modifier still applies to HP.
See [the recovery pulse](../wydgo748/internal/game/regen.go).

Classification `PARIDADE_NATIVA` for equipment index conversion, area
amounts, and pulse cadence. TMSRVIN `0x4BB4C0` limits the index to 1000 before
division by four, and `0x55EC40` multiplies the stored byte by four.
The WYD-Go runtime score keeps the raw index, and the pulse rounds it.
The persisted base score retains its existing validation.

The WYD KR 7.70+ catalog replaces recovery on 153 pants and 153 gloves with
bonuses to maximum HP and MP. The [server catalog](../wydgo748/data/catalog/itemlist.csv)
and the base, EN, and PT client catalogs copy these replacements.
Item 1347 uses `EF_HPADD 5`, and item 1350 uses `EF_MPADD 6`.
The client asset checksum policy tracks the base catalog.

The catalog audit compared 3,154 item rows with WYD KR 7.70+. It matched the
item identity before copying supported effects. It removed inherent elemental
resistance from 153 boots. Across all 182 boot rows, 115 match the KR mana
recovery bonus, 42 match the maximum MP bonus, and 25 have neither effect.
Mithril Robe Boots 1353 through 1355 use `EF_REGENMP` values 30, 30, and 35.
The KR client and this server multiply the recovery index by refinement.
Mithril Robe Boots with a base index of 30 show 270 at +9. They contribute
268 points to a pulse when equipped alone. A normal area restores 468 MP
with those boots, up to the amount missing.
The MP and HP earrings use `EF_MPADD` and `EF_HPADD` to increase the
respective maximum. Both grant 17% without refinement and 34% at +9.
Changing the maximum with an earring leaves the pulse amount fixed.
The audit also aligned other armor bonuses and 138 weapon rows, including
Ancient variants.

The [mount catalog](../wydgo748/data/content/mounts.json) and the client mount
table mirror rider bonuses from KR `MountData.bin` for 30 adult mount types.
The audit also checked ten timed horses and updated 12 timed KR imports via
their source IDs. The adult mount HP caps and rider damage shares use the
existing 7.48 rules.

Classification `MODERNIZACAO_COMPATIVEL`. The KR tooltip displays attack
speed in tenths of a percent.
The four entries match the local values after unit conversion. The import
keeps the 7.48 weapon requirements and class restrictions.

Angel bonuses use the KR item help. Red Angels grant 16% EXP and 32% drop
rate. Green Angels 3911 through 3913 grant 32% EXP. The Gold Angel grants
18% EXP and 50% drop rate. The base, EN, and PT item help match the server.

## PvP equipment and Ancient weapons

The KR catalog provides PvP defense for 100 Legend armor pieces. Each of the
five armor slots has 20 entries. Base defense ranges from 1.0% to 2.4%.
The import covers matching items in the active catalog. KR-only equipment has
no matching local item identity.

The 7.59 item ability reads the catalog and instance effects for IDs 56 and
57. Apply refinement, then keep the low byte of each item's result. Sum each
effect across equipment. For direct player hits, apply mount sharing and flat
piercing and absorption first. Multiply the rider damage by `(1000 + attack)
/ 1000`, then by `(1000 - defense) / 1000`. Truncate after each step. Bound
defense at 1000 and use the extended score limit before Mana Control.

The client uses the KR yellow tooltip rows with one decimal place. It reserves
space for combat and gem bonuses within the twelve native tooltip rows.
The EN and PT labels use independent resources from the Mystic Land messages.

KR gives all four Cronus Saver Ancient variants effect 76 at 10 and all four
Karicass Ancient variants effect 75 at 30. The KR client identifies the effects
as critical damage and piercing damage. WYD-Go adds effect 76 in percentage
points to the physical critical multiplier. Effect 75 adds flat piercing.
The available 7.59 server does not establish the KR critical formula.

The 7.59 Ancient Emerald and Garnet grades contribute 40 flat damage or
absorption. Special refinement contributes a separate 40 per step from +10
to +15. Diamond and Coral apply their fixed percentage per source. The
tooltip displays the total when grade and special refinement share a gem.

Classification `MODERNIZACAO_COMPATIVEL`. The PvP formula and Ancient gem
calculation follow the 7.59 reference. The tooltip adapts the KR presentation
to 7.48 resources. Validation in the game client is pending.

## TMProject 7.48 client

The TMProject client receives all 140 bytes directly in packets embedding
score. Level, attributes, masteries, HP/MP, attack, and defense are not
truncated, and the additional fields are part of `STRUCT_SCORE`. The active
contract is a coordinated client/server extension; it uses no sidecar and
provides no alternative path for the historical 7.48 executable.

The C++ ABI is guarded by `WYD748Compat.cpp`; the Go encoder has a byte-for-byte
test. Any order or size change requires an atomic change on both sides.
