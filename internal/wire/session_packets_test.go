package wire

import (
	"bytes"
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestUpdateScore754LayoutIncludesSixteenAffects(t *testing.T) {
	ch := model.Char{GuildID: 0x1234, GuildRank: model.GuildRankLeader, Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, Level: 65,
		MaxHP: 900, CurHP: 900, MaxMP: 700, CurMP: 700, MagicAmp: 12,
		Critical: 3, SaveMana: 4, RegenHP: 5, RegenMP: 6,
		ResistFire: 7, ResistIce: 8, ResistHoly: 9, ResistThunder: 10,
	}}
	ch.Affects[0] = model.Affect{Type: 24, ExpiresAt: time.Now().Add(80 * time.Second)}
	b := UpdateScore(1, ch)
	if len(b) != 236 || b[40] != 3 || b[41] != 4 || b[42] != 10 || b[43] != 24 ||
		binary.LittleEndian.Uint16(b[74:76]) != 0x0234 ||
		binary.LittleEndian.Uint16(b[76:78]) != model.GuildRankLeader ||
		!bytes.Equal(b[78:82], []byte{7, 8, 9, 10}) ||
		binary.LittleEndian.Uint16(b[82:84]) != 900 || binary.LittleEndian.Uint16(b[84:86]) != 700 ||
		b[86] != 12 || binary.LittleEndian.Uint32(b[104:108]) != 900 ||
		binary.LittleEndian.Uint32(b[112:116]) != 900 || binary.LittleEndian.Uint32(b[116:120]) != 700 {
		t.Fatalf("p754 SendScore incorreto: % X", b[40:92])
	}
}

func TestUpdateAffects748FullLayout(t *testing.T) {
	ch := model.Char{}
	ch.Affects[0] = model.Affect{Type: 24, Value: 7, Level: 40,
		ExpiresAt: time.Now().Add(80 * time.Second)}
	ch.Affects[15] = model.Affect{Type: 31, ClientType: 24, Value: 150, Level: 55,
		ExpiresAt: time.Now().Add(40 * time.Second)}
	b := UpdateAffects(9, ch)
	last := 12 + 15*8
	if len(b) != 140 || ParseHeader(b).Type != OpUpdateAffect || ParseHeader(b).ID != 9 ||
		b[12] != 24 || b[13] != 7 || binary.LittleEndian.Uint16(b[14:16]) != 40 ||
		binary.LittleEndian.Uint32(b[16:20]) != 10 ||
		b[last] != 24 || b[last+1] != 150 || binary.LittleEndian.Uint16(b[last+2:last+4]) != 55 ||
		binary.LittleEndian.Uint32(b[last+4:last+8]) != 5 {
		t.Fatalf("0x3B9 incorreto: first=% X last=% X", b[12:20], b[last:last+8])
	}
}

func TestCreateMobCarriesVisualAffects(t *testing.T) {
	affects := [16]model.Affect{}
	affects[0] = model.Affect{Type: 31, ClientType: 24, ExpiresAt: time.Now().Add(80 * time.Second)}
	b := CreateMob(77, "Buffed", 2200, 2100, nil, model.WireScore{CurHP: 100}, affects[:], 2)
	if len(b) != 176 || b[66] != 10 || b[67] != 24 {
		t.Fatalf("CreateMob sem Affect[0] visual: % X", b[66:70])
	}
}

func TestCreateMobCarriesAncientCodes(t *testing.T) {
	anct := []byte{0x20, 116, 0x40}
	b := CreateMobVisual(77, "Refinado", 2200, 2100, nil, anct,
		model.WireScore{CurHP: 100}, nil, 0)
	if !bytes.Equal(b[130:133], anct) {
		t.Fatalf("CreateMob perdeu AnctCode: % X", b[130:146])
	}
}

func TestMobScoreCarriesAndClearsVisualAffects(t *testing.T) {
	affects := [16]model.Affect{}
	affects[2] = model.Affect{Type: 1, ExpiresAt: time.Now().Add(16 * time.Second)}
	b := MobScore(1000, model.WireScore{CurHP: 90, CurMP: 20}, affects[:],
		model.ElementalResists{Fire: 7, Ice: 8, Sacred: 9, Thunder: 10})
	if len(b) != 92 || b[46] != 2 || b[47] != 1 ||
		!bytes.Equal(b[78:82], []byte{7, 8, 9, 10}) ||
		binary.LittleEndian.Uint16(b[82:84]) != 90 {
		t.Fatalf("MobScore visual incorreto: % X", b[42:86])
	}

	cleared := MobScore(1000, model.WireScore{CurHP: 90}, nil, model.ElementalResists{})
	if !bytes.Equal(cleared[42:74], make([]byte, 32)) {
		t.Fatalf("MobScore nao limpou affects: % X", cleared[42:74])
	}
}

