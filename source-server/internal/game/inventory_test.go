package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestAddToInvNeverUsesInvisibleSlot63(t *testing.T) {
	ch := &model.Char{}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		ch.Inv[i] = model.Item{Index: uint16(i + 1)}
	}
	if got := addToInv(ch, model.Item{Index: 400}); got != -1 {
		t.Fatalf("addToInv retornou slot invisivel %d", got)
	}
	if ch.Inv[model.PlayerCarrySlots].Index != 0 {
		t.Fatalf("slot invisivel recebeu item: %+v", ch.Inv[model.PlayerCarrySlots])
	}
}
