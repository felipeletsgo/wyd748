package wire

import (
	"bytes"
	"strings"
	"testing"
)

func TestCreateMobTabContract(t *testing.T) {
	for color := byte(0); color < 10; color++ {
		packet := CreateMobWithGuildRank(1, "Player", 100, 101, nil, nil, nil, nil, 2, 7, 9, 50, 6)
		before := append([]byte(nil), packet...)
		PutCreateMobTab(packet, strings.Repeat("x", 96), color)
		if len(packet) != 328 || !bytes.Equal(packet[:300], before[:300]) || packet[326] != 6 {
			t.Fatal("tab changed position, score, equipment, guild or cape metadata")
		}
		expectedColor := color
		if color > 8 {
			expectedColor = 0
		}
		if string(packet[300:325]) != strings.Repeat("x", 25) || packet[325] != 0 || packet[327] != expectedColor {
			t.Fatalf("invalid tab termination/color: %x", packet[300:])
		}
		PutCreateMobTab(packet, "", 0)
		if !bytes.Equal(packet[300:326], make([]byte, 26)) || packet[327] != 0 {
			t.Fatal("tab clear retained old bytes")
		}
	}
	for _, packet := range [][]byte{
		nil, Build(OpCreateMob, 0, 327), Build(OpCreateMobTrade, 0, 328),
		CreateMobTrade(1, "Shop", 100, 101, nil, nil, "A shop title"),
	} {
		before := append([]byte(nil), packet...)
		PutCreateMobTab(packet, "Changed", 8)
		if !bytes.Equal(before, packet) {
			t.Fatal("tab writer corrupted a short frame or trade title")
		}
	}
}
