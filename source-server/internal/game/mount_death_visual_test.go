package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestMountDeathKeepsItemEquippedAndPublishesEmptyVisualSlot(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Rider", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	w := worldWithNetworkedPlayers(owner, observer)
	owner.show(observer.ID)
	observer.show(owner.ID)

	mount := model.Item{Index: model.MountAdultBase}
	mount.SetMountHP(1)
	mount.SetMountLevel(40)
	mount.SetMountLongev(10)
	mount.SetMountFood(50)
	owner.Char.Equip[mountSlot] = mount
	if bodyMesh(owner.Char)[mountSlot] == 0 {
		t.Fatal("montaria viva começou invisível")
	}
	ownerPackets := owner.Session.QueuedPacketsForTest()
	observerPackets := observer.Session.QueuedPacketsForTest()

	if got := w.absorbMountDamage(owner, 4); got != 3 {
		t.Fatalf("dano do cavaleiro=%d, esperado 3", got)
	}
	if owner.Char.Equip[mountSlot].Index != model.MountAdultBase || owner.Char.Equip[mountSlot].MountHP() != 0 {
		t.Fatalf("morte removeu/corrompeu item autoritativo: %+v", owner.Char.Equip[mountSlot])
	}
	if bodyMesh(owner.Char)[mountSlot] != 0 {
		t.Fatal("montaria morta continuou na projeção de aparência")
	}
	if owner.Session.QueuedPacketsForTest() <= ownerPackets ||
		observer.Session.QueuedPacketsForTest() != observerPackets+1 {
		t.Fatalf("UpdateEquip não chegou a dono/observador: owner=%d/%d observer=%d/%d",
			owner.Session.QueuedPacketsForTest(), ownerPackets,
			observer.Session.QueuedPacketsForTest(), observerPackets)
	}

	// Um observador que só materializar o personagem depois da morte também
	// deve receber o slot 14 vazio no CreateMob, sem alterar Equip[14].
	packets := playerEnterViewPackets(owner)
	if len(packets) == 0 {
		t.Fatal("entrada na visão não gerou CreateMob")
	}
	if got := binary.LittleEndian.Uint16(packets[0][34+mountSlot*2:]); got != 0 {
		t.Fatalf("novo observador recebeu mesh de montaria morta 0x%X", got)
	}
}
