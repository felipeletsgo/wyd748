from pathlib import Path
import re

root = Path(__file__).resolve().parents[1] if "__file__" in globals() else Path.cwd()


def remove_tests(path: Path, forbidden: tuple[str, ...]) -> None:
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    changed = True
    while changed:
        changed = False
        for match in list(re.finditer(r'func\s+(Test[A-Za-z0-9_]+)\(t \*testing\.T\)\s*\{', text)):
            brace = text.find("{", match.start())
            depth = 0
            i = brace
            while i < len(text):
                if text[i] == "{":
                    depth += 1
                elif text[i] == "}":
                    depth -= 1
                    if depth == 0:
                        body = text[match.start():i + 1]
                        if any(token in body for token in forbidden):
                            end = i + 1
                            while end < len(text) and text[end] in "\r\n":
                                end += 1
                            text = text[:match.start()] + text[end:]
                            changed = True
                        break
                i += 1
            if changed:
                break
    path.write_text(text, encoding="utf-8", newline="\n")


def replace_test(path: Path, name: str, replacement: str) -> None:
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    marker = f"func {name}(t *testing.T)"
    start = text.find(marker)
    if start < 0:
        return
    brace = text.find("{", start)
    depth = 0
    i = brace
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                while end < len(text) and text[end] in "\r\n":
                    end += 1
                text = text[:start] + replacement.rstrip() + "\n\n" + text[end:]
                path.write_text(text, encoding="utf-8", newline="\n")
                return
        i += 1
    raise RuntimeError(f"unterminated test {name}")


# Canonical wire behavior is covered by source_client_test.go. These tests only
# exercised the deleted stock/compatibility packet family.
wire_tests = root / "internal/wire/session_packets_test.go"
remove_tests(wire_tests, (
    "EnterWorld(", "CNFNewCharacter", "CNFDeleteCharacter", "SetMobHpMp",
    "SetHpMpExtended", "Compatibility", "LegacyScore", "ClientProtocol",
))

replace_test(wire_tests, "TestUpdateScore754LayoutIncludesSixteenAffects", r'''func TestUpdateScoreCanonicalLayoutIncludesScoreAndAffects(t *testing.T) {
	ch := model.Char{GuildID: 0x1234, GuildRank: model.GuildRankLeader, Score: &model.Score{
		Version: model.ScoreVersion, Level: 65,
		MaxHP: 900, CurHP: 900, MaxMP: 700, CurMP: 700, MagicAmp: 12,
		Critical: 3, SaveMana: 4, RegenHP: 5, RegenMP: 6,
		ResistFire: 7, ResistIce: 8, ResistHoly: 9, ResistThunder: 10,
	}}
	ch.Affects[0] = model.Affect{Type: 24, ExpiresAt: time.Now().Add(80 * time.Second)}
	b := UpdateScore(1, ch)
	if len(b) != 244 || ParseHeader(b).Type != OpUpdateScore || ParseHeader(b).ID != 1 ||
		binary.LittleEndian.Uint32(b[32:36]) != 900 ||
		binary.LittleEndian.Uint32(b[40:44]) != 900 ||
		b[152] != 3 || b[153] != 4 ||
		binary.LittleEndian.Uint16(b[154:156]) != 0x180A ||
		binary.LittleEndian.Uint16(b[218:220]) != 0x0234 ||
		binary.LittleEndian.Uint16(b[220:222]) != model.GuildRankLeader ||
		!bytes.Equal(b[222:226], []byte{7, 8, 9, 10}) ||
		binary.LittleEndian.Uint16(b[236:238]) != 12 {
		t.Fatalf("canonical UpdateScore incorreto: % X", b)
	}
}''')

replace_test(wire_tests, "TestUpdateAffects748FullLayout", r'''func TestUpdateAffectsCanonicalFullLayout(t *testing.T) {
	ch := model.Char{}
	ch.Affects[0] = model.Affect{Type: 24, Value: 7, Level: 40,
		ExpiresAt: time.Now().Add(80 * time.Second)}
	ch.Affects[15] = model.Affect{Type: 31, ClientType: 24, Value: 150, Level: 55,
		ExpiresAt: time.Now().Add(40 * time.Second)}
	b := UpdateAffects(9, ch)
	last := 12 + 15*8
	if len(b) != 268 || ParseHeader(b).Type != OpUpdateAffect || ParseHeader(b).ID != 9 ||
		b[12] != 24 || b[13] != 40 || binary.LittleEndian.Uint16(b[14:16]) != 7 ||
		binary.LittleEndian.Uint32(b[16:20]) != 10 ||
		b[last] != 24 || b[last+1] != 55 || binary.LittleEndian.Uint16(b[last+2:last+4]) != 150 ||
		binary.LittleEndian.Uint32(b[last+4:last+8]) != 5 {
		t.Fatalf("canonical UpdateAffects incorreto: first=% X last=% X", b[12:20], b[last:last+8])
	}
}''')

