package wire

import "testing"

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
