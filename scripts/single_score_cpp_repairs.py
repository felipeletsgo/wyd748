from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
project = root / "client-source/tmproject/Projects/TMProject"


def remove_struct(text: str, name: str, keep_first: bool = False) -> str:
    pattern = re.compile(rf"\nstruct\s+{re.escape(name)}\s*\{{.*?\n\}};\s*\n", re.S)
    matches = list(pattern.finditer(text))
    if keep_first and len(matches) <= 1:
        return text
    start_at = 1 if keep_first else 0
    for match in reversed(matches[start_at:]):
        text = text[:match.start()] + "\n" + text[match.end():]
    return text


# Basedef contains several historical serialized-account layouts from the
# imported source. They are not part of the 7.48+ runtime ABI and, after the
# canonical Score migration, some even redeclare live struct names.
path = project / "Basedef.h"
text = path.read_text(encoding="utf-8-sig")
for name in ["STRUCT_MOB", "STRUCT_EXT1", "STRUCT_EXT2"]:
    text = remove_struct(text, name, keep_first=True)

# Remove every explicitly old struct. They must not remain as an accidental
# alternate serialization path.
old_names = sorted(set(re.findall(r"\bstruct\s+(STRUCT_[A-Za-z0-9_]*OLD[A-Za-z0-9_]*)\b", text)))
for name in old_names:
    text = remove_struct(text, name)

# No legacy score symbol may survive in Basedef.
if "STRUCT_SCORE_OLD" in text:
    raise RuntimeError("Basedef.h still references STRUCT_SCORE_OLD after old-layout removal")
path.write_text(text, encoding="utf-8", newline="\n")

# Canonical field names replace aliases removed from STRUCT_SCORE.
replacements = {
    ".MaxHp": ".MaxHP",
    ".MaxMp": ".MaxMP",
}
for path in project.rglob("*"):
    if path.suffix.lower() not in {".cpp", ".h"}:
        continue
    data = path.read_text(encoding="utf-8-sig")
    original = data
    for old, new in replacements.items():
        data = data.replace(old, new)
    # These names are only safe to rewrite when attached to an explicit score
    # object/member; generic .Damage/.Ac names elsewhere have other meanings.
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
        path.write_text(data, encoding="utf-8", newline="\n")

# The three ObjectManager sidecars duplicate fields already owned by CurrentScore.
# Redirect every consumer to CurrentScore and remove the duplicate storage.
header = project / "ObjectManager.h"
header_text = header.read_text(encoding="utf-8-sig")
header_text = re.sub(
    r"\n\t// WYD-Go keeps progression counters.*?\n\tunsigned int m_dwSkillBonus;",
    "",
    header_text,
    flags=re.S,
)
header.write_text(header_text, encoding="utf-8", newline="\n")

for path in project.rglob("*"):
    if path == header or path.suffix.lower() not in {".cpp", ".h"}:
        continue
    data = path.read_text(encoding="utf-8-sig")
    data = data.replace("m_dwScoreBonus", "m_stMobData.CurrentScore.StatusPts")
    data = data.replace("m_dwSpecialBonus", "m_stMobData.CurrentScore.MasterPts")
    data = data.replace("m_dwSkillBonus", "m_stMobData.CurrentScore.SkillPts")
    path.write_text(data, encoding="utf-8", newline="\n")

print("obsolete C++ score structs and sidecars removed")
