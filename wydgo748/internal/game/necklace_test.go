package game

import (
	"fmt"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func necklaceCatalog(t *testing.T) map[uint16]model.ItemDef {
	t.Helper()
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	return catalog.Items
}

func TestMysticAmuletInstalledCatalogAndSwap(t *testing.T) {
	items := necklaceCatalog(t)
	for index := uint16(559); index <= 562; index++ {
		for class := byte(0); class < 4; class++ {
			t.Run(fmt.Sprintf("item-%d-class-%d", index, class), func(t *testing.T) {
				w, p, _ := handlerTestWorld(t)
				w.items = items
				p.Char.Class = class
				p.Char.Score.Level = 92
				w.recalcPlayer(p.Char)
				before := effectiveScore(p.Char).Mastery
				item := model.Item{Index: index, UID: "mystic-amulet", Eff: [6]byte{43, 0}}
				p.Char.Inv[0] = item
				if items[index].Pos != 1<<9 || !w.canEquip(p.Char, item, 9) || w.canEquip(p.Char, item, 8) || w.canEquip(p.Char, item, 16) {
					t.Fatal("installed amulet does not obey the native necklace position")
				}
				p.Char.Score.Level = 91
				w.recalcPlayer(p.Char)
				if w.canEquip(p.Char, item, 9) {
					t.Fatal("mortal below the catalog requirement equipped the amulet")
				}
				p.Char.Score.Level = 92
				w.recalcPlayer(p.Char)
				request := make([]byte, 20)
				request[12], request[14], request[15] = placeInv, placeEquip, 9
				w.onSwapItem(p.Session, request)
				if p.Char.Equip[9] != item || p.Char.Inv[0] != (model.Item{}) || clientEquipProjection(p.Char)[9] != item {
					t.Fatal("equip lost the item identity/effects or omitted the client projection")
				}
				want := before
				want[index-559] += 10
				if got := effectiveScore(p.Char).Mastery; got != want {
					t.Fatalf("equipped mastery=%v, want %v", got, want)
				}
				request[12], request[13], request[14], request[15] = placeEquip, 9, placeInv, 1
				w.onSwapItem(p.Session, request)
				if p.Char.Inv[1] != item || p.Char.Equip[9] != (model.Item{}) || effectiveScore(p.Char).Mastery != before {
					t.Fatal("unequip failed to preserve the item or remove its mastery bonus")
				}
			})
		}
	}
}

func TestMysticAmuletPreservedOnEntry(t *testing.T) {
	w, p, session := newEnterWorldPlayer(t, 742, 1000)
	w.items = necklaceCatalog(t)
	item := model.Item{Index: 559, UID: "equipped-mystic-amulet", Eff: [6]byte{43, 9}}
	p.Account.Chars[0].Score.Level = 92
	p.Account.Chars[0].Equip[9] = item
	w.onEnterWorld(session, enterWorldPacket(0))
	if !p.InWorld || p.Char.Equip[9] != item || p.Char.Inv[0] != (model.Item{}) || clientEquipProjection(p.Char)[9] != item {
		t.Fatal("entry migrated or hid a valid necklace")
	}
	if w.planUnsupportedEquipment(p) {
		t.Fatal("repeated legacy migration changed a valid necklace")
	}
}

func TestMysticAmuletCriticalSwapDetachedAndRepeated(t *testing.T) {
	w, p, fallback, backend, queue, release := criticalCargoFixture(t)
	w.items = necklaceCatalog(t)
	p.Char.Score.Level = 92
	w.recalcPlayer(p.Char)
	item := model.Item{Index: 559, UID: "critical-mystic-amulet", Eff: [6]byte{43, 9}}
	p.Char.Inv[3] = item
	before := accountStateSnapshot(p.Account)
	request := criticalCargoSwapRequest(p, placeInv, 3, placeEquip, 9)
	w.onSwapItem(p.Session, request)
	proposal := awaitCriticalWrite(t, backend.entered)
	if proposal.accounts[0] == p.Account || proposal.accounts[0].Chars[p.CharSlot].Equip[9] != item {
		t.Fatal("critical swap omitted the detached necklace proposal")
	}
	w.onSwapItem(p.Session, request)
	assertCriticalCargoUnchanged(t, p, before, fallback)
	if len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("repeated pending swap wrote or published twice")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Equip[9] != item || p.Char.Inv[3] != (model.Item{}) {
		t.Fatal("acknowledged swap lost the necklace")
	}
	assertCriticalEquipmentPublication(t, p, request)
}