replace_test(wire_tests, "TestSkillHits748MultiLayout", r'''func TestSkillHitsCanonicalWideMultiLayout(t *testing.T) {
	targets := []SkillTarget{{ID: 1001, Damage: 40}, {ID: 1002, Damage: 55}}
	b := SkillHits(1, 2200, 2100, 2201, 2101, 1234, 70, 0, 9, 42, 13, targets)
	if len(b) != 112 || ParseHeader(b).Type != 0x36C ||
		binary.LittleEndian.Uint16(b[14:16]) != 2 ||
		binary.LittleEndian.Uint16(b[44:46]) != 1001 || binary.LittleEndian.Uint16(b[46:48]) != 40 ||
		binary.LittleEndian.Uint16(b[48:50]) != 1002 || binary.LittleEndian.Uint16(b[50:52]) != 55 ||
		binary.LittleEndian.Uint32(b[96:100]) != 0x58474D44 ||
		binary.LittleEndian.Uint32(b[100:104]) != 2 ||
		binary.LittleEndian.Uint32(b[104:108]) != 40 ||
		binary.LittleEndian.Uint32(b[108:112]) != 55 {
		t.Fatalf("SkillHits canonical multi invalido: %v", b)
	}
}''')

replace_test(wire_tests, "TestSkillHitExtendedKeepsSkillAndWideDamage", r'''func TestSkillHitWideKeepsSkillAndFullDamage(t *testing.T) {
	b := SkillHitExtended(1, 1001, 2200, 2100, 2201, 2101,
		275_000, 550_000, 1234, 70, 7, 0, 255)
	if len(b) != 60 ||
		int16(binary.LittleEndian.Uint16(b[24:26])) != 7 ||
		binary.LittleEndian.Uint16(b[46:48]) != 32_767 ||
		binary.LittleEndian.Uint32(b[48:52]) != 0x58474D44 ||
		binary.LittleEndian.Uint32(b[52:56]) != 1 ||
		binary.LittleEndian.Uint32(b[56:60]) != 275_000 || b[30] != 0 {
		t.Fatalf("SkillHit wide invalido: %v", b)
	}
}''')

replace_test(wire_tests, "TestCharListCarriesGuildIndexInSelection", r'''func TestCharListCarriesGuildIndexInSelection(t *testing.T) {
	ch := model.Char{Name: "GuildHero", X: 2100, Y: 2101, GuildID: 0x1234,
		Score: &model.Score{Version: model.ScoreVersion}}
	b := CharList("account", []model.Char{ch}, nil, 0)
	const guildOffset = 32 + 1216
	if len(b) != 2360 || binary.LittleEndian.Uint16(b[guildOffset:guildOffset+2]) != 0x0234 {
		t.Fatalf("Guild no SelectChar=0x%X, esperado id de 12 bits", binary.LittleEndian.Uint16(b[guildOffset:guildOffset+2]))
	}
}''')

replace_test(wire_tests, "TestMessageChat748Layout", r'''func TestMessageChatCanonicalLayout(t *testing.T) {
	b := MessageChat(7, "hello")
	h := ParseHeader(b)
	if len(b) != 140 || h.Type != OpMessageChat || h.ID != 7 || string(b[12:17]) != "hello" {
		t.Fatalf("MessageChat header/layout invalido: len=%d header=%+v", len(b), h)
	}
	// Size/checksum/tick are transport-owned and are finalized only by Session.Send.
	if h.Size != 0 || h.CheckSum != 0 || h.Tick != 0 {
		t.Fatalf("builder finalizou campos de transporte prematuramente: %+v", h)
	}
}''')

# Gameplay tests must assert model.Score directly. Projection-specific tests are
# invalid once compatibility helpers cease to exist.
remove_tests(root / "internal/game/affects_test.go", (
    "CompatibilityScore", "CompatibilityHPScale", "LegacyScore",
))
game_score_tests = root / "internal/game/score_test.go"
remove_tests(game_score_tests, (
    "CompatibilityScore", "CompatibilityCombatMP", "SetHpMpExtended",
    "CompatibilityVitals", "CompatibilityHPScale", "LegacyScore", "XSC2",
))
if game_score_tests.exists():
    text = game_score_tests.read_text(encoding="utf-8")
    if "binary." not in text:
        text = text.replace('\t"encoding/binary"\n', '')
    if "wire." not in text:
        text = text.replace('\t"wydgo/internal/wire"\n', '')
    game_score_tests.write_text(text, encoding="utf-8", newline="\n")

print("canonical score test regressions aligned")
