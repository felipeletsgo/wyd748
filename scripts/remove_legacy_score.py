from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8-sig")


def write(rel: str, text: str) -> None:
    path = ROOT / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def remove_go_func(text: str, name: str) -> str:
    marker = f"func {name}("
    start = text.find(marker)
    if start < 0:
        return text
    comment_start = start
    while comment_start > 0:
        prev_end = comment_start - 1
        prev_start = text.rfind("\n", 0, prev_end)
        line = text[prev_start + 1:prev_end + 1]
        if line.startswith("//") or line.strip() == "":
            comment_start = prev_start + 1
            if line.strip() == "":
                break
        else:
            break
    brace = text.find("{", start)
    if brace < 0:
        raise RuntimeError(f"function {name}: opening brace not found")
    depth = 0
    i = brace
    in_string = False
    escaped = False
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
                    while end < len(text) and text[end] in "\r\n":
                        end += 1
                    return text[:comment_start] + text[end:]
        i += 1
    raise RuntimeError(f"function {name}: closing brace not found")


def remove_cpp_struct(text: str, name: str) -> str:
    pattern = re.compile(rf"\nstruct\s+{re.escape(name)}\s*\{{.*?\n\}};\s*\n", re.S)
    text2, count = pattern.subn("\n", text, count=1)
    if count == 0:
        return text
    return text2


# ---------------------------------------------------------------------------
# Domain: model.Score is the only score type. No stock/WORD score projection.
# ---------------------------------------------------------------------------
model_rel = "internal/model/model.go"
model = read(model_rel)
start = model.find("// LegacyScore28 =")
end = model.find("const (\n\tScoreVersion", start)
if start < 0 or end < 0:
    raise RuntimeError("model.go: LegacyScore28 block not found")
model = model[:start] + model[end:]

start = model.find("const (\n\tcompatibilityMaximum")
end = model.find("// ElementalResists", start)
if start < 0 or end < 0:
    raise RuntimeError("model.go: compatibility block not found")
model = model[:start] + model[end:]
model = model.replace(
    "// Score e o unico score autoritativo de personagens e mobs e tambem o\n"
    "// contrato binario do client-source 7.48+. Todos os campos sao uint32; o\n"
    "// executavel stock 7.48 recebe somente LegacyScore28 na borda de protocolo.\n"
    "// Nenhuma projecao legada participa de calculo ou persistencia.",
    "// Score e a unica representacao de atributos de personagens e mobs e o\n"
    "// contrato binario compartilhado com o client-source 7.48+. Todos os\n"
    "// campos sao uint32; nao existe score legado, sidecar ou projecao paralela.",
)
model = model.replace('`json:"extendedScore"`', '`json:"score"`')
write(model_rel, model)

# Static content is migrated with the schema. Old files/accounts are not read.
for path in (ROOT / "data").rglob("*.json"):
    text = path.read_text(encoding="utf-8")
    if '"extendedScore"' in text:
        path.write_text(text.replace('"extendedScore"', '"score"'), encoding="utf-8", newline="\n")

# Remove score-era naming from gameplay without introducing aliases.
rename_go = {
    "maxExtendedStat": "maxScoreValue",
    "clampExtended": "clampScoreValue",
    "ensureExtendedScore": "ensureScore",
    "applyExtendedScore": "applyScore",
    "projectExtendedRuntime": "normalizeRuntimeScore",
    "effectiveExtended": "effectiveScore",
    "wireExtendedScore": "wireScoreState",
    "testExtended": "testScore",
}
for path in ROOT.rglob("*.go"):
    text = path.read_text(encoding="utf-8")
    original = text
    for old, new in rename_go.items():
        text = re.sub(rf"\b{re.escape(old)}\b", new, text)
    text = text.replace("extendedScore", "score").replace("ExtendedScore", "Score")
    if text != original:
        path.write_text(text, encoding="utf-8", newline="\n")

old_stats = ROOT / "internal/game/extended_stats.go"
new_stats = ROOT / "internal/game/score.go"
if old_stats.exists():
    if new_stats.exists():
        raise RuntimeError("internal/game/score.go already exists")
    old_stats.rename(new_stats)
old_stats_test = ROOT / "internal/game/extended_stats_test.go"
new_stats_test = ROOT / "internal/game/score_test.go"
if old_stats_test.exists():
    if new_stats_test.exists():
        raise RuntimeError("internal/game/score_test.go already exists")
    old_stats_test.rename(new_stats_test)

