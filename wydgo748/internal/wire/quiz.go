package wire

import (
	"encoding/binary"
	"errors"
)

// EXTENSAO_COORDENADA v1. Never accept legacy 0x2C7 as an event answer.
const (
	OpQuizChallenge   = 0x7F10
	OpQuizAnswer      = 0x7F11
	QuizChallengeSize = 148
	QuizAnswerSize    = 36
)

func QuizChallenge(id uint16, token [16]byte, question string, answers [4]int32) []byte {
	b := Build(OpQuizChallenge, id, QuizChallengeSize)
	binary.LittleEndian.PutUint16(b[12:14], 1)
	binary.LittleEndian.PutUint16(b[14:16], 1) // show; zero means close
	copy(b[16:32], token[:])
	binary.LittleEndian.PutUint32(b[32:36], 10000)
	copy(b[36:131], question)
	for i, a := range answers {
		binary.LittleEndian.PutUint32(b[132+i*4:136+i*4], uint32(a))
	}
	return b
}
func QuizClose(id uint16, token [16]byte) []byte {
	b := Build(OpQuizChallenge, id, QuizChallengeSize)
	binary.LittleEndian.PutUint16(b[12:14], 1)
	copy(b[16:32], token[:])
	return b
}

type QuizAnswerData struct {
	ID     uint16
	Token  [16]byte
	Choice uint16
}

func ParseQuizAnswer(b []byte) (QuizAnswerData, error) {
	var r QuizAnswerData
	if len(b) != QuizAnswerSize {
		return r, errors.New("quiz size")
	}
	h := ParseHeader(b)
	if h.Size != QuizAnswerSize || h.Type != OpQuizAnswer || binary.LittleEndian.Uint16(b[12:14]) != 1 ||
		binary.LittleEndian.Uint16(b[14:16]) > 3 || binary.LittleEndian.Uint32(b[32:36]) != 0 {
		return r, errors.New("quiz envelope")
	}
	r.ID = h.ID
	r.Choice = binary.LittleEndian.Uint16(b[14:16])
	copy(r.Token[:], b[16:32])
	return r, nil
}
