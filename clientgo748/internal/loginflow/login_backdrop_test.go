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
		Cells: []assets.TerrainCell{{Height: -10, PrimaryUVSelector: 0, PrimaryMaterialIndex: 28}, {Height: 0}, {Height: 10}, {Height: 20}},
	}
	materials, transform, err := loginTerrainGeometry(terrain)
	if err != nil {
		t.Fatal(err)
	}
	if len(materials) != 1 || materials[0].primarySlot != 38 || materials[0].secondarySlot != 256 {
		t.Fatalf("materials=%+v", materials)
	}
	geometry := materials[0].geometry
	if len(geometry.Vertices) != 4 || len(geometry.Indices) != 6 {
		t.Fatalf("geometry vertices=%d indices=%d", len(geometry.Vertices), len(geometry.Indices))
	}
	wantPositions := []graphics.Position3{
		{X: 0, Y: -1, Z: 0},
		{X: 0, Y: 1, Z: 2},
		{X: 2, Y: 0, Z: 0},
		{X: 2, Y: 2, Z: 2},
	}
	wantUV := []graphics.TexCoord2{{U: 0, V: 0}, {U: 0, V: 1}, {U: 1, V: 0}, {U: 1, V: 1}}
	wantSecondaryUV := []graphics.TexCoord2{{U: 0, V: 0}, {U: 0, V: .5}, {U: .5, V: 0}, {U: .5, V: .5}}
	for i, want := range wantPositions {
		if got := geometry.Vertices[i].Position; got != want {
			t.Fatalf("vertex %d=%+v want %+v", i, got, want)
		}
		if got := geometry.Vertices[i].TexCoord; got != wantUV[i] || !geometry.Vertices[i].HasTexCoord {
			t.Fatalf("vertex %d uv=%+v textured=%v want %+v", i, got, geometry.Vertices[i].HasTexCoord, wantUV[i])
		}
		if got := geometry.Vertices[i].SecondaryTexCoord; got != wantSecondaryUV[i] || !geometry.Vertices[i].HasSecondaryTexCoord {
			t.Fatalf("vertex %d secondary uv=%+v textured=%v want %+v", i, got, geometry.Vertices[i].HasSecondaryTexCoord, wantSecondaryUV[i])
		}
	}
	wantIndices := []uint16{0, 1, 2, 2, 1, 3}
	for i, want := range wantIndices {
		if geometry.Indices[i] != want {
			t.Fatalf("index %d=%d want %d", i, geometry.Indices[i], want)
		}
	}
	if transform.Position != (graphics.Position3{X: 1024, Z: 1664}) || transform.Scale != 1 {
		t.Fatalf("terrain transform=%+v", transform)
	}
}

func TestLoginTerrainGeometryGroupsCellsByMaterialPairAndKeepsPerCellUVs(t *testing.T) {
	terrain := assets.Terrain{
		Columns: 3, Rows: 2,
		Cells: []assets.TerrainCell{
			{PrimaryMaterialIndex: 28, PrimaryUVSelector: 0, SecondaryMaterialIndex: 2, SecondaryUVSelector: 0},
			{PrimaryMaterialIndex: 28, PrimaryUVSelector: 7, SecondaryMaterialIndex: 3, SecondaryUVSelector: 1},
			{}, {}, {}, {},
		},
	}
	materials, _, err := loginTerrainGeometry(terrain)
	if err != nil {
		t.Fatal(err)
	}
	if len(materials) != 2 || materials[0].primarySlot != 38 || materials[0].secondarySlot != 258 || materials[1].primarySlot != 38 || materials[1].secondarySlot != 259 {
		t.Fatalf("material slots=%+v", materials)
	}
	for i := range materials {
		if len(materials[i].geometry.Vertices) != 4 || len(materials[i].geometry.Indices) != 6 {
			t.Fatalf("material %d geometry=%+v", i, materials[i].geometry)
		}
	}
	wantSecond := []graphics.TexCoord2{{U: 1, V: 0}, {U: 1, V: 1}, {U: 0, V: 0}, {U: 0, V: 1}}
	for i, want := range wantSecond {
		if got := materials[1].geometry.Vertices[i].TexCoord; got != want {
			t.Fatalf("second material vertex %d uv=%+v want %+v", i, got, want)
		}
	}
	wantSecondSecondary := []graphics.TexCoord2{{U: 1, V: 0}, {U: .5, V: 0}, {U: 1, V: .5}, {U: .5, V: .5}}
	for i, want := range wantSecondSecondary {
		if got := materials[1].geometry.Vertices[i].SecondaryTexCoord; got != want {
			t.Fatalf("second material vertex %d secondary uv=%+v want %+v", i, got, want)
		}
	}
}

