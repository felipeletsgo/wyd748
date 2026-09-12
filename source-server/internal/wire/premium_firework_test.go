package wire

import (
	"bytes"
	"testing"
)

func TestPremiumFireworkPacketMatchesClient748Layout(t *testing.T) {
	pattern := [16]byte{0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81, 0x55, 0xAA, 0x0F, 0xF0, 0x0F}
	pkt := PremiumFirework(77, pattern)
	header := ParseHeader(pkt)
	if len(pkt) != 36 || header.Type != OpPremiumFirework || header.ID != 77 {
		t.Fatalf("header Premium Firework invalido: len=%d header=%+v", len(pkt), header)
	}
	if !bytes.Equal(pkt[20:36], pattern[:]) {
		t.Fatalf("bitmap em offset incorreto: got=% X want=% X", pkt[20:36], pattern)
	}
	for i, value := range pkt[12:20] {
		if value != 0 {
			t.Fatalf("campo reservado @%d nao zerado: %d", 12+i, value)
		}
	}
}
