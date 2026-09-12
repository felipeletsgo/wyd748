package app

import (
	"context"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
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

type fakeSession struct {
	events     *[]string
	connectErr error
	closeErr   error
	connected  bool
	closed     bool
}

type fakeEventSession struct {
	fakeSession
	startErr    error
	startCalls  int
	sessionData []protocol.SessionEvent
	drainLimits []int
}

func (s *fakeEventSession) StartReceiving() error {
	*s.events = append(*s.events, "session.start")
	s.startCalls++
	return s.startErr
}

func (s *fakeEventSession) DrainEvents(limit int) []protocol.SessionEvent {
	*s.events = append(*s.events, "session.drain")
	s.drainLimits = append(s.drainLimits, limit)
	if limit <= 0 || len(s.sessionData) == 0 {
		return nil
	}
	if limit > len(s.sessionData) {
		limit = len(s.sessionData)
	}
	drained := append([]protocol.SessionEvent(nil), s.sessionData[:limit]...)
	s.sessionData = s.sessionData[limit:]
	return drained
}

func (s *fakeSession) Connect() error {
	*s.events = append(*s.events, "session.connect")
	if s.connectErr != nil {
		return s.connectErr
	}
	s.connected = true
	return nil
}

func (s *fakeSession) Close() error {
	*s.events = append(*s.events, "session.close")
	s.closed = true
	return s.closeErr
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

func TestApplicationOwnsSessionAndClosesItBeforeSceneResources(t *testing.T) {
	var events []string
	session := &fakeSession{events: &events}
	window := &fakeWindow{events: &events, pollsUntilClose: 1}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{
		Title:   "WYD 7.48",
		Width:   800,
		Height:  600,
		Session: session,
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	if !session.connected || !session.closed {
		t.Fatalf("session lifecycle connected=%v closed=%v, want both true", session.connected, session.closed)
	}
	want := []string{
		"window.open",
		"renderer.initialize",
		"session.connect",
		"window.poll",
		"session.close",
		"renderer.close",
		"window.close",
	}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events = %v, want %v", events, want)
	}
}

func TestApplicationBeginsAndClearsLoginStateAroundConnectedSession(t *testing.T) {
	var events []string
	state := login.NewSessionState()
	session := &fakeSession{events: &events}
	application, err := New(Options{
		Title:   "WYD 7.48",
		Width:   800,
		Height:  600,
		Session: session,
		SessionConnected: func() error {
			events = append(events, "session.connected")
			return state.BeginConnect()
		},
		SessionDisconnected: func() {
			events = append(events, "session.disconnected")
			state.Disconnect()
		},
	}, &fakeWindow{events: &events, pollsUntilClose: 1}, &fakeRenderer{events: &events})
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	if state.Phase() != login.Disconnected {
		t.Fatalf("final login phase=%s want Disconnected", state.Phase())
	}
	want := []string{
		"window.open", "renderer.initialize", "session.connect", "session.connected",
		"window.poll", "session.close", "session.disconnected", "renderer.close", "window.close",
	}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events=%v want %v", events, want)
	}
}

func TestApplicationDefersSessionConnectUntilExplicitCall(t *testing.T) {
	var events []string
	session := &fakeSession{events: &events}
	application, err := New(Options{
		Title: "WYD 7.48", Width: 800, Height: 600,
		Session: session, DeferSessionConnect: true,
	}, &fakeWindow{events: &events, pollsUntilClose: 1}, &fakeRenderer{events: &events})
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	for _, event := range events {
		if event == "session.connect" {
			t.Fatalf("deferred session connected during Run: %v", events)
		}
	}
}

func TestApplicationConnectionFailureDoesNotPublishConnectedState(t *testing.T) {
	var events []string
	connectErr := errors.New("connect failed")
	connectedCalls := 0
	disconnectedCalls := 0
	application, err := New(Options{
		Title:   "WYD 7.48",
		Width:   800,
		Height:  600,
		Session: &fakeSession{events: &events, connectErr: connectErr},
		SessionConnected: func() error {
			connectedCalls++
			return nil
		},
		SessionDisconnected: func() {
			disconnectedCalls++
		},
	}, &fakeWindow{events: &events}, &fakeRenderer{events: &events})
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, connectErr) {
		t.Fatalf("Run error=%v want connect failure", err)
	}
	if connectedCalls != 0 || disconnectedCalls != 0 {
		t.Fatalf("lifecycle calls connected=%d disconnected=%d want zero", connectedCalls, disconnectedCalls)
	}
}

