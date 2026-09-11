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
