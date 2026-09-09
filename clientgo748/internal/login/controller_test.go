package login

import (
	"encoding/binary"
	"errors"
	"reflect"
	"testing"
)

type sentPacket struct {
	plain []byte
	key   byte
}

type controllerSender struct {
	err      error
	calls    int
	retained []byte
	sent     []sentPacket
}

func (s *controllerSender) Send(plain []byte, key byte) error {
	s.calls++
	s.retained = plain
	s.sent = append(s.sent, sentPacket{plain: append([]byte(nil), plain...), key: key})
	return s.err
}

func TestControllerAuthenticateBuildsExpectedPacketAndClearsSecrets(t *testing.T) {
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	sender := &controllerSender{}
	controller := mustController(t, state, sender, 0x11223344, 0xA7)
	password := []byte("password")
	adapter := [4]uint32{1, 2, 3, 4}

	if err := controller.Authenticate("account", password, adapter, 0x3456); err != nil {
		t.Fatal(err)
	}
	if state.Phase() != Authenticating {
		t.Fatalf("phase=%s want Authenticating", state.Phase())
	}
	if sender.calls != 1 || len(sender.sent) != 1 {
		t.Fatalf("send calls=%d packets=%d want 1", sender.calls, len(sender.sent))
	}
	got := sender.sent[0]
	if len(got.plain) != AccountLoginPacketSize || got.key != 0xA7 {
		t.Fatalf("packet size=%d key=%02X", len(got.plain), got.key)
	}
	if opcode := binary.LittleEndian.Uint16(got.plain[4:6]); opcode != OpcodeAccountLogin {
		t.Fatalf("opcode=%03X want %03X", opcode, OpcodeAccountLogin)
	}
	if id := binary.LittleEndian.Uint16(got.plain[6:8]); id != 0x3456 {
		t.Fatalf("id=%04X want 3456", id)
	}
	if tick := binary.LittleEndian.Uint32(got.plain[8:12]); tick != 0x11223344 {
		t.Fatalf("tick=%08X want 11223344", tick)
	}
	if string(got.plain[accountOffset:accountOffset+7]) != "account" ||
		string(got.plain[passwordOffset:passwordOffset+8]) != "password" {
		t.Fatalf("credentials were not present during Send: % X", got.plain[:passwordOffset+passwordSize])
	}
	if !allZero(password) {
		t.Fatalf("password survived Authenticate: % X", password)
	}
	if !allZero(sender.retained) {
		t.Fatal("authentication packet survived Send")
	}
}

func TestControllerAuthenticateClearsSecretsAndRollsBackOnFailures(t *testing.T) {
	sendErr := errors.New("send failed")
	keyErr := errors.New("random failed")
	tests := []struct {
		name       string
		senderErr  error
		keywordErr error
		wantCalls  int
	}{
		{name: "send", senderErr: sendErr, wantCalls: 1},
		{name: "keyword", keywordErr: keyErr, wantCalls: 0},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			state := NewSessionState()
			if err := state.BeginConnect(); err != nil {
				t.Fatal(err)
			}
			sender := &controllerSender{err: test.senderErr}
			controller, err := NewController(state, sender, ControllerOptions{
				Tick: func() uint32 { return 10 },
				KeyWord: func() (byte, error) {
					return 0x22, test.keywordErr
				},
			})
			if err != nil {
				t.Fatal(err)
			}
			password := []byte("password")
			err = controller.Authenticate("account", password, [4]uint32{}, 7)
			if !errors.Is(err, test.senderErr) && !errors.Is(err, test.keywordErr) {
				t.Fatalf("error=%v want injected failure", err)
			}
			if state.Phase() != Connecting {
				t.Fatalf("phase=%s want Connecting", state.Phase())
			}
			if sender.calls != test.wantCalls {
				t.Fatalf("send calls=%d want %d", sender.calls, test.wantCalls)
			}
			if !allZero(password) {
				t.Fatalf("password survived failure: % X", password)
			}
			if sender.retained != nil && !allZero(sender.retained) {
				t.Fatal("authentication packet survived failed Send")
			}
		})
	}
}

