package loginflow

import (
	"errors"
	"testing"

	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
	"wydclient748/internal/scene"
)

type fakeSender struct{}

func (fakeSender) Send([]byte, byte) error { return nil }

type fakeNavigator struct {
	current  scene.ID
	requests []scene.ID
	err      error
}

func (n *fakeNavigator) CurrentScene() (scene.ID, bool) { return n.current, n.current != "" }
func (n *fakeNavigator) RequestScene(id scene.ID) error {
	if n.err != nil {
		return n.err
	}
	n.requests = append(n.requests, id)
	return nil
}

func TestCoordinatorMapsAllPhases(t *testing.T) {
	want := map[login.Phase]scene.ID{
		login.Disconnected: ServerSelectionSceneID, login.Connecting: LoginSceneID,
		login.Authenticating: LoginSceneID, login.CharacterSelect: CharacterSelectSceneID,
		login.EnteringWorld: LoadingSceneID, login.InWorld: WorldSceneID,
		login.LoggingOut: WorldSceneID,
	}
	for phase, expected := range want {
		if got := sceneForPhase(phase, false); got != expected {
			t.Errorf("phase %s maps to %q, want %q", phase, got, expected)
		}
	}
	if got := sceneForPhase(login.Disconnected, true); got != LoginSceneID {
		t.Errorf("selected disconnected phase maps to %q, want %q", got, LoginSceneID)
	}
}

func TestCoordinatorServerSelectionDefersTransportUntilAuthentication(t *testing.T) {
	state := login.NewSessionState()
	navigator := &fakeNavigator{current: ServerSelectionSceneID}
	coordinator, err := New(state, fakeSender{}, navigator, Options{})
	if err != nil {
		t.Fatal(err)
	}
	if err := coordinator.ServerSelected(); err != nil {
		t.Fatal(err)
	}
	if state.Phase() != login.Disconnected {
		t.Fatalf("phase=%s, want Disconnected", state.Phase())
	}
	if err := coordinator.Synchronize(); err != nil {
		t.Fatal(err)
	}
	if len(navigator.requests) != 1 || navigator.requests[0] != LoginSceneID {
		t.Fatalf("requests=%v, want login transition", navigator.requests)
	}
}

func TestCoordinatorSynchronizesOnlyWhenSceneDiffers(t *testing.T) {
	state := login.NewSessionState()
	navigator := &fakeNavigator{current: ServerSelectionSceneID}
	coordinator, err := New(state, fakeSender{}, navigator, Options{})
	if err != nil {
		t.Fatal(err)
	}
	if err := coordinator.Synchronize(); err != nil {
		t.Fatal(err)
	}
	if len(navigator.requests) != 0 {
		t.Fatalf("requests=%v, want none", navigator.requests)
	}
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if err := coordinator.Synchronize(); err != nil {
		t.Fatal(err)
	}
	if len(navigator.requests) != 1 || navigator.requests[0] != LoginSceneID {
		t.Fatalf("requests=%v, want login after connecting", navigator.requests)
	}
}

func TestCoordinatorDelegatesConnectionAndDisconnect(t *testing.T) {
	state := login.NewSessionState()
	navigator := &fakeNavigator{current: LoginSceneID}
	coordinator, err := New(state, fakeSender{}, navigator, Options{})
	if err != nil {
		t.Fatal(err)
	}
	if err := coordinator.SessionConnected(); err != nil {
		t.Fatal(err)
	}
	if state.Phase() != login.Connecting {
		t.Fatalf("phase=%s, want Connecting", state.Phase())
	}
	coordinator.SessionDisconnected()
	if state.Phase() != login.Disconnected {
		t.Fatalf("phase=%s, want Disconnected", state.Phase())
	}
	if handled, err := coordinator.HandleSessionEvent(protocol.SessionEvent{Kind: protocol.SessionDisconnected}); err != nil || !handled {
		t.Fatalf("disconnect event handled=%v err=%v", handled, err)
	}
}

func TestCoordinatorReportsNavigatorFailure(t *testing.T) {
	state := login.NewSessionState()
	navigator := &fakeNavigator{current: CharacterSelectSceneID, err: errors.New("transition unavailable")}
	coordinator, err := New(state, fakeSender{}, navigator, Options{})
	if err != nil {
		t.Fatal(err)
	}
	if err := coordinator.Synchronize(); err == nil {
		t.Fatal("Synchronize succeeded despite navigator failure")
	}
}