func TestCriticalArmorUses748VisualSlot(t *testing.T) {
	if got := clientAffectType(model.Affect{Type: 31, ClientType: 24}); got != 24 {
		t.Fatalf("affect 31 foi enviado como %d, quer visual 24 do client 7.48", got)
	}
	if got := clientAffectType(model.Affect{Type: 31}); got != 31 {
		t.Fatalf("Coin Armor HT foi enviada como %d, quer 31", got)
	}
}

func TestSetShortSkillLayout(t *testing.T) {
	var skills [20]byte
	for i := range skills {
		skills[i] = byte(i + 1)
	}
	b := SetShortSkill(7, skills)
	if len(b) != 32 || ParseHeader(b).Type != OpSetShortSkill {
		t.Fatalf("header invalido: len=%d type=0x%X", len(b), ParseHeader(b).Type)
	}
	for i := range skills {
		if b[12+i] != skills[i] {
			t.Fatalf("skill[%d]=%d, esperado %d", i, b[12+i], skills[i])
		}
	}
}

func TestTrade748Layout(t *testing.T) {
	var items [15]model.Item
	var positions [15]int8
	for i := range positions {
		positions[i] = -1
	}
	items[0] = model.Item{Index: 4011, Eff: [6]byte{43, 9, 2, 20}}
	positions[0] = 7

	b := Trade(2, items, positions, 123456, true, 1)
	if len(b) != 156 || ParseHeader(b).Type != OpTrade || ParseHeader(b).ID != 2 ||
		binary.LittleEndian.Uint16(b[12:14]) != 4011 ||
		!bytes.Equal(b[14:20], items[0].Eff[:]) ||
		b[132] != 7 || b[133] != 0xFF ||
		binary.LittleEndian.Uint32(b[148:152]) != 123456 ||
		b[152] != 1 || binary.LittleEndian.Uint16(b[154:156]) != 1 {
		t.Fatalf("MSG_Trade 7.48 invalida: % X", b)
	}
	if close := CloseTrade(2); len(close) != 12 || ParseHeader(close).Type != OpCloseTrade || ParseHeader(close).ID != 2 {
		t.Fatalf("CloseTrade invalido: % X", close)
	}
	if check := CNFTradeCheck(2); len(check) != 12 || ParseHeader(check).Type != OpCNFTradeCheck || ParseHeader(check).ID != 2 {
		t.Fatalf("CNFTradeCheck invalido: % X", check)
	}
}

func TestGhostShop748Layouts(t *testing.T) {
	var items [12]model.Item
	var positions [12]int8
	var prices [12]uint32
	for i := range positions {
		positions[i] = -1
	}
	items[0] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	positions[0], prices[0] = 7, 123456
	list := AutoTrade("Minha Loja", items, positions, prices, 0, 9)
	if len(list) != 196 || ParseHeader(list).Type != OpAutoTrade ||
		string(list[12:22]) != "Minha Loja" ||
		binary.LittleEndian.Uint16(list[36:38]) != 4011 || list[132] != 7 ||
		binary.LittleEndian.Uint32(list[144:148]) != 123456 ||
		binary.LittleEndian.Uint16(list[194:196]) != 9 {
		t.Fatalf("MSG_AutoTrade 7.48 invalida: % X", list)
	}

	create := CreateMobTrade(25009, "Felipe", 2112, 2088, nil,
		model.WireScore{CurHP: 100}, "Minha Loja")
	if len(create) != 200 || ParseHeader(create).Type != OpCreateMobTrade ||
		binary.LittleEndian.Uint16(create[16:18]) != 25009 ||
		string(create[172:182]) != "Minha Loja" {
		t.Fatalf("MSG_CreateMobTrade 7.48 invalida: % X", create)
	}
	sold := ItemSold(9, 3)
	if len(sold) != 20 || ParseHeader(sold).Type != OpItemSold ||
		binary.LittleEndian.Uint32(sold[12:16]) != 9 ||
		binary.LittleEndian.Uint32(sold[16:20]) != 3 {
		t.Fatalf("MSG_ItemSold invalida: % X", sold)
	}
}

func TestMobMove748Layout(t *testing.T) {
	b := MobMove(1000, 2200, 2100, 2201, 2101, 2)
	if len(b) != 52 || ParseHeader(b).Type != OpAction || ParseHeader(b).ID != 1000 {
		t.Fatalf("header invalido: %+v", ParseHeader(b))
	}
	if binary.LittleEndian.Uint32(b[16:20]) != 2 || binary.LittleEndian.Uint32(b[20:24]) != 0 ||
		binary.LittleEndian.Uint16(b[24:26]) != 2201 || binary.LittleEndian.Uint16(b[26:28]) != 2101 || b[28] != 0 {
		t.Fatalf("movimento invalido: %v", b[12:29])
	}
}

func TestMobMove748CapsMovementSpeedAtFour(t *testing.T) {
	b := MobMove(1000, 2200, 2100, 2208, 2100, 15)
	if got := binary.LittleEndian.Uint32(b[16:20]); got != 4 {
		t.Fatalf("speed enviada=%d, quer 4", got)
	}
}

