package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func transKnightSkillChar(learned uint32) *model.Char {
	ch := &model.Char{Class: 0, LearnedSkill: learned, Score: testScore(model.Score{
		Level: 100, Int: 100, Mastery: [4]uint32{0, 40, 0, 80},
	})}
	ch.RuntimeScore = testScore(*ch.Score)
	return ch
}

// Official TMSRVIN BASE_GetSkillDamage (sub_4C3DE0) uses the server skill
// order: 7.48 Double Swing (16) is server skill 2, in tree 0.
func TestTransKnightSkillDamageUsesServerSkillOrder(t *testing.T) {
	w := &World{}
	swing := model.SkillDef{Index: 16, InstanceType: 1, InstanceValue: 13}
	// (100/2 + Special[1] 40 + 13 + 100/4 + 100/40) * 5/4 = 162.
	if got := w.baseSkillDamage(transKnightSkillChar(0), swing); got != 162 {
		t.Fatalf("Double Swing damage=%d, want 162", got)
	}
	// Tree 0 finisher is Destiny (7), not Freeze Spiral (23).
	if got := w.baseSkillDamage(transKnightSkillChar(1<<7), swing); got != 186 {
		t.Fatalf("Double Swing with Destiny=%d, want 186", got)
	}
	if got := w.baseSkillDamage(transKnightSkillChar(1<<23), swing); got != 162 {
		t.Fatalf("Freeze Spiral incorrectly boosted tree 0: %d", got)
	}
}

func TestTransKnightPassivesFollowOfficialServer(t *testing.T) {
	w := &World{}
	ch := &model.Char{Class: 0, LearnedSkill: 1<<7 | 1<<15, Score: testScore(model.Score{
		Attack: 100, Defense: 200, ResistHoly: 90,
	})}
	ch.RuntimeScore = testScore(*ch.Score)
	w.applyPassiveSkills(ch)
	e := effectiveScore(ch)
	// Critical Armor is a 7.48 buff: learning it adds no passive bonus.
	if e.Defense != 200 || e.Attack != 100 || e.Critical != 0 {
		t.Fatalf("Critical Armor passive=%+v", e)
	}
	// Destiny: holy resistance +20, clamped at 100.
	if e.ResistHoly != 100 {
		t.Fatalf("Destiny holy resistance=%d, want 100", e.ResistHoly)
	}
}

// Native 7.48 help 5015 (Critical Armor) and 5009 (Berserk), and the native
// client score FUN_0054FDB9 for affect 8.
func TestTransKnightNativeBuffs(t *testing.T) {
	now := time.Now()
	ch := &model.Char{Class: 0, Score: testScore(model.Score{Defense: 200})}
	ch.Affects[0] = model.Affect{Type: 31, ClientType: affectCriticalArmorClientType,
		Level: 120, Value: 100, ExpiresAt: now.Add(time.Minute)}
	(&World{}).applyAffectStats(ch)
	// Defense +(7 + 120/50)% = 109%; critical +(3 + 2)% = 50 tenths.
	if e := effectiveScore(ch); e.Defense != 218 || e.Critical != 50 {
		t.Fatalf("Critical Armor buff=%+v", e)
	}
	coin := &model.Char{Class: 3, Score: testScore(model.Score{Defense: 200})}
	coin.Affects[0] = model.Affect{Type: 31, ClientType: 31, Level: 120, Value: 100,
		ExpiresAt: now.Add(time.Minute)}
	(&World{}).applyAffectStats(coin)
	if e := effectiveScore(coin); e.Defense != 312 || e.Critical != 0 {
		t.Fatalf("Coin Armor changed=%+v", e)
	}

	plain := &model.Char{Class: 0, Score: testScore(model.Score{})}
	base := (&World{}).applyAffectStats(plain)
	berserk := &model.Char{Class: 0, Score: testScore(model.Score{})}
	berserk.Affects[0] = model.Affect{Type: 8, Level: 120, Value: 10, ExpiresAt: now.Add(time.Minute)}
	if got := (&World{}).applyAffectStats(berserk) - base; got != 22 {
		t.Fatalf("Berserk attack speed=%d, want 10 + 120/10", got)
	}
}

// Native 7.48 help 5014: Combat Sensibility raises the minimum physical
// damage by Special[2]/20; other characters get no raise.
func TestCombatSensibilityRaisesMinimumPhysicalDamage(t *testing.T) {
	low := func(int) int { return 0 }
	plain := &model.Char{Class: 0, Score: testScore(model.Score{Mastery: [4]uint32{255, 0, 200, 0}})}
	if got := skillDamageMastery(plain); got != 0 {
		t.Fatalf("mastery without Combat Sensibility=%d, want 0", got)
	}
	if got := hitDamageWithRNG(1000, 0, skillDamageMastery(plain), low); got != 990 {
		t.Fatalf("minimum hit=%d, want 990", got)
	}
	learned := &model.Char{Class: 0, LearnedSkill: 1 << 14,
		Score: testScore(model.Score{Mastery: [4]uint32{0, 0, 200, 0}})}
	if got := hitDamageWithRNG(1000, 0, skillDamageMastery(learned), low); got != 1040 {
		t.Fatalf("minimum hit with Combat Sensibility=%d, want 1040", got)
	}
}

func TestTransKnightAffectsUseServerSkillBits(t *testing.T) {
	now := time.Now()
	apply := func(w *World, learned uint32, affect model.Affect, shield bool) *model.Score {
		ch := &model.Char{Class: 0, LearnedSkill: learned, Score: testScore(model.Score{
			Level: 30, Attack: 100, Defense: 100, MaxHP: 1000, MaxMP: 400,
			Str: 10, Int: 10, Dex: 10, Con: 10,
		})}
		if shield {
			ch.Equip[7] = model.Item{Index: 900}
		}
		affect.ExpiresAt = now.Add(time.Minute)
		ch.Affects[0] = affect
		w.applyAffectStats(ch)
		return effectiveScore(ch)
	}
	w := &World{}
	// Assault + 5 with Critical Armor.
	if e := apply(w, 1<<15, model.Affect{Type: 13, Level: 40, Value: 7}, false); e.Attack != 116 {
		t.Fatalf("Assault with Critical Armor attack=%d, want 116", e.Attack)
	}
	// Growth with Destiny: CON + 10+40+40 + 40 + 200; STR/INT/DEX + 200.
	e := apply(w, 1<<7, model.Affect{Type: 14, Level: 40, Value: 10}, false)
	if e.Con != 340 || e.Str != 210 || e.Int != 210 || e.Dex != 210 ||
		e.MaxHP != 1660 || e.MaxMP != 800 {
		t.Fatalf("Growth with Destiny=%+v", e)
	}
	// Speed Up (bit 9) is not Destiny: plain Growth.
	if e := apply(w, 1<<9, model.Affect{Type: 14, Level: 40, Value: 10}, false); e.Con != 100 {
		t.Fatalf("Growth with Speed Up CON=%d, want 100", e.Con)
	}
	// Rescue with a shield and Critical Armor: (AC + Level) * 140%, resist +30.
	shieldWorld := &World{items: map[uint16]model.ItemDef{900: {Index: 900, Pos: 128}}}
	e = apply(shieldWorld, 1<<15, model.Affect{Type: 24, Level: 40}, true)
	if e.Defense != 196 || e.ResistFire != 30 || e.ResistThunder != 30 {
		t.Fatalf("Rescue with shield=%+v", e)
	}
}
