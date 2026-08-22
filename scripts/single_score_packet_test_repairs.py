from pathlib import Path

root = Path(__file__).resolve().parents[1]
path = root / "internal/wire/session_packets_test.go"


def replace_test(text: str, name: str, replacement: str) -> str:
    marker = f"func {name}(t *testing.T)"
    start = text.find(marker)
    if start < 0:
        if replacement.strip() in text:
            return text
        raise RuntimeError(f"test {name} not found")
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
                return text[:start] + replacement.rstrip() + "\n\n" + text[end:]
        i += 1
    raise RuntimeError(f"unterminated test {name}")


text = path.read_text(encoding="utf-8")
text = replace_test(text, "TestUpdateEtc748Layout", r'''func TestUpdateEtcCanonicalScoreLayout(t *testing.T) {
	ch := model.Char{
		CP: -25, Exp: 34000, LearnedSkill: 1 << 3, Gold: 99424,
		Score: &model.Score{
			Version: model.ScoreVersion,
			StatusPts: 7, MasterPts: 100, SkillPts: 150, MagicAmp: 70,
		},
	}
	b := UpdateEtc(7, ch)
	if len(b) != 168 || ParseHeader(b).Type != OpUpdateEtc || ParseHeader(b).ID != 7 ||
		binary.LittleEndian.Uint32(b[12:16]) != 0 ||
		binary.LittleEndian.Uint32(b[16:20]) != 34000 ||
		binary.LittleEndian.Uint32(b[20:24]) != 1<<3 ||
		binary.LittleEndian.Uint32(b[24:28]) != model.ScoreVersion ||
		binary.LittleEndian.Uint32(b[116:120]) != 70 ||
		binary.LittleEndian.Uint32(b[128:132]) != 7 ||
		binary.LittleEndian.Uint32(b[132:136]) != 100 ||
		binary.LittleEndian.Uint32(b[136:140]) != 150 ||
		binary.LittleEndian.Uint32(b[164:168]) != 99424 {
		t.Fatalf("canonical UpdateEtc invalido: % X", b)
	}
}''')
text = replace_test(text, "TestUpdateEtcExtendedWritesWidePointsTail", r'''func TestUpdateEtcCarriesWidePointsInsideCanonicalScore(t *testing.T) {
	ch := model.Char{
		Score: &model.Score{
			Version: model.ScoreVersion,
			StatusPts: 100000,
			MasterPts: 110000,
			SkillPts: 120000,
		},
	}
	b := UpdateEtc(1, ch)
	if len(b) != 168 ||
		binary.LittleEndian.Uint32(b[128:132]) != 100000 ||
		binary.LittleEndian.Uint32(b[132:136]) != 110000 ||
		binary.LittleEndian.Uint32(b[136:140]) != 120000 {
		t.Fatalf("wide points nao viajaram dentro do Score canonico: % X", b[120:144])
	}
}''')
path.write_text(text, encoding="utf-8", newline="\n")
print("canonical UpdateEtc packet regressions staged")
