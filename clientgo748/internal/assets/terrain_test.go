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