func TestLoginBackdropMaterialSlotsAndTextureValidation(t *testing.T) {
	backdrop, err := NewLoginBackdrop(assets.Terrain{
		Columns: 3, Rows: 2,
		Cells: []assets.TerrainCell{
			{PrimaryMaterialIndex: 28, SecondaryMaterialIndex: 2},
			{PrimaryMaterialIndex: 28, SecondaryMaterialIndex: 3},
			{}, {}, {}, {},
		},
	}, assets.CameraAction{Ticks: []assets.CameraActionTick{{Tick: 0}}})
	if err != nil {
		t.Fatal(err)
	}
	if got := backdrop.MaterialSlots(); len(got) != 3 || got[0] != 38 || got[1] != 258 || got[2] != 259 {
		t.Fatalf("material slots=%v", got)
	}
	texture := assets.Texture{Width: 1, Height: 1, Pixels: []byte{255, 255, 255, 255}}
	if err := backdrop.SetTerrainTextures(map[uint16]assets.Texture{38: texture, 258: texture}); err == nil {
		t.Fatal("missing material texture was accepted")
	}
	if err := backdrop.SetTerrainTextures(map[uint16]assets.Texture{38: texture, 258: texture, 259: texture}); err != nil {
		t.Fatal(err)
	}
}

func TestLoginBackdropTerrainRendererUploadsEachSlotOnceAndDrawsMaterialPairs(t *testing.T) {
	backdrop, err := NewLoginBackdrop(assets.Terrain{
		Columns: 3, Rows: 2,
		Cells: []assets.TerrainCell{
			{PrimaryMaterialIndex: 28, SecondaryMaterialIndex: 2},
			{PrimaryMaterialIndex: 28, SecondaryMaterialIndex: 3},
			{}, {}, {}, {},
		},
	}, assets.CameraAction{Ticks: []assets.CameraActionTick{{Tick: 0}}})
	if err != nil {
		t.Fatal(err)
	}
	texture := assets.Texture{Width: 1, Height: 1, Pixels: []byte{255, 255, 255, 255}}
	if err := backdrop.SetTerrainTextures(map[uint16]assets.Texture{38: texture, 258: texture, 259: texture}); err != nil {
		t.Fatal(err)
	}
	renderer := &loginBackdropTerrainProbe{}
	if err := backdrop.Render(renderer); err != nil {
		t.Fatal(err)
	}
	if err := backdrop.Render(renderer); err != nil {
		t.Fatal(err)
	}
	if len(renderer.uploads) != 3 || renderer.uploads[0] != 38 || renderer.uploads[1] != 258 || renderer.uploads[2] != 259 {
		t.Fatalf("terrain uploads=%v", renderer.uploads)
	}
	wantDraws := [][2]uint16{{38, 258}, {38, 259}, {38, 258}, {38, 259}}
	if len(renderer.draws) != len(wantDraws) {
		t.Fatalf("terrain draws=%v", renderer.draws)
	}
	for i, want := range wantDraws {
		if renderer.draws[i] != want {
			t.Fatalf("terrain draw %d=%v want %v", i, renderer.draws[i], want)
		}
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
		Cells: []assets.TerrainCell{{Height: 0, PrimaryUVSelector: 0}, {Height: 0}, {Height: 0}, {Height: 0}},
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

type loginBackdropTerrainProbe struct {
	uploads []uint16
	draws   [][2]uint16
}

func (*loginBackdropTerrainProbe) Initialize(uintptr) error { return nil }
func (*loginBackdropTerrainProbe) BeginFrame()              {}
func (*loginBackdropTerrainProbe) EndFrame()                {}
func (*loginBackdropTerrainProbe) Close() error             { return nil }
func (*loginBackdropTerrainProbe) DrawRect(int32, int32, int32, int32, graphics.Color) {
}
func (*loginBackdropTerrainProbe) DrawText(int32, int32, string, int32, graphics.Color) {
}
func (r *loginBackdropTerrainProbe) UploadTerrainTexture(slot uint16, _ assets.Texture) error {
	r.uploads = append(r.uploads, slot)
	return nil
}
func (r *loginBackdropTerrainProbe) DrawTerrainGeometry(primarySlot, secondarySlot uint16, _ graphics.MeshGeometry, _ graphics.SceneTransform, _ graphics.Camera) error {
	r.draws = append(r.draws, [2]uint16{primarySlot, secondarySlot})
	return nil
}
