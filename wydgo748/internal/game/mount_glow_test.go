package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestMountGlowLevelPropagatesToOwnerAndObservers(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Rider", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	w := worldWithNetworkedPlayers(owner, observer)
	owner.show(observer.ID)
	observer.show(owner.ID)
	mount := model.Item{Index: model.MountAdultBase}
	mount.SetMountHP(100)
	mount.SetMountLongev(10)
	mount.SetMountFood(50)
	owner.Char.Equip[mountSlot] = mount
	for level := 1; level <= 120; level++ {
		owner.Char.Equip[mountSlot].SetMountLevel(level)
		want := uint16(model.MountAdultBase) | uint16(level/10)<<12
		w.refreshAppearance(owner)
		for name, session := range map[string]interface {
			DequeuePacketForTest() ([]byte, bool)
			QueuedPacketsForTest() int
		}{"owner": owner.Session, "observer": observer.Session} {
			pkt, ok := session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpUpdateEquip ||
				wire.ParseHeader(pkt).ID != owner.ID || len(pkt) != 60 {
				t.Fatalf("level %d %s: invalid appearance packet: %x", level, name, pkt)
			}
			if got := binary.LittleEndian.Uint16(pkt[12+mountSlot*2:]); got != want {
				t.Fatalf("level %d %s: visual=%04x, want %04x", level, name, got, want)
			}
			if session.QueuedPacketsForTest() != 0 {
				t.Fatalf("level %d %s: appearance duplicated", level, name)
			}
		}
		packets := playerEnterViewPackets(owner)
		if len(packets) == 0 || binary.LittleEndian.Uint16(packets[0][34+mountSlot*2:]) != want {
			t.Fatalf("level %d: late observer lost the mount grade", level)
		}
	}
}
