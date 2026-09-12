package graphics

import (
	"errors"
	"fmt"
	"math"
)

var ErrInvalidScene3D = errors.New("clientgo748: invalid 3D scene transform")

// SceneTransform is the per-object transform consumed before camera
// projection. Angles are degrees to match the values used by TMProject scene
// setup and to keep scene code readable.
type SceneTransform struct {
	Position Position3
	Yaw      float32
	Pitch    float32
	Roll     float32
	Scale    float32
}

// Camera is a compact left-handed look-at camera. Projected positions use
// OpenGL NDC: X/Y and Z are all in [-1,+1].
type Camera struct {
	Position   Position3
	Target     Position3
	Up         Position3
	FOVDegrees float32
	Near       float32
	Far        float32
}

func (c Camera) normalized() (Camera, error) {
	if c.FOVDegrees == 0 {
		c.FOVDegrees = 45
	}
	if c.Near == 0 {
		c.Near = 0.05
	}
	if c.Far == 0 {
		c.Far = 1000
	}
	if c.Up == (Position3{}) {
		c.Up = Position3{Y: 1}
	}
	if !finitePosition(c.Position) || !finitePosition(c.Target) || !finitePosition(c.Up) ||
		!finiteFloat32(c.FOVDegrees) || !finiteFloat32(c.Near) || !finiteFloat32(c.Far) ||
		c.FOVDegrees <= 1 || c.FOVDegrees >= 179 || c.Near <= 0 || c.Far <= c.Near {
		return Camera{}, ErrInvalidScene3D
	}
	return c, nil
}

// ProjectMeshGeometry applies object rotation/scale/translation and a look-at
// perspective camera. Geometry attributes other than Position are preserved.
func ProjectMeshGeometry(geometry MeshGeometry, transform SceneTransform, camera Camera, aspect float32) (MeshGeometry, error) {
	return projectMeshGeometry(geometry, transform, camera, aspect, false)
}

// ProjectVisibleMeshGeometry projects triangle-list geometry while discarding
// triangles that cross or fall behind the near plane. Existing scene meshes
// keep the strict ProjectMeshGeometry behavior; broad terrain surfaces use this
// tolerant path because they naturally surround the active camera.
func ProjectVisibleMeshGeometry(geometry MeshGeometry, transform SceneTransform, camera Camera, aspect float32) (MeshGeometry, error) {
	return projectMeshGeometry(geometry, transform, camera, aspect, true)
}

func projectMeshGeometry(geometry MeshGeometry, transform SceneTransform, camera Camera, aspect float32, discardBehindNear bool) (MeshGeometry, error) {
	if !finiteFloat32(aspect) || aspect <= 0 {
		return MeshGeometry{}, fmt.Errorf("%w: invalid aspect ratio %g", ErrInvalidScene3D, aspect)
	}
	if transform.Scale == 0 {
		transform.Scale = 1
	}
	if !finitePosition(transform.Position) || !finiteFloat32(transform.Yaw) || !finiteFloat32(transform.Pitch) ||
		!finiteFloat32(transform.Roll) || !finiteFloat32(transform.Scale) || transform.Scale <= 0 {
		return MeshGeometry{}, ErrInvalidScene3D
	}
	camera, err := camera.normalized()
	if err != nil {
		return MeshGeometry{}, err
	}

	forward, ok := normalize3(sub3(camera.Target, camera.Position))
	if !ok {
		return MeshGeometry{}, fmt.Errorf("%w: camera target equals position", ErrInvalidScene3D)
	}
	right, ok := normalize3(cross3(camera.Up, forward))
	if !ok {
		return MeshGeometry{}, fmt.Errorf("%w: camera up is parallel to view direction", ErrInvalidScene3D)
	}
	up := cross3(forward, right)
	focal := float32(1 / math.Tan(float64(camera.FOVDegrees)*math.Pi/360))

	projected := MeshGeometry{
		Vertices: make([]MeshVertex, len(geometry.Vertices)),
	}
	visible := make([]bool, len(geometry.Vertices))
	for i, vertex := range geometry.Vertices {
		world := applySceneTransform(vertex.Position, transform)
		viewDelta := sub3(world, camera.Position)
		viewX := dot3(viewDelta, right)
		viewY := dot3(viewDelta, up)
		viewZ := dot3(viewDelta, forward)
		if viewZ <= camera.Near {
			if discardBehindNear {
				continue
			}
			return MeshGeometry{}, fmt.Errorf("%w: vertex %d is behind the near plane", ErrInvalidScene3D, i)
		}
		visible[i] = true
		ndcZ := ((viewZ-camera.Near)/(camera.Far-camera.Near))*2 - 1
		if ndcZ > 1 {
			ndcZ = 1
		}
		vertex.Position = Position3{
			X: (viewX * focal) / (viewZ * aspect),
			Y: (viewY * focal) / viewZ,
			Z: ndcZ,
		}
		projected.Vertices[i] = vertex
	}
	if !discardBehindNear {
		projected.Indices = append([]uint16(nil), geometry.Indices...)
		return projected, nil
	}
	if len(geometry.Indices)%3 != 0 {
		return MeshGeometry{}, fmt.Errorf("%w: triangle index count %d is not divisible by 3", ErrInvalidScene3D, len(geometry.Indices))
	}
	projected.Indices = make([]uint16, 0, len(geometry.Indices))
	for i := 0; i < len(geometry.Indices); i += 3 {
		a, b, c := geometry.Indices[i], geometry.Indices[i+1], geometry.Indices[i+2]
		if int(a) >= len(visible) || int(b) >= len(visible) || int(c) >= len(visible) {
			return MeshGeometry{}, fmt.Errorf("%w: triangle index outside vertex count", ErrInvalidScene3D)
		}
		if visible[a] && visible[b] && visible[c] {
			projected.Indices = append(projected.Indices, a, b, c)
		}
	}
	return projected, nil
}

func applySceneTransform(p Position3, t SceneTransform) Position3 {
	x, y, z := p.X*t.Scale, p.Y*t.Scale, p.Z*t.Scale
	yaw := float64(t.Yaw) * math.Pi / 180
	pitch := float64(t.Pitch) * math.Pi / 180
	roll := float64(t.Roll) * math.Pi / 180

	cy, sy := float32(math.Cos(yaw)), float32(math.Sin(yaw))
	x, z = cy*x+sy*z, -sy*x+cy*z
	cp, sp := float32(math.Cos(pitch)), float32(math.Sin(pitch))
	y, z = cp*y-sp*z, sp*y+cp*z
	cr, sr := float32(math.Cos(roll)), float32(math.Sin(roll))
	x, y = cr*x-sr*y, sr*x+cr*y
	return Position3{X: x + t.Position.X, Y: y + t.Position.Y, Z: z + t.Position.Z}
}

func sub3(a, b Position3) Position3 { return Position3{X: a.X - b.X, Y: a.Y - b.Y, Z: a.Z - b.Z} }
func dot3(a, b Position3) float32   { return a.X*b.X + a.Y*b.Y + a.Z*b.Z }
func cross3(a, b Position3) Position3 {
	return Position3{X: a.Y*b.Z - a.Z*b.Y, Y: a.Z*b.X - a.X*b.Z, Z: a.X*b.Y - a.Y*b.X}
}
func normalize3(v Position3) (Position3, bool) {
	length := float32(math.Sqrt(float64(dot3(v, v))))
	if !finiteFloat32(length) || length <= 1e-6 {
		return Position3{}, false
	}
	return Position3{X: v.X / length, Y: v.Y / length, Z: v.Z / length}, true
}
