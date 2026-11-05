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

func TestMountSwapBetweenTicksStartsAFullFoodHour(t *testing.T) {
	w, p, _, session := newTimedSwapWorld(t, nil)
	now := w.now()
	old, replacement := equippedAdultMount(10), equippedAdultMount(20)
	old.UID, replacement.UID = "old-mount", "replacement-mount"
	p.Char.Equip[mountSlot], p.Char.Inv[0] = old, replacement
	w.items[old.Index] = premiumTestDef(old.Index, mountSlot)
	p.NextMountTick = now.Add(time.Second)
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, mountSlot))
	if p.Char.Equip[mountSlot].UID != replacement.UID || !p.NextMountTick.IsZero() {
		t.Fatal("accepted mount swap did not reset the previous mount's food clock")
	}
	w.tickPlayerMounts(now)
	for _, elapsed := range []time.Duration{time.Second, time.Minute, time.Hour - time.Nanosecond} {
		w.tickPlayerMounts(now.Add(elapsed))
		if got := p.Char.Equip[mountSlot].MountFood(); got != 20 {
			t.Fatalf("replacement food=%d before its first equipped hour", got)
		}
	}
	w.tickPlayerMounts(now.Add(time.Hour))
	for range 100 {
		w.tickPlayerMounts(now.Add(time.Hour))
	}
	if got := p.Char.Equip[mountSlot].MountFood(); got != 19 {
		t.Fatalf("replacement food=%d after one hour and repeated ticks, want 19", got)
	}
}

func TestRejectedMountSwapPreservesFoodClock(t *testing.T) {
	w, p, _, session := newTimedSwapWorld(t, nil)
	p.Char.Equip[mountSlot] = equippedAdultMount(10)
	p.NextMountTick = w.now().Add(time.Second)
	deadline, mount := p.NextMountTick, p.Char.Equip[mountSlot]
	// A fairy is not eligible for the mount slot.
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, mountSlot))
	if p.Char.Equip[mountSlot] != mount || !p.NextMountTick.Equal(deadline) {
		t.Fatal("rejected mount move changed the food clock or mount")
	}
}

func TestSameMountSlotRequestCannotPostponeFoodClock(t *testing.T) {
	w, p, _, session := newTimedSwapWorld(t, nil)
	p.Char.Equip[mountSlot] = equippedAdultMount(10)
	w.items[model.MountAdultBase] = premiumTestDef(model.MountAdultBase, mountSlot)
	p.NextMountTick = w.now().Add(time.Second)
	deadline := p.NextMountTick
	w.onSwapItem(session, timedSwapPacket(placeEquip, mountSlot, placeEquip, mountSlot))
	if !p.NextMountTick.Equal(deadline) {
		t.Fatal("same-slot request postponed the equipped mount's food deadline")
	}
}