# ---------------------------------------------------------------------------
# Wire: delete every stock-score serializer and make the source ABI canonical.
# ---------------------------------------------------------------------------
codec_rel = "internal/wire/codec.go"
codec = read(codec_rel)
for name in [
    "MessageChat",
    "CreateMobTrade", "CreateMobTradeExtended",
    "PutScore",
    "putSelChar", "CharList", "CNFNewCharacter", "CNFDeleteCharacter", "EnterWorld",
    "CreateMob", "CreateMobVisual", "CreateMobExtendedWithGuildRank", "CreateMobVisualExtended",
    "SetMobHpMp", "SetHpMpExtended", "UpdateScore",
    "clientCompatibilityScore", "compatibilityScore", "CompatibilityVitals", "CompatibilityCombatMP",
    "MobScore", "MobScoreExtended", "UpdateAffects", "ShopList",
]:
    codec = remove_go_func(codec, name)
codec = codec.replace("scoreWireExtension", "wireScore")
codec = codec.replace("AttackHitExtended", "AttackHitWide")
codec = codec.replace("AttackHitExtendedResult", "AttackHitWideResult")
write(codec_rel, codec)

source_rel = "internal/wire/source_client.go"
source = read(source_rel)
start = source.find("// ClientProtocol identifies")
end = source.find("// putSourceSelChar", start)
if start < 0 or end < 0:
    raise RuntimeError("source_client.go: protocol compatibility block not found")
source = source[:start] + source[end:]
source = source.replace('\t"encoding/binary"\n', "")
source = source.replace("SourceOpCharList", "OpCharList")
source = source.replace("scoreWireExtension", "wireScore")

# Remove pure dispatch wrappers; the only remaining ABI is the source ABI.
for name in ["CharacterListForProtocol", "EnterWorldForProtocol", "MobScoreForProtocol", "UpdateScoreForProtocol"]:
    source = remove_go_func(source, name)

# Remove compatibility wrappers that contain the source implementation; clean
# replacements are appended below.
for name in [
    "CharacterSelectionUpdateForProtocol", "MobHpMpForProtocol", "HpMpForProtocol",
    "UpdateAffectsForProtocol", "CreateMobVisualExtendedForProtocol",
    "CreateMobExtendedWithGuildRankForProtocol", "CreateMobTradeExtendedForProtocol",
    "ShopListForProtocol", "MessageChatForProtocol",
]:
    source = remove_go_func(source, name)

source = source.replace("putSourceSelChar", "putSelChar")
source = source.replace("SourceCharList", "CharList")
source = source.replace("SourceEnterWorld", "EnterWorld")
source = source.replace("SourceUpdateScore", "UpdateScore")
source = source.replace("SourceMobScore", "MobScore")
source = source.replace("sourceCreateMob", "createMob")
source += r'''

// CharacterSelectionUpdate rebuilds the canonical four-character selection
// aggregate after create/delete/evolution.
func CharacterSelectionUpdate(opcode, id uint16, chars []model.Char) []byte {
	b := Build(opcode, id, 1288)
	putSelChar(b, 16, chars)
	return b
}

// SetHpMp publishes the four authoritative uint32 resources directly. There
// is no WORD prefix and no legacy fallback.
func SetHpMp(id uint16, score *model.Score) []byte {
	if score == nil {
		score = &model.Score{Version: model.ScoreVersion}
	}
	b := Build(OpSetHpMp, id, 28)
	putU32(b, 12, score.CurHP)
	putU32(b, 16, score.CurMP)
	putU32(b, 20, score.MaxHP)
	putU32(b, 24, score.MaxMP)
	return b
}

func MobHpMp(id uint16, currentHP, maxHP, currentMP, maxMP uint32) []byte {
	return SetHpMp(id, &model.Score{
		Version: model.ScoreVersion,
		CurHP: currentHP, MaxHP: maxHP,
		CurMP: currentMP, MaxMP: maxMP,
	})
}

func HpMp(id uint16, score *model.Score) []byte { return SetHpMp(id, score) }

func UpdateAffects(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateAffect, id, 268)
	putSourceAffects(b, 12, ch.Affects[:], time.Now())
	return b
}

func CreateMobVisual(id uint16, name string, x, y uint16, mesh []uint16, anct []byte,
	score *model.Score, affects []model.Affect, spawn uint16) []byte {
	return createMob(id, name, x, y, mesh, anct, score, affects, spawn, 0, 0, nil)
}

func CreateMobWithGuildRank(id uint16, name string, x, y uint16, mesh []uint16, anct []byte,
	score *model.Score, affects []model.Affect, spawn, guild uint16, guildRank byte, cp int16) []byte {
	return createMob(id, name, x, y, mesh, anct, score, affects, spawn, guild, guildRank, &cp)
}

func CreateMobTrade(id uint16, name string, x, y uint16, mesh []uint16, score *model.Score, title string) []byte {
	normal := createMob(id, name, x, y, mesh, nil, score, nil, 2, 0, 0, nil)
	b := Build(OpCreateMobTrade, SceneField, 352)
	copy(b[12:326], normal[12:326])
	copy(b[326:350], title)
	return b
}

func ShopList(items []model.Item, tax, shopType uint32) []byte {
	b := Build(OpShopList, SceneField, 236)
	putU32(b, 12, shopType)
	for i := 0; i < len(items) && i < 27; i++ {
		PutItem(b, 16+i*8, items[i])
	}
	putU32(b, 232, tax)
	return b
}

func MessageChat(id uint16, message string) []byte {
	b := Build(OpMessageChat, id, 140)
	copy(b[12:139], message)
	return b
}
'''
write(source_rel, source)

