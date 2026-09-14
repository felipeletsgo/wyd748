package wire

import (
	"encoding/binary"
	"testing"
)

func TestQuizWireContract(t *testing.T) {
	token := [16]byte{0xA5}
	b := QuizChallenge(7, token, "88 x 4 = ?", [4]int32{351, 352, 353, 359})
	binary.LittleEndian.PutUint16(b[0:2], uint16(len(b))) // transport finalizes before dispatch
	h := ParseHeader(b)
	if len(b) != 148 || h.Size != 148 || h.Type != 0x7F10 || h.ID != 7 || b[12] != 1 || b[14] != 1 || b[16] != 0xA5 ||
		binary.LittleEndian.Uint32(b[32:36]) != 10000 || string(b[36:46]) != "88 x 4 = ?" || b[46] != 0 || binary.LittleEndian.Uint32(b[136:140]) != 352 {
		t.Fatal("challenge ABI")
	}
	close := QuizClose(7, token)
	if close[14] != 0 || close[12] != 1 || close[16] != 0xA5 || len(close) != 148 {
		t.Fatal("close ABI")
	}
	answer := Build(OpQuizAnswer, 7, QuizAnswerSize)
	binary.LittleEndian.PutUint16(answer[0:2], QuizAnswerSize)
	answer[12] = 1
	answer[14] = 1
	copy(answer[16:32], token[:])
	if p, e := ParseQuizAnswer(answer); e != nil || p.ID != 7 || p.Choice != 1 || p.Token != token {
		t.Fatal(p, e)
	}
	for n := 0; n < 36; n++ {
		if _, e := ParseQuizAnswer(answer[:n]); e == nil {
			t.Fatal("truncation", n)
		}
	}
	for _, offset := range []int{0, 4, 12, 14, 32} {
		bad := append([]byte(nil), answer...)
		bad[offset] = 0xFF
		if _, e := ParseQuizAnswer(bad); e == nil {
			t.Fatal("bad envelope", offset)
		}
	}
	if _, e := ParseQuizAnswer(append(answer, 0)); e == nil {
		t.Fatal("tail")
	}
}
