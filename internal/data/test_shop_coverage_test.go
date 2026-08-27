package data

import (
	"encoding/json"
	"os"
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

func TestOneShotConsumablesDoNotLeaveTestShopsAsArtificialStacks(t *testing.T) {
	const effectAmount byte = 61
	want := map[string]map[uint16]bool{
		"ShopTest":  {412: false, 413: false, 4140: false},
		"ShopTest2": {412: false, 413: false},
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
			// Estes consumiveis representam uma tentativa por UID; EF_AMOUNT aqui faria
			// uma compra parecer nao consumida e esconderia a remocao da ultima unidade.
			for slot := 0; slot < 3; slot++ {
				if item.Eff[slot*2] == effectAmount {
					t.Errorf("%s item %d possui EF_AMOUNT artificial=%d", npc.Name, item.Index, item.Eff[slot*2+1])
				}
			}
		}
	}
	for shop, items := range want {
		for item, found := range items {
			if !found {
				t.Errorf("%s nao vende o consumivel unitario %d", shop, item)
			}
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

	costumes := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopCostum") })
	if len(costumes) != 135 {
		t.Errorf("lojas ShopCostume possuem %d trajes distintos; esperado 135", len(costumes))
	}
	for name, stock := range shops {
		if strings.HasPrefix(name, "ShopCostum") && len(stock) > 27 {
			t.Errorf("%s possui %d trajes; limite do client e 27", name, len(stock))
		}
	}
	krMounts := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopKRMt") })
	if len(krMounts) != 59 {
		t.Errorf("lojas ShopKRMt possuem %d montarias distintas; esperado 59", len(krMounts))
	}
	for name, stock := range shops {
		if strings.HasPrefix(name, "ShopKRMt") && len(stock) > 27 {
			t.Errorf("%s possui %d montarias; limite do client e 27", name, len(stock))
		}
	}
	firal := itemsInShops(shops, func(name string) bool { return name == "ShopFiral" })
	for item := uint16(3397); item <= 3416; item++ {
		if _, ok := firal[item]; !ok {
			t.Errorf("item de aparencia/tintura %d ausente de ShopFiral", item)
		}
	}
}

func TestMontariasKRUsamContratoPremium748(t *testing.T) {
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
	raw, err := os.ReadFile(filepath.Join("..", "..", "client748", "Mounts-KR.json"))
	if err != nil {
		t.Fatal(err)
	}
	if err := json.Unmarshal(raw, &manifest); err != nil {
		t.Fatal(err)
	}
	if manifest.BaseItem != 342 || len(manifest.Items) != 62 {
		t.Fatalf("manifesto KR inesperado: base=%d itens=%d", manifest.BaseItem, len(manifest.Items))
	}
	shire := catalog.Items[manifest.BaseItem]
	shops, _ := loadTestShopFixture(t)
	stock := itemsInShops(shops, func(name string) bool { return strings.HasPrefix(name, "ShopKRMt") })
	unavailable := map[uint16]struct{}{4211: {}, 4235: {}, 6003: {}}
	available := 0
	for _, item := range manifest.Items {
		if item.Available != nil && !*item.Available {
			if _, ok := unavailable[item.Item]; !ok {
				t.Errorf("montaria KR %d foi marcada indisponivel sem evidencia catalogada", item.Item)
			}
			if _, ok := catalog.Items[item.Item]; ok {
				t.Errorf("montaria KR incompleta %d continua materializada no catalogo autoritativo", item.Item)
			}
			if _, ok := stock[item.Item]; ok {
				t.Errorf("montaria KR incompleta %d foi exposta nas lojas", item.Item)
			}
			continue
		}
		available++
		def, ok := catalog.Items[item.Item]
		if !ok {
			t.Errorf("montaria KR %d ausente do catalogo", item.Item)
			continue
		}
		if def.Name != item.Name || def.Mesh != shire.Mesh || def.Pos != shire.Pos ||
			def.Price != shire.Price || def.Grade != shire.Grade {
			t.Errorf("montaria KR %d divergiu da base Shire: %+v", item.Item, def)
		}
		effects := make(map[string]int, len(def.StaticEffects))
		for _, effect := range def.StaticEffects {
			effects[effect.Name] = effect.Value
		}
		if effects["EF_CLASS"] != 43 || effects["EF_RANGE"] != 2 ||
			effects["111"] != 1 || effects["EF_WDAY"] != 30 ||
			effects["EF_RUNSPEED"] != 6 || effects["EF_DAMAGE"] != 520 ||
			effects["EF_MAGIC"] != 65 {
			t.Errorf("montaria KR %d divergiu do contrato premium: %+v", item.Item, effects)
		}
		if _, ok := stock[item.Item]; !ok {
			t.Errorf("montaria KR %d ausente das lojas", item.Item)
		}
	}
	if available != 59 {
		t.Fatalf("manifesto possui %d montarias KR completas; esperado 59", available)
	}
}

func TestTrajesKRCompletosMantemContrato748(t *testing.T) {
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
	raw, err := os.ReadFile(filepath.Join("..", "..", "client748", "Costumes-KR.json"))
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
				t.Errorf("traje KR %d sem assets completos foi exposto na loja", item.Item)
			}
			continue
		}
		available++
		if _, ok := stock[item.Item]; !ok {
			t.Errorf("traje KR completo %d ausente das lojas", item.Item)
		}
		def, ok := catalog.Items[item.Item]
		if !ok {
			t.Errorf("traje KR %d ausente do catalogo", item.Item)
			continue
		}
		if def.Name != item.Name || def.Mesh != 36 || def.Pos != 8192 {
			t.Errorf("metadados do traje %d divergiram: %+v", item.Item, def)
		}
		effects := make(map[string]int, len(def.StaticEffects))
		for _, effect := range def.StaticEffects {
			effects[effect.Name] = effect.Value
		}
		if effects["EF_CLASS"] != item.ClassMask || effects["EF_AC"] != 80 ||
			effects["EF_SAVEMANA"] != 10 || effects["111"] != 1 ||
			effects["EF_WDAY"] != 30 {
			t.Errorf("efeitos do traje %d divergiram: %+v", item.Item, effects)
		}
	}
	if available != 135 {
		t.Fatalf("manifesto possui %d trajes completos; esperado 135", available)
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

func TestTodasAsArmasDEDoCatalogoTemLojaDeTeste(t *testing.T) {
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
			t.Fatalf("%s possui %d itens; o client 7.48 exibe no maximo 27", name, len(stock))
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
				t.Errorf("arma D %d (%s) ausente das lojas ShopWeaponsD*", index, def.Name)
			}
		case 5:
			if _, ok := eStock[index]; !ok {
				t.Errorf("arma E %d (%s) ausente das lojas ShopWeaponsE*", index, def.Name)
			}
		}
	}
}

func TestEscudosDETemLojaDeTeste(t *testing.T) {
	shops, _ := loadTestShopFixture(t)
	requireShopItems(t, shops, "ShopShldDE", []uint16{1709, 1710, 1711, 1712})
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
		"ShopCostume", "ShopCostum02", "ShopCostum03", "ShopCostum04", "ShopCostum05",
		"ShopKRMt01", "ShopKRMt02", "ShopKRMt03",
		"ShopFiral", "ShopSetD", "ShopSetE", "ShopWeaponsD", "ShopWeaponsE",
		"ShopWpnD2", "ShopWpnE2", "ShopShldDE"}
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
