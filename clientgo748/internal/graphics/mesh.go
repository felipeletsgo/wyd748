package graphics

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"

	"wydclient748/internal/assets"
)

// ErrInvalidMeshGeometry marks an MSH that cannot safely reach a renderer.
var ErrInvalidMeshGeometry = errors.New("clientgo748: invalid mesh geometry")

// Position3 is the native-confirmed XYZ prefix of one MSH vertex record.
type Position3 struct {
	X float32
	Y float32
	Z float32
}

// MeshGeometry is the renderer-facing subset currently proven for MSH:
// object-space XYZ positions and triangle-list uint16 indices. Materials,
// normals, UVs, palette skinning and animation intentionally stay outside it.
type MeshGeometry struct {
	Positions []Position3
	Indices   []uint16
}

// ExtractMeshGeometry validates and copies only the MSH fields that the native
// 7.48 loader proves are geometry. It does not infer the remaining vertex data.
func ExtractMeshGeometry(mesh assets.Mesh) (MeshGeometry, error) {
	if mesh.VertexStride < 12 {
		return MeshGeometry{}, fmt.Errorf("%w: vertex stride %d is smaller than XYZ", ErrInvalidMeshGeometry, mesh.VertexStride)
	}
	expectedVertexBytes := uint64(mesh.VertexStride) * uint64(mesh.VertexCount)
	if expectedVertexBytes != uint64(len(mesh.Vertices)) {
		return MeshGeometry{}, fmt.Errorf("%w: vertex buffer has %d bytes, expected %d", ErrInvalidMeshGeometry, len(mesh.Vertices), expectedVertexBytes)
	}
	if mesh.FaceIndexCount%3 != 0 || uint64(mesh.FaceIndexCount) != uint64(len(mesh.Indices)) {
		return MeshGeometry{}, fmt.Errorf("%w: index count %d does not match triangle buffer length %d", ErrInvalidMeshGeometry, mesh.FaceIndexCount, len(mesh.Indices))
	}

	positions := make([]Position3, mesh.VertexCount)
	stride := int(mesh.VertexStride)
	for i := range positions {
		off := i * stride
		positions[i] = Position3{
			X: math.Float32frombits(binary.LittleEndian.Uint32(mesh.Vertices[off:])),
			Y: math.Float32frombits(binary.LittleEndian.Uint32(mesh.Vertices[off+4:])),
			Z: math.Float32frombits(binary.LittleEndian.Uint32(mesh.Vertices[off+8:])),
		}
	}

	indices := append([]uint16(nil), mesh.Indices...)
	for _, index := range indices {
		if uint32(index) >= mesh.VertexCount {
			return MeshGeometry{}, fmt.Errorf("%w: vertex index %d outside vertex count %d", ErrInvalidMeshGeometry, index, mesh.VertexCount)
		}
	}
	return MeshGeometry{Positions: positions, Indices: indices}, nil
}
