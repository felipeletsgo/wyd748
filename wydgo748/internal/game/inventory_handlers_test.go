package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
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
		t.Fatalf("delete incomplete: saves=%d item=%+v", st.saves, p.Char.Inv[4])
	}
}

func TestDeleteItemRollsBackAndRejectsForgedIndex(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	item := model.Item{Index: 413, UID: "11111111111141118111111111111111"}
	p.Char.Inv[2] = item

	w.onDeleteItem(p.Session, deleteItemPacket(2, 999))
	if st.saves != 0 || p.Char.Inv[2] != item {
		t.Fatal("forged index changed inventory")
	}

	st.err = errors.New("postgres unavailable")
	w.onDeleteItem(p.Session, deleteItemPacket(2, 413))
	if st.saves != 1 || p.Char.Inv[2] != item {
		t.Fatalf("delete rollback failed: saves=%d item=%+v", st.saves, p.Char.Inv[2])
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
		t.Fatalf("incorrect split: saves=%d source=%+v dest=%+v",
			st.saves, p.Char.Inv[5], p.Char.Inv[0])
	}
	if p.Char.Inv[0].UID == "" || p.Char.Inv[0].UID == source.UID {
		t.Fatalf("new stack has no independent identity: old=%q new=%q",
			source.UID, p.Char.Inv[0].UID)
	}
	if p.Char.Inv[0].Eff[2] != 7 || p.Char.Inv[0].Eff[3] != 3 {
		t.Fatalf("split lost additional effects: %+v", p.Char.Inv[0])
	}
}

func TestSplitItemRejectsNonStackAndRollsBackBothSlots(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Inv[3] = model.Item{Index: 700, UID: "11111111111141118111111111111111"}

	w.onSplitItem(p.Session, splitItemPacket(3, 700, 1))
	if st.saves != 0 || p.Char.Inv[0].Index != 0 || p.Char.Inv[3].Index != 700 {
		t.Fatal("item without EF_AMOUNT was split")
	}

	stack := model.Item{
		Index: 700,
		UID:   "11111111111141118111111111111111",
		Eff:   [6]byte{effectAmount, 8},
	}
	p.Char.Inv[3] = stack
	st.err = errors.New("save failed")
	w.onSplitItem(p.Session, splitItemPacket(3, 700, 3))
	if st.saves != 1 || p.Char.Inv[3] != stack || p.Char.Inv[0].Index != 0 {
		t.Fatalf("split rollback failed: saves=%d source=%+v dest=%+v",
			st.saves, p.Char.Inv[3], p.Char.Inv[0])
	}
}

func TestSplitItemQuantityBoundaries(t *testing.T) {
	for _, total := range []byte{0, 1, 2, 10, 255} {
		for _, amount := range []uint32{0, 1, 2, 9, 10, 254, 255, 256, ^uint32(0)} {
			t.Run(fmt.Sprintf("total_%d/amount_%d", total, amount), func(t *testing.T) {
				w, p, st := handlerTestWorld(t)
				p.Char.Inv[5] = model.Item{Index: 412,
					UID: "11111111111141118111111111111111", Eff: [6]byte{effectAmount, total}}
				before := p.Char.Inv
				w.onSplitItem(p.Session, splitItemPacket(5, 412, amount))
				valid := total > 1 && amount > 0 && amount < uint32(total)
				if !valid {
					if st.saves != 0 || p.Char.Inv != before {
						t.Fatal("rejected quantity changed or persisted inventory")
					}
					return
				}
				if st.saves != 1 || itemStackAmount(p.Char.Inv[5]) != uint32(total)-amount ||
					itemStackAmount(p.Char.Inv[0]) != amount || p.Char.Inv[5].UID != before[5].UID ||
					p.Char.Inv[0].UID == "" || p.Char.Inv[0].UID == before[5].UID {
					t.Fatal("accepted quantity did not conserve stacks and independent identities")
				}
			})
		}
	}
}

func TestSplitItemRejectsMalformedRequests(t *testing.T) {
	valid := splitItemPacket(5, 412, 3)
	for name, packet := range map[string][]byte{
		"short": valid[:23], "trailing": append(append([]byte{}, valid...), 0),
		"slot":         splitItemPacket(model.PlayerCarrySlots, 412, 3),
		"wrapped_slot": splitItemPacket(^uint32(0), 412, 3),
		"empty_index":  splitItemPacket(5, 0, 3),
		"wide_index":   splitItemPacket(5, 65536+412, 3),
		"stale_index":  splitItemPacket(5, 413, 3),
	} {
		t.Run(name, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			p.Char.Inv[5] = model.Item{Index: 412, Eff: [6]byte{effectAmount, 5}}
			before := p.Char.Inv
			w.onSplitItem(p.Session, packet)
			if st.saves != 0 || p.Char.Inv != before {
				t.Fatal("malformed request changed or persisted inventory")
			}
		})
	}
}

