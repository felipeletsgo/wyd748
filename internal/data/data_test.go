package data

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadNPCsRejectsFlatObsoleteScore(t *testing.T) {
	path := filepath.Join(t.TempDir(), "old.json")
	if err := os.WriteFile(path, []byte(`{
		"name":"FormatoAntigo",
		"tipo":"monstro",
		"level":10,
		"maxHP":100
	}`), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadNPCs(path); err == nil {
		t.Fatal("NPC com atributos planos obsoletos foi aceito")
	}
}

func TestLoadNPCsRejectsTrailingJSON(t *testing.T) {
	path := filepath.Join(t.TempDir(), "double.json")
	body := `{"name":"A","tipo":"npc","extendedScore":{"version":2}}` +
		`{"name":"B","tipo":"npc","extendedScore":{"version":2}}`
	if err := os.WriteFile(path, []byte(body), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadNPCs(path); err == nil {
		t.Fatal("arquivo com dois objetos NPC foi aceito")
	}
}
