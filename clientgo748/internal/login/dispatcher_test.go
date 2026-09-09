package login

import (
	"errors"
	"testing"

	"wydclient748/internal/protocol"
)

func TestDispatcherCompletesLoginLogoutAndReloginTransitions(t *testing.T) {
	state := authenticatedState(t)
	dispatcher, err := NewDispatcher(state)
	if err != nil {
		t.Fatal(err)
	}

	assertHandledPacket(t, dispatcher, characterListFixture())
	if _, err := state.BeginCharacterLogin(2, 0, 1, 2); err != nil {
		t.Fatal(err)
	}
	assertHandledPacket(t, dispatcher, enterWorldFixture(2, 0x3456))
	if _, err := state.BeginCharacterLogout(3); err != nil {
		t.Fatal(err)
	}
	assertHandledPacket(t, dispatcher, decodedPacket(OpcodeCharacterLogoutConfirmed, CharacterLogoutConfirmedPacketSize, 0x3456, 4))

	if state.Phase() != CharacterSelect {
		t.Fatalf("post-logout phase=%s", state.Phase())
	}
	if _, ok := state.World(); ok {
		t.Fatal("world survived dispatcher logout")
	}
	if _, err := state.BeginCharacterLogin(1, 0, 5, 6); err != nil {
		t.Fatal(err)
	}
	assertHandledPacket(t, dispatcher, enterWorldFixture(1, 0x4567))
	world, ok := state.World()
	if !ok || world.Slot != 1 || world.ClientID != 0x4567 {
		t.Fatalf("relogin world ok=%v snapshot=%+v", ok, world)
	}
}

func TestDispatcherRejectsOutOfOrderAndClientDirectionPackets(t *testing.T) {
	state := NewSessionState()
	dispatcher, err := NewDispatcher(state)
	if err != nil {
		t.Fatal(err)
	}
	handled, err := dispatcher.HandlePacket(characterListFixture())
	if !handled || !errors.Is(err, ErrInvalidState) {
		t.Fatalf("out-of-order handled=%v error=%v", handled, err)
	}

	for _, opcode := range []uint16{OpcodeAccountLogin, OpcodeCharacterLogin, OpcodeCharacterLogout} {
		handled, err = dispatcher.HandlePacket(protocol.Packet{Header: protocol.Header{Type: opcode}})
		if !handled || !errors.Is(err, ErrUnexpectedPacketDirection) {
			t.Fatalf("client opcode 0x%03X handled=%v error=%v", opcode, handled, err)
		}
	}
}

func TestDispatcherLeavesUnknownPacketsForFutureDomains(t *testing.T) {
	dispatcher, err := NewDispatcher(NewSessionState())
	if err != nil {
		t.Fatal(err)
	}
	handled, err := dispatcher.HandlePacket(protocol.Packet{Header: protocol.Header{Type: 0x777}})
	if err != nil || handled {
		t.Fatalf("unknown packet handled=%v error=%v", handled, err)
	}
}

func TestDispatcherDisconnectClearsAuthenticatedAndWorldState(t *testing.T) {
	state := authenticatedState(t)
	dispatcher, err := NewDispatcher(state)
	if err != nil {
		t.Fatal(err)
	}
	assertHandledPacket(t, dispatcher, characterListFixture())
	if _, err := state.BeginCharacterLogin(1, 0, 0, 0); err != nil {
		t.Fatal(err)
	}
	assertHandledPacket(t, dispatcher, enterWorldFixture(1, 0x1234))

	handled, err := dispatcher.HandleSessionEvent(protocol.SessionEvent{
		Kind: protocol.SessionDisconnected,
		Err:  errors.New("connection reset"),
	})
	if err != nil || !handled {
		t.Fatalf("disconnect handled=%v error=%v", handled, err)
	}
	if state.Phase() != Disconnected {
		t.Fatalf("disconnect phase=%s", state.Phase())
	}
	if _, ok := state.CharacterList(); ok {
		t.Fatal("character list survived disconnect")
	}
	if _, ok := state.World(); ok {
		t.Fatal("world survived disconnect")
	}
}

func TestDispatcherRejectsInvalidConstructionAndEventKind(t *testing.T) {
	if _, err := NewDispatcher(nil); err == nil {
		t.Fatal("NewDispatcher accepted nil state")
	}
	dispatcher, err := NewDispatcher(NewSessionState())
	if err != nil {
		t.Fatal(err)
	}
	if handled, err := dispatcher.HandleSessionEvent(protocol.SessionEvent{Kind: 99}); err == nil || handled {
		t.Fatalf("unknown event handled=%v error=%v", handled, err)
	}
	var nilDispatcher *Dispatcher
	if handled, err := nilDispatcher.HandlePacket(protocol.Packet{}); err == nil || handled {
		t.Fatalf("nil dispatcher handled=%v error=%v", handled, err)
	}
}

func authenticatedState(t *testing.T) *SessionState {
	t.Helper()
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{Account: "account", Password: []byte("password")}); err != nil {
		t.Fatal(err)
	}
	return state
}

func assertHandledPacket(t *testing.T, dispatcher *Dispatcher, packet protocol.Packet) {
	t.Helper()
	handled, err := dispatcher.HandleSessionEvent(protocol.SessionEvent{Kind: protocol.SessionPacket, Packet: packet})
	if err != nil || !handled {
		t.Fatalf("opcode 0x%03X handled=%v error=%v", packet.Header.Type, handled, err)
	}
}