func TestControllerAuthenticateClearsPasswordOnValidationFailure(t *testing.T) {
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	sender := &controllerSender{}
	controller := mustController(t, state, sender, 1, 2)
	password := []byte("password")
	if err := controller.Authenticate("", password, [4]uint32{}, 0); !errors.Is(err, ErrInvalidFixedText) {
		t.Fatalf("error=%v want ErrInvalidFixedText", err)
	}
	if !allZero(password) || sender.calls != 0 || state.Phase() != Connecting {
		t.Fatalf("password=% X calls=%d phase=%s", password, sender.calls, state.Phase())
	}
}

func TestControllerSelectCharacterRejectsInvalidEmptyAndDuplicateSlots(t *testing.T) {
	state := stateAtCharacterSelect(t)
	sender := &controllerSender{}
	controller := mustController(t, state, sender, 0x55667788, 0x33)

	for _, slot := range []int32{-1, SelectionCharacterCount} {
		if err := controller.SelectCharacter(slot, 0, 9); !errors.Is(err, ErrInvalidSlot) {
			t.Fatalf("slot %d error=%v want ErrInvalidSlot", slot, err)
		}
	}
	if err := controller.SelectCharacter(0, 0, 9); !errors.Is(err, ErrEmptySlot) {
		t.Fatalf("empty slot error=%v want ErrEmptySlot", err)
	}
	if sender.calls != 0 || state.Phase() != CharacterSelect {
		t.Fatalf("rejected slots sent=%d phase=%s", sender.calls, state.Phase())
	}

	if err := controller.SelectCharacter(2, -3, 9); err != nil {
		t.Fatal(err)
	}
	if sender.calls != 1 || state.Phase() != EnteringWorld {
		t.Fatalf("valid selection sent=%d phase=%s", sender.calls, state.Phase())
	}
	packet := sender.sent[0]
	if packet.key != 0x33 || binary.LittleEndian.Uint16(packet.plain[4:6]) != OpcodeCharacterLogin ||
		binary.LittleEndian.Uint16(packet.plain[6:8]) != 9 ||
		binary.LittleEndian.Uint32(packet.plain[8:12]) != 0x55667788 ||
		int32(binary.LittleEndian.Uint32(packet.plain[12:16])) != 2 ||
		int32(binary.LittleEndian.Uint32(packet.plain[16:20])) != -3 {
		t.Fatalf("unexpected character-login packet: key=%02X bytes=% X", packet.key, packet.plain)
	}
	if err := controller.SelectCharacter(1, 0, 9); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("duplicate selection error=%v want ErrInvalidState", err)
	}
	if sender.calls != 1 {
		t.Fatalf("duplicate selection sent another packet: %d", sender.calls)
	}
}

func TestControllerSelectionAndLogoutFailuresRestorePreviousState(t *testing.T) {
	sendErr := errors.New("send failed")
	state := stateAtCharacterSelect(t)
	sender := &controllerSender{err: sendErr}
	controller := mustController(t, state, sender, 20, 0x44)
	if err := controller.SelectCharacter(1, 0, 7); !errors.Is(err, sendErr) {
		t.Fatalf("selection error=%v want send failure", err)
	}
	if state.Phase() != CharacterSelect {
		t.Fatalf("selection rollback phase=%s", state.Phase())
	}
	if _, ok := state.PendingSlot(); ok {
		t.Fatal("selection rollback retained pending slot")
	}

	sender.err = nil
	if err := controller.SelectCharacter(1, 0, 7); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(1, 0xBEEF)); err != nil {
		t.Fatal(err)
	}
	sender.err = sendErr
	if err := controller.Logout(); !errors.Is(err, sendErr) {
		t.Fatalf("logout error=%v want send failure", err)
	}
	world, ok := state.World()
	if state.Phase() != InWorld || !ok || world.ClientID != 0xBEEF {
		t.Fatalf("logout rollback phase=%s ok=%v world=%+v", state.Phase(), ok, world)
	}
}

