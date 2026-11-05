package game

import (
	"encoding/binary"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

func quotedSalePacket(p *Player, slot uint16, price uint32, item model.Item) []byte {
	pkt := inboundPacket(wire.OpQuotedSellItem, 32)
	binary.LittleEndian.PutUint16(pkt[6:8], p.ID)
	binary.LittleEndian.PutUint16(pkt[12:14], 1)
	binary.LittleEndian.PutUint16(pkt[14:16], p.ShopNPC)
	binary.LittleEndian.PutUint16(pkt[16:18], placeInv)
	binary.LittleEndian.PutUint16(pkt[18:20], slot)
	binary.LittleEndian.PutUint32(pkt[20:24], price)
	binary.LittleEndian.PutUint16(pkt[24:26], item.Index)
	copy(pkt[26:32], item.Eff[:])
	return pkt
}

func TestQuotedSaleWaitsForDurableAcknowledgement(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejection"}[rejected], func(t *testing.T) {
			w, p, fallback, shop, backend, queue, release := criticalShopFixture(t, "sale")
			if rejected {
				backend.failure = store.RejectUnsentWrite("fixture did not call a durable adapter")
			}
			before := accountStateSnapshot(p.Account)
			packet := quotedSalePacket(p, 5, 250, p.Char.Inv[5])
			w.handle(command{s: p.Session, pkt: packet})
			write := awaitCriticalWrite(t, backend.entered)
			if write.accounts[0].Chars[p.CharSlot].Gold != 10250 || write.accounts[0].Chars[p.CharSlot].Inv[5] != (model.Item{}) {
				t.Fatal("quoted sale submitted a different price or item")
			}
			for _, completed := range []bool{false, true} {
				if completed {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.onQuotedSellItem(p.Session, packet)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 ||
					len(backend.entered) != 0 || fallback.saves != 0 || !w.criticalPlayerReserved(p) {
					t.Fatal("quoted sale published, mutated, or replayed before durable acknowledgement")
				}
			}
			w.processCriticalCompletions(1)
			if rejected {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || w.criticalPlayerReserved(p) {
					t.Fatal("certified rejection did not retain the item and balance")
				}
			} else {
				if p.Char.Gold != 10250 || p.Char.Inv[5] != (model.Item{}) {
					t.Fatal("committed quote lost its exact price")
				}
				assertCriticalShopPublication(t, p, "sale", shop.ID)
				w.onQuotedSellItem(p.Session, packet)
				if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Char.Gold != 10250 {
					t.Fatal("quoted sale replay credited twice")
				}
			}
		})
	}
}

func TestQuotedSaleCommitRejectionRollbackAndReplay(t *testing.T) {
	for _, name := range []string{"commit", "price_changed", "item_changed", "effects_changed", "save_failed"} {
		t.Run(name, func(t *testing.T) {
			w, p, st := saleIngressWorld(t)
			item := model.Item{Index: 400, Eff: [6]byte{61, 3}}
			p.Char.Inv[5] = item
			packet := quotedSalePacket(p, 5, 250, item)
			switch name {
			case "price_changed":
				binary.LittleEndian.PutUint32(packet[20:24], 251)
			case "item_changed":
				binary.LittleEndian.PutUint16(packet[24:26], 401)
			case "effects_changed":
				packet[27] = 2
			case "save_failed":
				st.err = errors.New("save failed")
			}
			w.handle(command{s: p.Session, pkt: packet})
			if name == "commit" {
				if p.Char.Inv[5].Index != 0 || p.Char.Gold != 5250 || st.saves != 1 {
					t.Fatal("quoted sale did not commit its exact displayed price")
				}
				w.handle(command{s: p.Session, pkt: packet})
				if p.Char.Gold != 5250 || st.saves != 1 {
					t.Fatal("replay credited the sale twice")
				}
			} else {
				if p.Char.Inv[5] != item || p.Char.Gold != 5000 {
					t.Fatal("rejected sale changed item or gold")
				}
				if name != "save_failed" && st.saves != 0 {
					t.Fatal("stale quote reached persistence")
				}
			}
		})
	}
}

func TestQuotedSaleIngressRejectsMalformedContracts(t *testing.T) {
	for _, name := range []string{"version", "type", "type_alias", "slot", "slot_alias", "short", "long"} {
		t.Run(name, func(t *testing.T) {
			w, p, st := saleIngressWorld(t)
			p.Char.Inv[0] = model.Item{Index: 400}
			packet := quotedSalePacket(p, 0, 250, p.Char.Inv[0])
			switch name {
			case "version":
				binary.LittleEndian.PutUint16(packet[12:14], 2)
			case "type":
				binary.LittleEndian.PutUint16(packet[16:18], 0)
			case "type_alias":
				binary.LittleEndian.PutUint16(packet[16:18], 0x101)
			case "slot":
				binary.LittleEndian.PutUint16(packet[18:20], 63)
			case "slot_alias":
				binary.LittleEndian.PutUint16(packet[18:20], 256)
			case "short":
				packet = packet[:31]
			case "long":
				packet = append(packet, 0)
			}
			binary.LittleEndian.PutUint16(packet[:2], uint16(len(packet)))
			w.handle(command{s: p.Session, pkt: packet})
			if p.Char.Inv[0].Index != 400 || p.Char.Gold != 5000 || st.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("invalid quote escaped the pre-dispatch gate")
			}
		})
	}
}
