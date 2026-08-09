package data

import (
	"path/filepath"
	"strings"
	"testing"
)

func loadTestShopFixture(t *testing.T) (map[string]map[uint16]struct{}, map[uint16]struct{}) {
	t.Helper()
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatiles(filepath.Join(root, "volatiles.json"), catalog.Items, catalog.Skills)
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

func TestCadaItemVolatileTemLojaDeTeste(t *testing.T) {
	shops, volatileItems := loadTestShopFixture(t)
	covered := itemsInShops(shops, func(name string) bool {
		return strings.HasPrefix(name, "ShopVol") || name == "ShopFairy" ||
			name == "ShopUseItem" || name == "ShopUse20A" || name == "ShopUse20B" ||
			name == "ShopPhaseD" || name == "ShopWater" || name == "ShopArch"
	})
	for item := range volatileItems {
		if _, ok := covered[item]; !ok {
			t.Errorf("volatile item %d ausente da matriz de lojas de teste", item)
		}
	}
	for name, stock := range shops {
		if strings.HasPrefix(name, "ShopVolTest") && len(stock) > 27 {
			t.Errorf("%s possui %d itens; o client 7.48 exibe no maximo 27", name, len(stock))
		}
	}
}

func TestMontariasTrajesEFuncoesTemporizadasTemLojaDeTeste(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	mounts := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopMt") })
	for _, ranges := range [][2]uint16{{2300, 2328}, {2330, 2358}, {2360, 2388}} {
		for item := ranges[0]; item <= ranges[1]; item++ {
			if _, ok := mounts[item]; !ok {
				t.Errorf("estagio de montaria %d ausente das lojas ShopMt", item)
			}
		}
	}
	timed := []uint16{3315, 3316, 3317, 3344, 3345, 3346, 3347, 3348, 3349,
		3351, 3352, 3353, 3354, 3355, 3356, 3357, 3438,
		3980, 3981, 3982, 3983, 3984, 3985, 3986, 3987, 3988, 3989}
		timeShop := itemsInShops(shops, func(name string) bool { return name == "ShopMtTime" })
		for _, item := range timed {
			if _, ok := timeShop[item]; !ok {
				t.Errorf("item temporizado de montaria %d ausente de ShopMtTime", item)
			}
		}

	costumes := itemsInShops(shops, func(name string) bool { return name == "ShopCostume" })
	for item := uint16(4151); item <= 4156; item++ {
		if _, ok := costumes[item]; !ok {
			t.Errorf("traje %d ausente de ShopCostume", item)
		}
	}
	firal := itemsInShops(shops, func(name string) bool { return name == "ShopFiral" })
	for item := uint16(3397); item <= 3416; item++ {
		if _, ok := firal[item]; !ok {
			t.Errorf("item de aparencia/tintura %d ausente de ShopFiral", item)
		}
	}
}

func TestNovasLojasDeTesteEstaoNoBlocoDeArmia(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	gens, err := LoadNPCGener(filepath.Join(root, "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	want := []string{"ShopVolTest1", "ShopVolTest2", "ShopVolTest3", "ShopVolTest4",
		"ShopVolTest5", "ShopVolTest6", "ShopVolTest7", "ShopMtEgg1", "ShopMtEgg2",
		"ShopMtBaby1", "ShopMtBaby2", "ShopMtAdult1", "ShopMtAdult2", "ShopMtTime",
		"ShopCostume", "ShopFiral"}
	seen := make(map[string]bool, len(want))
	for _, gen := range gens {
		if !containsShopName(want, gen.Leader) {
			continue
		}
		point := gen.Segments[0]
		if point.X < 2090 || point.X > 2130 || point.Y < 2090 || point.Y > 2120 {
			t.Errorf("%s esta fora do bloco de Armia: (%d,%d)", gen.Leader, point.X, point.Y)
		}
		seen[gen.Leader] = true
	}
	for _, name := range want {
		if !seen[name] {
			t.Errorf("loja de teste %s nao possui gerador em NPCGener", name)
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
