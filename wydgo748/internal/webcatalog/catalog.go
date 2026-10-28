// Package webcatalog projects public, static game tables without starting a
// world, opening a database, or exporting accounts and runtime item instances.
package webcatalog

import (
	"bytes"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"unicode"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

const SchemaVersion = 2

// Paths are relative to the data root (see DOCS/server/data-layout.md).
var sourceNames = []string{"catalog/itemlist.csv", "catalog/Itemname.csv", "catalog/ItemEffect.h",
	"catalog/SkillData.csv", "content/quests.json", "content/quest_zones.json"}

type Source struct {
	File   string `json:"file"`
	SHA256 string `json:"sha256"`
}

type Effect struct {
	Code  string `json:"code"`
	Value int    `json:"value"`
}

type Item struct {
	ID          uint16   `json:"id"`
	Name        string   `json:"name"`
	BaseValue   uint32   `json:"baseValue"`
	Effects     []Effect `json:"effects"`
	BookSkillID *int     `json:"bookSkillId,omitempty"`
}

type Skill struct {
	ID           int    `json:"id"`
	Name         string `json:"name"`
	BookItemID   uint16 `json:"bookItemId"`
	Class        *int   `json:"class"`
	SkillPoints  int    `json:"skillPoints"`
	BaseMana     int    `json:"baseMana"`
	BaseCooldown int    `json:"baseCooldown"`
}

type Catalog struct {
	SchemaVersion int      `json:"schemaVersion"`
	ContentHash   string   `json:"contentHash"`
	Sources       []Source `json:"sources"`
	Items         []Item   `json:"items"`
	Skills        []Skill  `json:"skills"`
	Quests        []Quest  `json:"quests"`
}

// Export reuses the boot loaders and checks that their inputs remain unchanged
// while reading. The hash covers the canonical projection with an empty hash.
func Export(dir string) ([]byte, error) {
	before, err := sources(dir)
	if err != nil {
		return nil, err
	}
	loaded, err := data.LoadCatalog(filepath.Join(dir, sourceNames[0]), filepath.Join(dir, sourceNames[1]), filepath.Join(dir, sourceNames[2]), filepath.Join(dir, sourceNames[3]))
	if err != nil {
		return nil, err
	}
	quests, err := data.LoadQuests(filepath.Join(dir, sourceNames[4]))
	if err != nil {
		return nil, err
	}
	zones, err := data.LoadQuestZones(filepath.Join(dir, sourceNames[5]))
	if err != nil {
		return nil, err
	}
	after, err := sources(dir)
	if err != nil {
		return nil, err
	}
	if !equalSources(before, after) {
		return nil, fmt.Errorf("catalog sources changed during export")
	}
	catalog, err := Project(loaded)
	if err != nil {
		return nil, err
	}
	catalog.Quests, err = ProjectQuests(quests, zones, catalog.Items)
	if err != nil {
		return nil, err
	}
	catalog.Sources = before
	canonical, err := json.Marshal(catalog)
	if err != nil {
		return nil, err
	}
	catalog.ContentHash = hash(canonical)
	encoded, err := json.MarshalIndent(catalog, "", "  ")
	return append(encoded, '\n'), err
}

func sources(dir string) ([]Source, error) {
	result := make([]Source, 0, len(sourceNames))
	for _, name := range sourceNames {
		contents, err := os.ReadFile(filepath.Join(dir, name))
		if err != nil {
			return nil, fmt.Errorf("read catalog source %s: %w", name, err)
		}
		result = append(result, Source{File: name, SHA256: hash(contents)})
	}
	return result, nil
}

func equalSources(a, b []Source) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}

func hash(contents []byte) string {
	sum := sha256.Sum256(contents)
	return hex.EncodeToString(sum[:])
}

func displayName(name string) (string, error) {
	name = strings.TrimSpace(strings.ReplaceAll(name, "_", " "))
	if name == "" {
		return "", fmt.Errorf("missing public name")
	}
	for _, char := range name {
		if char > unicode.MaxASCII || unicode.IsControl(char) {
			return "", fmt.Errorf("public name is not printable English ASCII: %q", name)
		}
	}
	return name, nil
}

