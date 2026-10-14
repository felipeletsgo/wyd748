package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func loadTestShopFixture(t *testing.T) (map[string]map[uint16]struct{}, map[uint16]struct{}) {
	t.Helper()
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	shops := make(map[string]map[uint16]struct{})
	for _, npc := range npcs {
		if !strings.HasPrefix(npc.Name, "Shop") {
			continue
		}
		items := make(map[uint16]struct{}, len(npc.Vende))
		for _, item := range npc.Vende {
			if item.Index != 0 {
				items[item.Index] = struct{}{}
			}
		}
		shops[npc.Name] = items
	}
	volatileItems := make(map[uint16]struct{}, len(volatiles.ItemCodes))
	for item := range volatiles.ItemCodes {
		volatileItems[item] = struct{}{}
	}
	return shops, volatileItems
}

func itemsInShops(shops map[string]map[uint16]struct{}, names func(string) bool) map[uint16]struct{} {
	items := make(map[uint16]struct{})
	for name, stock := range shops {
		if !names(name) {
			continue
		}
		for item := range stock {
			items[item] = struct{}{}
		}
	}
	return items
}

func requireShopItems(t *testing.T, shops map[string]map[uint16]struct{}, name string, want []uint16) {
	t.Helper()
	stock, ok := shops[name]
	if !ok {
		t.Fatalf("shop %s missing", name)
	}
	if len(stock) > 27 {
		t.Fatalf("%s has %d items; the 7.48 client shows at most 27", name, len(stock))
	}
	for _, item := range want {
		if _, ok := stock[item]; !ok {
			t.Errorf("%s does not sell item %d", name, item)
		}
	}
}

func TestOneShotConsumablesDoNotLeaveTestShopsAsArtificialStacks(t *testing.T) {
	const effectAmount byte = 61
	want := map[string]map[uint16]bool{
		"ShopTest": {412: false, 413: false, 4140: false},
	}

	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	for _, npc := range npcs {
		items, ok := want[npc.Name]
		if !ok {
			continue
		}
		for _, item := range npc.Vende {
			if _, tracked := items[item.Index]; !tracked {
				continue
			}
			items[item.Index] = true
			// These consumables stand for one attempt per UID; EF_AMOUNT here would
			// make a purchase look unconsumed and hide the removal of the last unit.
			for slot := 0; slot < 3; slot++ {
				if item.Eff[slot*2] == effectAmount {
					t.Errorf("%s item %d has an artificial EF_AMOUNT=%d", npc.Name, item.Index, item.Eff[slot*2+1])
				}
			}
		}
	}
	for shop, items := range want {
		for item, found := range items {
			if !found {
				t.Errorf("%s does not sell the single consumable %d", shop, item)
			}
		}
	}
}

// ShopTest2 deliberately sells Ori (412) and Lac (413) as 120-unit stacks for
// refinement testing; consumeOne removes one unit per attempt.
func TestShopTest2SellsRefinementPowderStacks(t *testing.T) {
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	found := map[uint16]bool{}
	for _, npc := range npcs {
		if npc.Name != "ShopTest2" {
			continue
		}
		for _, item := range npc.Vende {
			if item.Index == 412 || item.Index == 413 {
				found[item.Index] = item.Eff[0] == 61 && item.Eff[1] == 120
			}
		}
	}
	if !found[412] || !found[413] {
		t.Fatalf("ShopTest2 must sell Ori and Lac with EF_AMOUNT 120: %v", found)
	}
}

func TestEveryVolatileItemHasTestShop(t *testing.T) {
	shops, volatileItems := loadTestShopFixture(t)
	covered := itemsInShops(shops, func(name string) bool {
		return strings.HasPrefix(name, "ShopVol") || name == "ShopFairy" ||
			name == "ShopUseItem" || name == "ShopUse20A" || name == "ShopUse20B" ||
			name == "ShopPhaseD" || name == "ShopWater" || name == "ShopArch"
	})
	for item := range volatileItems {
		// Wyden coins hold donate and are sold only by the premium shop.
		if item == 3393 || item == 3394 || item == 3395 || item == 3396 || item == 3441 {
			continue
		}
		if _, ok := covered[item]; !ok {
			t.Errorf("volatile item %d missing from the test-shop matrix", item)
		}
	}
	for name, stock := range shops {
		if strings.HasPrefix(name, "ShopVolTest") && len(stock) > 27 {
			t.Errorf("%s has %d items; the 7.48 client shows at most 27", name, len(stock))
		}
	}
}

