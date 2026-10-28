package data

import (
	"path/filepath"
	"testing"
)

func TestMortalWeaponShopAdditionalEffects(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	want := map[string]int{"ShopWeaponsD": 20, "ShopWpnD2": 19, "ShopWpnE2": 10}
	for _, npc := range npcs {
		count, ok := want[npc.Name]
		if !ok {
			continue
		}
		if len(npc.Vende) != count || npc.Score == nil || npc.Score.Merchant != 1 {
			t.Fatalf("%s: invalid stock or merchant configuration", npc.Name)
		}
		for _, item := range npc.Vende {
			def, ok := catalog.Items[item.Index]
			if !ok {
				t.Fatalf("%s: unknown item %d", npc.Name, item.Index)
			}
			magic, tier, mobType := false, 0, 0
			for _, effect := range def.StaticEffects {
				switch effect.Name {
				case "EF_MAGIC":
					magic = effect.Value > 0
				case "EF_ITEMLEVEL":
					tier = effect.Value
				case "EF_MOBTYPE":
					mobType = effect.Value
				}
			}
			if tier != 4 && tier != 5 || mobType == 1 {
				t.Fatalf("%s: item %d is not a mortal D/E weapon", npc.Name, item.Index)
			}
			expected := [6]byte{2, 63}
			if magic {
				expected = [6]byte{60, 28}
			}
			if item.Eff != expected || item.UID != "" {
				t.Fatalf("%s item %d: effects=%v want=%v, UID=%q",
					npc.Name, item.Index, item.Eff, expected, item.UID)
			}
		}
		delete(want, npc.Name)
	}
	if len(want) != 0 {
		t.Fatalf("missing shops: %v", want)
	}
}

// Every mortal D/E weapon and armor set test shop must be spawned by an enabled generator,
// next to the other Armia test shops.
func TestMortalWeaponShopsHaveGenerators(t *testing.T) {
	geners, err := LoadChannelNPCGener(filepath.Join("..", "..", "data", "spawns", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	want := map[string]bool{"ShopWeaponsD": false, "ShopWpnD2": false, "ShopWeaponsE": false, "ShopWpnE2": false,
		"ShopSetD": false, "ShopSetE": false}
	for _, gener := range geners {
		if _, tracked := want[gener.Leader]; tracked && gener.Enabled {
			want[gener.Leader] = true
		}
	}
	for name, found := range want {
		if !found {
			t.Errorf("%s has no enabled generator", name)
		}
	}
}
