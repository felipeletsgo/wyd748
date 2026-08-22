from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def write(rel: str, text: str) -> None:
    path = ROOT / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def regex_once(text: str, pattern: str, replacement: str, label: str, flags: int = 0) -> str:
    result, count = re.subn(pattern, replacement, text, count=1, flags=flags)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one regex match, found {count}")
    return result


# ---------------------------------------------------------------------------
# Go domain: one canonical Score. LegacyScore28 remains only as a stock-7.48
# wire adapter. Old account JSON is intentionally incompatible and will be
# recreated, per project policy for this migration.
# ---------------------------------------------------------------------------
for path in ROOT.rglob("*.go"):
    if ".git" in path.parts:
        continue
    text = path.read_text(encoding="utf-8")
    original = text
    text = re.sub(r"\bExtendedRuntime\b", "RuntimeScore", text)
    text = re.sub(r"\bExtendedScoreVersion\b", "ScoreVersion", text)
    text = re.sub(r"\bMaxExtendedScoreValue\b", "MaxScoreValue", text)
    text = re.sub(r"\bExtendedScore\b", "Score", text)
    text = re.sub(r"\bWireScore\b", "LegacyScore28", text)
    # `Extended` was exclusively the old score field name. Longer identifiers
    # such as CreateMobExtended are intentionally left untouched by word-boundary
    # replacement and continue naming legacy stock packet helpers.
    text = re.sub(r"\bExtended\b", "Score", text)
    if text != original:
        path.write_text(text, encoding="utf-8", newline="\n")

model_path = "internal/model/model.go"
model = read(model_path)
model = model.replace("// LegacyScore28 = STRUCT_SCORE 7.48 (28 bytes no wire).", "// LegacyScore28 = STRUCT_SCORE stock 7.48 (28 bytes no wire).")
model = model.replace(
    "// Score e o score autoritativo de 32 bits usado por personagens e mobs.\n"
    "// LegacyScore28 continua sendo apenas a projecao de ABI de 28 bytes; nenhuma rotina\n"
    "// server-side de combate deve usa-lo como fonte de verdade.",
    "// Score e o unico score autoritativo de personagens e mobs e tambem o\n"
    "// contrato binario do client-source 7.48+. Todos os campos sao uint32; o\n"
    "// executavel stock 7.48 recebe somente LegacyScore28 na borda de protocolo.\n"
    "// Nenhuma projecao legada participa de calculo ou persistencia."
)
model = model.replace("\tAttackRun     byte      `json:\"attackRun,omitempty\"`\n\tMerchant      byte      `json:\"merchant,omitempty\"`", "\tAttackRun     uint32    `json:\"attackRun,omitempty\"`\n\tMerchant      uint32    `json:\"merchant,omitempty\"`")
model = model.replace('Score                 *Score `json:"extendedScore"`', 'Score                 *Score `json:"score"`')
model = model.replace('Score *Score `json:"extendedScore"`', 'Score *Score `json:"score"`')
model = model.replace("// Jogadores nao armazenam STRUCT_SCORE. Ele e projetado na borda.\n\tScore *Score", "// Score e persistido diretamente e compartilhado com o client-source.\n\tScore *Score")
model = model.replace("// RuntimeScore contem o score efetivo depois de buffs/debuffs. Score\n\t// permanece a base persistida;", "// RuntimeScore contem a copia efetiva depois de buffs/debuffs. Score\n\t// permanece a base persistida; essa copia nao define outro formato de score;")
model = model.replace("Merchant:  e.Merchant,", "Merchant:  byte(minScoreValue(e.Merchant, 255)),")
model = model.replace("AttackRun: e.AttackRun,", "AttackRun: byte(minScoreValue(e.AttackRun, 255)),")
model = model.replace("extendedScore", "score")
model = model.replace("ExtendedScore", "Score")
write(model_path, model)

# ---------------------------------------------------------------------------
# Canonical source-client score serializer: 35 uint32 fields = 140 bytes.
# No truncation and no XSC2 sidecar is needed by the source-client.
# ---------------------------------------------------------------------------
score48 = ROOT / "internal/wire/score48.go"
score_go = ROOT / "internal/wire/score.go"
if not score48.exists():
    raise RuntimeError("internal/wire/score48.go missing before migration")
