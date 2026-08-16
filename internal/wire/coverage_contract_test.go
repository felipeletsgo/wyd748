package wire

import (
	"bytes"
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func testExtendedWireScore() *model.ExtendedScore {
	return &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Level: 120, Attack: 1000, Defense: 900,
		MaxHP: 100000, CurHP: 75000, MaxMP: 50000, CurMP: 25000,
		Str: 100, Int: 200, Dex: 300, Con: 400,
	}
}

func assertPacketHeader(t *testing.T, pkt []byte, opcode uint16, size int) {
	t.Helper()
	if len(pkt) != size {
		t.Fatalf("len=%d, esperado=%d", len(pkt), size)
	}
	h := ParseHeader(pkt)
	if h.Type != opcode || h.Size != uint16(size) {
		t.Fatalf("header=%+v, opcode esperado=0x%X size=%d", h, opcode, size)
	}
}

func TestStandardAndCargoBuildersUseConfirmedOffsets(t *testing.T) {
	parm := StandardParm(OpPKInfo, 77, 0x11223344)
	assertPacketHeader(t, parm, OpPKInfo, 16)
	if binary.LittleEndian.Uint32(parm[12:16]) != 0x11223344 {
		t.Fatalf("StandardParm @12=%08X", binary.LittleEndian.Uint32(parm[12:16]))
	}

	gold := UpdateCargoGold(77, 123456)
	assertPacketHeader(t, gold, OpUpdateCargoGold, 16)
	if binary.LittleEndian.Uint32(gold[12:16]) != 123456 {
		t.Fatal("UpdateCargoGold deslocou saldo")
	}

	transfer := CargoGoldTransfer(OpDeposit, 77, 4321)
	assertPacketHeader(t, transfer, OpDeposit, 16)
	if binary.LittleEndian.Uint32(transfer[12:16]) != 4321 {
		t.Fatal("CargoGoldTransfer deslocou valor")
	}
}

func TestCharacterSelectionConfirmationBuildersKeep748Sizes(t *testing.T) {
	ch := model.Char{Name: "Tester", X: 2100, Y: 2100, Extended: testExtendedWireScore()}
	deleted := CNFDeleteCharacter(44, []model.Char{ch})
	assertPacketHeader(t, deleted, OpCNFDeleteCharacter, 756)
	if got := string(bytes.TrimRight(deleted[28:44], "\x00")); got != "Tester" {
		t.Fatalf("nome no SelChar=%q", got)
	}
	assertPacketHeader(t, NewCharacterFail(), OpNewCharacterFail, 12)
	logout := CNFCharacterLogout(44)
	assertPacketHeader(t, logout, OpCNFCharacterLogout, 12)
	if ParseHeader(logout).ID != 44 {
		t.Fatalf("logout ID=%d", ParseHeader(logout).ID)
	}
}

func TestExtendedWrappersMatchTheirCompatibilityProjection(t *testing.T) {
	ext := testExtendedWireScore()
	mesh := []uint16{1, 2, 3}
	anct := []byte{4, 5, 6}
	tradeWide := CreateMobTradeExtended(1000, "Shop", 100, 101, mesh, ext, "TITLE")
	tradeNarrow := CreateMobTrade(1000, "Shop", 100, 101, mesh, ext.CompatibilityScore(), "TITLE")
	if !bytes.Equal(tradeWide, tradeNarrow) {
		t.Fatal("CreateMobTradeExtended nao e adaptador puro da projecao")
	}

	visualWide := CreateMobVisualExtended(1000, "Mob", 100, 101, mesh, anct, ext, nil, 0)
	visualNarrow := CreateMobVisual(1000, "Mob", 100, 101, mesh, anct, ext.CompatibilityScore(), nil, 0)
	if !bytes.Equal(visualWide, visualNarrow) {
		t.Fatal("CreateMobVisualExtended divergiu da projecao estreita")
	}

	level, hp, maxHP := CompatibilityVitals(ext)
	score := ext.CompatibilityScore()
	if level != score.Level || hp != score.CurHP || maxHP != score.MaxHP {
		t.Fatalf("CompatibilityVitals=(%d,%d,%d) score=%+v", level, hp, maxHP, score)
	}
	if mp := CompatibilityCombatMP(ext); mp != uint32(score.CurMP) {
		t.Fatalf("CompatibilityCombatMP=%d, esperado=%d", mp, score.CurMP)
	}
	if CompatibilityCombatMP(nil) != 0 {
		t.Fatal("CompatibilityCombatMP(nil) deveria ser zero")
	}

	resist := model.ElementalResists{Fire: 10, Ice: 20, Sacred: 30, Thunder: 40}
	wideScore := MobScoreExtended(1000, ext, nil, resist)
	narrowScore := MobScore(1000, ext.CompatibilityScore(), nil, resist)
	if !bytes.Equal(wideScore, narrowScore) {
		t.Fatal("MobScoreExtended divergiu da projecao")
	}
}

