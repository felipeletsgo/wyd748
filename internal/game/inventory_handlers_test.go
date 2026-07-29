package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func deleteItemPacket(slot, index uint32) []byte {
	pkt := make([]byte, deleteItemPacketSize)
	binary.LittleEndian.PutUint32(pkt[12:16], slot)
	binary.LittleEndian.PutUint32(pkt[16:20], index)
	return pkt
}

func splitItemPacket(slot, index, amount uint32) []byte {
	pkt := make([]byte, splitItemPacketSize)
	binary.LittleEndian.PutUint32(pkt[12:16], slot)
	binary.LittleEndian.PutUint32(pkt[16:20], index)
	binary.LittleEndian.PutUint32(pkt[20:24], amount)
	return pkt
}

func TestDeleteItemPersistsBeforeConfirmation(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	item, err := materializeItem(model.Item{Index: 412, Eff: [6]byte{effectAmount, 10}})
	if err != nil {
		t.Fatal(err)
	}
	p.Char.Inv[4] = item

	w.onDeleteItem(p.Session, deleteItemPacket(4, 412))

	if st.saves != 1 || p.Char.Inv[4].Index != 0 {
		t.Fatalf("delete incompleto: saves=%d item=%+v", st.saves, p.Char.Inv[4])
	}
}

func TestDeleteItemRollsBackAndRejectsForgedIndex(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	item := model.Item{Index: 413, UID: "11111111111141118111111111111111"}
	p.Char.Inv[2] = item

	w.onDeleteItem(p.Session, deleteItemPacket(2, 999))
	if st.saves != 0 || p.Char.Inv[2] != item {
		t.Fatal("index forjado alterou o inventario")
	}

	st.err = errors.New("postgres indisponivel")
	w.onDeleteItem(p.Session, deleteItemPacket(2, 413))
	if st.saves != 1 || p.Char.Inv[2] != item {
		t.Fatalf("rollback do delete falhou: saves=%d item=%+v", st.saves, p.Char.Inv[2])
	}
}

func TestSplitItemCreatesIndependentUIDAndConservesAmount(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	source, err := materializeItem(model.Item{
		Index: 412,
		Eff:   [6]byte{effectAmount, 10, 7, 3},
	})
	if err != nil {
		t.Fatal(err)
	}
	p.Char.Inv[5] = source

	w.onSplitItem(p.Session, splitItemPacket(5, 412, 4))

	if st.saves != 1 || itemStackAmount(p.Char.Inv[5]) != 6 ||
		itemStackAmount(p.Char.Inv[0]) != 4 {
		t.Fatalf("split incorreto: saves=%d source=%+v dest=%+v",
			st.saves, p.Char.Inv[5], p.Char.Inv[0])
	}
	if p.Char.Inv[0].UID == "" || p.Char.Inv[0].UID == source.UID {
		t.Fatalf("pilha nova sem identidade independente: old=%q new=%q",
			source.UID, p.Char.Inv[0].UID)
	}
	if p.Char.Inv[0].Eff[2] != 7 || p.Char.Inv[0].Eff[3] != 3 {
		t.Fatalf("split perdeu efeitos adicionais: %+v", p.Char.Inv[0])
	}
}

func TestSplitItemRejectsNonStackAndRollsBackBothSlots(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Inv[3] = model.Item{Index: 700, UID: "11111111111141118111111111111111"}

	w.onSplitItem(p.Session, splitItemPacket(3, 700, 1))
	if st.saves != 0 || p.Char.Inv[0].Index != 0 || p.Char.Inv[3].Index != 700 {
		t.Fatal("item sem EF_AMOUNT foi separado")
	}

	stack := model.Item{
		Index: 700,
		UID:   "11111111111141118111111111111111",
		Eff:   [6]byte{effectAmount, 8},
	}
	p.Char.Inv[3] = stack
	st.err = errors.New("save falhou")
	w.onSplitItem(p.Session, splitItemPacket(3, 700, 3))
	if st.saves != 1 || p.Char.Inv[3] != stack || p.Char.Inv[0].Index != 0 {
		t.Fatalf("rollback do split falhou: saves=%d source=%+v dest=%+v",
			st.saves, p.Char.Inv[3], p.Char.Inv[0])
	}
}