func TestAttackHit748CompactLayout(t *testing.T) {
	b := AttackHit(1, 1002, 2200, 2100, 2201, 2101, 357, 0, 1234, 80)
	if len(b) != 48 || ParseHeader(b).Type != OpAttackOne {
		t.Fatalf("header invalido: %+v len=%d", ParseHeader(b), len(b))
	}
	if binary.LittleEndian.Uint16(b[12:14]) != 1 || binary.LittleEndian.Uint16(b[20:22]) != 2201 || b[28] != 5 || b[31] != 0 ||
		binary.LittleEndian.Uint32(b[32:36]) != 1234 || binary.LittleEndian.Uint16(b[36:38]) != 80 ||
		binary.LittleEndian.Uint32(b[40:44]) != 0 || binary.LittleEndian.Uint16(b[44:46]) != 1002 || binary.LittleEndian.Uint16(b[46:48]) != 357 {
		t.Fatalf("resultado de ataque invalido: %v", b)
	}
}

func TestExtendedScoreVitalsUseSignedCompatibilityPrefix(t *testing.T) {
	ext := &model.ExtendedScore{
		Attack: 150_000, MagicAttack: 175_000, Defense: 125_000,
		MaxHP: 250_000, MaxMP: 200_000, CurHP: 225_000, CurMP: 180_000,
		Str: 110_000, Int: 120_000, Dex: 130_000, Con: 140_000,
	}
	ch := model.Char{
		Extended: ext,
	}
	compat := ext.CompatibilityScore()
	score := UpdateScore(7, ch)
	if len(score) != 236 || binary.LittleEndian.Uint32(score[92:96]) != ext.MagicAttack ||
		binary.LittleEndian.Uint32(score[96:100]) != ext.Attack ||
		binary.LittleEndian.Uint32(score[100:104]) != ext.Defense ||
		binary.LittleEndian.Uint32(score[112:116]) != ext.CurHP ||
		binary.LittleEndian.Uint16(score[20:22]) != compat.MaxHP ||
		binary.LittleEndian.Uint16(score[26:28]) != compat.CurMP ||
		binary.LittleEndian.Uint16(score[84:86]) != compat.CurMP ||
		binary.LittleEndian.Uint32(score[228:232]) != model.ExtendedScoreVersion ||
		binary.LittleEndian.Uint32(score[232:236]) != 0x32435358 {
		t.Fatalf("cauda extended do score incorreta: len=%d tail=% X", len(score), score[92:])
	}

	vitals := SetHpMpExtended(7, ext)
	if len(vitals) != 36 || binary.LittleEndian.Uint16(vitals[12:14]) != compat.CurHP ||
		binary.LittleEndian.Uint16(vitals[14:16]) != compat.CurMP ||
		binary.LittleEndian.Uint32(vitals[20:24]) != ext.CurHP ||
		binary.LittleEndian.Uint32(vitals[32:36]) != ext.MaxMP {
		t.Fatalf("vitais extended incorretos: % X", vitals)
	}

	// O WORD leva o dano PROJETADO na escala do alvo (MaxHP 300k -> escala 10),
	// porque o client o subtrai do CurHP do prefixo nativo, que ja esta
	// escalado. A cauda uint32 leva o dano REAL, que alimenta o numero na tela.
	hit := AttackHitExtended(7, 1000, 1, 2, 3, 4, 150_000, 300_000, 0, 0)
	if len(hit) != 52 || binary.LittleEndian.Uint16(hit[46:48]) != 15_000 ||
		binary.LittleEndian.Uint32(hit[48:52]) != 150_000 {
		t.Fatalf("dano extended incorreto: % X", hit)
	}
}

func TestExtendedScoreV2TailAlignment(t *testing.T) {
	e := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Level:   101, Mastery: [4]uint32{102, 103, 104, 105},
		Critical: 106, Parry: 107, Range: 108,
		ResistFire: 109, ResistIce: 110, ResistHoly: 111, ResistThunder: 112,
		SaveMana: 113, MagicAmp: 114, RegenHP: 115, RegenMP: 116,
		AttackRun: 117, Merchant: 118,
	}
	packet := UpdateScore(7, model.Char{Extended: e})
	checks := map[int]uint32{
		156: 101, 160: 102, 164: 103, 168: 104, 172: 105,
		176: 106, 180: 107, 184: 108,
		188: 109, 192: 110, 196: 111, 200: 112,
		204: 113, 208: 114, 212: 115, 216: 116,
		220: 117, 224: 118, 228: model.ExtendedScoreVersion,
		232: 0x32435358,
	}
	if len(packet) != 236 {
		t.Fatalf("tamanho XSC2=%d", len(packet))
	}
	for offset, want := range checks {
		if got := binary.LittleEndian.Uint32(packet[offset : offset+4]); got != want {
			t.Fatalf("XSC2 @%d=%d, quer %d", offset, got, want)
		}
	}
}

