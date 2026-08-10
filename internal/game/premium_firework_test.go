package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func premiumFireworkPacket(slot uint32, x, y uint16, pattern [16]byte) []byte {
	pkt := inboundPacket(wire.OpUsePremiumFirework, premiumFireworkUsePacketSize)
	binary.LittleEndian.PutUint32(pkt[12:16], placeInv)
	binary.LittleEndian.PutUint32(pkt[16:20], slot)
	binary.LittleEndian.PutUint16(pkt[28:30], x)
	binary.LittleEndian.PutUint16(pkt[30:32], y)
	copy(pkt[34:50], pattern[:])
	return pkt
}

func premiumFireworkWorld() (*World, *Player, *Player, *Player, *craftStore, *fakeClock) {
	owner, _ := networkedTestPlayer(1, "Artist", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	outsider, _ := networkedTestPlayer(3, "Outsider", 2200, 2200)
	observer.show(owner.ID)
	store := &craftStore{}
	clock := newFakeClock(time.Date(2026, 8, 9, 12, 0, 0, 0, time.UTC))
	w := worldWithNetworkedPlayers(owner, observer, outsider)
	w.store = store
	w.clock = clock
	w.items = map[uint16]model.ItemDef{
		1728: {Index: 1728},
		3442: {Index: 3442},
	}
	w.volatiles = model.VolatileCatalog{
		Default: model.VolatileRule{Action: "generic"},
		Rules: map[int]model.VolatileRule{
			19: {Action: "firework", Consume: true},
		},
		Items: map[uint16]model.VolatileRule{
			3442: {Action: "firework", Consume: true, CustomPattern: true},
		},
		ItemCodes: map[uint16]int{1728: 19, 3442: 19},
	}
	owner.Char.Inv[0] = model.Item{
		Index: 3442,
		Eff:   [6]byte{effectAmount, 2},
		UID:   "00000000000000000000000000003442",
	}
	return w, owner, observer, outsider, store, clock
}

func TestPremiumFireworkConsumesPersistsAndPublishesCustomPattern(t *testing.T) {
	w, owner, observer, outsider, store, _ := premiumFireworkWorld()
	pattern := [16]byte{0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81, 0x55, 0xAA, 0x0F, 0xF0, 0x0F}

	w.onUsePremiumFirework(owner.Session,
		premiumFireworkPacket(0, owner.X, owner.Y, pattern))

	if item := owner.Char.Inv[0]; item.Index != 3442 || item.Eff[1] != 1 ||
		item.UID != "00000000000000000000000000003442" {
		t.Fatalf("pilha premium incorreta apos uso: %+v", item)
	}
	if store.saves != 1 || owner.LastPremiumFirework.IsZero() {
		t.Fatalf("uso nao foi confirmado: saves=%d last=%v", store.saves, owner.LastPremiumFirework)
	}
	// Dono recebe SendItem + 0x3CA; somente o observador visivel recebe 0x3CA.
	if got := owner.Session.QueuedPacketsForTest(); got != 2 {
		t.Fatalf("dono recebeu %d pacotes, quer 2", got)
	}
	if got := observer.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("observador recebeu %d pacotes, quer 1", got)
	}
	if got := outsider.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("jogador fora da visao recebeu %d pacotes", got)
	}
}

func TestPremiumFireworkRollsBackBeforePublishingOnSaveFailure(t *testing.T) {
	w, owner, observer, _, store, _ := premiumFireworkWorld()
	store.err = errors.New("database unavailable")
	before := owner.Char.Inv[0]

	w.onUsePremiumFirework(owner.Session,
		premiumFireworkPacket(0, owner.X, owner.Y, [16]byte{1}))

	if owner.Char.Inv[0] != before || !owner.LastPremiumFirework.IsZero() {
		t.Fatalf("falha deixou mutacao parcial: item=%+v last=%v",
			owner.Char.Inv[0], owner.LastPremiumFirework)
	}
	if store.saves != 1 || owner.Session.QueuedPacketsForTest() != 1 {
		t.Fatalf("rollback nao ressincronizou: saves=%d packets=%d",
			store.saves, owner.Session.QueuedPacketsForTest())
	}
	if observer.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("desenho foi publicado antes da persistencia")
	}
}

