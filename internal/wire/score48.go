package wire

import (
	"encoding/binary"
	"math"

	"wydgo/internal/model"
)

// ClientScore48Size is the exact size of STRUCT_SCORE used by the source-built
// client. It is a wire projection only; ExtendedScore remains authoritative.
const ClientScore48Size = 48

func clampScoreInt16(value uint32) int16 {
	if value > math.MaxInt16 {
		return math.MaxInt16
	}
	return int16(value)
}

func clampScoreInt32(value uint32) int32 {
	if value > math.MaxInt32 {
		return math.MaxInt32
	}
	return int32(value)
}

func clampScoreUint16(value uint32) uint16 {
	if value > math.MaxUint16 {
		return math.MaxUint16
	}
	return uint16(value)
}

// EncodeClientScore48 projects ExtendedScore into the source client's native
// 48-byte score. Attributes absent from this layout continue in XSC2.
//
// This encoder intentionally has no decoder: score values sent by the client
// are never accepted as authoritative gameplay state.
func EncodeClientScore48(ext *model.ExtendedScore) [ClientScore48Size]byte {
	var score [ClientScore48Size]byte
	if ext == nil {
		return score
	}

	binary.LittleEndian.PutUint16(score[0:2], uint16(clampScoreInt16(ext.Level)))
	binary.LittleEndian.PutUint32(score[4:8], uint32(clampScoreInt32(ext.Defense)))
	binary.LittleEndian.PutUint32(score[8:12], uint32(clampScoreInt32(ext.Attack)))
	score[12] = ext.Merchant
	score[13] = ext.AttackRun
	binary.LittleEndian.PutUint32(score[16:20], uint32(clampScoreInt32(ext.MaxHP)))
	binary.LittleEndian.PutUint32(score[20:24], uint32(clampScoreInt32(ext.MaxMP)))
	binary.LittleEndian.PutUint32(score[24:28], uint32(clampScoreInt32(ext.CurHP)))
	binary.LittleEndian.PutUint32(score[28:32], uint32(clampScoreInt32(ext.CurMP)))
	binary.LittleEndian.PutUint16(score[32:34], uint16(clampScoreInt16(ext.Str)))
	binary.LittleEndian.PutUint16(score[34:36], uint16(clampScoreInt16(ext.Int)))
	binary.LittleEndian.PutUint16(score[36:38], uint16(clampScoreInt16(ext.Dex)))
	binary.LittleEndian.PutUint16(score[38:40], uint16(clampScoreInt16(ext.Con)))
	for i, mastery := range ext.Mastery {
		binary.LittleEndian.PutUint16(score[40+i*2:42+i*2], clampScoreUint16(mastery))
	}

	return score
}
