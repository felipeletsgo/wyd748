package graphics

import (
	"errors"
	"testing"

	"wydclient748/internal/assets"
)

func TestSkeletonAppliesHierarchyAndBuildsPalette(t *testing.T) {
	skeleton, err := BuildSkeleton(assets.BoneFile{Links: []assets.BoneLink{
		{ParentID: ^uint32(0), BoneID: 0},
		{ParentID: 0, BoneID: 1},
		{ParentID: 1, BoneID: 2},
	}})
	if err != nil {
		t.Fatal(err)
	}

	frame0 := identityMeshMatrix()
	frame0[12] = 10
	frame1 := identityMeshMatrix()
	frame1[13] = 20
	frame2 := identityMeshMatrix()
	frame2[14] = 30
	skeleton.ApplyFrameMatrices([]assets.MeshMatrix{frame0, frame1, frame2})

	bind1 := identityMeshMatrix()
	bind1[12] = 1
	bind2 := identityMeshMatrix()
	bind2[13] = 2
	palette, err := skeleton.BuildSkinPalette(assets.Mesh{
		Palette:   []assets.MeshMatrix{identityMeshMatrix(), bind1, bind2},
		BoneNames: []uint32{0, 1, 2},
	})
	if err != nil {
		t.Fatal(err)
	}

	// BoneName 0 encontra a raiz sintética antes do osso BON 0.
	if palette[0] != identityMeshMatrix() {
		t.Fatalf("bone 0 should resolve to synthetic root: %+v", palette[0])
	}
	if palette[1][12] != 11 || palette[1][13] != 20 || palette[1][14] != 0 {
		t.Fatalf("unexpected bone 1 palette matrix: %+v", palette[1])
	}
	if palette[2][12] != 10 || palette[2][13] != 22 || palette[2][14] != 30 {
		t.Fatalf("unexpected bone 2 palette matrix: %+v", palette[2])
	}
}

func TestSkeletonUsesD3DXMatrixOrder(t *testing.T) {
	skeleton, err := BuildSkeleton(assets.BoneFile{Links: []assets.BoneLink{
		{ParentID: ^uint32(0), BoneID: 0},
		{ParentID: 0, BoneID: 1},
	}})
	if err != nil {
		t.Fatal(err)
	}

	parent := identityMeshMatrix()
	parent[12] = 10
	childScale := identityMeshMatrix()
	childScale[0] = 2
	skeleton.ApplyFrameMatrices([]assets.MeshMatrix{parent, childScale})

	bind := identityMeshMatrix()
	bind[12] = 3
	palette, err := skeleton.BuildSkinPalette(assets.Mesh{
		Palette:   []assets.MeshMatrix{bind},
		BoneNames: []uint32{1},
	})
	if err != nil {
		t.Fatal(err)
	}

	// combined = childScale * parentTranslation: a ordem inversa produziria 20.
	// palette = bindTranslation * combined: a ordem inversa produziria 13.
	if palette[0][0] != 2 || palette[0][12] != 16 {
		t.Fatalf("unexpected D3DX multiplication order: %+v", palette[0])
	}
}

func TestSkinMeshGeometryBlendsPaletteTransforms(t *testing.T) {
	geometry := MeshGeometry{
		Vertices: []MeshVertex{{
			Position:       Position3{X: 1, Y: 2, Z: 3},
			Normal:         Position3{Z: 1},
			BlendWeights:   [4]float32{0.25, 0.75},
			PaletteIndices: [4]uint8{0, 1},
			InfluenceCount: 2,
		}},
		Indices: []uint16{0, 0, 0},
	}
	palette0 := identityMeshMatrix()
	palette0[12] = 10
	palette1 := identityMeshMatrix()
	palette1[13] = 20

	skinned, err := SkinMeshGeometry(geometry, []assets.MeshMatrix{palette0, palette1})
	if err != nil {
		t.Fatal(err)
	}
	vertex := skinned.Vertices[0]
	if vertex.Position != (Position3{X: 3.5, Y: 17, Z: 3}) {
		t.Fatalf("unexpected blended position: %+v", vertex.Position)
	}
	if vertex.Normal != (Position3{Z: 1}) {
		t.Fatalf("unexpected blended normal: %+v", vertex.Normal)
	}
	if geometry.Vertices[0].Position != (Position3{X: 1, Y: 2, Z: 3}) {
		t.Fatal("SkinMeshGeometry mutated its input")
	}
}

func TestSkinMeshGeometryRejectsMissingPaletteEntry(t *testing.T) {
	_, err := SkinMeshGeometry(MeshGeometry{Vertices: []MeshVertex{{
		BlendWeights:   [4]float32{1},
		PaletteIndices: [4]uint8{1},
		InfluenceCount: 1,
	}}}, []assets.MeshMatrix{identityMeshMatrix()})
	if !errors.Is(err, ErrInvalidSkinning) {
		t.Fatalf("expected ErrInvalidSkinning, got %v", err)
	}
}

func TestBuildSkeletonRejectsBrokenHierarchy(t *testing.T) {
	tests := []assets.BoneFile{
		{Links: []assets.BoneLink{{ParentID: 42, BoneID: 1}}},
		{Links: []assets.BoneLink{{ParentID: ^uint32(0), BoneID: 1}, {ParentID: 1, BoneID: 1}}},
	}
	for i, bone := range tests {
		if _, err := BuildSkeleton(bone); !errors.Is(err, ErrInvalidSkeleton) {
			t.Fatalf("case %d: expected ErrInvalidSkeleton, got %v", i, err)
		}
	}
}

func TestBuildSkinPaletteRejectsMissingBone(t *testing.T) {
	skeleton, err := BuildSkeleton(assets.BoneFile{Links: []assets.BoneLink{{ParentID: ^uint32(0), BoneID: 0}}})
	if err != nil {
		t.Fatal(err)
	}
	skeleton.ApplyFrameMatrices(nil)
	_, err = skeleton.BuildSkinPalette(assets.Mesh{
		Palette:   []assets.MeshMatrix{identityMeshMatrix()},
		BoneNames: []uint32{7},
	})
	if !errors.Is(err, ErrInvalidSkeleton) {
		t.Fatalf("expected ErrInvalidSkeleton, got %v", err)
	}
}
