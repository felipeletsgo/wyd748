package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func mountMasterTestWorld(t *testing.T, random int) (*World, *Player, *Mob, *craftStore) {
	t.Helper()
	w, p, st := handlerTestWorld(t)
	w.rng = fixedRNG{value: random}
	master := &Mob{
		ID: 1800, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{
			Name: "Mount_Master", Tipo: model.TipoNPC,
			Score: &model.Score{Merchant: mountMasterMerchant},
		},
	}
	w.registerMobSpatial(master)
	p.show(master.ID)
	return w, p, master, st
}

func confirmedMountMasterPacket(id uint16, confirmed bool) []byte {
	pkt := useNPCPacket(id)
	if confirmed {
		binary.LittleEndian.PutUint32(pkt[16:20], 1)
	}
	return pkt
}

func deadMount(index uint16, longevity, food int) model.Item {
	mount := model.Item{Index: index}
	mount.SetMountHP(0)
	mount.SetMountLevel(40)
	mount.SetMountLongev(longevity)
	mount.SetMountFood(food)
	return mount
}

func TestMountMasterRevivesDeadEquippedMountThroughNPCClick(t *testing.T) {
	w, p, master, st := mountMasterTestWorld(t, 2)
	const mountIndex = model.MountAdultBase
	w.items[mountIndex] = model.ItemDef{Index: mountIndex, Name: "Pig", Price: 5_000}
	p.Char.Gold = 10_000
	p.Char.Equip[mountSlot] = deadMount(mountIndex, 10, 70)

	w.onUseNPC(p.Session, confirmedMountMasterPacket(master.ID, true))

	mount := p.Char.Equip[mountSlot]
	if st.saves != 1 || p.Char.Gold != 5_000 || mount.Index != mountIndex ||
		mount.MountHP() != mountMasterReviveHP || mount.MountLongev() != 8 || mount.MountFood() != 5 {
		t.Fatalf("revival incorreto: saves=%d gold=%d mount=%+v hp=%d life=%d food=%d",
			st.saves, p.Char.Gold, mount, mount.MountHP(), mount.MountLongev(), mount.MountFood())
	}
}

func TestMountMasterRequiresClientConfirmationBeforeCharging(t *testing.T) {
	w, p, master, st := mountMasterTestWorld(t, 0)
	const mountIndex = model.MountAdultBase
	w.items[mountIndex] = model.ItemDef{Index: mountIndex, Name: "Pig", Price: 5_000}
	p.Char.Gold = 10_000
	p.Char.Equip[mountSlot] = deadMount(mountIndex, 10, 70)

	w.onUseNPC(p.Session, confirmedMountMasterPacket(master.ID, false))

	if st.saves != 0 || p.Char.Gold != 10_000 || p.Char.Equip[mountSlot].MountHP() != 0 ||
		p.Char.Equip[mountSlot].MountLongev() != 10 {
		t.Fatalf("clique sem confirmacao mutou estado: saves=%d gold=%d mount=%+v",
			st.saves, p.Char.Gold, p.Char.Equip[mountSlot])
	}
}

func TestMountMasterCanFailAndRemoveMountWhenLongevityEnds(t *testing.T) {
	w, p, master, st := mountMasterTestWorld(t, 2)
	const mountIndex = model.MountBabyBase
	w.items[mountIndex] = model.ItemDef{Index: mountIndex, Name: "Pig's Baby", Price: 3_000}
	p.Char.Gold = 10_000
	p.Char.Equip[mountSlot] = deadMount(mountIndex, 2, 20)

	w.onUseNPC(p.Session, confirmedMountMasterPacket(master.ID, true))

	if st.saves != 1 || p.Char.Gold != 7_000 || p.Char.Equip[mountSlot].Index != 0 {
		t.Fatalf("falha nativa incorreta: saves=%d gold=%d mount=%+v",
			st.saves, p.Char.Gold, p.Char.Equip[mountSlot])
	}
}

func TestMountMasterRejectsAliveMountAndInsufficientGold(t *testing.T) {
	for _, test := range []struct {
		name string
		hp   int
		gold uint32
	}{
		{name: "alive", hp: 1, gold: 10_000},
		{name: "insufficient gold", hp: 0, gold: 4_999},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, master, st := mountMasterTestWorld(t, 0)
			const mountIndex = model.MountAdultBase
			w.items[mountIndex] = model.ItemDef{Index: mountIndex, Name: "Pig", Price: 5_000}
			p.Char.Gold = test.gold
			p.Char.Equip[mountSlot] = deadMount(mountIndex, 10, 20)
			p.Char.Equip[mountSlot].SetMountHP(test.hp)
			before := p.Char.Equip[mountSlot]

			w.onUseNPC(p.Session, confirmedMountMasterPacket(master.ID, true))

			if st.saves != 0 || p.Char.Gold != test.gold || p.Char.Equip[mountSlot] != before {
				t.Fatalf("recusa mutou estado: saves=%d gold=%d mount=%+v",
					st.saves, p.Char.Gold, p.Char.Equip[mountSlot])
			}
		})
	}
}

func TestMountMasterRollsBackWholeCharacterWhenSaveFails(t *testing.T) {
	w, p, master, st := mountMasterTestWorld(t, 1)
	const mountIndex = model.MountAdultBase
	w.items[mountIndex] = model.ItemDef{Index: mountIndex, Name: "Pig", Price: 5_000}
	p.Char.Gold = 10_000
	p.Char.Equip[mountSlot] = deadMount(mountIndex, 10, 70)
	before := cloneCharacterState(p.Char)
	st.err = errors.New("database unavailable")

	w.onUseNPC(p.Session, confirmedMountMasterPacket(master.ID, true))

	if st.saves != 1 || p.Char.Gold != before.Gold || p.Char.Equip[mountSlot] != before.Equip[mountSlot] {
		t.Fatalf("rollback incompleto: saves=%d gold=%d/%d mount=%+v/%+v",
			st.saves, p.Char.Gold, before.Gold, p.Char.Equip[mountSlot], before.Equip[mountSlot])
	}
}
