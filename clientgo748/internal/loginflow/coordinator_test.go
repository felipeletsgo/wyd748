package loginflow

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
	"wydclient748/internal/scene"
	"wydclient748/internal/world"
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

// A desconexão de rede deve limpar a seleção sem depender do fechamento da
// aplicação; repetir o evento não pode impedir a próxima tentativa de login.
func TestCoordinatorNetworkDisconnectRequiresServerReselection(t *testing.T) {
	state := login.NewSessionState()
	navigator := &fakeNavigator{current: LoginSceneID}
	c, err := New(state, fakeSender{}, navigator, Options{})
	if err != nil {
		t.Fatal(err)
	}
	if err := c.ServerSelected(); err != nil {
		t.Fatal(err)
	}
	if err := c.SessionConnected(); err != nil {
		t.Fatal(err)
	}
	for i := 0; i < 2; i++ {
		handled, err := c.HandleSessionEvent(protocol.SessionEvent{Kind: protocol.SessionDisconnected})
		if err != nil || !handled {
			t.Fatalf("disconnect %d: handled=%v err=%v", i, handled, err)
		}
		if state.Phase() != login.Disconnected || c.serverSelected {
			t.Fatalf("disconnect %d: phase=%s selected=%v", i, state.Phase(), c.serverSelected)
		}
	}
	if err := c.Synchronize(); err != nil {
		t.Fatal(err)
	}
	if len(navigator.requests) != 1 || navigator.requests[0] != ServerSelectionSceneID {
		t.Fatalf("requests=%v, want server selection", navigator.requests)
	}
	navigator.current = ServerSelectionSceneID
	if err := c.ServerSelected(); err != nil {
		t.Fatal(err)
	}
	if err := c.Synchronize(); err != nil {
		t.Fatal(err)
	}
	if len(navigator.requests) != 2 || navigator.requests[1] != LoginSceneID {
		t.Fatalf("requests=%v, want login after reselection", navigator.requests)
	}
	if err := c.SessionConnected(); err != nil {
		t.Fatalf("reconnect: %v", err)
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

func TestCoordinatorRoutesWorldPacketsOutsideLoginState(t *testing.T) {
	state := login.NewSessionState()
	navigator := &fakeNavigator{current: WorldSceneID}
	coordinator, err := New(state, fakeSender{}, navigator, Options{})
	if err != nil {
		t.Fatal(err)
	}
	raw := make([]byte, world.CreateMobPacketSize)
	binary.LittleEndian.PutUint16(raw[0:2], uint16(len(raw)))
	binary.LittleEndian.PutUint16(raw[4:6], world.OpcodeCreateMob)
	binary.LittleEndian.PutUint16(raw[6:8], 42)
	binary.LittleEndian.PutUint16(raw[16:18], 42)
	event := protocol.SessionEvent{Kind: protocol.SessionPacket, Packet: protocol.Packet{Header: protocol.Header{Size: uint16(len(raw)), Type: world.OpcodeCreateMob, ID: 42}, Raw: raw, Body: raw[protocol.HeaderSize:]}}
	handled, err := coordinator.HandleSessionEvent(event)
	if err != nil || !handled {
		t.Fatalf("handled=%v err=%v", handled, err)
	}
	entities := coordinator.WorldEntities()
	if len(entities) != 1 || entities[0].ID != 42 {
		t.Fatalf("entities=%+v", entities)
	}
}

// O cache de entidades pertence à entrada no mundo, não à conexão da conta.
func TestCoordinatorLogoutClearsEntitiesOnlyAfterValidConfirmation(t *testing.T) {
	state := login.NewSessionState()
	c, err := New(state, fakeSender{}, &fakeNavigator{current: WorldSceneID}, Options{})
	if err != nil {
		t.Fatal(err)
	}
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(login.AccountLoginRequest{Account: "account", Password: []byte("password")}); err != nil {
		t.Fatal(err)
	}
	list := emptyCharacterListPacket()
	copy(list.Raw[48:64], "Hero")
	if err := state.AcceptCharacterList(list); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginCharacterLogin(0, 0, 0, 0); err != nil {
		t.Fatal(err)
	}
	packet := func(op uint16, size int, id uint16) protocol.Packet {
		raw := protocol.NewPacket(op, id, 0, make([]byte, size-protocol.HeaderSize))
		binary.LittleEndian.PutUint16(raw[:2], uint16(size))
		return protocol.Packet{Header: protocol.Header{Size: uint16(size), Type: op, ID: id}, Raw: raw, Body: raw[protocol.HeaderSize:]}
	}
	if err := state.AcceptEnterWorld(packet(login.OpcodeEnterWorld, login.EnterWorldPacketSize, 0)); err != nil {
		t.Fatal(err)
	}
	mob := packet(world.OpcodeCreateMob, world.CreateMobPacketSize, 42)
	binary.LittleEndian.PutUint16(mob.Raw[16:18], 42)
	if _, err := c.HandleSessionEvent(protocol.SessionEvent{Kind: protocol.SessionPacket, Packet: mob}); err != nil {
		t.Fatal(err)
	}
	if len(c.WorldEntities()) != 1 {
		t.Fatal("missing entity before logout")
	}
	if _, err := state.BeginCharacterLogout(0); err != nil {
		t.Fatal(err)
	}
	invalid := packet(login.OpcodeCharacterLogoutConfirmed, login.CharacterLogoutConfirmedPacketSize, 1)
	if _, err := c.HandleSessionEvent(protocol.SessionEvent{Kind: protocol.SessionPacket, Packet: invalid}); err == nil {
		t.Fatal("accepted wrong character ID")
	}
	if len(c.WorldEntities()) != 1 {
		t.Fatal("rejected confirmation cleared entities")
	}
	valid := packet(login.OpcodeCharacterLogoutConfirmed, login.CharacterLogoutConfirmedPacketSize, 0)
	if _, err := c.HandleSessionEvent(protocol.SessionEvent{Kind: protocol.SessionPacket, Packet: valid}); err != nil {
		t.Fatal(err)
	}
	if len(c.WorldEntities()) != 0 {
		t.Fatal("entities survived confirmed logout")
	}
	if state.Phase() != login.CharacterSelect {
		t.Fatal("logout did not return to character selection")
	}
	if _, err := state.BeginCharacterLogin(0, 0, 0, 0); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptEnterWorld(packet(login.OpcodeEnterWorld, login.EnterWorldPacketSize, 0)); err != nil {
		t.Fatal(err)
	}
	if len(c.WorldEntities()) != 0 {
		t.Fatal("relogin inherited old entities")
	}
}