# Canonical packet names and signatures at all call sites.
wire_renames = {
    "CharacterListForProtocol": "CharList",
    "CharacterSelectionUpdateForProtocol": "CharacterSelectionUpdate",
    "EnterWorldForProtocol": "EnterWorld",
    "MobScoreForProtocol": "MobScore",
    "MobHpMpForProtocol": "MobHpMp",
    "HpMpForProtocol": "HpMp",
    "UpdateScoreForProtocol": "UpdateScore",
    "UpdateAffectsForProtocol": "UpdateAffects",
    "CreateMobVisualExtendedForProtocol": "CreateMobVisual",
    "CreateMobExtendedWithGuildRankForProtocol": "CreateMobWithGuildRank",
    "CreateMobTradeExtendedForProtocol": "CreateMobTrade",
    "ShopListForProtocol": "ShopList",
    "MessageChatForProtocol": "MessageChat",
    "AttackHitExtendedResult": "AttackHitWideResult",
    "AttackHitExtended": "AttackHitWide",
    "SkillHitsWide": "SkillHits",
}
for path in ROOT.rglob("*.go"):
    text = path.read_text(encoding="utf-8")
    original = text
    for old, new in wire_renames.items():
        text = re.sub(rf"\b{re.escape(old)}\b", new, text)
    # All protocol selectors were the first argument of packet builders.
    text = re.sub(r"(?:[A-Za-z_][A-Za-z0-9_]*\.)*[A-Za-z_][A-Za-z0-9_]*\.ClientProtocol\(\),\s*", "", text)
    # Login no longer negotiates an alternate ABI.
    text = re.sub(r"\n\s*s\.SetClientProtocol\(wire\.ClientProtocolFromLogin\(pkt\)\)\s*", "\n", text)
    if text != original:
        path.write_text(text, encoding="utf-8", newline="\n")

# Session has no protocol state: one client ABI exists.
session_rel = "internal/net/session.go"
session = read(session_rel)
session = re.sub(
    r"\n\t// protocol is selected once.*?\n\tprotocol atomic\.Uint32\n",
    "\n",
    session,
    flags=re.S,
)
for name in ["SetClientProtocol", "ClientProtocol"]:
    session = remove_go_func(session, name)
write(session_rel, session)

# Game packet helpers no longer inspect a recipient protocol.
client_proto_rel = "internal/game/client_protocol.go"
cp = read(client_proto_rel)
cp = cp.replace("return wire.UpdateScore(p.ID, *p.Char)", "return wire.UpdateScore(p.ID, *p.Char)")
cp = re.sub(r"wire\.([A-Za-z0-9_]+)\((?:[A-Za-z_][A-Za-z0-9_]*\.)*ClientProtocol\(\),\s*", r"wire.\1(", cp)
cp = cp.replace("s.ClientProtocol(), ", "").replace("observer.Session.ClientProtocol(), ", "")
cp = cp.replace("// mixed stock/source sessions can therefore coexist in the same World.", "// every observer receives the same canonical score ABI.")
write(client_proto_rel, cp)

