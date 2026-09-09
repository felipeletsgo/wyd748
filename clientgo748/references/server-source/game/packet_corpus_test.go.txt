package game

import (
	"encoding/hex"
	"os"
	"strings"
	"testing"

	"wydgo/internal/wire"
)

func readPacketCorpusHex(t *testing.T, name string) []byte {
	t.Helper()
	raw, err := os.ReadFile("testdata/packets/" + name)
	if err != nil {
		t.Fatal(err)
	}
	lines := strings.Split(string(raw), "\n")
	var payload strings.Builder
	for _, line := range lines {
		if comment := strings.IndexByte(line, '#'); comment >= 0 {
			line = line[:comment]
		}
		payload.WriteString(line)
	}
	packet, err := hex.DecodeString(strings.Join(strings.Fields(payload.String()), ""))
	if err != nil {
		t.Fatalf("decode %s: %v", name, err)
	}
	return packet
}

func TestReal748PacketCorpus(t *testing.T) {
	t.Run("captured movement axis", func(t *testing.T) {
		packet := readPacketCorpusHex(t, "move_0x366_52.hex")
		header := wire.ParseHeader(packet)
		if header.Size != 52 || header.Type != wire.OpAction || len(packet) != 52 {
			t.Fatalf("header=%+v len=%d", header, len(packet))
		}
		x, y := int(2486), int(2017)
		for _, encoded := range packet[28:30] {
			direction, ok := routeDirections[encoded]
			if !ok {
				t.Fatalf("unknown captured route direction %q", encoded)
			}
			x += direction[0]
			y += direction[1]
		}
		if x != 2487 || y != 2015 {
			t.Fatalf("captured route ended at (%d,%d)", x, y)
		}
	})
}