score_go.write_text('''package wire

import (
\t"encoding/binary"

\t"wydgo/internal/model"
)

// ClientScoreSize is the exact 7.48+ source-client STRUCT_SCORE size.
// The layout is intentionally simple: 35 little-endian uint32 words shared
// with model.Score and the C++ STRUCT_SCORE declaration.
const ClientScoreSize = 35 * 4

// EncodeClientScore serializes the canonical authoritative Score without
// narrowing. There is intentionally no decoder: client-sent score state is
// never authoritative.
func EncodeClientScore(score *model.Score) [ClientScoreSize]byte {
\tvar out [ClientScoreSize]byte
\tif score == nil {
\t\treturn out
\t}
\n\tvalues := [...]uint32{
\t\tscore.Version,
\t\tscore.Level,
\t\tscore.Attack,
\t\tscore.MagicAttack,
\t\tscore.Defense,
\t\tscore.MaxHP,
\t\tscore.MaxMP,
\t\tscore.CurHP,
\t\tscore.CurMP,
\t\tscore.Str,
\t\tscore.Int,
\t\tscore.Dex,
\t\tscore.Con,
\t\tscore.Accuracy,
\t\tscore.Evasion,
\t\tscore.Parry,
\t\tscore.Critical,
\t\tscore.Range,
\t\tscore.ResistFire,
\t\tscore.ResistIce,
\t\tscore.ResistHoly,
\t\tscore.ResistThunder,
\t\tscore.SaveMana,
\t\tscore.MagicAmp,
\t\tscore.RegenHP,
\t\tscore.RegenMP,
\t\tscore.StatusPts,
\t\tscore.MasterPts,
\t\tscore.SkillPts,
\t\tscore.Mastery[0],
\t\tscore.Mastery[1],
\t\tscore.Mastery[2],
\t\tscore.Mastery[3],
\t\tscore.AttackRun,
\t\tscore.Merchant,
\t}
\tfor i, value := range values {
\t\tbinary.LittleEndian.PutUint32(out[i*4:(i+1)*4], value)
\t}
\treturn out
}
''', encoding="utf-8", newline="\n")
score48.unlink()