func TestLegacyVitalsAlsoRefreshWideSidecar(t *testing.T) {
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		MaxHP:   321, MaxMP: 654, CurHP: 123, CurMP: 456,
	}
	b := SetHpMpExtended(7, ext)
	if len(b) != 36 || binary.LittleEndian.Uint16(b[12:14]) != 123 ||
		binary.LittleEndian.Uint32(b[20:24]) != 123 ||
		binary.LittleEndian.Uint32(b[24:28]) != 456 ||
		binary.LittleEndian.Uint32(b[28:32]) != 321 ||
		binary.LittleEndian.Uint32(b[32:36]) != 654 {
		t.Fatalf("vitais legados nao preencheram sidecar: % X", b)
	}
}

func TestSkillHit748CompactLayout(t *testing.T) {
	b := SkillHit(1, 1002, 2200, 2100, 2201, 2101, 88, 0, 1234, 500_070, 3, 9, 42)
	if len(b) != 48 || ParseHeader(b).Type != OpAttackOne ||
		int16(binary.LittleEndian.Uint16(b[24:26])) != 3 || b[28] != 9 || b[29] != 42 ||
		b[30] != 1 || binary.LittleEndian.Uint16(b[26:28]) != 30_000 ||
		binary.LittleEndian.Uint16(b[36:38]) != 30_000 ||
		binary.LittleEndian.Uint16(b[44:46]) != 1002 || binary.LittleEndian.Uint16(b[46:48]) != 88 {
		t.Fatalf("SkillHit invalido: %v", b)
	}
}

func TestMessageWhisperMail748Layout(t *testing.T) {
	b := MessageWhisper(1001, "Gremlin", "!Voce foi morto por mob.", 0)
	if len(b) != 128 || ParseHeader(b).Type != OpMessageWhisper ||
		ParseHeader(b).ID != 1001 || string(b[12:19]) != "Gremlin" || b[28] != '!' {
		t.Fatalf("carta nativa invalida: % X", b)
	}
}

func TestMessageWhisperKeepsNativeSlashCommandBody(t *testing.T) {
	b := MessageWhisper(7, "felipe", "mensagem privada", 0)
	if ParseHeader(b).ID != 7 || string(b[12:18]) != "felipe" ||
		string(b[28:44]) != "mensagem privada" || b[28] == '@' {
		t.Fatalf("whisper /nick foi alterado: % X", b)
	}
}

func TestSkillHits748MultiLayout(t *testing.T) {
	targets := []SkillTarget{{ID: 1001, Damage: 40}, {ID: 1002, Damage: 55}}
	b := SkillHits(1, 2200, 2100, 2201, 2101, 1234, 70, 0, 9, 42, 13, targets)
	if len(b) != 96 || ParseHeader(b).Type != 0x36C ||
		binary.LittleEndian.Uint16(b[14:16]) != 2 ||
		binary.LittleEndian.Uint16(b[44:46]) != 1001 || binary.LittleEndian.Uint16(b[46:48]) != 40 ||
		binary.LittleEndian.Uint16(b[48:50]) != 1002 || binary.LittleEndian.Uint16(b[50:52]) != 55 {
		t.Fatalf("SkillHits multi invalido: %v", b)
	}
}

func TestSkillHitExtendedKeepsSkillAndWideDamage(t *testing.T) {
	// Alvo com MaxHP 550k -> escala 19; 275_000/19 = 14474 (arredondado p/ cima).
	b := SkillHitExtended(1, 1001, 2200, 2100, 2201, 2101,
		275_000, 550_000, 1234, 70, 7, 0, 255)
	if len(b) != 52 || ParseHeader(b).Size != 52 ||
		int16(binary.LittleEndian.Uint16(b[24:26])) != 7 ||
		binary.LittleEndian.Uint16(b[46:48]) != 14_474 ||
		binary.LittleEndian.Uint32(b[48:52]) != 275_000 || b[30] != 0 {
		t.Fatalf("SkillHitExtended invalido: %v", b)
	}
}

func TestPhysicalAttackKeepsFloatingDamagePath(t *testing.T) {
	b := AttackHitExtended(1, 1001, 2200, 2100, 2201, 2101, 275_000, 550_000, 1234, 70)
	if len(b) != 52 || b[30] != 0 || binary.LittleEndian.Uint32(b[48:52]) != 275_000 {
		t.Fatalf("AttackHitExtended sem visual/full damage: % X", b)
	}
}

func TestSkillShopListLayout(t *testing.T) {
	b := ShopList([]model.Item{{Index: 5000}, {Index: 5001}}, 0, ShopSkill)
	if len(b) != 532 || binary.LittleEndian.Uint32(b[12:16]) != ShopSkill ||
		binary.LittleEndian.Uint16(b[16:18]) != 5000 || binary.LittleEndian.Uint16(b[24:26]) != 5001 {
		t.Fatalf("loja de skills invalida")
	}
}

