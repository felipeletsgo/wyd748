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
    path.write_bytes(text.replace("\r\n", "\n").encode(encoding))


def replace_go_func(path: Path, name: str, replacement: str) -> None:
    text = path.read_text(encoding="utf-8")
    marker = f"func {name}("
    start = text.find(marker)
    if start < 0:
        if replacement.strip() in text:
            return
        raise RuntimeError(f"Go function {name} not found in {path}")
    brace = text.find("{", start)
    if brace < 0:
        raise RuntimeError(f"Go function {name} has no body")
    depth = 0
    in_string = False
    escaped = False
    i = brace
    while i < len(text):
        c = text[i]
        if in_string:
            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif c == '"':
                in_string = False
        else:
            if c == '"':
                in_string = True
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    end = i + 1
                    text = text[:start] + replacement.rstrip() + text[end:]
                    path.write_text(text, encoding="utf-8", newline="\n")
                    return
        i += 1
    raise RuntimeError(f"unterminated Go function {name}")


# Every score-bearing incremental packet embeds STRUCT_SCORE directly. Scalar
# copies of Critical/SaveMana/resists/MagicAmp/point counters are deleted.
basedef = project / "Basedef.h"
text, enc = read_source(basedef)
update_score = r'''constexpr auto MSG_UpdateScore_Opcode = 0x336;
struct MSG_UpdateScore
{
	MSG_STANDARD Header;
	STRUCT_SCORE Score;
	unsigned short Affect[32];
	unsigned short Guild;
	unsigned short GuildLevel;
	int ReqHp;
	int ReqMp;
	char LearnedSkill;
};'''
text, count = re.subn(
    r'constexpr auto MSG_UpdateScore_Opcode = 0x336;\nstruct MSG_UpdateScore\n\{.*?\n\};',
    update_score,
    text,
    count=1,
    flags=re.S,
)
if count != 1 and update_score not in text:
    raise RuntimeError("MSG_UpdateScore definition not found")

update_etc = r'''constexpr auto MSG_UpdateEtc_Opcode = 0x337;
struct MSG_UpdateEtc
{
	MSG_STANDARD Header;
	unsigned int Hold;
	unsigned int Exp;
	unsigned int LearnedSkill;
	STRUCT_SCORE Score;
	int Coin;
};'''
text, count = re.subn(
    r'constexpr auto MSG_UpdateEtc_Opcode = 0x337;\nstruct MSG_UpdateEtc\n\{.*?\n\};',
    update_etc,
    text,
    count=1,
    flags=re.S,
)
if count != 1 and update_etc not in text:
    raise RuntimeError("MSG_UpdateEtc definition not found")
write_source(basedef, text, enc)

# Client consumers already copy the canonical Score in UpdateScore. Remove all
# writes sourced from the deleted scalar mirrors. UpdateEtc now copies Score too.
tmhuman = project / "TMHuman.cpp"
text, enc = read_source(tmhuman)
text = re.sub(
    r'^.*pUpdateScore->(?:Critical|SaveMana|Magician|Resist\[[0-3]\]).*\n',
    '',
    text,
    flags=re.M,
)
block = re.compile(
    r'\s*g_pObjectManager->m_stMobData\.LearnedSkill\[0\]\s*=.*?'
    r'g_pObjectManager->m_stMobData\.Coin\s*=\s*pUpdateEtc->Coin;',
    re.S,
)
replacement = r'''
        g_pObjectManager->m_stMobData.LearnedSkill[0] = (int)pUpdateEtc->LearnedSkill;
        g_pObjectManager->m_stMobData.LearnedSkill[1] = 0;
        g_pObjectManager->m_stMobData.Exp = pUpdateEtc->Exp;
        memcpy(&m_stScore, &pUpdateEtc->Score, sizeof(m_stScore));
        memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pUpdateEtc->Score, sizeof(STRUCT_SCORE));
        g_pObjectManager->m_stMobData.Coin = pUpdateEtc->Coin;'''
text, count = block.subn(replacement, text, count=1)
if count != 1 and "pUpdateEtc->Score" not in text:
    raise RuntimeError("TMHuman UpdateEtc compatibility block not found")
write_source(tmhuman, text, enc)

