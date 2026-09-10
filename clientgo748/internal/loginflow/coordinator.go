// Package loginflow composes the login state machine with the application
// scene navigator. It contains no rendering, socket, or platform ownership.
package loginflow

import (
	"errors"
	"fmt"

	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
	"wydclient748/internal/scene"
	"wydclient748/internal/world"
)

// Scene IDs are stable logical names; their visual implementation is supplied
// by the application.
const (
	ServerSelectionSceneID scene.ID = "server-selection"
	LoginSceneID           scene.ID = "login"
	CharacterSelectSceneID scene.ID = "character-select"
	LoadingSceneID         scene.ID = "loading"
	WorldSceneID           scene.ID = "world"
)

// Navigator is the smallest scene-manager capability needed by the flow.
type Navigator interface {
	CurrentScene() (scene.ID, bool)
	RequestScene(scene.ID) error
}

// Coordinator is the sole composition point for login packets and scene
// synchronization. Controller emits requests; Dispatcher consumes packets.
type Coordinator struct {
	state           *login.SessionState
	controller      *login.Controller
	dispatcher      *login.Dispatcher
	worldState      *world.State
	worldDispatcher *world.Dispatcher
	movement        *world.MovementController
	navigator       Navigator
	// serverSelected is local bootstrap state. The native 7.48 selector keeps
	// the transport disconnected while it reveals the login controls; the
	// connection starts only after the account form is submitted.
	serverSelected bool
}

type Options struct {
	Controller login.ControllerOptions
	Movement   world.MovementOptions
}

func New(state *login.SessionState, sender login.Sender, navigator Navigator, options Options) (*Coordinator, error) {
	if state == nil {
		return nil, errors.New("loginflow: session state is required")
	}
	if navigator == nil {
		return nil, errors.New("loginflow: navigator is required")
	}
	controller, err := login.NewController(state, sender, options.Controller)
	if err != nil {
		return nil, fmt.Errorf("loginflow: create controller: %w", err)
	}
	dispatcher, err := login.NewDispatcher(state)
	if err != nil {
		return nil, fmt.Errorf("loginflow: create dispatcher: %w", err)
	}
	worldState := world.NewState()
	worldDispatcher, err := world.NewDispatcher(worldState)
	if err != nil {
		return nil, fmt.Errorf("loginflow: create world dispatcher: %w", err)
	}
	movement, err := world.NewMovementController(state, sender, options.Movement)
	if err != nil {
		return nil, fmt.Errorf("loginflow: create movement controller: %w", err)
	}
	return &Coordinator{state: state, controller: controller, dispatcher: dispatcher, worldState: worldState, worldDispatcher: worldDispatcher, movement: movement, navigator: navigator}, nil
}

func (c *Coordinator) Authenticate(account string, password []byte, adapter [4]uint32, id uint16) error {
	if c == nil || c.controller == nil {
		return errors.New("loginflow: coordinator is not initialized")
	}
	return c.controller.Authenticate(account, password, adapter, id)
}

func (c *Coordinator) SelectCharacter(slot int32, force int32, id uint16) error {
	if c == nil || c.controller == nil {
		return errors.New("loginflow: coordinator is not initialized")
	}
	return c.controller.SelectCharacter(slot, force, id)
}

func (c *Coordinator) Logout() error {
	if c == nil || c.controller == nil {
		return errors.New("loginflow: coordinator is not initialized")
	}
	return c.controller.Logout()
}

// Move envia somente uma intenção; posição, rota e colisão permanecem no
// servidor e retornam pelos snapshots de mundo.
func (c *Coordinator) Move(targetX, targetY uint16) error {
	if c == nil || c.movement == nil {
		return errors.New("loginflow: movement controller is not initialized")
	}
	return c.movement.Move(targetX, targetY)
}

// SessionConnected starts a fresh transport session. Any stale state must
// have been disconnected before the transport is connected again.
func (c *Coordinator) SessionConnected() error {
	if c == nil || c.state == nil {
		return errors.New("loginflow: coordinator is not initialized")
	}
	return c.state.BeginConnect()
}

// ServerSelected records the endpoint confirmation without opening the
// transport. This mirrors TMSelectServerScene: B_SERVER_SEL_OK changes the
// visible UI, while B_LOGIN_OK performs the actual socket connect.
func (c *Coordinator) ServerSelected() error {
	if c == nil || c.state == nil {
		return errors.New("loginflow: coordinator is not initialized")
	}
	if c.state.Phase() != login.Disconnected {
		return fmt.Errorf("loginflow: server selection requires Disconnected, got %s", c.state.Phase())
	}
	c.serverSelected = true
	return nil
}

func (c *Coordinator) SessionDisconnected() {
	if c == nil || c.state == nil {
		return
	}
	c.state.Disconnect()
	c.worldState.Reset()
	c.serverSelected = false
}

func (c *Coordinator) HandleSessionEvent(event protocol.SessionEvent) (bool, error) {
	if c == nil || c.dispatcher == nil {
		return false, errors.New("loginflow: coordinator is not initialized")
	}
	if event.Kind == protocol.SessionDisconnected {
		c.worldState.Reset()
		// O contrato local exige nova seleção após desconexão, inclusive
		// quando o evento chega pela rede sem o callback de Close da aplicação.
		c.serverSelected = false
		return c.dispatcher.HandleSessionEvent(event)
	}
	if handled, err := c.dispatcher.HandleSessionEvent(event); handled || err != nil {
		return handled, err
	}
	if event.Kind == protocol.SessionPacket {
		return c.worldDispatcher.HandlePacket(event.Packet)
	}
	return false, nil
}

// WorldEntities devolve cópias do snapshot atual para a cena/renderizador.
func (c *Coordinator) WorldEntities() []world.Entity {
	if c == nil || c.worldState == nil {
		return nil
	}
	return c.worldState.Snapshot()
}

// Synchronize requests at most one transition and uses the final state seen
// after all packets in the frame have been dispatched.
func (c *Coordinator) Synchronize() error {
	if c == nil || c.state == nil || c.navigator == nil {
		return errors.New("loginflow: coordinator is not initialized")
	}
	want := sceneForPhase(c.state.Phase(), c.serverSelected)
	if want == "" {
		return nil
	}
	current, ok := c.navigator.CurrentScene()
	if ok && current == want {
		return nil
	}
	if err := c.navigator.RequestScene(want); err != nil {
		return fmt.Errorf("loginflow: request %s scene: %w", want, err)
	}
	return nil
}

func sceneForPhase(phase login.Phase, serverSelected bool) scene.ID {
	switch phase {
	case login.Disconnected, login.Connecting, login.Authenticating:
		if phase == login.Disconnected {
			if serverSelected {
				return LoginSceneID
			}
			return ServerSelectionSceneID
		}
		return LoginSceneID
	case login.CharacterSelect:
		return CharacterSelectSceneID
	case login.EnteringWorld:
		return LoadingSceneID
	case login.InWorld, login.LoggingOut:
		return WorldSceneID
	default:
		return ""
	}
}