# ---------------------------------------------------------------------------
# Source-client packet family. Every packet embedding STRUCT_SCORE is resized
# to the canonical 140-byte layout. Stock serializers remain separate.
# ---------------------------------------------------------------------------
source_path = "internal/wire/source_client.go"
source = read(source_path)
source = source.replace("EncodeClientScore48", "EncodeClientScore")
source = source.replace("ClientScore48Size", "ClientScoreSize")
source = source.replace("904-byte STRUCT_SELCHAR", "1272-byte STRUCT_SELCHAR")
source = source.replace("Its 48-byte\n// score and uint64 EXP are source-client presentation fields; Score\n// and the account model remain the only server-authoritative state.", "Its canonical 140-byte\n// Score is copied without narrowing; uint64 EXP remains a presentation field.")
source = source.replace("offset+272+(slot*18+equipSlot)*8", "offset+640+(slot*18+equipSlot)*8")
source = source.replace("offset+848+slot*2", "offset+1216+slot*2")
source = source.replace("offset+856+slot*4", "offset+1224+slot*4")
source = source.replace("offset+872+slot*8", "offset+1240+slot*8")
source = source.replace("packetSize = 1992", "packetSize = 2360")
source = source.replace("cargoOff   = 936", "cargoOff   = 1304")
source = source.replace("putU32(b, 1960, cargoGold)", "putU32(b, 2328, cargoGold)")
source = source.replace("copy(b[1964:1980], accName)", "copy(b[2332:2348], accName)")
source = source.replace("exact 1992-byte MSG_CNFAccountLogin", "exact 2360-byte MSG_CNFAccountLogin")
source = source.replace("904-byte SelChar", "1272-byte SelChar")
source = source.replace("b := Build(opcode, id, 920)", "b := Build(opcode, id, 1288)")
source = source.replace("serializes the 1728-byte MSG_CNFCharacterLogin", "serializes the 2104-byte MSG_CNFCharacterLogin")
source = source.replace("b := Build(OpEnterWorld, id, 1728)", "b := Build(OpEnterWorld, id, 2104)")
source = regex_once(
    source,
    r"const \(\n\t\tsourceShortSkillOffset = 1062\n\t\tsourceExt1Offset       = sourceShortSkillOffset \+ 16\n\t\tsourceExt1AffectOffset = sourceExt1Offset \+ 8\*4\n\t\)",
    "const (\n\t\tsourceShortSkillOffset = 1246\n\t\t// MSVC aligns STRUCT_EXT1 to four bytes after ShortSkill[16].\n\t\t// The old serializer incorrectly started it two bytes early.\n\t\tsourceExt1Offset       = 1264\n\t\tsourceExt1AffectOffset = sourceExt1Offset + 8*4\n\t)",
    "source enter-world offsets",
)
source = source.replace("copy(b[mob+44:mob+92], baseScore[:])\n\tcopy(b[mob+92:mob+140], runtimeScore[:])", "copy(b[mob+44:mob+184], baseScore[:])\n\tcopy(b[mob+184:mob+324], runtimeScore[:])")
source = source.replace("PutItem(b, mob+140+i*8, item)", "PutItem(b, mob+324+i*8, item)")
source = source.replace("PutItem(b, mob+284+i*8, item)", "PutItem(b, mob+468+i*8, item)")
source = source.replace("putU32(b, mob+796, ch.LearnedSkill)", "putU32(b, mob+980, ch.LearnedSkill)")
source = source.replace("putU32(b, mob+800, ch.SecondaryLearnedSkill)", "putU32(b, mob+984, ch.SecondaryLearnedSkill)")
source = source.replace("mob+804", "mob+988")
source = source.replace("mob+806", "mob+990")
source = source.replace("mob+808", "mob+992")
source = source.replace("mob+810", "mob+994")
source = source.replace("mob+811", "mob+995")
source = source.replace("mob+812:mob+816", "mob+996:mob+1000")
source = source.replace("mob+816", "mob+1000")
source = source.replace("mob+817", "mob+1001")
source = source.replace("mob+818", "mob+1002")
source = source.replace("mob+819", "mob+1003")
source = source.replace("mob+820", "mob+1004")
source = source.replace("mob+821", "mob+1005")
source = source.replace("mob+822", "mob+1006")
source = source.replace("mob+823", "mob+1007")
source = source.replace("putU16(b, 1056, slot)", "putU16(b, 1240, slot)")
source = source.replace("putU16(b, 1058, id)", "putU16(b, 1242, id)")
source = source.replace("first affect is Ext1.Affect[0] at 1110 (not 1112)", "first affect is Ext1.Affect[0] at 1296")
source = source.replace("Ext1.Data is\n\t// eight DWORDs and is part of the native 0x114 packet, not optional padding.", "Ext1.Data is eight DWORDs; the two bytes before Ext1 are MSVC alignment padding.")
source = source.replace("152-byte 0x336 structure", "244-byte 0x336 structure")
source = source.replace("b := Build(OpUpdateScore, id, 152)", "b := Build(OpUpdateScore, id, 244)")
source = source.replace("copy(b[12:60], score[:])", "copy(b[12:152], score[:])")
source = source.replace("b[60] = clampByte(int(ext.Critical))", "b[152] = clampByte(int(ext.Critical))")
source = source.replace("b[61] = clampByte(int(ext.SaveMana))", "b[153] = clampByte(int(ext.SaveMana))")
source = source.replace("putAffectWords(b, 62, ch.Affects[:], time.Now())", "putAffectWords(b, 154, ch.Affects[:], time.Now())")
source = source.replace("putU16(b, 126, GuildWireID(ch.GuildID))", "putU16(b, 218, GuildWireID(ch.GuildID))")
source = source.replace("putU16(b, 128, uint16(ch.GuildRank))", "putU16(b, 220, uint16(ch.GuildRank))")
source = source.replace("b[130] = clampByte(int(ext.ResistFire))", "b[222] = clampByte(int(ext.ResistFire))")
source = source.replace("b[131] = clampByte(int(ext.ResistIce))", "b[223] = clampByte(int(ext.ResistIce))")
source = source.replace("b[132] = clampByte(int(ext.ResistHoly))", "b[224] = clampByte(int(ext.ResistHoly))")
source = source.replace("b[133] = clampByte(int(ext.ResistThunder))", "b[225] = clampByte(int(ext.ResistThunder))")
source = source.replace("putU16(b, 144, compatibilityU16(ext.MagicAmp))", "putU16(b, 236, compatibilityU16(ext.MagicAmp))")
source = source.replace("b[148] = 0", "b[240] = 0")
# SourceMobScore has the same packet layout but a different affect/resist source.
source = source.replace("putAffectWords(b, 62, affects, time.Now())", "putAffectWords(b, 154, affects, time.Now())")
source = source.replace("b[130] = clampByte(int(resist.Fire))", "b[222] = clampByte(int(resist.Fire))")
source = source.replace("b[131] = clampByte(int(resist.Ice))", "b[223] = clampByte(int(resist.Ice))")
source = source.replace("b[132] = clampByte(int(resist.Sacred))", "b[224] = clampByte(int(resist.Sacred))")
source = source.replace("b[133] = clampByte(int(resist.Thunder))", "b[225] = clampByte(int(resist.Thunder))")
source = source.replace("source client's 152-byte score refresh", "source client's 244-byte score refresh")
source = source.replace("reads a 48-byte STRUCT_SCORE directly at byte 12", "reads the canonical STRUCT_SCORE directly at byte 12")
source = source.replace("serializes TMProject's 236-byte MSG_CreateMob", "serializes TMProject's 328-byte MSG_CreateMob")
source = source.replace("48-byte presentation score", "canonical 140-byte Score")
source = source.replace("b := Build(OpCreateMob, SceneField, 236)", "b := Build(OpCreateMob, SceneField, 328)")
source = source.replace("copy(b[140:188], score[:])", "copy(b[140:280], score[:])")
source = source.replace("putU16(b, 188, spawn)", "putU16(b, 280, spawn)")
source = source.replace("copy(b[190:208], anct)", "copy(b[282:300], anct)")
source = source.replace("source client's 260-byte trade", "source client's 352-byte trade")
source = source.replace("b := Build(OpCreateMobTrade, SceneField, 260)", "b := Build(OpCreateMobTrade, SceneField, 352)")
source = source.replace("copy(b[12:234], normal[12:234])", "copy(b[12:326], normal[12:326])")
source = source.replace("copy(b[234:258], title)", "copy(b[326:350], title)")
# New canonical type names after the domain rename.
source = source.replace("*model.ExtendedScore", "*model.Score")
source = source.replace("model.ExtendedScore{", "model.Score{")
source = source.replace("model.ExtendedScoreVersion", "model.ScoreVersion")
write(source_path, source)

