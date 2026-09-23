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

## TMProject 7.48 client

The TMProject client receives all 140 bytes directly in packets embedding
score. Level, attributes, masteries, HP/MP, attack, and defense are not
truncated, and the additional fields are part of `STRUCT_SCORE`. The active
contract is a coordinated client/server extension; it uses no sidecar and
provides no alternative path for the historical 7.48 executable.

The C++ ABI is guarded by `WYD748Compat.cpp`; the Go encoder has a byte-for-byte
test. Any order or size change requires an atomic change on both sides.
