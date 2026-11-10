package wire

import (
	"encoding/binary"
	"testing"
)

func validDropQuery() []byte {
	b := Build(OpDropListQuery, 1, DropListQuerySize)
	binary.LittleEndian.PutUint16(b[0:2], DropListQuerySize)
	binary.LittleEndian.PutUint16(b[12:14], 1)
	binary.LittleEndian.PutUint32(b[16:20], 77)
	copy(b[24:], "Skeleton")
	return b
}

func TestDropListQueryRejectsMalformedFrames(t *testing.T) {
	b := validDropQuery()
	if q, ok := ParseDropListQuery(b); !ok || q.Query != "Skeleton" || q.ID != 77 {
		t.Fatal("valid request rejected")
	}
	for n := 0; n < len(b); n++ {
		if _, ok := ParseDropListQuery(b[:n]); ok {
			t.Fatalf("truncated length %d accepted", n)
		}
	}
	for _, mutate := range []func([]byte){
		func(p []byte) { p[12] = 2 }, func(p []byte) { p[14] = 2 }, func(p []byte) { p[15] = 1 },
		func(p []byte) { p[16] = 0 }, func(p []byte) { p[20] = 1 }, func(p []byte) { p[35] = 1 },
		func(p []byte) { p[88] = 2 }, func(p []byte) { p[14] = 1; p[903] = 128 },
	} {
		p := append([]byte(nil), b...)
		mutate(p)
		if _, ok := ParseDropListQuery(p); ok {
			t.Fatal("malformed request accepted")
		}
	}
	b[14] = 1
	b[88+6499/8] |= 1 << uint(6499%8)
	if _, ok := ParseDropListQuery(b); !ok {
		t.Fatal("highest item ID rejected")
	}
}

func TestDropListResultWireLayout(t *testing.T) {
	q := DropListQuery{ID: 9, Mode: 1, Offset: 10}
	b := DropListResult(q, 21, 32, 0, []DropListRow{{Item: 412, Flags: 1, Base: 1000, Current: 2000, Monster: "Skeleton"}})
	if len(b) != 716 || binary.LittleEndian.Uint16(b[4:6]) != OpDropListResult || binary.LittleEndian.Uint16(b[6:8]) != SceneField ||
		binary.LittleEndian.Uint32(b[20:24]) != 10 || binary.LittleEndian.Uint16(b[32:34]) != 1 ||
		binary.LittleEndian.Uint16(b[36:38]) != 412 || string(b[56:64]) != "Skeleton" {
		t.Fatal("result ABI changed")
	}
}