func TestApplicationReceiveFailureClearsPublishedLoginState(t *testing.T) {
	var events []string
	state := login.NewSessionState()
	startErr := errors.New("receiver failed")
	session := &fakeEventSession{
		fakeSession: fakeSession{events: &events},
		startErr:    startErr,
	}
	application, err := New(Options{
		Title:               "WYD 7.48",
		Width:               800,
		Height:              600,
		Session:             session,
		SessionEventHandler: func(protocol.SessionEvent) error { return nil },
		SessionConnected:    state.BeginConnect,
		SessionDisconnected: state.Disconnect,
	}, &fakeWindow{events: &events}, &fakeRenderer{events: &events})
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, startErr) {
		t.Fatalf("Run error=%v want receiver failure", err)
	}
	if state.Phase() != login.Disconnected {
		t.Fatalf("phase=%s want Disconnected after receiver failure", state.Phase())
	}
}

func TestApplicationReconnectsAfterRemoteDisconnect(t *testing.T) {
	var events []string
	state := login.NewSessionState()
	session := &fakeEventSession{fakeSession: fakeSession{events: &events}}
	a, err := New(Options{
		Title: "test", Width: 800, Height: 600, Session: session,
		SessionConnected: state.BeginConnect,
		SessionEventHandler: func(event protocol.SessionEvent) error {
			if event.Kind == protocol.SessionDisconnected {
				state.Disconnect()
			}
			return nil
		},
	}, &fakeWindow{events: &events}, &fakeRenderer{events: &events})
	if err != nil {
		t.Fatal(err)
	}
	for attempt := 0; attempt < 3; attempt++ {
		if err := a.ConnectSession(); err != nil {
			t.Fatal(err)
		}
		if state.Phase() != login.Connecting {
			t.Fatalf("attempt %d: phase=%s", attempt, state.Phase())
		}
		session.sessionData = []protocol.SessionEvent{{Kind: protocol.SessionDisconnected}}
		if err := a.dispatchSessionEvents(); err != nil {
			t.Fatal(err)
		}
	}
	if session.startCalls != 3 {
		t.Fatalf("receiver starts=%d want 3", session.startCalls)
	}
}

