from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]


def remove_go_func(text: str, name: str) -> str:
    marker = f"func {name}("
    start = text.find(marker)
    if start < 0:
        return text
    comment_start = start
    while comment_start > 0:
        prev = text.rfind("\n", 0, comment_start - 1)
        line = text[prev + 1:comment_start].rstrip("\r\n")
        if line.lstrip().startswith("//"):
            comment_start = prev + 1
            continue
        break
    brace = text.find("{", start)
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
    raise RuntimeError(f"unterminated Go function {name}")


def write(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", newline="\n")


# Visibility has one ABI. Remove the stock/source wrapper and materialize every
# player with the canonical 140-byte Score packets.
path = root / "internal/game/visibility.go"
text = path.read_text(encoding="utf-8")
start = text.find("// playerEnterViewPackets monta")
end = text.find("// refreshAppearance publica", start)
if start < 0 or end < 0:
    raise RuntimeError("visibility.go canonical materialization block not found")
replacement = '''// playerEnterViewPackets materializes a player using the only 7.48+ ABI.\n// CreateMob carries the canonical Score; HpMp immediately confirms live\n// resources and ActionStop fixes the initial pose without rebuilding gameplay.\nfunc playerEnterViewPackets(subject *Player) [][]byte {\n\tif subject == nil || subject.Char == nil {\n\t\treturn nil\n\t}\n\treturn [][]byte{\n\t\twire.CreateMobWithGuildRank(subject.ID, subject.Char.Name, subject.X, subject.Y,\n\t\t\tbodyMesh(subject.Char), bodyAncient(subject.Char), wireScoreState(subject.Char),\n\t\t\tsubject.Char.Affects[:], 2, subject.Char.GuildID, subject.Char.GuildRank, subject.Char.CP),\n\t\twire.HpMp(subject.ID, wireScoreState(subject.Char)),\n\t\twire.ActionStop(subject.ID, subject.X, subject.Y),\n\t}\n}\n\nfunc sendPlayerEnterView(observer, subject *Player) {\n\tif observer == nil || observer.Session == nil {\n\t\treturn\n\t}\n\tfor _, pkt := range playerEnterViewPackets(subject) {\n\t\tobserver.Session.Send(pkt)\n\t}\n}\n\n'''
text = text[:start] + replacement + text[end:]
write(path, text)

# Projection-only HP diagnostics are invalid in a single-score architecture.
hp_debug = root / "internal/game/hp_debug.go"
if hp_debug.exists():
    hp_debug.unlink()

path = root / "internal/game/commands.go"
text = path.read_text(encoding="utf-8")
text = text.replace(', "hpdebug": "hpdebug"', '')
text = re.sub(r'\n\tcase "hpdebug":\n\t\tw\.dumpHPProjection\(s, p\)', '', text)
write(path, text)

# Remove tests whose only purpose was the deleted projection command.
for rel in ["internal/game/coverage_more_test.go", "internal/game/hp_base_invariant_test.go"]:
    path = root / rel
    if not path.exists():
        continue
    text = path.read_text(encoding="utf-8")
    # Delete complete test functions/subtests only when they mention hpdebug or
    # dumpHPProjection. Compile-time references must not survive.
    for name in re.findall(r'func\s+(Test[A-Za-z0-9_]+)\(t \*testing\.T\)', text):
        marker = f"func {name}(t *testing.T)"
        pos = text.find(marker)
        if pos < 0:
            continue
        brace = text.find("{", pos)
        depth = 0
        i = brace
        while i < len(text):
            if text[i] == "{": depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    body = text[pos:i+1]
                    if "hpdebug" in body.lower() or "dumpHPProjection" in body:
                        text = text[:pos] + text[i+1:]
                    break
            i += 1
    write(path, text)

# source_client_test.go is rewritten around the only supported ABI. Historical
# stock/source negotiation tests are intentionally not preserved.
path = root / "internal/wire/source_client_test.go"
write(path, r'''package wire

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func canonicalWireTestChar() model.Char {
	score := &model.Score{
		Version: model.ScoreVersion,
		Level: 7, Attack: 321, MagicAttack: 444, Defense: 123,
		MaxHP: 6000, MaxMP: 2000, CurHP: 5000, CurMP: 1500,
		Critical: 11, SaveMana: 9, MagicAmp: 65,
	}
	ch := model.Char{Name: "ProtocolHero", X: 2100, Y: 2101, Gold: 777, Exp: 998877, Score: score}
	ch.Equip[15] = model.Item{Index: 4152}
	ch.Inv[62] = model.Item{Index: 4011}
	ch.Affects[0] = model.Affect{Type: 24, Level: 40, Value: 150, ExpiresAt: time.Now().Add(80 * time.Second)}
	return ch
}

func TestCanonicalSelectionAndEnterWorldUseScore140(t *testing.T) {
	ch := canonicalWireTestChar()
	selection := CharacterSelectionUpdate(OpCNFNewCharacter, 3, []model.Char{ch})
	if len(selection) != 1288 || binary.LittleEndian.Uint16(selection[16:18]) != 2100 {
		t.Fatalf("selection ABI: len=%d", len(selection))
	}
	// STRUCT_SELCHAR starts at 16; Score[0] starts at +80 and Attack at +8.
	if got := binary.LittleEndian.Uint32(selection[104:108]); got != 321 {
		t.Fatalf("selection Attack=%d", got)
	}

	enter := EnterWorld(9, 3, ch)
	if len(enter) != 2104 || ParseHeader(enter).Type != OpEnterWorld ||
		binary.LittleEndian.Uint16(enter[1240:1242]) != 3 ||
		binary.LittleEndian.Uint16(enter[1242:1244]) != 9 {
		t.Fatalf("enter-world ABI: len=%d", len(enter))
	}
	// STRUCT_MOB starts at 16. BaseScore @44 and CurrentScore @184.
	if got := binary.LittleEndian.Uint32(enter[68:72]); got != 321 {
		t.Fatalf("base Attack=%d", got)
	}
	if got := binary.LittleEndian.Uint32(enter[208:212]); got != 321 {
		t.Fatalf("runtime Attack=%d", got)
	}
	if got := binary.LittleEndian.Uint16(enter[16+324+15*8 : 16+326+15*8]); got != 4152 {
		t.Fatalf("equip[15]=%d", got)
	}
	if got := binary.LittleEndian.Uint16(enter[16+468+62*8 : 16+470+62*8]); got != 4011 {
		t.Fatalf("inv[62]=%d", got)
	}
}

func TestCanonicalScoreRefreshUsesEmbeddedScore(t *testing.T) {
	ch := canonicalWireTestChar()
	b := UpdateScore(7, ch)
	if len(b) != 244 || ParseHeader(b).Type != OpUpdateScore {
		t.Fatalf("UpdateScore ABI: len=%d", len(b))
	}
	if got := binary.LittleEndian.Uint32(b[20:24]); got != ch.Score.Attack {
		t.Fatalf("Attack=%d", got)
	}
	if got := binary.LittleEndian.Uint32(b[32:36]); got != ch.Score.MaxHP {
		t.Fatalf("MaxHP=%d", got)
	}
	if got := binary.LittleEndian.Uint32(b[40:44]); got != ch.Score.CurHP {
		t.Fatalf("CurHP=%d", got)
	}
	if reqHP, reqMP := binary.LittleEndian.Uint32(b[228:232]), binary.LittleEndian.Uint32(b[232:236]); reqHP != 0 || reqMP != 0 {
		t.Fatalf("pending costs=%d/%d", reqHP, reqMP)
	}
}

func TestCanonicalMobPacketsUseScore140(t *testing.T) {
	score := &model.Score{Version: model.ScoreVersion, Attack: 777, Defense: 333, MaxHP: 12000, CurHP: 11000}
	create := CreateMobVisual(1001, "Observer", 100, 101, []uint16{22}, []byte{3}, score, nil, 0)
	if len(create) != 328 || ParseHeader(create).Type != OpCreateMob || binary.LittleEndian.Uint32(create[148:152]) != 777 {
		t.Fatalf("CreateMob ABI: len=%d", len(create))
	}
	refresh := MobScore(1001, score, nil, model.ElementalResists{Fire: 10, Ice: 20, Sacred: 30, Thunder: 40})
	if len(refresh) != 244 || binary.LittleEndian.Uint32(refresh[20:24]) != 777 {
		t.Fatalf("MobScore ABI: len=%d", len(refresh))
	}
}

func TestCanonicalHpMpUsesOnlyUint32Resources(t *testing.T) {
	score := &model.Score{Version: model.ScoreVersion, CurHP: 750000, CurMP: 250000, MaxHP: 1000000, MaxMP: 500000}
	b := SetHpMp(1001, score)
	if len(b) != 28 || ParseHeader(b).Type != OpSetHpMp ||
		binary.LittleEndian.Uint32(b[12:16]) != score.CurHP ||
		binary.LittleEndian.Uint32(b[16:20]) != score.CurMP ||
		binary.LittleEndian.Uint32(b[20:24]) != score.MaxHP ||
		binary.LittleEndian.Uint32(b[24:28]) != score.MaxMP {
		t.Fatalf("HpMp ABI: % X", b)
	}
}

func TestCanonicalCharListAndPresentationPackets(t *testing.T) {
	ch := canonicalWireTestChar()
	var cargo [model.MaxCargo]model.Item
	cargo[3] = model.Item{Index: 4011}
	b := CharList("account", []model.Char{ch}, cargo[:], 54321)
	if len(b) != 2360 || ParseHeader(b).Type != OpCharList || binary.LittleEndian.Uint32(b[120:124]) != ch.Score.Attack {
		t.Fatalf("CharList ABI: len=%d", len(b))
	}
	trade := CreateMobTrade(5, "Shop", 102, 103, nil, ch.Score, "Store")
	if len(trade) != 352 || string(trade[326:331]) != "Store" {
		t.Fatalf("CreateMobTrade ABI")
	}
	shop := ShopList([]model.Item{{Index: 4011}}, 3, ShopNormal)
	if len(shop) != 236 || binary.LittleEndian.Uint16(shop[16:18]) != 4011 || binary.LittleEndian.Uint32(shop[232:236]) != 3 {
		t.Fatalf("ShopList ABI")
	}
	chat := MessageChat(5, "hello")
	if len(chat) != 140 || string(chat[12:17]) != "hello" {
		t.Fatalf("MessageChat ABI")
	}
}
''')

# Remove every session-packet test function that directly references a deleted
# score compatibility symbol. Unrelated protocol tests stay intact.
path = root / "internal/wire/session_packets_test.go"
text = path.read_text(encoding="utf-8")
forbidden = (
    "LegacyScore28", "CompatibilityScore", "CompatibilityHPScale",
    "SetHpMpExtended", "CreateMobExtendedWithGuildRank", "ClientProtocol",
    "XSC2", "CreateMob(",
)
changed = True
while changed:
    changed = False
    matches = list(re.finditer(r'func\s+(Test[A-Za-z0-9_]+)\(t \*testing\.T\)\s*\{', text))
    for match in matches:
        brace = text.find("{", match.start())
        depth = 0
        i = brace
        while i < len(text):
            if text[i] == "{": depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    body = text[match.start():i+1]
                    if any(token in body for token in forbidden):
                        start = match.start()
                        end = i + 1
                        while end < len(text) and text[end] in "\r\n": end += 1
                        text = text[:start] + text[end:]
                        changed = True
                    break
            i += 1
        if changed:
            break
# Canonical ShopList has 27 entries / 236 bytes; any remaining old size check is obsolete.
text = text.replace("len(b) != 532", "len(b) != 236")
# Scalar WORD fields clamp at the field width, not a score-specific 30k projection.
text = text.replace("!= 30_000", "!= 65_535")
write(path, text)

print("final single-score Go/test references removed")
