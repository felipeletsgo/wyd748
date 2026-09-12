package graphics

import (
	"math"
	"testing"
)

func TestProjectMeshGeometryCentersLookAtTarget(t *testing.T) {
	geometry := MeshGeometry{Vertices: []MeshVertex{{Position: Position3{X: 2, Y: 0, Z: 5}}}}
	projected, err := ProjectMeshGeometry(geometry, SceneTransform{Position: Position3{X: 8}, Scale: 1}, Camera{
		Position: Position3{X: 10, Y: 0, Z: 0}, Target: Position3{X: 10, Y: 0, Z: 5},
		FOVDegrees: 60, Near: .1, Far: 100,
	}, 1)
	if err != nil {
		t.Fatal(err)
	}
	p := projected.Vertices[0].Position
	if math.Abs(float64(p.X)) > 1e-5 || math.Abs(float64(p.Y)) > 1e-5 || p.Z <= -1 || p.Z >= 1 {
		t.Fatalf("unexpected projected center: %+v", p)
	}
}

func TestProjectMeshGeometryYawAndAspect(t *testing.T) {
	geometry := MeshGeometry{Vertices: []MeshVertex{{Position: Position3{X: 1}}}}
	projected, err := ProjectMeshGeometry(geometry, SceneTransform{Yaw: 90, Position: Position3{Z: 6}}, Camera{
		Position: Position3{}, Target: Position3{Z: 1}, FOVDegrees: 90, Near: .1, Far: 100,
	}, 2)
	if err != nil {
		t.Fatal(err)
	}
	p := projected.Vertices[0].Position
	if math.Abs(float64(p.X)) > 1e-5 || math.Abs(float64(p.Y)) > 1e-5 {
		t.Fatalf("yawed vertex should remain centered, got %+v", p)
	}
}

func TestProjectMeshGeometryRejectsNearPlane(t *testing.T) {
	_, err := ProjectMeshGeometry(MeshGeometry{Vertices: []MeshVertex{{Position: Position3{Z: .01}}}}, SceneTransform{Scale: 1}, Camera{
		Target: Position3{Z: 1}, FOVDegrees: 60, Near: .1, Far: 100,
	}, 1)
	if err == nil {
		t.Fatal("expected near-plane rejection")
	}
}

func TestProjectVisibleMeshGeometryDropsTrianglesBehindNearPlane(t *testing.T) {
	geometry := MeshGeometry{
		Vertices: []MeshVertex{
			{Position: Position3{X: -1, Z: 2}},
			{Position: Position3{X: 1, Z: 2}},
			{Position: Position3{Y: 1, Z: 2}},
			{Position: Position3{Z: -.1}},
		},
		Indices: []uint16{0, 1, 2, 0, 2, 3},
	}
	projected, err := ProjectVisibleMeshGeometry(geometry, SceneTransform{Scale: 1}, Camera{
		Target: Position3{Z: 1}, FOVDegrees: 60, Near: .1, Far: 100,
	}, 1)
	if err != nil {
		t.Fatal(err)
	}
	if len(projected.Indices) != 3 || projected.Indices[0] != 0 || projected.Indices[1] != 1 || projected.Indices[2] != 2 {
		t.Fatalf("visible indices=%v want [0 1 2]", projected.Indices)
	}
}

func TestProjectVisibleMeshGeometryPreservesTerrainTextureCoordinates(t *testing.T) {
	wantPrimary := TexCoord2{U: .25, V: .75}
	wantSecondary := TexCoord2{U: .5, V: .25}
	geometry := MeshGeometry{
		Vertices: []MeshVertex{
			{
				Position:             Position3{X: -1, Z: 2},
				TexCoord:             wantPrimary,
				SecondaryTexCoord:    wantSecondary,
				HasTexCoord:          true,
				HasSecondaryTexCoord: true,
			},
			{Position: Position3{X: 1, Z: 2}},
			{Position: Position3{Y: 1, Z: 2}},
		},
		Indices: []uint16{0, 1, 2},
	}
	projected, err := ProjectVisibleMeshGeometry(geometry, SceneTransform{Scale: 1}, Camera{
		Target: Position3{Z: 1}, FOVDegrees: 60, Near: .1, Far: 100,
	}, 1)
	if err != nil {
		t.Fatal(err)
	}
	got := projected.Vertices[0]
	if got.TexCoord != wantPrimary || got.SecondaryTexCoord != wantSecondary || !got.HasTexCoord || !got.HasSecondaryTexCoord {
		t.Fatalf("projected texture coordinates changed: %+v", got)
	}
}

func TestProjectVisibleMeshGeometryNativeFrontFaceIsClockwise(t *testing.T) {
	geometry := MeshGeometry{
		Vertices: []MeshVertex{
			{Position: Position3{X: -1, Y: -1, Z: 2}},
			{Position: Position3{Y: 1, Z: 2}},
			{Position: Position3{X: 1, Y: -1, Z: 2}},
		},
		Indices: []uint16{0, 1, 2},
	}
	projected, err := ProjectVisibleMeshGeometry(geometry, SceneTransform{Scale: 1}, Camera{
		Target: Position3{Z: 1}, FOVDegrees: 60, Near: .1, Far: 100,
	}, 1)
	if err != nil {
		t.Fatal(err)
	}
	a := projected.Vertices[0].Position
	b := projected.Vertices[1].Position
	c := projected.Vertices[2].Position
	signedArea := (b.X-a.X)*(c.Y-a.Y) - (b.Y-a.Y)*(c.X-a.X)
	if signedArea >= 0 {
		t.Fatalf("native front-facing triangle projected with signed area %g, want clockwise (< 0)", signedArea)
	}
}