# ---------------------------------------------------------------------------
# C++ source client: the exact same 35-word logical Score as Go. Aliases keep
# existing TMProject member spellings readable while removing the narrow ABI.
# ---------------------------------------------------------------------------
basedef_path = "client-source/tmproject/Projects/TMProject/Basedef.h"
basedef = read(basedef_path)
old_score = '''struct STRUCT_SCORE
{
\tshort Level;
\tint Ac;
\tint Damage;
\tchar Reserved;
\tchar AttackRun;
\tint MaxHp;
\tint MaxMp;
\tint Hp;
\tint Mp;
\tshort Str;
\tshort Int;
\tshort Dex;
\tshort Con;
\tunsigned short Special[4];
};'''
new_score = '''// Canonical WYD 7.48+ score shared byte-for-byte with model.Score.
// Every member occupies one uint32 word. The anonymous aliases preserve the
// imported TMProject names without creating a second representation.
struct STRUCT_SCORE
{
\tunsigned int Version;
\tunsigned int Level;
\tunion { unsigned int Attack; unsigned int Damage; };
\tunsigned int MagicAttack;
\tunion { unsigned int Defense; unsigned int Ac; };
\tunion { unsigned int MaxHP; unsigned int MaxHp; };
\tunion { unsigned int MaxMP; unsigned int MaxMp; };
\tunion { unsigned int CurHP; unsigned int Hp; };
\tunion { unsigned int CurMP; unsigned int Mp; };
\tunsigned int Str;
\tunsigned int Int;
\tunsigned int Dex;
\tunsigned int Con;
\tunsigned int Accuracy;
\tunsigned int Evasion;
\tunsigned int Parry;
\tunsigned int Critical;
\tunsigned int Range;
\tunsigned int ResistFire;
\tunsigned int ResistIce;
\tunsigned int ResistHoly;
\tunsigned int ResistThunder;
\tunsigned int SaveMana;
\tunsigned int MagicAmp;
\tunsigned int RegenHP;
\tunsigned int RegenMP;
\tunsigned int StatusPts;
\tunsigned int MasterPts;
\tunsigned int SkillPts;
\tunion { unsigned int Mastery[4]; unsigned int Special[4]; };
\tunsigned int AttackRun;
\tunion { unsigned int Merchant; unsigned int Reserved; };
};'''
basedef = replace_once(basedef, old_score, new_score, "STRUCT_SCORE")
write(basedef_path, basedef)

