package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// Values from the runtime SkillData.csv. Mana Control's formula is audited
// separately; this fixture only supplies its cast and affect metadata.
func foemaBuffTestSkills() []model.SkillDef {
	return []model.SkillDef{
		{Index: 41, Name: "Velocidade", ManaSpent: 52, Delay: 5, Range: 0,
			MaxTarget: 13, Party: 1, AffectType: 2, AffectValue: 1, AffectTime: 600},
		{Index: 43, Name: "Escudo Magico", ManaSpent: 52, Delay: 6, Range: 2,
			MaxTarget: 1, AffectType: 11, AffectValue: 15, AffectTime: 600},
		{Index: 44, Name: "Arma Magica", ManaSpent: 78, Delay: 5, Range: 0,
			MaxTarget: 13, Party: 1, AffectType: 9, AffectValue: 5, AffectTime: 600},
		{Index: 46, Name: "Controle de Mana", ManaSpent: 130, Delay: 0, Range: 0,
			MaxTarget: 1, AffectType: 18, AffectValue: 100, AffectTime: 600},
	}
}

func TestSkillAffectUsesSkillDataWithoutIndexOverride(t *testing.T) {
	kind, value, ok := skillAffect(model.SkillDef{Index: 3, AffectType: 99, AffectValue: 321})
	if !ok || kind != 99 || value != 321 {
		t.Fatalf("skillAffect=%d,%d,%v; esperado SkillData 99,321,true", kind, value, ok)
	}
}

