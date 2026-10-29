package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func huntressSkillChar(learned uint32) *model.Char {
	ch := &model.Char{Class: 3, LearnedSkill: learned, Score: testScore(model.Score{
		Level: 100, Str: 100, Attack: 500, Mastery: [4]uint32{0, 30, 60, 90},
	})}
	ch.RuntimeScore = testScore(*ch.Score)
	return ch
}

// Official TMSRVIN BASE_GetSkillDamage (sub_4C3DE0) uses the server skill
// order. Huntress 7.48 skills map to it by name.
func TestHuntressSkillDamageUsesServerSkillOrder(t *testing.T) {
	w := &World{}
	knife := model.SkillDef{Index: 94, InstanceType: 1, InstanceValue: 35}
	// Throwing Knife is server skill 86: tree 1, Special[2].
	// (3*0 + 3*100 + 100/2 + 60 + 35) * 5/4 = 556.
	if got := w.baseSkillDamage(huntressSkillChar(0), knife); got != 556 {
		t.Fatalf("Throwing Knife damage=%d, want 556", got)
	}
	// Tree 1 finisher is Spirit Change (87), not 7.48 skill 95.
	if got := w.baseSkillDamage(huntressSkillChar(1<<(87-72)), knife); got != 611 {
		t.Fatalf("Throwing Knife with Spirit Change=%d, want 611", got)
	}
	if got := w.baseSkillDamage(huntressSkillChar(1<<(95-72)), knife); got != 556 {
		t.Fatalf("Rapid Hit incorrectly boosted tree 1: %d", got)
	}

	// Snick Attack is tree 2; its finisher is Hide in Shadow (75).
	snick := model.SkillDef{Index: 88, InstanceType: 1, InstanceValue: 12}
	plain := w.baseSkillDamage(huntressSkillChar(0), snick)
	if got := w.baseSkillDamage(huntressSkillChar(1<<(75-72)), snick); got != plain*120/100 {
		t.Fatalf("Snick Attack with Hide in Shadow=%d, want %d", got, plain*120/100)
	}

	// Rapid Hit (7.48 skill 95) is server skill 79 and uses the attack score.
	rapid := model.SkillDef{Index: 95, InstanceType: 1}
	if got := w.baseSkillDamage(huntressSkillChar(0), rapid); got != 500 {
		t.Fatalf("Rapid Hit damage=%d, want attack 500", got)
	}
}

func TestHuntressAffectsFollowOfficialServer(t *testing.T) {
	now := time.Now()
	apply := func(learned uint32, affect model.Affect) *model.Score {
		ch := &model.Char{Class: 3, LearnedSkill: learned, Score: testScore(model.Score{
			Level: 100, Attack: 100, Defense: 100, MaxHP: 1000, MaxMP: 400,
			Mastery: [4]uint32{0, 0, 60, 0},
		})}
		affect.ExpiresAt = now.Add(time.Minute)
		ch.Affects[0] = affect
		(&World{}).applyAffectStats(ch)
		return effectiveScore(ch)
	}

	// Transfer Armor Class: AC - (Level/3 + 10); attack + Level/10 + Value,
	// plus 5 with Rapid Hit (server bit 7).
	transfer := model.Affect{Type: 21, Level: 40}
	if e := apply(0, transfer); e.Attack != 104 || e.Defense != 77 {
		t.Fatalf("Transfer Armor Class=%+v", e)
	}
	if e := apply(1<<(95-72), transfer); e.Attack != 109 {
		t.Fatalf("Transfer Armor Class with Rapid Hit attack=%d, want 109", e.Attack)
	}

	// Coin Armor: AC + Level/10 + Value, plus 100 with Spirit Change.
	coin := model.Affect{Type: 31, Level: 40, Value: 0}
	if e := apply(0, coin); e.Defense != 104 {
		t.Fatalf("Coin Armor defense=%d, want 104", e.Defense)
	}
	if e := apply(1<<(87-72), coin); e.Defense != 204 {
		t.Fatalf("Coin Armor with Spirit Change defense=%d, want 204", e.Defense)
	}

	// Spirit Change, mortal: MP/4 + (Special[2]+Level)/4 = 140 moves to HP + 750.
	if e := apply(0, model.Affect{Type: 38, Level: 40}); e.MaxHP != 1890 || e.MaxMP != 260 {
		t.Fatalf("Spirit Change HP=%d MP=%d, want 1890/260", e.MaxHP, e.MaxMP)
	}
}
