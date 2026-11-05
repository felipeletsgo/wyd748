package game

import (
	"bytes"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestMountDamagePublishesEachOwnerSnapshotAcrossAutoFeedThreshold(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Rider", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	w := worldWithNetworkedPlayers(owner, observer)
	owner.show(observer.ID)
	observer.show(owner.ID)
	w.mounts = model.MountCatalog{Types: map[int]model.MountStats{
		21: {MaxHP: 28000, RiderDamagePct: 71},
	}}
	mount := equippedAdultMount(30)
	mount.Index = 2381
	mount.SetMountHP(15000)
	owner.Char.Equip[mountSlot] = mount
	before := observer.Session.QueuedPacketsForTest()
	for _, hp := range []int{14420, 13840, 13260} {
		if got := w.absorbMountDamage(owner, 2000); got != 1420 {
			t.Fatalf("rider damage=%d, want 1420", got)
		}
		item := owner.Char.Equip[mountSlot]
		if item.MountHP() != hp || item.MountFood() != 30 {
			t.Fatalf("damage changed the wrong mount state: %+v", item)
		}
		packet, ok := owner.Session.DequeuePacketForTest()
		want := wire.SendItem(owner.ID, placeEquip, mountSlot, item)
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpSendItem ||
			wire.ParseHeader(packet).ID != owner.ID || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatal("owner did not receive exact authoritative mount HP in Equip[14]")
		}
		if owner.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != before {
			t.Fatal("nonlethal damage sent duplicate owner or observer appearance packets")
		}
	}
	// This is the actual input consumed by the client's C.C. 50% comparison.
	if owner.Char.Equip[mountSlot].MountHP() >= 28000/2 {
		t.Fatal("fixture did not cross the configured auto-feed threshold")
	}
}

func TestMountDamageWithoutAbsorptionDoesNotPublishOrConsumeFood(t *testing.T) {
	for _, incoming := range []int{0, 1} {
		owner, _ := networkedTestPlayer(1, "Rider", 2100, 2100)
		w := worldWithNetworkedPlayers(owner)
		owner.Char.Equip[mountSlot] = equippedAdultMount(10)
		before := owner.Char.Equip[mountSlot]
		if got := w.absorbMountDamage(owner, incoming); got != incoming {
			t.Fatalf("rider damage=%d, want %d", got, incoming)
		}
		if owner.Char.Equip[mountSlot] != before || owner.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("zero absorption changed mount state or published an unchanged item")
		}
	}
}
