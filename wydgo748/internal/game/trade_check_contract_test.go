package game

import (
	"bytes"
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Exercise the first check through a bilateral invitation and validated offers,
// not by injecting an already-checked TradeState into the handler.
func tradeCheckContractWorld(t *testing.T) (*World, *Player, *Player, *batchGameStore) {
	t.Helper()
	a, _ := networkedTestPlayer(1, "CheckOwner", 2100, 2100)
	b, _ := networkedTestPlayer(2, "CheckPeer", 2101, 2100)
	w, store := economyWorld(a, b)
	a.Char.Inv[0] = model.Item{Index: 100, Eff: [6]byte{43, 9}}
	b.Char.Inv[0] = model.Item{Index: 200, Eff: [6]byte{43, 6}}
	a.Char.Gold, b.Char.Gold = 1000, 2000
	w.items[100], w.items[200] = model.ItemDef{Index: 100}, model.ItemDef{Index: 200}
	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 0, false))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 0, false))
	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 100, false, 0))
	w.onTrade(b.Session, tradeOfferPacket(a.ID, b.Char, 200, false, 0))
	if a.Trade == nil || b.Trade == nil || a.Trade.Checked || b.Trade.Checked {
		t.Fatal("fixture did not establish two unchecked offers")
	}
	for _, p := range []*Player{a, b} {
		for p.Session.QueuedPacketsForTest() > 0 {
			tradeCheckResponse(t, p)
		}
	}
	return w, a, b, store
}

func tradeCheckResponse(t *testing.T, p *Player) []byte {
	t.Helper()
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) {
		t.Fatal("missing or invalid encrypted trade response")
	}
	if len(packet) < 12 || int(wire.ParseHeader(packet).Size) != len(packet) {
		t.Fatal("trade response has an inconsistent envelope")
	}
	return packet
}

func assertFirstTradeCheck(t *testing.T, a, b *Player, store *batchGameStore) {
	t.Helper()
	ack := tradeCheckResponse(t, a)
	header := wire.ParseHeader(ack)
	if len(ack) != 12 || header.Type != wire.OpCNFTradeCheck || header.ID != a.ID {
		t.Fatalf("first check must acknowledge only the owner with 0x386/12: % x", ack)
	}
	peer := tradeCheckResponse(t, b)
	header = wire.ParseHeader(peer)
	expected := wire.Trade(b.ID, a.Trade.Items, a.Trade.CarryPos, a.Trade.Gold, true, a.ID)
	if len(peer) != 156 || header.Type != wire.OpTrade || header.ID != b.ID || !bytes.Equal(peer[12:], expected[12:]) {
		t.Fatalf("peer did not receive the authoritative checked offer: % x", peer)
	}
	if a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("first check emitted unexpected extra packets")
	}
	if !a.Trade.Checked || b.Trade.Checked || store.saves != 0 || store.batchSaves != 0 {
		t.Fatal("first check changed the peer confirmation or persisted the trade")
	}
}

func TestTradeCheckContractPublicationAndRepetition(t *testing.T) {
	w, a, b, store := tradeCheckContractWorld(t)
	beforeA, beforeB := a.Char.Inv, b.Char.Inv
	request := tradeOfferPacket(b.ID, a.Char, 100, true, 0)
	for attempt := 0; attempt < 2; attempt++ {
		w.onTrade(a.Session, request)
		assertFirstTradeCheck(t, a, b, store)
		if a.Char.Inv != beforeA || b.Char.Inv != beforeB || a.Char.Gold != 1000 || b.Char.Gold != 2000 {
			t.Fatal("first or repeated check transferred items or gold")
		}
	}
	// Revoking the first check updates only the peer offer; it cannot emit a
	// stale local success acknowledgement or persist an economic transaction.
	w.onTrade(a.Session, tradeOfferPacket(b.ID, a.Char, 100, false, 0))
	peer := tradeCheckResponse(t, b)
	if wire.ParseHeader(peer).Type != wire.OpTrade || peer[152] != 0 || a.Trade.Checked || b.Trade.Checked ||
		a.Session.QueuedPacketsForTest() != 0 || b.Session.QueuedPacketsForTest() != 0 || store.saves != 0 || store.batchSaves != 0 {
		t.Fatal("revoking a check produced a success acknowledgement or persisted state")
	}
}

