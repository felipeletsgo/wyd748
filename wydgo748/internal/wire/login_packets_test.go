package wire

import (
	"encoding/binary"
	"testing"
)

func TestSelectionFailureSignalsUseHeaderOnly748Envelope(t *testing.T) {
	tests := []struct {
		name   string
		packet []byte
		opcode uint16
	}{
		{name: "already playing", packet: AlreadyPlaying(), opcode: OpAlreadyPlaying},
		{name: "new character failed", packet: NewCharacterFail(), opcode: OpNewCharacterFail},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if len(tt.packet) != HeaderSize {
				t.Fatalf("tamanho = %d, esperado %d", len(tt.packet), HeaderSize)
			}
			if got := binary.LittleEndian.Uint16(tt.packet[4:6]); got != tt.opcode {
				t.Fatalf("opcode = %#x, esperado %#x", got, tt.opcode)
			}
		})
	}
}