func TestRepurchaseList748Layout(t *testing.T) {
	var entries [10]RepurchaseEntry
	entries[0] = RepurchaseEntry{Order: 7, Item: model.Item{Index: 400, Eff: [6]byte{61, 3}}, Price: 1200}
	b := RepurchaseList(9, 1100, entries)
	if len(b) != 176 || ParseHeader(b).Type != OpRebuy || ParseHeader(b).ID != 9 ||
		binary.LittleEndian.Uint32(b[12:16]) != 1100 ||
		binary.LittleEndian.Uint32(b[16:20]) != 7 ||
		binary.LittleEndian.Uint16(b[20:22]) != 400 || b[22] != 61 || b[23] != 3 ||
		binary.LittleEndian.Uint32(b[28:32]) != 1200 {
		t.Fatalf("MSG_RepurchaseItems 7.48 invalido: % X", b[:40])
	}
}

func TestGroundItemPacketLayouts(t *testing.T) {
	it := model.Item{Index: 4011, Eff: [6]byte{43, 9, 2, 20}}
	drop := CNFDropItem(1, 12, 3, 2200, 2100)
	if len(drop) != 28 || ParseHeader(drop).Type != OpCNFDropItem || ParseHeader(drop).ID != 0 ||
		binary.LittleEndian.Uint32(drop[12:16]) != 1 ||
		binary.LittleEndian.Uint32(drop[16:20]) != 12 ||
		binary.LittleEndian.Uint32(drop[20:24]) != 3 ||
		binary.LittleEndian.Uint16(drop[24:26]) != 2200 ||
		binary.LittleEndian.Uint16(drop[26:28]) != 2100 {
		t.Fatalf("CNFDropItem invalido: %v", drop)
	}

	get := CNFGetItem(1, 12)
	if len(get) != 28 || ParseHeader(get).Type != OpCNFGetItem || ParseHeader(get).ID != SceneField ||
		binary.LittleEndian.Uint32(get[12:16]) != 1 ||
		binary.LittleEndian.Uint32(get[16:20]) != 12 ||
		!bytes.Equal(get[20:28], make([]byte, 8)) {
		t.Fatalf("CNFGetItem invalido: %v", get)
	}

	create := CreateItem(2200, 2100, 10000, it, 1, 2, 3, 4, 7)
	if len(create) != 32 || ParseHeader(create).Type != OpCreateItem || ParseHeader(create).ID != SceneField ||
		binary.LittleEndian.Uint16(create[12:14]) != 2200 ||
		binary.LittleEndian.Uint16(create[14:16]) != 2100 ||
		binary.LittleEndian.Uint16(create[16:18]) != 10000 ||
		binary.LittleEndian.Uint16(create[18:20]) != it.Index ||
		create[26] != 1 || create[27] != 2 || create[28] != 3 || create[29] != 4 ||
		binary.LittleEndian.Uint16(create[30:32]) != 7 {
		t.Fatalf("CreateItem invalido: %v", create)
	}

	remove := RemoveItem(10000)
	if len(remove) != 16 || ParseHeader(remove).Type != OpRemoveItem ||
		binary.LittleEndian.Uint32(remove[12:16]) != 10000 {
		t.Fatalf("RemoveItem invalido: %v", remove)
	}
}

func TestCNFMobKill748Layout(t *testing.T) {
	b := CNFMobKill(1003, 1, 9000)
	if len(b) != 24 || ParseHeader(b).Type != OpCNFMobKill || ParseHeader(b).ID != SceneField ||
		binary.LittleEndian.Uint32(b[12:16]) != 0 || binary.LittleEndian.Uint16(b[16:18]) != 1003 ||
		binary.LittleEndian.Uint16(b[18:20]) != 1 || binary.LittleEndian.Uint32(b[20:24]) != 9000 {
		t.Fatalf("CNFMobKill invalido: %v", b)
	}
}

func TestRemoveMobTypeThreeRematerializesDeadPlayer(t *testing.T) {
	b := RemoveMob(9, 3)
	if len(b) != 16 || ParseHeader(b).Type != OpRemoveMob || ParseHeader(b).ID != 9 ||
		binary.LittleEndian.Uint32(b[12:16]) != 3 {
		t.Fatalf("RemoveMob de rematerializacao invalido: % X", b)
	}
}

func TestPlayerMove748NormalizesForRemoteClient(t *testing.T) {
	route := []byte{'6', '3', '6', '6', 0, '9'}
	b := PlayerMove(7, 2100, 2101, 2104, 2105, 4, route)
	if len(b) != 52 || ParseHeader(b).Type != OpAction || ParseHeader(b).ID != 7 ||
		binary.LittleEndian.Uint16(b[12:14]) != 2100 ||
		binary.LittleEndian.Uint32(b[16:20]) != 4 || binary.LittleEndian.Uint32(b[20:24]) != 0 ||
		binary.LittleEndian.Uint16(b[24:26]) != 2104 || string(b[28:32]) != "6366" ||
		b[32] != 0 || b[33] != 0 {
		t.Fatalf("movimento 7.48 invalido: %v", b)
	}
}

