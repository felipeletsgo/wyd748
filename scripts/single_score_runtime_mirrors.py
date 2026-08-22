from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
project = root / "client-source/tmproject/Projects/TMProject"


def read_source(path: Path) -> tuple[str, str]:
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        return raw.decode("utf-8-sig"), "utf-8-sig"
    try:
        return raw.decode("utf-8"), "utf-8"
    except UnicodeDecodeError:
        return raw.decode("cp1252"), "cp1252"


def write_source(path: Path, text: str, encoding: str) -> None:
    path.write_bytes(text.replace("\r\n", "\n").encode(encoding))


# STRUCT_MOB keeps its external size while score-owned fields move exclusively
# into CurrentScore. The enlarged dummy region preserves CurrentKill/TotalKill
# and therefore the surrounding character-login packet offsets.
basedef = project / "Basedef.h"
text, encoding = read_source(basedef)
old_head = "\tchar MobName[16];\n\tchar Clan;\n\tchar Merchant;\n\tunsigned short Guild;"
new_head = "\tchar MobName[16];\n\tchar Clan;\n\tunsigned short Guild;"
text = text.replace(old_head, new_head)

old_tail = """\tunsigned int LearnedSkill[2];
\tshort ScoreBonus;
\tshort SpecialBonus;
\tshort SkillBonus;
\tchar Critical;
\tchar SaveMana;
\tchar ShortSkill[4];
\tchar GuildLevel;
\tchar Magician;
\tchar RegenHP;
\tchar RegenMP;
\tchar Resist[4];
\tchar dummy[212];
\tunsigned short CurrentKill;
\tunsigned short TotalKill;
"""
new_tail = """\tunsigned int LearnedSkill[2];
\tchar ShortSkill[4];
\tchar GuildLevel;
\tchar dummy[227];
\tunsigned short CurrentKill;
\tunsigned short TotalKill;
"""
if old_tail in text:
    text = text.replace(old_tail, new_tail, 1)
elif new_tail not in text:
    raise RuntimeError("STRUCT_MOB score mirror tail not found")

# Delete the stale commented duplicate layout beneath STRUCT_MOB.
text = re.sub(
    r"\n\t//char MobName\[16\];.*?\n\t//unsigned short TotalKill;\n",
    "\n",
    text,
    count=1,
    flags=re.S,
)

old_subclass = """\tunsigned int LearnedSkill[2];
\tSTRUCT_ITEM Equip;
\tSTRUCT_SCORE CurrentScore;
\tlong long Exp;
\tchar ShortSkill[20];
\tshort ScoreBonus;
\tshort SkillBonus;
"""
new_subclass = """\tunsigned int LearnedSkill[2];
\tSTRUCT_ITEM Equip;
\tSTRUCT_SCORE CurrentScore;
\tlong long Exp;
\tchar ShortSkill[20];
\tchar Reserved[4];
"""
if old_subclass in text:
    text = text.replace(old_subclass, new_subclass, 1)
elif new_subclass not in text:
    raise RuntimeError("STRUCT_SUBCLASS score mirror tail not found")
write_source(basedef, text, encoding)

