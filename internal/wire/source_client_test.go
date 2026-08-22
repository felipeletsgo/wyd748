package wire

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestClientProtocolFromLoginKeepsStockAsDefault(t *testing.T) {
	stock := make([]byte, 116)
	if got := ClientProtocolFromLogin(stock); got != ClientProtocolStock748 {
		t.Fatalf("stock protocol = %d; want %d", got, ClientProtocolStock748)
	}
	binary.LittleEndian.PutUint32(stock[44:48], SourceClientProtocolMarker)
	if got := ClientProtocolFromLogin(stock); got != ClientProtocolSource748 {
		t.Fatalf("source protocol = %d; want %d", got, ClientProtocolSource748)
	}
}

func TestSourceSelectionUpdateAndEnterWorldMatchTMProjectABI(t *testing.T) {
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Level:   7, Attack: 321, Defense: 123, MaxHP: 6000, CurHP: 5000,
		MaxMP: 2000, CurMP: 1500, Critical: 11, SaveMana: 9,
	}
	ch := model.Char{Name: "ProtocolHero", X: 2100, Y: 2101, Gold: 777, Exp: 998877, Extended: ext}
	ch.Equip[15] = model.Item{Index: 4152}
	ch.Inv[62] = model.Item{Index: 4011}
	ch.Affects[0] = model.Affect{Type: 24, Level: 40, Value: 150, ExpiresAt: time.Now().Add(80 * time.Second)}

	selection := CharacterSelectionUpdateForProtocol(ClientProtocolSource748, OpCNFNewCharacter, 3, []model.Char{ch})
	if len(selection) != 920 || binary.LittleEndian.Uint16(selection[16:18]) != 2100 {
		t.Fatalf("source selection layout invalid: len=%d x=%d", len(selection), binary.LittleEndian.Uint16(selection[16:18]))
	}

	enter := SourceEnterWorld(9, 3, ch)
	if len(enter) != 1728 || ParseHeader(enter).Type != OpEnterWorld || binary.LittleEndian.Uint16(enter[1058:1060]) != 9 {
		t.Fatalf("source enter-world layout invalid: len=%d header=%+v id=%d", len(enter), ParseHeader(enter), binary.LittleEndian.Uint16(enter[1058:1060]))
	}
	if got := binary.LittleEndian.Uint16(enter[1056:1058]); got != 3 {
		t.Fatalf("source enter-world slot = %d; want 3", got)
	}
	if got := binary.LittleEndian.Uint16(enter[16+140+15*8 : 16+142+15*8]); got != 4152 {
		t.Fatalf("source equip[15] = %d", got)
	}
	if got := binary.LittleEndian.Uint16(enter[16+284+62*8 : 16+286+62*8]); got != 4011 {
		t.Fatalf("source inv[62] = %d", got)
	}
	// Source STRUCT_AFFECT is Type, Level, signed Value, Time. Ext1.Data[8]
	// occupies 32 bytes before Affect[0], so the first affect is exactly @1110.
	affect := 1110
	if enter[affect] != 24 || enter[affect+1] != 40 || binary.LittleEndian.Uint16(enter[affect+2:affect+4]) != 150 {
		t.Fatalf("source affect bytes = % X", enter[affect:affect+8])
	}
}

func TestSourceWorldPacketsKeepSourceOnlySizes(t *testing.T) {
	ext := &model.ExtendedScore{Version: model.ExtendedScoreVersion, Attack: 999, MaxHP: 7000, CurHP: 6500}
	cp := int16(5)
	create := sourceCreateMob(4, "Observer", 100, 101, []uint16{22}, []byte{3}, ext, nil, 2, 7, 1, &cp)
	if len(create) != 236 || ParseHeader(create).Type != OpCreateMob || binary.LittleEndian.Uint32(create[148:152]) != 999 {
		t.Fatalf("source CreateMob layout invalid: len=%d header=%+v", len(create), ParseHeader(create))
	}
	trade := CreateMobTradeExtendedForProtocol(ClientProtocolSource748, 5, "Shop", 102, 103, []uint16{22}, ext, "Store")
	if len(trade) != 260 || string(trade[234:239]) != "Store" {
		t.Fatalf("source CreateMobTrade layout invalid")
	}
	shop := ShopListForProtocol(ClientProtocolSource748, []model.Item{{Index: 4011}}, 3, ShopNormal)
	if len(shop) != 236 || binary.LittleEndian.Uint16(shop[16:18]) != 4011 || binary.LittleEndian.Uint32(shop[232:236]) != 3 {
		t.Fatalf("source ShopList layout invalid")
	}
	chat := MessageChatForProtocol(ClientProtocolSource748, 5, "hello")
	if len(chat) != 140 || string(chat[12:17]) != "hello" {
		t.Fatalf("source MessageChat layout invalid")
	}
}

