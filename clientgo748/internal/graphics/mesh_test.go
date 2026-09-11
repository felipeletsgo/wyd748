package graphics

import (
	"encoding/binary"
	"errors"
	"math"
	"testing"

	"wydclient748/internal/assets"
)

func TestExtractMeshGeometryUsesXYZPrefixAndIndices(t *testing.T) {
	vertices := make([]byte, 2*16)
	putFloat32(vertices[0:], 1.25)
	putFloat32(vertices[4:], -2.5)
	putFloat32(vertices[8:], 3.75)
	putFloat32(vertices[16:], 4)
	putFloat32(vertices[20:], 5)
	putFloat32(vertices[24:], 6)

	geometry, err := ExtractMeshGeometry(assets.Mesh{
		VertexStride:   16,
		VertexCount:    2,
		FaceIndexCount: 3,
		Vertices:       vertices,
		Indices:        []uint16{0, 1, 0},
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(geometry.Positions) != 2 || geometry.Positions[0] != (Position3{X: 1.25, Y: -2.5, Z: 3.75}) || geometry.Positions[1] != (Position3{X: 4, Y: 5, Z: 6}) {
		t.Fatalf("unexpected positions: %+v", geometry.Positions)
	}
	if len(geometry.Indices) != 3 || geometry.Indices[1] != 1 {
		t.Fatalf("unexpected indices: %v", geometry.Indices)
	}
}

func TestExtractMeshGeometryRejectsUnsafeBuffers(t *testing.T) {
	tests := []assets.Mesh{
		{VertexStride: 8},
		{VertexStride: 12, VertexCount: 1, Vertices: make([]byte, 8)},
		{VertexStride: 12, VertexCount: 1, FaceIndexCount: 2, Vertices: make([]byte, 12), Indices: []uint16{0, 0}},
		{VertexStride: 12, VertexCount: 1, FaceIndexCount: 3, Vertices: make([]byte, 12), Indices: []uint16{0, 1, 0}},
	}
	for i, mesh := range tests {
		if _, err := ExtractMeshGeometry(mesh); !errors.Is(err, ErrInvalidMeshGeometry) {
			t.Fatalf("case %d: expected ErrInvalidMeshGeometry, got %v", i, err)
		}
	}
}

func putFloat32(dst []byte, value float32) {
	binary.LittleEndian.PutUint32(dst, math.Float32bits(value))
}
