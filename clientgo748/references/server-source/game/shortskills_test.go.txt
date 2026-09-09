package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestFilterShortSkillsKeepsOnlyLearned(t *testing.T) {
	ch := model.Char{Class: 0, LearnedSkill: 1 << 3}
	for i := range ch.ShortSkill {
		ch.ShortSkill[i] = 0xFF
	}
	ch.ShortSkill[0], ch.ShortSkill[1] = 3, 7
	filterShortSkills(&ch)
	if ch.ShortSkill[0] != 3 || ch.ShortSkill[1] != 0xFF {
		t.Fatalf("atalhos filtrados incorretamente: %v", ch.ShortSkill[:2])
	}
}
