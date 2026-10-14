package wire

import (
	"bytes"
	"encoding/binary"
	"testing"
)

func TestKefraPortalStateV1(t *testing.T) {
	for _, defeated := range []bool{false, true} {
		b := KefraPortalState(defeated)
		h := ParseHeader(b)
		if len(b) != 16 || h.Type != 0x7F30 || h.ID != 0x7530 ||
			binary.LittleEndian.Uint16(b[12:14]) != 1 {
			t.Fatalf("invalid Kefra portal v1 envelope: %x", b)
		}
		// Canonical client/server payload: version LE16, defeated LE16.
		want := []byte{1, 0, 0, 0}
		if defeated {
			want[2] = 1
		}
		if !bytes.Equal(b[12:], want) {
			t.Fatalf("defeated=%v payload=%x want=%x", defeated, b[12:], want)
		}
		if !bytes.Equal(b[:4], make([]byte, 4)) || !bytes.Equal(b[8:12], make([]byte, 4)) {
			t.Fatal("transport-owned header fields were filled by the builder")
		}
		fixture := "kefra_portal_0x7f30_alive.hex"
		if defeated {
			fixture = "kefra_portal_0x7f30_defeated.hex"
		}
		FinishPacket(b, 7)
		decoded, checksumOK, err := ReadPacket(bytes.NewReader(b))
		if err != nil || !checksumOK {
			t.Fatalf("portal transport roundtrip: checksum=%v err=%v", checksumOK, err)
		}
		decoded[2], decoded[3] = 0, 0
		if !bytes.Equal(decoded, loadProtocolFixture(t, fixture)) {
			t.Fatalf("portal frame differs from shared fixture %s: %x", fixture, decoded)
		}
	}
}