func TestPlayerMove748ClampsServerSpeed(t *testing.T) {
	if got := binary.LittleEndian.Uint32(PlayerMove(7, 2100, 2100, 2101, 2100, 0, nil)[16:20]); got != 1 {
		t.Fatalf("velocidade minima=%d, quer 1", got)
	}
	if got := binary.LittleEndian.Uint32(PlayerMove(7, 2100, 2100, 2101, 2100, 15, nil)[16:20]); got != 6 {
		t.Fatalf("velocidade maxima=%d, quer 6", got)
	}
}

func TestIllusionMoveUsesEffectSix(t *testing.T) {
	b := IllusionMove(7, 2100, 2100, 2106, 2104, 4)
	if ParseHeader(b).Type != OpIllusion || binary.LittleEndian.Uint32(b[16:20]) != 4 ||
		binary.LittleEndian.Uint32(b[20:24]) != 6 || binary.LittleEndian.Uint16(b[24:26]) != 2106 {
		t.Fatalf("Illusion MSG_Action2 invalida: % X", b)
	}
}

func TestUpdateEtc748Layout(t *testing.T) {
	// Layout p754_SendEtc: Hold zerado DWORD@12, exp@16, LearnedSkill@20 (OBRIGATORIO:
	// e daqui que o client aprende as skills), statusPts@24, masterPts@26,
	// skillPts@28, magic@30, gold@32.
	ch := model.Char{CP: -25, Exp: 34000, LearnedSkill: 1 << 3, NextExp: 649715, Gold: 99424,
		Extended: &model.ExtendedScore{StatusPts: 7, MasterPts: 100, SkillPts: 150, MagicAmp: 70}}
	b := UpdateEtc(1, ch)
	if len(b) != 48 || ParseHeader(b).Type != OpUpdateEtc ||
		binary.LittleEndian.Uint32(b[12:16]) != 0 || binary.LittleEndian.Uint32(b[16:20]) != 34000 ||
		binary.LittleEndian.Uint32(b[20:24]) != 1<<3 || binary.LittleEndian.Uint16(b[24:26]) != 7 ||
		binary.LittleEndian.Uint16(b[26:28]) != 100 || binary.LittleEndian.Uint16(b[28:30]) != 150 ||
		binary.LittleEndian.Uint16(b[30:32]) != 70 ||
		binary.LittleEndian.Uint32(b[32:36]) != 99424 {
		t.Fatalf("UpdateEtc 7.48 invalido: %v", b)
	}
}

func TestUpdateEtcDoesNotLeakChaosIntoHold(t *testing.T) {
	for _, cp := range []int16{-75, -1, 0, 1, 75} {
		b := UpdateEtc(1, model.Char{CP: cp})
		if got := binary.LittleEndian.Uint32(b[12:16]); got != 0 {
			t.Fatalf("CP=%d vazou para Hold: %d", cp, got)
		}
	}
}

func TestCreateMobProjectsSignedChaosToNativeByte(t *testing.T) {
	for _, tc := range []struct {
		cp   int16
		want byte
	}{
		{-75, 0}, {0, 75}, {75, 150},
	} {
		b := CreateMobExtendedWithGuildRank(7, "Player", 2100, 2100, nil, nil,
			&model.ExtendedScore{Version: model.ExtendedScoreVersion}, nil, 2, 0, 0, tc.cp)
		if got := b[30]; got != tc.want {
			t.Fatalf("CP=%d byte=%d, esperado %d", tc.cp, got, tc.want)
		}
	}
}

func TestCreateMobCarriesGuildRankInSpawnWord(t *testing.T) {
	b := CreateMobExtendedWithGuildRank(7, "Guilded", 2100, 2100, nil, nil,
		&model.ExtendedScore{Version: model.ExtendedScoreVersion}, nil, 2, 0x1234,
		model.GuildRankSubFirst, 0)
	if got := binary.LittleEndian.Uint16(b[98:100]); got != 0x0234 {
		t.Fatalf("guild wire=%#x, esperado id de 12 bits", got)
	}
	if b[128] != 2 || b[129] != model.GuildRankSubFirst {
		t.Fatalf("spawn/guild level=%d/%d", b[128], b[129])
	}
}

func TestCharListCarriesGuildIndexInSelection(t *testing.T) {
	b := CharList("account", []model.Char{{Name: "Guilded", GuildID: 0x1234}}, nil, 0)
	if got := binary.LittleEndian.Uint16(b[12+704 : 12+706]); got != 0x0234 {
		t.Fatalf("Guild no SelectChar=%#x, esperado id de 12 bits", got)
	}
}

func TestEnterWorldProjectsSignedChaosToNativeByte(t *testing.T) {
	for _, tc := range []struct {
		cp   int16
		want byte
	}{
		{-75, 0}, {0, 75}, {75, 150},
	} {
		b := EnterWorld(7, model.Char{
			Name: "Player", CP: tc.cp,
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion},
		})
		if got := b[16+12]; got != tc.want {
			t.Fatalf("CP=%d byte=%d, esperado %d", tc.cp, got, tc.want)
		}
	}
}