compat_path = "client-source/tmproject/Projects/TMProject/WYD748Compat.cpp"
compat = read(compat_path)
compat = regex_once(
    compat,
    r"// These assertions protect the source client's side of the contract mirrored\n// by internal/wire/score48\.go\..*?static_assert\(offsetof\(STRUCT_SCORE, Special\) == 40, \"STRUCT_SCORE\.Special offset changed\"\);",
    '''// These assertions lock the canonical 7.48+ Score shared with Go. Any field
// insertion, narrowing or reordering must fail the Win32 build immediately.
static_assert(sizeof(STRUCT_SCORE) == 140, "canonical STRUCT_SCORE must remain 140 bytes");
static_assert(offsetof(STRUCT_SCORE, Version) == 0, "STRUCT_SCORE.Version offset changed");
static_assert(offsetof(STRUCT_SCORE, Level) == 4, "STRUCT_SCORE.Level offset changed");
static_assert(offsetof(STRUCT_SCORE, Attack) == 8, "STRUCT_SCORE.Attack offset changed");
static_assert(offsetof(STRUCT_SCORE, MagicAttack) == 12, "STRUCT_SCORE.MagicAttack offset changed");
static_assert(offsetof(STRUCT_SCORE, Defense) == 16, "STRUCT_SCORE.Defense offset changed");
static_assert(offsetof(STRUCT_SCORE, MaxHP) == 20, "STRUCT_SCORE.MaxHP offset changed");
static_assert(offsetof(STRUCT_SCORE, MaxMP) == 24, "STRUCT_SCORE.MaxMP offset changed");
static_assert(offsetof(STRUCT_SCORE, CurHP) == 28, "STRUCT_SCORE.CurHP offset changed");
static_assert(offsetof(STRUCT_SCORE, CurMP) == 32, "STRUCT_SCORE.CurMP offset changed");
static_assert(offsetof(STRUCT_SCORE, Str) == 36, "STRUCT_SCORE.Str offset changed");
static_assert(offsetof(STRUCT_SCORE, Int) == 40, "STRUCT_SCORE.Int offset changed");
static_assert(offsetof(STRUCT_SCORE, Dex) == 44, "STRUCT_SCORE.Dex offset changed");
static_assert(offsetof(STRUCT_SCORE, Con) == 48, "STRUCT_SCORE.Con offset changed");
static_assert(offsetof(STRUCT_SCORE, Accuracy) == 52, "STRUCT_SCORE.Accuracy offset changed");
static_assert(offsetof(STRUCT_SCORE, Evasion) == 56, "STRUCT_SCORE.Evasion offset changed");
static_assert(offsetof(STRUCT_SCORE, Parry) == 60, "STRUCT_SCORE.Parry offset changed");
static_assert(offsetof(STRUCT_SCORE, Critical) == 64, "STRUCT_SCORE.Critical offset changed");
static_assert(offsetof(STRUCT_SCORE, Range) == 68, "STRUCT_SCORE.Range offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistFire) == 72, "STRUCT_SCORE.ResistFire offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistIce) == 76, "STRUCT_SCORE.ResistIce offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistHoly) == 80, "STRUCT_SCORE.ResistHoly offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistThunder) == 84, "STRUCT_SCORE.ResistThunder offset changed");
static_assert(offsetof(STRUCT_SCORE, SaveMana) == 88, "STRUCT_SCORE.SaveMana offset changed");
static_assert(offsetof(STRUCT_SCORE, MagicAmp) == 92, "STRUCT_SCORE.MagicAmp offset changed");
static_assert(offsetof(STRUCT_SCORE, RegenHP) == 96, "STRUCT_SCORE.RegenHP offset changed");
static_assert(offsetof(STRUCT_SCORE, RegenMP) == 100, "STRUCT_SCORE.RegenMP offset changed");
static_assert(offsetof(STRUCT_SCORE, StatusPts) == 104, "STRUCT_SCORE.StatusPts offset changed");
static_assert(offsetof(STRUCT_SCORE, MasterPts) == 108, "STRUCT_SCORE.MasterPts offset changed");
static_assert(offsetof(STRUCT_SCORE, SkillPts) == 112, "STRUCT_SCORE.SkillPts offset changed");
static_assert(offsetof(STRUCT_SCORE, Mastery) == 116, "STRUCT_SCORE.Mastery offset changed");
static_assert(offsetof(STRUCT_SCORE, AttackRun) == 132, "STRUCT_SCORE.AttackRun offset changed");
static_assert(offsetof(STRUCT_SCORE, Merchant) == 136, "STRUCT_SCORE.Merchant offset changed");''',
    "score static asserts",
    flags=re.S,
)
compat = regex_once(
    compat,
    r"// The source-client response family deliberately keeps the imported 48-byte\n// score.*?(?=// Client diagnostics are accepted)",
    '''// The source-client response family embeds the canonical 140-byte Score.
// These values are mirrored byte-for-byte by internal/wire/source_client.go.
static_assert(sizeof(STRUCT_SELCHAR) == 1272, "source STRUCT_SELCHAR ABI changed");
static_assert(sizeof(STRUCT_MOB) == 1224, "source STRUCT_MOB ABI changed");
static_assert(sizeof(STRUCT_EXT1) == 288, "source STRUCT_EXT1 ABI changed");
static_assert(sizeof(STRUCT_EXT2) == 552, "source STRUCT_EXT2 ABI changed");
static_assert(sizeof(MSG_CNFAccountLogin) == 2360, "source character-list packet ABI changed");
static_assert(offsetof(MSG_CNFAccountLogin, SecretCode) == 12, "source character-list SecretCode offset changed");
static_assert(offsetof(MSG_CNFAccountLogin, SelChar) == 32, "source character-list SelChar offset changed");
static_assert(offsetof(MSG_CNFAccountLogin, Cargo) == 1304, "source character-list cargo offset changed");
static_assert(sizeof(MSG_CNFNewCharacter) == 1288, "source create-character response ABI changed");
static_assert(sizeof(MSG_CNFDeleteCharacter) == 1288, "source delete-character response ABI changed");
static_assert(sizeof(MSG_CNFCharacterLogin) == 2104, "source enter-world packet ABI changed");
static_assert(offsetof(MSG_CNFCharacterLogin, Slot) == 1240, "source selected-slot offset changed");
static_assert(offsetof(MSG_CNFCharacterLogin, ClientID) == 1242, "source runtime ClientID offset changed");
static_assert(offsetof(MSG_CNFCharacterLogin, Ext1) == 1264, "source Ext1 alignment changed");
static_assert(offsetof(STRUCT_MOB, ScoreBonus) == 988, "source status-point mirror offset changed");
static_assert(offsetof(STRUCT_MOB, SpecialBonus) == 990, "source mastery-point mirror offset changed");
static_assert(offsetof(STRUCT_MOB, SkillBonus) == 992, "source skill-point mirror offset changed");
static_assert(sizeof(MSG_CreateMob) == 328, "source CreateMob packet ABI changed");
static_assert(sizeof(MSG_CreateMobTrade) == 352, "source CreateMobTrade packet ABI changed");
static_assert(sizeof(MSG_UpdateScore) == 244, "source UpdateScore packet ABI changed");
static_assert(offsetof(MSG_UpdateScore, ReqHp) == 228, "source UpdateScore.ReqHp offset changed");
static_assert(offsetof(MSG_UpdateScore, ReqMp) == 232, "source UpdateScore.ReqMp offset changed");
static_assert(offsetof(MSG_UpdateScore, Magician) == 236, "source UpdateScore.Magician offset changed");
static_assert(offsetof(MSG_UpdateScore, LearnedSkill) == 240, "source UpdateScore.LearnedSkill offset changed");
static_assert(sizeof(MSG_UpdateAffect) == 268, "source UpdateAffect packet ABI changed");
static_assert(sizeof(MSG_ShopList) == 236, "source ShopList packet ABI changed");
// These packets are incremental notifications, not alternative score models.
static_assert(sizeof(MSG_UpdateEquip) == 60, "7.48 UpdateEquip packet ABI changed");
static_assert(sizeof(MSG_UpdateEtc) == 48, "WYD-Go UpdateEtc extension ABI changed");
static_assert(sizeof(MSG_SetHpMp) == 36, "WYD-Go HP/MP incremental ABI changed");
''',
    "source ABI static asserts",
    flags=re.S,
)
write(compat_path, compat)