func TestSourceScoreRefreshKeepsPendingCostsSeparateFromCurrentResources(t *testing.T) {
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Attack:  1234, MaxHP: 9000, CurHP: 8000, MaxMP: 7000, CurMP: 6000,
		MagicAmp: 65,
	}
	ch := model.Char{Extended: ext, LearnedSkill: 0xFFFFFFFF}
	b := SourceUpdateScore(7, ch)
	if len(b) != 152 || ParseHeader(b).Type != OpUpdateScore {
		t.Fatalf("source UpdateScore len/opcode = %d/%#x", len(b), ParseHeader(b).Type)
	}
	// TMProject aligns the two int fields after Resist[4]. They are pending
	// skill costs, not CurHP/CurMP, and must remain zero on a score refresh.
	if reqHP, reqMP := binary.LittleEndian.Uint32(b[136:140]), binary.LittleEndian.Uint32(b[140:144]); reqHP != 0 || reqMP != 0 {
		t.Fatalf("source pending costs = hp:%d mp:%d; want zero", reqHP, reqMP)
	}
	if got := binary.LittleEndian.Uint16(b[144:146]); got != 65 {
		t.Fatalf("source magic amp = %d; want 65", got)
	}
	// LearnedSkill is only an avatar-effect selector in 0x336. The complete
	// learned masks are sent by UpdateEtc, so a 32-bit mask must not leak here.
	if b[148] != 0 {
		t.Fatalf("source score avatar selector = %d; want zero", b[148])
	}
}

func TestSourceMobScoreUsesTheSourceScoreABI(t *testing.T) {
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Attack:  777, Defense: 333, MaxHP: 12000, CurHP: 11000, MagicAmp: 44,
	}
	resist := model.ElementalResists{Fire: 10, Ice: 20, Sacred: 30, Thunder: 40}
	b := SourceMobScore(1001, ext, nil, resist)
	if len(b) != 152 || ParseHeader(b).Type != OpUpdateScore {
		t.Fatalf("source mob score len/opcode = %d/%#x", len(b), ParseHeader(b).Type)
	}
	// STRUCT_SCORE begins at byte 12 and Damage/Attack is its third DWORD.
	if got := binary.LittleEndian.Uint32(b[20:24]); got != 777 {
		t.Fatalf("source mob attack = %d; want 777", got)
	}
	if got := [4]byte{b[130], b[131], b[132], b[133]}; got != [4]byte{10, 20, 30, 40} {
		t.Fatalf("source mob resists = %v", got)
	}
	if got := binary.LittleEndian.Uint16(b[144:146]); got != 44 {
		t.Fatalf("source mob magic amp = %d; want 44", got)
	}
}

func TestMobHpMpForProtocolKeepsSourceResourcesWide(t *testing.T) {
	b := MobHpMpForProtocol(ClientProtocolSource748, 1001, 750_000, 1_000_000, 250_000, 500_000)
	if len(b) != 36 || ParseHeader(b).Type != OpSetHpMp {
		t.Fatalf("source mob HP/MP len/opcode = %d/%#x", len(b), ParseHeader(b).Type)
	}
	// TMHuman selects these DWORDs only for the 36-byte source ABI; the WORD
	// prefix remains a proportional projection for stock-compatible bars.
	if got := binary.LittleEndian.Uint32(b[20:24]); got != 750_000 {
		t.Fatalf("source current HP = %d; want 750000", got)
	}
	if got := binary.LittleEndian.Uint32(b[28:32]); got != 1_000_000 {
		t.Fatalf("source max HP = %d; want 1000000", got)
	}

	stock := MobHpMpForProtocol(ClientProtocolStock748, 1001, 750_000, 1_000_000, 250_000, 500_000)
	if len(stock) != 20 {
		t.Fatalf("stock mob HP/MP len = %d; want 20", len(stock))
	}
}

func TestHpMpForProtocolUsesRecipientABI(t *testing.T) {
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		CurHP:   750_000,
		MaxHP:   1_000_000,
		CurMP:   250_000,
		MaxMP:   500_000,
	}
	source := HpMpForProtocol(ClientProtocolSource748, 1001, ext)
	if len(source) != 36 || binary.LittleEndian.Uint32(source[20:24]) != ext.CurHP {
		t.Fatalf("source player HP/MP packet = len %d hp %d; want 36/%d", len(source), binary.LittleEndian.Uint32(source[20:24]), ext.CurHP)
	}
	stock := HpMpForProtocol(ClientProtocolStock748, 1001, ext)
	if len(stock) != 20 {
		t.Fatalf("stock player HP/MP packet len = %d; want 20", len(stock))
	}
}

func TestSourceCharListMatchesTMProjectABI(t *testing.T) {
	ext := &model.ExtendedScore{Version: model.ExtendedScoreVersion, Level: 77, Attack: 1234, MaxHP: 4567, CurHP: 4000}
	char := model.Char{Name: "SourceHero", X: 2100, Y: 2101, Gold: 99, Exp: 123456, Extended: ext}
	char.Equip[0] = model.Item{Index: 22}
	var cargo [model.MaxCargo]model.Item
	cargo[3] = model.Item{Index: 4011}

	b := SourceCharList("account", []model.Char{char}, cargo[:], 54321)
	if len(b) != 1992 || ParseHeader(b).Type != SourceOpCharList {
		t.Fatalf("source char-list len/opcode = %d/%#x", len(b), ParseHeader(b).Type)
	}
	if got := binary.LittleEndian.Uint16(b[32:34]); got != 2100 {
		t.Fatalf("HomeTownX = %d", got)
	}
	if got := binary.LittleEndian.Uint32(b[32+80+8 : 32+80+12]); got != 1234 {
		t.Fatalf("score attack = %d", got)
	}
	if got := binary.LittleEndian.Uint64(b[32+872 : 32+880]); got != 123456 {
		t.Fatalf("EXP = %d", got)
	}
	if got := binary.LittleEndian.Uint16(b[936+3*8 : 936+3*8+2]); got != 4011 {
		t.Fatalf("cargo item = %d", got)
	}
	if got := binary.LittleEndian.Uint32(b[1960:1964]); got != 54321 {
		t.Fatalf("cargo coin = %d", got)
	}
}