func TestApplicationSessionLifecycleCallbacksRequireSession(t *testing.T) {
	var events []string
	_, err := New(Options{
		Title:            "WYD 7.48",
		Width:            800,
		Height:           600,
		SessionConnected: func() error { return nil },
	}, &fakeWindow{events: &events}, &fakeRenderer{events: &events})
	if err == nil {
		t.Fatal("New accepted lifecycle callbacks without a session")
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

func TestApplicationDispatchesSessionEventsOnFrameBeforeSceneUpdate(t *testing.T) {
	var events []string
	session := &fakeEventSession{
		fakeSession: fakeSession{events: &events},
		sessionData: []protocol.SessionEvent{
			{Kind: protocol.SessionPacket, Packet: protocol.Packet{Header: protocol.Header{Type: 0x10A}}},
			{Kind: protocol.SessionPacket, Packet: protocol.Packet{Header: protocol.Header{Type: 0x114}}},
		},
	}
	window := &fakeWindow{events: &events, pollsUntilClose: 2}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{
		Title:                 "WYD 7.48",
		Width:                 800,
		Height:                600,
		Session:               session,
		SessionEventsPerFrame: 1,
		SessionEventHandler: func(event protocol.SessionEvent) error {
			events = append(events, "session.handle")
			return nil
		},
		InitialSceneID: "login",
		InitialScene: func() (scene.Scene, error) {
			return &applicationScene{id: "login", events: &events}, nil
		},
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	if session.startCalls != 1 {
		t.Fatalf("StartReceiving calls=%d, want 1", session.startCalls)
	}
	if !reflect.DeepEqual(session.drainLimits, []int{1, 1}) {
		t.Fatalf("drain limits=%v, want [1 1]", session.drainLimits)
	}
	firstHandle := indexOf(events, "session.handle")
	firstUpdate := indexOf(events, "scene.update")
	if firstHandle < 0 || firstUpdate < 0 || firstHandle >= firstUpdate {
		t.Fatalf("session event was not handled before scene update: %v", events)
	}
	if len(session.sessionData) != 0 {
		t.Fatalf("undispatched session events=%+v", session.sessionData)
	}
}

func TestApplicationRunsSceneSynchronizerAfterInputBeforeUpdate(t *testing.T) {
	var events []string
	window := &fakeWindow{events: &events, pollsUntilClose: 2}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{
		Title: "WYD 7.48", Width: 800, Height: 600,
		InitialSceneID: "login",
		InitialScene: func() (scene.Scene, error) {
			return &applicationScene{id: "login", events: &events}, nil
		},
		SceneSynchronizer: func() error {
			events = append(events, "scene.sync")
			return nil
		},
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	if err := application.Run(context.Background()); err != nil {
		t.Fatal(err)
	}
	sync := indexOf(events, "scene.sync")
	update := indexOf(events, "scene.update")
	if sync < 0 || update < 0 || sync >= update {
		t.Fatalf("synchronizer did not run before scene update: %v", events)
	}
}

func TestApplicationSessionHandlerFailurePropagatesAndClosesBeforeScenes(t *testing.T) {
	var events []string
	handlerErr := errors.New("dispatch failed")
	session := &fakeEventSession{
		fakeSession: fakeSession{events: &events},
		sessionData: []protocol.SessionEvent{{Kind: protocol.SessionPacket}},
	}
	window := &fakeWindow{events: &events, pollsUntilClose: 2}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{
		Title:   "WYD 7.48",
		Width:   800,
		Height:  600,
		Session: session,
		SessionEventHandler: func(protocol.SessionEvent) error {
			events = append(events, "session.handle")
			return handlerErr
		},
		InitialSceneID: "login",
		InitialScene: func() (scene.Scene, error) {
			return &applicationScene{id: "login", events: &events}, nil
		},
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, handlerErr) {
		t.Fatalf("Run error=%v, want handler error", err)
	}
	closePosition := indexOf(events, "session.close")
	exitPosition := indexOf(events, "scene.exit")
	if closePosition < 0 || exitPosition < 0 || closePosition >= exitPosition {
		t.Fatalf("session receiver was not closed before scenes: %v", events)
	}
}

func TestApplicationSessionHandlerRequiresEventSession(t *testing.T) {
	var events []string
	_, err := New(Options{
		Title:               "WYD 7.48",
		Width:               800,
		Height:              600,
		Session:             &fakeSession{events: &events},
		SessionEventHandler: func(protocol.SessionEvent) error { return nil },
	}, &fakeWindow{events: &events}, &fakeRenderer{events: &events})
	if err == nil {
		t.Fatal("New accepted a session event handler without EventSession")
	}
}

func TestApplicationReceiveStartFailureRollsBackOwnedResources(t *testing.T) {
	var events []string
	startErr := errors.New("receiver failed")
	session := &fakeEventSession{
		fakeSession: fakeSession{events: &events},
		startErr:    startErr,
	}
	window := &fakeWindow{events: &events}
	renderer := &fakeRenderer{events: &events}
	application, err := New(Options{
		Title:               "WYD 7.48",
		Width:               800,
		Height:              600,
		Session:             session,
		SessionEventHandler: func(protocol.SessionEvent) error { return nil },
	}, window, renderer)
	if err != nil {
		t.Fatal(err)
	}
	err = application.Run(context.Background())
	if !errors.Is(err, startErr) {
		t.Fatalf("Run error=%v, want start error", err)
	}
	want := []string{
		"window.open", "renderer.initialize", "session.connect", "session.start",
		"session.close", "renderer.close", "window.close",
	}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events=%v, want %v", events, want)
	}
}

func indexOf(events []string, target string) int {
	for index, event := range events {
		if event == target {
			return index
		}
	}
	return -1
}
