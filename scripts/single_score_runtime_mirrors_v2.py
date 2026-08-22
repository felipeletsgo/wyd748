from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
project = root / "client-source/tmproject/Projects/TMProject"


def read_source(path: Path) -> tuple[str, str]:
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        return raw.decode("utf-8-sig").replace("\r\n", "\n"), "utf-8-sig"
    try:
        return raw.decode("utf-8").replace("\r\n", "\n"), "utf-8"
    except UnicodeDecodeError:
        return raw.decode("cp1252").replace("\r\n", "\n"), "cp1252"


def write_source(path: Path, text: str, encoding: str) -> None:
    path.write_bytes(text.encode(encoding))


def rewrite_struct(text: str, name: str, transform) -> str:
    pattern = re.compile(rf"(struct\s+{re.escape(name)}\s*\{{)(.*?)(\n\}};)", re.S)
    match = pattern.search(text)
    if not match:
        raise RuntimeError(f"{name} not found")
    body = transform(match.group(2))
    return text[:match.start()] + match.group(1) + body + match.group(3) + text[match.end():]


basedef = project / "Basedef.h"
text, encoding = read_source(basedef)


def mob_transform(body: str) -> str:
    # Merchant is score-owned. Removing the byte is ABI-neutral here because
    # Guild remains naturally aligned at offset 18 after Clan@16 + pad@17.
    body = re.sub(r"\n\s*char\s+Merchant\s*;", "", body, count=1)
    start = re.search(r"\n\s*unsigned\s+int\s+LearnedSkill\s*\[2\]\s*;", body)
    end = re.search(r"\n\s*unsigned\s+short\s+TotalKill\s*;", body)
    if not start or not end or end.end() <= start.start():
        raise RuntimeError("STRUCT_MOB runtime tail boundaries not found")
    tail = """
\tunsigned int LearnedSkill[2];
\tchar ShortSkill[4];
\tchar GuildLevel;
\tchar dummy[227];
\tunsigned short CurrentKill;
\tunsigned short TotalKill;"""
    body = body[:start.start()] + tail + body[end.end():]
    # Imported source kept a commented copy of the superseded layout. It is
    # documentation noise after the ABI is statically asserted below.
    body = re.sub(r"\n\s*//char MobName\[16\];.*?//unsigned short TotalKill;\s*", "\n", body, flags=re.S)
    return body


text = rewrite_struct(text, "STRUCT_MOB", mob_transform)


def subclass_transform(body: str) -> str:
    body, count = re.subn(
        r"\n\s*short\s+ScoreBonus\s*;\s*\n\s*short\s+SkillBonus\s*;",
        "\n\tchar Reserved[4];",
        body,
        count=1,
    )
    if count == 0 and "Reserved[4]" not in body:
        raise RuntimeError("STRUCT_SUBCLASS score mirrors not found")
    return body


text = rewrite_struct(text, "STRUCT_SUBCLASS", subclass_transform)
write_source(basedef, text, encoding)

# Redirect every known runtime owner to CurrentScore. This does not create
# aliases: these are direct reads/writes of the canonical STRUCT_SCORE object.
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
for path in project.rglob("*"):
    if path.suffix.lower() not in {".cpp", ".h"} or path == basedef:
        continue
    data, enc = read_source(path)
    original = data
    for old, new in scalar_replacements.items():
        data = data.replace(old, new)
    for i, field in enumerate(("ResistFire", "ResistIce", "ResistHoly", "ResistThunder")):
        data = data.replace(f"m_stMobData.Resist[{i}]", f"m_stMobData.CurrentScore.{field}")
        data = data.replace(f"pMobData->Resist[{i}]", f"pMobData->CurrentScore.{field}")
    data = data.replace(
        "memcpy(pMobData->Resist, pUpdateScore->Resist, sizeof(pMobData->Resist));",
        "pMobData->CurrentScore.ResistFire = pUpdateScore->Resist[0];\n"
        "        pMobData->CurrentScore.ResistIce = pUpdateScore->Resist[1];\n"
        "        pMobData->CurrentScore.ResistHoly = pUpdateScore->Resist[2];\n"
        "        pMobData->CurrentScore.ResistThunder = pUpdateScore->Resist[3];",
    )
    if data != original:
        write_source(path, data, enc)

# STRUCT_MOB keeps size 1224 in this step; only its internal score mirrors move.
source = root / "internal/wire/source_client.go"
data = source.read_text(encoding="utf-8")
pattern = re.compile(
    r"\text := wireScore\(ch\)\n"
    r"\tputU16\(b, mob\+988, packetU16\(ext\.StatusPts\)\).*?"
    r"\tb\[mob\+1007\] = clampByte\(int\(ext\.ResistThunder\)\)\n",
    re.S,
)
replacement = "\tcopy(b[mob+988:mob+992], ch.ShortSkill[:4])\n\tb[mob+992] = ch.GuildRank\n"
data, count = pattern.subn(replacement, data, count=1)
if count == 0 and replacement not in data:
    raise RuntimeError("EnterWorld runtime score mirror block not found")
source.write_text(data, encoding="utf-8", newline="\n")

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

print("runtime score mirrors structurally removed")
