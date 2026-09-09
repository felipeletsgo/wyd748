package loginflow

import (
	"errors"
	"fmt"
	"time"

	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/scene"
)

// SceneFactories returns independent logical scenes. They deliberately have
// no renderer or socket and can be replaced by visual scenes later.
func SceneFactories(state *login.SessionState) map[scene.ID]scene.Factory {
	return map[scene.ID]scene.Factory{
		LoginSceneID: func() (scene.Scene, error) {
			return newStateScene(LoginSceneID, state, login.Disconnected, login.Connecting, login.Authenticating)
		},
		CharacterSelectSceneID: func() (scene.Scene, error) {
			return newStateScene(CharacterSelectSceneID, state, login.CharacterSelect)
		},
		LoadingSceneID: func() (scene.Scene, error) { return newStateScene(LoadingSceneID, state, login.EnteringWorld) },
		WorldSceneID: func() (scene.Scene, error) {
			return newStateScene(WorldSceneID, state, login.InWorld, login.LoggingOut)
		},
	}
}

type stateScene struct {
	id       scene.ID
	state    *login.SessionState
	accepted map[login.Phase]struct{}
}

func newStateScene(id scene.ID, state *login.SessionState, phases ...login.Phase) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	accepted := make(map[login.Phase]struct{}, len(phases))
	for _, phase := range phases {
		accepted[phase] = struct{}{}
	}
	return &stateScene{id: id, state: state, accepted: accepted}, nil
}

func (s *stateScene) ID() scene.ID { return s.id }
func (s *stateScene) Enter() error {
	return s.validate("enter")
}
func (s *stateScene) HandleEvent(input.Event) error { return nil }
func (s *stateScene) Update(time.Duration) error {
	return s.validate("update")
}
func (s *stateScene) Render() error { return nil }
func (s *stateScene) Exit() error   { return nil }
func (s *stateScene) Close() error  { return nil }

func (s *stateScene) validate(operation string) error {
	if s == nil || s.state == nil {
		return errors.New("loginflow: scene state is unavailable")
	}
	phase := s.state.Phase()
	if _, ok := s.accepted[phase]; !ok {
		return fmt.Errorf("loginflow: %s scene %q does not accept phase %s", operation, s.id, phase)
	}
	return nil
}
