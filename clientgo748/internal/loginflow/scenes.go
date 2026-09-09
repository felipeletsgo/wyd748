package loginflow

import (
	"errors"
	"fmt"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/scene"
	"wydclient748/internal/ui"
)

var errNoCharacterSelected = errors.New("loginflow: no character selected")

// SceneFactories returns independent logical scenes. They deliberately have
// no renderer or socket and can be replaced by visual scenes later.
func SceneFactories(state *login.SessionState) map[scene.ID]scene.Factory {
	return SceneFactoriesWithVisuals(state, VisualOptions{})
}

// VisualOptions supplies optional drawing and login submission without
// transferring renderer or transport ownership into a scene.
type VisualOptions struct {
	ShapeRenderer   graphics.ShapeRenderer
	LoginTexture    *assets.Texture
	Authenticate    func(account string, password []byte) error
	SelectCharacter func(slot int32) error
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
			if options.ShapeRenderer != nil || options.SelectCharacter != nil {
				return newCharacterSelectScene(state, options)
			}
			return newStateScene(CharacterSelectSceneID, state, login.CharacterSelect)
		},
		LoadingSceneID: func() (scene.Scene, error) {
			if options.ShapeRenderer != nil {
				return newLoadingScene(state, options.ShapeRenderer)
			}
			return newStateScene(LoadingSceneID, state, login.EnteringWorld)
		},
		WorldSceneID: func() (scene.Scene, error) {
			if options.ShapeRenderer != nil {
				return newWorldScene(state, options.ShapeRenderer)
			}
			return newStateScene(WorldSceneID, state, login.InWorld, login.LoggingOut)
		},
	}
}

type characterSelectScene struct {
	state           *login.SessionState
	renderer        graphics.ShapeRenderer
	selectCharacter func(int32) error
	selected        int
	status          string
}

func newCharacterSelectScene(state *login.SessionState, options VisualOptions) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	return &characterSelectScene{state: state, renderer: options.ShapeRenderer, selectCharacter: options.SelectCharacter, selected: -1}, nil
}
func (s *characterSelectScene) ID() scene.ID { return CharacterSelectSceneID }
func (s *characterSelectScene) Enter() error {
	if err := s.validate(); err != nil {
		return err
	}
	s.selected = -1
	s.status = ""
	list, _ := s.state.CharacterList()
	for i := range list.Characters {
		if list.Characters[i].Occupied() {
			s.selected = i
			break
		}
	}
	return nil
}
func (s *characterSelectScene) HandleEvent(event input.Event) error {
	if err := s.validate(); err != nil {
		return err
	}
	list, _ := s.state.CharacterList()
	if event.Kind == input.KindMouseButtonDown && event.Button == 1 {
		for i := 0; i < login.SelectionCharacterCount; i++ {
			if cardRect(i).Contains(event.X, event.Y) && list.Characters[i].Occupied() {
				s.selected = i
				return nil
			}
		}
		if selectRect().Contains(event.X, event.Y) {
			return s.submit()
		}
	}
	if event.Kind == input.KindKeyDown {
		switch event.Key {
		case 0x25:
			s.move(-1) // VK_LEFT
		case 0x27:
			s.move(1) // VK_RIGHT
		case 0x0D:
			return s.submit()
		}
	}
	return nil
}
func (s *characterSelectScene) Update(time.Duration) error { return s.validate() }
func (s *characterSelectScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	if s.renderer == nil {
		return nil
	}
	list, _ := s.state.CharacterList()
	for i := 0; i < login.SelectionCharacterCount; i++ {
		color := graphics.Color{R: .12, G: .13, B: .18, A: 1}
		if list.Characters[i].Occupied() {
			color = graphics.Color{R: .18, G: .24, B: .34, A: 1}
		}
		if i == s.selected {
			color = graphics.Color{R: .25, G: .48, B: .78, A: 1}
		}
		r := cardRect(i)
		s.renderer.DrawRect(r.X, r.Y, r.Width, r.Height, color)
	}
	r := selectRect()
	s.renderer.DrawRect(r.X, r.Y, r.Width, r.Height, graphics.Color{R: .12, G: .34, B: .62, A: 1})
	return nil
}
func (s *characterSelectScene) Exit() error  { return nil }
func (s *characterSelectScene) Close() error { return nil }
func (s *characterSelectScene) validate() error {
	if s == nil || s.state == nil {
		return errors.New("loginflow: character-select scene is unavailable")
	}
	if s.state.Phase() != login.CharacterSelect {
		return fmt.Errorf("loginflow: character-select scene does not accept phase %s", s.state.Phase())
	}
	if _, ok := s.state.CharacterList(); !ok {
		return errors.New("loginflow: character list is unavailable")
	}
	return nil
}
func (s *characterSelectScene) move(delta int) {
	list, _ := s.state.CharacterList()
	if delta == 0 {
		return
	}
	step := sign(delta)
	start := s.selected
	if start < 0 {
		start = 0
	}
	for n := 0; n < login.SelectionCharacterCount; n++ {
		i := (start + step*(n+1)) % login.SelectionCharacterCount
		if i < 0 {
			i += login.SelectionCharacterCount
		}
		s.selected = i
		if list.Characters[i].Occupied() {
			return
		}
	}
}
func (s *characterSelectScene) submit() error {
	if s.selected < 0 {
		s.status = "Select a character."
		return errNoCharacterSelected
	}
	if s.selectCharacter == nil {
		s.status = "Character selection is unavailable."
		return errors.New("loginflow: select callback is unavailable")
	}
	if err := s.selectCharacter(int32(s.selected)); err != nil {
		s.status = "Character selection failed. Please try again."
		return err
	}
	s.status = "Loading..."
	return nil
}
func sign(v int) int {
	if v < 0 {
		return -1
	}
	return 1
}
func cardRect(i int) ui.Rect { return ui.Rect{X: 70 + int32(i)*185, Y: 190, Width: 165, Height: 180} }
func selectRect() ui.Rect    { return ui.Rect{X: 330, Y: 420, Width: 140, Height: 38} }

