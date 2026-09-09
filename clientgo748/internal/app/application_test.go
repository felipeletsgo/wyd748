package app

import (
	"context"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/input"
	"wydclient748/internal/scene"
)

type fakeWindow struct {
	events          *[]string
	openErr         error
	closeErr        error
	pollsUntilClose int
	polls           int
}

func (w *fakeWindow) Open(string, int, int) error {
	*w.events = append(*w.events, "window.open")
	return w.openErr
}
func (*fakeWindow) Handle() uintptr { return 42 }
func (w *fakeWindow) PollEvents() []input.Event {
	*w.events = append(*w.events, "window.poll")
	w.polls++
	return nil
}
func (w *fakeWindow) ShouldClose() bool {
	return w.pollsUntilClose > 0 && w.polls >= w.pollsUntilClose
}
func (w *fakeWindow) Close() error {
	*w.events = append(*w.events, "window.close")
	return w.closeErr
}

type fakeRenderer struct {
	events    *[]string
	initErr   error
	closeErr  error
	gotHandle uintptr
}

type fakeTextureRenderer struct {
	fakeRenderer
	uploaded  []assets.Texture
	drawCount int
}

func (r *fakeTextureRenderer) UploadTexture(texture assets.Texture) error {
	r.uploaded = append(r.uploaded, assets.Texture{
		Width: texture.Width, Height: texture.Height, SourceBits: texture.SourceBits,
		Pixels: append([]byte(nil), texture.Pixels...),
	})
	return nil
}

func (r *fakeTextureRenderer) DrawTexture() { r.drawCount++ }

type fakeTextureSource struct {
	texture assets.Texture
	path    string
	closed  bool
}

func (s *fakeTextureSource) LoadTexture(path string) (assets.Texture, error) {
	if path != s.path {
		return assets.Texture{}, errors.New("unexpected asset path")
	}
	return assets.Texture{
		Width: s.texture.Width, Height: s.texture.Height, SourceBits: s.texture.SourceBits,
		Pixels: append([]byte(nil), s.texture.Pixels...),
	}, nil
}

func (s *fakeTextureSource) Close() error {
	s.closed = true
	return nil
}

func (r *fakeRenderer) Initialize(handle uintptr) error {
	*r.events = append(*r.events, "renderer.initialize")
	r.gotHandle = handle
	return r.initErr
}
func (r *fakeRenderer) BeginFrame() { *r.events = append(*r.events, "renderer.begin") }
func (r *fakeRenderer) EndFrame()   { *r.events = append(*r.events, "renderer.end") }
func (r *fakeRenderer) Close() error {
	*r.events = append(*r.events, "renderer.close")
	return r.closeErr
}

func TestRunOwnsAndClosesResourcesInReverseOrder(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events, pollsUntilClose: 2}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{Title: "WYD 7.48", Width: 800, Height: 600}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	if renderer.gotHandle != 42 {
		t.Fatalf("renderer handle = %d, want 42", renderer.gotHandle)
	}
	want := []string{
		"window.open",
		"renderer.initialize",
		"window.poll",
		"renderer.begin",
		"renderer.end",
		"window.poll",
		"renderer.close",
		"window.close",
	}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events = %v, want %v", events, want)
	}
	if err := application.Close(); err != nil {
		t.Fatal(err)
	}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("second Close repeated teardown: %v", events)
	}
}

func TestRendererFailureClosesOnlyTheWindow(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events}
	renderer := &fakeRenderer{events: &events, initErr: errors.New("render failed")}
	application, err := New(Options{Title: "WYD 7.48", Width: 800, Height: 600}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, renderer.initErr) {
		t.Fatalf("Run error = %v, want renderer failure", err)
	}
	want := []string{"window.open", "renderer.initialize", "window.close"}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events = %v, want %v", events, want)
	}
}

func TestWindowFailureDoesNotCloseUnownedResources(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events, openErr: errors.New("window failed")}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{Title: "WYD 7.48", Width: 800, Height: 600}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, window.openErr) {
		t.Fatalf("Run error = %v, want window failure", err)
	}
	want := []string{"window.open"}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events = %v, want %v", events, want)
	}
}

