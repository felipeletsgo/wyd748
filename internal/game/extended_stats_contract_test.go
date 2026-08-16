package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestExtendedStatHelpersCoverNilRuntimeAndPersistedFallbacks(t *testing.T) {
	if playerLevel(nil) != 0 || playerMastery(nil, 0) != 0 || playerSkillPoints(nil) != 0 ||
		playerAttackRun(nil) != 0 || playerCurHP(nil) != 0 || playerMaxHP(nil) != 0 ||
		playerCurMP(nil) != 0 || playerMaxMP(nil) != 0 || playerAttack(nil) != 0 ||
		playerMagicAttack(nil) != 0 || playerDefense(nil) != 0 || playerStr(nil) != 0 ||
		playerInt(nil) != 0 || playerDex(nil) != 0 || playerCon(nil) != 0 || wireExtendedScore(nil) != nil {
		t.Fatal("helpers nil nao retornaram zeros")
	}

	base := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Level: 9, Attack: 10, MagicAttack: 11, Defense: 12,
		MaxHP: 100, CurHP: 80, MaxMP: 70, CurMP: 60,
		Str: 13, Int: 14, Dex: 15, Con: 16,
		SkillPts: 17, Mastery: [4]uint32{18, 19, 20, 21}, AttackRun: 22,
	}
	ch := &model.Char{Extended: base}
	if playerLevel(ch) != 9 || playerMastery(ch, 2) != 20 || playerSkillPoints(ch) != 17 ||
		playerAttackRun(ch) != 22 || playerCurHP(ch) != 80 || playerMaxHP(ch) != 100 ||
		playerCurMP(ch) != 60 || playerMaxMP(ch) != 70 || playerAttack(ch) != 10 ||
		playerMagicAttack(ch) != 11 || playerDefense(ch) != 12 || playerStr(ch) != 13 ||
		playerInt(ch) != 14 || playerDex(ch) != 15 || playerCon(ch) != 16 || wireExtendedScore(ch) != base {
		t.Fatal("helpers nao leram Extended persistido")
	}
	if playerMastery(ch, -1) != 0 || playerMastery(ch, 4) != 0 {
		t.Fatal("mastery fora de 0..3 nao foi recusada")
	}

	runtime := *base
	runtime.Level = 99
	runtime.Attack = 98
	ch.ExtendedRuntime = &runtime
	if playerLevel(ch) != 99 || playerAttack(ch) != 98 || effectiveExtended(ch) != ch.ExtendedRuntime {
		t.Fatal("runtime efetivo nao teve precedencia")
	}
}

func TestEnsureExtendedScorePanicsOnBrokenAuthoritativeState(t *testing.T) {
	for _, ch := range []*model.Char{
		{},
		{Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion + 1}},
	} {
		func() {
			defer func() {
				if recover() == nil {
					t.Fatal("estado Extended invalido nao provocou panic fail-fast")
				}
			}()
			ensureExtendedScore(ch)
		}()
	}
	ensureExtendedScore(nil)
}

func TestClampRuntimeCoversAllWideFieldsAndFunctionalCaps(t *testing.T) {
	tooHigh := model.MaxExtendedScoreValue + 1
	e := &model.ExtendedScore{
		Level: tooHigh, Attack: tooHigh, MagicAttack: tooHigh, Defense: tooHigh,
		MaxHP: tooHigh, MaxMP: 100, CurHP: tooHigh, CurMP: 200,
		Str: tooHigh, Int: tooHigh, Dex: tooHigh, Con: tooHigh,
		Accuracy: tooHigh, Evasion: tooHigh, Parry: tooHigh, Critical: tooHigh, Range: tooHigh,
		ResistFire: 101, ResistIce: tooHigh, ResistHoly: 77, ResistThunder: 1000,
		SaveMana: 1000, MagicAmp: tooHigh, RegenHP: 256, RegenMP: tooHigh,
		StatusPts: tooHigh, MasterPts: tooHigh, SkillPts: tooHigh,
		Mastery: [4]uint32{tooHigh, 1, tooHigh, 2},
	}
	clampRuntime(&e)
	if e.Version != model.ExtendedScoreVersion || e.Level != maxExtendedStat || e.Attack != maxExtendedStat ||
		e.MagicAttack != maxExtendedStat || e.Defense != maxExtendedStat || e.MaxHP != maxExtendedStat ||
		e.CurHP != maxExtendedStat || e.CurMP != 100 || e.Str != maxExtendedStat || e.Int != maxExtendedStat ||
		e.Dex != maxExtendedStat || e.Con != maxExtendedStat || e.Accuracy != maxExtendedStat ||
		e.Evasion != maxExtendedStat || e.Parry != maxExtendedStat || e.Critical != maxExtendedStat ||
		e.Range != maxExtendedStat || e.ResistFire != 100 || e.ResistIce != 100 || e.ResistHoly != 77 ||
		e.ResistThunder != 100 || e.SaveMana != 99 || e.MagicAmp != maxExtendedStat || e.RegenHP != 255 ||
		e.RegenMP != 255 || e.StatusPts != maxExtendedStat || e.MasterPts != maxExtendedStat ||
		e.SkillPts != maxExtendedStat || e.Mastery != [4]uint32{maxExtendedStat, 1, maxExtendedStat, 2} {
		t.Fatalf("clampRuntime incompleto: %+v", e)
	}
	clampRuntime(nil)
	if clampExtended(maxExtendedStat-1) != maxExtendedStat-1 || clampExtended(tooHigh) != maxExtendedStat {
		t.Fatal("clampExtended divergiu")
	}
}