# ---------------------------------------------------------------------------
# Regression test: values that were previously truncated or absent from the
# 48-byte source score must survive byte-for-byte in the canonical serializer.
# ---------------------------------------------------------------------------
score_test = ROOT / "internal/wire/score_test.go"
score_test.write_text('''package wire

import (
\t"encoding/binary"
\t"testing"

\t"wydgo/internal/model"
)

func TestEncodeClientScorePreservesCanonicalUint32Fields(t *testing.T) {
\ts := &model.Score{
\t\tVersion: model.ScoreVersion,
\t\tLevel: 70000,
\t\tAttack: 123456,
\t\tMagicAttack: 234567,
\t\tDefense: 345678,
\t\tMaxHP: 4_000_000,
\t\tMaxMP: 3_000_000,
\t\tCurHP: 2_500_000,
\t\tCurMP: 1_500_000,
\t\tStr: 100000,
\t\tInt: 100001,
\t\tDex: 100002,
\t\tCon: 100003,
\t\tAccuracy: 456789,
\t\tEvasion: 567890,
\t\tParry: 678901,
\t\tCritical: 789012,
\t\tRange: 890123,
\t\tResistFire: 91,
\t\tResistIce: 92,
\t\tResistHoly: 93,
\t\tResistThunder: 94,
\t\tSaveMana: 95,
\t\tMagicAmp: 901234,
\t\tRegenHP: 1234,
\t\tRegenMP: 2345,
\t\tStatusPts: 500000,
\t\tMasterPts: 600000,
\t\tSkillPts: 700000,
\t\tMastery: [4]uint32{100000, 200000, 300000, 400000},
\t\tAttackRun: 0x12345678,
\t\tMerchant: 0x23456789,
\t}
\tb := EncodeClientScore(s)
\tif len(b) != 140 {
\t\tt.Fatalf("canonical score size=%d want=140", len(b))
\t}
\twant := []uint32{
\t\ts.Version, s.Level, s.Attack, s.MagicAttack, s.Defense,
\t\ts.MaxHP, s.MaxMP, s.CurHP, s.CurMP,
\t\ts.Str, s.Int, s.Dex, s.Con,
\t\ts.Accuracy, s.Evasion, s.Parry, s.Critical, s.Range,
\t\ts.ResistFire, s.ResistIce, s.ResistHoly, s.ResistThunder,
\t\ts.SaveMana, s.MagicAmp, s.RegenHP, s.RegenMP,
\t\ts.StatusPts, s.MasterPts, s.SkillPts,
\t\ts.Mastery[0], s.Mastery[1], s.Mastery[2], s.Mastery[3],
\t\ts.AttackRun, s.Merchant,
\t}
\tfor i, expected := range want {
\t\tgot := binary.LittleEndian.Uint32(b[i*4:(i+1)*4])
\t\tif got != expected {
\t\t\tt.Fatalf("word[%d]=%d want=%d", i, got, expected)
\t\t}
\t}
}
''', encoding="utf-8", newline="\n")

