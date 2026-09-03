package wire

import (
	"bytes"
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestUpdateScoreCanonicalLayoutIncludesScoreAndAffects(t *testing.T) {
	ch := model.Char{GuildID: 0x1234, GuildRank: model.GuildRankLeader, Score: &model.Score{
		Version: model.ScoreVersion, Level: 65,
		MaxHP: 900, CurHP: 900, MaxMP: 700, CurMP: 700, MagicAmp: 12,
		Critical: 3, SaveMana: 4, RegenHP: 5, RegenMP: 6,
		ResistFire: 7, ResistIce: 8, ResistHoly: 9, ResistThunder: 10,
	}}
	ch.Affects[0] = model.Affect{Type: 24, ExpiresAt: time.Now().Add(80 * time.Second)}
	b := UpdateScore(1, ch)
	if len(b) != 232 || ParseHeader(b).Type != OpUpdateScore || ParseHeader(b).ID != 1 ||
		binary.LittleEndian.Uint32(b[32:36]) != 900 ||
		binary.LittleEndian.Uint32(b[40:44]) != 900 ||
		binary.LittleEndian.Uint32(b[12+64:12+68]) != 3 ||
		binary.LittleEndian.Uint32(b[12+88:12+92]) != 4 ||
		binary.LittleEndian.Uint32(b[12+72:12+76]) != 7 ||
		binary.LittleEndian.Uint16(b[152:154]) != 0x180A ||
		binary.LittleEndian.Uint16(b[216:218]) != 0x0234 ||
		binary.LittleEndian.Uint16(b[218:220]) != model.GuildRankLeader ||
		binary.LittleEndian.Uint32(b[220:224]) != 0 ||
		binary.LittleEndian.Uint32(b[224:228]) != 0 {
		t.Fatalf("canonical UpdateScore incorreto: % X", b)
	}
}

func TestUpdateAffectsCanonicalFullLayout(t *testing.T) {
	ch := model.Char{}
	ch.Affects[0] = model.Affect{Type: 24, Value: 7, Level: 40,
		ExpiresAt: time.Now().Add(80 * time.Second)}
	ch.Affects[15] = model.Affect{Type: 31, ClientType: 24, Value: 150, Level: 55,
		ExpiresAt: time.Now().Add(40 * time.Second)}
	b := UpdateAffects(9, ch)
	last := 12 + 15*8
	// Opcode 0x3B9 must end immediately after the sixteenth native 7.48 affect.
	if len(b) != 140 || ParseHeader(b).Type != OpUpdateAffect || ParseHeader(b).ID != 9 ||
		b[12] != 24 || b[13] != 40 || binary.LittleEndian.Uint16(b[14:16]) != 7 ||
		binary.LittleEndian.Uint32(b[16:20]) != 10 ||
		b[last] != 24 || b[last+1] != 55 || binary.LittleEndian.Uint16(b[last+2:last+4]) != 150 ||
		binary.LittleEndian.Uint32(b[last+4:last+8]) != 5 {
		t.Fatalf("canonical UpdateAffects incorreto: first=% X last=% X", b[12:20], b[last:last+8])
	}
}

func TestCriticalArmorUses748VisualSlot(t *testing.T) {
	if got := clientAffectType(model.Affect{Type: 31, ClientType: 24}); got != 24 {
		t.Fatalf("affect 31 foi enviado como %d, quer visual 24 do client 7.48", got)
	}
	if got := clientAffectType(model.Affect{Type: 50}); got != 24 {
		t.Fatalf("estado legado da Armadura Critica foi enviado como %d, quer visual 24", got)
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

func TestSkillHit748CompactLayout(t *testing.T) {
	b := SkillHit(1, 1002, 2200, 2100, 2201, 2101, 88, 0, 1234, 500_070, 3, 9, 42)
	if len(b) != 48 || ParseHeader(b).Type != OpAttackOne ||
		int16(binary.LittleEndian.Uint16(b[24:26])) != 3 || b[28] != 9 || b[29] != 42 ||
		b[30] != 1 || binary.LittleEndian.Uint16(b[26:28]) != 65_535 ||
		binary.LittleEndian.Uint16(b[36:38]) != 65_535 ||
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

func TestSkillHitsCanonicalWideMultiLayout(t *testing.T) {
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
}

func TestSkillHitWideKeepsSkillAndFullDamage(t *testing.T) {
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
}

func TestPhysicalAttackKeepsFloatingDamagePath(t *testing.T) {
	b := AttackHitWide(1, 1001, 2200, 2100, 2201, 2101, 275_000, 550_000, 1234, 70)
	if len(b) != 52 || b[30] != 0 || binary.LittleEndian.Uint32(b[48:52]) != 275_000 {
		t.Fatalf("AttackHitWide sem visual/full damage: % X", b)
	}
}

func TestSkillShopListLayout(t *testing.T) {
	b := ShopList([]model.Item{{Index: 5000}, {Index: 5001}}, 0, ShopSkill)
	if len(b) != 236 || binary.LittleEndian.Uint32(b[12:16]) != ShopSkill ||
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
	b := CNFMobKill(1003, 1, 77, 9000)
	if len(b) != 24 || ParseHeader(b).Type != OpCNFMobKill || ParseHeader(b).ID != SceneField ||
		binary.LittleEndian.Uint32(b[12:16]) != 77 || binary.LittleEndian.Uint16(b[16:18]) != 1003 ||
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

func TestUpdateEtcNativeCompactLayout(t *testing.T) {
	ch := model.Char{
		CP: -25, Exp: 34000, Hold: 1234, LearnedSkill: 1 << 3, Gold: 99424,
		Score: &model.Score{
			Version:   model.ScoreVersion,
			StatusPts: 7, MasterPts: 100, SkillPts: 150, MagicAmp: 70,
		},
	}
	b := UpdateEtc(7, ch)
	// 0x337 is not STRUCT_SCORE in 7.48: FUN_0055890a accepts exactly 36 bytes.
	if len(b) != 36 || ParseHeader(b).Type != OpUpdateEtc || ParseHeader(b).ID != 7 ||
		binary.LittleEndian.Uint32(b[12:16]) != 1234 ||
		binary.LittleEndian.Uint32(b[16:20]) != 34000 ||
		binary.LittleEndian.Uint32(b[20:24]) != 1<<3 ||
		binary.LittleEndian.Uint16(b[24:26]) != 7 ||
		binary.LittleEndian.Uint16(b[26:28]) != 100 ||
		binary.LittleEndian.Uint16(b[28:30]) != 150 ||
		binary.LittleEndian.Uint16(b[30:32]) != 70 ||
		binary.LittleEndian.Uint32(b[32:36]) != 99424 {
		t.Fatalf("UpdateEtc compacto invalido: % X", b)
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

func TestCharListCarriesGuildIndexInSelection(t *testing.T) {
	ch := model.Char{Name: "GuildHero", X: 2100, Y: 2101, GuildID: 0x1234,
		Score: &model.Score{Version: model.ScoreVersion}}
	b := CharList("account", []model.Char{ch}, nil, 0)
	const guildOffset = 32 + 1216
	if len(b) != 2360 || binary.LittleEndian.Uint16(b[guildOffset:guildOffset+2]) != 0x0234 {
		t.Fatalf("Guild no SelectChar=0x%X, esperado id de 12 bits", binary.LittleEndian.Uint16(b[guildOffset:guildOffset+2]))
	}
}

func TestMessageChatCanonicalLayout(t *testing.T) {
	b := MessageChat(7, "hello")
	h := ParseHeader(b)
	if len(b) != 108 || h.Type != OpMessageChat || h.ID != 7 || string(b[12:17]) != "hello" {
		t.Fatalf("MessageChat header/layout invalido: len=%d header=%+v", len(b), h)
	}
	// Size/checksum/tick are transport-owned and are finalized only by Session.Send.
	if h.Size != 0 || h.CheckSum != 0 || h.Tick != 0 {
		t.Fatalf("builder finalizou campos de transporte prematuramente: %+v", h)
	}
}

func TestMessageChatTruncatesTextAndKeepsTerminator(t *testing.T) {
	b := MessageChat(7, string(bytes.Repeat([]byte{'x'}, 120)))
	if len(b) != 108 {
		t.Fatalf("MessageChat len=%d, esperado 108", len(b))
	}
	if !bytes.Equal(b[12:107], bytes.Repeat([]byte{'x'}, 95)) {
		t.Fatal("MessageChat nao preservou exatamente os primeiros 95 bytes")
	}
	if b[107] != 0 {
		t.Fatalf("MessageChat perdeu terminador NUL final: 0x%02X", b[107])
	}
}

func TestIndexedMessageExtensionCanonicalLayout(t *testing.T) {
	b := MessageIndexed(-1000)
	h := ParseHeader(b)
	if len(b) != 108 || h.Type != OpMessageIndexed || h.ID != 0 {
		t.Fatalf("MessageIndexed header/layout invalido: len=%d header=%+v", len(b), h)
	}
	if got := int16(binary.LittleEndian.Uint16(b[14:16])); got != -1000 {
		t.Fatalf("MessageIndexed indice relativo=%d, esperado -1000", got)
	}
	if b[12] != 0 || b[13] != 0 || b[107] != 0 {
		t.Fatalf("MessageIndexed seletor/reservado/terminador alterados: % X", []byte{b[12], b[13], b[107]})
	}
}

func TestParameterizedMessageExtensionBoundsAndSanitizesCSV(t *testing.T) {
	b := MessageParameterized(-561, "alpha,beta", "ga\x00mma", "three", "four", "five", "six", "ignored")
	h := ParseHeader(b)
	if len(b) != 108 || h.Type != OpMessageParameterized || h.ID != 0 {
		t.Fatalf("MessageParameterized header/layout invalido: len=%d header=%+v", len(b), h)
	}
	if got := int16(binary.LittleEndian.Uint16(b[14:16])); got != -561 {
		t.Fatalf("MessageParameterized indice relativo=%d, esperado -561", got)
	}
	if got := cStringForTest(b[16:108]); got != "alpha beta,ga mma,three,four,five,six" {
		t.Fatalf("MessageParameterized CSV=%q", got)
	}
	if bytes.Contains(b[16:107], []byte("ignored")) || b[107] != 0 {
		t.Fatal("MessageParameterized aceitou o setimo parametro ou perdeu o terminador")
	}

	long := MessageParameterized(-1000, string(bytes.Repeat([]byte{'x'}, 200)))
	if !bytes.Equal(long[16:107], bytes.Repeat([]byte{'x'}, 91)) || long[107] != 0 {
		t.Fatal("MessageParameterized nao truncou o CSV no limite de 91 bytes")
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

func cStringForTest(b []byte) string {
	for i, c := range b {
		if c == 0 {
			return string(b[:i])
		}
	}
	return string(b)
}

func TestUpdateEtcClampsWidePointsToNativeWords(t *testing.T) {
	ch := model.Char{
		Score: &model.Score{
			Version:   model.ScoreVersion,
			StatusPts: 100000,
			MasterPts: 110000,
			SkillPts:  120000,
		},
	}
	b := UpdateEtc(1, ch)
	// Wide counters still travel in 0x336; this incremental packet exposes only
	// the WORD projection present in the 7.48 executable.
	if len(b) != 36 ||
		binary.LittleEndian.Uint16(b[24:26]) != 65_535 ||
		binary.LittleEndian.Uint16(b[26:28]) != 65_535 ||
		binary.LittleEndian.Uint16(b[28:30]) != 65_535 {
		t.Fatalf("wide points nao foram limitados ao ABI WORD: % X", b[24:30])
	}
}

func TestSwapItem748ConfirmationLayout(t *testing.T) {
	b := SwapItem(7, 1, 3, 1, 14, 1100)
	// FUN_00486808 consumes the echoed four cell bytes and merchant target.
	if len(b) != 20 || ParseHeader(b).Type != OpSwapItem || ParseHeader(b).ID != 7 ||
		b[12] != 1 || b[13] != 3 || b[14] != 1 || b[15] != 14 ||
		binary.LittleEndian.Uint16(b[16:18]) != 1100 {
		t.Fatalf("SwapItem 7.48 invalido: % X", b)
	}
}

func TestBuyItem748ConfirmationLayout(t *testing.T) {
	b := BuyItem(7, 1100, 27, 14, 4321)
	// FUN_00487b92 copies the shop cell to Carry and updates Coin from this 24B response.
	if len(b) != 24 || ParseHeader(b).Type != OpBuyItem || ParseHeader(b).ID != 7 ||
		binary.LittleEndian.Uint16(b[12:14]) != 1100 ||
		binary.LittleEndian.Uint16(b[14:16]) != 27 ||
		binary.LittleEndian.Uint16(b[16:18]) != 14 ||
		binary.LittleEndian.Uint32(b[20:24]) != 4321 {
		t.Fatalf("BuyItem 7.48 invalido: % X", b)
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
