package loginflow

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
)

func TestSceneFactoriesValidatePhaseOnEnterAndUpdate(t *testing.T) {
	state := login.NewSessionState()
	factories := SceneFactories(state)
	scene, err := factories[LoginSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := scene.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := scene.Update(time.Millisecond); err != nil {
		t.Fatal(err)
	}
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if err := scene.Update(0); err != nil {
		t.Fatal(err)
	}
	_ = scene.Close()
}

func TestVisualLoginFactoryUsesControlsAndSubmitCallback(t *testing.T) {
	state := login.NewSessionState()
	calls := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{Authenticate: func(string, []byte) error { calls++; return nil }})
	s, err := factories[LoginSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	_ = s.HandleEvent(input.Event{Kind: input.KindText, Rune: 'a'})
	_ = s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x09})
	_ = s.HandleEvent(input.Event{Kind: input.KindText, Rune: 'b'})
	if err := s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err != nil {
		t.Fatal(err)
	}
	if calls != 1 {
		t.Fatalf("submit calls=%d want 1", calls)
	}
	_ = s.Close()
}

func TestServerSelectionSelectsRowBeforeConnecting(t *testing.T) {
	state := login.NewSessionState()
	calls := 0
	var selected ServerEntry
	factories := SceneFactoriesWithVisuals(state, VisualOptions{
		Servers:      []ServerEntry{{Name: "Alpha", Address: "127.0.0.1:8281"}},
		SelectServer: func(entry ServerEntry) error { calls++; selected = entry; return nil },
	})
	s, err := factories[ServerSelectionSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	layout := serverSelectionLayoutFor(nil, 1)
	row := layout.serverRow(0)
	if err := s.HandleEvent(input.Event{Kind: input.KindMouseButtonDown, Button: 1, X: row.X + 1, Y: row.Y + 1}); err != nil {
		t.Fatal(err)
	}
	if calls != 0 {
		t.Fatalf("row click connected immediately: calls=%d", calls)
	}
	if err := s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err != nil {
		t.Fatal(err)
	}
	if calls != 1 || selected.Address != "127.0.0.1:8281" {
		t.Fatalf("selection=%+v calls=%d", selected, calls)
	}
	_ = s.Close()
}

func TestServerSelectionLayoutCentersNativeRootAndSkin(t *testing.T) {
	for _, test := range []struct {
		width, height int32
		rootX, rootY  int32
	}{
		{width: 800, height: 600, rootX: 243, rootY: 172},
		{width: 1024, height: 768, rootX: 355, rootY: 256},
		{width: 1280, height: 960, rootX: 483, rootY: 352},
	} {
		layout := serverSelectionLayoutFor(viewportProbe{width: test.width, height: test.height}, 2)
		if layout.root.X != test.rootX || layout.root.Y != test.rootY {
			t.Fatalf("viewport %dx%d root=%+v, want (%d,%d)", test.width, test.height, layout.root, test.rootX, test.rootY)
		}
		if layout.texture.X != layout.root.X+28 || layout.texture.Y != layout.root.Y {
			t.Fatalf("viewport %dx%d texture=%+v not anchored to root=%+v", test.width, test.height, layout.texture, layout.root)
		}
		if !layout.connect.Contains(layout.connect.X+1, layout.connect.Y+1) {
			t.Fatalf("connect control does not contain its own anchor: %+v", layout.connect)
		}
		if !layout.close.Contains(layout.close.X+1, layout.close.Y+1) {
			t.Fatalf("close control does not contain its own anchor: %+v", layout.close)
		}
	}
}

func TestServerSelectionCloseRequestsApplicationShutdown(t *testing.T) {
	state := login.NewSessionState()
	requested := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{
		Servers:      []ServerEntry{{Name: "Alpha", Address: "127.0.0.1:8281"}},
		RequestClose: func() error { requested++; return nil },
	})
	s, err := factories[ServerSelectionSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	layout := serverSelectionLayoutFor(nil, 1)
	if err := s.HandleEvent(input.Event{Kind: input.KindMouseButtonDown, Button: 1, X: layout.close.X + 1, Y: layout.close.Y + 1}); err != nil {
		t.Fatal(err)
	}
	if requested != 1 {
		t.Fatalf("close requests=%d want 1", requested)
	}
	_ = s.Close()
}

func TestServerSelectionLayoutKeepsRowsAndHitboxesTogether(t *testing.T) {
	layout := serverSelectionLayoutFor(viewportProbe{width: 1024, height: 768}, 3)
	first, second := layout.serverRow(0), layout.serverRow(1)
	if first.X != second.X || second.Y-first.Y != 27 {
		t.Fatalf("rows drifted from one root: first=%+v second=%+v", first, second)
	}
	if !first.Contains(first.X+first.Width/2, first.Y+first.Height/2) {
		t.Fatal("first row hitbox does not contain its visual center")
	}
}

type viewportProbe struct{ width, height int32 }

func (p viewportProbe) Initialize(uintptr) error                            { return nil }
func (p viewportProbe) BeginFrame()                                         {}
func (p viewportProbe) EndFrame()                                           {}
func (p viewportProbe) Close() error                                        { return nil }
func (p viewportProbe) DrawRect(int32, int32, int32, int32, graphics.Color) {}
func (p viewportProbe) ClientViewport() (int32, int32)                      { return p.width, p.height }

func TestServerSelectionRejectsMissingEndpoint(t *testing.T) {
	state := login.NewSessionState()
	calls := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{
		Servers:      []ServerEntry{{Name: "Offline", Address: ""}},
		SelectServer: func(ServerEntry) error { calls++; return nil },
	})
	s, err := factories[ServerSelectionSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err == nil {
		t.Fatal("missing endpoint accepted")
	}
	if calls != 0 {
		t.Fatalf("callback calls=%d want 0", calls)
	}
	_ = s.Close()
}

func TestServerSelectionKeepsSceneAliveWhenConnectionFails(t *testing.T) {
	state := login.NewSessionState()
	calls := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{
		Servers: []ServerEntry{{Name: "Alpha", Address: "127.0.0.1:8281"}},
		SelectServer: func(ServerEntry) error {
			calls++
			return errors.New("server unavailable")
		},
	})
	s, err := factories[ServerSelectionSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err != nil {
		t.Fatalf("recoverable connection failure escaped scene: %v", err)
	}
	selector := s.(*serverSelectionScene)
	if calls != 1 || selector.status != "Unable to connect to the selected server." {
		t.Fatalf("calls=%d status=%q", calls, selector.status)
	}
	_ = s.Close()
}

func TestCharacterSceneRejectsIncompatiblePhase(t *testing.T) {
	state := login.NewSessionState()
	factory := SceneFactories(state)[CharacterSelectSceneID]
	s, err := factory()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err == nil {
		t.Fatal("character-select scene accepted Disconnected")
	}
	_ = s.Close()
}

func TestVisualCharacterSceneRejectsSubmitWhenAllSlotsAreEmpty(t *testing.T) {
	state := login.NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(login.AccountLoginRequest{Account: "account", Password: []byte("password")}); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptCharacterList(emptyCharacterListPacket()); err != nil {
		t.Fatal(err)
	}
	calls := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{SelectCharacter: func(int32) error {
		calls++
		return nil
	}})
	s, err := factories[CharacterSelectSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	err = s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D})
	if !errors.Is(err, errNoCharacterSelected) {
		t.Fatalf("submit error=%v want=%v", err, errNoCharacterSelected)
	}
	if calls != 0 {
		t.Fatalf("select callback calls=%d want 0", calls)
	}
	_ = s.Close()
}

func emptyCharacterListPacket() protocol.Packet {
	raw := make([]byte, login.CharacterListPacketSize)
	binary.LittleEndian.PutUint16(raw[0:2], uint16(len(raw)))
	raw[2] = 0x5A
	raw[3] = 0xA5
	binary.LittleEndian.PutUint16(raw[4:6], login.OpcodeCharacterList)
	return protocol.Packet{
		Header: protocol.Header{Size: uint16(len(raw)), KeyWord: raw[2], CheckSum: raw[3], Type: login.OpcodeCharacterList},
		Raw:    raw,
		Body:   raw[protocol.HeaderSize:],
	}
}