# Remove the obsolete narrow-source serializer test if present. Its replacement
# above explicitly covers the regression that matters to 7.48+.
old_score_test = ROOT / "internal/wire/score48_test.go"
if old_score_test.exists():
    old_score_test.unlink()

# ---------------------------------------------------------------------------
# Policy/docs: 7.48 is the behavioral baseline, while deliberate source ABI
# improvements are first-class when client and server migrate atomically.
# ---------------------------------------------------------------------------
client_agents_path = "client-source/AGENTS.md"
client_agents = read(client_agents_path)
client_agents = client_agents.replace(
    "The source client must compile exclusively for WYD 7.48. It must not keep multi-version branches or runtime compatibility modes.",
    "The source client targets WYD 7.48+: 7.48 gameplay, UI, assets and behavior with deliberate structural improvements. Intentional ABI upgrades are allowed when source client and Go server migrate atomically; accidental newer-version behavior is not."
)
client_agents = client_agents.replace(
    "Port the supported behavior into one 7.48 implementation, then remove newer-version branches once that path is covered.",
    "Classify each difference as 7.48 baseline, intentional 7.48+ improvement, accidental incompatibility, or temporary bridge. Port semantics deliberately and remove obsolete bridges once both sides share the new contract."
)
write(client_agents_path, client_agents)

