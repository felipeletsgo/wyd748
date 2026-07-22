package wire

import (
	"encoding/binary"
	"testing"
)

func TestPartyRequestLayout748(t *testing.T) {
	b := PartyRequest(7, "Lider", 2, 65, 900, 1200, 8)
	if len(b) != 44 || binary.LittleEndian.Uint16(b[4:6]) != OpPartyRequest {
		t.Fatalf("header do convite incorreto")
	}
	if b[12] != 2 || b[13] != 0 || binary.LittleEndian.Uint16(b[14:16]) != 65 ||
		binary.LittleEndian.Uint16(b[16:18]) != 1200 || binary.LittleEndian.Uint16(b[18:20]) != 900 ||
		binary.LittleEndian.Uint16(b[20:22]) != 7 || string(b[22:27]) != "Lider" ||
		binary.LittleEndian.Uint32(b[40:44]) != 8 {
		t.Fatalf("layout do convite incorreto: %v", b)
	}
}

func TestPartyMemberAndRemoveLayouts748(t *testing.T) {
	b := PartyMember(9, "Membro", 3, 2, 50, 700, 1000)
	if len(b) != 40 || binary.LittleEndian.Uint16(b[4:6]) != OpPartyAdd || b[12] != 3 || b[13] != 2 ||
		binary.LittleEndian.Uint16(b[20:22]) != 9 {
		t.Fatalf("layout de membro incorreto: %v", b)
	}
	remove := PartyRemove(9)
	if len(remove) != 16 || binary.LittleEndian.Uint16(remove[4:6]) != OpPartyRemove ||
		binary.LittleEndian.Uint32(remove[12:16]) != 9 {
		t.Fatalf("layout de remocao incorreto: %v", remove)
	}
}