// Project is an explicit public-field allowlist, not a serialization of the
// internal model. Skill names use the English book table only after validating
// the ID/texture/mesh relationship used by the current client skill icons.
func Project(loaded model.Catalog) (Catalog, error) {
	result := Catalog{SchemaVersion: SchemaVersion, Sources: []Source{}, Items: []Item{}, Skills: []Skill{}, Quests: []Quest{}}
	for id, def := range loaded.Items {
		if id != def.Index {
			return Catalog{}, fmt.Errorf("item identity mismatch: %d", id)
		}
		if id == 0 || def.Name == "NULL" || strings.TrimSpace(def.Name) == "" {
			continue
		}
		name, err := displayName(def.Name)
		if err != nil {
			return Catalog{}, fmt.Errorf("item %d: %w", id, err)
		}
		item := Item{ID: id, Name: name, BaseValue: def.Price, Effects: []Effect{}}
		for _, effect := range def.StaticEffects {
			item.Effects = append(item.Effects, Effect{Code: effect.Name, Value: effect.Value})
		}
		if def.Mesh == -1 {
			if _, exists := loaded.Skills[def.Texture]; exists && int(id) == 5000+def.Texture {
				skillID := def.Texture
				item.BookSkillID = &skillID
			}
		}
		result.Items = append(result.Items, item)
	}
	for id, def := range loaded.Skills {
		if id != def.Index {
			return Catalog{}, fmt.Errorf("skill identity mismatch: %d", id)
		}
		if def.Name == "NULL" {
			if def != (model.SkillDef{Index: id, Name: "NULL"}) {
				return Catalog{}, fmt.Errorf("skill %d has a nonempty NULL definition", id)
			}
			continue
		}
		if id < 0 || id > 102 {
			return Catalog{}, fmt.Errorf("skill %d has no verified public name binding", id)
		}
		bookID := uint16(5000 + id)
		book, exists := loaded.Items[bookID]
		if !exists || book.Index != bookID || book.Mesh != -1 || book.Texture != id || book.Name == "NULL" {
			return Catalog{}, fmt.Errorf("skill %d has an invalid English book binding", id)
		}
		name, err := displayName(book.Name)
		if err != nil {
			return Catalog{}, fmt.Errorf("skill %d: %w", id, err)
		}
		if def.SkillPoint < 0 || def.ManaSpent < 0 || def.Delay < 0 {
			return Catalog{}, fmt.Errorf("skill %d has invalid base costs", id)
		}
		skill := Skill{ID: id, Name: name, BookItemID: bookID, SkillPoints: def.SkillPoint, BaseMana: def.ManaSpent, BaseCooldown: def.Delay}
		if id < 96 {
			class := id / 24
			skill.Class = &class
		}
		result.Skills = append(result.Skills, skill)
	}
	sort.Slice(result.Items, func(i, j int) bool { return result.Items[i].ID < result.Items[j].ID })
	sort.Slice(result.Skills, func(i, j int) bool { return result.Skills[i].ID < result.Skills[j].ID })
	return result, nil
}

// Write leaves the previous artifact untouched on export failure and avoids
// changing its timestamp when the projection is identical.
func Write(dir, output string) error {
	encoded, err := Export(dir)
	if err != nil {
		return err
	}
	if previous, err := os.ReadFile(output); err == nil && bytes.Equal(previous, encoded) {
		return nil
	}
	parent := filepath.Dir(output)
	if err := os.MkdirAll(parent, 0755); err != nil {
		return err
	}
	file, err := os.CreateTemp(parent, ".web-catalog-*")
	if err != nil {
		return err
	}
	temporary := file.Name()
	defer os.Remove(temporary)
	if _, err = file.Write(encoded); err != nil {
		file.Close()
		return err
	}
	if err = file.Close(); err != nil {
		return err
	}
	return os.Rename(temporary, output)
}
