package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestRegenAmountMatchesRegenMobFormula(t *testing.T) {
	if got := regenAmount(600, 2); got != 20 {
		t.Fatalf("regen=%d, quer 20", got)
	}
}

func TestApplyPlayerRegenRestoresAndCapsHPMP(t *testing.T) {
	ch := &model.Char{
		Extended: testExtended(model.ExtendedScore{MaxHP: 600, MaxMP: 300, CurHP: 590, CurMP: 280}),
	}
	ch.Extended.RegenHP, ch.Extended.RegenMP = 2, 4
	if !applyPlayerRegen(ch) {
		t.Fatal("pulso nao alterou o personagem")
	}
	if playerCurHP(ch) != 600 || playerCurMP(ch) != 300 {
		t.Fatalf("HP/MP=%d/%d, quer 600/300", playerCurHP(ch), playerCurMP(ch))
	}
}

func TestApplyPlayerRegenDoesNotRevive(t *testing.T) {
	ch := &model.Char{Extended: testExtended(model.ExtendedScore{MaxHP: 100, MaxMP: 100})}
	if applyPlayerRegen(ch) || playerCurHP(ch) != 0 {
		t.Fatal("regeneracao reviveu personagem morto")
	}
}
