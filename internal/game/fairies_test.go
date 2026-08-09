package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func fairyTestDef(index uint16, days int) model.ItemDef {
	def := model.ItemDef{
		Index: index,
		DynamicEffectNames: map[byte]string{
			1: "EF_WDAY",
			2: "EF_HOUR",
			3: "EF_MIN",
		},
	}
	if days > 0 {
		def.StaticEffects = []model.StaticEffect{
			{Name: "EF_WDAY", Value: days},
			{Name: "EF_HOUR", Value: 0},
			{Name: "EF_MIN", Value: 0},
		}
	}
	return def
}

func fairyTestWorld() *World {
	items := make(map[uint16]model.ItemDef)
	for _, index := range []uint16{3900, 3901, 3902, 3903, 3904, 3905, 3906, 3907, 3908} {
		days := 3
		if index >= 3903 && index <= 3905 {
			days = 5
		}
		if index >= 3906 {
			days = 7
		}
		items[index] = fairyTestDef(index, days)
	}
	for _, index := range []uint16{3911, 3912, 3913} {
		items[index] = fairyTestDef(index, 7)
	}
	items[3914] = fairyTestDef(3914, 0)
	items[3915] = fairyTestDef(3915, 0)
	return &World{items: items}
}

func fairyWithMinutes(t *testing.T, w *World, index uint16, minutes int) model.Item {
	t.Helper()
	item := model.Item{Index: index}
	def := w.items[index]
	if !writeFairyTimerMinutes(&item, def, minutes) {
		t.Fatalf("writeFairyTimerMinutes(%d, %d) failed", index, minutes)
	}
	return item
}

func TestFairyBonusMatrix(t *testing.T) {
	w := fairyTestWorld()
	tests := []struct {
		name       string
		index      uint16
		exp, drop  int
		inherited  bool
	}{
		{name: "green", index: 3900, exp: 16},
		{name: "blue", index: 3901, drop: 32},
		{name: "red", index: 3902, exp: 8, drop: 16},
		{name: "silver", index: 3914, exp: 16, drop: 32, inherited: true},
		{name: "gold", index: 3915, exp: 24, drop: 48, inherited: true},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			ch := &model.Char{}
			if tc.inherited {
				ch.Equip[fairySlot] = fairyWithMinutes(t, w, tc.index, 60)
			} else {
				ch.Equip[fairySlot] = model.Item{Index: tc.index}
			}
			got := w.activeFairyBonus(ch)
			if got.expPercent != tc.exp || got.dropPercent != tc.drop {
				t.Fatalf("bonus=%+v, want exp=%d drop=%d", got, tc.exp, tc.drop)
			}
		})
	}
}

func TestFairyTimerStartsEquippedAndPausesUnequipped(t *testing.T) {
	w := fairyTestWorld()
	ch := &model.Char{Equip: [16]model.Item{fairySlot: {Index: 3900}}}
	p := &Player{Char: ch, InWorld: true}
	now := time.Unix(1_700_000_000, 0)

	// Primeiro tick apos equip/login materializa 3 dias, mas nao cobra um
	// minuto que o personagem nao passou equipado no servidor.
	w.tickEquippedFairy(p, now)
	remaining, ok := fairyTimerMinutes(ch.Equip[fairySlot], w.items[3900])
	if !ok || remaining != 3*24*60 {
		t.Fatalf("remaining=%d ok=%v, want %d", remaining, ok, 3*24*60)
	}

	p.NextMountTick = now
	w.tickEquippedFairy(p, now)
	remaining, _ = fairyTimerMinutes(ch.Equip[fairySlot], w.items[3900])
	if remaining != 3*24*60-1 {
		t.Fatalf("equipped remaining=%d, want %d", remaining, 3*24*60-1)
	}

	// Desequipar move o mesmo item/UID para Carry. Mesmo com o deadline vencido,
	// o timer nao e tocado fora do Equip[13].
	ch.Inv[0], ch.Equip[fairySlot] = ch.Equip[fairySlot], model.Item{}
	paused, _ := fairyTimerMinutes(ch.Inv[0], w.items[3900])
	p.NextMountTick = now
	w.tickEquippedFairy(p, now.Add(24*time.Hour))
	stillPaused, _ := fairyTimerMinutes(ch.Inv[0], w.items[3900])
	if stillPaused != paused {
		t.Fatalf("unequipped timer moved: %d -> %d", paused, stillPaused)
	}

	// Reequipar continua exatamente do saldo salvo; nao reinicializa 3 dias.
	ch.Equip[fairySlot], ch.Inv[0] = ch.Inv[0], model.Item{}
	p.NextMountTick = time.Time{}
	w.tickEquippedFairy(p, now.Add(24*time.Hour))
	resumed, _ := fairyTimerMinutes(ch.Equip[fairySlot], w.items[3900])
	if resumed != paused {
		t.Fatalf("reequip reset timer: got %d want %d", resumed, paused)
	}
	p.NextMountTick = now.Add(24 * time.Hour)
	w.tickEquippedFairy(p, now.Add(24*time.Hour))
	resumed, _ = fairyTimerMinutes(ch.Equip[fairySlot], w.items[3900])
	if resumed != paused-1 {
		t.Fatalf("reequip countdown=%d, want %d", resumed, paused-1)
	}
}

func TestFairyTimerDoesNotAdvanceOffline(t *testing.T) {
	w := fairyTestWorld()
	ch := &model.Char{}
	ch.Equip[fairySlot] = fairyWithMinutes(t, w, 3902, 1234)
	p := &Player{Char: ch, InWorld: false, NextMountTick: time.Unix(1, 0)}

	w.tickEquippedFairy(p, time.Unix(1, 0).Add(30*24*time.Hour))
	remaining, _ := fairyTimerMinutes(ch.Equip[fairySlot], w.items[3902])
	if remaining != 1234 {
		t.Fatalf("offline timer=%d, want 1234", remaining)
	}
}

func TestSilverAndGoldPreserveInheritedTimer(t *testing.T) {
	w := fairyTestWorld()
	for _, index := range []uint16{3914, 3915} {
		t.Run(string(rune(index)), func(t *testing.T) {
			item := fairyWithMinutes(t, w, index, 2*24*60+3*60+4)
			before := item
			if !w.initializeFairyTimer(&item) {
				t.Fatalf("initializeFairyTimer(%d) rejected inherited timer", index)
			}
			if item != before {
				t.Fatalf("fairy %d inherited timer changed: before=%v after=%v", index, before.Eff, item.Eff)
			}
		})
	}
}

func TestDirectSilverWithoutInheritedTimerIsInactive(t *testing.T) {
	w := fairyTestWorld()
	ch := &model.Char{}
	ch.Equip[fairySlot] = model.Item{Index: 3914}
	if got := w.activeFairyBonus(ch); got != (fairyBonus{}) {
		t.Fatalf("direct silver without timer must be inactive, got %+v", got)
	}
	if w.hasActiveSilverFairy(ch) {
		t.Fatal("silver without inherited timer must not enable Water automation")
	}
}

func TestExpiredFairyProvidesNoBonus(t *testing.T) {
	w := fairyTestWorld()
	ch := &model.Char{}
	ch.Equip[fairySlot] = fairyWithMinutes(t, w, 3915, 0)
	if got := w.activeFairyBonus(ch); got != (fairyBonus{}) {
		t.Fatalf("expired fairy bonus=%+v", got)
	}
}
