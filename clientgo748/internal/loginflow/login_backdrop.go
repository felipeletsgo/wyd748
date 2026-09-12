package loginflow

import (
	"errors"
	"fmt"
	"math"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
)

var ErrInvalidLoginBackdrop = errors.New("loginflow: invalid login backdrop")

const (
	loginTerrainStep        float32 = 2
	loginTerrainHeightScale float32 = .1
	loginTerrainSectorSize  float32 = 128
)

// LoginBackdrop owns the visual terrain and camera timeline shared by the
// Server and Login scenes. Scene transitions must not reset elapsed time.
type LoginBackdrop struct {
	geometry  graphics.MeshGeometry
	transform graphics.SceneTransform
	action    assets.CameraAction
	elapsed   time.Duration
}

func NewLoginBackdrop(terrain assets.Terrain, action assets.CameraAction) (*LoginBackdrop, error) {
	geometry, transform, err := loginTerrainGeometry(terrain)
	if err != nil {
		return nil, err
	}
	if len(action.Ticks) == 0 {
		return nil, fmt.Errorf("%w: camera action has no keyframes", ErrInvalidLoginBackdrop)
	}
	ticks := append([]assets.CameraActionTick(nil), action.Ticks...)
	return &LoginBackdrop{
		geometry:  geometry,
		transform: transform,
		action:    assets.CameraAction{Ticks: ticks},
	}, nil
}

func loginTerrainGeometry(terrain assets.Terrain) (graphics.MeshGeometry, graphics.SceneTransform, error) {
	if terrain.Columns < 2 || terrain.Rows < 2 || len(terrain.Cells) != terrain.Columns*terrain.Rows {
		return graphics.MeshGeometry{}, graphics.SceneTransform{}, fmt.Errorf("%w: invalid terrain dimensions", ErrInvalidLoginBackdrop)
	}
	vertexCount := terrain.Columns * terrain.Rows
	if vertexCount > int(^uint16(0))+1 {
		return graphics.MeshGeometry{}, graphics.SceneTransform{}, fmt.Errorf("%w: terrain has %d vertices", ErrInvalidLoginBackdrop, vertexCount)
	}

	vertices := make([]graphics.MeshVertex, vertexCount)
	for y := 0; y < terrain.Rows; y++ {
		for x := 0; x < terrain.Columns; x++ {
			cell := terrain.Cells[y*terrain.Columns+x]
			vertices[y*terrain.Columns+x] = graphics.MeshVertex{
				Position: graphics.Position3{
					X: float32(x) * loginTerrainStep,
					Y: float32(cell.Height) * loginTerrainHeightScale,
					Z: float32(y) * loginTerrainStep,
				},
				Normal: graphics.Position3{Y: 1},
			}
		}
	}

	indices := make([]uint16, 0, (terrain.Columns-1)*(terrain.Rows-1)*6)
	for y := 0; y < terrain.Rows-1; y++ {
		for x := 0; x < terrain.Columns-1; x++ {
			topLeft := uint16(y*terrain.Columns + x)
			topRight := topLeft + 1
			bottomLeft := uint16((y+1)*terrain.Columns + x)
			bottomRight := bottomLeft + 1
			indices = append(indices,
				topLeft, bottomLeft, topRight,
				topRight, bottomLeft, bottomRight,
			)
		}
	}

	transform := graphics.SceneTransform{
		Position: graphics.Position3{
			X: float32(terrain.HeaderWidth) * loginTerrainSectorSize,
			Z: float32(terrain.HeaderHeight) * loginTerrainSectorSize,
		},
		Scale: 1,
	}
	return graphics.MeshGeometry{Vertices: vertices, Indices: indices}, transform, nil
}

func loginBackdropCamera(pose assets.CameraActionPose) graphics.Camera {
	horizon := float64(pose.HorizonAngle)
	vertical := float64(pose.VerticalAngle)
	cosVertical := math.Cos(vertical)
	direction := graphics.Position3{
		X: float32(math.Cos(horizon) * cosVertical),
		Y: float32(math.Sin(vertical)),
		Z: float32(math.Sin(horizon) * cosVertical),
	}
	position := graphics.Position3{X: pose.Position[0], Y: pose.Position[1], Z: pose.Position[2]}
	return graphics.Camera{
		Position: position,
		Target: graphics.Position3{
			X: position.X + direction.X,
			Y: position.Y + direction.Y,
			Z: position.Z + direction.Z,
		},
		Up: graphics.Position3{Y: 1},
	}
}

func (b *LoginBackdrop) Advance(dt time.Duration) error {
	if b == nil {
		return ErrInvalidLoginBackdrop
	}
	if dt < 0 {
		return fmt.Errorf("%w: negative elapsed delta", ErrInvalidLoginBackdrop)
	}
	b.elapsed += dt
	return nil
}

func (b *LoginBackdrop) Render(renderer graphics.Renderer) error {
	if b == nil {
		return ErrInvalidLoginBackdrop
	}
	sceneRenderer, ok := renderer.(graphics.SceneGeometryRenderer)
	if !ok {
		return graphics.ErrNotImplemented
	}
	pose, ok := b.action.Pose(b.elapsed, [3]float32{})
	if !ok {
		return fmt.Errorf("%w: camera action is empty", ErrInvalidLoginBackdrop)
	}
	return sceneRenderer.DrawSceneGeometry(b.geometry, b.transform, loginBackdropCamera(pose))
}
