package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestSkillAffectUsesSkillDataWithoutIndexOverride(t *testing.T) {
	kind, value, ok := skillAffect(model.SkillDef{Index: 3, AffectType: 99, AffectValue: 321})
	if !ok || kind != 99 || value != 321 {
		t.Fatalf("skillAffect=%d,%d,%v; esperado SkillData 99,321,true", kind, value, ok)
	}
}

func TestTKAffectFormulas759(t *testing.T) {
	w := &World{}
	base := model.ExtendedScore{Level: 10, Attack: 100, Defense: 100, MaxHP: 1000,
		CurHP: 1000, Str: 8, Int: 4, Dex: 7, Con: 6}
	tests := []struct {
		affect     model.Affect
		attack, ac uint16
		hp, con    uint16
	}{
		{model.Affect{Type: 13, Value: 7, Level: 40}, 111, 90, 1000, 6},
		{model.Affect{Type: 14, Value: 10, Level: 40}, 100, 100, 1320, 46},
		{model.Affect{Type: 24, Value: 0, Level: 40}, 100, 125, 1000, 6},
		{model.Affect{Type: 31, Value: 150, Level: 40}, 100, 270, 1000, 6},
	}
	for _, tt := range tests {
		ch := &model.Char{Class: 0, Extended: testExtended(base)}
		tt.affect.ExpiresAt = time.Now().Add(time.Minute)
		ch.Affects[0] = tt.affect
		w.applyAffectStats(ch)
		e := effectiveExtended(ch)
		if e.Attack != uint32(tt.attack) || e.Defense != uint32(tt.ac) ||
			e.MaxHP != uint32(tt.hp) || e.Con != uint32(tt.con) {
			t.Fatalf("affect=%d score=%+v", tt.affect.Type, e)
		}
	}
}

func TestSetAffectRefreshesSameType(t *testing.T) {
	ch := &model.Char{}
	if !setAffect(ch, 13, 7, 10, 2) || !setAffect(ch, 13, 9, 20, 3) {
		t.Fatal("nao aplicou/renovou affect")
	}
	count := 0
	for _, a := range ch.Affects {
		if a.Type == 13 {
			count++
			if a.Value != 9 || a.Level != 20 {
				t.Fatalf("renovacao incorreta: %+v", a)
			}
		}
	}
	if count != 1 {
		t.Fatalf("affect duplicado: %d", count)
	}
}

func TestSetAffectRejectsWeakerOrShorterReplacement(t *testing.T) {
	now := time.Now()
	for _, tc := range []struct {
		name                   string
		value, level, duration int
	}{
		{"mais fraco", 8, 19, 4},
		{"valor menor", 8, 20, 4},
		{"mais curto", 9, 20, 2},
	} {
		t.Run(tc.name, func(t *testing.T) {
			ch := &model.Char{}
			ch.Affects[0] = model.Affect{Type: 13, Value: 9, Level: 20, ExpiresAt: now.Add(24 * time.Second)}
			if setAffect(ch, 13, tc.value, tc.level, tc.duration) {
				t.Fatal("substituicao degradante foi aceita")
			}
			if ch.Affects[0].Value != 9 || ch.Affects[0].Level != 20 {
				t.Fatalf("affect existente foi alterado: %+v", ch.Affects[0])
			}
		})
	}
}

func TestBMTransformationUsesW2PPInterpolation(t *testing.T) {
	now := time.Now()
	ch := &model.Char{Class: 2, LearnedSkill: 1 << (65 - 48), Extended: testExtended(model.ExtendedScore{
		Attack: 100, Defense: 100, MaxHP: 1000, AttackRun: 0x11,
	})}
	ch.Affects[0] = model.Affect{Type: 16, Value: 1, Level: 200, ExpiresAt: now.Add(time.Minute)}
	(&World{}).applyAffectStats(ch)
	e := effectiveExtended(ch)
	if e.Attack != 180 || e.Defense != 105 || e.MaxHP != 1050 {
		t.Fatalf("transformacao lobo incorreta: %+v", e)
	}
	if e.Critical != 11 || bodyMesh(ch)[0] != 22 {
		t.Fatalf("bonus/visual da transformacao incorreto: extended=%+v mesh=%v", e, bodyMesh(ch)[0])
	}
}

func TestMobPublicExtendedDoesNotTruncateAuthoritativeHP(t *testing.T) {
	def := testNPCDef(model.ExtendedScore{
		Attack: 300_000, Defense: 250_000, MaxHP: 1_000_000, MaxMP: 500_000,
	})
	mob := &Mob{Def: def, HP: 750_000}
	extended := mobPublicExtended(mob)
	if mob.HP != 750_000 || def.Extended.MaxHP != 1_000_000 {
		t.Fatalf("projecao alterou estado autoritativo: hp=%d max=%d", mob.HP, def.Extended.MaxHP)
	}
	score := extended.CompatibilityScore()
	if extended.Attack != 300_000 || extended.Defense != 250_000 ||
		score.Attack != 1_000 || score.Defense != 1_000 ||
		uint32(score.CurHP)*4 < uint32(score.MaxHP)*3-1 ||
		uint32(score.CurHP)*4 > uint32(score.MaxHP)*3+1 {
		t.Fatalf("score visual do mob incorreto: %+v", score)
	}
}