# Opcode documentation now describes the only supported 7.48+ source ABI.
op_rel = "internal/wire/opcodes.go"
op = read(op_rel)
op = op.replace("OpCharList           = 0x10E // S->C lista de chars (1800B)", "OpCharList           = 0x10A // S->C lista de chars (2360B, Score uint32)")
op = op.replace("OpEnterWorld         = 0x114 // S->C STRUCT_MOB completo (788B)", "OpEnterWorld         = 0x114 // S->C STRUCT_MOB completo (2104B)")
op = op.replace("OpCreateMobTrade     = 0x363 // S->C cria clone de auto-loja com titulo (200B)", "OpCreateMobTrade     = 0x363 // S->C cria clone de auto-loja com titulo (352B)")
op = op.replace("OpCreateMob          = 0x364 // S->C cria mob/player (176B, Status@100)", "OpCreateMob          = 0x364 // S->C cria mob/player (328B, Score@140)")
op = op.replace("OpSetHpMp            = 0x181 // S->C HP/MP do mob (20B)", "OpSetHpMp            = 0x181 // S->C HP/MP uint32 (28B)")
op = op.replace("OpUpdateScore        = 0x336 // S->C score/affects publicos de player ou mob (92B)", "OpUpdateScore        = 0x336 // S->C Score uint32/affects (244B)")
op = op.replace("OpUpdateAffect       = 0x3B9 // S->C 16 affects completos (140B no client 7.48)", "OpUpdateAffect       = 0x3B9 // S->C 32 affects completos (268B)")
op = op.replace("OpShopList           = 0x17C // S->C lista da loja / abre a janela (532B, ShopType@12=1)", "OpShopList           = 0x17C // S->C lista da loja / abre a janela (236B, 27 slots)")
write(op_rel, op)

# ---------------------------------------------------------------------------
# C++ source: only canonical STRUCT_SCORE, no aliases/OLD structs/fallbacks.
# ---------------------------------------------------------------------------
basedef_rel = "client-source/tmproject/Projects/TMProject/Basedef.h"
basedef = read(basedef_rel)
score_start = basedef.find("// Canonical WYD 7.48+ score")
score_end = basedef.find("union STRUCT_BONUSEFFECT", score_start)
if score_start < 0 or score_end < 0:
    raise RuntimeError("Basedef.h: canonical STRUCT_SCORE block not found")
score_block = '''// Canonical WYD 7.48+ score shared byte-for-byte with model.Score.\n// Every field is one unsigned 32-bit word; no legacy aliases are retained.\nstruct STRUCT_SCORE\n{\n\tunsigned int Version;\n\tunsigned int Level;\n\tunsigned int Attack;\n\tunsigned int MagicAttack;\n\tunsigned int Defense;\n\tunsigned int MaxHP;\n\tunsigned int MaxMP;\n\tunsigned int CurHP;\n\tunsigned int CurMP;\n\tunsigned int Str;\n\tunsigned int Int;\n\tunsigned int Dex;\n\tunsigned int Con;\n\tunsigned int Accuracy;\n\tunsigned int Evasion;\n\tunsigned int Parry;\n\tunsigned int Critical;\n\tunsigned int Range;\n\tunsigned int ResistFire;\n\tunsigned int ResistIce;\n\tunsigned int ResistHoly;\n\tunsigned int ResistThunder;\n\tunsigned int SaveMana;\n\tunsigned int MagicAmp;\n\tunsigned int RegenHP;\n\tunsigned int RegenMP;\n\tunsigned int StatusPts;\n\tunsigned int MasterPts;\n\tunsigned int SkillPts;\n\tunsigned int Mastery[4];\n\tunsigned int AttackRun;\n\tunsigned int Merchant;\n};\n\n'''
basedef = basedef[:score_start] + score_block + basedef[score_end:]
# Old persistent/layout structs are not part of the 7.48+ source contract.
basedef = basedef.replace("STRUCT_MOB_OLD", "STRUCT_MOB").replace("STRUCT_EXT1_OLD", "STRUCT_EXT1").replace("STRUCT_EXT2_OLD", "STRUCT_EXT2")
for old_name in ["STRUCT_SCORE_OLD", "STRUCT_SUBCLASS_OLD", "STRUCT_EXT1_OLD", "STRUCT_EXT2_OLD", "STRUCT_MOB_OLD"]:
    basedef = remove_cpp_struct(basedef, old_name)
# 0x181 is rewritten as four uint32 values, with no compatibility prefix.
basedef = re.sub(
    r"struct MSG_SetHpMp\s*\{.*?\n\};",
    "struct MSG_SetHpMp\n{\n\tMSG_STANDARD Header;\n\tunsigned int Hp;\n\tunsigned int Mp;\n\tunsigned int MaxHp;\n\tunsigned int MaxMp;\n};",
    basedef,
    count=1,
    flags=re.S,
)
write(basedef_rel, basedef)

