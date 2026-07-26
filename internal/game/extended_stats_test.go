package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func testExtended(score model.ExtendedScore) *model.ExtendedScore {
	score.Version = model.ExtendedScoreVersion
	return &score
}

func testNPCDef(score model.ExtendedScore) *model.NPCDef {
	return &model.NPCDef{Tipo: model.TipoMonstro, Extended: testExtended(score)}
}

func TestApplyExtendedScoreProjectsCompatibilityWithoutLosingWideValues(t *testing.T) {
	ch := &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Attack:  150_000, MagicAttack: 175_000, Defense: 125_000,
		MaxHP: 250_000, MaxMP: 200_000, CurHP: 225_000, CurMP: 180_000,
		Str: 110_000, Int: 120_000, Dex: 130_000, Con: 140_000,
		Accuracy: 160_000, Evasion: 105_000,
	}}
	applyExtendedScore(ch)
	compat := wireExtendedScore(ch).CompatibilityScore()

	if ch.Extended.Attack != 150_000 || playerAttack(ch) != 150_000 ||
		playerMagicAttack(ch) != 175_000 || compat.Attack != 1_000 ||
		compat.Defense != 1_000 {
		t.Fatalf("stats wide/projecao incorretos: ext=%+v score=%+v",
			*ch.Extended, compat)
	}
	if compat.CurHP >= compat.MaxHP || compat.CurMP >= compat.MaxMP {
		t.Fatalf("proporcao de vitais perdida: HP=%d/%d MP=%d/%d",
			compat.CurHP, compat.MaxHP, compat.CurMP, compat.MaxMP)
	}
	if compat.MaxHP > 30_000 || compat.MaxMP > 30_000 || compat.Str > 1_000 {
		t.Fatalf("projecao legada ultrapassou short assinado seguro: score=%+v", compat)
	}
}

func TestExtendedResourcesNeverFallBackToCompatibilityProjection(t *testing.T) {
	ch := &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		MaxHP:   500_000, CurHP: 400_000,
		MaxMP: 400_000, CurMP: 300_000,
	}}
	applyExtendedScore(ch)
	if !spendPlayerMP(ch, 25_000) {
		t.Fatal("mana wide recusada")
	}
	restorePlayerHP(ch, 50_000)
	if playerCurHP(ch) != 450_000 || playerCurMP(ch) != 275_000 {
		t.Fatalf("recursos reais foram projetados: hp=%d mp=%d score=%+v",
			playerCurHP(ch), playerCurMP(ch), wireExtendedScore(ch).CompatibilityScore())
	}
	if ch.Extended.CurHP != 450_000 || ch.Extended.CurMP != 275_000 {
		t.Fatalf("base persistida nao acompanhou recursos: %+v", *ch.Extended)
	}
	if playerCombatMP(ch) != wire.CompatibilityCombatMP(wireExtendedScore(ch)) || playerCombatMP(ch) >= playerCurMP(ch) {
		t.Fatalf("canal de combate nao usa exclusivamente a projecao: wire=%d real=%d score=%d",
			playerCombatMP(ch), playerCurMP(ch), wire.CompatibilityCombatMP(wireExtendedScore(ch)))
	}
}

func TestPossessedUsesWideMaxHPWithoutAccumulating(t *testing.T) {
	ch := &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
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
	if playerCurHP(ch) != 500_000 {
		t.Fatalf("Possuido derrubou HP real para %d", playerCurHP(ch))
	}
}