skill_path = ".agents/skills/wyd-go-feature/SKILL.md"
skill = read(skill_path)
skill = skill.replace(
    "ABI/UI/protocolo/lifecycle devem seguir a 7.48; referencias novas nao sao alternativa de implementacao.",
    "A 7.48 continua sendo a referencia de comportamento/UI/assets. O 7.48+ pode adotar melhorias estruturais de versoes novas quando client-source e servidor forem migrados juntos, com contrato documentado, static_assert e teste byte-a-byte."
)
skill = skill.replace(
    "Aplique o comportamento 7.48 sem carregar layout/offset/endereco de outra versao.",
    "Aplique o comportamento 7.48 e trate qualquer layout novo como melhoria intencional do 7.48+, nunca como copia cega de offset/endereco de outra versao."
)
write(skill_path, skill)

# Replace the old ExtendedScore document with the canonical Score contract.
extended_doc = ROOT / "DOCS/EXTENDED_SCORE.md"
score_doc = ROOT / "DOCS/SCORE.md"
score_doc.write_text('''# Score canônico — WYD 7.48+

## Contrato

`model.Score` é a única representação autoritativa de atributos de personagem e mob.
O mesmo layout lógico é declarado como `STRUCT_SCORE` no client-source. São 35 campos
`uint32`, em ordem fixa, totalizando **140 bytes** no wire do client-source.

Ordem: `Version`, `Level`, `Attack`, `MagicAttack`, `Defense`, `MaxHP`, `MaxMP`,
`CurHP`, `CurMP`, `Str`, `Int`, `Dex`, `Con`, `Accuracy`, `Evasion`, `Parry`,
`Critical`, `Range`, quatro resistências, `SaveMana`, `MagicAmp`, `RegenHP`,
`RegenMP`, `StatusPts`, `MasterPts`, `SkillPts`, quatro `Mastery`, `AttackRun`
e `Merchant`.

Todos os valores persistidos continuam limitados a 2.000.000.000 para manter os
cálculos intermediários e caminhos antigos do renderer dentro do domínio signed.

## Autoridade

- `World` muta `Score`; o client nunca envia score autoritativo.
- `RuntimeScore` é apenas uma cópia calculada do mesmo tipo `Score` com buffs/debuffs;
  não é um segundo formato nem uma fonte persistente.
- PostgreSQL persiste `Score` diretamente no estado da conta. O JSON antigo com
  `extendedScore` não possui migração: as contas anteriores a este contrato serão
  recriadas.
- `LegacyScore28` existe somente na borda do executável stock 7.48. Ele não pode
  entrar em fórmulas, store ou estado do World.

## Client-source 7.48+

O client-source recebe os 140 bytes diretamente nos pacotes que embutem score.
Não há truncamento de level, atributos, masteries, HP/MP, ataque ou defesa, e os
campos que antes dependiam do sidecar `XSC2` agora fazem parte de `STRUCT_SCORE`.
`XSC2` pode continuar existindo exclusivamente para o executável stock enquanto
essa compatibilidade for mantida.

A ABI C++ é protegida por `WYD748Compat.cpp`; o encoder Go possui teste byte-a-byte.
Qualquer alteração de ordem/tamanho exige mudança atômica nas duas pontas.
''', encoding="utf-8", newline="\n")
if extended_doc.exists():
    extended_doc.unlink()

# Update textual references to the renamed canonical document and terminology.
for path in list(ROOT.rglob("*.md")):
    if path == score_doc:
        continue
    text = path.read_text(encoding="utf-8")
    original = text
    text = text.replace("DOCS/EXTENDED_SCORE.md", "DOCS/SCORE.md")
    text = text.replace("ExtendedScore v2", "Score v2")
    text = text.replace("ExtendedScore", "Score")
    text = text.replace("WireScore", "LegacyScore28")
    if text != original:
        path.write_text(text, encoding="utf-8", newline="\n")

print("canonical Score convergence applied")
