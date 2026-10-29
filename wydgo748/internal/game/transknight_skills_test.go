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
	// Critical Armor: AC 120%, attack + AC/10, critical +100.
	if e.Defense != 240 || e.Attack != 124 || e.Critical != 100 {
		t.Fatalf("Critical Armor=%+v", e)
	}
	// Destiny: holy resistance +20, clamped at 100.
	if e.ResistHoly != 100 {
		t.Fatalf("Destiny holy resistance=%d, want 100", e.ResistHoly)
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
