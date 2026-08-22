package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func testScore(score model.Score) *model.Score {
	score.Version = model.ScoreVersion
	return &score
}

func testNPCDef(score model.Score) *model.NPCDef {
	return &model.NPCDef{Tipo: model.TipoMonstro, Score: testScore(score)}
}

func TestPossessedUsesWideMaxHPWithoutAccumulating(t *testing.T) {
	ch := &model.Char{Score: &model.Score{
		Version: model.ScoreVersion,
		Attack:  250_000, Defense: 200_000, Con: 150_000,
		MaxHP: 500_000, CurHP: 500_000, MaxMP: 400_000, CurMP: 400_000,
	}}
	ch.Affects[0] = model.Affect{
		Type: 14, Level: 200, Value: 10, ExpiresAt: time.Now().Add(time.Minute),
	}
	w := &World{}
	w.recalcPlayer(ch)
	firstMax := playerMaxHP(ch)
	w.recalcPlayer(ch)
	if firstMax != 501_280 || playerMaxHP(ch) != firstMax {
		t.Fatalf("Possuido wide incorreto/acumulado: primeiro=%d segundo=%d",
			firstMax, playerMaxHP(ch))
	}
	if playerCurHP(ch) != firstMax {
		t.Fatalf("Possuido nao preservou o estado cheio: %d/%d", playerCurHP(ch), firstMax)
	}
}

// TestExtendedMagicAttackScalesSkillDamage cobre a REGRA DO FELIPE (nao-vanilla):
// o MagicAttack base do personagem integra o dano das skills magicas, como o
// Attack integra o fisico. Um char com muito MagicAttack persistido deve ver a
// skill escalar com ele -- antes a formula 7.59 ignorava o MATK e a skill saia
// so com o termo de INT (bug: 500k de MATK -> skill de 600).
func TestExtendedMagicAttackScalesSkillDamage(t *testing.T) {
	newChar := func(matk uint32) *model.Char {
		ch := &model.Char{Class: 1, Score: &model.Score{
			Version: model.ScoreVersion, Level: 400, Mastery: [4]uint32{255, 255, 255, 255},
			Attack: 250_000, MagicAttack: matk, Int: 150_000,
			MaxHP: 500_000, CurHP: 500_000, MaxMP: 400_000, CurMP: 400_000,
		}}
		applyScore(ch)
		return ch
	}
	skill := model.SkillDef{Index: 24, InstanceType: 1, InstanceValue: 100}
	w := &World{}
	low := w.baseSkillDamage(newChar(0), skill)
	high := w.baseSkillDamage(newChar(300_000), skill)
	if high <= low {
		t.Fatalf("MagicAttack deveria escalar o dano da skill: matk0=%d matk300k=%d", low, high)
	}
	// O MATK (300k) entra no core e ainda e amplificado (>=1x): o ganho nao pode
	// ser menor que o proprio MATK.
	if high-low < 300_000 {
		t.Fatalf("MagicAttack nao entrou no dano da skill: delta=%d", high-low)
	}
}

func TestApplyBonusConsumesExtendedPointsAndRaisesWideAttribute(t *testing.T) {
	ch := &model.Char{
		Score: &model.Score{
			Version: model.ScoreVersion, Level: 400,
			Str: 8, Int: 4, Dex: 7, Con: 6,
			StatusPts: 3_372, MaxHP: 80, CurHP: 80, MaxMP: 45, CurMP: 45,
		},
	}
	applyScore(ch)
	if !applyBonus(ch, 0, 0) {
		t.Fatal("bonus STR wide recusado")
	}
	(&World{}).recalcPlayer(ch)
	if ch.Score.Str != 108 || ch.Score.Attack != 0 ||
		ch.Score.StatusPts != 3_272 || playerAttack(ch) == 0 {
		t.Fatalf("bonus wide incorreto: STR=%d ATK=%d pontos=%d",
			ch.Score.Str, ch.Score.Attack, ch.Score.StatusPts)
	}
}

func TestExtendedAttributesUpdateDerivedResources(t *testing.T) {
	ch := &model.Char{Class: 1, Score: &model.Score{
		Version: model.ScoreVersion, Level: 400,
		MaxHP: 60, CurHP: 60, MaxMP: 65, CurMP: 65,
		Str: 5, Int: 8, Dex: 5, Con: 5, StatusPts: 3_372,
	}}
	applyScore(ch)
	if !applyBonus(ch, 0, 1) { // +100 INT
		t.Fatal("bonus INT wide recusado")
	}
	w := &World{}
	w.recalcPlayer(ch)
	if playerInt(ch) != 108 || playerMagicAttack(ch) != 298 || playerMaxMP(ch) != 1_465 {
		t.Fatalf("INT nao seguiu formula 7.59/MP wide: stats=%+v runtime=%+v",
			*ch.Score, *ch.RuntimeScore)
	}
	if !applyBonus(ch, 0, 3) { // +100 CON
		t.Fatal("bonus CON wide recusado")
	}
	w.recalcPlayer(ch)
	if playerCon(ch) != 105 || playerMaxHP(ch) != 660 {
		t.Fatalf("CON nao atualizou HP por recálculo: %+v", *ch.RuntimeScore)
	}
}