func TestMountsCostumesAndTimedFunctionsHaveTestShop(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	mounts := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopMt") })
	for _, ranges := range [][2]uint16{{2300, 2328}, {2330, 2358}, {2360, 2388}} {
		for item := ranges[0]; item <= ranges[1]; item++ {
			if _, ok := mounts[item]; !ok {
				t.Errorf("mount stage %d missing from the ShopMt shops", item)
			}
		}
	}
	timed := []uint16{3315, 3316, 3317, 3344, 3345, 3346, 3347, 3348, 3349,
		3351, 3352, 3353, 3354, 3355, 3356, 3357, 3438,
		3980, 3981, 3982, 3983, 3984, 3985, 3986, 3987, 3988, 3989}
	timeShop := itemsInShops(shops, func(name string) bool { return name == "ShopMtTime" })
	for _, item := range timed {
		if _, ok := timeShop[item]; !ok {
			t.Errorf("timed mount item %d missing from ShopMtTime", item)
		}
	}

	costumes := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopCostum") })
	if len(costumes) != 143 {
		t.Errorf("ShopCostume shops have %d distinct costumes; want 143", len(costumes))
	}
	for name, stock := range shops {
		if strings.HasPrefix(name, "ShopCostum") && len(stock) > 27 {
			t.Errorf("%s has %d costumes; the client limit is 27", name, len(stock))
		}
	}
	krMounts := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopKRMt") })
	if len(krMounts) != 99 {
		t.Errorf("ShopKRMt shops have %d distinct mounts; want 99", len(krMounts))
	}
	for name, stock := range shops {
		if strings.HasPrefix(name, "ShopKRMt") && len(stock) > 27 {
			t.Errorf("%s has %d mounts; the client limit is 27", name, len(stock))
		}
	}
	firal := itemsInShops(shops, func(name string) bool { return name == "ShopFiral" })
	for item := uint16(3397); item <= 3416; item++ {
		if _, ok := firal[item]; !ok {
			t.Errorf("appearance/dye item %d missing from ShopFiral", item)
		}
	}
}

