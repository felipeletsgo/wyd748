package game

import (
	"encoding/binary"
	"reflect"
	"testing"

	"wydgo/internal/wire"
)

func TestCriticalInventoryCertifiedFailureCancelsAndAllowsRetry(t *testing.T) {
	for _, kind := range []string{"delete", "split"} {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalInventoryFixture(t, 5, 0)
			w.criticalAccountContainment = true
			backend.failure = criticalCertifiedRejection(t)
			before := accountStateSnapshot(p.Account)
			request := criticalInventoryRequest(kind, p.ID, 5)
			callCriticalInventory(w, p, kind, request)
			awaitCriticalWrite(t, backend.entered)
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.shuttingDown || p.PersistencePoisoned || p.Session.IsClosed() || w.criticalPlayerReserved(p) ||
				len(queue.pending) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 {
				t.Fatal("certified failure changed inventory, retained ownership, or disconnected the player")
			}
			for p.Session.QueuedPacketsForTest() != 0 {
				packet, _ := p.Session.DequeuePacketForTest()
				if !wire.Decrypt(packet) {
					t.Fatal("invalid rollback packet")
				}
				if opcode := wire.ParseHeader(packet).Type; opcode != wire.OpSendItem && opcode != wire.OpMessagePanel {
					t.Fatalf("rollback published an unexpected opcode: %#x", opcode)
				}
			}
			backend.failure = nil
			callCriticalInventory(w, p, kind, request)
			awaitCriticalWrite(t, backend.entered)
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 0 || p.Session.IsClosed() {
				t.Fatal("the connected player could not retry after cancellation")
			}
		})
	}
}

func TestCriticalCompositionCertifiedFailureKeepsIngredientsGoldAndTrade(t *testing.T) {
	for _, withTrade := range []bool{false, true} {
		name := "without-trade"
		if withTrade {
			name = "with-trade"
		}
		t.Run(name, func(t *testing.T) {
			w, p, fallback, backend, queue, req, rng, release := criticalTinyFixture(t, true, true)
			w.criticalAccountContainment = true
			backend.failure = criticalCertifiedRejection(t)
			var peer *Player
			if withTrade {
				peer = criticalTinyTradePeer(t, w, p)
			}
			before, trade := accountStateSnapshot(p.Account), p.Trade
			w.onCombineTiny(p.Session, criticalTinyRequest(p.ID, req))
			awaitCriticalWrite(t, backend.entered)
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.shuttingDown || p.PersistencePoisoned || p.Session.IsClosed() || w.criticalPlayerReserved(p) ||
				len(queue.pending) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade ||
				fallback.saves != 0 || rng.calls != 1 || peer != nil && (peer.Trade == nil || peer.Session.IsClosed()) {
				t.Fatal("certified composition failure consumed ingredients, charged gold, changed trade, or closed a session")
			}
			canceled := false
			for p.Session.QueuedPacketsForTest() != 0 {
				packet, _ := p.Session.DequeuePacketForTest()
				if !wire.Decrypt(packet) {
					t.Fatal("invalid composition rollback packet")
				}
				switch opcode := wire.ParseHeader(packet).Type; opcode {
				case wire.OpCombineComplete:
					if binary.LittleEndian.Uint32(packet[12:16]) != 0 {
						t.Fatal("canceled persistence published a successful or consumed failed roll")
					}
					canceled = true
				case wire.OpMessagePanel, wire.OpUpdateCarry, wire.OpUpdateEtc:
				default:
					t.Fatalf("rollback published an unexpected opcode: %#x", opcode)
				}
			}
			if !canceled {
				t.Fatal("composition UI did not receive the cancellation result")
			}
		})
	}
}