func TestPremiumFireworkRejectsForgedItemBitmapAndPosition(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*World, *Player, []byte)
	}{
		{
			name: "ordinary firecracker cannot use premium packet",
			mutate: func(_ *World, p *Player, _ []byte) {
				p.Char.Inv[0] = model.Item{Index: 1728, Eff: [6]byte{effectAmount, 2}}
			},
		},
		{
			name: "unused bitmap bits must be zero",
			mutate: func(_ *World, _ *Player, pkt []byte) {
				pkt[46] = 0x10 // bitmap byte 12: bit 100 is outside the 10x10 grid.
			},
		},
		{
			name: "client coordinates cannot spoof authoritative position",
			mutate: func(_ *World, _ *Player, pkt []byte) {
				binary.LittleEndian.PutUint16(pkt[28:30], 3000)
			},
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, owner, observer, _, store, _ := premiumFireworkWorld()
			pkt := premiumFireworkPacket(0, owner.X, owner.Y, [16]byte{1})
			tc.mutate(w, owner, pkt)
			before := owner.Char.Inv[0]

			w.onUsePremiumFirework(owner.Session, pkt)

			if owner.Char.Inv[0] != before || store.saves != 0 ||
				observer.Session.QueuedPacketsForTest() != 0 {
				t.Fatalf("pedido forjado alterou estado: item=%+v saves=%d observer=%d",
					owner.Char.Inv[0], store.saves, observer.Session.QueuedPacketsForTest())
			}
			if owner.Session.QueuedPacketsForTest() != 1 {
				t.Fatalf("slot otimista nao foi ressincronizado: packets=%d",
					owner.Session.QueuedPacketsForTest())
			}
		})
	}
}

func TestPremiumFireworkCooldownPreventsReplaySaveFlood(t *testing.T) {
	w, owner, observer, _, store, clock := premiumFireworkWorld()
	pkt := premiumFireworkPacket(0, owner.X, owner.Y, [16]byte{1})
	w.onUsePremiumFirework(owner.Session, pkt)
	w.onUsePremiumFirework(owner.Session, pkt)
	if owner.Char.Inv[0].Eff[1] != 1 || store.saves != 1 ||
		observer.Session.QueuedPacketsForTest() != 1 {
		t.Fatalf("replay imediato passou: amount=%d saves=%d observer=%d",
			owner.Char.Inv[0].Eff[1], store.saves, observer.Session.QueuedPacketsForTest())
	}

	clock.Advance(premiumFireworkCooldown)
	w.onUsePremiumFirework(owner.Session, pkt)
	if owner.Char.Inv[0].Index != 0 || store.saves != 2 ||
		observer.Session.QueuedPacketsForTest() != 2 {
		t.Fatalf("uso apos cooldown falhou: item=%+v saves=%d observer=%d",
			owner.Char.Inv[0], store.saves, observer.Session.QueuedPacketsForTest())
	}
}

func TestPremiumFireworkCannotFallBackToOrdinaryUseItem(t *testing.T) {
	w, owner, observer, _, store, _ := premiumFireworkWorld()
	w.onUseItem(owner.Session, useItemPacket(0, 0))
	if owner.Char.Inv[0].Eff[1] != 2 || store.saves != 0 ||
		observer.Session.QueuedPacketsForTest() != 0 {
		t.Fatalf("0x373 consumiu/publicou premium: item=%+v saves=%d observer=%d",
			owner.Char.Inv[0], store.saves, observer.Session.QueuedPacketsForTest())
	}
	if owner.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("0x373 premium deveria apenas ressincronizar o slot")
	}
}
