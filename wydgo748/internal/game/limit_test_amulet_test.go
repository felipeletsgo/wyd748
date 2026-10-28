package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// The Limit_Test_Amulet (6015) is a staff test item sold by ShopAmulet: it
// reuses the Crystal Amulet appearance and adds 100000 to each attribute so
// the score, HP and damage limits can be exercised in game.
func TestLimitTestAmuletAddsAttributesAndIsSold(t *testing.T) {
	const amulet = 6015
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	def, ok := catalog.Items[amulet]
	if !ok || def.Name != "Limit_Test_Amulet" || def.Pos != 512 || def.ReqLevel != 0 {
		t.Fatalf("item %d = %+v", amulet, def)
	}
	w := &World{items: catalog.Items}
	ch := &model.Char{Class: 0, Score: testScore(model.Score{Level: 100, Str: 8, Int: 4, Dex: 7, Con: 6})}
	w.recalcPlayer(ch)
	before := effectiveScore(ch)
	ch.Equip[9] = model.Item{Index: amulet}
	w.recalcPlayer(ch)
	after := effectiveScore(ch)
	for name, delta := range map[string]int64{
		"STR": int64(after.Str) - int64(before.Str), "INT": int64(after.Int) - int64(before.Int),
		"DEX": int64(after.Dex) - int64(before.Dex), "CON": int64(after.Con) - int64(before.Con),
	} {
		if delta != 100000 {
			t.Fatalf("%s grew by %d, want 100000", name, delta)
		}
	}
	if after.MaxHP <= before.MaxHP || after.Attack <= before.Attack {
		t.Fatalf("derived score did not grow: HP %d->%d attack %d->%d",
			before.MaxHP, after.MaxHP, before.Attack, after.Attack)
	}

	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	for _, npc := range npcs {
		if npc.Name != "ShopAmulet" {
			continue
		}
		for _, item := range npc.Vende {
			if item.Index == amulet {
				return
			}
		}
	}
	t.Fatal("ShopAmulet does not sell the Limit_Test_Amulet")
}
