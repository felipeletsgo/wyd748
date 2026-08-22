from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
project = root / "client-source/tmproject/Projects/TMProject"


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
for path in project.rglob("*"):
    if path.suffix.lower() not in {".cpp", ".h"}:
        continue
    data, encoding = read_source(path)
    original = data
    for old, new in replacements.items():
        data = data.replace(old, new)
    for prefix in ["m_stScore", "BaseScore", "CurrentScore"]:
        data = data.replace(f"{prefix}.Hp", f"{prefix}.CurHP")
        data = data.replace(f"{prefix}.Mp", f"{prefix}.CurMP")
        data = data.replace(f"{prefix}.Damage", f"{prefix}.Attack")
        data = data.replace(f"{prefix}.Ac", f"{prefix}.Defense")
        data = data.replace(f"{prefix}.Reserved", f"{prefix}.Merchant")
        data = data.replace(f"{prefix}.Special", f"{prefix}.Mastery")
    data = data.replace("pUpdateScore->Score.Hp", "pUpdateScore->Score.CurHP")
    data = data.replace("pUpdateScore->Score.Mp", "pUpdateScore->Score.CurMP")
    data = data.replace("pUpdateScore->Score.Damage", "pUpdateScore->Score.Attack")
    data = data.replace("pUpdateScore->Score.Ac", "pUpdateScore->Score.Defense")
    data = data.replace("pUpdateScore->Score.Reserved", "pUpdateScore->Score.Merchant")
    data = data.replace("pUpdateScore->Score.Special", "pUpdateScore->Score.Mastery")
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

print("obsolete C++ score structs and sidecars removed")
