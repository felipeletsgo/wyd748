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
	fairyDays := map[uint16]int{
		3900: 3, 3901: 3, 3902: 3,
		3903: 5, 3904: 5, 3905: 5,
		3906: 7, 3907: 7, 3908: 7,
		3911: 7, 3912: 15, 3913: 30,
		3914: 7, 3915: 7,
	}
	for index, wantDays := range fairyDays {
		def, ok := catalog.Items[index]
		if !ok {
			t.Fatalf("fada %d ausente do catalogo", index)
		}
		effects := make(map[string]int, len(def.StaticEffects))
		for _, effect := range def.StaticEffects {
			effects[effect.Name] = effect.Value
		}
		day, dayOK := effects["EF_WDAY"]
		hour, hourOK := effects["EF_HOUR"]
		minute, minuteOK := effects["EF_MIN"]
		if !dayOK || !hourOK || !minuteOK || day != wantDays || hour != 0 || minute != 0 {
			t.Fatalf("timer da fada %d invalido: effects=%v want=%dd 0h 0m", index, effects, wantDays)
		}
	}
	skill, ok := catalog.Skills[0]
	if !ok || skill.Name == "" || skill.SkillPoint != 24 || skill.ManaSpent != 15 {
		t.Fatalf("skill 0 invalida: %+v", skill)
	}
}

func TestSkillsComeFromAuthoritativeSkillData(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	want := map[int][3]int{
		3:  {24, 200, 600 / 4},
		5:  {0, 0, 600 / 4},
		11: {13, 7, 12 / 4},
		13: {14, 10, 600 / 4},
		15: {50, 100, 600 / 4},
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
		83: "Extração",
		84: "Alquimia",
		85: "Escudo_Dourado",
	}
	for index, name := range want {
		if skill, ok := catalog.Skills[index]; !ok || !strings.EqualFold(skill.Name, name) {
			t.Fatalf("skill %d=%+v, quer %q", index, skill, name)
		}
	}
	if skill := catalog.Skills[83]; skill.Passive != 1 {
		t.Fatalf("skill 83 perdeu o Passive ao remover a coluna Unknown: %+v", skill)
	}
}

func TestCatalogRejectsDuplicateAndOutOfRangeIDs(t *testing.T) {
	itemLine := "1,Item,1.1,0.0.0.0.0,0,1,0,0,0\n"
	itemPath := filepath.Join(t.TempDir(), "itemlist.csv")
	if err := os.WriteFile(itemPath, []byte(itemLine+itemLine), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadItemList(itemPath); err == nil {
		t.Fatal("itemlist com ID duplicado foi aceito")
	}
	if err := os.WriteFile(itemPath, []byte(strings.Replace(itemLine, "1,Item", "6500,Item", 1)), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadItemList(itemPath); err == nil {
		t.Fatal("item fora de ItemListSize foi aceito")
	}

	firstSkill, err := os.ReadFile("../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	line := strings.SplitN(string(firstSkill), "\n", 2)[0] + "\n"
	skillPath := filepath.Join(t.TempDir(), "SkillData.csv")
	if err := os.WriteFile(skillPath, []byte(line+line), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadSkills(skillPath); err == nil {
		t.Fatal("SkillData com ID duplicado foi aceito")
	}
	parts := strings.Split(strings.TrimSpace(line), ",")
	parts[0] = "104"
	if err := os.WriteFile(skillPath, []byte(strings.Join(parts, ",")+"\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadSkills(skillPath); err == nil {
		t.Fatal("SkillData com ID fora da tabela 7.48 foi aceito")
	}
	parts = strings.Split(strings.TrimSpace(line), ",")
	parts[13] = "256.0.0.0.0.0.0.0"
	if err := os.WriteFile(skillPath, []byte(strings.Join(parts, ",")+"\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadSkills(skillPath); err == nil {
		t.Fatal("SkillData com byte de animacao invalido foi aceito")
	}
}
