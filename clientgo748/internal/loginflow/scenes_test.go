package loginflow

import (
	"testing"
	"time"

	"wydclient748/internal/input"
	"wydclient748/internal/login"
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