type loadingScene struct {
	state    *login.SessionState
	renderer graphics.ShapeRenderer
}

func newLoadingScene(state *login.SessionState, renderer graphics.ShapeRenderer) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	return &loadingScene{state: state, renderer: renderer}, nil
}
func (s *loadingScene) ID() scene.ID                  { return LoadingSceneID }
func (s *loadingScene) Enter() error                  { return s.validate() }
func (s *loadingScene) HandleEvent(input.Event) error { return s.validate() }
func (s *loadingScene) Update(time.Duration) error    { return s.validate() }
func (s *loadingScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	if s.renderer != nil {
		s.renderer.DrawRect(290, 290, 220, 18, graphics.Color{R: .12, G: .34, B: .62, A: 1})
	}
	return nil
}
func (s *loadingScene) Exit() error  { return nil }
func (s *loadingScene) Close() error { return nil }
func (s *loadingScene) validate() error {
	if s == nil || s.state == nil {
		return errors.New("loginflow: loading scene is unavailable")
	}
	if s.state.Phase() != login.EnteringWorld {
		return fmt.Errorf("loginflow: loading scene does not accept phase %s", s.state.Phase())
	}
	return nil
}

type worldScene struct {
	state    *login.SessionState
	renderer graphics.ShapeRenderer
}

func newWorldScene(state *login.SessionState, renderer graphics.ShapeRenderer) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	return &worldScene{state: state, renderer: renderer}, nil
}
func (s *worldScene) ID() scene.ID                  { return WorldSceneID }
func (s *worldScene) Enter() error                  { return s.validate() }
func (s *worldScene) HandleEvent(input.Event) error { return s.validate() }
func (s *worldScene) Update(time.Duration) error    { return s.validate() }
func (s *worldScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	if s.renderer != nil {
		s.renderer.DrawRect(20, 20, 180, 48, graphics.Color{R: .08, G: .12, B: .18, A: .9})
	}
	return nil
}
func (s *worldScene) Exit() error  { return nil }
func (s *worldScene) Close() error { return nil }
func (s *worldScene) validate() error {
	if s == nil || s.state == nil {
		return errors.New("loginflow: world scene is unavailable")
	}
	switch s.state.Phase() {
	case login.InWorld, login.LoggingOut:
		if _, ok := s.state.World(); !ok {
			return errors.New("loginflow: world snapshot is unavailable")
		}
		return nil
	default:
		return fmt.Errorf("loginflow: world scene does not accept phase %s", s.state.Phase())
	}
}

type loginScene struct {
	state             *login.SessionState
	form              *ui.LoginForm
	renderer          graphics.ShapeRenderer
	loginTexture      *assets.Texture
	textureRenderer   graphics.TextureRenderer
	placementRenderer graphics.TexturePlacementRenderer
}

const (
	loginTextureX      int32 = 272
	loginTextureY      int32 = 172
	loginTextureWidth  int32 = 256
	loginTextureHeight int32 = 256
)

func newLoginScene(state *login.SessionState, options VisualOptions) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	textureRenderer, _ := options.ShapeRenderer.(graphics.TextureRenderer)
	placementRenderer, _ := options.ShapeRenderer.(graphics.TexturePlacementRenderer)
	return &loginScene{state: state, renderer: options.ShapeRenderer, form: ui.NewLoginForm(options.Authenticate), loginTexture: options.LoginTexture, textureRenderer: textureRenderer, placementRenderer: placementRenderer}, nil
}

func (s *loginScene) ID() scene.ID { return LoginSceneID }
func (s *loginScene) Enter() error {
	if err := s.validate(); err != nil {
		return err
	}
	if s.loginTexture != nil && s.textureRenderer != nil {
		if err := s.textureRenderer.UploadTexture(*s.loginTexture); err != nil {
			return fmt.Errorf("loginflow: upload login UI: %w", err)
		}
	}
	return nil
}
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
	if s.loginTexture != nil {
		switch {
		case s.placementRenderer != nil && s.textureRenderer != nil:
			s.placementRenderer.DrawTextureAt(loginTextureX, loginTextureY, loginTextureWidth, loginTextureHeight)
		case s.textureRenderer != nil:
			// A backend without placement support still presents the native
			// artwork; it owns the fallback composition policy.
			s.textureRenderer.DrawTexture()
		}
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
