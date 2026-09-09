package wire

import (
	"encoding/binary"
	"errors"
	"testing"
)

func TestClientIntegrityChallengeMatchesNativeLayout(t *testing.T) {
	got := ClientIntegrityChallenge(0x1234, -7, 0x1020304)
	if len(got) != ClientIntegrityPacketSize {
		t.Fatalf("tamanho=%d", len(got))
	}
	want := make([]byte, ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint16(want[4:6], OpClientIntegrityChallenge)
	binary.LittleEndian.PutUint16(want[6:8], 0x1234)
	negativeCategory := int32(-7)
	binary.LittleEndian.PutUint32(want[12:16], uint32(negativeCategory))
	binary.LittleEndian.PutUint32(want[16:20], 0x1020304)
	if string(got) != string(want) {
		t.Fatalf("layout divergente\n got=% X\nwant=% X", got, want)
	}
}

func TestParseClientIntegrityResponse(t *testing.T) {
	pkt := make([]byte, ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint16(pkt[0:2], ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint16(pkt[4:6], OpClientIntegrityResponse)
	binary.LittleEndian.PutUint16(pkt[6:8], 321)
	binary.LittleEndian.PutUint32(pkt[12:16], 9)
	binary.LittleEndian.PutUint32(pkt[16:20], 340)
	negativeValue := int32(-45)
	binary.LittleEndian.PutUint32(pkt[20:24], uint32(negativeValue))

	got, err := ParseClientIntegrityResponse(pkt)
	if err != nil {
		t.Fatal(err)
	}
	if got.ID != 321 || got.Category != 9 || got.ByteOffset != 340 || got.Value != -45 {
		t.Fatalf("resposta=%+v", got)
	}
}

func TestParseClientIntegrityResponseRejectsInvalidEnvelope(t *testing.T) {
	valid := make([]byte, ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint16(valid[0:2], ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint16(valid[4:6], OpClientIntegrityResponse)

	tests := map[string][]byte{
		"short":  valid[:ClientIntegrityPacketSize-1],
		"size":   append([]byte(nil), valid...),
		"opcode": append([]byte(nil), valid...),
	}
	binary.LittleEndian.PutUint16(tests["size"][0:2], ClientIntegrityPacketSize-1)
	binary.LittleEndian.PutUint16(tests["opcode"][4:6], OpPing)
	for name, pkt := range tests {
		t.Run(name, func(t *testing.T) {
			if _, err := ParseClientIntegrityResponse(pkt); !errors.Is(err, ErrInvalidClientIntegrityResponse) {
				t.Fatalf("erro=%v", err)
			}
		})
	}
}
