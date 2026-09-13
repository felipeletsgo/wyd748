package wire

import (
	"encoding/binary"
	"testing"
)

func TestGuildWarInfoEnemyAndReset(t *testing.T) {
	pkt := GuildWarInfo(321)
	if len(pkt) != 24 || ParseHeader(pkt).Type != OpWarInfo || ParseHeader(pkt).ID != SceneField || binary.LittleEndian.Uint32(pkt[12:16]) != 321 {
		t.Fatal("guild war snapshot")
	}
	for _, v := range pkt[16:] {
		if v != 0 {
			t.Fatal("unrelated war state changed")
		}
	}
	if binary.LittleEndian.Uint32(GuildWarInfo(0)[12:16]) != 0 {
		t.Fatal("reset")
	}
}

func TestWarInfoUsesCompleteNativeSnapshot(t *testing.T) {
	pkt := WarInfo()
	header := ParseHeader(pkt)
	if len(pkt) != 24 || header.Type != OpWarInfo || header.ID != SceneField {
		t.Fatalf("WarInfo header/layout invalido: len=%d header=%+v", len(pkt), header)
	}
	for offset, value := range pkt[12:24] {
		if value != 0 {
			t.Fatalf("WarInfo payload @%d nao zerado: %d", 12+offset, value)
		}
	}
}
