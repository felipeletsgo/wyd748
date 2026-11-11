package wire

import (
	"encoding/binary"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestChannelMigrationKeepsOpaqueTicketOffsets(t *testing.T) {
	packet := ChannelMigration(0x1234, "account", "opaque-ticket")
	if len(packet) != 80 || ParseHeader(packet).Type != 0x52A || ParseHeader(packet).ID != 0x1234 {
		t.Fatalf("migration header=%+v length=%d", ParseHeader(packet), len(packet))
	}
	if got := strings.TrimRight(string(packet[12:27]), "\x00"); got != "account" {
		t.Fatalf("account=%q", got)
	}
	if got := strings.TrimRight(string(packet[28:79]), "\x00"); got != "opaque-ticket" {
		t.Fatalf("opaque ticket=%q", got)
	}
	if packet[27] != 0 || packet[79] != 0 {
		t.Fatal("migration reserved bytes were not zeroed")
	}
}

func TestResourceAndAuraRecoveryPacketsCarryFullValues(t *testing.T) {
	mob := MobHpMp(77, 70000, 90000, 80000, 100000)
	if ParseHeader(mob).Type != OpSetHpMp || binary.LittleEndian.Uint32(mob[12:16]) != 70000 ||
		binary.LittleEndian.Uint32(mob[20:24]) != 90000 || binary.LittleEndian.Uint32(mob[16:20]) != 80000 ||
		binary.LittleEndian.Uint32(mob[24:28]) != 100000 {
		t.Fatalf("mob resources=%x", mob)
	}
	player := HpMp(88, &model.Score{CurHP: 1, CurMP: 2, MaxHP: 3, MaxMP: 4})
	if binary.LittleEndian.Uint32(player[12:16]) != 1 || binary.LittleEndian.Uint32(player[24:28]) != 4 {
		t.Fatalf("player resources=%x", player)
	}
	if empty := HpMp(88, nil); len(empty) != 28 || binary.LittleEndian.Uint32(empty[12:]) != 0 {
		t.Fatalf("nil score packet=%x", empty)
	}
	aura := AuraOfLifeRecovery(88, 65000, 50000)
	if ParseHeader(aura).Type != OpSetHpDam || len(aura) != 20 || binary.LittleEndian.Uint32(aura[12:16]) != 65000 ||
		binary.LittleEndian.Uint16(aura[16:18]) != 32767 {
		t.Fatalf("aura recovery packet=%x", aura)
	}
}

func TestPhysicalTwoTargetWideResultAndVisualModifiers(t *testing.T) {
	packet := AttackHitsWideTwoResult(1, 10, 11, 12, 13, 14, 15, 0xA5, []SkillTarget{
		{ID: 2, Damage: 70000, MaxHP: 100000}, {ID: 3, Damage: 80000, MaxHP: 100000},
	})
	if len(packet) != 68 || ParseHeader(packet).Type != OpAttackTwo || packet[31] != 0xA5 ||
		binary.LittleEndian.Uint32(packet[52:56]) != 0x58474D44 || binary.LittleEndian.Uint32(packet[56:60]) != 2 ||
		binary.LittleEndian.Uint32(packet[60:64]) != 70000 || binary.LittleEndian.Uint32(packet[64:68]) != 80000 {
		t.Fatalf("physical wide result=%x", packet)
	}

	short := []byte{1, 2, 3}
	if got := AttackerHold(short, 123); len(got) != len(short) || got[2] != 3 {
		t.Fatal("short attacker packet changed")
	}
	packet = make([]byte, 44)
	AttackerHold(packet, 0x12345678)
	if binary.LittleEndian.Uint32(packet[40:44]) != 0x12345678 {
		t.Fatal("attacker hold was not written to FakeExp")
	}
	visual := make([]byte, 32)
	visual[31] = 1
	SpectralVisual(visual)
	if visual[31] != 9 {
		t.Fatalf("spectral flag=%d", visual[31])
	}
	shortVisual := make([]byte, 31)
	SpectralVisual(shortVisual)
	if len(shortVisual) != 31 {
		t.Fatal("short visual packet changed length")
	}
}
