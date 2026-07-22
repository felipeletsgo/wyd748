package data

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadCharacterTemplatesProjectFile(t *testing.T) {
	path := filepath.Join("..", "..", "data", "character_templates.json")
	catalog, err := LoadCatalog(
		filepath.Join("..", "..", "data", "itemlist.csv"),
		filepath.Join("..", "..", "data", "Itemname.csv"),
		filepath.Join("..", "..", "data", "SkillData.csv"),
	)
	if err != nil {
		t.Fatal(err)
	}
	cfg, err := LoadCharacterTemplates(path, catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	if len(cfg.Classes) != 4 || cfg.Spawn.X != 2100 || cfg.Spawn.Y != 2100 {
		t.Fatalf("layout carregado incorretamente: %+v", cfg)
	}
	for class, tpl := range cfg.Classes {
		if int(tpl.Class) != class || tpl.ExtendedScore.MaxHP == 0 || tpl.Equip.Rosto.Index == 0 {
			t.Fatalf("classe %d invalida: %+v", class, tpl)
		}
	}
}

func TestLoadCharacterTemplatesRejectsMissingClasses(t *testing.T) {
	path := filepath.Join(t.TempDir(), "chars.json")
	if err := os.WriteFile(path, []byte(`{"spawn":{"x":1,"y":1},"classes":[]}`), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadCharacterTemplates(path, nil); err == nil {
		t.Fatal("layout sem as quatro classes foi aceito")
	}
}

func TestLoadCharacterTemplatesRejectsObsoleteFields(t *testing.T) {
	path := filepath.Join(t.TempDir(), "chars.json")
	body := `{
		"spawn":{"x":2100,"y":2100},
		"classes":[],
		"score":{"level":1}
	}`
	if err := os.WriteFile(path, []byte(body), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadCharacterTemplates(path, nil); err == nil {
		t.Fatal("template com campo score obsoleto foi aceito")
	}
}
