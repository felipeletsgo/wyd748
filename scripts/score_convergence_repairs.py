from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]


def replace_in(rel: str, old: str, new: str, *, count: int | None = None) -> None:
    path = root / rel
    text = path.read_text(encoding="utf-8")
    found = text.count(old)
    if count is not None and found != count:
        raise RuntimeError(f"{rel}: expected {count} matches for {old!r}, found {found}")
    if found == 0:
        raise RuntimeError(f"{rel}: no match for {old!r}")
    path.write_text(text.replace(old, new), encoding="utf-8", newline="\n")


# model.Score keeps Merchant and AttackRun as uint32. Only the stock/native
# semantic nibble operations narrow at explicit boundaries.
replace_in(
    "internal/model/model.go",
    "\tdirection := n.Direction & 0xF0\n\tscore.Merchant = direction | (score.Merchant & 0x0F)",
    "\tdirection := uint32(n.Direction & 0xF0)\n\tscore.Merchant = direction | (score.Merchant & 0x0F)",
    count=1,
)

# MakeScore is the canonical name after the model migration. Rename all old
# callers instead of keeping a compatibility wrapper around the removed name.
for path in root.rglob("*.go"):
    text = path.read_text(encoding="utf-8")
    if "MakeExtendedScore" in text:
        path.write_text(text.replace("MakeExtendedScore", "MakeScore"), encoding="utf-8", newline="\n")

# data/npcs is authoritative static content. Its key is an adapter name, not a
# second score representation; preserve the existing dataset while the Go field
# itself is the single canonical Score.
replace_in(
    "internal/model/model.go",
    '\tScore *Score `json:"score"`\n\t// Carry',
    '\tScore *Score `json:"extendedScore"`\n\t// Carry',
    count=1,
)

# Conversion from the captured native NPC binary enters the canonical uint32
# fields explicitly. No byte field survives beyond this loader boundary.
replace_in("cmd/npcconvert/main.go", "\t\t\tMerchant:      merchant,", "\t\t\tMerchant:      uint32(merchant),", count=1)
replace_in("cmd/npcconvert/main.go", "\t\t\tAttackRun:     b[s+7],", "\t\t\tAttackRun:     uint32(b[s+7]),", count=1)

# AttackRun is logically a packed byte, but physically a uint32 in Score. Keep
# nibble math in uint32 and narrow only in helpers whose public return is byte.
replace_in(
    "internal/game/affects.go",
    "\t\tattackRun = attackRun&0xf0 | byte(run)",
    "\t\tattackRun = attackRun&0xf0 | uint32(run)",
    count=1,
)
replace_in(
    "internal/game/affects.go",
    "\treturn attackRun\n}\n\nfunc effectiveMobResistances",
    "\treturn byte(attackRun & 0xff)\n}\n\nfunc effectiveMobResistances",
    count=1,
)
replace_in(
    "internal/game/affects.go",
    "e.AttackRun = e.AttackRun&0xF0 | byte(run)",
    "e.AttackRun = e.AttackRun&0xF0 | uint32(run)",
    count=2,
)
replace_in(
    "internal/game/affects.go",
    "e.AttackRun = byte(attackSpeed<<4) | e.AttackRun&0x0F",
    "e.AttackRun = uint32(attackSpeed<<4) | e.AttackRun&0x0F",
    count=1,
)
replace_in(
    "internal/game/affects.go",
    "e.AttackRun = byte(attackSpeed<<4) | byte(run)",
    "e.AttackRun = uint32(attackSpeed<<4 | run)",
    count=1,
)
replace_in(
    "internal/game/affects.go",
    "ext.AttackRun = effectiveMobAttackRunAt(m, now)",
    "ext.AttackRun = uint32(effectiveMobAttackRunAt(m, now))",
    count=1,
)
replace_in(
    "internal/game/boss_spawn.go",
    "def.Score.AttackRun = byte(*stats.AttackRun)",
    "def.Score.AttackRun = uint32(*stats.AttackRun)",
    count=1,
)
replace_in(
    "internal/game/equipment.go",
    "ch.RuntimeScore.AttackRun = byte(attackSpeed<<4 | runSpeed)",
    "ch.RuntimeScore.AttackRun = uint32(attackSpeed<<4 | runSpeed)",
    count=1,
)
replace_in(
    "internal/game/extended_stats.go",
    "\t\treturn e.AttackRun\n\t}\n\treturn 0\n}\n\nfunc playerCurHP",
    "\t\treturn byte(e.AttackRun & 0xff)\n\t}\n\treturn 0\n}\n\nfunc playerCurHP",
    count=1,
)
replace_in(
    "internal/game/skill_summons.go",
    "attackRun := byte(clampInt(int(t.MoveSpeed), 1, 15))",
    "attackRun := uint32(clampInt(int(t.MoveSpeed), 1, 15))",
    count=1,
)

