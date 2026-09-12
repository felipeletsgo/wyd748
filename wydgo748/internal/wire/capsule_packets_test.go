package wire

import (
	"encoding/binary"
	"testing"
)

func TestCapsuleInfoPacketUsesNativeResponseLayout(t *testing.T) {
	info := CapsuleInfoData{
		Class: 26, Level: 90, Str: 123, Int: 456, Dex: 789, Con: 999,
		Mastery: [2]uint16{10, 20},
		Skill:   [9]uint16{1, 2, 3, 4, 5, 6, 7, 8, 9},
		Quest:   0x1000,
	}
	pkt := CNFCapsuleInfo(0x4321, 0x1234, info)
	if len(pkt) != 52 || ParseHeader(pkt).Type != OpCNFCapsuleInfo {
		t.Fatalf("cabecalho da capsula: len=%d type=%#x", len(pkt), ParseHeader(pkt).Type)
	}
	if got := ParseHeader(pkt).ID; got != 0x4321 {
		t.Fatalf("session id=%#x", got)
	}
	if got := binary.LittleEndian.Uint32(pkt[12:16]); got != 0x1234 {
		t.Fatalf("index=%#x", got)
	}
	if got := binary.LittleEndian.Uint16(pkt[16:18]); got != 26 {
		t.Fatalf("class=%d", got)
	}
	if got := binary.LittleEndian.Uint16(pkt[32:34]); got != 1 {
		t.Fatalf("skill[0]=%d", got)
	}
	if got := binary.LittleEndian.Uint16(pkt[48:50]); got != 9 {
		t.Fatalf("skill[8]=%d", got)
	}
	if got := binary.LittleEndian.Uint16(pkt[50:52]); got != 0x1000 {
		t.Fatalf("quest=%#x", got)
	}
}
