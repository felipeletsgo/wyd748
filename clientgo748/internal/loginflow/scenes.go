package loginflow

import (
	"errors"
	"fmt"
	"time"

	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/scene"
	"wydclient748/internal/ui"
)

// SceneFactories returns independent logical scenes. They deliberately have
// no renderer or socket and can be replaced by visual scenes later.
func SceneFactories(state *login.SessionState) map[scene.ID]scene.Factory {
	return SceneFactoriesWithVisuals(state, VisualOptions{})
}

// VisualOptions supplies optional drawing and login submission without
// transferring renderer or transport ownership into a scene.
type VisualOptions struct {
	ShapeRenderer graphics.ShapeRenderer
	Authenticate  func(account string, password []byte) error
}

func SceneFactoriesWithVisuals(state *login.SessionState, options VisualOptions) map[scene.ID]scene.Factory {
	return map[scene.ID]scene.Factory{
		LoginSceneID: func() (scene.Scene, error) {
			if options.ShapeRenderer != nil || options.Authenticate != nil {
				return newLoginScene(state, options)
			}
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

type loginScene struct {
	state    *login.SessionState
	form     *ui.LoginForm
	renderer graphics.ShapeRenderer
}

func newLoginScene(state *login.SessionState, options VisualOptions) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	return &loginScene{state: state, renderer: options.ShapeRenderer, form: ui.NewLoginForm(options.Authenticate)}, nil
}

func (s *loginScene) ID() scene.ID { return LoginSceneID }
func (s *loginScene) Enter() error { return s.validate() }
func (s *loginScene) HandleEvent(event input.Event) error {
	if err := s.validate(); err != nil {
		return err
	}
	return s.form.HandleEvent(event)
}
func (s *loginScene) Update(time.Duration) error { return s.validate() }
func (s *loginScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	s.form.Render(s.renderer)
	return nil
}
func (s *loginScene) Exit() error  { clear(s.form.Password); s.form.Password = nil; return nil }
func (s *loginScene) Close() error { return s.Exit() }
func (s *loginScene) validate() error {
	if s == nil || s.state == nil || s.form == nil {
		return errors.New("loginflow: login scene is unavailable")
	}
	switch s.state.Phase() {
	case login.Disconnected, login.Connecting, login.Authenticating:
		return nil
	default:
		return fmt.Errorf("loginflow: login scene does not accept phase %s", s.state.Phase())
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