func TestTKAffectFormulas759(t *testing.T) {
	w := &World{}
	base := model.Score{Level: 10, Attack: 100, Defense: 100, MaxHP: 1000,
		CurHP: 1000, Str: 8, Int: 4, Dex: 7, Con: 6}
	tests := []struct {
		affect     model.Affect
		attack, ac uint16
		hp, con    uint16
	}{
		{model.Affect{Type: 13, Value: 7, Level: 40}, 111, 90, 1000, 6},
		{model.Affect{Type: 14, Value: 10, Level: 40}, 100, 100, 1320, 46},
		{model.Affect{Type: 24, Value: 0, Level: 40}, 100, 125, 1000, 6},
		// Official TMSRVIN: Coin Armor adds Level/10 + Value.
		{model.Affect{Type: 31, Value: 150, Level: 40}, 100, 254, 1000, 6},
	}
	for _, tt := range tests {
		ch := &model.Char{Class: 0, Score: testScore(base)}
		tt.affect.ExpiresAt = time.Now().Add(time.Minute)
		ch.Affects[0] = tt.affect
		w.applyAffectStats(ch)
		e := effectiveScore(ch)
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

func TestAthenaMasteryCapAndLifecycle(t *testing.T) {
	now := time.Date(2026, 9, 16, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(now)
	w := &World{clock: clock}
	ch := &model.Char{Class: 1, Score: testScore(model.Score{
		Level: 399, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
		Mastery: [4]uint32{300, 319, 320, 100},
	})}
	w.recalcPlayer(ch)
	baseMastery := ch.Score.Mastery
	before := ch.RuntimeScore.Mastery
	p := &Player{ID: 1, InWorld: true, Char: ch, Session: &net.Session{ID: 1}}
	result := w.applySupportSkill(p, skillCastRequest{}, model.SkillDef{
		Index: 45, Name: "Toque da Athena", AffectType: 15,
		AffectValue: 7, AffectTime: 10,
	}, 320) // bonus 32 + 7; nunca ultrapassa o teto autoritativo de 320.
	if len(result) != 1 {
		t.Fatal("Athena nao foi aplicada")
	}
	want := [4]uint32{320, 320, 320, 139}
	for recalc := 0; recalc < 3; recalc++ {
		if got := ch.RuntimeScore.Mastery; got != want {
			t.Fatalf("recalc %d: mastery=%v, want %v", recalc, got, want)
		}
		if ch.Score.Mastery != baseMastery {
			t.Fatal("Athena alterou as especializacoes base")
		}
		w.recalcPlayer(ch)
	}
	clock.Advance(ch.Affects[0].ExpiresAt.Sub(now))
	w.recalcPlayer(ch)
	if got := ch.RuntimeScore.Mastery; got != before {
		t.Fatalf("Athena expirada deixou bonus: mastery=%v, want %v", got, before)
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

func TestBMTransformationUsesNative748Interpolation(t *testing.T) {
	now := time.Now()
	ch := &model.Char{Class: 2, LearnedSkill: 1 << (69 - 48), Score: testScore(model.Score{
		Attack: 100, Defense: 100, MaxHP: 1000, AttackRun: 0x11, Mastery: [4]uint32{0, 0, 120, 0},
	})}
	ch.Affects[0] = model.Affect{Type: 16, Value: 1, Level: 200, ExpiresAt: now.Add(time.Minute)}
	(&World{}).applyAffectStats(ch)
	e := effectiveScore(ch)
	if e.Attack != 140 || e.Defense != 110 || e.MaxHP != 1050 || e.ResistFire != 5 {
		t.Fatalf("transformacao lobo incorreta: %+v", e)
	}
	// 50 + 5*(120/50) = 60 (6.0%).
	if e.Critical != 60 || bodyMesh(ch)[0] != 22 {
		t.Fatalf("bonus/visual da transformacao incorreto: extended=%+v mesh=%v", e, bodyMesh(ch)[0])
	}
}

// Official TMSRVIN sub_4BB4C0: Wolf, Bear and Astaroth bonuses need skill 69;
// Titan and Eden bonuses are unconditional; learned Eden (71) adds 5%/2%.
func TestBMTransformationBonusesFollowOfficialServer(t *testing.T) {
	type want struct{ attack, defense, hp, resist, run, attackSpeed int }
	cases := []struct {
		name    string
		form    int
		learned uint32
		want    want
	}{
		{"wolf without 69", 1, 1<<(65-48) | 1<<(67-48), want{130, 110, 1050, 0, 2, 30}},
		{"bear without 69", 2, 1 << (67 - 48), want{105, 115, 1400, 0, 1, 10}},
		{"bear with 69", 2, 1 << (69 - 48), want{105, 115, 1600, 20, 1, 30}},
		{"astaroth with 69", 3, 1 << (69 - 48), want{130, 115, 1250, 5, 2, 50}},
		{"titan", 4, 0, want{125, 125, 1150, 20, 1, 30}},
		{"eden", 5, 0, want{133, 120, 1330, 17, 3, 40}},
		{"titan with 71", 4, 1 << (71 - 48), want{130, 127, 1150, 20, 1, 30}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			now := time.Now()
			ch := &model.Char{Class: 2, LearnedSkill: tc.learned, Score: testScore(model.Score{
				Attack: 100, Defense: 100, MaxHP: 1000, AttackRun: 0x11,
			})}
			ch.Affects[0] = model.Affect{Type: 16, Value: tc.form, Level: 200, ExpiresAt: now.Add(time.Minute)}
			attackSpeed := (&World{}).applyAffectStats(ch)
			e := effectiveScore(ch)
			got := want{int(e.Attack), int(e.Defense), int(e.MaxHP), int(e.ResistFire),
				int(e.AttackRun & 0x0F), attackSpeed}
			if got != tc.want {
				t.Fatalf("form %d: got %+v, want %+v", tc.form, got, tc.want)
			}
			if tc.form != 1 && e.Critical != 0 {
				t.Fatalf("form %d added critical %d", tc.form, e.Critical)
			}
		})
	}
}

func TestBMTransformationKeepsFullHPWhenAnotherBuffRecalculatesScore(t *testing.T) {
	now := time.Now()
	w := &World{clock: newFakeClock(now)}
	ch := &model.Char{Class: 2, Score: testScore(model.Score{
		Level: 100, MaxHP: 70, CurHP: 70, MaxMP: 55, CurMP: 55,
		Int: 6, Con: 5, AttackRun: 0x11,
	})}
	// Homem Urso (transformacao BM, affect 16/value 2) ja esta ativo.
	ch.Affects[0] = model.Affect{
		Type: 16, Value: 2, Level: 239, ExpiresAt: now.Add(time.Minute),
	}
	w.recalcPlayer(ch)
	transformedMax := playerMaxHP(ch)
	setPlayerCurHP(ch, transformedMax)
	if transformedMax <= ch.Score.MaxHP {
		t.Fatalf("pre-condicao: transformacao nao elevou MaxHP: base=%d runtime=%d",
			ch.Score.MaxHP, transformedMax)
	}

	// Passe pelo caminho vivo de uma skill de suporte posterior. Protecao e
	// Forca Elemental entram por esta mesma fronteira e chamam recalcPlayer.
	p := &Player{ID: 1, InWorld: true, Char: ch, Session: &net.Session{ID: 1}}
	result := w.applySupportSkill(p, skillCastRequest{}, model.SkillDef{
		Index: 52, Name: "Elemental Protection", AffectType: 11,
		AffectValue: 10, AffectTime: 8,
	}, 255)
	if len(result) != 1 {
		t.Fatalf("buff de suporte nao foi aplicado: %+v", result)
	}

	if got := playerCurHP(ch); got != transformedMax {
		t.Fatalf("buff derrubou HP transformado: atual=%d, quer=%d (base sem transformacao=%d)",
			got, transformedMax, ch.Score.MaxHP)
	}
	if playerCurHP(ch) != playerMaxHP(ch) {
		t.Fatalf("personagem deixou de estar cheio: %d/%d", playerCurHP(ch), playerMaxHP(ch))
	}
	if ch.Score.CurHP > ch.Score.MaxHP {
		t.Fatalf("persistencia recebeu HP impossivel: %d/%d", ch.Score.CurHP, ch.Score.MaxHP)
	}

	// O mesmo recalc nao pode funcionar como cura para um BM ferido.
	const damagedHP = uint32(100)
	setPlayerCurHP(ch, damagedHP)
	w.recalcPlayer(ch)
	if got := playerCurHP(ch); got != damagedHP {
		t.Fatalf("buff curou/feriu BM que nao estava cheio: atual=%d, quer=%d", got, damagedHP)
	}
}

// Native SetAffect overwrites a BM form in either direction. Eden (5) must not
// lock out the lower forms, and a lower form must not block a higher one.
func TestBMTransformationSwitchesFormsInEitherDirection(t *testing.T) {
	forms := map[int]int{64: 1, 66: 2, 68: 3, 70: 4, 71: 5}
	faces := map[int]uint16{1: 22, 2: 23, 3: 24, 4: 25, 5: 32}
	for _, seq := range [][2]int{{71, 64}, {64, 71}, {70, 68}, {71, 66}} {
		w, p, _ := handlerTestWorld(t)
		w.clock = newFakeClock(time.Now()) // bodyMesh reads the wall clock.
		p.Char.Class = 2
		for _, skillIndex := range seq {
			skill := model.SkillDef{Index: skillIndex, AffectType: affectBMTransform, AffectValue: forms[skillIndex]}
			if got := w.applySupportSkillToTargets(p, []*Player{p}, skill, 100); len(got) != 1 {
				t.Fatalf("%v: skill %d was not applied", seq, skillIndex)
			}
		}
		want := forms[seq[1]]
		count := 0
		for _, affect := range p.Char.Affects {
			if affect.Type == affectBMTransform {
				count++
				if affect.Value != want {
					t.Fatalf("%v: form %d, want %d", seq, affect.Value, want)
				}
			}
		}
		if count != 1 {
			t.Fatalf("%v: %d transformation slots, want 1", seq, count)
		}
		if got := bodyMesh(p.Char)[0]; got != faces[want] {
			t.Fatalf("%v: face %d, want %d", seq, got, faces[want])
		}
	}
}
