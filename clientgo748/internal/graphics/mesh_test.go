package graphics

import (
	"encoding/binary"
	"errors"
	"math"
	"path/filepath"
	"testing"

	"wydclient748/internal/assets"
)

func TestExtractMeshGeometryDecodesSkinnedVertex(t *testing.T) {
	vertices := make([]byte, 44)
	putFloat32(vertices[0:], 1.25)
	putFloat32(vertices[4:], -2.5)
	putFloat32(vertices[8:], 3.75)
	putFloat32(vertices[12:], 0.25)
	putFloat32(vertices[16:], 0.5)
	copy(vertices[20:24], []byte{2, 1, 0, 99})
	putFloat32(vertices[24:], 0)
	putFloat32(vertices[28:], 1)
	putFloat32(vertices[32:], 0)
	putFloat32(vertices[36:], 0.75)
	putFloat32(vertices[40:], 0.125)

	geometry, err := ExtractMeshGeometry(assets.Mesh{
		FVF:            4378,
		VertexStride:   44,
		FaceInfluence:  3,
		Palette:        make([]assets.MeshMatrix, 3),
		BoneNames:      []uint32{10, 11, 12},
		VertexCount:    1,
		FaceIndexCount: 3,
		Vertices:       vertices,
		Indices:        []uint16{0, 0, 0},
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(geometry.Vertices) != 1 {
		t.Fatalf("unexpected vertex count: %d", len(geometry.Vertices))
	}
	v := geometry.Vertices[0]
	if v.Position != (Position3{X: 1.25, Y: -2.5, Z: 3.75}) || v.Normal != (Position3{Y: 1}) || v.TexCoord != (TexCoord2{U: 0.75, V: 0.125}) {
		t.Fatalf("unexpected attributes: %+v", v)
	}
	if v.InfluenceCount != 3 || v.BlendWeights != ([4]float32{0.25, 0.5, 0.25, 0}) || v.PaletteIndices != ([4]uint8{2, 1, 0, 99}) {
		t.Fatalf("unexpected skinning: %+v", v)
	}
}

func TestExtractMeshGeometryDecodesStaticVertex(t *testing.T) {
	vertices := make([]byte, 24)
	putFloat32(vertices[0:], 1)
	putFloat32(vertices[4:], 2)
	putFloat32(vertices[8:], 3)
	putFloat32(vertices[12:], 0)
	putFloat32(vertices[16:], 0)
	putFloat32(vertices[20:], 1)

	geometry, err := ExtractMeshGeometry(assets.Mesh{
		FVF:            18,
		VertexStride:   24,
		VertexCount:    1,
		FaceIndexCount: 3,
		Vertices:       vertices,
		Indices:        []uint16{0, 0, 0},
	})
	if err != nil {
		t.Fatal(err)
	}
	v := geometry.Vertices[0]
	if v.Position != (Position3{X: 1, Y: 2, Z: 3}) || v.Normal != (Position3{Z: 1}) || v.HasTexCoord || v.InfluenceCount != 0 {
		t.Fatalf("unexpected static vertex: %+v", v)
	}
}

func TestExtractMeshGeometryRejectsUnsafeBuffers(t *testing.T) {
	tests := []assets.Mesh{
		{FVF: 18, VertexStride: 12},
		{FVF: 18, VertexStride: 24, VertexCount: 1, Vertices: make([]byte, 8)},
		{FVF: 18, VertexStride: 24, VertexCount: 1, FaceIndexCount: 2, Vertices: make([]byte, 24), Indices: []uint16{0, 0}},
		{FVF: 18, VertexStride: 24, VertexCount: 1, FaceIndexCount: 3, Vertices: make([]byte, 24), Indices: []uint16{0, 1, 0}},
		{FVF: 4378, VertexStride: 44, FaceInfluence: 3, VertexCount: 1, FaceIndexCount: 3, Vertices: make([]byte, 44), Indices: []uint16{0, 0, 0}},
	}
	for i, mesh := range tests {
		if _, err := ExtractMeshGeometry(mesh); !errors.Is(err, ErrInvalidMeshGeometry) {
			t.Fatalf("case %d: expected ErrInvalidMeshGeometry, got %v", i, err)
		}
	}
}

func TestExtractMeshGeometryDecodesCurrentMeshCatalog(t *testing.T) {
	paths, err := filepath.Glob(filepath.Join("..", "..", "assets", "current", "mesh", "*.msh"))
	if err != nil {
		t.Fatal(err)
	}
	if len(paths) == 0 {
		t.Fatal("current MSH catalog is empty")
	}

	staticCount := 0
	skinnedCount := 0
	for _, path := range paths {
		mesh, err := assets.LoadMSHFile(path)
		if err != nil {
			t.Fatalf("load %s: %v", filepath.Base(path), err)
		}
		geometry, err := ExtractMeshGeometry(mesh)
		if err != nil {
			t.Fatalf("decode %s: %v", filepath.Base(path), err)
		}
		if len(geometry.Vertices) != int(mesh.VertexCount) || len(geometry.Indices) != int(mesh.FaceIndexCount) {
			t.Fatalf("decode %s: geometry counts do not match MSH header", filepath.Base(path))
		}
		if mesh.FaceInfluence == 0 {
			staticCount++
		} else {
			skinnedCount++
		}
	}
	if staticCount == 0 || skinnedCount == 0 {
		t.Fatalf("expected static and skinned assets, got static=%d skinned=%d", staticCount, skinnedCount)
	}
}

func TestExtractMSAGeometryDecodesOfficialStaticMesh(t *testing.T) {
	mesh, err := assets.LoadMSAFile(filepath.Join("..", "..", "assets", "current", "mesh", "kswa08.msa"))
	if err != nil {
		t.Fatal(err)
	}
	geometry, err := ExtractMSAGeometry(mesh)
	if err != nil {
		t.Fatal(err)
	}
	if len(geometry.Vertices) != int(mesh.VertexCount) || len(geometry.Indices) != len(mesh.Indices) {
		t.Fatalf("geometry counts = %d/%d, want %d/%d", len(geometry.Vertices), len(geometry.Indices), mesh.VertexCount, len(mesh.Indices))
	}
	v := geometry.Vertices[0]
	if !v.HasTexCoord || !v.HasSecondaryTexCoord || v.TexCoord != v.SecondaryTexCoord {
		t.Fatalf("native duplicated UV contract was not preserved: %+v", v)
	}
}

func TestExtractMSAMaterialGeometrySlicesNativeFaceRangeWithoutRebasing(t *testing.T) {
	mesh := assets.MSAMesh{
		VertexCount: 4,
		Indices:     []uint16{0, 1, 2, 2, 1, 3},
		Attributes: []assets.MSAAttributeRange{{
			FaceStart: 1,
			FaceCount: 1,
			VertexStart: 1,
			VertexCount: 3,
		}},
	}
	geometry := MeshGeometry{
		Vertices: make([]MeshVertex, 4),
		Indices:  append([]uint16(nil), mesh.Indices...),
	}
	material, err := ExtractMSAMaterialGeometry(mesh, geometry, 0)
	if err != nil {
		t.Fatal(err)
	}
	want := []uint16{2, 1, 3}
	if len(material.Vertices) != 4 || len(material.Indices) != len(want) {
		t.Fatalf("material geometry counts=%d/%d", len(material.Vertices), len(material.Indices))
	}
	for i := range want {
		if material.Indices[i] != want[i] {
			t.Fatalf("material index %d=%d want %d", i, material.Indices[i], want[i])
		}
	}
}

func TestExtractMeshGeometryClampsTinyNegativeImplicitWeight(t *testing.T) {
	vertices := make([]byte, 40)
	putFloat32(vertices[12:], 1.000001)
	copy(vertices[16:20], []byte{0, 0, 0, 0})
	mesh := assets.Mesh{
		FVF:            4376,
		VertexStride:   40,
		FaceInfluence:  2,
		Palette:        make([]assets.MeshMatrix, 1),
		BoneNames:      []uint32{1},
		VertexCount:    1,
		FaceIndexCount: 3,
		Vertices:       vertices,
		Indices:        []uint16{0, 0, 0},
	}
	geometry, err := ExtractMeshGeometry(mesh)
	if err != nil {
		t.Fatal(err)
	}
	if geometry.Vertices[0].BlendWeights[1] != 0 {
		t.Fatalf("expected clamped implicit weight, got %g", geometry.Vertices[0].BlendWeights[1])
	}
}

func putFloat32(dst []byte, value float32) {
	binary.LittleEndian.PutUint32(dst, math.Float32bits(value))
}
