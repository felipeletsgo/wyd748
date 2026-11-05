package game

import (
	"encoding/binary"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

type saleExpectation struct {
	price uint32
	item  model.Item
}

func validQuotedSaleSource(pkt []byte) bool {
	return len(pkt) == 32 && binary.LittleEndian.Uint16(pkt[12:14]) == 1 &&
		binary.LittleEndian.Uint16(pkt[16:18]) == placeInv &&
		binary.LittleEndian.Uint16(pkt[18:20]) < model.PlayerCarrySlots
}

func (w *World) onQuotedSellItem(s *net.Session, pkt []byte) {
	if !validQuotedSaleSource(pkt) {
		return
	}
	quote := saleExpectation{price: binary.LittleEndian.Uint32(pkt[20:24])}
	quote.item.Index = binary.LittleEndian.Uint16(pkt[24:26])
	copy(quote.item.Eff[:], pkt[26:32])
	legacy := make([]byte, 20)
	copy(legacy[:12], pkt[:12])
	binary.LittleEndian.PutUint16(legacy[:2], uint16(len(legacy)))
	binary.LittleEndian.PutUint16(legacy[4:6], wire.OpSellItem)
	copy(legacy[12:18], pkt[14:20])
	w.onSellItem(s, legacy, quote)
}
