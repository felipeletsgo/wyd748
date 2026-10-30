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

func TestInstanceQuestionTextUsesVersion2(t *testing.T) {
	p := InstanceQuestionText(17, 470, "Question?", 10000)
	if binary.LittleEndian.Uint16(p[12:14]) != 2 || binary.LittleEndian.Uint16(p[14:16]) != 470 ||
		string(p[20:29]) != "Question?" {
		t.Fatalf("invalid v2 question packet: %x", p)
	}
	closed := InstanceQuestionText(17, 470, "", 0)
	if binary.LittleEndian.Uint16(closed[12:14]) != 1 || binary.LittleEndian.Uint16(closed[14:16]) != 0 {
		t.Fatal("close packet must stay version 1")
	}
	if v1 := InstanceQuestionText(17, 0, "Question?", 10000); binary.LittleEndian.Uint16(v1[12:14]) != 1 {
		t.Fatal("question without a row must stay version 1")
	}
}
