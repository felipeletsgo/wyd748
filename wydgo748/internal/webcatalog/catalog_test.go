package webcatalog

import (
	"bytes"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestAuthoritativeExport(t *testing.T) {
	dir := filepath.Join("..", "..", "data")
	first, err := Export(dir)
	if err != nil {
		t.Fatal(err)
	}
	second, err := Export(dir)
	if err != nil || !bytes.Equal(first, second) {
		t.Fatalf("nondeterministic export: %v", err)
	}
	var catalog Catalog
	if err := json.Unmarshal(first, &catalog); err != nil {
		t.Fatal(err)
	}
	if catalog.SchemaVersion != SchemaVersion || len(catalog.Items) < 3000 || len(catalog.Skills) != 103 || len(catalog.Sources) != 6 || len(catalog.Quests) != 18 {
		t.Fatalf("unexpected catalog sizes: items=%d skills=%d", len(catalog.Items), len(catalog.Skills))
	}
	claimed := catalog.ContentHash
	catalog.ContentHash = ""
	canonical, _ := json.Marshal(catalog)
	if hash(canonical) != claimed {
		t.Fatal("content hash does not cover canonical projection")
	}
	for i, item := range catalog.Items {
		if i > 0 && item.ID <= catalog.Items[i-1].ID {
			t.Fatal("items are not uniquely sorted")
		}
		if _, err := displayName(item.Name); err != nil {
			t.Fatal(err)
		}
		if item.BookSkillID != nil && (*item.BookSkillID < 0 || *item.BookSkillID >= len(catalog.Skills)) {
			t.Fatal("orphan book relationship")
		}
	}
	for i, skill := range catalog.Skills {
		if skill.ID != i || skill.BookItemID != uint16(5000+i) || strings.Contains(skill.Name, "_") {
			t.Fatalf("invalid skill binding: %+v", skill)
		}
		if i < 96 && (skill.Class == nil || *skill.Class != i/24) {
			t.Fatal("invalid class partition")
		}
		if i >= 96 && skill.Class != nil {
			t.Fatal("shared skill assigned a class")
		}
	}
	if catalog.Skills[0].Name != "Heaven's Dust" {
		t.Fatalf("English book name was not used: %q", catalog.Skills[0].Name)
	}
	for _, private := range []string{"DynamicEffectNames", "account", "password", "token", "session", "ActAlt", "InstanceValue", filepath.ToSlash(dir)} {
		if bytes.Contains(first, []byte(private)) {
			t.Fatalf("unexpected internal field: %s", private)
		}
	}
}

func fixture() model.Catalog {
	return model.Catalog{Items: map[uint16]model.ItemDef{5000: {Index: 5000, Mesh: -1, Texture: 0, Name: "Heaven's_Dust", Price: 123}}, Skills: map[int]model.SkillDef{0: {Index: 0, Name: "Historical internal name", ManaSpent: 5, Delay: 3}}}
}

func TestProjectRejectsInvalidBindings(t *testing.T) {
	for _, scenario := range []struct {
		name   string
		mutate func(*model.Catalog)
	}{
		{"missing book", func(c *model.Catalog) { delete(c.Items, 5000) }},
		{"wrong mesh", func(c *model.Catalog) { d := c.Items[5000]; d.Mesh = 0; c.Items[5000] = d }},
		{"wrong texture", func(c *model.Catalog) { d := c.Items[5000]; d.Texture = 1; c.Items[5000] = d }},
		{"wrong item identity", func(c *model.Catalog) { d := c.Items[5000]; d.Index = 5001; c.Items[5000] = d }},
		{"wrong skill identity", func(c *model.Catalog) { d := c.Skills[0]; d.Index = 1; c.Skills[0] = d }},
		{"non-ASCII public name", func(c *model.Catalog) { d := c.Items[5000]; d.Name = "Dust\u0080"; c.Items[5000] = d }},
		{"control character", func(c *model.Catalog) {
			d := c.Items[5000]
			d.Name = "Dust\n"
			c.Items[5000] = d
			d.Name = "D\nust"
			c.Items[5000] = d
		}},
		{"unknown skill", func(c *model.Catalog) { c.Skills[104] = model.SkillDef{Index: 104, Name: "Unbound"} }},
		{"nonempty sentinel", func(c *model.Catalog) { c.Skills[103] = model.SkillDef{Index: 103, Name: "NULL", Delay: 1} }},
		{"negative cost", func(c *model.Catalog) { d := c.Skills[0]; d.ManaSpent = -1; c.Skills[0] = d }},
	} {
		t.Run(scenario.name, func(t *testing.T) {
			c := fixture()
			scenario.mutate(&c)
			if _, err := Project(c); err == nil {
				t.Fatal("invalid public contract accepted")
			}
		})
	}
}

func TestWritePreservesArtifactAndHashesChangedInputs(t *testing.T) {
	dir := t.TempDir()
	for _, name := range sourceNames {
		contents, err := os.ReadFile(filepath.Join("..", "..", "data", name))
		if err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(dir, name), contents, 0600); err != nil {
			t.Fatal(err)
		}
	}
	output := filepath.Join(t.TempDir(), "catalog.json")
	if err := Write(dir, output); err != nil {
		t.Fatal(err)
	}
	before, _ := os.ReadFile(output)
	stat, _ := os.Stat(output)
	if err := Write(dir, output); err != nil {
		t.Fatal(err)
	}
	statAgain, _ := os.Stat(output)
	if !stat.ModTime().Equal(statAgain.ModTime()) {
		t.Fatal("identical export rewritten")
	}
	namePath := filepath.Join(dir, "Itemname.csv")
	names, _ := os.ReadFile(namePath)
	changed := bytes.Replace(names, []byte("Heaven's_Dust"), []byte("Heaven's_Dust_Test"), 1)
	if bytes.Equal(changed, names) {
		t.Fatal("fixture binding missing")
	}
	if err := os.WriteFile(namePath, changed, 0600); err != nil {
		t.Fatal(err)
	}
	if err := Write(dir, output); err != nil {
		t.Fatal(err)
	}
	after, _ := os.ReadFile(output)
	var old, new Catalog
	json.Unmarshal(before, &old)
	json.Unmarshal(after, &new)
	if old.ContentHash == new.ContentHash || old.Sources[1].SHA256 == new.Sources[1].SHA256 || !bytes.Contains(after, []byte("Heaven's Dust Test")) {
		t.Fatal("changed input not reflected")
	}
	if err := os.WriteFile(filepath.Join(dir, "SkillData.csv"), []byte("not,a,valid,skill\n"), 0600); err != nil {
		t.Fatal(err)
	}
	if err := Write(dir, output); err == nil {
		t.Fatal("invalid source accepted")
	}
	retained, _ := os.ReadFile(output)
	if !bytes.Equal(retained, after) {
		t.Fatal("failed export replaced previous artifact")
	}
	temps, _ := filepath.Glob(filepath.Join(filepath.Dir(output), ".web-catalog-*"))
	if len(temps) != 0 {
		t.Fatal("temporary artifact leaked")
	}
}