func TestApplyAndProjectExtendedScoreContracts(t *testing.T) {
	applyExtendedScore(nil)
	projectExtendedRuntime(nil)
	projectExtendedRuntime(&model.Char{})

	base := &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 90, MaxMP: 80, CurMP: 70}
	ch := &model.Char{Extended: base}
	applyExtendedScore(ch)
	if ch.ExtendedRuntime == nil || ch.ExtendedRuntime == ch.Extended || *ch.ExtendedRuntime != *ch.Extended {
		t.Fatal("applyExtendedScore nao criou copia efetiva")
	}
	ch.ExtendedRuntime.CurHP = model.MaxExtendedScoreValue + 1
	ch.ExtendedRuntime.MaxHP = 50
	projectExtendedRuntime(ch)
	if ch.ExtendedRuntime.CurHP != 50 {
		t.Fatalf("projectExtendedRuntime nao limitou HP: %+v", ch.ExtendedRuntime)
	}
}

func TestPlayerHPMPSetRestoreAndSpendRespectBaseAndRuntimeCaps(t *testing.T) {
	setPlayerCurHP(nil, 1)
	setPlayerCurMP(nil, 1)

	ch := &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 50, MaxMP: 80, CurMP: 40,
	}}
	setPlayerCurHP(ch, 200)
	setPlayerCurMP(ch, 200)
	if ch.Extended.CurHP != 100 || ch.Extended.CurMP != 80 || ch.ExtendedRuntime == nil ||
		ch.ExtendedRuntime.CurHP != 100 || ch.ExtendedRuntime.CurMP != 80 {
		t.Fatalf("setters sem runtime divergiram: base=%+v runtime=%+v", ch.Extended, ch.ExtendedRuntime)
	}

	// Runtime pode ter teto maior que a base por buff. O vivo usa esse teto,
	// enquanto o snapshot persistido nunca ultrapassa o teto base.
	ch.Extended.CurHP, ch.Extended.CurMP = 50, 40
	ch.ExtendedRuntime.MaxHP, ch.ExtendedRuntime.MaxMP = 150, 120
	ch.ExtendedRuntime.CurHP, ch.ExtendedRuntime.CurMP = 50, 40
	setPlayerCurHP(ch, 140)
	setPlayerCurMP(ch, 110)
	if ch.ExtendedRuntime.CurHP != 140 || ch.Extended.CurHP != 100 ||
		ch.ExtendedRuntime.CurMP != 110 || ch.Extended.CurMP != 80 {
		t.Fatalf("separacao base/runtime falhou: base=%+v runtime=%+v", ch.Extended, ch.ExtendedRuntime)
	}

	if restored := restorePlayerHP(ch, 50); restored != 10 || playerCurHP(ch) != 150 {
		t.Fatalf("restorePlayerHP restored=%d hp=%d", restored, playerCurHP(ch))
	}
	if restored := restorePlayerMP(ch, 50); restored != 10 || playerCurMP(ch) != 120 {
		t.Fatalf("restorePlayerMP restored=%d mp=%d", restored, playerCurMP(ch))
	}
	if spendPlayerMP(ch, 121) {
		t.Fatal("spendPlayerMP gastou acima do saldo")
	}
	if !spendPlayerMP(ch, 20) || playerCurMP(ch) != 100 {
		t.Fatalf("spendPlayerMP valido falhou: mp=%d", playerCurMP(ch))
	}
}

func TestExtendedUtilityProjectionAndMinContracts(t *testing.T) {
	if minU32(1, 2) != 1 || minU32(2, 1) != 1 || minU32(2, 2) != 2 {
		t.Fatal("minU32 divergiu")
	}
	if playerCombatMP(nil) != 0 {
		t.Fatal("playerCombatMP(nil) deveria ser zero")
	}
	ch := &model.Char{Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxMP: 100000, CurMP: 50000}}
	if got := playerCombatMP(ch); got == 0 || got > 65535 {
		t.Fatalf("playerCombatMP projetado invalido: %d", got)
	}
}
