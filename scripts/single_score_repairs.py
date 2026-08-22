from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]


def rw(rel: str, transform):
    path = root / rel
    text = path.read_text(encoding="utf-8")
    new = transform(text)
    path.write_text(new, encoding="utf-8", newline="\n")


# NPC current HP is bounded directly by the canonical Score.
def fix_model(text: str) -> str:
    old = "\tscore.CurHP = minScoreValue(currentHP, score.MaxHP)"
    new = "\tscore.CurHP = currentHP\n\tif score.CurHP > score.MaxHP {\n\t\tscore.CurHP = score.MaxHP\n\t}"
    return text.replace(old, new, 1)

rw("internal/model/model.go", fix_model)


# Session no longer negotiates or stores a protocol family.
def fix_session(text: str) -> str:
    text = re.sub(
        r"\n// SetClientProtocol records.*?\nfunc \(s \*Session\) ClientProtocol\(\) wire\.ClientProtocol \{.*?\n\}\n",
        "\n",
        text,
        flags=re.S,
    )
    return text

rw("internal/net/session.go", fix_session)


# Runtime Score is already uint32. Compact combat packets may narrow at their
# own serialization field, but gameplay never asks wire for a score projection.
def fix_game_score(text: str) -> str:
    text = text.replace('\n\t"wydgo/internal/wire"', "")
    text = re.sub(
        r"// Os pacotes de combate.*?func playerCombatMP\(ch \*model\.Char\) uint32 \{\n\treturn wire\.CompatibilityCombatMP\(effectiveScore\(ch\)\)\n\}",
        "// playerCombatMP returns the authoritative MP; packet builders own any field-width encoding.\nfunc playerCombatMP(ch *model.Char) uint32 {\n\treturn playerCurMP(ch)\n}",
        text,
        flags=re.S,
    )
    return text

rw("internal/game/score.go", fix_game_score)


# Party UI has WORD fields, but it consumes values directly from Score. This is
# packet-field encoding, not a second score representation.
def fix_party(text: str) -> str:
    patterns = [
        (r"level, currentHP, maximumHP := wire\.CompatibilityVitals\(wireScoreState\(inviter\.Char\)\)",
         "level, currentHP, maximumHP := playerLevel(inviter.Char), playerCurHP(inviter.Char), playerMaxHP(inviter.Char)"),
        (r"level, currentHP, maximumHP := wire\.CompatibilityVitals\(wireScoreState\(member\.Char\)\)",
         "level, currentHP, maximumHP := playerLevel(member.Char), playerCurHP(member.Char), playerMaxHP(member.Char)"),
        (r"level, currentHP, maximumHP := wire\.CompatibilityVitals\(wireScoreState\(candidate\.Char\)\)",
         "level, currentHP, maximumHP := playerLevel(candidate.Char), playerCurHP(candidate.Char), playerMaxHP(candidate.Char)"),
    ]
    for pattern, replacement in patterns:
        text = re.sub(pattern, replacement, text)
    return text

rw("internal/game/party.go", fix_party)


# Narrow scalar packet fields explicitly. No helper may imply a legacy Score.
def fix_codec(text: str) -> str:
    text = text.replace("compatibilityU16", "packetU16")
    text = text.replace("func packetU16(value uint32) uint16 {\n\tif value > 30_000 {\n\t\treturn 30_000\n\t}\n\treturn uint16(value)\n}",
                        "func packetU16(value uint32) uint16 {\n\tif value > 65_535 {\n\t\treturn 65_535\n\t}\n\treturn uint16(value)\n}")

    # The public SkillHits is the uint32-aware source-client packet builder in
    # skill_hits_wide.go. Keep this compact layout as an internal prefix helper.
    text = text.replace("func SkillHits(attackerID, attackerX, attackerY, targetX, targetY uint16,\n",
                        "func buildSkillHitsPacket(attackerID, attackerX, attackerY, targetX, targetY uint16,\n", 1)
    text = text.replace("\treturn SkillHits(attackerID, attackerX, attackerY, targetX, targetY,\n\t\tcurrentExp, currentMP, skill, motion, mastery, 1,\n\t\t[]SkillTarget{{ID: targetID, Damage: damage, MaxHP: targetMaxHP}})\n}\n\n// SpectralVisual",
                        "\treturn buildSkillHitsPacket(attackerID, attackerX, attackerY, targetX, targetY,\n\t\tcurrentExp, currentMP, skill, motion, mastery, 1,\n\t\t[]SkillTarget{{ID: targetID, Damage: damage, MaxHP: targetMaxHP}})\n}\n\n// SpectralVisual")

    # Damage display fields are signed WORDs by packet definition. Clamp that
    # visual field directly; actual HP and real damage remain uint32 elsewhere.
    text = re.sub(
        r"func wireDamage\(t SkillTarget\) uint16 \{.*?\n\}",
        "func wireDamage(t SkillTarget) uint16 {\n"
        "\tif t.Miss {\n\t\treturn 0xFFFD\n\t}\n"
        "\tif t.Heal > 0 {\n\t\tv := t.Heal\n\t\tif v > 32767 { v = 32767 }\n\t\treturn uint16(-int16(v))\n\t}\n"
        "\tv := t.Damage\n\tif v > 32767 { v = 32767 }\n\treturn uint16(v)\n}",
        text,
        count=1,
        flags=re.S,
    )
    return text

rw("internal/wire/codec.go", fix_codec)


def fix_skill_hits(text: str) -> str:
    text = text.replace("base := SkillHits(attackerID, attackerX, attackerY, targetX, targetY,",
                        "base := buildSkillHitsPacket(attackerID, attackerX, attackerY, targetX, targetY,")
    text = text.replace("Patch-WYD748-ExtendedStats.ps1", "client-source 7.48+")
    text = text.replace("patch .xstat", "client-source")
    return text

rw("internal/wire/skill_hits_wide.go", fix_skill_hits)


# Canonical source packet code may narrow individual non-Score fields, but does
# not use compatibility terminology or stock-client branches.
def fix_source(text: str) -> str:
    text = text.replace("compatibilityU16", "packetU16")
    text = text.replace("stock XSC2", "removed stock score")
    text = text.replace("stock-client builder", "canonical builder")
    return text

rw("internal/wire/source_client.go", fix_source)


# Party packet fields remain WORD-sized in the existing UI ABI. Accept uint32
# values from Score and narrow only at the field write.
def fix_party_wire(text: str) -> str:
    text = text.replace(
        "func PartyRequest(leaderID uint16, name string, class byte, level, hp, maxHP uint16, targetID uint16) []byte {",
        "func PartyRequest(leaderID uint16, name string, class byte, level, hp, maxHP uint32, targetID uint16) []byte {",
    )
    text = text.replace(
        "func PartyMember(id uint16, name string, class, partyIndex byte, level, hp, maxHP uint16) []byte {",
        "func PartyMember(id uint16, name string, class, partyIndex byte, level, hp, maxHP uint32) []byte {",
    )
    text = text.replace("func partyDisplayHP(hp, maxHP uint16) (uint16, uint16) {",
                        "func partyDisplayHP(hp, maxHP uint32) (uint16, uint16) {")
    text = text.replace("\treturn hp, maxHP\n}\n\nfunc putU16", "\treturn packetU16(hp), packetU16(maxHP)\n}\n\nfunc putU16", 1)
    # After partyDisplayHP, level also needs explicit packet encoding.
    text = text.replace("putU16(b, 14, level)", "putU16(b, 14, packetU16(level))")
    return text

rw("internal/wire/codec.go", fix_party_wire)

print("single-score repairs applied")
