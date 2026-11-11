package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestScoreGettersHandleNilAndPreferRuntimeScore(t *testing.T) {
	base := &model.Score{Version: model.ScoreVersion, Level: 1, Mastery: [4]uint32{1, 2, 3, 4}, SkillPts: 5,
		AttackRun: 260, CurHP: 6, MaxHP: 7, CurMP: 8, MaxMP: 9, Attack: 10, MagicAttack: 11, Defense: 12, Str: 13, Int: 14, Dex: 15, Con: 16}
	character := &model.Char{Score: base}
	runtime := *base
	runtime.Level, runtime.Mastery[2], runtime.SkillPts = 101, 103, 105
	runtime.AttackRun, runtime.CurHP, runtime.MaxHP, runtime.CurMP, runtime.MaxMP = 160, 106, 107, 108, 109
	runtime.Attack, runtime.MagicAttack, runtime.Defense = 110, 111, 112
	runtime.Str, runtime.Int, runtime.Dex, runtime.Con = 113, 114, 115, 116

	tests := []struct {
		name         string
		read         func(*model.Char) any
		zero         any
		baseValue    any
		runtimeValue any
	}{
		{name: "level", read: func(ch *model.Char) any { return playerLevel(ch) }, zero: uint32(0), baseValue: uint32(1), runtimeValue: uint32(101)},
		{name: "mastery", read: func(ch *model.Char) any { return playerMastery(ch, 2) }, zero: uint32(0), baseValue: uint32(3), runtimeValue: uint32(103)},
		{name: "skill points", read: func(ch *model.Char) any { return playerSkillPoints(ch) }, zero: uint32(0), baseValue: uint32(5), runtimeValue: uint32(105)},
		{name: "attack run", read: func(ch *model.Char) any { return playerAttackRun(ch) }, zero: byte(0), baseValue: byte(4), runtimeValue: byte(160)},
		{name: "current hp", read: func(ch *model.Char) any { return playerCurHP(ch) }, zero: uint32(0), baseValue: uint32(6), runtimeValue: uint32(106)},
		{name: "maximum hp", read: func(ch *model.Char) any { return playerMaxHP(ch) }, zero: uint32(0), baseValue: uint32(7), runtimeValue: uint32(107)},
		{name: "current mp", read: func(ch *model.Char) any { return playerCurMP(ch) }, zero: uint32(0), baseValue: uint32(8), runtimeValue: uint32(108)},
		{name: "maximum mp", read: func(ch *model.Char) any { return playerMaxMP(ch) }, zero: uint32(0), baseValue: uint32(9), runtimeValue: uint32(109)},
		{name: "attack", read: func(ch *model.Char) any { return playerAttack(ch) }, zero: int(0), baseValue: int(10), runtimeValue: int(110)},
		{name: "magic attack", read: func(ch *model.Char) any { return playerMagicAttack(ch) }, zero: int(0), baseValue: int(11), runtimeValue: int(111)},
		{name: "defense", read: func(ch *model.Char) any { return playerDefense(ch) }, zero: int(0), baseValue: int(12), runtimeValue: int(112)},
		{name: "strength", read: func(ch *model.Char) any { return playerStr(ch) }, zero: int(0), baseValue: int(13), runtimeValue: int(113)},
		{name: "intelligence", read: func(ch *model.Char) any { return playerInt(ch) }, zero: int(0), baseValue: int(14), runtimeValue: int(114)},
		{name: "dexterity", read: func(ch *model.Char) any { return playerDex(ch) }, zero: int(0), baseValue: int(15), runtimeValue: int(115)},
		{name: "constitution", read: func(ch *model.Char) any { return playerCon(ch) }, zero: int(0), baseValue: int(16), runtimeValue: int(116)},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			if got := tc.read(nil); got != tc.zero {
				t.Fatalf("nil score value=%v, want %v", got, tc.zero)
			}
			if got := tc.read(character); got != tc.baseValue {
				t.Fatalf("base score value=%v, want %v", got, tc.baseValue)
			}
			character.RuntimeScore = &runtime
			if got := tc.read(character); got != tc.runtimeValue {
				t.Fatalf("runtime score value=%v, want %v", got, tc.runtimeValue)
			}
			character.RuntimeScore = nil
		})
	}
	if playerMastery(character, -1) != 0 || playerMastery(character, 4) != 0 {
		t.Fatal("mastery read accepted an invalid branch")
	}
	if effectiveScore(nil) != nil || wireScoreState(nil) != nil {
		t.Fatal("nil character returned a score")
	}
	normalizeRuntimeScore(nil)
	normalizeRuntimeScore(character)
	applyScore(nil)
	setPlayerCurHP(nil, 1)
	setPlayerCurMP(nil, 1)
	if restorePlayerHP(nil, 1) != 0 || restorePlayerMP(nil, 1) != 0 || spendPlayerMP(nil, 1) {
		t.Fatal("nil character changed health or mana")
	}
}