func TestMessageChat748Layout(t *testing.T) {
	b := MessageChat(7, "Inventario limpo")
	if len(b) != 108 || ParseHeader(b).Type != OpMessageChat || ParseHeader(b).ID != 7 {
		t.Fatalf("MessageChat header/layout invalido: len=%d header=%+v", len(b), ParseHeader(b))
	}
	if got := string(b[12 : 12+16]); got != "Inventario limpo" {
		t.Fatalf("MessageChat texto=%q", got)
	}
	if b[107] != 0 {
		t.Fatal("MessageChat sem terminador final")
	}
}

func TestMessagePanel748Layout(t *testing.T) {
	b := MessagePanel("Inventario limpo")
	if len(b) != 108 || ParseHeader(b).Type != OpMessagePanel || ParseHeader(b).ID != 0 {
		t.Fatalf("MessagePanel header/layout invalido: len=%d header=%+v", len(b), ParseHeader(b))
	}
	if got := string(b[12 : 12+16]); got != "Inventario limpo" {
		t.Fatalf("MessagePanel texto=%q", got)
	}
	if b[107] != 0 {
		t.Fatal("MessagePanel sem terminador final")
	}
}

func TestDaySync748HiddenCalendarLayout(t *testing.T) {
	b := DaySync()
	if len(b) != 108 || ParseHeader(b).Type != OpMessagePanel || ParseHeader(b).ID != 0 {
		t.Fatalf("DaySync header/layout invalido: len=%d header=%+v", len(b), ParseHeader(b))
	}
	if got := string(b[12:20]); got != "!#11  2\x00" {
		t.Fatalf("DaySync payload=%q", got)
	}
}

func TestCharacterListsWriteAllFourSlots(t *testing.T) {
	chars := make([]model.Char, 4)
	for i := range chars {
		chars[i] = model.Char{
			Name:     []string{"TKTeste", "FMTeste", "BMTeste", "HTTeste"}[i],
			X:        uint16(2112 + i),
			Y:        uint16(2088 + i),
			Gold:     uint32(100 + i),
			Exp:      uint32(1000 + i),
			Extended: &model.ExtendedScore{Level: uint32(i)},
		}
		chars[i].Equip[0].Index = uint16(1 + i*10)
	}
	var cargo [model.MaxCargo]model.Item
	cargo[7] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	b := CharList("conta", chars, cargo[:], 123456)
	if len(b) != 1800 || ParseHeader(b).Type != OpCharList {
		t.Fatalf("char-list invalida: len=%d type=%X", len(b), ParseHeader(b).Type)
	}
	for slot, ch := range chars {
		if got := binary.LittleEndian.Uint16(b[12+slot*2:]); got != ch.X {
			t.Fatalf("slot %d X=%d, quer %d", slot, got, ch.X)
		}
		if got := cStringForTest(b[12+16+slot*16 : 12+16+(slot+1)*16]); got != ch.Name {
			t.Fatalf("slot %d nome=%q, quer %q", slot, got, ch.Name)
		}
		if got := binary.LittleEndian.Uint16(b[12+192+(slot*16)*8:]); got != ch.Equip[0].Index {
			t.Fatalf("slot %d rosto=%d, quer %d", slot, got, ch.Equip[0].Index)
		}
	}
	if got := binary.LittleEndian.Uint16(b[756+7*8:]); got != 4011 {
		t.Fatalf("cargo[7]=%d, quer 4011", got)
	}
	if got := binary.LittleEndian.Uint32(b[1780:1784]); got != 123456 {
		t.Fatalf("cargo gold=%d, quer 123456", got)
	}
	cnf := CNFNewCharacter(7, chars)
	if len(cnf) != 756 || ParseHeader(cnf).Type != OpCNFNewCharacter || ParseHeader(cnf).ID != 7 {
		t.Fatalf("confirmacao de criacao invalida: len=%d header=%+v", len(cnf), ParseHeader(cnf))
	}
}

func cStringForTest(b []byte) string {
	for i, c := range b {
		if c == 0 {
			return string(b[:i])
		}
	}
	return string(b)
}

