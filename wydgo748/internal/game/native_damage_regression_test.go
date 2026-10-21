package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func nativeDamageZeroRNG(int) int { return 0 }

func TestEffectiveMobElementalDefenseAppliesClassMultiplier(t *testing.T) {
	now := time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC)
	regular := &Mob{Def: &model.NPCDef{ClassInfo: 0, Score: &model.Score{Defense: 100}}}
	classOne := &Mob{Def: &model.NPCDef{ClassInfo: 1, Score: &model.Score{Defense: 100}}}
	classOneWithDebuff := &Mob{
		Def:     &model.NPCDef{ClassInfo: 1, Score: &model.Score{Defense: 100}},
		Affects: [16]model.Affect{{Type: 12, Value: 20, ExpiresAt: now.Add(time.Minute)}},
	}

	if got := effectiveMobElementalDefenseAt(regular, now); got != 100 {
		t.Fatalf("regular elemental defense=%d, want 100", got)
	}
	if got := effectiveMobElementalDefenseAt(classOne, now); got != 150 {
		t.Fatalf("class-1 elemental defense=%d, want 150", got)
	}
	if got := effectiveMobElementalDefenseAt(classOneWithDebuff, now); got != 120 {
		t.Fatalf("debuffed class-1 elemental defense=%d, want 120", got)
	}
}

func TestMobPhysicalHitMobUsesNativeDamagePath(t *testing.T) {
	now := time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC)
	attacker := &Mob{Def: &model.NPCDef{Score: &model.Score{Attack: 100}}}
	target := &Mob{Def: &model.NPCDef{Score: &model.Score{Defense: 20}}}

	got := mobPhysicalHitMobAt(attacker, target, nativeDamageZeroRNG, now)
	want := uint32(hitDamageWithRNG(100, 20, 0, nativeDamageZeroRNG))
	if got != want {
		t.Fatalf("monster physical damage=%d, want BASE_GetDamage result %d", got, want)
	}
	if got == 90 {
		t.Fatalf("monster physical damage still uses raw attack-defense/2 arithmetic: %d", got)
	}
}

func TestPlayerElementalSkillAppliesClassOneDefenseMultiplier(t *testing.T) {
	regular := skillPvPFinalDamageForTargetWithRNG(1_000, 100, 0, 0, nativeDamageZeroRNG)
	classOne := skillPvPFinalDamageForTargetWithRNG(1_000, 100, 0, 1, nativeDamageZeroRNG)
	if regular != 810 {
		t.Fatalf("regular PvP elemental damage=%d, want 810", regular)
	}
	if classOne != 765 {
		t.Fatalf("class-1 PvP elemental damage=%d, want 765", classOne)
	}
}
