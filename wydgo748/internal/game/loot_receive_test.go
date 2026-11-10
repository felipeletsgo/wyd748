package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestGoldFairyLootGroupingAndAtomicOverflow(t *testing.T) {
	for _, fairy := range []uint16{0, 3914, 3915} {
		w := fairyTestWorld()
		ch := &model.Char{}
		ch.Equip[fairySlot] = model.Item{Index: fairy}
		ch.Inv[0] = testMergeStack("existing", 250)
		incoming := testMergeStack("incoming", 10)
		slots, ok := w.receiveLoot(ch, incoming, 5)
		if !ok {
			t.Fatal("loot rejected with free inventory")
		}
		if fairy == 3915 {
			if len(slots) != 2 || itemStackAmount(ch.Inv[0]) != 255 || itemStackAmount(ch.Inv[5]) != 5 || ch.Inv[5].UID != "incoming" {
				t.Fatal("gold fairy did not preserve both quantities and identities")
			}
		} else if len(slots) != 1 || itemStackAmount(ch.Inv[0]) != 250 || ch.Inv[5] != incoming {
			t.Fatal("grouping was enabled without a gold fairy")
		}
	}
	w := fairyTestWorld()
	ch := &model.Char{}
	ch.Equip[fairySlot] = model.Item{Index: 3915}
	for i := range ch.Inv {
		ch.Inv[i] = model.Item{Index: 999}
	}
	ch.Inv[0] = testMergeStack("target", 250)
	before := ch.Inv
	if _, ok := w.receiveLoot(ch, testMergeStack("origin", 10), -1); ok || ch.Inv != before {
		t.Fatal("failed residual placement partially changed a full inventory")
	}
	if _, ok := w.receiveLoot(ch, testMergeStack("origin", 5), -1); !ok || itemStackAmount(ch.Inv[0]) != 255 {
		t.Fatal("a fully mergeable stack needs no free slot")
	}
}

func TestGoldFairyGroupsSinglePowdersWithoutAmountEffect(t *testing.T) {
	w := fairyTestWorld()
	for _, index := range []uint16{412, 413} {
		ch := &model.Char{}
		ch.Equip[fairySlot] = model.Item{Index: 3915}
		ch.Inv[0] = model.Item{Index: index, UID: "target"}
		if _, ok := w.receiveLoot(ch, model.Item{Index: index, UID: "incoming"}, -1); !ok ||
			itemStackAmount(ch.Inv[0]) != 2 || ch.Inv[1].Index != 0 {
			t.Fatal("single powders were not grouped")
		}
	}
}

func TestGoldFairyGroundPickupCommitsOnce(t *testing.T) {
	w, p, g, _, backend, queue, release := criticalPickupFixture(t)
	w.items[3915] = fairyTestDef(3915, 7)
	p.Char.Equip[fairySlot] = model.Item{Index: 3915}
	p.Char.Inv[0] = testMergeStack("target", 250)
	g.Item = testMergeStack("origin", 10)
	before := p.Char.Inv
	request := criticalPickupRequest(g.ID, 7)
	w.onGetItem(p.Session, request)
	write := awaitCriticalWrite(t, backend.entered)
	if p.Char.Inv != before || itemStackAmount(write.accounts[0].Chars[p.CharSlot].Inv[0]) != 255 {
		t.Fatal("pickup published early or proposed an incorrect merge")
	}
	w.onGetItem(p.Session, request)
	if len(queue.pending) != 1 {
		t.Fatal("repeated pickup created another write")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if itemStackAmount(p.Char.Inv[0]) != 255 || itemStackAmount(p.Char.Inv[7]) != 5 || w.groundItems[g.ID] != nil {
		t.Fatal("pickup did not commit both stacks and remove the ground item")
	}
	w.onGetItem(p.Session, request)
	if len(queue.pending) != 0 || itemStackAmount(p.Char.Inv[7]) != 5 {
		t.Fatal("replayed pickup duplicated loot")
	}
}

func TestMobCoinDropConvertsWithoutInventorySpace(t *testing.T) {
	for _, gold := range []uint32{100, maxCharacterGold - 3} {
		w := fairyTestWorld()
		w.items[454] = model.ItemDef{Index: 454, Price: 4}
		w.volatiles = model.VolatileCatalog{Items: map[uint16]model.VolatileRule{454: {Action: "ground_gold", ValueSource: "price"}}, ItemCodes: map[uint16]int{454: 2}}
		p := &Player{Char: &model.Char{Gold: gold}}
		for i := range p.Char.Inv {
			p.Char.Inv[i] = model.Item{Index: 999}
		}
		carry := make([]model.Item, 12)
		carry[11] = model.Item{Index: 454}
		m := &Mob{Def: &model.NPCDef{Carry: carry, Score: &model.Score{Level: 10}}}
		before := p.Char.Inv
		drops := w.planMobDrops(p, m)
		if p.Char.Inv != before {
			t.Fatal("currency occupied an inventory slot")
		}
		if gold == 100 {
			if p.Char.Gold != 104 || len(drops) != 0 {
				t.Fatal("coin was not converted exactly once")
			}
		} else if p.Char.Gold != gold || len(drops) != 1 || drops[0].inventoryPos != -1 {
			t.Fatal("gold cap destroyed value instead of preserving the coin on the ground")
		}
	}
}
