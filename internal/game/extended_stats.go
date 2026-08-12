package game

import (
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const maxExtendedStat uint32 = model.MaxExtendedScoreValue

func clampExtended(value uint32) uint32 {
	if value > maxExtendedStat {
		return maxExtendedStat
	}
	return value
}

// ensureExtendedScore garante a unica representacao autoritativa do jogador.
func ensureExtendedScore(ch *model.Char) {
	if ch == nil {
		return
	}
	if ch.Extended == nil {
		panic("game: personagem sem extendedScore")
	}
	if ch.Extended.Version != model.ExtendedScoreVersion {
		panic("game: versao de extendedScore invalida")
	}
}

func clampRuntime(e *model.ExtendedScore) {
	if e == nil {
		return
	}
	e.Version = model.ExtendedScoreVersion
	e.Level = clampExtended(e.Level)
	e.Attack = clampExtended(e.Attack)
	e.MagicAttack = clampExtended(e.MagicAttack)
	e.Defense = clampExtended(e.Defense)
	e.MaxHP = clampExtended(e.MaxHP)
	e.MaxMP = clampExtended(e.MaxMP)
	e.CurHP = minU32(clampExtended(e.CurHP), e.MaxHP)
	e.CurMP = minU32(clampExtended(e.CurMP), e.MaxMP)
	e.Str = clampExtended(e.Str)
	e.Int = clampExtended(e.Int)
	e.Dex = clampExtended(e.Dex)
	e.Con = clampExtended(e.Con)
	e.Accuracy = clampExtended(e.Accuracy)
	e.Evasion = clampExtended(e.Evasion)
	e.Parry = clampExtended(e.Parry)
	e.Critical = clampExtended(e.Critical)
	e.Range = clampExtended(e.Range)
	e.ResistFire = minU32(clampExtended(e.ResistFire), 100)
	e.ResistIce = minU32(clampExtended(e.ResistIce), 100)
	e.ResistHoly = minU32(clampExtended(e.ResistHoly), 100)
	e.ResistThunder = minU32(clampExtended(e.ResistThunder), 100)
	e.SaveMana = minU32(clampExtended(e.SaveMana), 99)
	e.MagicAmp = clampExtended(e.MagicAmp)
	e.RegenHP = minU32(clampExtended(e.RegenHP), 255)
	e.RegenMP = minU32(clampExtended(e.RegenMP), 255)
	e.StatusPts = clampExtended(e.StatusPts)
	e.MasterPts = clampExtended(e.MasterPts)
	e.SkillPts = clampExtended(e.SkillPts)
	for i := range e.Mastery {
		e.Mastery[i] = clampExtended(e.Mastery[i])
	}
}

func applyExtendedScore(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureExtendedScore(ch)
	effective := *ch.Extended
	clampRuntime(&effective)
	ch.ExtendedRuntime = &effective
}

// projectExtendedRuntime foi mantida como ponto unico de validacao para os
// recalculos existentes. Nenhum Score estreito e armazenado aqui.
func projectExtendedRuntime(ch *model.Char) {
	if ch == nil || ch.ExtendedRuntime == nil {
		return
	}
	clampRuntime(ch.ExtendedRuntime)
}

func effectiveExtended(ch *model.Char) *model.ExtendedScore {
	if ch == nil {
		return nil
	}
	ensureExtendedScore(ch)
	if ch.ExtendedRuntime != nil {
		return ch.ExtendedRuntime
	}
	return ch.Extended
}

func playerLevel(ch *model.Char) uint32 {
	if e := effectiveExtended(ch); e != nil {
		return e.Level
	}
	return 0
}

func playerMastery(ch *model.Char, branch int) uint32 {
	if branch < 0 || branch >= 4 {
		return 0
	}
	if e := effectiveExtended(ch); e != nil {
		return e.Mastery[branch]
	}
	return 0
}

func playerSkillPoints(ch *model.Char) uint32 {
	if e := effectiveExtended(ch); e != nil {
		return e.SkillPts
	}
	return 0
}

func playerAttackRun(ch *model.Char) byte {
	if e := effectiveExtended(ch); e != nil {
		return e.AttackRun
	}
	return 0
}

func playerCurHP(ch *model.Char) uint32 {
	if e := effectiveExtended(ch); e != nil {
		return e.CurHP
	}
	return 0
}

func playerMaxHP(ch *model.Char) uint32 {
	if e := effectiveExtended(ch); e != nil {
		return e.MaxHP
	}
	return 0
}

func playerCurMP(ch *model.Char) uint32 {
	if e := effectiveExtended(ch); e != nil {
		return e.CurMP
	}
	return 0
}

func playerMaxMP(ch *model.Char) uint32 {
	if e := effectiveExtended(ch); e != nil {
		return e.MaxMP
	}
	return 0
}

func setPlayerCurHP(ch *model.Char, value uint32) {
	if ch == nil {
		return
	}
	ensureExtendedScore(ch)
	value = minU32(value, playerMaxHP(ch))
	// O valor VIVO (limitado pelo teto efetivo) fica no runtime; o base
	// so aceita ate o proprio teto, senao o autosave grava cur > max.
	ch.Extended.CurHP = minU32(value, ch.Extended.MaxHP)
	if ch.ExtendedRuntime != nil {
		ch.ExtendedRuntime.CurHP = value
	} else {
		applyExtendedScore(ch)
	}
}

func setPlayerCurMP(ch *model.Char, value uint32) {
	if ch == nil {
		return
	}
	ensureExtendedScore(ch)
	value = minU32(value, playerMaxMP(ch))
	// O valor VIVO (limitado pelo teto efetivo) fica no runtime; o base
	// so aceita ate o proprio teto, senao o autosave grava cur > max.
	ch.Extended.CurMP = minU32(value, ch.Extended.MaxMP)
	if ch.ExtendedRuntime != nil {
		ch.ExtendedRuntime.CurMP = value
	} else {
		applyExtendedScore(ch)
	}
}

func playerAttack(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
		return int(e.Attack)
	}
	return 0
}

func playerMagicAttack(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
		return int(e.MagicAttack)
	}
	return 0
}

func playerDefense(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
		return int(e.Defense)
	}
	return 0
}

func playerStr(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
		return int(e.Str)
	}
	return 0
}

func playerInt(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
		return int(e.Int)
	}
	return 0
}

func playerDex(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
		return int(e.Dex)
	}
	return 0
}

func playerCon(ch *model.Char) int {
	if e := effectiveExtended(ch); e != nil {
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

func wireExtendedScore(ch *model.Char) *model.ExtendedScore {
	return effectiveExtended(ch)
}

// Os pacotes de combate 7.48 ainda carregam CurrentMP em WORD. A projecao e
// calculada sob demanda e nao pode contaminar o recurso real uint32.
func playerCombatMP(ch *model.Char) uint32 {
	return wire.CompatibilityCombatMP(effectiveExtended(ch))
}

func minU32(a, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}
