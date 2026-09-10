package main

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadOptionalTerrainMissingIsNonFatal(t *testing.T) {
	terrain, err := loadOptionalTerrain(filepath.Join(t.TempDir(), "missing.trn"))
	if err != nil {
		t.Fatalf("loadOptionalTerrain missing: %v", err)
	}
	if terrain != nil {
		t.Fatal("missing diagnostic terrain returned a value")
	}
}

func TestLoadOptionalTerrainRejectsMalformedPresentFile(t *testing.T) {
	path := filepath.Join(t.TempDir(), "broken.trn")
	if err := os.WriteFile(path, []byte("broken"), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := loadOptionalTerrain(path); err == nil {
		t.Fatal("malformed diagnostic terrain was accepted")
	}
}