# Go UpdateScore/MobScore mirror the new 232-byte C++ structure exactly.
source = root / "internal/wire/source_client.go"
replace_go_func(source, "UpdateScore", r'''func UpdateScore(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateScore, id, 232)
	score := EncodeClientScore(wireScore(ch))
	copy(b[12:152], score[:])
	putAffectWords(b, 152, ch.Affects[:], time.Now())
	putU16(b, 216, GuildWireID(ch.GuildID))
	putU16(b, 218, uint16(ch.GuildRank))
	// ReqHp/ReqMp are pending skill costs, not current resources. A plain score
	// refresh does not charge either resource.
	putU32(b, 220, 0)
	putU32(b, 224, 0)
	// LearnedSkill here is only the one-byte avatar-effect selector.
	b[228] = 0
	return b
}''')
replace_go_func(source, "MobScore", r'''func MobScore(id uint16, scoreState *model.Score, affects []model.Affect) []byte {
	b := Build(OpUpdateScore, id, 232)
	score := EncodeClientScore(scoreState)
	copy(b[12:152], score[:])
	putAffectWords(b, 152, affects, time.Now())
	return b
}''')

# UpdateEtc is an independent synchronization boundary, so it embeds the same
# canonical Score rather than inventing point-counter fields.
codec = root / "internal/wire/codec.go"
replace_go_func(codec, "UpdateEtc", r'''func UpdateEtc(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateEtc, id, 168)
	putU32(b, 12, 0) // Hold: native reserved EXP field.
	putU32(b, 16, uint32(ch.Exp))
	putU32(b, 20, ch.LearnedSkill)
	score := EncodeClientScore(wireScore(ch))
	copy(b[24:164], score[:])
	putU32(b, 164, ch.Gold)
	return b
}''')

# Mob public Score must include the effective elemental resistance state before
# it reaches wire; no second resistance argument is allowed at the boundary.
affects = root / "internal/game/affects.go"
data = affects.read_text(encoding="utf-8")
old = """\text.Defense = uint32(effectiveMobDefenseAt(m, now))
\text.Attack = uint32(effectiveMobAttackAt(m, now))
\text.AttackRun = uint32(effectiveMobAttackRunAt(m, now))
\treturn ext
"""
new = """\text.Defense = uint32(effectiveMobDefenseAt(m, now))
\text.Attack = uint32(effectiveMobAttackAt(m, now))
\text.AttackRun = uint32(effectiveMobAttackRunAt(m, now))
\tresist := effectiveMobResistancesAt(m, now)
\text.ResistFire = resist.Fire
\text.ResistIce = resist.Ice
\text.ResistHoly = resist.Sacred
\text.ResistThunder = resist.Thunder
\treturn ext
"""
if old in data:
    data = data.replace(old, new, 1)
elif new not in data:
    raise RuntimeError("mobPublicExtendedAt score block not found")
data = data.replace(
    "wire.MobScore(m.ID,\n\t\t\tmobPublicExtendedAt(m, now), m.Affects[:], effectiveMobResistancesAt(m, now))",
    "wire.MobScore(m.ID, mobPublicExtendedAt(m, now), m.Affects[:])",
)
affects.write_text(data, encoding="utf-8", newline="\n")

# ABI guards: UpdateScore is 232 bytes; UpdateEtc is 168 and carries Score@24.
compat = project / "WYD748Compat.cpp"
text, enc = read_source(compat)
text = text.replace(
    'static_assert(sizeof(MSG_UpdateScore) == 244, "source UpdateScore packet ABI changed");',
    'static_assert(sizeof(MSG_UpdateScore) == 232, "canonical UpdateScore packet ABI changed");',
)
text = re.sub(r'^static_assert\(offsetof\(MSG_UpdateScore, (?:ReqHp|ReqMp|Magician|LearnedSkill)\).*?\n', '', text, flags=re.M)
anchor = 'static_assert(sizeof(MSG_UpdateScore) == 232, "canonical UpdateScore packet ABI changed");\n'
asserts = (
    'static_assert(offsetof(MSG_UpdateScore, Affect) == 152, "canonical UpdateScore.Affect offset changed");\n'
    'static_assert(offsetof(MSG_UpdateScore, Guild) == 216, "canonical UpdateScore.Guild offset changed");\n'
    'static_assert(offsetof(MSG_UpdateScore, ReqHp) == 220, "canonical UpdateScore.ReqHp offset changed");\n'
    'static_assert(offsetof(MSG_UpdateScore, ReqMp) == 224, "canonical UpdateScore.ReqMp offset changed");\n'
    'static_assert(offsetof(MSG_UpdateScore, LearnedSkill) == 228, "canonical UpdateScore.LearnedSkill offset changed");\n'
)
if asserts not in text:
    if anchor not in text:
        raise RuntimeError("UpdateScore assert anchor not found")
    text = text.replace(anchor, anchor + asserts, 1)
