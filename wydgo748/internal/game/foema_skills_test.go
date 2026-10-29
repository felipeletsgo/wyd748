package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// Official TMSRVIN InstanceType 6: Heal and Recovery without the old x1.4
// and 548 cap; Bless Revival (31) adds x1.2 to Recovery and, for a
// Celestial, the caster's level.
func TestFoemaHealFollowsOfficialServer(t *testing.T) {
	plain := &model.Char{Class: 1, Score: testScore(model.Score{Level: 300})}
	if got := foemaHealAmount(plain, 27, 200, 100); got != 500 {
		t.Fatalf("Heal=%d, want 100 + 2*200", got)
	}
	if got := foemaHealAmount(plain, 29, 200, 150); got != 450 {
		t.Fatalf("Recovery=%d, want 150 + 3*200/2", got)
	}
	revival := &model.Char{Class: 1, LearnedSkill: 1 << 7, Score: testScore(model.Score{Level: 300})}
	if got := foemaHealAmount(revival, 29, 200, 150); got != 540 {
		t.Fatalf("Recovery with Bless Revival=%d, want 450 x 1.2", got)
	}
	revival.Evolution = "celestial"
	if got := foemaHealAmount(revival, 27, 200, 100); got != 800 {
		t.Fatalf("Celestial Heal with Bless Revival=%d, want 500 + level 300", got)
	}
}

func TestHolyJudgementKeepsTwoThirdsOfHP(t *testing.T) {
	ch := &model.Char{Class: 1, Score: testScore(model.Score{MaxHP: 3000, CurHP: 3000})}
	if got := holyJudgementSacrifice(ch); got != 3000 || playerCurHP(ch) != 2000 {
		t.Fatalf("Holy Judgement bonus=%d hp=%d, want 3000/2000", got, playerCurHP(ch))
	}
	ch.LearnedSkill = 1 << 7
	if got := holyJudgementSacrifice(ch); got != 2200 || playerCurHP(ch) != 1333 {
		t.Fatalf("Holy Judgement with Bless Revival bonus=%d hp=%d, want 2200/1333", got, playerCurHP(ch))
	}
}

func TestBlessRevivalChance(t *testing.T) {
	for mastery, want := range map[int]int{0: 30, 45: 36, 255: 64, 320: 72, 999: 90} {
		if got := foemaRevivalChance(mastery); got != want {
			t.Fatalf("revival chance mastery=%d: %d, want %d", mastery, got, want)
		}
	}
}

// Official TMSRVIN and native 7.48 FUN_0054FDB9: the holder's class and
// learned bits 20 (Magic Weapon) and 23 (tree finisher) scale the buffs.
func TestFoemaBuffBonusesUseHolderSkills(t *testing.T) {
	now := time.Now()
	apply := func(class byte, learned uint32, affect model.Affect) *model.Score {
		ch := &model.Char{Class: class, LearnedSkill: learned, Score: testScore(model.Score{
			Attack: 1000, Defense: 100, Mastery: [4]uint32{100, 100, 100, 100},
		})}
		affect.ExpiresAt = now.Add(time.Minute)
		ch.Affects[0] = affect
		(&World{}).applyAffectStats(ch)
		return effectiveScore(ch)
	}
	weapon := model.Affect{Type: 9, Level: 60, Value: 5}
	if e := apply(0, 0, weapon); e.Attack != 1086 {
		t.Fatalf("Magic Weapon on others=%d, want (1000 + 35) x 1.05", e.Attack)
	}
	if e := apply(1, 1<<20, weapon); e.Attack != 1410 {
		t.Fatalf("Magic Weapon on its Foema=%d, want (1000 + 175) x 1.2", e.Attack)
	}
	if e := apply(1, 1<<20|1<<23, weapon); e.Attack != 1492 {
		t.Fatalf("Magic Weapon with bit 23=%d, want (1000 + 175) x 1.27", e.Attack)
	}
	shield := model.Affect{Type: 11, Level: 60, Value: 15}
	if e := apply(1, 1<<23, shield); e.Defense != 235 {
		t.Fatalf("Magic Shield with bit 23=%d, want 100 + 35 + 100", e.Defense)
	}
	athena := model.Affect{Type: 15, Level: 100, Value: 7}
	if e := apply(1, 1<<23, athena); e.Mastery[1] != 120 {
		t.Fatalf("Athena with bit 23 mastery=%d, want 100 + 17 x 1.2", e.Mastery[1])
	}
}

func TestCureRemovesAffect33(t *testing.T) {
	ch := &model.Char{}
	ch.Affects[0] = model.Affect{Type: 33, ExpiresAt: time.Now().Add(time.Minute)}
	if !cleansePlayer(ch, false) || ch.Affects[0].Type != 0 {
		t.Fatal("Cure kept affect 33")
	}
}

func TestTeleportRejectsTargetsAboveLevelLimit(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Foema", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Member", 2210, 2200)
	party := &Party{Members: []*Player{caster, target}}
	caster.Party, target.Party = party, party
	setLevel := func(p *Player, level uint32) {
		p.Char.Score.Level = level
		if p.Char.RuntimeScore != nil {
			p.Char.RuntimeScore.Level = level
		}
	}
	setLevel(caster, 100)
	setLevel(target, 251)
	w := testSpatialWorld(nil, caster, target)
	skill := model.SkillDef{Index: 42, InstanceType: 9, Range: 20, MaxTarget: 1}
	if got := w.applySupportSkill(caster, skillCastRequest{TargetID: target.ID}, skill, 50); len(got) != 0 {
		t.Fatalf("Teleport moved a level 251 target with limit 250: %+v", got)
	}
	setLevel(target, 250)
	if got := w.applySupportSkill(caster, skillCastRequest{TargetID: target.ID}, skill, 50); len(got) != 1 {
		t.Fatal("Teleport rejected a target at the level limit")
	}
}

// Official TMSRVIN sub_575830: Cancellation removes one Immunity from an
// immune target and applies affect 32 only to a target without Immunity.
func TestCancellationBreaksImmunityOrBlocksPotions(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Caster", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Target", 2201, 2200)
	w := testSpatialWorld(nil, caster, target)
	skill := model.SkillDef{Index: cancellationSkill, TargetType: 1, Aggressive: 1,
		AffectType: affectCancellation, AffectTime: 1, MaxTarget: 1}
	now := w.now()
	target.Char.Affects[0] = model.Affect{Type: 19, ExpiresAt: now.Add(time.Minute)}
	w.executePlayerSkill(caster, []*Player{target}, skill, 100, 0)
	if target.Char.Affects[0].Type != 0 || activePlayerAffectAt(target.Char, affectCancellation, now) != nil {
		t.Fatalf("immune target: %+v", target.Char.Affects[:2])
	}
	w.executePlayerSkill(caster, []*Player{target}, skill, 100, 0)
	if activePlayerAffectAt(target.Char, affectCancellation, now) == nil {
		t.Fatal("Cancellation did not apply affect 32 without Immunity")
	}
}