func TestKRMountsUsePremium748Contract(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	type mountItem struct {
		Item      uint16 `json:"item"`
		Name      string `json:"name"`
		Available *bool  `json:"available"`
	}
	var manifest struct {
		BaseItem uint16      `json:"baseItem"`
		Items    []mountItem `json:"items"`
	}
	// The manifest belongs to the client runtime, a sibling of the Go module.
	raw, err := os.ReadFile(filepath.Join("..", "..", "..", "tmproject", "client748", "Mounts-KR.json"))
	if err != nil {
		t.Fatal(err)
	}
	if err := json.Unmarshal(raw, &manifest); err != nil {
		t.Fatal(err)
	}
	if manifest.BaseItem != 342 || len(manifest.Items) != 101 {
		t.Fatalf("unexpected KR manifest: base=%d items=%d", manifest.BaseItem, len(manifest.Items))
	}
	shire := catalog.Items[manifest.BaseItem]
	const krMountIconMesh = 2821
	if timed := catalog.Items[3980]; timed.Mesh != krMountIconMesh {
		t.Fatalf("timed horse icon mesh = %d; want %d", timed.Mesh, krMountIconMesh)
	}
	shops, _ := loadTestShopFixture(t)
	stock := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopKRMt") })
	unavailable := map[uint16]struct{}{4211: {}, 4235: {}}
	available := 0
	for _, item := range manifest.Items {
		if item.Available != nil && !*item.Available {
			if _, ok := unavailable[item.Item]; !ok {
				t.Errorf("KR mount %d was marked unavailable without cataloged evidence", item.Item)
			}
			if _, ok := catalog.Items[item.Item]; ok {
				t.Errorf("incomplete KR mount %d is still materialized in the authoritative catalog", item.Item)
			}
			if _, ok := stock[item.Item]; ok {
				t.Errorf("incomplete KR mount %d was offered in the shops", item.Item)
			}
			continue
		}
		available++
		def, ok := catalog.Items[item.Item]
		if !ok {
			t.Errorf("KR mount %d missing from the catalog", item.Item)
			continue
		}
		// The Shire face row's bush mesh (7) and model class (43) are not item
		// data: KR mounts use the timed-horse icon and allow every class.
		if def.Name != item.Name || def.Mesh != krMountIconMesh || def.Pos != shire.Pos ||
			def.Price != shire.Price || def.Grade != shire.Grade {
			t.Errorf("KR mount %d differs from the Shire base: %+v", item.Item, def)
		}
		effects := make(map[string]int, len(def.StaticEffects))
		for _, effect := range def.StaticEffects {
			effects[effect.Name] = effect.Value
		}
		if effects["EF_CLASS"] != 255 || effects["EF_RANGE"] != 2 ||
			effects["EF_NOTRADE"] != 1 || effects["EF_WDAY"] != 30 ||
			effects["EF_RUNSPEED"] != 6 || effects["EF_DAMAGE"] != 520 ||
			effects["EF_MAGIC"] != 65 {
			t.Errorf("KR mount %d differs from the premium contract: %+v", item.Item, effects)
		}
		if _, ok := stock[item.Item]; !ok {
			t.Errorf("KR mount %d missing from the shops", item.Item)
		}
	}
	if available != 99 {
		t.Fatalf("the manifest has %d complete KR mounts; want 99", available)
	}
}

func TestCompleteKRCostumesKeep748Contract(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	type costumeItem struct {
		Item      uint16 `json:"item"`
		Name      string `json:"name"`
		ClassMask int    `json:"classMask"`
		Available bool   `json:"available"`
	}
	var manifest struct {
		Items []costumeItem `json:"items"`
	}
	raw, err := os.ReadFile(filepath.Join("..", "..", "..", "tmproject", "client748", "Costumes-KR.json"))
	if err != nil {
		t.Fatal(err)
	}
	if err := json.Unmarshal(raw, &manifest); err != nil {
		t.Fatal(err)
	}
	shops, _ := loadTestShopFixture(t)
	stock := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopCostum") })
	available := 0
	for _, item := range manifest.Items {
		if !item.Available {
			if _, ok := stock[item.Item]; ok {
				t.Errorf("KR costume %d without complete assets was offered in the shop", item.Item)
			}
			continue
		}
		available++
		if _, ok := stock[item.Item]; !ok {
			t.Errorf("complete KR costume %d missing from the shops", item.Item)
		}
		def, ok := catalog.Items[item.Item]
		if !ok {
			t.Errorf("KR costume %d missing from the catalog", item.Item)
			continue
		}
		// The catalog loader moves native slot-13 costumes to the costume slot.
		if def.Name != item.Name || def.Mesh != 36 || def.Pos != model.CostumeSlotMask {
			t.Errorf("metadata of costume %d differ: %+v", item.Item, def)
		}
		effects := make(map[string]int, len(def.StaticEffects))
		for _, effect := range def.StaticEffects {
			effects[effect.Name] = effect.Value
		}
		if effects["EF_CLASS"] != item.ClassMask || effects["EF_AC"] != 80 ||
			effects["EF_SAVEMANA"] != 10 || effects["EF_NOTRADE"] != 1 ||
			effects["EF_WDAY"] != 30 {
			t.Errorf("effects of costume %d differ: %+v", item.Item, effects)
		}
	}
	if available != 143 {
		t.Fatalf("the manifest has %d complete costumes; want 143", available)
	}
}

