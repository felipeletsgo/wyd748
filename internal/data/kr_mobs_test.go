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
	{"Dragon", 199}, {"Helriohdon", 346}, {"BerialGuard", 347},
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

func TestKRMobPreviewGroupsMoveAwayFromArmia(t *testing.T) {
	gens, err := LoadNPCGener(filepath.Join("..", "..", "data", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	byLeader := make(map[string][]int)
	for i := range gens {
		if gens[i].Segments[0].Y == 2107 && gens[i].Segments[0].X >= 2201 {
			byLeader[gens[i].Leader] = append(byLeader[gens[i].Leader], i)
		}
	}
	previousX := uint16(2189) // Agmo_Krill reference point east of Armia.
	for _, want := range krMobFaces {
		indexes := byLeader[want.name]
		if len(indexes) != 1 {
			t.Errorf("%s possui %d grupos KR; esperado 1", want.name, len(indexes))
			continue
		}
		g := gens[indexes[0]]
		if g.MaxNumMob != 10 || g.MinGroup != 9 || g.MaxGroup != 9 {
			t.Errorf("grupo %s nao materializa exatamente 10 mobs: max=%d group=%d..%d",
				want.name, g.MaxNumMob, g.MinGroup, g.MaxGroup)
		}
		if g.Segments[0].Y != 2107 || g.Segments[0].X <= previousX {
			t.Errorf("grupo %s nao avanca para fora de Armia: (%d,%d), anterior X=%d",
				want.name, g.Segments[0].X, g.Segments[0].Y, previousX)
		}
		if gap := g.Segments[0].X - previousX; gap != 12 {
			t.Errorf("grupo %s possui espacamento %d, esperado 12", want.name, gap)
		}
		previousX = g.Segments[0].X
	}
}