text = text.replace(
    'static_assert(sizeof(MSG_UpdateEtc) == 48, "WYD-Go UpdateEtc extension ABI changed");',
    'static_assert(sizeof(MSG_UpdateEtc) == 168, "canonical UpdateEtc packet ABI changed");\n'
    'static_assert(offsetof(MSG_UpdateEtc, Score) == 24, "canonical UpdateEtc.Score offset changed");\n'
    'static_assert(offsetof(MSG_UpdateEtc, Coin) == 164, "canonical UpdateEtc.Coin offset changed");',
)
write_source(compat, text, enc)

# Canonical tests follow the packet definitions rather than removed mirrors.
source_test = root / "internal/wire/source_client_test.go"
data = source_test.read_text(encoding="utf-8")
data = data.replace("len(b) != 244", "len(b) != 232")
data = data.replace(
    "binary.LittleEndian.Uint32(b[228:232]), binary.LittleEndian.Uint32(b[232:236])",
    "binary.LittleEndian.Uint32(b[220:224]), binary.LittleEndian.Uint32(b[224:228])",
)
data = data.replace(
    "MobScore(1001, score, nil, model.ElementalResists{Fire: 10, Ice: 20, Sacred: 30, Thunder: 40})",
    "MobScore(1001, score, nil)",
)
source_test.write_text(data, encoding="utf-8", newline="\n")

session_test = root / "internal/wire/session_packets_test.go"
data = session_test.read_text(encoding="utf-8")
# Replace the UpdateScore test with one that proves the scalar mirrors are gone.
start = data.find("func TestUpdateScoreCanonicalLayoutIncludesScoreAndAffects(t *testing.T)")
if start >= 0:
    brace = data.find("{", start)
    depth = 0
    i = brace
    while i < len(data):
        if data[i] == "{": depth += 1
        elif data[i] == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                replacement_test = r'''func TestUpdateScoreCanonicalLayoutIncludesScoreAndAffects(t *testing.T) {
	ch := model.Char{GuildID: 0x1234, GuildRank: model.GuildRankLeader, Score: &model.Score{
		Version: model.ScoreVersion, Level: 65,
		MaxHP: 900, CurHP: 900, MaxMP: 700, CurMP: 700, MagicAmp: 12,
		Critical: 3, SaveMana: 4, RegenHP: 5, RegenMP: 6,
		ResistFire: 7, ResistIce: 8, ResistHoly: 9, ResistThunder: 10,
	}}
	ch.Affects[0] = model.Affect{Type: 24, ExpiresAt: time.Now().Add(80 * time.Second)}
	b := UpdateScore(1, ch)
	if len(b) != 232 || ParseHeader(b).Type != OpUpdateScore || ParseHeader(b).ID != 1 ||
		binary.LittleEndian.Uint32(b[32:36]) != 900 ||
		binary.LittleEndian.Uint32(b[40:44]) != 900 ||
		binary.LittleEndian.Uint32(b[12+64:12+68]) != 3 ||
		binary.LittleEndian.Uint32(b[12+88:12+92]) != 4 ||
		binary.LittleEndian.Uint32(b[12+72:12+76]) != 7 ||
		binary.LittleEndian.Uint16(b[152:154]) != 0x180A ||
		binary.LittleEndian.Uint16(b[216:218]) != 0x0234 ||
		binary.LittleEndian.Uint16(b[218:220]) != model.GuildRankLeader ||
		binary.LittleEndian.Uint32(b[220:224]) != 0 ||
		binary.LittleEndian.Uint32(b[224:228]) != 0 {
		t.Fatalf("canonical UpdateScore incorreto: % X", b)
	}
}'''
                data = data[:start] + replacement_test + data[end:]
                break
        i += 1
session_test.write_text(data, encoding="utf-8", newline="\n")

print("score packet mirrors removed; all score-bearing packets use STRUCT_SCORE")
