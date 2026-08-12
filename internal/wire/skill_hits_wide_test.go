package wire

import (
	"encoding/binary"
	"testing"
)

func TestSkillHitsWideMultiCarriesAllRealDamagesInOnePacket(t *testing.T) {
	targets := []SkillTarget{
		{ID: 1001, Damage: 70_000, MaxHP: 200_000},
		{ID: 1002, Damage: 80_000, MaxHP: 300_000},
		{ID: 1003, Damage: 90_000, MaxHP: 400_000},
	}
	pkt := SkillHitsWide(1, 10, 10, 12, 12, 0, 500, 34, 5, 100, 13, targets)
	if got, want := len(pkt), 96+8+4*len(targets); got != want {
		t.Fatalf("size=%d want=%d", got, want)
	}
	if got := binary.LittleEndian.Uint16(pkt[0:2]); int(got) != len(pkt) {
		t.Fatalf("header size=%d packet=%d", got, len(pkt))
	}
	if got := binary.LittleEndian.Uint32(pkt[96:100]); got != 0x58474D44 {
		t.Fatalf("tail magic=%08X", got)
	}
	if got := binary.LittleEndian.Uint32(pkt[100:104]); got != uint32(len(targets)) {
		t.Fatalf("tail count=%d", got)
	}
	for i, target := range targets {
		if got := binary.LittleEndian.Uint32(pkt[104+i*4 : 108+i*4]); got != target.Damage {
			t.Fatalf("damage[%d]=%d want=%d", i, got, target.Damage)
		}
		// Prefix remains projected per target, not the raw uint32 damage.
		if got := binary.LittleEndian.Uint16(pkt[46+i*4 : 48+i*4]); got != wireDamage(target) {
			t.Fatalf("projected[%d]=%d want=%d", i, got, wireDamage(target))
		}
	}
}

func TestSkillHitsWideTwoTargetOffsetsMatchClientPatch(t *testing.T) {
	targets := []SkillTarget{{ID: 1001, Damage: 111}, {ID: 1002, Damage: 222}}
	pkt := SkillHitsWide(1, 10, 10, 12, 12, 0, 500, 34, 5, 100, 2, targets)
	if got, want := len(pkt), 52+8+8; got != want {
		t.Fatalf("size=%d want=%d", got, want)
	}
	if got := binary.LittleEndian.Uint32(pkt[60:64]); got != 111 {
		t.Fatalf("damage0@60=%d", got)
	}
	if got := binary.LittleEndian.Uint32(pkt[64:68]); got != 222 {
		t.Fatalf("damage1@64=%d", got)
	}
}

func TestSkillHitsWideSingleKeepsExisting39DOffset(t *testing.T) {
	target := SkillTarget{ID: 1001, Damage: 123_456, MaxHP: 500_000}
	pkt := SkillHitsWide(1, 10, 10, 12, 12, 0, 500, 34, 5, 100, 1, []SkillTarget{target})
	if len(pkt) != 52 {
		t.Fatalf("size=%d want=52", len(pkt))
	}
	if got := binary.LittleEndian.Uint32(pkt[48:52]); got != target.Damage {
		t.Fatalf("damage@48=%d want=%d", got, target.Damage)
	}
	if pkt[30] != 0 {
		t.Fatalf("FlagLocal=%d want=0", pkt[30])
	}
}