func TestDESetsOfEveryClassHaveTestShop(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	requireShopItems(t, shops, "ShopSetD", []uint16{
		1206, 1209, 1212, 1215, 1218, // TK - Golden Embersed
		1341, 1344, 1347, 1350, 1353, // FM - Mithril Robe
		1491, 1494, 1497, 1500, 1503, // BM - Meta Elemental
		1641, 1644, 1647, 1650, 1653, // HT - Gold Curve Web
	})
	requireShopItems(t, shops, "ShopSetE", []uint16{
		1225, 1226, 1227, 1228, 1229, // TK - Slayer
		1360, 1361, 1362, 1363, 1364, // FM - Templess
		1510, 1511, 1512, 1513, 1514, // BM - Raven
		1660, 1661, 1662, 1663, 1664, // HT - Region
	})
}

func TestEveryCatalogDEWeaponHasTestShop(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	dStock := itemsInShops(shops, func(name string) bool {
		return strings.HasPrefix(name, "ShopWeaponsD") || name == "ShopWpnD2"
	})
	eStock := itemsInShops(shops, func(name string) bool {
		return strings.HasPrefix(name, "ShopWeaponsE") || name == "ShopWpnE2"
	})
	for name, stock := range shops {
		if (strings.HasPrefix(name, "ShopWeaponsD") || strings.HasPrefix(name, "ShopWeaponsE") ||
			name == "ShopWpnD2" || name == "ShopWpnE2") && len(stock) > 27 {
			t.Fatalf("%s has %d items; the 7.48 client shows at most 27", name, len(stock))
		}
	}
	for index, def := range catalog.Items {
		itemLevel := 0
		weapon := false
		for _, effect := range def.StaticEffects {
			switch effect.Name {
			case "EF_ITEMLEVEL":
				itemLevel = effect.Value
			case "EF_WTYPE":
				weapon = true
			}
		}
		if !weapon {
			continue
		}
		switch itemLevel {
		case 4:
			if _, ok := dStock[index]; !ok {
				t.Errorf("weapon D %d (%s) missing from the ShopWeaponsD* shops", index, def.Name)
			}
		case 5:
			if _, ok := eStock[index]; !ok {
				t.Errorf("weapon E %d (%s) missing from the ShopWeaponsE* shops", index, def.Name)
			}
		}
	}
}

func TestDEShieldsHaveTestShop(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	requireShopItems(t, shops, "ShopShldDE", []uint16{1709, 1710, 1711, 1712})
}

func TestNewTestShopsAreInTheArmiaBlock(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	gens, err := LoadNPCGener(filepath.Join(root, "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	extra, err := LoadNPCGener(filepath.Join(root, "NPCGenerTest.txt"))
	if err != nil {
		t.Fatal(err)
	}
	gens = append(gens, extra...)
	want := []string{"ShopVolTest1", "ShopVolTest2", "ShopVolTest3", "ShopVolTest4",
		"ShopVolTest5", "ShopVolTest6", "ShopVolTest7", "ShopMtEgg1", "ShopMtEgg2",
		"ShopMtBaby1", "ShopMtBaby2", "ShopMtAdult1", "ShopMtAdult2", "ShopMtTime",
		"ShopCostume", "ShopCostum02", "ShopCostum03", "ShopCostum04", "ShopCostum05", "ShopCostum06",
		"ShopKRMt01", "ShopKRMt02", "ShopKRMt03", "ShopKRMt04",
		"ShopFiral", "ShopSetD", "ShopSetE", "ShopWeaponsD", "ShopWeaponsE",
		"ShopWpnD2", "ShopWpnE2", "ShopShldDE"}
	seen := make(map[string]bool, len(want))
	for _, gen := range gens {
		if !containsShopName(want, gen.Leader) {
			continue
		}
		point := gen.Segments[0]
		if point.X < 2090 || point.X > 2130 || point.Y < 2090 || point.Y > 2120 {
			t.Errorf("%s is outside the Armia block: (%d,%d)", gen.Leader, point.X, point.Y)
		}
		seen[gen.Leader] = true
	}
	for _, name := range want {
		if !seen[name] {
			t.Errorf("test shop %s has no loadable generator", name)
		}
	}
}

func containsShopName(names []string, want string) bool {
	for _, name := range names {
		if name == want {
			return true
		}
	}
	return false
}