# Remove protocol marker from shared login structs and source login code.
shared_rel = "client-source/tmproject/CommonFiles/SharedStructs.h"
shared = read(shared_rel)
shared = re.sub(r"\n// The source-built client advertises.*?WYD748_SOURCE_PROTOCOL_MARKER.*?\n", "\n", shared, flags=re.S)
write(shared_rel, shared)

select_rel = "client-source/tmproject/Projects/TMProject/TMSelectServerScene.cpp"
select = read(select_rel)
select = re.sub(r"\n\s*// Advertise the source-client packet family.*?stAccountLogin\.DBNeedSave = WYD748_SOURCE_PROTOCOL_MARKER;", "", select, flags=re.S)
write(select_rel, select)

# Simplify the source 0x181 handler to the one canonical packet layout.
human_rel = "client-source/tmproject/Projects/TMProject/TMHuman.cpp"
human = read(human_rel)
old_start = human.find("    // WYD-Go selects 0x181 by observer ABI")
old_end = human.find("    m_stScore.Hp = hp;", old_start)
if old_start < 0 or old_end < 0:
    raise RuntimeError("TMHuman.cpp: legacy 0x181 branch not found")
replacement = (
    "    // WYD-Go 7.48+ carries one uint32 resource layout.\n"
    "    const unsigned int hp = pStd->Hp;\n"
    "    const unsigned int mp = pStd->Mp;\n"
    "    const unsigned int maxHp = pStd->MaxHp;\n"
    "    const unsigned int maxMp = pStd->MaxMp;\n\n"
)
human = human[:old_start] + replacement + human[old_end:]
# Canonical field names replace the old STRUCT_SCORE aliases.
for old, new in [
    ("m_stScore.Hp", "m_stScore.CurHP"), ("m_stScore.Mp", "m_stScore.CurMP"),
    ("m_stScore.MaxHp", "m_stScore.MaxHP"), ("m_stScore.MaxMp", "m_stScore.MaxMP"),
    ("pUpdateScore->Score.Hp", "pUpdateScore->Score.CurHP"),
    ("pUpdateScore->Score.Mp", "pUpdateScore->Score.CurMP"),
    ("pUpdateScore->Score.MaxHp", "pUpdateScore->Score.MaxHP"),
    ("pUpdateScore->Score.MaxMp", "pUpdateScore->Score.MaxMP"),
    ("pUpdateScore->Score.Reserved", "pUpdateScore->Score.Merchant"),
]:
    human = human.replace(old, new)
write(human_rel, human)

compat_rel = "client-source/tmproject/Projects/TMProject/WYD748Compat.cpp"
compat = read(compat_rel)
compat = compat.replace('static_assert(sizeof(MSG_SetHpMp) == 36, "WYD-Go HP/MP incremental ABI changed");', 'static_assert(sizeof(MSG_SetHpMp) == 28, "canonical HP/MP incremental ABI changed");')
compat = compat.replace("// These packets are incremental notifications, not alternative score models.\n", "// Incremental notifications never carry an alternate score representation.\n")
write(compat_rel, compat)

# The old binary patch was solely an ExtendedScore/stock-client compatibility
# path. The source build is now the client and owns the struct directly.
legacy_patch = ROOT / "client748/Patch-WYD748-ExtendedStats.ps1"
if legacy_patch.exists():
    legacy_patch.unlink()
for rel in ["client748/Apply-WYD748.ps1", "client748/PATCHES.md", "README.md", "DOCS/EMULATOR.md", "DOCS/IMPLEMENTED.md", "DOCS/IMPLEMENTATION_STATUS.md", "DOCS/ROADMAP.md", "DOCS/SCORE.md"]:
    path = ROOT / rel
    if not path.exists():
        continue
    text = path.read_text(encoding="utf-8-sig")
    text = text.replace("Patch-WYD748-ExtendedStats.ps1", "")
    text = text.replace("LegacyScore28", "Score")
    text = text.replace("ExtendedScore", "Score").replace("extendedScore", "score")
    text = text.replace("XSC2", "canonical Score")
    path.write_text(text, encoding="utf-8", newline="\n")

# Old convergence scripts describe/restore the removed compatibility model and
# must not remain as executable migration paths.
for rel in ["scripts/score_convergence.py", "scripts/score_convergence_repairs.py"]:
    path = ROOT / rel
    if path.exists():
        path.unlink()

# Tests dedicated to score projection/stock sidecars are obsolete by design.
for rel in ["internal/model/hp_scale_test.go", "internal/wire/hp_scale_wire_test.go"]:
    path = ROOT / rel
    if path.exists():
        path.unlink()

print("legacy score compatibility removed; canonical Score is the only model")