func TestControllerLoginLogoutAndReloginUsesAuthoritativeWorldID(t *testing.T) {
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	sender := &controllerSender{}
	var tick uint32 = 100
	controller, err := NewController(state, sender, ControllerOptions{
		Tick:    func() uint32 { tick++; return tick },
		KeyWord: func() (byte, error) { return byte(tick), nil },
	})
	if err != nil {
		t.Fatal(err)
	}
	password := []byte("password")
	if err := controller.Authenticate("account", password, [4]uint32{}, 0); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptCharacterList(characterListFixture()); err != nil {
		t.Fatal(err)
	}
	if err := controller.SelectCharacter(2, 0, 0); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(2, 0xCAFE)); err != nil {
		t.Fatal(err)
	}
	if err := controller.Logout(); err != nil {
		t.Fatal(err)
	}
	logout := sender.sent[2]
	if binary.LittleEndian.Uint16(logout.plain[4:6]) != OpcodeCharacterLogout ||
		binary.LittleEndian.Uint16(logout.plain[6:8]) != 0xCAFE {
		t.Fatalf("logout did not use authoritative ClientID: % X", logout.plain)
	}
	if err := state.AcceptCharacterLogout(decodedPacket(OpcodeCharacterLogoutConfirmed, CharacterLogoutConfirmedPacketSize, 0xCAFE, tick)); err != nil {
		t.Fatal(err)
	}
	if err := controller.SelectCharacter(1, 0, 0); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(1, 0xD00D)); err != nil {
		t.Fatal(err)
	}
	world, ok := state.World()
	if len(sender.sent) != 4 || !ok || world.Slot != 1 || world.ClientID != 0xD00D {
		t.Fatalf("relogin sends=%d ok=%v world=%+v", len(sender.sent), ok, world)
	}
}

func TestControllerKeywordFailureDoesNotLeaveCharacterStatePending(t *testing.T) {
	keyErr := errors.New("random failed")
	state := stateAtCharacterSelect(t)
	sender := &controllerSender{}
	controller, err := NewController(state, sender, ControllerOptions{
		Tick:    func() uint32 { return 1 },
		KeyWord: func() (byte, error) { return 0, keyErr },
	})
	if err != nil {
		t.Fatal(err)
	}
	if err := controller.SelectCharacter(1, 0, 0); !errors.Is(err, keyErr) {
		t.Fatalf("error=%v want key failure", err)
	}
	if state.Phase() != CharacterSelect || sender.calls != 0 {
		t.Fatalf("phase=%s send calls=%d", state.Phase(), sender.calls)
	}
}

func TestNewControllerValidatesDependencies(t *testing.T) {
	state := NewSessionState()
	if _, err := NewController(nil, &controllerSender{}, ControllerOptions{}); err == nil {
		t.Fatal("nil state accepted")
	}
	if _, err := NewController(state, nil, ControllerOptions{}); err == nil {
		t.Fatal("nil sender accepted")
	}
	var nilController *Controller
	password := []byte("secret")
	if err := nilController.Authenticate("account", password, [4]uint32{}, 0); err == nil {
		t.Fatal("nil controller accepted authentication")
	}
	if !allZero(password) {
		t.Fatalf("nil controller retained password: % X", password)
	}
}

func mustController(t *testing.T, state *SessionState, sender Sender, tick uint32, key byte) *Controller {
	t.Helper()
	controller, err := NewController(state, sender, ControllerOptions{
		Tick:    func() uint32 { return tick },
		KeyWord: func() (byte, error) { return key, nil },
	})
	if err != nil {
		t.Fatal(err)
	}
	return controller
}

func stateAtCharacterSelect(t *testing.T) *SessionState {
	t.Helper()
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{Account: "account", Password: []byte("password")}); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptCharacterList(characterListFixture()); err != nil {
		t.Fatal(err)
	}
	return state
}

func allZero(values []byte) bool {
	return reflect.DeepEqual(values, make([]byte, len(values)))
}
