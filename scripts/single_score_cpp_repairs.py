from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
project = root / "client-source/tmproject/Projects/TMProject"
common = root / "client-source/tmproject/CommonFiles"


def read_source(path: Path) -> tuple[str, str]:
    """Read legacy TMProject text without silently changing its encoding."""
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        return raw.decode("utf-8-sig"), "utf-8-sig"
    try:
        return raw.decode("utf-8"), "utf-8"
    except UnicodeDecodeError:
        return raw.decode("cp1252"), "cp1252"


def write_source(path: Path, text: str, encoding: str) -> None:
    path.write_bytes(text.replace("\r\n", "\n").encode(encoding))


def remove_struct(text: str, name: str, keep_first: bool = False) -> str:
    pattern = re.compile(rf"\nstruct\s+{re.escape(name)}\s*\{{.*?\n\}};\s*\n", re.S)
    matches = list(pattern.finditer(text))
    if keep_first and len(matches) <= 1:
        return text
    start_at = 1 if keep_first else 0
    for match in reversed(matches[start_at:]):
        text = text[:match.start()] + "\n" + text[match.end():]
    return text


# Basedef contains historical serialized-account layouts from the imported
# source. They are not part of the 7.48+ runtime ABI. Keep only the first live
# declarations and delete every explicitly old layout.
path = project / "Basedef.h"
text, encoding = read_source(path)
for name in ["STRUCT_MOB", "STRUCT_EXT1", "STRUCT_EXT2"]:
    text = remove_struct(text, name, keep_first=True)

old_names = sorted(set(re.findall(r"\bstruct\s+(STRUCT_[A-Za-z0-9_]*OLD[A-Za-z0-9_]*)\b", text)))
for name in old_names:
    text = remove_struct(text, name)

if "STRUCT_SCORE_OLD" in text:
    raise RuntimeError("Basedef.h still references STRUCT_SCORE_OLD after old-layout removal")
write_source(path, text, encoding)

# Canonical field names replace aliases removed from STRUCT_SCORE. Only score
# member expressions are rewritten; generic Damage/Ac members mean other things.
replacements = {
    ".MaxHp": ".MaxHP",
    ".MaxMp": ".MaxMP",
}
score_member_names = {
    "Hp": "CurHP",
    "Mp": "CurMP",
    "Damage": "Attack",
    "Ac": "Defense",
    "Reserved": "Merchant",
    "Special": "Mastery",
}
for path in project.rglob("*"):
    if path.suffix.lower() not in {".cpp", ".h"}:
        continue
    data, encoding = read_source(path)
    original = data
    for old, new in replacements.items():
        data = data.replace(old, new)
    for prefix in ["m_stScore", "BaseScore", "CurrentScore"]:
        for old, new in score_member_names.items():
            data = data.replace(f"{prefix}.{old}", f"{prefix}.{new}")
    for old, new in score_member_names.items():
        data = data.replace(f"pUpdateScore->Score.{old}", f"pUpdateScore->Score.{new}")
        # Character-selection arrays are canonical STRUCT_SCORE values too.
        data = re.sub(
            rf"(\b(?:[A-Za-z_][A-Za-z0-9_]*(?:->|\.))*Score\[[^\]]+\])\.{old}\b",
            rf"\1.{new}",
            data,
        )
    if data != original:
        write_source(path, data, encoding)

# Progression counters now live only in CurrentScore. Remove ObjectManager
# sidecars and redirect every consumer to the canonical fields.
header = project / "ObjectManager.h"
header_text, header_encoding = read_source(header)
updated_header = re.sub(
    r"\n\t// WYD-Go keeps progression counters.*?\n\tunsigned int m_dwSkillBonus;",
    "",
    header_text,
    flags=re.S,
)
if updated_header != header_text:
    write_source(header, updated_header, header_encoding)

for path in project.rglob("*"):
    if path == header or path.suffix.lower() not in {".cpp", ".h"}:
        continue
    data, encoding = read_source(path)
    original = data
    data = data.replace("m_dwScoreBonus", "m_stMobData.CurrentScore.StatusPts")
    data = data.replace("m_dwSpecialBonus", "m_stMobData.CurrentScore.MasterPts")
    data = data.replace("m_dwSkillBonus", "m_stMobData.CurrentScore.SkillPts")
    if data != original:
        write_source(path, data, encoding)

# A single wire ABI no longer needs the SRC2 login marker that selected between
# stock and source score layouts. DBNeedSave returns to an ordinary zero field.
shared = common / "SharedStructs.h"
shared_text, shared_encoding = read_source(shared)
shared_text = re.sub(
    r"\n// The source-built client advertises.*?WYD748_SOURCE_PROTOCOL_MARKER = 0x32435253;[^\n]*\n",
    "\n",
    shared_text,
    flags=re.S,
)
write_source(shared, shared_text, shared_encoding)

login = project / "TMSelectServerScene.cpp"
login_text, login_encoding = read_source(login)
login_text = re.sub(
    r"\n\s*// Advertise the source-client packet family.*?stAccountLogin\.DBNeedSave = WYD748_SOURCE_PROTOCOL_MARKER;",
    "\n\t\tstAccountLogin.DBNeedSave = 0;",
    login_text,
    flags=re.S,
)
write_source(login, login_text, login_encoding)

print("obsolete C++ score structs, sidecars and protocol marker removed")
