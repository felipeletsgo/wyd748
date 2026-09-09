package world

import (
	"encoding/binary"
	"testing"

	"wydclient748/internal/login"
)

type movementSender struct {
	packet []byte
	key    byte
	err    error
}

func (s *movementSender) Send(packet []byte, key byte) error {
	s.packet = append([]byte(nil), packet...)
	s.key = key
	return s.err
}

func TestBuildMovementPacketUsesAuthoritativeOriginAndTarget(t *testing.T) {
	packet := buildMovementPacket(login.WorldSnapshot{ClientID: 0xCAFE, PosX: 2450, PosY: 1777}, 2500, 1800, 0x11223344)
	if len(packet) != ActionPacketSize {
		t.Fatalf("packet size=%d want %d", len(packet), ActionPacketSize)
	}
	if got := binary.LittleEndian.Uint16(packet[4:6]); got != OpcodeMove {
		t.Fatalf("opcode=%03X want %03X", got, OpcodeMove)
	}
	if got := binary.LittleEndian.Uint16(packet[6:8]); got != 0xCAFE {
		t.Fatalf("client id=%04X", got)
	}
	if got := binary.LittleEndian.Uint32(packet[8:12]); got != 0x11223344 {
		t.Fatalf("tick=%08X", got)
	}
	if got := binary.LittleEndian.Uint16(packet[12:14]); got != 2450 {
		t.Fatalf("origin x=%d", got)
	}
	if got := binary.LittleEndian.Uint16(packet[14:16]); got != 1777 {
		t.Fatalf("origin y=%d", got)
	}
	if got := binary.LittleEndian.Uint16(packet[24:26]); got != 2500 {
		t.Fatalf("target x=%d", got)
	}
	if got := binary.LittleEndian.Uint16(packet[26:28]); got != 1800 {
		t.Fatalf("target y=%d", got)
	}
}

func TestMovementRequiresWorldPhase(t *testing.T) {
	state := login.NewSessionState()
	sender := &movementSender{}
	controller, err := NewMovementController(state, sender, MovementOptions{
		Tick:    func() uint32 { return 1 },
		KeyWord: func() (byte, error) { return 2, nil },
	})
	if err != nil {
		t.Fatal(err)
	}
	if err := controller.Move(1, 2); err == nil {
		t.Fatal("movement accepted outside InWorld")
	}
	if sender.packet != nil {
		t.Fatal("movement sent outside InWorld")
	}
}

func TestMovementControllerValidatesDependencies(t *testing.T) {
	state := login.NewSessionState()
	if _, err := NewMovementController(nil, &movementSender{}, MovementOptions{}); err == nil {
		t.Fatal("nil state accepted")
	}
	if _, err := NewMovementController(state, nil, MovementOptions{}); err == nil {
		t.Fatal("nil sender accepted")
	}
}
