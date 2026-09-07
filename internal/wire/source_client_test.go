package wire

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func canonicalWireTestChar() model.Char {
	score := &model.Score{
		Version: model.ScoreVersion,
		Level:   7, Attack: 321, MagicAttack: 444, Defense: 123,
		MaxHP: 6000, MaxMP: 2000, CurHP: 5000, CurMP: 1500,
		Critical: 11, SaveMana: 9, MagicAmp: 65,
	}
	ch := model.Char{Name: "ProtocolHero", X: 2100, Y: 2101, Gold: 777, Exp: 998877, Hold: 4321, Score: score}
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
	if got := binary.LittleEndian.Uint32(enter[1264:1268]); got != ch.Hold {
		t.Fatalf("Ext1.Data[0]/Hold=%d", got)
	}
}

func TestCanonicalScoreRefreshUsesEmbeddedScore(t *testing.T) {
	ch := canonicalWireTestChar()
	b := UpdateScore(7, ch)
	if len(b) != 232 || ParseHeader(b).Type != OpUpdateScore {
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
	if reqHP, reqMP := binary.LittleEndian.Uint32(b[220:224]), binary.LittleEndian.Uint32(b[224:228]); reqHP != 0 || reqMP != 0 {
		t.Fatalf("pending costs=%d/%d", reqHP, reqMP)
	}
}

func TestCanonicalMobPacketsUseScore140(t *testing.T) {
	score := &model.Score{Version: model.ScoreVersion, Attack: 777, Defense: 333, MaxHP: 12000, CurHP: 11000}
	mesh := make([]uint16, 18)
	anct := make([]byte, 18)
	mesh[0], mesh[17], anct[0], anct[17] = 22, 39, 3, 7
	create := CreateMobWithGuildRank(1001, "Observer", 100, 101, mesh, anct,
		score, nil, 2, 0x1234, model.GuildRankLeader, 55)
	if len(create) != 328 || ParseHeader(create).Type != OpCreateMob || ParseHeader(create).ID != SceneField ||
		binary.LittleEndian.Uint16(create[12:14]) != 100 ||
		binary.LittleEndian.Uint16(create[14:16]) != 101 ||
		binary.LittleEndian.Uint16(create[16:18]) != 1001 ||
		string(create[18:26]) != "Observer" ||
		create[30] != CPNameByte(55) ||
		binary.LittleEndian.Uint16(create[34:36]) != 22 ||
		binary.LittleEndian.Uint16(create[68:70]) != 39 ||
		binary.LittleEndian.Uint16(create[134:136]) != 0x0234 ||
		create[136] != model.GuildRankLeader ||
		binary.LittleEndian.Uint32(create[148:152]) != 777 ||
		binary.LittleEndian.Uint16(create[280:282]) != 2 ||
		create[282] != 3 || create[299] != 7 {
		t.Fatalf("CreateMob ABI: len=%d", len(create))
	}
	refresh := MobScore(1001, score, nil)
	if len(refresh) != 232 || binary.LittleEndian.Uint32(refresh[20:24]) != 777 {
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
	if len(trade) != 352 || ParseHeader(trade).Type != OpCreateMobTrade ||
		ParseHeader(trade).ID != SceneField ||
		binary.LittleEndian.Uint16(trade[12:14]) != 102 ||
		binary.LittleEndian.Uint16(trade[14:16]) != 103 ||
		binary.LittleEndian.Uint16(trade[16:18]) != 5 ||
		string(trade[18:22]) != "Shop" || string(trade[326:331]) != "Store" ||
		trade[350] != 0 || trade[351] != 0 {
		t.Fatalf("CreateMobTrade ABI")
	}
	shop := ShopList([]model.Item{{Index: 4011}}, 3, ShopNormal)
	if len(shop) != 236 || binary.LittleEndian.Uint16(shop[16:18]) != 4011 || binary.LittleEndian.Uint32(shop[232:236]) != 3 {
		t.Fatalf("ShopList ABI")
	}
	chat := MessageChat(5, "hello")
	if len(chat) != 108 || string(chat[12:17]) != "hello" {
		t.Fatalf("MessageChat ABI")
	}
}
