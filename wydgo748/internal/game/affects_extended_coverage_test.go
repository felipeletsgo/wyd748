package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func affectStatsFixture(affectType byte, value, level int) *model.Char {
	ch := &model.Char{Evolution: "mortal", Score: &model.Score{
		Version: model.ScoreVersion, Level: 100, Str: 300, Int: 300, Dex: 300, Con: 300,
		MaxHP: 5000, CurHP: 5000, MaxMP: 4000, CurMP: 3000,
		Mastery: [4]uint32{10, 20, 30, 40}, ResistFire: 30, ResistIce: 25, ResistHoly: 20, ResistThunder: 15,
	}}
	applyScore(ch)
	ch.RuntimeScore.Attack = 1000
	ch.RuntimeScore.MagicAttack = 800
	ch.RuntimeScore.Defense = 600
	ch.RuntimeScore.Evasion = 10
	ch.Affects[0] = model.Affect{
		Type: affectType, Value: value, Level: level,
		ExpiresAt: time.Now().Add(time.Hour),
	}
	return ch
}

func TestApplyExtendedAffectStatsRemainingCasesAndClamps(t *testing.T) {
	w := &World{}
	if w.applyAffectStats(nil) != 0 || w.applyExtendedAffectStats(&model.Char{}) != 0 {
		t.Fatal("missing character score produced affect stats")
	}

	t.Run("pursuit reduces each elemental resistance", func(t *testing.T) {
		ch := affectStatsFixture(3, 5, 0)
		w.applyExtendedAffectStats(ch)
		if ch.RuntimeScore.ResistFire != 25 || ch.RuntimeScore.ResistIce != 20 ||
			ch.RuntimeScore.ResistHoly != 15 || ch.RuntimeScore.ResistThunder != 10 {
			t.Fatalf("pursuit resistances: %+v", ch.RuntimeScore)
		}
	})

	t.Run("damage penalty cannot make attack negative", func(t *testing.T) {
		ch := affectStatsFixture(10, 2000, 100)
		w.applyExtendedAffectStats(ch)
		if ch.RuntimeScore.Attack != 0 {
			t.Fatalf("negative attack was not clamped: %d", ch.RuntimeScore.Attack)
		}
	})

	t.Run("negative multipliers clamp to zero", func(t *testing.T) {
		ch := affectStatsFixture(4, -120, 0)
		w.applyExtendedAffectStats(ch)
		if ch.RuntimeScore.Attack != 0 || ch.RuntimeScore.MagicAttack != 0 {
			t.Fatalf("negative damage multiplier was not clamped: attack=%d magic=%d",
				ch.RuntimeScore.Attack, ch.RuntimeScore.MagicAttack)
		}
	})

	t.Run("percentage overflow clamps to the score ceiling", func(t *testing.T) {
		ch := affectStatsFixture(4, 200, 0)
		ch.RuntimeScore.Attack = maxScoreValue / 2
		ch.RuntimeScore.MagicAttack = maxScoreValue / 2
		w.applyExtendedAffectStats(ch)
		if ch.RuntimeScore.Attack != maxScoreValue || ch.RuntimeScore.MagicAttack != maxScoreValue {
			t.Fatalf("percentage overflow escaped the score ceiling: attack=%d magic=%d",
				ch.RuntimeScore.Attack, ch.RuntimeScore.MagicAttack)
		}
	})

	t.Run("defense penalty applies its configured percentage", func(t *testing.T) {
		ch := affectStatsFixture(12, 20, 0)
		w.applyExtendedAffectStats(ch)
		if ch.RuntimeScore.Defense != 480 {
			t.Fatalf("defense after 20%% penalty=%d, want 480", ch.RuntimeScore.Defense)
		}
	})

	t.Run("extended defense and evasion affects apply", func(t *testing.T) {
		defense := affectStatsFixture(25, 10, 100)
		w.applyExtendedAffectStats(defense)
		if defense.RuntimeScore.Defense != 731 || defense.RuntimeScore.ResistFire != 33 {
			t.Fatalf("extended defense affect: defense=%d resist=%d",
				defense.RuntimeScore.Defense, defense.RuntimeScore.ResistFire)
		}
		evasion := affectStatsFixture(26, 2, 10)
		w.applyExtendedAffectStats(evasion)
		if evasion.RuntimeScore.Evasion != 40 {
			t.Fatalf("evasion affect=%d, want 40", evasion.RuntimeScore.Evasion)
		}
	})

	t.Run("celestial growth passive increases constitution", func(t *testing.T) {
		ch := affectStatsFixture(14, 3, 20)
		ch.Evolution = "celestial"
		ch.LearnedSkill = celestialSoulBit
		before := ch.RuntimeScore.Con
		w.applyExtendedAffectStats(ch)
		if ch.RuntimeScore.Con <= before {
			t.Fatalf("celestial growth did not add constitution: %d -> %d", before, ch.RuntimeScore.Con)
		}
	})
}
