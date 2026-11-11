package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestLoadQuestsAllowsMissingFileAndLoadsStrictValidJSON(t *testing.T) {
	missing, err := LoadQuests(filepath.Join(t.TempDir(), "missing.json"))
	if err != nil {
		t.Fatalf("LoadQuests(missing) error = %v", err)
	}
	if missing.Version != model.QuestFileVersion || len(missing.Quests) != 0 {
		t.Fatalf("LoadQuests(missing) = %+v, want empty current-version file", missing)
	}

	path := filepath.Join(t.TempDir(), "quests.json")
	data := `{"version":1,"quests":[{"id":7,"npc":"Guide","messages":{"offer":"Start?","success":"Done."}}]}` + "\n \t"
	if err := os.WriteFile(path, []byte(data), 0o600); err != nil {
		t.Fatal(err)
	}

	loaded, err := LoadQuests(path)
	if err != nil {
		t.Fatalf("LoadQuests(valid) error = %v", err)
	}
	if loaded.Version != model.QuestFileVersion || len(loaded.Quests) != 1 ||
		loaded.Quests[0].ID != 7 || loaded.Quests[0].NPC != "Guide" {
		t.Fatalf("LoadQuests(valid) = %+v, want quest 7 from Guide", loaded)
	}
}

func TestLoadQuestsRejectsInvalidDocuments(t *testing.T) {
	tests := []struct {
		name string
		json string
		want string
	}{
		{name: "malformed JSON", json: `{"version":`, want: "parse"},
		{name: "unknown field", json: `{"version":1,"quests":[],"unexpected":true}`, want: "unknown field"},
		{name: "trailing JSON value", json: `{"version":1,"quests":[]} {}`, want: "conteudo JSON adicional"},
		{name: "unsupported version", json: `{"version":2,"quests":[]}`, want: "versao 2"},
		{name: "invalid quest", json: `{"version":1,"quests":[{"id":1,"npc":"Guide","messages":{"success":"Done."}}]}`, want: "mensagem de oferta"},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			path := filepath.Join(t.TempDir(), "quests.json")
			if err := os.WriteFile(path, []byte(tt.json), 0o600); err != nil {
				t.Fatal(err)
			}
			_, err := LoadQuests(path)
			if err == nil || !strings.Contains(err.Error(), tt.want) {
				t.Fatalf("LoadQuests() error = %v, want error containing %q", err, tt.want)
			}
		})
	}
}

func TestLoadQuestsReturnsNonMissingReadErrors(t *testing.T) {
	path := filepath.Join(t.TempDir(), "quests\x00.json")
	file, err := LoadQuests(path)
	if err == nil {
		t.Fatal("LoadQuests() error = nil, want non-missing read error")
	}
	if file.Version != model.QuestFileVersion || len(file.Quests) != 0 {
		t.Fatalf("LoadQuests() = %+v, want initialized empty file alongside read error", file)
	}
}