func TestTradeCheckContractRejections(t *testing.T) {
	for _, tc := range []struct {
		name   string
		mutate func(*World, *Player, *Player, []byte) []byte
	}{
		{"truncated", func(_ *World, _, _ *Player, pkt []byte) []byte { return pkt[:155] }},
		{"oversized", func(_ *World, _, _ *Player, pkt []byte) []byte { return append(pkt, 0) }},
		{"invalid-check", func(_ *World, _, _ *Player, pkt []byte) []byte { pkt[152] = 2; return pkt }},
		{"forged-item", func(_ *World, _, _ *Player, pkt []byte) []byte { pkt[12]++; return pkt }},
		{"forged-effect", func(_ *World, _, _ *Player, pkt []byte) []byte { pkt[14]++; return pkt }},
		{"missing-position", func(_ *World, _, _ *Player, pkt []byte) []byte { pkt[132] = 0xff; return pkt }},
		{"changed-gold", func(_ *World, _, _ *Player, pkt []byte) []byte {
			binary.LittleEndian.PutUint32(pkt[148:152], 101)
			return pkt
		}},
		{"negative-gold", func(_ *World, _, _ *Player, pkt []byte) []byte {
			binary.LittleEndian.PutUint32(pkt[148:152], 0xffffffff)
			return pkt
		}},
		{"insufficient-gold", func(_ *World, _, _ *Player, pkt []byte) []byte {
			binary.LittleEndian.PutUint32(pkt[148:152], 1001)
			return pkt
		}},
		{"unknown-item", func(w *World, _, _ *Player, pkt []byte) []byte { delete(w.items, 100); return pkt }},
		{"sender-dead", func(_ *World, a, _ *Player, pkt []byte) []byte { effectiveScore(a.Char).CurHP = 0; return pkt }},
		{"peer-dead", func(_ *World, _, b *Player, pkt []byte) []byte { effectiveScore(b.Char).CurHP = 0; return pkt }},
		{"peer-out-of-world", func(_ *World, _, b *Player, pkt []byte) []byte { b.InWorld = false; return pkt }},
		{"peer-out-of-range", func(_ *World, _, b *Player, pkt []byte) []byte { b.X = 3000; return pkt }},
		{"nonmutual-session", func(_ *World, _, b *Player, pkt []byte) []byte { b.Trade.OpponentID = 3; return pkt }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, a, b, store := tradeCheckContractWorld(t)
			beforeA, beforeB := a.Char.Inv, b.Char.Inv
			request := tc.mutate(w, a, b, tradeOfferPacket(b.ID, a.Char, 100, true, 0))
			w.onTrade(a.Session, request)
			if a.Char.Inv != beforeA || b.Char.Inv != beforeB || a.Char.Gold != 1000 || b.Char.Gold != 2000 ||
				store.saves != 0 || store.batchSaves != 0 || (a.Trade != nil && a.Trade.Checked) || (b.Trade != nil && b.Trade.Checked) {
				t.Fatal("rejected check mutated economic or confirmation state")
			}
			for _, p := range []*Player{a, b} {
				for p.Session.QueuedPacketsForTest() > 0 {
					packet := tradeCheckResponse(t, p)
					header := wire.ParseHeader(packet)
					if header.Type == wire.OpCNFTradeCheck || (header.Type == wire.OpTrade && packet[152] != 0) {
						t.Fatal("rejected check published a success acknowledgement")
					}
				}
			}
		})
	}
}
