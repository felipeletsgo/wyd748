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
	ShapeRenderer graphics.ShapeRenderer
	ServerTexture *assets.Texture
	Servers       []ServerEntry
	SelectServer  func(ServerEntry) error
	// RequestClose asks the application to close after the current input
	// dispatch. The scene never destroys the window or renderer directly.
	RequestClose    func() error
	LoginTexture    *assets.Texture
	LoginLogoLeft   *assets.Texture
	LoginLogoRight  *assets.Texture
	Authenticate    func(account string, password []byte) error
	SelectCharacter func(slot int32) error
}

func SceneFactoriesWithVisuals(state *login.SessionState, options VisualOptions) map[scene.ID]scene.Factory {
	return map[scene.ID]scene.Factory{
		ServerSelectionSceneID: func() (scene.Scene, error) {
			if options.ShapeRenderer != nil || options.SelectServer != nil || options.RequestClose != nil {
				return newServerSelectionScene(state, options)
			}
			return newStateScene(ServerSelectionSceneID, state, login.Disconnected)
		},
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

// ServerEntry is the endpoint presented by the bootstrap selector.
type ServerEntry struct {
	Name    string
	Address string
}

type serverSelectionScene struct {
	state        *login.SessionState
	renderer     graphics.ShapeRenderer
	texture      *assets.Texture
	textureDraw  graphics.TextureRenderer
	placement    graphics.TexturePlacementRenderer
	servers      []ServerEntry
	selectServer func(ServerEntry) error
	requestClose func() error
	selected     int
	status       string
}

func newServerSelectionScene(state *login.SessionState, options VisualOptions) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	servers := append([]ServerEntry(nil), options.Servers...)
	if len(servers) == 0 {
		servers = []ServerEntry{{Name: "Local Server", Address: "127.0.0.1:8281"}}
	}
	textureRenderer, _ := options.ShapeRenderer.(graphics.TextureRenderer)
	placement, _ := options.ShapeRenderer.(graphics.TexturePlacementRenderer)
	return &serverSelectionScene{state: state, renderer: options.ShapeRenderer, texture: options.ServerTexture,
		textureDraw: textureRenderer, placement: placement, servers: servers,
		selectServer: options.SelectServer, requestClose: options.RequestClose, selected: 0}, nil
}

func (s *serverSelectionScene) ID() scene.ID { return ServerSelectionSceneID }
func (s *serverSelectionScene) Enter() error {
	if err := s.validate(); err != nil {
		return err
	}
	s.selected, s.status = 0, ""
	if s.texture != nil && s.textureDraw != nil {
		if err := s.textureDraw.UploadTexture(*s.texture); err != nil {
			return fmt.Errorf("loginflow: upload server selection UI: %w", err)
		}
	}
	return nil
}
func (s *serverSelectionScene) HandleEvent(event input.Event) error {
	if err := s.validate(); err != nil {
		return err
	}
	if event.Kind == input.KindMouseButtonDown && event.Button == 1 {
		layout := serverSelectionLayoutFor(s.renderer, len(s.servers))
		for i := range s.servers {
			if layout.serverRow(i).Contains(event.X, event.Y) {
				// A row selects an endpoint; only the explicit CONNECT control
				// (or Enter) opens the transport.
				s.selected = i
				return nil
			}
		}
		if layout.connect.Contains(event.X, event.Y) {
			return s.submit()
		}
		if layout.close.Contains(event.X, event.Y) {
			return s.closeRequested()
		}
	}
	if event.Kind == input.KindKeyDown {
		switch event.Key {
		case 0x25, 0x26:
			s.move(-1)
		case 0x27, 0x28:
			s.move(1)
		case 0x0D:
			return s.submit()
		}
	}
	return nil
}
func (s *serverSelectionScene) Update(time.Duration) error { return s.validate() }
func (s *serverSelectionScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	layout := serverSelectionLayoutFor(s.renderer, len(s.servers))
	if s.texture != nil {
		switch {
		case s.placement != nil && s.textureDraw != nil:
			s.placement.DrawTextureAt(layout.texture.X, layout.texture.Y, layout.texture.Width, layout.texture.Height)
		case s.textureDraw != nil:
			s.textureDraw.DrawTexture()
		}
	}
	if s.renderer == nil {
		return nil
	}
	text, _ := s.renderer.(graphics.TextRenderer)
	for i, entry := range s.servers {
		r := layout.serverRow(i)
		color := graphics.Color{R: .78, G: .82, B: .90, A: 1}
		if i == s.selected {
			color = graphics.Color{R: 1, G: 1, B: 1, A: 1}
		}
		if text != nil {
			name := entry.Name
			if name == "" {
				name = entry.Address
			}
			text.DrawText(r.X, r.Y+6, name, 12, color)
		}
	}
	if text != nil {
		text.DrawText(layout.serverTitle.X, layout.serverTitle.Y, "SERVER", 11, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		text.DrawText(layout.channelTitle.X, layout.channelTitle.Y, "CHANNEL", 11, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		// The endpoint is an internal transport value, not the channel label.
		// The native selector shows this state as "No Server." until a channel
		// list is returned by the selected server.
		text.DrawText(layout.channel.X, layout.channel.Y, "No Server.", 10, graphics.Color{R: .88, G: .90, B: .94, A: 1})
		text.DrawText(layout.connect.X+7, layout.connect.Y+6, "CONNECT", 9, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		text.DrawText(layout.close.X+17, layout.close.Y+6, "CLOSE", 9, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		if s.status != "" {
			text.DrawText(layout.status.X, layout.status.Y, s.status, 10, graphics.Color{R: .72, G: .76, B: .84, A: 1})
		}
	}
	return nil
}
func (s *serverSelectionScene) Exit() error  { return nil }
func (s *serverSelectionScene) Close() error { return nil }
func (s *serverSelectionScene) validate() error {
	if s == nil || s.state == nil {
		return errors.New("loginflow: server-selection scene is unavailable")
	}
	if s.state.Phase() != login.Disconnected {
		return fmt.Errorf("loginflow: server-selection scene does not accept phase %s", s.state.Phase())
	}
	if len(s.servers) == 0 {
		return errors.New("loginflow: server list is empty")
	}
	return nil
}
func (s *serverSelectionScene) move(delta int) {
	if len(s.servers) == 0 || delta == 0 {
		return
	}
	s.selected = (s.selected + delta) % len(s.servers)
	if s.selected < 0 {
		s.selected += len(s.servers)
	}
}
func (s *serverSelectionScene) submit() error {
	if s.selectServer == nil {
		s.status = "Server selection is unavailable."
		return errors.New("loginflow: server selection callback is unavailable")
	}
	entry := s.servers[s.selected]
	if entry.Address == "" {
		s.status = "Selected server has no endpoint."
		return errors.New("loginflow: selected server has no endpoint")
	}
	if err := s.selectServer(entry); err != nil {
		s.status = "Unable to connect to the selected server."
		// A reachable-window transport failure is recoverable UI state. Keep the
		// selector alive so the player can retry instead of terminating the client.
		return nil
	}
	s.status = "Connecting..."
	return nil
}

func (s *serverSelectionScene) closeRequested() error {
	if s.requestClose == nil {
		s.status = "Close is unavailable."
		return errors.New("loginflow: close callback is unavailable")
	}
	if err := s.requestClose(); err != nil {
		s.status = "Unable to close the client."
		return err
	}
	return nil
}

// serverSelectionLayout mirrors the native 7.48 root (0x120E): the root is
// centered once, then the official 256x256 skin and all dynamic controls are
// positioned relative to it. This prevents DPI/window-size changes from
// separating visual rows, text and hitboxes.
type serverSelectionLayout struct {
	root         ui.Rect
	texture      ui.Rect
	serverTitle  serverPoint
	channelTitle serverPoint
	channel      serverPoint
	connect      ui.Rect
	close        ui.Rect
	status       serverPoint
}

type serverPoint struct{ X, Y int32 }

const (
	serverRootWidth  int32 = 313
	serverRootHeight int32 = 256
	serverSkinSize   int32 = 256
)

func serverSelectionLayoutFor(renderer graphics.ShapeRenderer, count int) serverSelectionLayout {
	width, height := int32(800), int32(600)
	if provider, ok := renderer.(graphics.ViewportProvider); ok {
		if candidateWidth, candidateHeight := provider.ClientViewport(); candidateWidth > 0 && candidateHeight > 0 {
			width, height = candidateWidth, candidateHeight
		}
	}
	root := ui.Rect{X: (width - serverRootWidth) / 2, Y: (height - serverRootHeight) / 2, Width: serverRootWidth, Height: serverRootHeight}
	texture := ui.Rect{X: root.X + (serverRootWidth-serverSkinSize)/2, Y: root.Y, Width: serverSkinSize, Height: serverSkinSize}
	return serverSelectionLayout{
		root:         root,
		texture:      texture,
		serverTitle:  serverPoint{X: texture.X + 30, Y: root.Y + 12},
		channelTitle: serverPoint{X: texture.X + 158, Y: root.Y + 12},
		channel:      serverPoint{X: texture.X + 157, Y: root.Y + 54},
		connect:      ui.Rect{X: texture.X + 16, Y: root.Y + 218, Width: 66, Height: 24},
		close:        ui.Rect{X: texture.X + 99, Y: root.Y + 218, Width: 66, Height: 24},
		status:       serverPoint{X: root.X, Y: root.Y + serverRootHeight + 10},
	}
}

func (l serverSelectionLayout) serverRow(index int) ui.Rect {
	if index < 0 {
		index = 0
	}
	return ui.Rect{X: l.texture.X + 18, Y: l.root.Y + 47 + int32(index)*27, Width: 112, Height: 23}
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
	loginLogoLeft     *assets.Texture
	loginLogoRight    *assets.Texture
	textureRenderer   graphics.TextureRenderer
	placementRenderer graphics.TexturePlacementRenderer
	layers            graphics.LayeredTextureRenderer
}

const (
	loginDesignWidth   int32 = 800
	loginDesignHeight  int32 = 600
	loginTextureWidth  int32 = 256
	loginTextureHeight int32 = 256
	loginLogoWidth     int32 = 256
	loginLogoHeight    int32 = 256
)

type loginLayout struct {
	panel, logoLeft, logoRight        ui.Rect
	account, password, submit         ui.Rect
	accountText, passwordText, status serverPoint
}

func loginLayoutFor(renderer graphics.ShapeRenderer) loginLayout {
	width, height := loginDesignWidth, loginDesignHeight
	if provider, ok := renderer.(graphics.ViewportProvider); ok {
		if w, h := provider.ClientViewport(); w > 0 && h > 0 {
			width, height = w, h
		}
	}
	rootX := (width - loginDesignWidth) / 2
	rootY := (height - loginDesignHeight) / 2
	panel := ui.Rect{X: rootX + 272, Y: rootY + 172, Width: loginTextureWidth, Height: loginTextureHeight}
	// logo1/logo2 are the two halves of the official WYD FC mark. They are
	// deliberately kept as separate layers because each WYT contains alpha.
	logoX := rootX + (loginDesignWidth-loginLogoWidth*2)/2
	logoY := rootY + 28
	return loginLayout{
		panel:        panel,
		logoLeft:     ui.Rect{X: logoX, Y: logoY, Width: loginLogoWidth, Height: loginLogoHeight},
		logoRight:    ui.Rect{X: logoX + loginLogoWidth, Y: logoY, Width: loginLogoWidth, Height: loginLogoHeight},
		account:      ui.Rect{X: panel.X + 72, Y: panel.Y + 30, Width: 118, Height: 28},
		password:     ui.Rect{X: panel.X + 72, Y: panel.Y + 57, Width: 118, Height: 28},
		submit:       ui.Rect{X: panel.X + 68, Y: panel.Y + 84, Width: 74, Height: 29},
		accountText:  serverPoint{X: panel.X + 82, Y: panel.Y + 38},
		passwordText: serverPoint{X: panel.X + 82, Y: panel.Y + 64},
		status:       serverPoint{X: panel.X, Y: panel.Y + 270},
	}
}

func newLoginScene(state *login.SessionState, options VisualOptions) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	textureRenderer, _ := options.ShapeRenderer.(graphics.TextureRenderer)
	placementRenderer, _ := options.ShapeRenderer.(graphics.TexturePlacementRenderer)
	layers, _ := options.ShapeRenderer.(graphics.LayeredTextureRenderer)
	return &loginScene{state: state, renderer: options.ShapeRenderer, form: ui.NewLoginForm(options.Authenticate), loginTexture: options.LoginTexture, loginLogoLeft: options.LoginLogoLeft, loginLogoRight: options.LoginLogoRight, textureRenderer: textureRenderer, placementRenderer: placementRenderer, layers: layers}, nil
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
	if s.layers != nil {
		if s.loginLogoLeft != nil {
			if err := s.layers.UploadTextureLayer("login-logo-left", *s.loginLogoLeft); err != nil {
				return fmt.Errorf("loginflow: upload left login logo: %w", err)
			}
		}
		if s.loginLogoRight != nil {
			if err := s.layers.UploadTextureLayer("login-logo-right", *s.loginLogoRight); err != nil {
				return fmt.Errorf("loginflow: upload right login logo: %w", err)
			}
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
	layout := loginLayoutFor(s.renderer)
	s.form.AccountRect, s.form.PasswordRect, s.form.ButtonRect = layout.account, layout.password, layout.submit
	if s.layers != nil {
		s.layers.DrawTextureLayer("login-logo-left", layout.logoLeft.X, layout.logoLeft.Y, layout.logoLeft.Width, layout.logoLeft.Height)
		s.layers.DrawTextureLayer("login-logo-right", layout.logoRight.X, layout.logoRight.Y, layout.logoRight.Width, layout.logoRight.Height)
	}
	if s.loginTexture != nil {
		switch {
		case s.placementRenderer != nil && s.textureRenderer != nil:
			s.placementRenderer.DrawTextureAt(layout.panel.X, layout.panel.Y, layout.panel.Width, layout.panel.Height)
		case s.textureRenderer != nil:
			// A backend without placement support still presents the native
			// artwork; it owns the fallback composition policy.
			s.textureRenderer.DrawTexture()
		}
	}
	s.form.RenderAt(s.renderer, layout.accountText.X, layout.accountText.Y, layout.passwordText.X, layout.passwordText.Y, layout.status.X, layout.status.Y)
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
