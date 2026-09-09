package app

import (
	"context"
	"errors"
	"reflect"
	"testing"
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
func (w *fakeWindow) PollEvents() {
	*w.events = append(*w.events, "window.poll")
	w.polls++
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
