package wire

import (
	"encoding/binary"
	"math"
	"testing"

	"wydgo/internal/model"
)

func TestEncodeClientScore48Layout(t *testing.T) {
	ext := &model.ExtendedScore{
		Version:   model.ExtendedScoreVersion,
		Level:     400,
		Defense:   123_456,
		Attack:    654_321,
		Merchant:  0x31,
		AttackRun: 0x66,
		MaxHP:     4_100_000,
		MaxMP:     3_200_000,
		CurHP:     3_900_000,
		CurMP:     2_700_000,
		Str:       4_000,
		Int:       3_500,
		Dex:       3_200,
		Con:       2_800,
		Mastery:   [4]uint32{255, 1_024, 65_535, 70_000},
	}

	score := EncodeClientScore48(ext)
	if len(score) != ClientScore48Size {
		t.Fatalf("score size = %d; want %d", len(score), ClientScore48Size)
	}
	if got := int16(binary.LittleEndian.Uint16(score[0:2])); got != 400 {
		t.Fatalf("level = %d; want 400", got)
	}
	if score[2] != 0 || score[3] != 0 || score[14] != 0 || score[15] != 0 {
		t.Fatalf("alignment padding was not zeroed: %v %v", score[2:4], score[14:16])
	}
	if got := int32(binary.LittleEndian.Uint32(score[4:8])); got != 123_456 {
		t.Fatalf("defense = %d; want 123456", got)
	}
	if got := int32(binary.LittleEndian.Uint32(score[8:12])); got != 654_321 {
		t.Fatalf("attack = %d; want 654321", got)
	}
	if score[12] != 0x31 || score[13] != 0x66 {
		t.Fatalf("merchant/attackRun = %02X/%02X", score[12], score[13])
	}
	for offset, want := range map[int]int32{
		16: 4_100_000,
		20: 3_200_000,
		24: 3_900_000,
		28: 2_700_000,
	} {
		if got := int32(binary.LittleEndian.Uint32(score[offset : offset+4])); got != want {
			t.Fatalf("int32 @%d = %d; want %d", offset, got, want)
		}
	}
	for offset, want := range map[int]int16{32: 4_000, 34: 3_500, 36: 3_200, 38: 2_800} {
		if got := int16(binary.LittleEndian.Uint16(score[offset : offset+2])); got != want {
			t.Fatalf("int16 @%d = %d; want %d", offset, got, want)
		}
	}
	for i, want := range []uint16{255, 1_024, 65_535, 65_535} {
		offset := 40 + i*2
		if got := binary.LittleEndian.Uint16(score[offset : offset+2]); got != want {
			t.Fatalf("mastery[%d] = %d; want %d", i, got, want)
		}
	}
}

func TestEncodeClientScore48ClampsSignedFields(t *testing.T) {
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		Level:   math.MaxUint32,
		Defense: math.MaxUint32,
		Attack:  math.MaxUint32,
		Str:     math.MaxUint32,
	}

	score := EncodeClientScore48(ext)
	if got := int16(binary.LittleEndian.Uint16(score[0:2])); got != math.MaxInt16 {
		t.Fatalf("level clamp = %d; want %d", got, int16(math.MaxInt16))
	}
	if got := int32(binary.LittleEndian.Uint32(score[4:8])); got != math.MaxInt32 {
		t.Fatalf("defense clamp = %d; want %d", got, int32(math.MaxInt32))
	}
	if got := int32(binary.LittleEndian.Uint32(score[8:12])); got != math.MaxInt32 {
		t.Fatalf("attack clamp = %d; want %d", got, int32(math.MaxInt32))
	}
	if got := int16(binary.LittleEndian.Uint16(score[32:34])); got != math.MaxInt16 {
		t.Fatalf("str clamp = %d; want %d", got, int16(math.MaxInt16))
	}
}

func TestEncodeClientScore48NilIsZero(t *testing.T) {
	if got := EncodeClientScore48(nil); got != [ClientScore48Size]byte{} {
		t.Fatalf("nil score encoded non-zero bytes: %v", got)
	}
}
