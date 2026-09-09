package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
)

// Terrain is the bounded, ownership-safe representation of one native 7.48
// TRN resource. The cell payload remains opaque until its field semantics are
// confirmed in the native loader; callers can still inspect and render a
// deterministic diagnostic surface without inventing height or collision.
type Terrain struct {
	Name           string
	HeaderWidth    uint8
	HeaderHeight   uint8
	Columns        int
	Rows           int
	CellRecordSize int
	Cells          []TerrainCell
}

// TerrainCell owns a copy of the native record. Keeping bytes opaque is
// intentional: texture, height and attribute meanings are not interchangeable
// and must not be guessed from a later client implementation.
type TerrainCell struct {
	X, Y uint16
	Raw  [12]byte
}

const (
	terrainHeaderMin = 8
	terrainCellSize  = 12
	maxTerrainCells  = 65536
)

var (
	ErrInvalidTerrain = errors.New("assets: invalid TRN terrain")
	ErrTerrainTooLarge = errors.New("assets: TRN terrain exceeds limits")
)

// ParseTerrain decodes the framing observed in the official 7.48 TRN assets:
// a length-prefixed name, two header bytes, then fixed-size cell records. It
// validates every length before allocating and returns no alias to input.
func ParseTerrain(data []byte) (Terrain, error) {
	if len(data) < terrainHeaderMin || int(data[0])+3 > len(data) {
		return Terrain{}, ErrInvalidTerrain
	}
	nameLen := int(data[0])
	if nameLen == 0 || nameLen > 32 {
		return Terrain{}, fmt.Errorf("%w: invalid name length %d", ErrInvalidTerrain, nameLen)
	}
	headerEnd := 1 + nameLen + 2
	if headerEnd > len(data) || (len(data)-headerEnd)%terrainCellSize != 0 {
		return Terrain{}, ErrInvalidTerrain
	}
	cellCount := (len(data) - headerEnd) / terrainCellSize
	if cellCount == 0 || cellCount > maxTerrainCells {
		return Terrain{}, ErrTerrainTooLarge
	}
	columns := int(math.Sqrt(float64(cellCount)))
	if columns*columns != cellCount {
		return Terrain{}, fmt.Errorf("%w: cell count %d is not a square", ErrInvalidTerrain, cellCount)
	}
	rows := columns
	cells := make([]TerrainCell, cellCount)
	for i := range cells {
		offset := headerEnd + i*terrainCellSize
		copy(cells[i].Raw[:], data[offset:offset+terrainCellSize])
		cells[i].X = uint16(i % columns)
		cells[i].Y = uint16(i / columns)
	}
	return Terrain{
		Name: string(data[1 : 1+nameLen]), HeaderWidth: data[1+nameLen],
		HeaderHeight: data[2+nameLen], Columns: columns, Rows: rows,
		CellRecordSize: terrainCellSize, Cells: cells,
	}, nil
}

// Cell returns a copy of one cell and rejects coordinates outside the parsed
// surface. The returned Raw array is independent and safe to retain.
func (t Terrain) Cell(x, y int) (TerrainCell, bool) {
	if x < 0 || y < 0 || x >= t.Columns || y >= t.Rows {
		return TerrainCell{}, false
	}
	return t.Cells[y*t.Columns+x], true
}

// NativeDimensions exposes the two header bytes for diagnostics only. Their
// semantic role is deliberately not promoted to gameplay state yet.
func (t Terrain) NativeDimensions() (uint8, uint8) {
	return t.HeaderWidth, t.HeaderHeight
}

// TerrainRecordWord is a bounded diagnostic accessor, not a semantic field.
func (c TerrainCell) TerrainRecordWord(offset int) (uint16, bool) {
	if offset < 0 || offset+2 > len(c.Raw) || offset%2 != 0 {
		return 0, false
	}
	return binary.LittleEndian.Uint16(c.Raw[offset : offset+2]), true
}
