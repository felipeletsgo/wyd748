package game

import (
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const maxScoreValue uint32 = model.MaxScoreValue

func clampScoreValue(value uint32) uint32 {
	if value > maxScoreValue {
		return maxScoreValue
	}
	return value
}

// ensureScore garante a unica representacao autoritativa do jogador.
func ensureScore(ch *model.Char) {
	if ch == nil {
		return
	}
	if ch.Score == nil {
		panic("game: personagem sem score")
	}
	if ch.Score.Version != model.ScoreVersion {
		panic("game: versao de score invalida")
	}
}

func clampRuntime(e *model.Score) {
	if e == nil {
		return
	}
	e.Version = model.ScoreVersion
	e.Level = clampScoreValue(e.Level)
	e.Attack = clampScoreValue(e.Attack)
	e.MagicAttack = clampScoreValue(e.MagicAttack)
	e.Defense = clampScoreValue(e.Defense)
	e.MaxHP = clampScoreValue(e.MaxHP)
	e.MaxMP = clampScoreValue(e.MaxMP)
	e.CurHP = minU32(clampScoreValue(e.CurHP), e.MaxHP)
	e.CurMP = minU32(clampScoreValue(e.CurMP), e.MaxMP)
	e.Str = clampScoreValue(e.Str)
	e.Int = clampScoreValue(e.Int)
	e.Dex = clampScoreValue(e.Dex)
	e.Con = clampScoreValue(e.Con)
	e.Accuracy = clampScoreValue(e.Accuracy)
	e.Evasion = clampScoreValue(e.Evasion)
	e.Parry = clampScoreValue(e.Parry)
	e.Critical = clampScoreValue(e.Critical)
	e.Range = clampScoreValue(e.Range)
	e.ResistFire = minU32(clampScoreValue(e.ResistFire), 100)
	e.ResistIce = minU32(clampScoreValue(e.ResistIce), 100)
	e.ResistHoly = minU32(clampScoreValue(e.ResistHoly), 100)
	e.ResistThunder = minU32(clampScoreValue(e.ResistThunder), 100)
	e.SaveMana = minU32(clampScoreValue(e.SaveMana), 99)
	e.MagicAmp = clampScoreValue(e.MagicAmp)
	e.RegenHP = minU32(clampScoreValue(e.RegenHP), 255)
	e.RegenMP = minU32(clampScoreValue(e.RegenMP), 255)
	e.StatusPts = clampScoreValue(e.StatusPts)
	e.MasterPts = clampScoreValue(e.MasterPts)
	e.SkillPts = clampScoreValue(e.SkillPts)
	for i := range e.Mastery {
		e.Mastery[i] = clampScoreValue(e.Mastery[i])
	}
}

func applyScore(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	effective := *ch.Score
	clampRuntime(&effective)
	ch.RuntimeScore = &effective
}

// normalizeRuntimeScore foi mantida como ponto unico de validacao para os
// recalculos existentes. Nenhum Score estreito e armazenado aqui.
func normalizeRuntimeScore(ch *model.Char) {
	if ch == nil || ch.RuntimeScore == nil {
		return
	}
	clampRuntime(ch.RuntimeScore)
}

func effectiveScore(ch *model.Char) *model.Score {
	if ch == nil {
		return nil
	}
	ensureScore(ch)
	if ch.RuntimeScore != nil {
		return ch.RuntimeScore
	}
	return ch.Score
}

func playerLevel(ch *model.Char) uint32 {
	if e := effectiveScore(ch); e != nil {
		return e.Level
	}
	return 0
}

func playerMastery(ch *model.Char, branch int) uint32 {
	if branch < 0 || branch >= 4 {
		return 0
	}
	if e := effectiveScore(ch); e != nil {
		return e.Mastery[branch]
	}
	return 0
}

func playerSkillPoints(ch *model.Char) uint32 {
	if e := effectiveScore(ch); e != nil {
		return e.SkillPts
	}
	return 0
}

func playerAttackRun(ch *model.Char) byte {
	if e := effectiveScore(ch); e != nil {
		return byte(e.AttackRun & 0xff)
	}
	return 0
}

func playerCurHP(ch *model.Char) uint32 {
	if e := effectiveScore(ch); e != nil {
		return e.CurHP
	}
	return 0
}

func playerMaxHP(ch *model.Char) uint32 {
	if e := effectiveScore(ch); e != nil {
		return e.MaxHP
	}
	return 0
}

func playerCurMP(ch *model.Char) uint32 {
	if e := effectiveScore(ch); e != nil {
		return e.CurMP
	}
	return 0
}

func playerMaxMP(ch *model.Char) uint32 {
	if e := effectiveScore(ch); e != nil {
		return e.MaxMP
	}
	return 0
}

func setPlayerCurHP(ch *model.Char, value uint32) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	value = minU32(value, playerMaxHP(ch))
	// O valor VIVO (limitado pelo teto efetivo) fica no runtime; o base
	// so aceita ate o proprio teto, senao o autosave grava cur > max.
	ch.Score.CurHP = minU32(value, ch.Score.MaxHP)
	if ch.RuntimeScore != nil {
		ch.RuntimeScore.CurHP = value
	} else {
		applyScore(ch)
	}
}

func setPlayerCurMP(ch *model.Char, value uint32) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	value = minU32(value, playerMaxMP(ch))
	// O valor VIVO (limitado pelo teto efetivo) fica no runtime; o base
	// so aceita ate o proprio teto, senao o autosave grava cur > max.
	ch.Score.CurMP = minU32(value, ch.Score.MaxMP)
	if ch.RuntimeScore != nil {
		ch.RuntimeScore.CurMP = value
	} else {
		applyScore(ch)
	}
}

func playerAttack(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.Attack)
	}
	return 0
}

func playerMagicAttack(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.MagicAttack)
	}
	return 0
}

func playerDefense(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.Defense)
	}
	return 0
}

func playerStr(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.Str)
	}
	return 0
}

func playerInt(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.Int)
	}
	return 0
}

func playerDex(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.Dex)
	}
	return 0
}

func playerCon(ch *model.Char) int {
	if e := effectiveScore(ch); e != nil {
		return int(e.Con)
	}
	return 0
}

func restorePlayerHP(ch *model.Char, amount uint32) uint32 {
	old := playerCurHP(ch)
	setPlayerCurHP(ch, minU32(playerMaxHP(ch), old+amount))
	return playerCurHP(ch) - old
}

func restorePlayerMP(ch *model.Char, amount uint32) uint32 {
	old := playerCurMP(ch)
	setPlayerCurMP(ch, minU32(playerMaxMP(ch), old+amount))
	return playerCurMP(ch) - old
}

func spendPlayerMP(ch *model.Char, amount uint32) bool {
	current := playerCurMP(ch)
	if current < amount {
		return false
	}
	setPlayerCurMP(ch, current-amount)
	return true
}

func wireScoreState(ch *model.Char) *model.Score {
	return effectiveScore(ch)
}

// Os pacotes de combate 7.48 ainda carregam CurrentMP em WORD. A projecao e
// calculada sob demanda e nao pode contaminar o recurso real uint32.
func playerCombatMP(ch *model.Char) uint32 {
	return wire.CompatibilityCombatMP(effectiveScore(ch))
}

func minU32(a, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}
