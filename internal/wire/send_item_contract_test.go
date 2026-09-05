package wire

import (
	"bytes"
	"testing"

	"wydgo/internal/model"
)

// Fixture tambem consumido pelo teste C++ da fronteira de recepcao. Campos
// internos do item (UID/datas) nunca podem ampliar os oito bytes do payload.
func TestSendItemLegacyFrame(t *testing.T) {
	item := model.Item{Index: 0x1234, Eff: [6]byte{1, 2, 3, 4, 5, 6},
		UID: "server-only", ActivatedUnix: 123, ExpiresUnix: 456}
	want := []byte{24, 0, 0, 0, 0x82, 1, 0x34, 0x12,
		0, 0, 0, 0, 2, 0, 127, 0, 0x34, 0x12, 1, 2, 3, 4, 5, 6}
	packet := SendItem(0x1234, 2, 127, item)
	// O builder deixa Size zerado; quem enquadra e cifra e o transporte.
	FinishPacket(packet, 7)
	got, checksumOK, err := ReadPacket(bytes.NewReader(packet))
	if err != nil || !checksumOK {
		t.Fatalf("SendItem roundtrip: checksum=%v err=%v", checksumOK, err)
	}
	// O fixture plaintext normaliza somente os dois bytes da cifragem.
	got[2], got[3] = 0, 0
	if !bytes.Equal(got, want) {
		t.Fatalf("SendItem wire mismatch: got % x, want % x", got, want)
	}
}
