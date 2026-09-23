package game

import (
	"encoding/binary"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCharacterTransferUnavailablePreservesAccountAndRepliesWithNativeError(t *testing.T) {
	w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe", Gold: 1234})
	before := append([]model.Char(nil), p.Account.Chars...)
	request := inboundPacket(wire.OpCharacterTransfer, 52)
	binary.LittleEndian.PutUint32(request[16:20], 0)
	copy(request[20:36], "Felipe")
	copy(request[36:52], "Felipe")
	w.handle(command{s: session, pkt: request})
	if !reflect.DeepEqual(p.Account.Chars, before) || p.InWorld || p.CharSlot != -1 {
		t.Fatal("rejeicao da transferencia alterou personagem ou sessao")
	}
	if session.QueuedPacketsForTest() != 1 {
		t.Fatalf("esperada uma resposta, recebidas %d", session.QueuedPacketsForTest())
	}
	response, ok := session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(response) || len(response) != 52 ||
		wire.ParseHeader(response).Type != wire.OpCharacterTransfer ||
		binary.LittleEndian.Uint32(response[12:16]) != 4 ||
		binary.LittleEndian.Uint32(response[16:20]) != 0 {
		t.Fatal("resposta 0xFAA nao corresponde ao erro generico do client")
	}
}

func TestCharacterTransferUnavailableRejectsMalformedAndWorldReplay(t *testing.T) {
	w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe"})
	for _, size := range []int{51, 53} {
		if w.validateInboundCommand(session, inboundPacket(wire.OpCharacterTransfer, size)) {
			t.Fatalf("0xFAA com %d bytes foi aceito", size)
		}
	}
	for _, slot := range []uint32{4, ^uint32(0)} {
		badSlot := inboundPacket(wire.OpCharacterTransfer, 52)
		binary.LittleEndian.PutUint32(badSlot[16:20], slot)
		w.onCharacterTransferUnavailable(session, badSlot)
		if session.QueuedPacketsForTest() != 0 {
			t.Fatalf("slot invalido %d recebeu resposta", slot)
		}
	}
	badResult := inboundPacket(wire.OpCharacterTransfer, 52)
	binary.LittleEndian.PutUint32(badResult[12:16], 1)
	w.onCharacterTransferUnavailable(session, badResult)
	if session.QueuedPacketsForTest() != 0 {
		t.Fatal("pacote com Result de resposta foi aceito como pedido")
	}
	valid := inboundPacket(wire.OpCharacterTransfer, 52)
	p.InWorld = true
	if w.validateInboundCommand(session, valid) {
		t.Fatal("transferencia repetida no mundo foi aceita")
	}
	if session.QueuedPacketsForTest() != 0 || p.Account.Chars[0].Name != "Felipe" {
		t.Fatal("pedido recusado alterou fila ou conta")
	}
}