// TestExtendedMagicAttackScalesSkillDamage cobre a REGRA DO FELIPE (nao-vanilla):
// o MagicAttack base do personagem integra o dano das skills magicas, como o
// Attack integra o fisico. Um char com muito MagicAttack persistido deve ver a
// skill escalar com ele -- antes a formula 7.59 ignorava o MATK e a skill saia
// so com o termo de INT (bug: 500k de MATK -> skill de 600).
func TestExtendedMagicAttackScalesSkillDamage(t *testing.T) {
	newChar := func(matk uint32) *model.Char {
		ch := &model.Char{Class: 1, Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion, Level: 400, Mastery: [4]uint32{255, 255, 255, 255},
			Attack: 250_000, MagicAttack: matk, Int: 150_000,
			MaxHP: 500_000, CurHP: 500_000, MaxMP: 400_000, CurMP: 400_000,
		}}
		applyExtendedScore(ch)
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
		Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion, Level: 400,
			Str: 8, Int: 4, Dex: 7, Con: 6,
			StatusPts: 3_372, MaxHP: 80, CurHP: 80, MaxMP: 45, CurMP: 45,
		},
	}
	applyExtendedScore(ch)
	if !applyBonus(ch, 0, 0) {
		t.Fatal("bonus STR wide recusado")
	}
	(&World{}).recalcPlayer(ch)
	if ch.Extended.Str != 108 || ch.Extended.Attack != 0 ||
		ch.Extended.StatusPts != 3_272 || playerAttack(ch) == 0 {
		t.Fatalf("bonus wide incorreto: STR=%d ATK=%d pontos=%d",
			ch.Extended.Str, ch.Extended.Attack, ch.Extended.StatusPts)
	}
}

func TestExtendedAttributesUpdateDerivedResources(t *testing.T) {
	ch := &model.Char{Class: 1, Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, Level: 400,
		MaxHP: 60, CurHP: 60, MaxMP: 65, CurMP: 65,
		Str: 5, Int: 8, Dex: 5, Con: 5, StatusPts: 3_372,
	}}
	applyExtendedScore(ch)
	if !applyBonus(ch, 0, 1) { // +100 INT
		t.Fatal("bonus INT wide recusado")
	}
	w := &World{}
	w.recalcPlayer(ch)
	if playerInt(ch) != 108 || playerMagicAttack(ch) != 298 || playerMaxMP(ch) != 1_465 {
		t.Fatalf("INT nao seguiu formula 7.59/MP wide: stats=%+v runtime=%+v",
			*ch.Extended, *ch.ExtendedRuntime)
	}
	if !applyBonus(ch, 0, 3) { // +100 CON
		t.Fatal("bonus CON wide recusado")
	}
	w.recalcPlayer(ch)
	if playerCon(ch) != 105 || playerMaxHP(ch) != 660 {
		t.Fatalf("CON nao atualizou HP por recálculo: %+v", *ch.ExtendedRuntime)
	}
}

func TestRepeatedWideSkillsKeepRealManaAndProjectedClientChannelInSync(t *testing.T) {
	ch := &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		MaxHP:   500_000, CurHP: 450_000,
		MaxMP: 400_000, CurMP: 369_867,
	}}
	applyExtendedScore(ch)

	for cast := 0; cast < 20; cast++ {
		if !spendPlayerMP(ch, 227) {
			t.Fatalf("cast %d recusado com MP real=%d", cast, playerCurMP(ch))
		}
		projected := playerCombatMP(ch)
		if projected == 0 || projected > 30_000 {
			t.Fatalf("cast %d gerou MP wire invalido=%d", cast, projected)
		}

		hit := wire.SkillHitExtended(1, 1000, 10, 10, 11, 10,
			300_000, 0, 0, projected, 7, 5, 255)
		if got := uint32(binary.LittleEndian.Uint16(hit[26:28])); got != projected {
			t.Fatalf("cast %d truncou MP no 0x39D: got=%d want=%d", cast, got, projected)
		}

		score := wire.UpdateScore(1, *ch)
		if got := binary.LittleEndian.Uint32(score[116:120]); got != playerCurMP(ch) {
			t.Fatalf("cast %d perdeu MP real no 0x336: got=%d want=%d",
				cast, got, playerCurMP(ch))
		}
		if got := binary.LittleEndian.Uint16(score[26:28]); uint32(got) != projected {
			t.Fatalf("cast %d perdeu projecao no prefixo 0x336: got=%d want=%d",
				cast, got, projected)
		}
	}

	vitals := wire.SetHpMpExtended(1, wireExtendedScore(ch))
	if got := binary.LittleEndian.Uint32(vitals[20:24]); got != playerCurHP(ch) {
		t.Fatalf("HP real ausente do 0x181: got=%d want=%d", got, playerCurHP(ch))
	}
	_, projectedHP, _ := wire.CompatibilityVitals(wireExtendedScore(ch))
	if got := binary.LittleEndian.Uint16(vitals[12:14]); got != projectedHP {
		t.Fatalf("barra de HP nao recebeu projecao: got=%d want=%d", got, projectedHP)
	}
}
