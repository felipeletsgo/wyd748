package loginflow

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
)

func TestSceneFactoriesValidatePhaseOnEnterAndUpdate(t *testing.T) {
	state := login.NewSessionState()
	factories := SceneFactories(state)
	scene, err := factories[LoginSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := scene.Enter(); err != nil {
		t.Fatal(err)
	}
	if err := scene.Update(time.Millisecond); err != nil {
		t.Fatal(err)
	}
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if err := scene.Update(0); err != nil {
		t.Fatal(err)
	}
	_ = scene.Close()
}

func TestVisualLoginFactoryUsesControlsAndSubmitCallback(t *testing.T) {
	state := login.NewSessionState()
	calls := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{Authenticate: func(string, []byte) error { calls++; return nil }})
	s, err := factories[LoginSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	_ = s.HandleEvent(input.Event{Kind: input.KindText, Rune: 'a'})
	_ = s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x09})
	_ = s.HandleEvent(input.Event{Kind: input.KindText, Rune: 'b'})
	if err := s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err != nil {
		t.Fatal(err)
	}
	if calls != 1 {
		t.Fatalf("submit calls=%d want 1", calls)
	}
	_ = s.Close()
}

func TestCharacterSceneRejectsIncompatiblePhase(t *testing.T) {
	state := login.NewSessionState()
	factory := SceneFactories(state)[CharacterSelectSceneID]
	s, err := factory()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err == nil {
		t.Fatal("character-select scene accepted Disconnected")
	}
	_ = s.Close()
}

func TestVisualCharacterSceneRejectsSubmitWhenAllSlotsAreEmpty(t *testing.T) {
	state := login.NewSessionState()
	if err := state.BeginConnect(); err != nil {
		t.Fatal(err)
	}
	if _, err := state.BeginAuthentication(login.AccountLoginRequest{Account: "account", Password: []byte("password")}); err != nil {
		t.Fatal(err)
	}
	if err := state.AcceptCharacterList(emptyCharacterListPacket()); err != nil {
		t.Fatal(err)
	}
	calls := 0
	factories := SceneFactoriesWithVisuals(state, VisualOptions{SelectCharacter: func(int32) error {
		calls++
		return nil
	}})
	s, err := factories[CharacterSelectSceneID]()
	if err != nil {
		t.Fatal(err)
	}
	if err := s.Enter(); err != nil {
		t.Fatal(err)
	}
	err = s.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D})
	if !errors.Is(err, errNoCharacterSelected) {
		t.Fatalf("submit error=%v want=%v", err, errNoCharacterSelected)
	}
	if calls != 0 {
		t.Fatalf("select callback calls=%d want 0", calls)
	}
	_ = s.Close()
}

func emptyCharacterListPacket() protocol.Packet {
	raw := make([]byte, login.CharacterListPacketSize)
	binary.LittleEndian.PutUint16(raw[0:2], uint16(len(raw)))
	raw[2] = 0x5A
	raw[3] = 0xA5
	binary.LittleEndian.PutUint16(raw[4:6], login.OpcodeCharacterList)
	return protocol.Packet{
		Header: protocol.Header{Size: uint16(len(raw)), KeyWord: raw[2], CheckSum: raw[3], Type: login.OpcodeCharacterList},
		Raw:    raw,
		Body:   raw[protocol.HeaderSize:],
	}
}
