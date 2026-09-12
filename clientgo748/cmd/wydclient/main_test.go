package main

import (
	"encoding/binary"
	"math"
	"os"
	"path/filepath"
	"testing"
)

func TestLoadLoginBackdropBuildsTerrainAndCameraPair(t *testing.T) {
	dir := t.TempDir()
	terrainPath := filepath.Join(dir, "Field0813.trn")
	cameraPath := filepath.Join(dir, "DemoCamAction4.bin")

	terrainData := make([]byte, 1+5+2+4*12)
	terrainData[0] = 5
	copy(terrainData[1:6], "field")
	terrainData[6], terrainData[7] = 8, 13
	terrainData[8], terrainData[20], terrainData[32], terrainData[44] = 0, 1, 2, 3
	if err := os.WriteFile(terrainPath, terrainData, 0600); err != nil {
		t.Fatal(err)
	}

	cameraData := make([]byte, 4+28)
	binary.LittleEndian.PutUint32(cameraData[:4], 1)
	binary.LittleEndian.PutUint32(cameraData[4:8], 0)
	binary.LittleEndian.PutUint16(cameraData[8:10], 0)
	binary.LittleEndian.PutUint32(cameraData[12:16], math.Float32bits(1102))
	binary.LittleEndian.PutUint32(cameraData[16:20], math.Float32bits(1))
	binary.LittleEndian.PutUint32(cameraData[20:24], math.Float32bits(1690.5))
	binary.LittleEndian.PutUint32(cameraData[24:28], math.Float32bits(float32(math.Pi)))
	binary.LittleEndian.PutUint32(cameraData[28:32], math.Float32bits(0))
	if err := os.WriteFile(cameraPath, cameraData, 0600); err != nil {
		t.Fatal(err)
	}

	backdrop, err := loadLoginBackdrop(terrainPath, cameraPath)
	if err != nil {
		t.Fatal(err)
	}
	if backdrop == nil {
		t.Fatal("loadLoginBackdrop returned nil")
	}
}

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
