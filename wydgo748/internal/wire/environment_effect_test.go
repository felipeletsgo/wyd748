package wire

import (
	"bytes"
	"testing"
)

func TestEnvironmentEffectNative748Layout(t *testing.T) {
	b := EnvironmentEffect([4]uint16{3436, 1302, 3450, 1317}, 32)
	h := ParseHeader(b)
	if len(b) != 24 || h.Type != 0x3A2 || h.ID != 30000 {
		t.Fatalf("invalid native environment-effect envelope: %x", b)
	}
	// FUN_00489618 reads the rectangle at +12/+14/+16/+18 and effect at +20.
	want := []byte{0x6c, 0x0d, 0x16, 0x05, 0x7a, 0x0d, 0x25, 0x05, 32, 0, 0, 0}
	if !bytes.Equal(b[12:], want) {
		t.Fatalf("environment-effect payload=%x want=%x", b[12:], want)
	}
}
