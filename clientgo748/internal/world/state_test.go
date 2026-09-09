package world

import (
	"encoding/binary"
	"testing"

	"wydclient748/internal/protocol"
)

func packet(opcode, id uint16, size int) protocol.Packet {
	raw := make([]byte, size)
	binary.LittleEndian.PutUint16(raw[0:2], uint16(size))
	binary.LittleEndian.PutUint16(raw[4:6], opcode)
	binary.LittleEndian.PutUint16(raw[6:8], id)
	return protocol.Packet{Header: protocol.Header{Size: uint16(size), Type: opcode, ID: id}, Raw: raw, Body: raw[protocol.HeaderSize:]}
}

func TestDispatcherMaterializesUpdatesAndRemovesEntity(t *testing.T) {
	state := NewState()
	dispatcher, err := NewDispatcher(state)
	if err != nil {
		t.Fatal(err)
	}
	p := packet(OpcodeCreateMob, 7, CreateMobPacketSize)
	binary.LittleEndian.PutUint16(p.Raw[12:14], 120)
	binary.LittleEndian.PutUint16(p.Raw[14:16], 240)
	binary.LittleEndian.PutUint16(p.Raw[16:18], 7)
	copy(p.Raw[18:34], []byte("Goblin"))
	binary.LittleEndian.PutUint32(p.Raw[140+4:140+8], 15)
	if handled, err := dispatcher.HandlePacket(p); err != nil || !handled {
		t.Fatalf("create handled=%v err=%v", handled, err)
	}
	e, ok := state.Entity(7)
	if !ok || e.Name != "Goblin" || e.PosX != 120 || e.Score.Level != 15 {
		t.Fatalf("entity=%+v ok=%v", e, ok)
	}
	a := packet(OpcodeMove, 7, ActionPacketSize)
	binary.LittleEndian.PutUint16(a.Raw[12:14], 121)
	binary.LittleEndian.PutUint16(a.Raw[14:16], 241)
	if _, err := dispatcher.HandlePacket(a); err != nil {
		t.Fatal(err)
	}
	e, _ = state.Entity(7)
	if e.PosX != 121 || e.PosY != 241 {
		t.Fatalf("position=%d,%d", e.PosX, e.PosY)
	}
	r := packet(OpcodeRemoveMob, 7, RemoveMobPacketSize)
	binary.LittleEndian.PutUint32(r.Raw[12:16], 1)
	if _, err := dispatcher.HandlePacket(r); err != nil {
		t.Fatal(err)
	}
	if _, ok := state.Entity(7); ok {
		t.Fatal("entity survived removal")
	}
}

func TestDispatcherRejectsWrongSizeBeforeMutation(t *testing.T) {
	dispatcher, _ := NewDispatcher(NewState())
	p := packet(OpcodeCreateMob, 7, CreateMobPacketSize-1)
	if handled, err := dispatcher.HandlePacket(p); !handled || err == nil {
		t.Fatalf("handled=%v err=%v", handled, err)
	}
}

func TestTradeTitleIsPreservedOnBaseRefresh(t *testing.T) {
	state := NewState()
	dispatcher, _ := NewDispatcher(state)
	trade := packet(OpcodeCreateMobTrade, 9, CreateMobTradePacketSize)
	binary.LittleEndian.PutUint16(trade.Raw[16:18], 9)
	copy(trade.Raw[326:350], []byte("Shop"))
	if _, err := dispatcher.HandlePacket(trade); err != nil {
		t.Fatal(err)
	}
	base := packet(OpcodeCreateMob, 9, CreateMobPacketSize)
	binary.LittleEndian.PutUint16(base.Raw[16:18], 9)
	if _, err := dispatcher.HandlePacket(base); err != nil {
		t.Fatal(err)
	}
	e, _ := state.Entity(9)
	if e.TradeTitle != "Shop" {
		t.Fatalf("title=%q", e.TradeTitle)
	}
}
