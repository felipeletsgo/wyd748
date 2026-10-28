package game

import (
	"encoding/json"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestNPCMountStockBootAndLivePreparation(t *testing.T) {
	for _, index := range []uint16{2330, 2360} {
		w := &World{rng: fixedRNG{value: 7}, npcs: []model.NPCDef{{Vende: []model.Item{{Index: index}, {Index: 411}}}}}
		w.initShopMounts()
		boot := w.npcs[0].Vende[0]
		live := model.NPCDef{Vende: []model.Item{{Index: index}, {Index: 411}}}
		w.prepareNPCShopMounts(&live)
		if live.Vende[0] != boot || boot.MountHP() == 0 || boot.MountLongev() != 17 || boot.MountFood() == 0 {
			t.Fatalf("mount %d differs between boot and live preparation: %+v %+v", index, boot, live.Vende[0])
		}
		live.Vende[0].SetMountHP(1234)
		live.Vende[0].SetMountLevel(5)
		live.Vende[0].SetMountFood(12)
		live.Vende[0].SetMountLongev(24)
		before := append([]model.Item(nil), live.Vende...)
		w.prepareNPCShopMounts(&live)
		w.prepareNPCShopMounts(&live)
		if !reflect.DeepEqual(live.Vende, before) {
			t.Fatal("repeated preparation changed explicit mount state or ordinary stock")
		}
		live.Vende[0].SetMountHP(0)
		before = append([]model.Item(nil), live.Vende...)
		w.prepareNPCShopMounts(&live)
		if !reflect.DeepEqual(live.Vende, before) {
			t.Fatal("explicit dead mount was resurrected")
		}
	}
}

func TestNPCLiveMountEditWhilePurchasePersists(t *testing.T) {
	w, p, _, shop, backend, queue, release := criticalShopFixture(t, "buy")
	w.items[2360] = model.ItemDef{Index: 2360, Price: 1000}
	shop.Def.Vende = []model.Item{{Index: 2360}}
	w.rng = fixedRNG{value: 7}
	w.prepareNPCShopMounts(shop.Def)
	original := shop.Def.Vende[0]
	w.handle(command{s: p.Session, pkt: criticalShopRequest("buy", p.ID, shop.ID)})
	write := awaitCriticalWrite(t, backend.entered)
	w.invalidateNPCShopWindows(shop.Def.Name)
	shop.Def.Vende = []model.Item{{Index: 411}}
	if !w.deferCriticalCommand(command{s: p.Session, pkt: useNPCPacket(shop.ID)}) {
		t.Fatal("merchant reopen bypassed purchase reservation")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	purchased := p.Char.Inv[1]
	if purchased != write.accounts[0].Chars[p.CharSlot].Inv[1] || purchased.Index != 2360 || purchased.MountHP() != original.MountHP() || purchased.MountLongev() != original.MountLongev() || purchased.MountFood() != original.MountFood() || p.Char.Gold != 9000 {
		t.Fatal("concurrent edit changed the admitted mount purchase")
	}
}

func TestNPCLiveMountStockCriticalPurchase(t *testing.T) {
	for _, failure := range []bool{false, true} {
		t.Run(map[bool]string{false: "committed", true: "uncertain"}[failure], func(t *testing.T) {
			w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, "buy")
			defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
			if err != nil {
				t.Fatal(err)
			}
			catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
			if err != nil {
				t.Fatal(err)
			}
			defs[0].Vende = []model.Item{{Index: 2360}}
			w.npcs, w.items, w.channel, w.rng = defs, catalog.Items, 1, fixedRNG{value: 7}
			raw, _ := json.Marshal(defs[0])
			w.npcDefinitionState = &npcDefinitionRuntime{source: map[string]json.RawMessage{"Aki": raw}, active: map[string]bool{"Aki": true}, versions: map[string]uint32{"Aki": 1}}
			shop.Def = &w.npcs[0]
			w.initShopMounts()
			// The database retains the zero-state blueprint. Even an unrelated
			// attribute edit must prepare it before replacing live stock.
			var edited model.NPCDef
			if err := json.Unmarshal(raw, &edited); err != nil {
				t.Fatal(err)
			}
			edited.ExpReward++
			next, _ := json.Marshal(edited)
			entry := store.NPCDefinitionRecord{Channel: 1, Name: "Aki", Version: 2, Definition: next}
			w.installNPCDefinitionChanges([]store.NPCDefinitionRecord{entry})
			prepared := w.npcs[0].Vende[0]
			if w.npcDefinitionState.versions["Aki"] != 2 || prepared.MountHP() == 0 || prepared.MountLongev() == 0 || prepared.MountFood() == 0 {
				t.Fatal("live revision installed a dead mount")
			}
			w.installNPCDefinitionChanges([]store.NPCDefinitionRecord{entry})
			if w.npcs[0].Vende[0] != prepared {
				t.Fatal("revision replay changed stock")
			}
			p.Char.Gold = 100000
			w.onUseNPC(p.Session, useNPCPacket(shop.ID))
			drainPackets(p)
			if failure {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			before := accountStateSnapshot(p.Account)
			w.onBuyItem(p.Session, criticalShopRequest("buy", p.ID, shop.ID))
			proposal := awaitCriticalWrite(t, backend.entered)
			item := proposal.accounts[0].Chars[p.CharSlot].Inv[1]
			if item.Index != 2360 || item.MountHP() != prepared.MountHP() || item.MountFood() != prepared.MountFood() || item.MountLongev() != prepared.MountLongev() {
				t.Fatal("purchase proposal lost prepared mount state")
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("purchase mutated account before persistence")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if failure {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !p.PersistencePoisoned || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("uncertain purchase changed or published economy")
				}
			} else if p.Char.Inv[1] != item || p.Char.Gold != proposal.accounts[0].Chars[p.CharSlot].Gold || fallback.saves != 0 {
				t.Fatal("committed purchase differs from prepared stock")
			}
		})
	}
}
