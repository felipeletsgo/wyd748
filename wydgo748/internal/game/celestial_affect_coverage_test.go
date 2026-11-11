package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCarryItemBuffsReplacesMatchesAndProtectsSoulLimitSlot(t *testing.T) {
	var target, source [16]model.Affect
	target[0] = model.Affect{Type: 5, Value: 1}
	for i := 1; i < len(target)-1; i++ {
		target[i] = model.Affect{Type: 9, Value: i}
	}
	target[15] = model.Affect{Type: 77, Value: 99}
	expires := time.Unix(2_000_000_000, 0)
	source[0] = model.Affect{Type: 5, Value: 42, SourceItemIndex: 100, ExpiresAt: expires}
	source[1] = model.Affect{Type: 6, Value: 43, SourceItemIndex: 101}
	source[2] = model.Affect{Type: 0, Value: 44, SourceItemIndex: 102}
	source[3] = model.Affect{Type: 7, Value: 45}

	carryItemBuffs(&target, source)

	if target[0] != source[0] {
		t.Fatalf("same-type item buff was not replaced: got %+v, want %+v", target[0], source[0])
	}
	if target[1].Type != 9 || target[15] != (model.Affect{Type: 77, Value: 99}) {
		t.Fatalf("buff without a legal target displaced protected state: slot1=%+v soulLimit=%+v", target[1], target[15])
	}
	if target[2].Type != 9 || target[3].Type != 9 {
		t.Fatal("invalid source affects were copied")
	}
}

func TestCarryItemBuffsUsesOriginalIndexThenFirstAvailableSlot(t *testing.T) {
	var target, source [16]model.Affect
	target[0] = model.Affect{Type: 1, Value: 10}
	source[4] = model.Affect{Type: 2, Value: 20, SourceItemIndex: 200}
	source[8] = model.Affect{Type: 3, Value: 30, SourceItemIndex: 300}

	carryItemBuffs(&target, source)

	if target[4] != source[4] || target[8] != source[8] {
		t.Fatalf("item buffs did not retain their free original indexes: at4=%+v at8=%+v", target[4], target[8])
	}

	var packed [16]model.Affect
	packed[0] = model.Affect{Type: 1}
	packed[2] = model.Affect{Type: 2}
	carryItemBuffs(&packed, [16]model.Affect{{Type: 3, SourceItemIndex: 400}})
	if packed[1].SourceItemIndex != 400 {
		t.Fatalf("item buff did not use the first available non-Soul slot: %+v", packed[1])
	}
}