# Redirect live TMProject state reads/writes to the canonical CurrentScore.
scalar_replacements = {
    "m_stMobData.ScoreBonus": "m_stMobData.CurrentScore.StatusPts",
    "m_stMobData.SpecialBonus": "m_stMobData.CurrentScore.MasterPts",
    "m_stMobData.SkillBonus": "m_stMobData.CurrentScore.SkillPts",
    "m_stMobData.Critical": "m_stMobData.CurrentScore.Critical",
    "m_stMobData.SaveMana": "m_stMobData.CurrentScore.SaveMana",
    "m_stMobData.Magician": "m_stMobData.CurrentScore.MagicAmp",
    "m_stMobData.RegenHP": "m_stMobData.CurrentScore.RegenHP",
    "m_stMobData.RegenMP": "m_stMobData.CurrentScore.RegenMP",
    "m_stMobData.Merchant": "m_stMobData.CurrentScore.Merchant",
    "pMobData->ScoreBonus": "pMobData->CurrentScore.StatusPts",
    "pMobData->SpecialBonus": "pMobData->CurrentScore.MasterPts",
    "pMobData->SkillBonus": "pMobData->CurrentScore.SkillPts",
    "pMobData->Critical": "pMobData->CurrentScore.Critical",
    "pMobData->SaveMana": "pMobData->CurrentScore.SaveMana",
    "pMobData->Magician": "pMobData->CurrentScore.MagicAmp",
    "pMobData->RegenHP": "pMobData->CurrentScore.RegenHP",
    "pMobData->RegenMP": "pMobData->CurrentScore.RegenMP",
    "pMobData->Merchant": "pMobData->CurrentScore.Merchant",
}
resist_fields = ["ResistFire", "ResistIce", "ResistHoly", "ResistThunder"]
for path in project.rglob("*"):
    if path.suffix.lower() not in {".cpp", ".h"}:
        continue
    data, enc = read_source(path)
    original = data
    for old, new in scalar_replacements.items():
        data = data.replace(old, new)
    for i, field in enumerate(resist_fields):
        data = data.replace(f"m_stMobData.Resist[{i}]", f"m_stMobData.CurrentScore.{field}")
        data = data.replace(f"pMobData->Resist[{i}]", f"pMobData->CurrentScore.{field}")
    # UpdateScore previously copied the four-byte resistance mirror as an array.
    data = data.replace(
        "memcpy(pMobData->Resist, pUpdateScore->Resist, sizeof(pMobData->Resist));",
        "pMobData->CurrentScore.ResistFire = pUpdateScore->Resist[0];\n"
        "        pMobData->CurrentScore.ResistIce = pUpdateScore->Resist[1];\n"
        "        pMobData->CurrentScore.ResistHoly = pUpdateScore->Resist[2];\n"
        "        pMobData->CurrentScore.ResistThunder = pUpdateScore->Resist[3];",
    )
    if data != original:
        write_source(path, data, enc)

# EnterWorld no longer serializes score mirrors after LearnedSkill. ShortSkill
# and GuildLevel move to their canonical runtime-only positions; the remainder
# of STRUCT_MOB stays zero padding so Slot/ClientID offsets do not move.
source = root / "internal/wire/source_client.go"
data = source.read_text(encoding="utf-8")
old = """\text := wireScore(ch)
\tputU16(b, mob+988, packetU16(ext.StatusPts))
\tputU16(b, mob+990, packetU16(ext.MasterPts))
\tputU16(b, mob+992, packetU16(ext.SkillPts))
\tb[mob+994] = clampByte(int(ext.Critical))
\tb[mob+995] = clampByte(int(ext.SaveMana))
\tcopy(b[mob+996:mob+1000], ch.ShortSkill[:4])
\tb[mob+1000] = ch.GuildRank
\tb[mob+1001] = clampByte(int(ext.MagicAmp))
\tb[mob+1002] = clampByte(int(ext.RegenHP))
\tb[mob+1003] = clampByte(int(ext.RegenMP))
\tb[mob+1004] = clampByte(int(ext.ResistFire))
\tb[mob+1005] = clampByte(int(ext.ResistIce))
\tb[mob+1006] = clampByte(int(ext.ResistHoly))
\tb[mob+1007] = clampByte(int(ext.ResistThunder))
"""
new = """\tcopy(b[mob+988:mob+992], ch.ShortSkill[:4])
\tb[mob+992] = ch.GuildRank
"""
if old in data:
    data = data.replace(old, new, 1)
elif new not in data:
    raise RuntimeError("EnterWorld score mirror block not found")
source.write_text(data, encoding="utf-8", newline="\n")

# Compile-time ABI guards document that only internal score mirrors moved.
compat = project / "WYD748Compat.cpp"
data, enc = read_source(compat)
data = re.sub(r'^static_assert\(offsetof\(STRUCT_MOB, (?:ScoreBonus|SpecialBonus|SkillBonus)\).*?\n', '', data, flags=re.M)
anchor = 'static_assert(sizeof(STRUCT_MOB) == 1224, "source STRUCT_MOB ABI changed");\n'
asserts = (
    'static_assert(offsetof(STRUCT_MOB, ShortSkill) == 988, "source ShortSkill offset changed");\n'
    'static_assert(offsetof(STRUCT_MOB, GuildLevel) == 992, "source GuildLevel offset changed");\n'
    'static_assert(offsetof(STRUCT_MOB, CurrentKill) == 1220, "source CurrentKill offset changed");\n'
)
if asserts not in data:
    if anchor not in data:
        raise RuntimeError("STRUCT_MOB size assert anchor not found")
    data = data.replace(anchor, anchor + asserts, 1)
write_source(compat, data, enc)

print("runtime score mirrors removed from STRUCT_MOB and STRUCT_SUBCLASS")
