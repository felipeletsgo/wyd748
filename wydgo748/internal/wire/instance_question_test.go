package wire

import (
	"encoding/binary"
	"strings"
	"testing"
)

func TestInstanceQuestionContract(t *testing.T) {
	for _, tc := range []struct {
		text string
		ms   uint32
		want uint32
	}{
		{"Question?", 10000, 10000}, {strings.Repeat("A", 120), 20000, 10000},
		{"", 10000, 0}, {"Question?", 0, 0},
	} {
		p := InstanceQuestion(17, tc.text, tc.ms)
		FinishPacket(p, 1)
		if !Decrypt(p) {
			t.Fatal("question did not survive transport encryption")
		}
		h := ParseHeader(p)
		if len(p) != 116 || h.Size != 116 || h.Type != 0x7F12 || h.ID != 17 ||
			binary.LittleEndian.Uint16(p[12:14]) != 1 || binary.LittleEndian.Uint16(p[14:16]) != 0 ||
			binary.LittleEndian.Uint32(p[16:20]) != tc.want || p[115] != 0 {
			t.Fatalf("invalid v1 question packet: %x", p)
		}
		if tc.want != 0 && string(p[20:20+min(len(tc.text), 95)]) != tc.text[:min(len(tc.text), 95)] {
			t.Fatal("question text was not preserved")
		}
		if tc.want == 0 && p[20] != 0 {
			t.Fatal("close packet retained question text")
		}
	}
}
