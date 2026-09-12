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
	envDir := filepath.Join(dir, "Env")
	if err := os.MkdirAll(envDir, 0700); err != nil {
		t.Fatal(err)
	}
	terrainPath := filepath.Join(envDir, "Field0813.trn")
	cameraPath := filepath.Join(dir, "DemoCamAction4.bin")

	terrainData := make([]byte, 1+5+2+4*12)
	terrainData[0] = 5
	copy(terrainData[1:6], "field")
	terrainData[6], terrainData[7] = 8, 13
	terrainData[8], terrainData[20], terrainData[32], terrainData[44] = 0, 1, 2, 3
	terrainData[9] = 28
	if err := os.WriteFile(terrainPath, terrainData, 0600); err != nil {
		t.Fatal(err)
	}
	textureList := make([]byte, 512*0x108)
	copy(textureList[38*0x108:], []byte(`Env\Tile01010.wys`))
	copy(textureList[256*0x108:], []byte(`Env\Tile01010.wys`))
	if err := os.WriteFile(filepath.Join(envDir, "EnvTextureList3.bin"), textureList, 0600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(envDir, "Tile01010.wys"), testLoginWYS(), 0600); err != nil {
		t.Fatal(err)
	}
	// The login bootstrap now always consumes the native Field0813 object
	// stream. Keep this fixture focused on terrain/camera by using one object
	// type handled by a specialized native branch rather than the ordinary MSA
	// path exercised by the static-object tests.
	fieldObject := make([]byte, 28)
	binary.LittleEndian.PutUint32(fieldObject[0:4], 2)
	if err := os.WriteFile(filepath.Join(envDir, "Field0813.dat"), fieldObject, 0600); err != nil {
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

func TestResolveNativeAssetPathNormalizesWindowsSeparatorsAndRejectsEscape(t *testing.T) {
	root := t.TempDir()
	want := filepath.Join(root, "Env", "Tile01010.wys")
	got, err := resolveNativeAssetPath(root, `Env\Tile01010.wys`)
	if err != nil {
		t.Fatal(err)
	}
	if got != want {
		t.Fatalf("resolved path=%q want %q", got, want)
	}
	if _, err := resolveNativeAssetPath(root, `..\outside.wys`); err == nil {
		t.Fatal("path traversal was accepted")
	}
}

func TestLoginStaticObjectNullTypesDoNotUseOrdinaryMSAPath(t *testing.T) {
	for _, objectType := range []uint32{657, 658} {
		if isOrdinaryLoginStaticObjectType(objectType) {
			t.Fatalf("object type %d entered ordinary MSA path; native 7.48 marks it as a null object", objectType)
		}
	}
}

func testLoginWYS() []byte {
	data := make([]byte, 1+128+8)
	copy(data[:5], []byte("WS10 "))
	dds := data[1:]
	binary.LittleEndian.PutUint32(dds[4:8], 124)
	binary.LittleEndian.PutUint32(dds[12:16], 4)
	binary.LittleEndian.PutUint32(dds[16:20], 4)
	binary.LittleEndian.PutUint32(dds[76:80], 32)
	binary.LittleEndian.PutUint32(dds[80:84], 4)
	dds[0x54] = '2'
	return data
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

func TestRuntimeAssetPathPrefersAssetsBesideExecutable(t *testing.T) {
	executable, err := os.Executable()
	if err != nil {
		t.Fatal(err)
	}

	name := "runtime-path-test.wyt"
	want := filepath.Join(filepath.Dir(executable), "assets", "current", "UI", name)
	if err := os.MkdirAll(filepath.Dir(want), 0700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(want, []byte("test"), 0600); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = os.Remove(want) })

	if got := runtimeAssetPath("UI", name); got != want {
		t.Fatalf("runtimeAssetPath() = %q, want executable-local asset %q", got, want)
	}
}

func TestRuntimeAssetPathFallsBackToRelativePackagePath(t *testing.T) {
	name := "missing-runtime-path-test.wyt"
	want := filepath.Join("assets", "current", "UI", name)

	if got := runtimeAssetPath("UI", name); got != want {
		t.Fatalf("runtimeAssetPath() = %q, want relative fallback %q", got, want)
	}
}
