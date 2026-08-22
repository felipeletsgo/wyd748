from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]


def write(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", newline="\n")


# single_score_game_repairs.py already canonicalizes visibility. This pass only
# verifies that the protocol selector is gone; reruns must therefore be safe.
visibility = (root / "internal/game/visibility.go").read_text(encoding="utf-8")
if "ClientProtocol" in visibility or "ClientProtocolStock748" in visibility:
    raise RuntimeError("visibility.go still contains protocol compatibility")
if "func playerEnterViewPackets(subject *Player)" not in visibility:
    raise RuntimeError("visibility.go canonical materialization function missing")

# Projection diagnostics no longer belong to a one-score architecture.
hp_debug = root / "internal/game/hp_debug.go"
if hp_debug.exists():
    hp_debug.unlink()

path = root / "internal/game/commands.go"
text = path.read_text(encoding="utf-8")
text = text.replace(', "hpdebug": "hpdebug"', '')
text = re.sub(r'\n\tcase "hpdebug":\n\t\tw\.dumpHP(?:Projection|State)\(s, p\)', '', text)
write(path, text)

# Remove tests whose only purpose was the deleted projection/debug command.
for rel in ["internal/game/coverage_more_test.go", "internal/game/hp_base_invariant_test.go"]:
    path = root / rel
    if not path.exists():
        continue
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
                        if "hpdebug" in body.lower() or "dumpHPProjection" in body or "dumpHPState" in body:
                            end = i + 1
                            while end < len(text) and text[end] in "\r\n":
                                end += 1
                            text = text[:match.start()] + text[end:]
                            changed = True
                        break
                i += 1
            if changed:
                break
    write(path, text)

# The source-client tests define the only supported score/wire ABI. Historical
# stock/source negotiation and projection tests are deliberately replaced.
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
	if got := binary.LittleEndian.Uint32(selection[104:108]); got != 321 {
		t.Fatalf("selection Attack=%d", got)
	}

	enter := EnterWorld(9, 3, ch)
	if len(enter) != 2104 || ParseHeader(enter).Type != OpEnterWorld ||
		binary.LittleEndian.Uint16(enter[1240:1242]) != 3 ||
		binary.LittleEndian.Uint16(enter[1242:1244]) != 9 {
		t.Fatalf("enter-world ABI: len=%d", len(enter))
	}
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

# Drop only regression tests that exercise APIs deleted by the single-score
# migration. Unrelated packet tests remain intact.
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
text = text.replace("len(b) != 532", "len(b) != 236")
text = text.replace("!= 30_000", "!= 65_535")
write(path, text)

print("final single-score Go/test references removed")
