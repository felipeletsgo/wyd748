package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func saleIngressWorld(t *testing.T) (*World, *Player, *craftStore) {
	t.Helper()
	w, p, st := handlerTestWorld(t)
	w.items[400] = model.ItemDef{Index: 400, Price: 1000}
	shop := &Mob{ID: 1100, X: 2101, Y: 2100, Def: &model.NPCDef{
		Name: "Merchant", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: nativeShopMerchant},
	}}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	p.Char.Gold = 5000
	return w, p, st
}

func saleIngressPacket(merchant, sourceType, position uint16) []byte {
	pkt := inboundPacket(wire.OpSellItem, 20)
	binary.LittleEndian.PutUint16(pkt[12:14], merchant)
	binary.LittleEndian.PutUint16(pkt[14:16], sourceType)
	binary.LittleEndian.PutUint16(pkt[16:18], position)
	return pkt
}

func TestSaleIngressRejectsFullWidthSourceBeforeSideEffects(t *testing.T) {
	tests := []struct {
		name     string
		typ, pos uint16
	}{
		{"equipment", 0, 0}, {"cargo", 2, 0},
		{"type_high_byte_alias", 0x0101, 0},
		{"negative_type_alias", 0xFF01, 0},
		{"reserved_slot", placeInv, model.PlayerCarrySlots},
		{"position_high_byte_alias", placeInv, 0x0100},
		{"negative_position_alias", placeInv, 0xFF00},
		{"negative_position", placeInv, 0xFFFF},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, p, st := saleIngressWorld(t)
			for i := range p.Char.Inv {
				p.Char.Inv[i] = model.Item{Index: 400}
			}
			p.Trade = &TradeState{OpponentID: 2, CarryPos: emptyTradePositions()}
			beforeChar, beforeTrade := *p.Char, *p.Trade
			pkt := saleIngressPacket(p.ShopNPC, tt.typ, tt.pos)
			beforePacket := append([]byte(nil), pkt...)
			w.handle(command{s: p.Session, pkt: pkt})
			if !reflect.DeepEqual(*p.Char, beforeChar) || st.saves != 0 || p.ShopNPC != 1100 {
				t.Fatal("invalid full-width sale mutated character, persistence, or merchant context")
			}
			if p.Trade == nil || *p.Trade != beforeTrade || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("invalid full-width sale canceled trade or published a response")
			}
			if state := w.security[p.Session]; state == nil || state.violations != 1 {
				t.Fatal("invalid full-width sale was not counted at ingress")
			}
			for i := range pkt {
				if pkt[i] != beforePacket[i] {
					t.Fatal("ingress modified borrowed request bytes")
				}
			}
		})
	}
}

func TestSaleIngressValidSlotsPersistOnceAndRejectReplay(t *testing.T) {
	for _, slot := range []int{0, model.PlayerCarrySlots - 1} {
		t.Run(fmt.Sprintf("slot_%d", slot), func(t *testing.T) {
			w, p, st := saleIngressWorld(t)
			p.Char.Inv[slot] = model.Item{Index: 400}
			pkt := saleIngressPacket(p.ShopNPC, placeInv, uint16(slot))
			// Native tail padding has no source-index semantics.
			pkt[18], pkt[19] = 0xFF, 0xA5
			w.handle(command{s: p.Session, pkt: pkt})
			if p.Char.Inv[slot].Index != 0 || p.Char.Gold != 5250 || st.saves != 1 {
				t.Fatal("canonical sale did not persist the expected item/gold transition")
			}
			if p.Session.QueuedPacketsForTest() != 2 {
				t.Fatal("canonical sale must publish only the slot and gold snapshots")
			}
			w.handle(command{s: p.Session, pkt: pkt})
			if p.Char.Gold != 5250 || st.saves != 1 || p.Session.QueuedPacketsForTest() != 2 {
				t.Fatal("repeated sale credited or published an empty slot")
			}
		})
	}
}

func TestSaleIngressPersistenceFailureRestoresState(t *testing.T) {
	w, p, st := saleIngressWorld(t)
	p.Char.Inv[0] = model.Item{Index: 400}
	st.err = errors.New("sale persistence unavailable")
	before := *p.Char
	w.handle(command{s: p.Session, pkt: saleIngressPacket(p.ShopNPC, placeInv, 0)})
	if !reflect.DeepEqual(*p.Char, before) || st.saves != 1 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("failed sale persistence changed state or published success")
	}
}
