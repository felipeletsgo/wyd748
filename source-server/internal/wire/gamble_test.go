package wire

import (
	"encoding/binary"
	"testing"
)

func TestResultGambleNative748Layout(t *testing.T) {
	result := [5]byte{1, 2, 3, 4, 5}
	stops := [3]byte{21, 0, 7}
	pkt := ResultGamble(0x1234, result, stops, -7, 0x89ABCDEF)
	if len(pkt) != 36 || binary.LittleEndian.Uint16(pkt[4:6]) != OpResultGamble ||
		binary.LittleEndian.Uint16(pkt[6:8]) != 0x1234 {
		t.Fatalf("header/layout incorrect: len=%d type=%04X id=%04X", len(pkt),
			binary.LittleEndian.Uint16(pkt[4:6]), binary.LittleEndian.Uint16(pkt[6:8]))
	}
	if got := [5]byte{pkt[12], pkt[13], pkt[14], pkt[15], pkt[16]}; got != result {
		t.Fatalf("result offset incorrect: %v", got)
	}
	if got := [3]byte{pkt[17], pkt[18], pkt[19]}; got != stops {
		t.Fatalf("stop offset incorrect: %v", got)
	}
	for i, value := range pkt[20:28] {
		if value != 0 {
			t.Fatalf("reserved byte %d is nonzero: %d", i, value)
		}
	}
	if int32(binary.LittleEndian.Uint32(pkt[28:32])) != -7 ||
		binary.LittleEndian.Uint32(pkt[32:36]) != 0x89ABCDEF {
		t.Fatalf("tail fields incorrect: prize=%d jackpot=%08X",
			int32(binary.LittleEndian.Uint32(pkt[28:32])), binary.LittleEndian.Uint32(pkt[32:36]))
	}
}
