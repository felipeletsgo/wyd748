package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestCharacterTemplateLoaderRejectsInvalidLayouts(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(
		filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"),
		filepath.Join(root, "catalog", "SkillData.csv"),
	)
	if err != nil {
		t.Fatal(err)
	}
	fixture := filepath.Join(root, "content", "character_templates.json")
	baseline, err := LoadCharacterTemplates(fixture, catalog.Items)
	if err != nil {
		t.Fatal(err)
	}

	for _, tc := range []struct {
		name   string
		mutate func(*model.CharacterTemplateFile)
		want   string
	}{
		{name: "zero spawn coordinate", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Spawn.X = 0 }, want: "coordenada inicial invalida"},
		{name: "invalid class", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[0].Class = 4 }, want: "classe invalida"},
		{name: "duplicate class", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[1].Class = cfg.Classes[0].Class }, want: "duplicada"},
		{name: "invalid score version", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[0].Score.Version = 0 }, want: "score v0"},
		{name: "zero base HP", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[0].Score.MaxHP = 0 }, want: "HP/MP base"},
		{name: "zero base MP", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[0].Score.MaxMP = 0 }, want: "HP/MP base"},
		{name: "inventory exceeds capacity", mutate: func(cfg *model.CharacterTemplateFile) {
			cfg.Classes[0].Inventory = make([]model.Item, model.PlayerCarrySlots+1)
		}, want: "excede"},
		{name: "equipment UID", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[0].Equip.Rosto.UID = "persisted" }, want: "possui UID"},
		{name: "inventory UID", mutate: func(cfg *model.CharacterTemplateFile) {
			cfg.Classes[0].Inventory = []model.Item{{Index: 1, UID: "persisted"}}
		}, want: "possui UID"},
		{name: "unknown initial item", mutate: func(cfg *model.CharacterTemplateFile) { cfg.Classes[0].Equip.Rosto.Index = 65535 }, want: "nao existe no catalogo"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			cfg := cloneCharacterTemplateFile(t, baseline)
			tc.mutate(&cfg)
			path := writeCharacterTemplateFile(t, cfg)
			if _, err := LoadCharacterTemplates(path, catalog.Items); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("LoadCharacterTemplates error = %v, want substring %q", err, tc.want)
			}
		})
	}
}

func TestCharacterTemplateLoaderRejectsMissingMalformedAndTrailingJSON(t *testing.T) {
	if _, err := LoadCharacterTemplates(filepath.Join(t.TempDir(), "missing.json"), nil); err == nil {
		t.Fatal("missing template file was accepted")
	}
	for _, tc := range []struct {
		name string
		body string
	}{
		{name: "malformed JSON", body: "{"},
		{name: "trailing JSON", body: `{"spawn":{"x":1,"y":1},"classes":[{},{},{},{}]} {}`},
	} {
		t.Run(tc.name, func(t *testing.T) {
			path := filepath.Join(t.TempDir(), "chars.json")
			if err := os.WriteFile(path, []byte(tc.body), 0o600); err != nil {
				t.Fatal(err)
			}
			if _, err := LoadCharacterTemplates(path, nil); err == nil {
				t.Fatal("invalid template JSON was accepted")
			}
		})
	}
}

func cloneCharacterTemplateFile(t *testing.T, input model.CharacterTemplateFile) model.CharacterTemplateFile {
	t.Helper()
	body, err := json.Marshal(input)
	if err != nil {
		t.Fatal(err)
	}
	var cloned model.CharacterTemplateFile
	if err := json.Unmarshal(body, &cloned); err != nil {
		t.Fatal(err)
	}
	return cloned
}

func writeCharacterTemplateFile(t *testing.T, cfg model.CharacterTemplateFile) string {
	t.Helper()
	body, err := json.Marshal(cfg)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "chars.json")
	if err := os.WriteFile(path, body, 0o600); err != nil {
		t.Fatal(err)
	}
	return path
}
