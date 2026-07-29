package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestItemEffectFileIsAuthoritative(t *testing.T) {
	path := filepath.Join(t.TempDir(), "ItemEffect.h")
	if err := os.WriteFile(path, []byte("#define EF_DAMAGE 200\n#define EF_SANC 43\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	effects, err := loadItemEffects(path)
	if err != nil {
		t.Fatal(err)
	}
	if effects[200] != "EF_DAMAGE" {
		t.Fatalf("ItemEffect nao foi respeitado: %#v", effects)
	}
	if _, legacy := effects[2]; legacy {
		t.Fatal("loader reintroduziu silenciosamente o ID legado de EF_DAMAGE")
	}
}

func TestLoadConverted748Catalog(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	if len(catalog.Items) < 2800 || len(catalog.Skills) != 104 {
		t.Fatalf("catalogo incompleto: itens=%d skills=%d", len(catalog.Items), len(catalog.Skills))
	}
	helmet, ok := catalog.Items[1103]
	if !ok || helmet.Name != "Leather_Helmet(Arcane)" || helmet.Price != 1650 || helmet.Pos != 2 {
		t.Fatalf("item 1103 invalido: %+v", helmet)
	}
	foundAC := false
	for _, effect := range helmet.StaticEffects {
		if effect.Name == "EF_AC" && effect.Value == 4 {
			foundAC = true
		}
	}
	if !foundAC {
		t.Fatalf("EF_AC do item 1103 nao carregado: %+v", helmet.StaticEffects)
	}
	skill, ok := catalog.Skills[0]
	if !ok || !strings.EqualFold(skill.Name, "Giro_Da_Furia") ||
		skill.SkillPoint != 24 || skill.ManaSpent != 15 {
		t.Fatalf("skill 0 invalida: %+v", skill)
	}
}

func TestTKBuffsComeFromW2PPSkillData(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	want := map[int][3]int{
		3:  {24, 0, 45 / 4},
		5:  {0, 0, 99 / 4},
		11: {13, 7, 99 / 4},
		13: {14, 10, 99 / 4},
		15: {31, 150, 75 / 4},
	}
	for index, expected := range want {
		skill := catalog.Skills[index]
		if skill.AffectType != expected[0] || skill.AffectValue != expected[1] || skill.AffectTime != expected[2] {
			t.Fatalf("skill %d divergiu do SkillData W2PP: %+v", index, skill)
		}
	}
	if aura := catalog.Skills[5]; aura.TickType != 17 || aura.TickValue != 75 {
		t.Fatalf("Aura da Vida sem regen 7.59: %+v", aura)
	}
}

func TestW2PPSpecialSkillIndices(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	want := map[int]string{
		79: "Tempestade_de_Raios",
		83: "Alquimia",
		84: "Extracao",
		85: "Escudo_Dourado",
	}
	for index, name := range want {
		if skill, ok := catalog.Skills[index]; !ok || !strings.EqualFold(skill.Name, name) {
			t.Fatalf("skill %d=%+v, quer %q", index, skill, name)
		}
	}
}
