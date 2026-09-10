package loginflow

import (
	"errors"
	"fmt"
	"sort"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/scene"
	"wydclient748/internal/ui"
	"wydclient748/internal/world"
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
	LoginControls   []assets.SceneControl
	UIStrings       map[int32]string
	LoginLogoLeft   *assets.Texture
	LoginLogoRight  *assets.Texture
	Authenticate    func(account string, password []byte) error
	SelectCharacter func(slot int32) error
	// WorldEntities returns a defensive snapshot owned by the coordinator.
	// The scene never retains or mutates the returned slice.
	WorldEntities func() []world.Entity
	RequestMove   func(targetX, targetY uint16) error
	// Terrain is an optional, read-only diagnostic surface. It does not alter
	// authoritative movement or collision decisions.
	Terrain *assets.Terrain
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
				return newWorldScene(state, options.ShapeRenderer, options.WorldEntities, options.RequestMove, options.Terrain)
			}
			return newStateScene(WorldSceneID, state, login.InWorld, login.LoggingOut)
		},
	}
}

// ServerEntry is the endpoint presented by the bootstrap selector.
type ServerEntry struct {
	Name    string
	Channel string
	Address string
}

type serverSelectionScene struct {
	state        *login.SessionState
	renderer     graphics.ShapeRenderer
	texture      *assets.Texture
	textureDraw  graphics.TextureRenderer
	placement    graphics.TexturePlacementRenderer
	groups       []serverGroup
	selectServer func(ServerEntry) error
	requestClose func() error
	selected     int
	channel      int
	status       string
}

type serverGroup struct {
	name     string
	channels []ServerEntry
}

func newServerSelectionScene(state *login.SessionState, options VisualOptions) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	entries := append([]ServerEntry(nil), options.Servers...)
	if len(entries) == 0 {
		entries = []ServerEntry{{Name: "Local Server", Channel: "Channel 1", Address: "127.0.0.1:8281"}}
	}
	groups := make([]serverGroup, 0, len(entries))
	groupIndex := make(map[string]int, len(entries))
	for _, entry := range entries {
		if entry.Channel == "" {
			entry.Channel = "Channel 1"
		}
		index, ok := groupIndex[entry.Name]
		if !ok {
			index = len(groups)
			groupIndex[entry.Name] = index
			groups = append(groups, serverGroup{name: entry.Name})
		}
		groups[index].channels = append(groups[index].channels, entry)
	}
	textureRenderer, _ := options.ShapeRenderer.(graphics.TextureRenderer)
	placement, _ := options.ShapeRenderer.(graphics.TexturePlacementRenderer)
	return &serverSelectionScene{state: state, renderer: options.ShapeRenderer, texture: options.ServerTexture,
		textureDraw: textureRenderer, placement: placement, groups: groups,
		selectServer: options.SelectServer, requestClose: options.RequestClose, selected: 0, channel: 0}, nil
}

