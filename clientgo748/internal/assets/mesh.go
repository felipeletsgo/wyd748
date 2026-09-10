package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"os"
)

// Mesh é a representação própria do client Go para o framing MSH observado
// nos assets 7.48. Os buffers são cópias do arquivo e pertencem ao caller;
// nenhum deles é compartilhado com o arquivo ou com o renderer.
type Mesh struct {
	ParentID       uint32
	ID             uint32
	FVF            uint32
	VertexStride   uint32
	FaceInfluence  uint32
	Palette        []MeshMatrix
	BoneNames      []uint32
	VertexCount    uint32
	FaceIndexCount uint32
	Vertices       []byte
	Indices        []uint16
}

// MeshMatrix preserva uma matriz D3DX 4x4 no layout linear little-endian do
// asset. A conversão para a API gráfica fica fora do parser.
type MeshMatrix [16]float32

var (
	ErrInvalidMSH   = errors.New("clientgo748: invalid MSH mesh")
	ErrTruncatedMSH = errors.New("clientgo748: truncated MSH mesh")
	ErrMeshTooLarge = errors.New("clientgo748: MSH mesh exceeds safety limits")
)

const (
	mshHeaderSize       = 8 * 4
	maxMeshVertexStride = 4096
	maxMeshVertices     = 1 << 20
	maxMeshIndices      = 1 << 22
	maxMeshPalette      = 1024
)

// LoadMSHFile lê um MSH sem alterar o asset original.
func LoadMSHFile(path string) (Mesh, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return Mesh{}, fmt.Errorf("clientgo748: read MSH %q: %w", path, err)
	}
	m, err := ParseMSH(b)
	if err != nil {
		return Mesh{}, fmt.Errorf("clientgo748: parse MSH %q: %w", path, err)
	}
	return m, nil
}

// ParseMSH decodifica o framing usado pelo loader candidato do TMProject:
// oito uint32, paleta de matrizes, nomes de ossos, vértices e índices 16-bit.
// O parser valida todas as contagens e exige que não sobrem bytes silenciosos.
func ParseMSH(data []byte) (Mesh, error) {
	if len(data) < mshHeaderSize {
		return Mesh{}, ErrTruncatedMSH
	}
	h := make([]uint32, 8)
	for i := range h {
		h[i] = binary.LittleEndian.Uint32(data[i*4:])
	}
	parent, id, fvf, stride, influence, palette, vertices, indices := h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7]
	if stride == 0 || stride > maxMeshVertexStride || vertices > maxMeshVertices || indices > maxMeshIndices || palette > maxMeshPalette {
		return Mesh{}, ErrMeshTooLarge
	}
	if indices%3 != 0 {
		return Mesh{}, fmt.Errorf("%w: face index count %d is not divisible by 3", ErrInvalidMSH, indices)
	}
	off := uint64(mshHeaderSize)
	need := func(n uint64) error {
		if n > uint64(len(data))-off {
			return ErrTruncatedMSH
		}
		off += n
		return nil
	}
	paletteBytes := uint64(palette) * 64
	if err := need(paletteBytes); err != nil {
		return Mesh{}, err
	}
	pm := make([]MeshMatrix, palette)
	for i := range pm {
		start := mshHeaderSize + i*64
		for j := range pm[i] {
			pm[i][j] = math.Float32frombits(binary.LittleEndian.Uint32(data[start+j*4:]))
		}
	}
	namesStart := int(off)
	if err := need(uint64(palette) * 4); err != nil {
		return Mesh{}, err
	}
	names := make([]uint32, palette)
	for i := range names {
		names[i] = binary.LittleEndian.Uint32(data[namesStart+i*4:])
	}
	vertexBytes := uint64(stride) * uint64(vertices)
	vertexStart := int(off)
	if err := need(vertexBytes); err != nil {
		return Mesh{}, err
	}
	indexStart := int(off)
	if err := need(uint64(indices) * 2); err != nil {
		return Mesh{}, err
	}
	if off != uint64(len(data)) {
		return Mesh{}, fmt.Errorf("%w: trailing %d bytes", ErrInvalidMSH, uint64(len(data))-off)
	}
	verts := append([]byte(nil), data[vertexStart:indexStart]...)
	idx := make([]uint16, indices)
	for i := range idx {
		idx[i] = binary.LittleEndian.Uint16(data[indexStart+i*2:])
	}
	return Mesh{ParentID: parent, ID: id, FVF: fvf, VertexStride: stride, FaceInfluence: influence, Palette: pm, BoneNames: names, VertexCount: vertices, FaceIndexCount: indices, Vertices: verts, Indices: idx}, nil
}
