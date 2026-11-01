package game

import (
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// W1/D1: detached producers settle a certified rejection without closing any
// session, entering maintenance or retaining the rejected reservation.
func assertRejectedKeptOnline(t *testing.T, w *World, queue *criticalPersistence, players ...*Player) {
	t.Helper()
	if w.shuttingDown || len(queue.pending) != 0 || queue.quarantined != 0 {
		t.Fatalf("maintenance=%t pending=%d quarantined=%d", w.shuttingDown, len(queue.pending), queue.quarantined)
	}
	for _, p := range players {
		if p.Session.IsClosed() || p.PersistencePoisoned || w.criticalPlayerReserved(p) {
			t.Fatalf("player %d closed=%t poisoned=%t reserved=%t", p.ID, p.Session.IsClosed(), p.PersistencePoisoned, w.criticalPlayerReserved(p))
		}
	}
}

func settleRejected(t *testing.T, w *World, backend *criticalTestStore, queue *criticalPersistence) {
	t.Helper()
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
}

func TestCriticalTradeCertifiedRejectionClosesOffersAndKeepsBothOnline(t *testing.T) {
	w, a, b, fallback, backend, queue, release := criticalTradeFixture(t, 1, 0, "both")
	release()
	backend.failure = criticalCertifiedRejection(t)
	beforeA, beforeB := accountStateSnapshot(a.Account), accountStateSnapshot(b.Account)
	w.commitTrade(a, b)
	settleRejected(t, w, backend, queue)
	assertRejectedKeptOnline(t, w, queue, a, b)
	if !reflect.DeepEqual(accountStateSnapshot(a.Account), beforeA) || !reflect.DeepEqual(accountStateSnapshot(b.Account), beforeB) ||
		a.Trade != nil || b.Trade != nil || fallback.saves != 0 || fallback.batchSaves != 0 {
		t.Fatal("rejected trade moved items or gold, kept the offers or saved inline")
	}
	for _, p := range []*Player{a, b} {
		assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
		assertCriticalPickupPacket(t, p, wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
		assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
		assertCriticalPickupPacket(t, p, wire.MessagePanel(w.trFor(p, "Save failed. The inventory was not changed.")))
	}
}

func TestCriticalDetachedProducersKeepSessionOnCertifiedRejection(t *testing.T) {
	for _, tc := range []struct {
		name    string
		prepare func(*World, *Player)
		submit  func(*World, *Player) error
		packets func(*World, *Player) [][]byte
	}{
		{"cargo-gold-deposit", func(_ *World, p *Player) { p.Char.Gold = 500 }, func(w *World, p *Player) error {
			return w.queueCargoGoldTransfer(p, true, 200)
		}, func(w *World, p *Player) [][]byte {
			return [][]byte{wire.UpdateCargoGold(wire.SceneField, p.Account.CargoGold), wire.UpdateEtc(p.ID, *p.Char),
				wire.MessagePanel(w.trFor(p, "Save failed. The inventory was not changed."))}
		}},
		{"merchant-sale", func(_ *World, p *Player) { p.Char.Inv[3] = model.Item{Index: 1000, UID: "sold"} }, func(w *World, p *Player) error {
			return w.queueShopSale(p, 3, 50, 0, 0)
		}, func(w *World, p *Player) [][]byte {
			return [][]byte{wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold), wire.UpdateEtc(p.ID, *p.Char),
				wire.MessagePanel(w.trFor(p, "Save failed. The inventory was not changed."))}
		}},
		{"hell-store-purchase", func(_ *World, p *Player) { p.Char.Gold = 900 }, func(w *World, p *Player) error {
			return w.queueHellStorePurchase(p, model.Item{Index: 1000, UID: "bought"}, 4, 100, 0)
		}, func(w *World, p *Player) [][]byte {
			return [][]byte{wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold), wire.UpdateEtc(p.ID, *p.Char),
				wire.MessagePanel(w.trFor(p, "Save failed. The purchase was not applied."))}
		}},
		{"chaos-recovery", func(_ *World, p *Player) { p.Char.CP = -5 }, func(w *World, p *Player) error {
			w.submitChaosRecovery(p, w.now())
			return nil
		}, func(*World, *Player) [][]byte { return nil }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalAutosaveFixture(t, 0)
			release()
			tc.prepare(w, p)
			backend.failure = criticalCertifiedRejection(t)
			before, nextCP := accountStateSnapshot(p.Account), p.NextCPRecovery
			if err := tc.submit(w, p); err != nil {
				t.Fatal(err)
			}
			settleRejected(t, w, backend, queue)
			assertRejectedKeptOnline(t, w, queue, p)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || fallback.batchSaves != 0 {
				t.Fatal("rejected proposal changed live state or saved inline")
			}
			for _, packet := range tc.packets(w, p) {
				assertCriticalPickupPacket(t, p, packet)
			}
			if p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("rejected proposal published a success response")
			}
			if tc.name == "chaos-recovery" && (!p.NextCPRecovery.Equal(nextCP) || !p.backgroundPersistenceDeferred(w.now())) {
				t.Fatal("rejected CP recovery consumed its deadline or retries on the next tick")
			}
		})
	}
}
