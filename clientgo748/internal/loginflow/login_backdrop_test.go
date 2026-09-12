package loginflow

import (
	"math"
	"testing"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
	"wydclient748/internal/login"
)

func TestLoginTerrainGeometryUsesNativeSpacingHeightAndSectorOffset(t *testing.T) {
	terrain := assets.Terrain{
		HeaderWidth: 8, HeaderHeight: 13, Columns: 2, Rows: 2,
		Cells: []assets.TerrainCell{{Height: -10}, {Height: 0}, {Height: 10}, {Height: 20}},
	}
	geometry, transform, err := loginTerrainGeometry(terrain)
	if err != nil {
		t.Fatal(err)
	}
	if len(geometry.Vertices) != 4 || len(geometry.Indices) != 6 {
		t.Fatalf("geometry vertices=%d indices=%d", len(geometry.Vertices), len(geometry.Indices))
	}
	wantPositions := []graphics.Position3{
		{X: 0, Y: -1, Z: 0},
		{X: 2, Y: 0, Z: 0},
		{X: 0, Y: 1, Z: 2},
		{X: 2, Y: 2, Z: 2},
	}
	for i, want := range wantPositions {
		if got := geometry.Vertices[i].Position; got != want {
			t.Fatalf("vertex %d=%+v want %+v", i, got, want)
		}
	}
	wantIndices := []uint16{0, 2, 1, 1, 2, 3}
	for i, want := range wantIndices {
		if geometry.Indices[i] != want {
			t.Fatalf("index %d=%d want %d", i, geometry.Indices[i], want)
		}
	}
	if transform.Position != (graphics.Position3{X: 1024, Z: 1664}) || transform.Scale != 1 {
		t.Fatalf("terrain transform=%+v", transform)
	}
}

func TestLoginBackdropCameraUsesNativeHorizonAndVerticalDirection(t *testing.T) {
	pose := assets.CameraActionPose{
		Position:      [3]float32{10, 20, 30},
		HorizonAngle:  float32(math.Pi / 2),
		VerticalAngle: float32(math.Pi / 6),
	}
	camera := loginBackdropCamera(pose)
	if camera.Position != (graphics.Position3{X: 10, Y: 20, Z: 30}) {
		t.Fatalf("camera position=%+v", camera.Position)
	}
	if math.Abs(float64(camera.Target.X-10)) > 1e-5 ||
		math.Abs(float64(camera.Target.Y-20.5)) > 1e-5 ||
		math.Abs(float64(camera.Target.Z-(30+float32(math.Cos(math.Pi/6))))) > 1e-5 {
		t.Fatalf("camera target=%+v", camera.Target)
	}
	if camera.Up != (graphics.Position3{Y: 1}) {
		t.Fatalf("camera up=%+v", camera.Up)
	}
}

func TestLoginBackdropTimelinePersistsAcrossServerAndLoginScenes(t *testing.T) {
	backdrop := newTestLoginBackdrop(t)
	state := login.NewSessionState()
	renderer := &loginBackdropRenderProbe{}
	factories := SceneFactoriesWithVisuals(state, VisualOptions{
		ShapeRenderer: renderer,
		LoginBackdrop: backdrop,
		Servers:       []ServerEntry{{Name: "Alpha", Address: "127.0.0.1:8281"}},
	})
	serverScene, err := factories[ServerSelectionSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	loginScene, err := factories[LoginSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := serverScene.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := serverScene.Update(900 * time.Millisecond); err != nil {
		t.Fatal(err)
	}
	if err := serverScene.Exit(); err != nil {
		t.Fatal(err)
	}
	if err := loginScene.Enter(); err != nil {
		t.Fatal(err)
	}
	if backdrop.elapsed != 900*time.Millisecond {
		t.Fatalf("login Enter reset backdrop elapsed=%s", backdrop.elapsed)
	}
	if err := loginScene.Update(100 * time.Millisecond); err != nil {
		t.Fatal(err)
	}
	if backdrop.elapsed != time.Second {
		t.Fatalf("shared backdrop elapsed=%s want 1s", backdrop.elapsed)
	}
}

func TestLoginBackdropRendersBeforeServerAndLoginUI(t *testing.T) {
	backdrop := newTestLoginBackdrop(t)
	state := login.NewSessionState()
	renderer := &loginBackdropRenderProbe{}
	factories := SceneFactoriesWithVisuals(state, VisualOptions{
		ShapeRenderer: renderer,
		LoginBackdrop: backdrop,
		Servers:       []ServerEntry{{Name: "Alpha", Address: "127.0.0.1:8281"}},
	})

	serverScene, err := factories[ServerSelectionSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := serverScene.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := serverScene.Render(); err != nil {
		t.Fatal(err)
	}
	if len(renderer.order) < 2 || renderer.order[0] != "backdrop" {
		t.Fatalf("server render order=%v", renderer.order)
	}

	renderer.order = nil
	loginScene, err := factories[LoginSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := loginScene.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := loginScene.Render(); err != nil {
		t.Fatal(err)
	}
	if len(renderer.order) < 2 || renderer.order[0] != "backdrop" {
		t.Fatalf("login render order=%v", renderer.order)
	}
}

func newTestLoginBackdrop(t *testing.T) *LoginBackdrop {
	t.Helper()
	backdrop, err := NewLoginBackdrop(assets.Terrain{
		HeaderWidth: 8, HeaderHeight: 13, Columns: 2, Rows: 2,
		Cells: []assets.TerrainCell{{Height: 0}, {Height: 0}, {Height: 0}, {Height: 0}},
	}, assets.CameraAction{Ticks: []assets.CameraActionTick{
		{Tick: 0, X: 1025, Y: 1, Z: 1665, HorizonAngle: 0, VerticalAngle: 0},
		{Tick: 1000, X: 1026, Y: 1, Z: 1665, HorizonAngle: 0, VerticalAngle: 0},
	}})
	if err != nil {
		t.Fatal(err)
	}
	return backdrop
}

type loginBackdropRenderProbe struct {
	order []string
}

func (*loginBackdropRenderProbe) Initialize(uintptr) error { return nil }
func (*loginBackdropRenderProbe) BeginFrame()              {}
func (*loginBackdropRenderProbe) EndFrame()                {}
func (*loginBackdropRenderProbe) Close() error             { return nil }
func (r *loginBackdropRenderProbe) DrawRect(int32, int32, int32, int32, graphics.Color) {
	r.order = append(r.order, "rect")
}
func (r *loginBackdropRenderProbe) DrawText(int32, int32, string, int32, graphics.Color) {
	r.order = append(r.order, "text")
}
func (r *loginBackdropRenderProbe) DrawSceneGeometry(graphics.MeshGeometry, graphics.SceneTransform, graphics.Camera) error {
	r.order = append(r.order, "backdrop")
	return nil
}
