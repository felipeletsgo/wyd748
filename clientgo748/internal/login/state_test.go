package login

import (
	"encoding/binary"
	"errors"
	"testing"
)

func TestSessionStateLoginLogoutAndRelogin(t *testing.T) {
	state := NewSessionState()
	if state.Phase() != Disconnected {
		t.Fatalf("initial phase=%s", state.Phase())
	}
	if err := state.BeginConnect(); err != nil {
		t.Fatalf("BeginConnect: %v", err)
	}
	loginPacket, err := state.BeginAuthentication(AccountLoginRequest{
		Account: "account", Password: []byte("password"), ID: 7, Tick: 10,
	})
	if err != nil {
		t.Fatalf("BeginAuthentication: %v", err)
	}
	if state.Phase() != Authenticating || binary.LittleEndian.Uint16(loginPacket[4:6]) != OpcodeAccountLogin {
		t.Fatalf("authentication phase=%s packet=% X", state.Phase(), loginPacket)
	}

	listPacket := characterListFixture()
	if err := state.AcceptCharacterList(listPacket); err != nil {
		t.Fatalf("AcceptCharacterList: %v", err)
	}
	if state.Phase() != CharacterSelect {
		t.Fatalf("character-list phase=%s", state.Phase())
	}
	selectionPacket, err := state.BeginCharacterLogin(2, -3, 9, 11)
	if err != nil {
		t.Fatalf("BeginCharacterLogin: %v", err)
	}
	if state.Phase() != EnteringWorld || int32(binary.LittleEndian.Uint32(selectionPacket[12:16])) != 2 ||
		int32(binary.LittleEndian.Uint32(selectionPacket[16:20])) != -3 {
		t.Fatalf("selection phase=%s packet=% X", state.Phase(), selectionPacket)
	}
	if pending, ok := state.PendingSlot(); !ok || pending != 2 {
		t.Fatalf("pending=%d ok=%v", pending, ok)
	}

	if err := state.AcceptEnterWorld(enterWorldFixture(2, 0x3456)); err != nil {
		t.Fatalf("AcceptEnterWorld: %v", err)
	}
	world, ok := state.World()
	if !ok || state.Phase() != InWorld || world.ClientID != 0x3456 || world.Mob.Name != "WorldHero" {
		t.Fatalf("world ok=%v phase=%s snapshot=%+v", ok, state.Phase(), world)
	}
	logoutPacket, err := state.BeginCharacterLogout(12)
	if err != nil {
		t.Fatalf("BeginCharacterLogout: %v", err)
	}
	if state.Phase() != LoggingOut || binary.LittleEndian.Uint16(logoutPacket[4:6]) != OpcodeCharacterLogout ||
		binary.LittleEndian.Uint16(logoutPacket[6:8]) != 0x3456 {
		t.Fatalf("logout phase=%s packet=% X", state.Phase(), logoutPacket)
	}
	confirmation := decodedPacket(OpcodeCharacterLogoutConfirmed, CharacterLogoutConfirmedPacketSize, 0x3456, 13)
	if err := state.AcceptCharacterLogout(confirmation); err != nil {
		t.Fatalf("AcceptCharacterLogout: %v", err)
	}
	if state.Phase() != CharacterSelect {
		t.Fatalf("post-logout phase=%s", state.Phase())
	}
	if _, ok := state.World(); ok {
		t.Fatal("world survived character logout")
	}
	list, ok := state.CharacterList()
	if !ok || list.Characters[2].Name != "CharacterTwo" {
		t.Fatalf("character list not retained: ok=%v list=%+v", ok, list)
	}

	if _, err := state.BeginCharacterLogin(1, 0, 9, 14); err != nil {
		t.Fatalf("relogin selection: %v", err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(1, 0x4567)); err != nil {
		t.Fatalf("relogin world: %v", err)
	}
	world, ok = state.World()
	if !ok || world.Slot != 1 || world.ClientID != 0x4567 {
		t.Fatalf("relogin reused old snapshot: ok=%v world=%+v", ok, world)
	}
}

func TestSessionStateRollbackAndInvalidPacketsDoNotMutate(t *testing.T) {
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{}); !errors.Is(err, ErrInvalidFixedText) {
		t.Fatalf("invalid credentials error=%v", err)
	}
	if state.Phase() != Connecting {
		t.Fatalf("invalid credentials changed phase=%s", state.Phase())
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{Account: "a", Password: []byte("b")}); err != nil {
		t.Fatal(err)
	}

	badList := characterListFixture()
	badList.Header.Type = OpcodeEnterWorld
	if err := state.AcceptCharacterList(badList); !errors.Is(err, ErrUnexpectedOpcode) {
		t.Fatalf("bad list error=%v", err)
	}
	if state.Phase() != Authenticating {
		t.Fatalf("bad list changed phase=%s", state.Phase())
	}
	if _, ok := state.CharacterList(); ok {
		t.Fatal("bad list was published")
	}
	if err := state.AcceptCharacterList(characterListFixture()); err != nil {
		t.Fatal(err)
	}

	if _, err := state.BeginCharacterLogin(0, 0, 0, 0); !errors.Is(err, ErrEmptySlot) {
		t.Fatalf("empty slot error=%v", err)
	}
	if state.Phase() != CharacterSelect {
		t.Fatalf("empty slot changed phase=%s", state.Phase())
	}
	for _, slot := range []int32{-1, SelectionCharacterCount} {
		if _, err := state.BeginCharacterLogin(slot, 0, 0, 0); !errors.Is(err, ErrInvalidSlot) {
			t.Fatalf("slot %d error=%v", slot, err)
		}
	}

	if _, err := state.BeginCharacterLogin(2, 0, 0, 0); err != nil {
		t.Fatal(err)
	}
	mismatch := enterWorldFixture(1, 0x1111)
	if err := state.AcceptEnterWorld(mismatch); !errors.Is(err, ErrSlotMismatch) {
		t.Fatalf("slot mismatch error=%v", err)
	}
	if state.Phase() != EnteringWorld {
		t.Fatalf("slot mismatch changed phase=%s", state.Phase())
	}
	if _, ok := state.World(); ok {
		t.Fatal("slot mismatch published world")
	}
	if pending, ok := state.PendingSlot(); !ok || pending != 2 {
		t.Fatalf("slot mismatch changed pending=%d ok=%v", pending, ok)
	}
	badWorld := enterWorldFixture(2, 0x2222)
	badWorld.Raw = badWorld.Raw[:len(badWorld.Raw)-1]
	badWorld.Body = badWorld.Raw[12:]
	if err := state.AcceptEnterWorld(badWorld); !errors.Is(err, ErrInvalidPacketSize) {
		t.Fatalf("bad world error=%v", err)
	}
	if err := state.CancelCharacterLogin(); err != nil || state.Phase() != CharacterSelect {
		t.Fatalf("CancelCharacterLogin error=%v phase=%s", err, state.Phase())
	}

	if _, err := state.BeginCharacterLogin(2, 0, 0, 0); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(2, 0x2222)); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginCharacterLogout(1); err != nil {
		t.Fatal(err)
	}
	wrongID := decodedPacket(OpcodeCharacterLogoutConfirmed, CharacterLogoutConfirmedPacketSize, 0x3333, 2)
	if err := state.AcceptCharacterLogout(wrongID); err == nil {
		t.Fatal("wrong logout ID was accepted")
	}
	if state.Phase() != LoggingOut {
		t.Fatalf("wrong logout ID changed phase=%s", state.Phase())
	}
	if world, ok := state.World(); !ok || world.ClientID != 0x2222 {
		t.Fatalf("wrong logout ID changed world: ok=%v world=%+v", ok, world)
	}
	if err := state.CancelCharacterLogout(); err != nil || state.Phase() != InWorld {
		t.Fatalf("CancelCharacterLogout error=%v phase=%s", err, state.Phase())
	}
}

