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

func requireShopItems(t *testing.T, shops map[string]map[uint16]struct{}, name string, want []uint16) {
	t.Helper()
	stock, ok := shops[name]
	if !ok {
		t.Fatalf("loja %s ausente", name)
	}
	if len(stock) > 27 {
		t.Fatalf("%s possui %d itens; o client 7.48 exibe no maximo 27", name, len(stock))
	}
	for _, item := range want {
		if _, ok := stock[item]; !ok {
			t.Errorf("%s nao vende o item %d", name, item)
		}
	}
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

func TestSetsDEDeTodasAsClassesTemLojaDeTeste(t *testing.T) {
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

func TestTodasAsArmasCanonicasDETemLojaDeTeste(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	dStock := shops["ShopWeaponsD"]
	eStock := shops["ShopWeaponsE"]
	if len(dStock) > 27 || len(eStock) > 27 {
		t.Fatalf("lojas de armas excedem a janela 7.48: D=%d E=%d", len(dStock), len(eStock))
	}
	for index, def := range catalog.Items {
		// A faixa 800..949 e a tabela canonica de armas mortais. Variantes
		// Ancient/Legend posteriores reutilizam os mesmos modelos e nao fazem
		// parte desta matriz de compra basica.
		if index < 800 || index >= 950 {
			continue
		}
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
				t.Errorf("arma D %d (%s) ausente de ShopWeaponsD", index, def.Name)
			}
		case 5:
			if _, ok := eStock[index]; !ok {
				t.Errorf("arma E %d (%s) ausente de ShopWeaponsE", index, def.Name)
			}
		}
	}
}

func TestNovasLojasDeTesteEstaoNoBlocoDeArmia(t *testing.T) {
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
		"ShopCostume", "ShopFiral", "ShopSetD", "ShopSetE", "ShopWeaponsD", "ShopWeaponsE"}
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
			t.Errorf("loja de teste %s nao possui gerador carregavel", name)
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
