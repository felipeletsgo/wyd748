package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"reflect"
	"testing"
	"wydgo/internal/model"
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
	raw, err := os.ReadFile("testdata/kr-native.json")
	if err != nil {
		t.Fatal(err)
	}
	var approved []model.NPCDef
	if err := json.Unmarshal(raw, &approved); err != nil {
		t.Fatal(err)
	}
	native := make(map[string]model.NPCDef)
	for _, def := range approved {
		native[def.Name] = def
	}

	catalog, err := LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
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
		_, imported := native[want.name]
		if !imported && got.Equip.Rosto.Index != want.face {
			t.Errorf("%s usa face %d, esperado %d", want.name, got.Equip.Rosto.Index, want.face)
		}
		if item, ok := catalog.Items[want.face]; !ok || item.Name != want.name {
			t.Errorf("face %d de %s ausente/divergente no catalogo: %+v", want.face, want.name, item)
		}

		// Approved native definitions replace provisional preview clones.
		if expected, ok := native[want.name]; ok {
			if !reflect.DeepEqual(got, expected) {
				t.Errorf("%s differs from its approved native definition", want.name)
			}
			continue
		}
		// Supplemental previews retain the isolated Agmo_Krill baseline.
		got.Name = base.Name
		got.Equip.Rosto.Index = base.Equip.Rosto.Index
		if !reflect.DeepEqual(got, base) {
			t.Errorf("%s divergiu do gameplay template Agmo_Krill", want.name)
		}
	}
}

func TestKRMobPreviewGroupsMaterializeOneMob(t *testing.T) {
	gens, err := LoadChannelNPCGener(filepath.Join("..", "..", "data", "spawns", "NPCGener.txt"))
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
		expected := 1
		if want.name == "DeathWorm" {
			expected = 0 // Owner removed preview 5134 because its coordinates were invalid.
		}
		if matches != expected {
			t.Errorf("%s has %d KR preview groups; expected %d", want.name, matches, expected)
		}
	}
}
