package data

import (
	"path/filepath"
	"reflect"
	"testing"
)

var krMobFaces = []struct {
	name string
	face uint16
}{
	{"Scorpion", 70}, {"Murmy", 71}, {"DeathWorm", 72},
	{"Anubis", 73}, {"AnubisElite", 74}, {"AnubisGuard", 75},
	{"BerialGuard", 347},
	{"tteok", 348}, {"LegendBerial", 399}, {"BahamutQ_R", 4082},
	{"BahamutQ_B", 4083}, {"LavaGolem", 4084}, {"LavaGrub", 4085},
	{"Bahamut", 4086}, {"CB_Trant", 4087}, {"Sand_Lich", 4088},
	{"Ice_Lich", 4089},
}

func TestKRMobsCloneAgmoKrillAndHaveCatalogFaces(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]int, len(npcs))
	for i := range npcs {
		byName[npcs[i].Name] = i
	}
	baseIndex, ok := byName["Agmo_Krill"]
	if !ok {
		t.Fatal("template Agmo_Krill ausente")
	}
	base := npcs[baseIndex]

	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	for _, want := range krMobFaces {
		index, ok := byName[want.name]
		if !ok {
			t.Errorf("template KR ausente: %s", want.name)
			continue
		}
		got := npcs[index]
		if got.Equip.Rosto.Index != want.face {
			t.Errorf("%s usa face %d, esperado %d", want.name, got.Equip.Rosto.Index, want.face)
		}
		if item, ok := catalog.Items[want.face]; !ok || item.Name != want.name {
			t.Errorf("face %d de %s ausente/divergente no catalogo: %+v", want.face, want.name, item)
		}

		// Nome e indice da face sao as duas unicas diferencas permitidas em
		// relacao ao template solicitado pelo usuario.
		got.Name = base.Name
		got.Equip.Rosto.Index = base.Equip.Rosto.Index
		if !reflect.DeepEqual(got, base) {
			t.Errorf("%s divergiu do gameplay template Agmo_Krill", want.name)
		}
	}
}

func TestKRMobPreviewGroupsMaterializeOneMob(t *testing.T) {
	gens, err := LoadNPCGener(filepath.Join("..", "..", "data", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}

	for _, want := range krMobFaces {
		matches := 0
		for i := range gens {
			g := gens[i]
			if g.Leader != want.name || g.Follower != want.name {
				continue
			}
			if g.MaxNumMob == 1 && g.MinGroup == 1 && g.MaxGroup == 1 {
				matches++
			}
		}
		if matches != 1 {
			t.Errorf("%s possui %d grupos KR de um mob; esperado 1", want.name, matches)
		}
	}
}
