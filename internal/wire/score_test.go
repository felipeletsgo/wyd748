package wire

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestEncodeClientScorePreservesCanonicalUint32Fields(t *testing.T) {
	s := &model.Score{
		Version:       model.ScoreVersion,
		Level:         70000,
		Attack:        123456,
		MagicAttack:   234567,
		Defense:       345678,
		MaxHP:         4_000_000,
		MaxMP:         3_000_000,
		CurHP:         2_500_000,
		CurMP:         1_500_000,
		Str:           100000,
		Int:           100001,
		Dex:           100002,
		Con:           100003,
		Accuracy:      456789,
		Evasion:       567890,
		Parry:         678901,
		Critical:      789012,
		Range:         890123,
		ResistFire:    91,
		ResistIce:     92,
		ResistHoly:    93,
		ResistThunder: 94,
		SaveMana:      95,
		MagicAmp:      901234,
		RegenHP:       1234,
		RegenMP:       2345,
		StatusPts:     500000,
		MasterPts:     600000,
		SkillPts:      700000,
		Mastery:       [4]uint32{100000, 200000, 300000, 400000},
		AttackRun:     0x12345678,
		Merchant:      0x23456789,
	}
	b := EncodeClientScore(s)
	if len(b) != 140 {
		t.Fatalf("canonical score size=%d want=140", len(b))
	}
	want := []uint32{
		s.Version, s.Level, s.Attack, s.MagicAttack, s.Defense,
		s.MaxHP, s.MaxMP, s.CurHP, s.CurMP,
		s.Str, s.Int, s.Dex, s.Con,
		s.Accuracy, s.Evasion, s.Parry, s.Critical, s.Range,
		s.ResistFire, s.ResistIce, s.ResistHoly, s.ResistThunder,
		s.SaveMana, s.MagicAmp, s.RegenHP, s.RegenMP,
		s.StatusPts, s.MasterPts, s.SkillPts,
		s.Mastery[0], s.Mastery[1], s.Mastery[2], s.Mastery[3],
		s.AttackRun, s.Merchant,
	}
	for i, expected := range want {
		got := binary.LittleEndian.Uint32(b[i*4 : (i+1)*4])
		if got != expected {
			t.Fatalf("word[%d]=%d want=%d", i, got, expected)
		}
	}
}
