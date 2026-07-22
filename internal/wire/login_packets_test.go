package wire

import (
	"encoding/binary"
	"testing"
)

func TestAlreadyPlayingIsNative748Signal(t *testing.T) {
	b := AlreadyPlaying()
	if len(b) != 12 || binary.LittleEndian.Uint16(b[4:6]) != OpAlreadyPlaying {
		t.Fatalf("pacote AlreadyPlaying invalido: %v", b)
	}
}