func TestSessionStateRejectsOutOfOrderAndDuplicateEvents(t *testing.T) {
	state := NewSessionState()
	if err := state.AcceptCharacterList(characterListFixture()); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("out-of-order list error=%v", err)
	}
	if _, err := state.BeginCharacterLogin(1, 0, 0, 0); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("out-of-order selection error=%v", err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(1, 1)); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("out-of-order world error=%v", err)
	}
	if _, err := state.BeginCharacterLogout(0); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("out-of-order logout error=%v", err)
	}

	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if err := state.BeginConnect(); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("duplicate connect error=%v", err)
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{Account: "a", Password: []byte("b")}); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{Account: "a", Password: []byte("b")}); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("duplicate authentication error=%v", err)
	}
	if err := state.AcceptCharacterList(characterListFixture()); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptCharacterList(characterListFixture()); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("duplicate list error=%v", err)
	}
}

func TestSessionStateDisconnectAndAccessorsOwnCopies(t *testing.T) {
	state := NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(AccountLoginRequest{Account: "a", Password: []byte("b")}); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptCharacterList(characterListFixture()); err != nil {
		t.Fatal(err)
	}
	list, ok := state.CharacterList()
	if !ok {
		t.Fatal("missing list")
	}
	list.SecretCode[0] = 0
	list.Characters[1].Equip[0].Index = 0
	again, _ := state.CharacterList()
	if again.SecretCode[0] != 0x80 || again.Characters[1].Equip[0].Index != 2000 {
		t.Fatal("CharacterList accessor leaked mutable state")
	}

	if _, err := state.BeginCharacterLogin(1, 0, 0, 0); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptEnterWorld(enterWorldFixture(1, 0x4444)); err != nil {
		t.Fatal(err)
	}
	world, ok := state.World()
	if !ok {
		t.Fatal("missing world")
	}
	world.Mob.Carry[0].Index = 0
	againWorld, _ := state.World()
	if againWorld.Mob.Carry[0].Index != 6000 {
		t.Fatal("World accessor leaked mutable state")
	}

	state.Disconnect()
	state.Disconnect()
	if state.Phase() != Disconnected {
		t.Fatalf("disconnect phase=%s", state.Phase())
	}
	if _, ok := state.CharacterList(); ok {
		t.Fatal("character list survived disconnect")
	}
	if _, ok := state.World(); ok {
		t.Fatal("world survived disconnect")
	}
	if _, ok := state.PendingSlot(); ok {
		t.Fatal("pending slot survived disconnect")
	}
}

func TestNilSessionStateIsSafe(t *testing.T) {
	var state *SessionState
	if state.Phase() != Disconnected {
		t.Fatalf("nil phase=%s", state.Phase())
	}
	if err := state.BeginConnect(); !errors.Is(err, ErrInvalidState) {
		t.Fatalf("nil BeginConnect error=%v", err)
	}
	state.Disconnect()
	if _, ok := state.CharacterList(); ok {
		t.Fatal("nil state returned list")
	}
	if _, ok := state.World(); ok {
		t.Fatal("nil state returned world")
	}
}
