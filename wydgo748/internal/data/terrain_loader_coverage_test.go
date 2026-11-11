package data

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadTerrainReportsMissingAndWrongSizedFiles(t *testing.T) {
	if _, err := LoadTerrain(filepath.Join(t.TempDir(), "missing-height"), "unused"); err == nil {
		t.Fatal("LoadTerrain accepted a missing height map")
	}
	shortHeight := filepath.Join(t.TempDir(), "short-height.dat")
	if err := os.WriteFile(shortHeight, []byte{1, 2, 3}, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadTerrain(shortHeight, "unused"); err == nil {
		t.Fatal("LoadTerrain accepted a short height map")
	}

	height := filepath.Join("..", "..", "data", "world", "maps", "HeightMap.dat")
	if _, err := LoadTerrain(height, filepath.Join(t.TempDir(), "missing-attribute")); err == nil {
		t.Fatal("LoadTerrain accepted a missing attribute map")
	}
	shortAttribute := filepath.Join(t.TempDir(), "short-attribute.dat")
	if err := os.WriteFile(shortAttribute, []byte{1, 2, 3}, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadTerrain(height, shortAttribute); err == nil {
		t.Fatal("LoadTerrain accepted a short attribute map")
	}
}
