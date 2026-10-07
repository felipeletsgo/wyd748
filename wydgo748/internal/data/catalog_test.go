package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
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
		t.Fatalf("ItemEffect was not honored: %#v", effects)
	}
	if _, legacy := effects[2]; legacy {
		t.Fatal("the loader silently reintroduced the legacy EF_DAMAGE ID")
	}
}

func TestLoadConverted748Catalog(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	if len(catalog.Items) < 2800 || len(catalog.Skills) != 104 {
		t.Fatalf("incomplete catalog: items=%d skills=%d", len(catalog.Items), len(catalog.Skills))
	}
	helmet, ok := catalog.Items[1103]
	if !ok || helmet.Name != "Leather_Helmet(Arcane)" || helmet.Price != 1650 || helmet.Pos != 2 {
		t.Fatalf("invalid item 1103: %+v", helmet)
	}
	foundAC := false
	for _, effect := range helmet.StaticEffects {
		if effect.Name == "EF_AC" && effect.Value == 4 {
			foundAC = true
		}
	}
	if !foundAC {
		t.Fatalf("EF_AC of item 1103 not loaded: %+v", helmet.StaticEffects)
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
			t.Fatalf("fairy %d missing from the catalog", index)
		}
		effects := make(map[string]int, len(def.StaticEffects))
		for _, effect := range def.StaticEffects {
			effects[effect.Name] = effect.Value
		}
		day, dayOK := effects["EF_WDAY"]
		hour, hourOK := effects["EF_HOUR"]
		minute, minuteOK := effects["EF_MIN"]
		if !dayOK || !hourOK || !minuteOK || day != wantDays || hour != 0 || minute != 0 {
			t.Fatalf("invalid timer of fairy %d: effects=%v want=%dd 0h 0m", index, effects, wantDays)
		}
	}
	skill, ok := catalog.Skills[0]
	if !ok || skill.Name == "" || skill.SkillPoint != 24 || skill.ManaSpent != 15 {
		t.Fatalf("invalid skill 0: %+v", skill)
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
			t.Fatalf("skill %d differs from the WYD 7.48 SkillData: %+v", index, skill)
		}
	}
	if aura := catalog.Skills[5]; aura.TickType != 17 || aura.TickValue != 75 {
		t.Fatalf("skill 5 without the 7.59 regeneration: %+v", aura)
	}
}

func TestNative748SpecialSkillIndices(t *testing.T) {
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
			t.Fatalf("skill %d=%+v, want %q", index, skill, name)
		}
	}
	if skill := catalog.Skills[83]; skill.Passive != 1 {
		t.Fatalf("skill 83 lost Passive when the Unknown column was removed: %+v", skill)
	}
}

func TestCatalogRejectsDuplicateAndOutOfRangeIDs(t *testing.T) {
	itemLine := "1,Item,1.1,0.0.0.0.0,0,1,0,0,0\n"
	itemPath := filepath.Join(t.TempDir(), "itemlist.csv")
	if err := os.WriteFile(itemPath, []byte(itemLine+itemLine), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadItemList(itemPath); err == nil {
		t.Fatal("an itemlist with a duplicate ID was accepted")
	}
	if err := os.WriteFile(itemPath, []byte(strings.Replace(itemLine, "1,Item", "6500,Item", 1)), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadItemList(itemPath); err == nil {
		t.Fatal("an item outside ItemListSize was accepted")
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
		t.Fatal("SkillData with a duplicate ID was accepted")
	}
	parts := strings.Split(strings.TrimSpace(line), ",")
	parts[0] = "104"
	if err := os.WriteFile(skillPath, []byte(strings.Join(parts, ",")+"\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadSkills(skillPath); err == nil {
		t.Fatal("SkillData with an ID outside the 7.48 table was accepted")
	}
	parts = strings.Split(strings.TrimSpace(line), ",")
	parts[13] = "256.0.0.0.0.0.0.0"
	if err := os.WriteFile(skillPath, []byte(strings.Join(parts, ",")+"\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadSkills(skillPath); err == nil {
		t.Fatal("SkillData with an invalid animation byte was accepted")
	}
}

func TestNumericStaticEffectsTakeTheHeaderName(t *testing.T) {
	items := map[uint16]model.ItemDef{
		1: {Index: 1, StaticEffects: []model.StaticEffect{{Name: "111", Value: 1}, {Name: "200", Value: 7}}},
	}
	effects := map[byte]string{111: "EF_NOTRADE"}
	if err := validateStaticEffects(items, effects); err != nil {
		t.Fatal(err)
	}
	got := items[1].StaticEffects
	if got[0].Name != "EF_NOTRADE" || got[0].Value != 1 {
		t.Fatalf("named numeric effect = %+v; want EF_NOTRADE/1", got[0])
	}
	if got[1].Name != "200" || got[1].Value != 7 {
		t.Fatalf("unnamed numeric effect = %+v; want 200/7 preserved", got[1])
	}
}
