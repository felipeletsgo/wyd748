package loginflow

import (
	"testing"
	"time"

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