func TestCloseAttemptsBothOwnedResourcesAndJoinsErrors(t *testing.T) {
	var events []string
	rendererCloseErr := errors.New("renderer close failed")
	windowCloseErr := errors.New("window close failed")
	window := &fakeWindow{events: &events, pollsUntilClose: 1, closeErr: windowCloseErr}
	renderer := &fakeRenderer{events: &events, closeErr: rendererCloseErr}
	application, err := New(Options{Title: "WYD 7.48", Width: 800, Height: 600}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, rendererCloseErr) || !errors.Is(err, windowCloseErr) {
		t.Fatalf("Run error = %v, want both close errors", err)
	}
	wantTail := []string{"renderer.close", "window.close"}
	if !reflect.DeepEqual(events[len(events)-2:], wantTail) {
		t.Fatalf("teardown tail = %v, want %v", events[len(events)-2:], wantTail)
	}
}

func TestProtectedTextureSourceIsUploadedAndClosed(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events, pollsUntilClose: 1}
	renderer := &fakeTextureRenderer{fakeRenderer: fakeRenderer{events: &events}}
	source := &fakeTextureSource{
		path:    "UI/logo1.wydasset",
		texture: assets.Texture{Width: 1, Height: 1, SourceBits: 32, Pixels: []byte{1, 2, 3, 4}},
	}
	application, err := New(Options{
		Title:         "WYD 7.48",
		Width:         800,
		Height:        600,
		LogoSource:    source,
		LogoAssetPath: source.path,
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	if len(renderer.uploaded) != 1 || !reflect.DeepEqual(renderer.uploaded[0].Pixels, source.texture.Pixels) {
		t.Fatalf("uploaded textures = %#v, want one protected texture", renderer.uploaded)
	}
	if renderer.drawCount != 0 {
		t.Fatalf("draw count = %d, want no draw after immediate close", renderer.drawCount)
	}
	if !source.closed {
		t.Fatal("application did not close the owned texture source")
	}
}

func TestProtectedTextureFailureClosesSourceBeforeWindow(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events}
	renderer := &fakeTextureRenderer{fakeRenderer: fakeRenderer{events: &events}}
	source := &fakeTextureSource{path: "UI/logo1.wydasset"}
	application, err := New(Options{
		Title:         "WYD 7.48",
		Width:         800,
		Height:        600,
		LogoSource:    source,
		LogoAssetPath: "UI/missing.wydasset",
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err == nil {
		t.Fatal("Run succeeded with a missing protected texture")
	}
	if !source.closed {
		t.Fatal("application did not close the source after texture failure")
	}
	if len(events) < 2 || events[len(events)-1] != "window.close" {
		t.Fatalf("teardown events = %v, want window close after source cleanup", events)
	}
}

type applicationScene struct {
	id     scene.ID
	events *[]string
}

func (s *applicationScene) ID() scene.ID { return s.id }
func (s *applicationScene) Enter() error {
	*s.events = append(*s.events, "scene.enter")
	return nil
}
func (s *applicationScene) HandleEvent(input.Event) error {
	*s.events = append(*s.events, "scene.event")
	return nil
}
func (s *applicationScene) Update(time.Duration) error {
	*s.events = append(*s.events, "scene.update")
	return nil
}
func (s *applicationScene) Render() error {
	*s.events = append(*s.events, "scene.render")
	return nil
}
func (s *applicationScene) Exit() error {
	*s.events = append(*s.events, "scene.exit")
	return nil
}
func (s *applicationScene) Close() error {
	*s.events = append(*s.events, "scene.close")
	return nil
}

func TestApplicationRunsSceneAndTearsItDownBeforeRenderer(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events, pollsUntilClose: 2}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{
		Title:          "WYD 7.48",
		Width:          800,
		Height:         600,
		InitialSceneID: "title",
		InitialScene: func() (scene.Scene, error) {
			return &applicationScene{id: "title", events: &events}, nil
		},
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	positions := make(map[string]int, len(events))
	for i, event := range events {
		if _, exists := positions[event]; !exists {
			positions[event] = i
		}
	}
	for _, event := range []string{"scene.enter", "scene.update", "scene.render", "scene.exit", "scene.close", "renderer.close", "window.close"} {
		if _, exists := positions[event]; !exists {
			t.Fatalf("missing lifecycle event %q; all events: %v", event, events)
		}
	}
	if !(positions["scene.enter"] < positions["scene.update"] &&
		positions["scene.update"] < positions["scene.render"] &&
		positions["scene.render"] < positions["scene.exit"] &&
		positions["scene.exit"] < positions["scene.close"] &&
		positions["scene.close"] < positions["renderer.close"] &&
		positions["renderer.close"] < positions["window.close"]) {
		t.Fatalf("invalid lifecycle order; all events: %v", events)
	}
}