func TestEnterWorldWritesAuthoritativeMobTail(t *testing.T) {
	short := [20]byte{3, 7, 11, 15}
	ch := model.Char{
		LearnedSkill: 0x00FFFFFF,
		GuildID:      12,
		GuildRank:    model.GuildRankLeader,
		ShortSkill:   short,
		Extended: &model.ExtendedScore{
			StatusPts: 115, MasterPts: 44, SkillPts: 192, MagicAmp: 70,
			Critical: 9, SaveMana: 10, RegenHP: 11, RegenMP: 12,
			ResistFire: 13, ResistIce: 14, ResistHoly: 15, ResistThunder: 16,
		},
	}
	b := EnterWorld(1, ch)
	const tail = 16 + 732
	if len(b) != 788 ||
		binary.LittleEndian.Uint32(b[tail:tail+4]) != 0x00FFFFFF ||
		binary.LittleEndian.Uint16(b[tail+4:tail+6]) != 115 ||
		binary.LittleEndian.Uint16(b[tail+6:tail+8]) != 44 ||
		binary.LittleEndian.Uint16(b[tail+8:tail+10]) != 192 ||
		b[tail+10] != 9 || b[tail+11] != 10 ||
		!bytes.Equal(b[tail+12:tail+16], short[:4]) ||
		b[tail+17] != 70 || b[tail+18] != 11 || b[tail+19] != 12 ||
		!bytes.Equal(b[tail+20:tail+24], []byte{13, 14, 15, 16}) {
		t.Fatalf("cauda do STRUCT_MOB ausente/incorreta: % X", b[tail:tail+24])
	}
	if b[tail+16] != model.GuildRankLeader {
		t.Fatalf("GuildMemberType=%d, esperado %d", b[tail+16], model.GuildRankLeader)
	}
}

func TestUpdateEtcExtendedWritesWidePointsTail(t *testing.T) {
	ch := model.Char{
		Extended: &model.ExtendedScore{
			StatusPts: 100000,
			MasterPts: 110000,
			SkillPts:  120000,
		},
	}
	b := UpdateEtc(7, ch)
	if len(b) != 48 || ParseHeader(b).Type != OpUpdateEtc {
		t.Fatalf("UpdateEtc extended invalido: len=%d header=%+v", len(b), ParseHeader(b))
	}
	if got := binary.LittleEndian.Uint32(b[36:40]); got != 100000 {
		t.Fatalf("status wide=%d", got)
	}
	if got := binary.LittleEndian.Uint32(b[40:44]); got != 110000 {
		t.Fatalf("mastery wide=%d", got)
	}
	if got := binary.LittleEndian.Uint32(b[44:48]); got != 120000 {
		t.Fatalf("skill wide=%d", got)
	}
}

func TestSetMobHpMpProjectsWideResourcesProportionally(t *testing.T) {
	b := SetMobHpMp(1000, 750_000, 1_000_000, 250_000, 500_000)
	if len(b) != 20 || ParseHeader(b).Type != OpSetHpMp {
		t.Fatalf("SetMobHpMp invalido: len=%d header=%+v", len(b), ParseHeader(b))
	}
	curHP := binary.LittleEndian.Uint16(b[12:14])
	curMP := binary.LittleEndian.Uint16(b[14:16])
	maxHP := binary.LittleEndian.Uint16(b[16:18])
	maxMP := binary.LittleEndian.Uint16(b[18:20])
	if maxHP > 30_000 || maxMP > 30_000 ||
		uint32(curHP)*4 < uint32(maxHP)*3-1 || uint32(curHP)*4 > uint32(maxHP)*3+1 ||
		uint32(curMP)*2 < uint32(maxMP)-1 || uint32(curMP)*2 > uint32(maxMP)+1 {
		t.Fatalf("projecao wide do mob incorreta: hp=%d/%d mp=%d/%d", curHP, maxHP, curMP, maxMP)
	}
}

func TestSysQuitLayout(t *testing.T) {
	b := SysQuit(9)
	if len(b) != 16 || ParseHeader(b).Type != OpSysQuit || ParseHeader(b).ID != 9 {
		t.Fatalf("SysQuit invalido: %+v len=%d", ParseHeader(b), len(b))
	}
}

func TestMotion748Layout(t *testing.T) {
	b := Motion(9, 100, 5)
	if len(b) != 20 || ParseHeader(b).Type != OpMotion || ParseHeader(b).ID != 9 {
		t.Fatalf("Motion invalido: %+v len=%d", ParseHeader(b), len(b))
	}
	if got := binary.LittleEndian.Uint16(b[12:14]); got != 100 {
		t.Fatalf("motion=%d", got)
	}
	if got := binary.LittleEndian.Uint16(b[14:16]); got != 5 {
		t.Fatalf("parm=%d", got)
	}
	if got := binary.LittleEndian.Uint32(b[16:20]); got != 0 {
		t.Fatalf("NotUsed=%d", got)
	}
}

func TestSelfEquipCarriesTintAnctCode(t *testing.T) {
	equip := make([]model.Item, 16)
	equip[6] = model.Item{Index: 700, Eff: [6]byte{116, 4}}
	b := SelfEquip(9, equip)
	if len(b) != 60 || ParseHeader(b).Type != OpUpdateEquip {
		t.Fatalf("UpdateEquip invalido: len=%d header=%+v", len(b), ParseHeader(b))
	}
	if got := binary.LittleEndian.Uint16(b[12+6*2:]); got != model.VisualItemCode(equip[6], false) {
		t.Fatalf("ItemEff[6]=%d", got)
	}
	if got := b[44+6]; got != model.AncientCode(equip[6]) {
		t.Fatalf("AnctCode[6]=%d, esperado %d", got, model.AncientCode(equip[6]))
	}
}
