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

// The active 7.48 books define mastery and finisher ownership.
func TestHuntressSkillDamageUses748TreeOrder(t *testing.T) {
	w := &World{}
	knife := model.SkillDef{Index: 94, InstanceType: 1, InstanceValue: 35}
	// Capture mastery is 90, distinct from Trade 60 and Survival 30.
	if got := w.baseSkillDamage(huntressSkillChar(0), knife); got != 593 {
		t.Fatalf("Throwing Knife damage=%d, want 593", got)
	}
	if got := w.baseSkillDamage(huntressSkillChar(1<<(87-72)), knife); got != 593 {
		t.Fatalf("Trade finisher changed Capture damage=%d", got)
	}
	if got := w.baseSkillDamage(huntressSkillChar(1<<(95-72)), knife); got != 711 {
		t.Fatalf("Capture finisher damage=%d, want 711", got)
	}

	// Snick Attack is Capture; Hide in Shadow cannot boost it.
	snick := model.SkillDef{Index: 88, InstanceType: 1, InstanceValue: 12}
	plain := w.baseSkillDamage(huntressSkillChar(0), snick)
	if got := w.baseSkillDamage(huntressSkillChar(1<<(75-72)), snick); got != plain {
		t.Fatalf("Snick Attack with Hide in Shadow=%d, want %d", got, plain)
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
	// plus 5 with the Survival finisher (7.48 bit 7).
	transfer := model.Affect{Type: 21, Level: 40}
	if e := apply(0, transfer); e.Attack != 104 || e.Defense != 77 {
		t.Fatalf("Transfer Armor Class=%+v", e)
	}
	if e := apply(1<<(79-72), transfer); e.Attack != 109 {
		t.Fatalf("Meditation with Survival finisher attack=%d, want 109", e.Attack)
	}
	if e := apply(1<<(95-72), transfer); e.Attack != 104 {
		t.Fatalf("Capture finisher changed Meditation attack=%d", e.Attack)
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
