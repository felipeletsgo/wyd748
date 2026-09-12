package wire

import (
	"bytes"
	"encoding/binary"
	"errors"
	"testing"
)

func TestReadPacketRejectsFramesOutsideNativeLimit(t *testing.T) {
	for _, size := range []uint16{0, HeaderSize - 1, MaxPacketSize + 1, 65535} {
		var header [2]byte
		binary.LittleEndian.PutUint16(header[:], size)
		if _, _, err := ReadPacket(bytes.NewReader(header[:])); !errors.Is(err, ErrBadSize) {
			t.Fatalf("size=%d: erro=%v, esperado ErrBadSize", size, err)
		}
	}
}

func TestReadPacketAcceptsNativeMaximum(t *testing.T) {
	packet := make([]byte, MaxPacketSize)
	binary.LittleEndian.PutUint16(packet[:2], MaxPacketSize)
	Encrypt(packet, 7)
	got, checksumOK, err := ReadPacket(bytes.NewReader(packet))
	if err != nil || !checksumOK || len(got) != MaxPacketSize {
		t.Fatalf("frame maximo: len=%d checksum=%v err=%v", len(got), checksumOK, err)
	}
}

func FuzzReadPacketNeverPanics(f *testing.F) {
	f.Add([]byte{})
	f.Add([]byte{12, 0})
	f.Add([]byte{0xFF, 0xFF, 1, 2, 3})
	f.Fuzz(func(t *testing.T, packet []byte) {
		_, _, _ = ReadPacket(bytes.NewReader(packet))
	})
}
