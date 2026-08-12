package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func equippedAdultMount(food int) model.Item {
	mount := model.Item{Index: model.MountAdultBase}
	mount.SetMountHP(1000)
	mount.SetMountLongev(30)
	mount.SetMountFood(food)
	return mount
}

func TestMountFoodConsumesOnePerCompleteOnlineHour(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Rider", 2100, 2100)
	p.Char.Equip[mountSlot] = equippedAdultMount(10)
	w := worldWithNetworkedPlayers(p)
	now := time.Unix(1_700_000_000, 0)

	// O primeiro tick inicia a hora; login/equip nunca cobra Food imediatamente.
	w.tickPlayerMounts(now)
	if got := p.Char.Equip[mountSlot].MountFood(); got != 10 {
		t.Fatalf("first tick food=%d, want 10", got)
	}
	if want := now.Add(time.Hour); !p.NextMountTick.Equal(want) {
		t.Fatalf("first deadline=%v, want %v", p.NextMountTick, want)
	}

	w.tickPlayerMounts(now.Add(time.Hour - time.Nanosecond))
	if got := p.Char.Equip[mountSlot].MountFood(); got != 10 {
		t.Fatalf("food before a complete hour=%d, want 10", got)
	}

	w.tickPlayerMounts(now.Add(time.Hour))
	if got := p.Char.Equip[mountSlot].MountFood(); got != 9 {
		t.Fatalf("food after one hour=%d, want 9", got)
	}

	w.tickPlayerMounts(now.Add(2 * time.Hour))
	if got := p.Char.Equip[mountSlot].MountFood(); got != 8 {
		t.Fatalf("food after two hours=%d, want 8", got)
	}
}

func TestMountFoodClockPausesWhileUnequipped(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Rider", 2100, 2100)
	p.Char.Equip[mountSlot] = equippedAdultMount(10)
	w := worldWithNetworkedPlayers(p)
	now := time.Unix(1_700_000_000, 0)

	w.tickPlayerMounts(now)
	p.Char.Inv[0], p.Char.Equip[mountSlot] = p.Char.Equip[mountSlot], model.Item{}
	w.tickPlayerMounts(now.Add(30 * time.Minute))
	if !p.NextMountTick.IsZero() {
		t.Fatalf("unequipped mount kept food deadline %v", p.NextMountTick)
	}

	// Horas fora do slot nao contam. Reequipar inicia uma nova hora inteira.
	p.Char.Equip[mountSlot], p.Char.Inv[0] = p.Char.Inv[0], model.Item{}
	w.tickPlayerMounts(now.Add(24 * time.Hour))
	if got := p.Char.Equip[mountSlot].MountFood(); got != 10 {
		t.Fatalf("reequip consumed food immediately: %d", got)
	}
	w.tickPlayerMounts(now.Add(25*time.Hour - time.Nanosecond))
	if got := p.Char.Equip[mountSlot].MountFood(); got != 10 {
		t.Fatalf("reequipped food before a complete hour=%d, want 10", got)
	}
	w.tickPlayerMounts(now.Add(25 * time.Hour))
	if got := p.Char.Equip[mountSlot].MountFood(); got != 9 {
		t.Fatalf("reequipped food after a complete hour=%d, want 9", got)
	}
}