func TestSplitItemFullInventoryAndRepeatedRequest(t *testing.T) {
	t.Run("full inventory", func(t *testing.T) {
		w, p, st := handlerTestWorld(t)
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			p.Char.Inv[slot] = model.Item{Index: 412, Eff: [6]byte{effectAmount, 5}}
		}
		before := p.Char.Inv
		w.onSplitItem(p.Session, splitItemPacket(5, 412, 3))
		if st.saves != 0 || p.Char.Inv != before {
			t.Fatal("full inventory split changed or persisted inventory")
		}
	})
	t.Run("repeat exceeds remaining quantity", func(t *testing.T) {
		w, p, st := handlerTestWorld(t)
		p.Char.Inv[5] = model.Item{Index: 412,
			UID: "11111111111141118111111111111111", Eff: [6]byte{effectAmount, 5}}
		packet := splitItemPacket(5, 412, 3)
		w.onSplitItem(p.Session, packet)
		if st.saves != 1 {
			t.Fatal("initial split was not saved")
		}
		before := p.Char.Inv
		w.onSplitItem(p.Session, packet)
		if st.saves != 1 || p.Char.Inv != before {
			t.Fatal("repeated request ignored the authoritative remaining quantity")
		}
	})
}

// Observe the real persistence boundary without changing the shared store
// double or bypassing Session.Send's framing and encryption.
type splitConfirmationStore struct {
	*craftStore
	beforeSave func(*model.Account)
}

func (s *splitConfirmationStore) SaveAccount(account *model.Account) error {
	s.beforeSave(account)
	return s.craftStore.SaveAccount(account)
}

func TestSplitItemOutboundSlotSnapshots(t *testing.T) {
	for _, sourceSlot := range []int{0, 5, model.PlayerCarrySlots - 1} {
		for effectSlot := 0; effectSlot < 3; effectSlot++ {
			for _, failSave := range []bool{false, true} {
				t.Run(fmt.Sprintf("source_%d/effect_%d/save_failure_%t", sourceSlot, effectSlot, failSave), func(t *testing.T) {
					w, p, st := handlerTestWorld(t)
					source := model.Item{Index: 412, UID: "11111111111141118111111111111111",
						Eff: [6]byte{7, 3, 8, 4, 9, 5}, ActivatedUnix: 123, ExpiresUnix: 456}
					source.Eff[effectSlot*2], source.Eff[effectSlot*2+1] = effectAmount, 10
					p.Char.Inv[sourceSlot] = source
					destSlot := 0
					if sourceSlot == 0 {
						destSlot = 1
					}
					before := p.Char.Inv
					if failSave {
						st.err = errors.New("save rejected")
					}
					w.store = &splitConfirmationStore{craftStore: st, beforeSave: func(account *model.Account) {
						if queued := p.Session.QueuedPacketsForTest(); queued != 0 {
							t.Fatalf("queued %d packets before persistence completed", queued)
						}
						persisted := account.Chars[p.CharSlot].Inv
						if persisted != p.Char.Inv || itemStackAmount(persisted[sourceSlot]) != 6 ||
							itemStackAmount(persisted[destSlot]) != 4 || persisted[sourceSlot].UID != source.UID ||
							persisted[destSlot].UID == "" || persisted[destSlot].UID == source.UID {
							t.Fatal("save did not receive both resulting stacks and independent identities")
						}
					}}

					w.onSplitItem(p.Session, splitItemPacket(uint32(sourceSlot), 412, 4))

					if st.saves != 1 || p.Session.QueuedPacketsForTest() != 2 {
						t.Fatalf("want one save and two confirmations: saves=%d queued=%d",
							st.saves, p.Session.QueuedPacketsForTest())
					}
					want := before
					if !failSave {
						want[sourceSlot].Eff[effectSlot*2+1] = 6
						want[destSlot] = source
						want[destSlot].UID = p.Char.Inv[destSlot].UID
						want[destSlot].Eff[effectSlot*2+1] = 4
					}
					if p.Char.Inv != want {
						t.Fatal("final inventory differs from the committed or rolled-back snapshot")
					}
					for sequence, slot := range []int{sourceSlot, destSlot} {
						packet, ok := p.Session.DequeuePacketForTest()
						if !ok || len(packet) != 24 || !wire.Decrypt(packet) {
							t.Fatalf("confirmation %d is not a valid encrypted 24-byte frame", sequence)
						}
						header := wire.ParseHeader(packet)
						if header.Size != 24 || header.Type != wire.OpSendItem || header.ID != p.ID ||
							binary.LittleEndian.Uint16(packet[12:14]) != uint16(placeInv) ||
							binary.LittleEndian.Uint16(packet[14:16]) != uint16(slot) {
							t.Fatalf("confirmation %d has the wrong recipient, destination, or envelope: % x", sequence, packet)
						}
						item := want[slot]
						if binary.LittleEndian.Uint16(packet[16:18]) != item.Index ||
							[6]byte(packet[18:24]) != item.Eff {
							t.Fatalf("confirmation %d differs from authoritative slot %d: % x", sequence, slot, packet[16:24])
						}
					}
				})
			}
		}
	}
}