func TestInventoryMovementAndWorldItemBuildersUseExactLayouts(t *testing.T) {
	item := model.Item{Index: 4011, Eff: [6]byte{1, 2, 3, 4, 5, 6}}
	carry := UpdateCarry(77, []model.Item{item}, 999)
	assertPacketHeader(t, carry, OpUpdateCarry, 528)
	if binary.LittleEndian.Uint16(carry[12:14]) != item.Index || binary.LittleEndian.Uint32(carry[524:528]) != 999 {
		t.Fatal("UpdateCarry deslocou item ou coin")
	}

	stop := ActionStop(1000, 123, 456)
	assertPacketHeader(t, stop, OpAction, 52)
	if binary.LittleEndian.Uint16(stop[12:14]) != 123 || binary.LittleEndian.Uint16(stop[14:16]) != 456 ||
		binary.LittleEndian.Uint32(stop[16:20]) != 2 || binary.LittleEndian.Uint32(stop[20:24]) != 1 ||
		binary.LittleEndian.Uint16(stop[24:26]) != 123 || binary.LittleEndian.Uint16(stop[26:28]) != 456 {
		t.Fatal("ActionStop nao preservou layout Action 7.48")
	}

	sent := SendItem(77, 1, 5, item)
	assertPacketHeader(t, sent, OpSendItem, 24)
	if binary.LittleEndian.Uint16(sent[12:14]) != 1 || binary.LittleEndian.Uint16(sent[14:16]) != 5 ||
		binary.LittleEndian.Uint16(sent[16:18]) != item.Index {
		t.Fatal("SendItem deslocou destino/item")
	}

	war := WarInfo()
	assertPacketHeader(t, war, OpWarInfo, 24)
	if ParseHeader(war).ID != SceneField {
		t.Fatal("WarInfo nao usa SceneField")
	}

	updated := UpdateItem(1234, 7)
	assertPacketHeader(t, updated, OpUpdateItem, 20)
	if binary.LittleEndian.Uint32(updated[12:16]) != 1234 || binary.LittleEndian.Uint32(updated[16:20]) != 7 {
		t.Fatal("UpdateItem deslocou ItemID/State")
	}
}

func TestSpectralVisualOnlyTouchesNativeFlagByteWhenAvailable(t *testing.T) {
	short := []byte{1, 2, 3}
	if got := SpectralVisual(short); !bytes.Equal(got, short) {
		t.Fatal("SpectralVisual alterou pacote truncado")
	}
	pkt := make([]byte, 48)
	pkt[31] = 0x02
	got := SpectralVisual(pkt)
	if got[31] != 0x0A {
		t.Fatalf("DoubleCritical@31=%02X, esperado 0A", got[31])
	}
}

func TestFinishPacketRoundTripsSizeEncryptionAndChecksum(t *testing.T) {
	pkt := Build(OpPing, 12, HeaderSize)
	FinishPacket(pkt, 9)
	decoded, checksum, err := ReadPacket(bytes.NewReader(pkt))
	if err != nil || !checksum {
		t.Fatalf("FinishPacket nao produziu frame valido: checksum=%v err=%v", checksum, err)
	}
	h := ParseHeader(decoded)
	if h.Size != HeaderSize || h.Type != OpPing || h.ID != 12 {
		t.Fatalf("round-trip do header divergiu: %+v", h)
	}
}