# Merchant belongs to Score and is therefore uint32. Shop routing consumes the
# authoritative value directly; protocol-specific byte packing happens later.
replace_in(
    "internal/game/handlers.go",
    "func shopTypeForMerchant(merchant byte) (uint32, bool)",
    "func shopTypeForMerchant(merchant uint32) (uint32, bool)",
    count=1,
)

# The source-client tests are part of the ABI contract. Update them atomically
# with the packet definitions instead of weakening/removing the assertions.
test = root / "internal/wire/source_client_test.go"
text = test.read_text(encoding="utf-8")
replacements = {
    "len(selection) != 920": "len(selection) != 1288",
    "len(enter) != 1728": "len(enter) != 2104",
    "enter[1058:1060]": "enter[1242:1244]",
    "enter[1056:1058]": "enter[1240:1242]",
    "16+140+15*8": "16+324+15*8",
    "16+142+15*8": "16+326+15*8",
    "16+284+62*8": "16+468+62*8",
    "16+286+62*8": "16+470+62*8",
    "first affect is exactly @1110": "first affect is exactly @1296",
    "affect := 1110": "affect := 1296",
    "len(create) != 236": "len(create) != 328",
    "len(trade) != 260": "len(trade) != 352",
    "trade[234:239]": "trade[326:331]",
    "len(b) != 152": "len(b) != 244",
    "b[136:140]": "b[228:232]",
    "b[140:144]": "b[232:236]",
    "b[144:146]": "b[236:238]",
    "b[148]": "b[240]",
    "b[130], b[131], b[132], b[133]": "b[222], b[223], b[224], b[225]",
    "len(b) != 1992": "len(b) != 2360",
    "32+872": "32+1240",
    "32+880": "32+1248",
    "936+3*8": "1304+3*8",
    "1960:1964": "2328:2332",
}
for old, new in replacements.items():
    if old not in text:
        raise RuntimeError(f"source_client_test.go: no match for {old!r}")
    text = text.replace(old, new)
test.write_text(text, encoding="utf-8", newline="\n")

# Persistence intentionally switches account characters to the canonical
# `score` key. Old accounts are not migrated; the regression must now reject
# the removed `extendedScore` representation instead of rejecting `score`.
model_test = root / "internal/model/model_test.go"
text = model_test.read_text(encoding="utf-8")
text = text.replace("func TestCharJSONPersistsOnlyExtendedScore", "func TestCharJSONPersistsCanonicalScore", 1)
text = text.replace(
    'bytes.Contains(data, []byte("nextExp")) ||\n\t\tbytes.Contains(data, []byte(`"score":`))',
    'bytes.Contains(data, []byte("nextExp")) ||\n\t\tbytes.Contains(data, []byte(`"extendedScore":`))',
    1,
)
text = text.replace(
    'if !bytes.Contains(data, []byte(`"extendedScore":{"version":2,"level":50,"attack":200`)) {\n\t\tt.Fatalf("extended score autoritativo ausente: %s", data)\n\t}',
    'if !bytes.Contains(data, []byte(`"score":{"version":2,"level":50,"attack":200`)) {\n\t\tt.Fatalf("score canonico ausente: %s", data)\n\t}',
    1,
)
text = text.replace("func TestExtendedCharJSONNeverPersistsLegacyProjection", "func TestWideCharJSONPersistsCanonicalScore", 1)
text = text.replace(
    'if bytes.Contains(data, []byte(`"score":`)) {\n\t\tt.Fatalf("personagem wide ainda gravou score legado: %s", data)\n\t}',
    'if bytes.Contains(data, []byte(`"extendedScore":`)) || !bytes.Contains(data, []byte(`"score":`)) {\n\t\tt.Fatalf("personagem wide nao gravou somente o score canonico: %s", data)\n\t}',
    1,
)
model_test.write_text(text, encoding="utf-8", newline="\n")

print("score convergence uint32 and ABI repairs applied")
