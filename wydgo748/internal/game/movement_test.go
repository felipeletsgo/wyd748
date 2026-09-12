package game

import (
	"encoding/binary"
	"testing"
)

func TestActionTarget748UsesDestinationInsteadOfSegmentOrigin(t *testing.T) {
	pkt := make([]byte, 52)
	binary.LittleEndian.PutUint16(pkt[12:14], 2112)
	binary.LittleEndian.PutUint16(pkt[14:16], 2088)
	binary.LittleEndian.PutUint16(pkt[24:26], 2140)
	binary.LittleEndian.PutUint16(pkt[26:28], 2101)

	x, y := actionTarget748(pkt)
	if x != 2140 || y != 2101 {
		t.Fatalf("destino 7.48=(%d,%d), esperado (2140,2101)", x, y)
	}
}
