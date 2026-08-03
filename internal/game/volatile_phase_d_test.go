package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestUseItemRequestReadsWarpID(t *testing.T) {
	pkt := make([]byte, 36)
	binary.LittleEndian.PutUint32(pkt[12:16], placeInv)
	binary.LittleEndian.PutUint32(pkt[16:20], 1)
	binary.LittleEndian.PutUint32(pkt[32:36], 10)
	req, ok := parseUseItemRequest(pkt)
	if !ok || req.warpID != 10 {
		t.Fatalf("warpID=%d ok=%v", req.warpID, ok)
	}
	binary.LittleEndian.PutUint32(pkt[32:36], 0x10000000|10)
	if _, ok := parseUseItemRequest(pkt); !ok {
		// The parser preserves the complete DWORD; the action-specific handler
		// rejects values outside 1..10 instead of silently truncating them.
		t.Fatalf("WarpID DWORD valido foi rejeitado")
	}
	if _, ok := parseUseItemRequest(append(pkt, 0)); ok {
		t.Fatal("pacote de uso de item sobredimensionado foi aceito")
	}
}

func TestSpecialSkillBits(t *testing.T) {
	for skill := 97; skill <= 101; skill++ {
		bit, ok := specialSkillBit(skill)
		if !ok || bit != uint(skill-72) {
			t.Fatalf("skill %d -> bit %d ok=%v", skill, bit, ok)
		}
		ch := &model.Char{LearnedSkill: uint32(1) << bit}
		if !specialSkillLearned(ch, skill) {
			t.Fatalf("skill %d deveria estar aprendida", skill)
		}
	}
}

func TestMagicalPillBonusEntersAuthoritativeBudget(t *testing.T) {
	ch := &model.Char{Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, Level: 50}, SkillPointBonus: 9}
	syncSkillPoints(ch)
	want := uint32(mortalSkillPointBudget(50) + 9)
	if ch.Extended.SkillPts != want {
		t.Fatalf("skillPts=%d want=%d", ch.Extended.SkillPts, want)
	}
}

func TestAffect35AddsTenPercentWithoutAccumulating(t *testing.T) {
	ch := &model.Char{Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1000, CurHP: 1000}}
	ch.Affects[0] = model.Affect{Type: 35, Value: 10, ExpiresAt: time.Now().Add(time.Hour)}
	w := &World{}
	w.applyExtendedAffectStats(ch)
	if ch.ExtendedRuntime.MaxHP != 1100 {
		t.Fatalf("primeiro maxHP=%d", ch.ExtendedRuntime.MaxHP)
	}
	ch.ExtendedRuntime = nil
	w.applyExtendedAffectStats(ch)
	if ch.ExtendedRuntime.MaxHP != 1100 {
		t.Fatalf("recalculo acumulou: %d", ch.ExtendedRuntime.MaxHP)
	}
}
