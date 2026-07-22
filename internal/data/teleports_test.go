package data

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadTeleports(t *testing.T) {
	path := filepath.Join(t.TempDir(), "teleports.ini")
	content := "# comentario\n2118,2102,1044,1724,700,Armia para Noatum\n"
	if err := os.WriteFile(path, []byte(content), 0o600); err != nil {
		t.Fatal(err)
	}
	got, err := LoadTeleports(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != 1 || got[0].SourceX != 2118 || got[0].DestY != 1724 ||
		got[0].Price != 700 || got[0].Name != "Armia para Noatum" {
		t.Fatalf("teleporte incorreto: %+v", got)
	}
}

func TestLoadTeleportsRejectsInvalidCoordinate(t *testing.T) {
	path := filepath.Join(t.TempDir(), "teleports.ini")
	if err := os.WriteFile(path, []byte("5000,1,2,3,0,invalido\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadTeleports(path); err == nil {
		t.Fatal("coordenada fora do mapa deveria falhar")
	}
}
