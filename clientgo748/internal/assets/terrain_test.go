package assets

import (
	"bytes"
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestParseOfficialCharacterTerrain(t *testing.T) {
	_, file, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	path := filepath.Join(filepath.Dir(file), "..", "..", "assets", "current", "Env", "Character.trn")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("ReadFile(Character.trn): %v", err)
	}
	terrain, err := ParseTerrain(data)
	if err != nil {
		t.Fatalf("ParseTerrain(Character.trn): %v", err)
	}
	if terrain.Name != "Field" || terrain.HeaderWidth != 0x10 || terrain.HeaderHeight != 0x10 {
		t.Fatalf("header = name %q dimensions %02x/%02x", terrain.Name, terrain.HeaderWidth, terrain.HeaderHeight)
	}
	if terrain.Columns != 64 || terrain.Rows != 64 || len(terrain.Cells) != 4096 {
		t.Fatalf("surface = %dx%d with %d cells", terrain.Columns, terrain.Rows, len(terrain.Cells))
	}
	cell, ok := terrain.Cell(63, 63)
	if !ok || cell.X != 63 || cell.Y != 63 {
		t.Fatalf("last cell = %#v, ok=%v", cell, ok)
	}
	if _, ok := terrain.Cell(-1, 0); ok {
		t.Fatal("negative coordinate accepted")
	}
	if _, ok := terrain.Cell(64, 0); ok {
		t.Fatal("out-of-range coordinate accepted")
	}
	if word, ok := cell.TerrainRecordWord(0); !ok || word != uint16(cell.Raw[0])|uint16(cell.Raw[1])<<8 {
		t.Fatalf("record word = %04x, ok=%v", word, ok)
	}
	if cell.Height != int8(cell.Raw[0]) {
		t.Fatalf("height = %d, raw[0] = %d", cell.Height, int8(cell.Raw[0]))
	}
	if cell.PrimaryMaterialIndex != cell.Raw[1] || cell.PrimaryUVSelector != cell.Raw[2] ||
		cell.SecondaryMaterialIndex != cell.Raw[3] || cell.SecondaryUVSelector != cell.Raw[4] {
		t.Fatalf("promoted terrain fields do not match raw record: %#v", cell)
	}
	if cell.PrimaryTextureSlot() != uint16(cell.Raw[1])+10 || cell.SecondaryTextureSlot() != uint16(cell.Raw[3])+0x100 {
		t.Fatalf("native texture slot projection mismatch: primary=%d secondary=%d", cell.PrimaryTextureSlot(), cell.SecondaryTextureSlot())
	}
}

func TestNativeTerrainUVTables(t *testing.T) {
	primary, ok := PrimaryTerrainUV(7)
	if !ok || primary != ([4]TerrainUV{{1, 0}, {1, 1}, {0, 0}, {0, 1}}) {
		t.Fatalf("primary selector 7 = %#v, ok=%v", primary, ok)
	}
	secondary, ok := SecondaryTerrainUV(31)
	if !ok || secondary != ([4]TerrainUV{{.5, .5}, {.5, 1}, {0, .5}, {0, 1}}) {
		t.Fatalf("secondary selector 31 = %#v, ok=%v", secondary, ok)
	}
	if _, ok := PrimaryTerrainUV(8); ok {
		t.Fatal("primary selector 8 accepted")
	}
	if _, ok := SecondaryTerrainUV(32); ok {
		t.Fatal("secondary selector 32 accepted")
	}
}

func TestParseTerrainOwnsCellBytes(t *testing.T) {
	data := []byte{1, 'X', 1, 1}
	data = append(data, make([]byte, terrainCellSize)...)
	terrain, err := ParseTerrain(data)
	if err != nil {
		t.Fatal(err)
	}
	data[len(data)-terrainCellSize] = 0xff
	cell, _ := terrain.Cell(0, 0)
	if cell.Raw[0] != 0 {
		t.Fatalf("cell aliases input: %x", cell.Raw[0])
	}
}

func TestParseTerrainPreservesSignedNativeHeight(t *testing.T) {
	data := []byte{1, 'X', 1, 1}
	data = append(data, make([]byte, terrainCellSize)...)
	data[4] = 0x80
	terrain, err := ParseTerrain(data)
	if err != nil {
		t.Fatal(err)
	}
	cell, ok := terrain.Cell(0, 0)
	if !ok || cell.Height != -128 {
		t.Fatalf("height = %d, ok=%v; want -128", cell.Height, ok)
	}
}

func TestParseTerrainRejectsMalformedInput(t *testing.T) {
	valid := append([]byte{1, 'X', 1, 1}, make([]byte, terrainCellSize)...)
	tests := []struct {
		name string
		data []byte
		want error
	}{
		{"short name", []byte{1}, ErrInvalidTerrain},
		{"zero name", append([]byte{0, 1, 1}, make([]byte, terrainCellSize)...), ErrInvalidTerrain},
		{"truncated record", valid[:len(valid)-1], ErrInvalidTerrain},
		{"non-square payload", append([]byte{1, 'X', 1, 1}, make([]byte, terrainCellSize*2)...), ErrInvalidTerrain},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			_, err := ParseTerrain(test.data)
			if !errors.Is(err, test.want) {
				t.Fatalf("error = %v, want %v", err, test.want)
			}
		})
	}
}

func TestTerrainRecordWordBounds(t *testing.T) {
	cell := TerrainCell{}
	if _, ok := cell.TerrainRecordWord(-2); ok {
		t.Fatal("negative word offset accepted")
	}
	if _, ok := cell.TerrainRecordWord(1); ok {
		t.Fatal("unaligned word offset accepted")
	}
	if _, ok := cell.TerrainRecordWord(12); ok {
		t.Fatal("out-of-range word offset accepted")
	}
	if !bytes.Equal(cell.Raw[:], make([]byte, terrainCellSize)) {
		t.Fatal("unexpected zero-value cell")
	}
}