func (s *serverSelectionScene) ID() scene.ID { return ServerSelectionSceneID }
func (s *serverSelectionScene) Enter() error {
	if err := s.validate(); err != nil {
		return err
	}
	s.selected, s.channel, s.status = 0, 0, ""
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
		layout := serverSelectionLayoutFor(s.renderer, len(s.groups))
		for i := range s.groups {
			if layout.serverRow(i).Contains(event.X, event.Y) {
				s.selected = i
				s.channel = 0
				return nil
			}
		}
		if s.selected >= 0 && s.selected < len(s.groups) {
			for i := range s.groups[s.selected].channels {
				if layout.channelRow(i).Contains(event.X, event.Y) {
					s.channel = i
					return nil
				}
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
		case 0x1B: // VK_ESCAPE: native selector closes the launcher window.
			return s.closeRequested()
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
	layout := serverSelectionLayoutFor(s.renderer, len(s.groups))
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
	for i, group := range s.groups {
		r := layout.serverRow(i)
		color := graphics.Color{R: .78, G: .82, B: .90, A: 1}
		if i == s.selected {
			color = graphics.Color{R: 1, G: 1, B: 1, A: 1}
		}
		if text != nil {
			name := group.name
			text.DrawText(r.X, r.Y+6, name, 12, color)
		}
	}
	if text != nil {
		text.DrawText(layout.serverTitle.X, layout.serverTitle.Y, "SERVER", 11, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		text.DrawText(layout.channelTitle.X, layout.channelTitle.Y, "CHANNEL", 11, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		if s.selected >= 0 && s.selected < len(s.groups) {
			for i, entry := range s.groups[s.selected].channels {
				r := layout.channelRow(i)
				color := graphics.Color{R: .78, G: .82, B: .90, A: 1}
				if i == s.channel {
					color = graphics.Color{R: 1, G: 1, B: 1, A: 1}
				}
				text.DrawText(r.X, r.Y+6, entry.Channel, 10, color)
			}
		}
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
	if len(s.groups) == 0 {
		return errors.New("loginflow: server list is empty")
	}
	for i, group := range s.groups {
		if len(group.channels) == 0 {
			return fmt.Errorf("loginflow: server group %d has no channels", i)
		}
	}
	return nil
}
func (s *serverSelectionScene) move(delta int) {
	if len(s.groups) == 0 || delta == 0 {
		return
	}
	s.selected = (s.selected + delta) % len(s.groups)
	if s.selected < 0 {
		s.selected += len(s.groups)
	}
	s.channel = 0
}
func (s *serverSelectionScene) submit() error {
	if s.selectServer == nil {
		s.status = "Server selection is unavailable."
		return errors.New("loginflow: server selection callback is unavailable")
	}
	if s.selected < 0 || s.selected >= len(s.groups) || s.channel < 0 || s.channel >= len(s.groups[s.selected].channels) {
		s.status = "Select a channel."
		return errors.New("loginflow: no channel selected")
	}
	entry := s.groups[s.selected].channels[s.channel]
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
		serverTitle:  serverPoint{X: texture.X + 30, Y: root.Y + 17},
		channelTitle: serverPoint{X: texture.X + 166, Y: root.Y + 17},
		channel:      serverPoint{X: texture.X + 165, Y: root.Y + 59},
		connect:      ui.Rect{X: texture.X + 16, Y: root.Y + 218, Width: 66, Height: 24},
		close:        ui.Rect{X: texture.X + 99, Y: root.Y + 218, Width: 66, Height: 24},
		status:       serverPoint{X: root.X, Y: root.Y + serverRootHeight + 10},
	}
}

func (l serverSelectionLayout) serverRow(index int) ui.Rect {
	if index < 0 {
		index = 0
	}
	return ui.Rect{X: l.texture.X + 18, Y: l.root.Y + 52 + int32(index)*27, Width: 112, Height: 23}
}

func (l serverSelectionLayout) channelRow(index int) ui.Rect {
	if index < 0 {
		index = 0
	}
	return ui.Rect{X: l.texture.X + 151, Y: l.root.Y + 52 + int32(index)*27, Width: 140, Height: 23}
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
	state         *login.SessionState
	renderer      graphics.ShapeRenderer
	worldEntities func() []world.Entity
	requestMove   func(uint16, uint16) error
	terrain       *assets.Terrain
}

func newWorldScene(state *login.SessionState, renderer graphics.ShapeRenderer, entities func() []world.Entity, requestMove func(uint16, uint16) error, terrain *assets.Terrain) (scene.Scene, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	return &worldScene{state: state, renderer: renderer, worldEntities: entities, requestMove: requestMove, terrain: terrain}, nil
}
func (s *worldScene) ID() scene.ID { return WorldSceneID }
func (s *worldScene) Enter() error { return s.validate() }
func (s *worldScene) HandleEvent(event input.Event) error {
	if err := s.validate(); err != nil {
		return err
	}
	if event.Kind == input.KindMouseButtonDown && event.Button == 1 && s.requestMove != nil {
		if event.X < 0 || event.Y < 72 {
			return nil
		}
		snapshot, ok := s.state.World()
		if !ok {
			return errors.New("loginflow: world snapshot is unavailable")
		}
		width, height := int32(800), int32(600)
		if viewport, ok := s.renderer.(graphics.ViewportProvider); ok {
			if w, h := viewport.ClientViewport(); w > 0 && h > 0 {
				width, height = w, h
			}
		}
		targetX, targetY := worldTargetForClick(snapshot, event.X, event.Y, width, height)
		if targetX < 0 || targetX > 65535 || targetY < 0 || targetY > 65535 {
			return errors.New("loginflow: movement target is outside the world bounds")
		}
		return s.requestMove(uint16(targetX), uint16(targetY))
	}
	return nil
}

func worldTargetForClick(snapshot login.WorldSnapshot, x, y, width, height int32) (int32, int32) {
	return int32(snapshot.PosX) + (x-width/2)/8,
		int32(snapshot.PosY) + (y-(height+72)/2)/8
}
func (s *worldScene) Update(time.Duration) error { return s.validate() }
func (s *worldScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	if s.renderer == nil {
		return nil
	}
	snapshot, _ := s.state.World()
	width, height := int32(800), int32(600)
	if viewport, ok := s.renderer.(graphics.ViewportProvider); ok {
		if w, h := viewport.ClientViewport(); w > 0 && h > 0 {
			width, height = w, h
		}
	}
	// This is a diagnostic world surface until the terrain/mesh pipeline is
	// connected. Coordinates remain authoritative: the player marker is always
	// centered while the grid exposes the received map position and movement
	// updates without inventing client-side world state.
	s.renderer.DrawRect(0, 0, width, height, graphics.Color{R: .035, G: .055, B: .075, A: 1})
	drawTerrainDiagnostic(s.renderer, s.terrain, 8, 80, 8)
	for x := int32(0); x < width; x += 64 {
		s.renderer.DrawRect(x, 72, 1, height-72, graphics.Color{R: .10, G: .14, B: .18, A: 1})
	}
	for y := int32(72); y < height; y += 64 {
		s.renderer.DrawRect(0, y, width, 1, graphics.Color{R: .10, G: .14, B: .18, A: 1})
	}
	playerX, playerY := width/2-8, (height+72)/2-8
	s.renderer.DrawRect(playerX, playerY, 16, 16, graphics.Color{R: .25, G: .75, B: .95, A: 1})
	if s.worldEntities != nil {
		entities := s.worldEntities()
		drawWorldEntities(s.renderer, entities, snapshot, width, height)
	}
	if text, ok := s.renderer.(graphics.TextRenderer); ok {
		text.DrawText(16, 16, "WORLD", 14, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		text.DrawText(16, 38, fmt.Sprintf("Position: %d, %d", snapshot.PosX, snapshot.PosY), 11, graphics.Color{R: .78, G: .84, B: .90, A: 1})
		name := snapshot.Mob.Name
		if name == "" {
			name = "Character"
		}
		text.DrawText(playerX-24, playerY-20, name, 10, graphics.Color{R: .75, G: .90, B: 1, A: 1})
	}
	return nil
}

// drawTerrainDiagnostic projects only the confirmed signed height field. The
// result is a visual inspection aid, not a client-side map or collision mesh.
func drawTerrainDiagnostic(renderer graphics.ShapeRenderer, terrain *assets.Terrain, originX, originY, cellSize int32) {
	if renderer == nil || terrain == nil || terrain.Columns == 0 || terrain.Rows == 0 || cellSize <= 0 {
		return
	}
	for y := 0; y < terrain.Rows; y++ {
		for x := 0; x < terrain.Columns; x++ {
			cell, ok := terrain.Cell(x, y)
			if !ok {
				continue
			}
			// Map signed height deterministically to a restrained blue/green
			// ramp. No gameplay meaning is assigned to this color.
			value := float32(int(cell.Height)+128) / 255
			renderer.DrawRect(originX+int32(x)*cellSize, originY+int32(y)*cellSize, cellSize, cellSize,
				graphics.Color{R: .08 + value*.18, G: .16 + value*.42, B: .24 + value*.58, A: 1})
		}
	}
}

// drawWorldEntities only projects an immutable server snapshot onto the
// diagnostic surface. It deliberately does not interpolate, create or remove
// entities; those decisions remain in world.State and the server.
func drawWorldEntities(renderer graphics.ShapeRenderer, entities []world.Entity, player login.WorldSnapshot, width, height int32) {
	if renderer == nil || len(entities) == 0 {
		return
	}
	ordered := append([]world.Entity(nil), entities...)
	sort.Slice(ordered, func(i, j int) bool { return ordered[i].ID < ordered[j].ID })
	text, hasText := renderer.(graphics.TextRenderer)
	for _, entity := range ordered {
		if entity.ID == player.ClientID {
			continue
		}
		dx := int32(entity.PosX) - int32(player.PosX)
		dy := int32(entity.PosY) - int32(player.PosY)
		x := width/2 + dx*8 - 6
		y := (height+72)/2 + dy*8 - 6
		if x < -12 || y < 60 || x > width+12 || y > height+12 {
			continue
		}
		renderer.DrawRect(x, y, 12, 12, graphics.Color{R: .92, G: .42, B: .24, A: 1})
		if hasText {
			name := entity.Name
			if name == "" {
				name = "Mob"
			}
			text.DrawText(x-12, y-16, name, 9, graphics.Color{R: 1, G: .78, B: .68, A: 1})
		}
	}
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
	loginControls     []assets.SceneControl
	captions          []loginCaption
	textureRenderer   graphics.TextureRenderer
	placementRenderer graphics.TexturePlacementRenderer
	layers            graphics.LayeredTextureRenderer
}

const (
	loginDesignWidth   int32 = 800
	loginDesignHeight  int32 = 600
	loginTextureWidth  int32 = 215
	loginTextureHeight int32 = 153
	loginLogoWidth     int32 = 256
	loginLogoHeight    int32 = 256
)

type loginLayout struct {
	panel, logoLeft, logoRight        ui.Rect
	account, password, submit         ui.Rect
	accountText, passwordText, status serverPoint
}

func (l *loginLayout) applyNativeControls(controls []assets.SceneControl) {
	panel, ok := assets.FindControl(controls, 4608)
	if !ok || len(panel.Words) < 7 {
		return
	}
	// The native panel record supplies the authoritative artwork dimensions;
	// retain viewport centering while avoiding the obsolete hardcoded crop.
	if panel.Words[5] > 0 && panel.Words[6] > 0 {
		l.panel.Width, l.panel.Height = panel.Words[5], panel.Words[6]
		l.panel.X = (l.panel.X*2 + loginTextureWidth - l.panel.Width) / 2
		l.panel.Y = (l.panel.Y*2 + loginTextureHeight - l.panel.Height) / 2
	}
	for id, dst := range map[int32]*ui.Rect{5121: &l.account, 5122: &l.password, 4609: &l.submit} {
		c, found := assets.FindControl(controls, id)
		if !found || len(c.Words) < 7 {
			continue
		}
		*dst = ui.Rect{X: l.panel.X + c.Words[3], Y: l.panel.Y + c.Words[4], Width: c.Words[5], Height: c.Words[6]}
	}
	// Modernização local: texto e input compartilham a geometria já carregada.
	// DrawText usa a origem superior esquerda, não uma baseline tipográfica.
	l.accountText = serverPoint{X: l.account.X, Y: l.account.Y + (l.account.Height-7)/2}
	l.passwordText = serverPoint{X: l.password.X, Y: l.password.Y + (l.password.Height-7)/2}
}

func loginLayoutFor(renderer graphics.ShapeRenderer) loginLayout {
	width, height := loginDesignWidth, loginDesignHeight
	if provider, ok := renderer.(graphics.ViewportProvider); ok {
		if w, h := provider.ClientViewport(); w > 0 && h > 0 {
			width, height = w, h
		}
	}
	// TMSelectServerScene::InitializeScene centers the login panel directly in
	// the client viewport. Keep this in viewport pixels instead of translating
	// through an 800x600 design root; the native controls and their hitboxes then
	// remain aligned at 800x600, 1024x768, 1280x960 and larger windows.
	panel := ui.Rect{
		X:      (width - loginTextureWidth) / 2,
		Y:      (height - loginTextureHeight) / 2,
		Width:  loginTextureWidth,
		Height: loginTextureHeight,
	}
	// logo1/logo2 are the two halves of the official WYD FC mark. They are
	// deliberately kept as separate layers because each WYT contains alpha.
	// The native scene anchors them to the viewport, not to the login panel:
	// screenWidth/2-256 and screenWidth/2. Its 7.48 layout also scales the
	// ten-pixel top margin with the viewport height and adds 20/40 pixels in
	// 1024/1280-wide modes.
	logoY := (height * 10) / loginDesignHeight
	switch width {
	case 1024:
		logoY += 20
	case 1280:
		logoY += 40
	}
	return loginLayout{
		panel:        panel,
		logoLeft:     ui.Rect{X: width/2 - loginLogoWidth, Y: logoY, Width: loginLogoWidth, Height: loginLogoHeight},
		logoRight:    ui.Rect{X: width / 2, Y: logoY, Width: loginLogoWidth, Height: loginLogoHeight},
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
	controls, err := copyLoginControls(options.LoginControls)
	if err != nil {
		return nil, err
	}
	captions, err := loginCaptions(controls, options.UIStrings)
	if err != nil {
		return nil, err
	}
	textureRenderer, _ := options.ShapeRenderer.(graphics.TextureRenderer)
	placementRenderer, _ := options.ShapeRenderer.(graphics.TexturePlacementRenderer)
	layers, _ := options.ShapeRenderer.(graphics.LayeredTextureRenderer)
	return &loginScene{state: state, renderer: options.ShapeRenderer, form: ui.NewLoginForm(options.Authenticate), loginTexture: options.LoginTexture, loginLogoLeft: options.LoginLogoLeft, loginLogoRight: options.LoginLogoRight, loginControls: controls, captions: captions, textureRenderer: textureRenderer, placementRenderer: placementRenderer, layers: layers}, nil
}

// loginCaption guarda somente valores próprios e coordenadas relativas ao root.
type loginCaption struct {
	rect ui.Rect
	text string
}

// loginCaptions resolve os índices do recurso: button DWORD 9 e text DWORD 12
// (FUN_004974ec). O alinhamento com fonte bitmap é local, não paridade de fonte.
func loginCaptions(controls []assets.SceneControl, table map[int32]string) ([]loginCaption, error) {
	if table == nil {
		return nil, nil
	} // Permite testes lógicos sem renderer/assets.
	var captions []loginCaption
	for _, id := range []int32{5632, 5633, 5634, 4609, 4611, 4610} {
		c, ok := assets.FindControl(controls, id)
		kind, count := int32(12), 13
		if id < 5000 {
			kind, count = 2, 10
		}
		if !ok || c.Kind != kind || len(c.Words) != count || c.Words[1] != 4608 || c.Words[3] < 0 || c.Words[4] < 0 || c.Words[5] <= 0 || c.Words[6] <= 0 || int64(c.Words[3])+int64(c.Words[5]) > int64(loginTextureWidth) || int64(c.Words[4])+int64(c.Words[6]) > int64(loginTextureHeight) {
			return nil, fmt.Errorf("loginflow: invalid caption control %d", id)
		}
		caption, ok := table[c.Words[count-1]]
		if !ok || caption == "" {
			return nil, fmt.Errorf("loginflow: missing caption for %d", id)
		}
		captions = append(captions, loginCaption{rect: ui.Rect{X: c.Words[3], Y: c.Words[4], Width: c.Words[5], Height: c.Words[6]}, text: caption})
	}
	return captions, nil
}

// copyLoginControls protege o contrato já consumido pela cena. Recurso presente
// e inválido é erro, não autorização para misturar controles de dois layouts.
// A cópia profunda pertence à cena; mutar as opções depois não altera hitboxes.
func copyLoginControls(controls []assets.SceneControl) ([]assets.SceneControl, error) {
	if len(controls) == 0 {
		return nil, nil // Cenas sem assets continuam disponíveis para testes headless.
	}
	want := map[int32]struct{ kind, words, parent int32 }{
		4608: {1, 10, 0}, 5121: {13, 46, 4608},
		5122: {13, 46, 4608}, 4609: {2, 10, 4608},
	}
	seen := make(map[int32]bool, len(controls))
	owned := make([]assets.SceneControl, len(controls))
	for i, c := range controls {
		id, ok := c.ID()
		if !ok || seen[id] {
			return nil, fmt.Errorf("loginflow: missing or duplicate control ID %d", id)
		}
		seen[id] = true
		if expected, required := want[id]; required {
			if c.Kind != expected.kind || len(c.Words) != int(expected.words) || c.Words[1] != expected.parent || c.Words[5] <= 0 || c.Words[6] <= 0 {
				return nil, fmt.Errorf("loginflow: invalid login control %d", id)
			}
			if id == 4608 {
				if c.Words[5] != loginTextureWidth || c.Words[6] != loginTextureHeight {
					return nil, errors.New("loginflow: login panel does not match texture crop")
				}
			} else if c.Words[3] < 0 || c.Words[4] < 0 || int64(c.Words[3])+int64(c.Words[5]) > int64(loginTextureWidth) || int64(c.Words[4])+int64(c.Words[6]) > int64(loginTextureHeight) {
				return nil, fmt.Errorf("loginflow: control %d exceeds login panel", id)
			}
		}
		owned[i] = assets.SceneControl{Kind: c.Kind, Words: append([]int32(nil), c.Words...)}
	}
	for id := range want {
		if !seen[id] {
			return nil, fmt.Errorf("loginflow: required login control %d is missing", id)
		}
	}
	return owned, nil
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
	s.syncLayout()
	return s.form.HandleEvent(event)
}

// syncLayout atualiza hitboxes antes do input, inclusive antes do primeiro
// Render e após resize. Não guarda coordenadas de um viewport anterior.
func (s *loginScene) syncLayout() loginLayout {
	layout := loginLayoutFor(s.renderer)
	layout.applyNativeControls(s.loginControls)
	s.form.AccountRect, s.form.PasswordRect, s.form.ButtonRect = layout.account, layout.password, layout.submit
	return layout
}

func (s *loginScene) Update(time.Duration) error { return s.validate() }
func (s *loginScene) Render() error {
	if err := s.validate(); err != nil {
		return err
	}
	layout := s.syncLayout()
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
	if text, ok := s.renderer.(graphics.TextRenderer); ok {
		for _, caption := range s.captions {
			r := caption.rect
			// Fonte bitmap 5x7, avanço 6: centrar a tinta, não a baseline.
			x := layout.panel.X + r.X + (r.Width-(int32(len([]rune(caption.text)))*6-1))/2
			y := layout.panel.Y + r.Y + (r.Height-7)/2
			text.DrawText(x, y, caption.text, 12, graphics.Color{R: 1, G: 1, B: 1, A: 1})
		}
	}
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
