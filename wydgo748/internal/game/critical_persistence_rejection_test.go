package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// Obtain a real adapter-certified pre-write rejection. Test backends may return
// this outcome, but cannot certify an arbitrary error through a public marker.
func criticalCertifiedRejection(t *testing.T) error {
	t.Helper()
	err := (&store.PostgresStore{}).SavePlayerStateBatch(nil, "", nil)
	if !store.IsWriteRejected(err) {
		t.Fatalf("adapter did not certify invalid-write rejection: %v", err)
	}
	return err
}

func TestCriticalOwnerCertifiedRejectionSettlement(t *testing.T) {
	for _, mode := range []string{"rejected", "unknown", "indeterminate-rejection", "reject-error", "reject-panic", "no-handler", "success"} {
		t.Run(mode, func(t *testing.T) {
			failure := criticalCertifiedRejection(t)
			if mode == "success" {
				failure = nil
			} else if mode == "unknown" {
				failure = errors.New("commit acknowledgement lost")
			} else if mode == "indeterminate-rejection" {
				failure = errors.Join(failure, errCriticalPersistenceIndeterminate)
			}
			backend := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: failure}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			owner := newCriticalPersistenceOwner(queue)
			world := &World{criticalWrites: owner}
			applied, rejected, published := 0, 0, 0
			var reject func(criticalPersistenceResult) error
			if mode != "no-handler" {
				reject = func(result criticalPersistenceResult) error {
					rejected++
					if !store.IsWriteRejected(result.err) || queue.accounts["one"] == 0 || len(queue.pending) != 1 {
						t.Fatal("rejection callback ran without certification or reservation")
					}
					if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "one"}}}, func(criticalPersistenceResult) error { return nil }); !errors.Is(err, errCriticalPersistenceBusy) {
						t.Fatalf("rejection released the fence before settlement: %v", err)
					}
					if mode == "reject-error" {
						return errCriticalPersistenceIdentity
					}
					if mode == "reject-panic" {
						panic("failure settlement panic")
					}
					return nil
				}
			}
			id, err := owner.submitWithRejection(criticalWrite{accounts: []*model.Account{{Name: "one"}}},
				func(result criticalPersistenceResult) error { applied++; return result.err },
				func(criticalPersistenceResult) error { published++; return nil }, reject)
			if err != nil {
				t.Fatal(err)
			}
			result := awaitCriticalResult(t, queue)
			if applied != 0 || rejected != 0 || published != 0 || queue.accounts["one"] != id {
				t.Fatal("worker ran a continuation or released its reservation")
			}
			world.applyCriticalCompletion(result)
			settled := mode == "rejected" || mode == "success"
			wantRejected, wantApplied, wantPublished := 0, 1, 0
			if mode == "rejected" || mode == "reject-error" || mode == "reject-panic" {
				wantRejected, wantApplied = 1, 0
			}
			if mode == "success" {
				wantPublished = 1
			}
			if rejected != wantRejected || applied != wantApplied || published != wantPublished || world.shuttingDown == settled {
				t.Fatalf("mode=%s applied=%d rejected=%d published=%d maintenance=%t", mode, applied, rejected, published, world.shuttingDown)
			}
			if settled {
				if len(queue.pending) != 0 || len(owner.continuations) != 0 || queue.accounts["one"] != 0 {
					t.Fatal("settled operation retained reservations or closures")
				}
			} else {
				retained := owner.continuations[id]
				if len(queue.pending) != 1 || queue.accounts["one"] != id || retained == nil || retained.err == nil ||
					retained.apply != nil || retained.afterAck != nil || retained.reject != nil {
					t.Fatal("failed settlement released ownership or retained callbacks")
				}
			}
			if err := owner.complete(result); err == nil || applied != wantApplied || rejected != wantRejected || published != wantPublished {
				t.Fatal("duplicate completion replayed a continuation")
			}
		})
	}
}

func TestCriticalNativeConsumableCertifiedRejection(t *testing.T) {
	for _, tc := range criticalNativeCases() {
		for _, mode := range []string{"solo", "trade", "closed-owner", "closed-peer", "closed-both"} {
			t.Run(tc.name+"/"+mode, func(t *testing.T) {
				w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
				configureCriticalNative(w, p, tc)
				var peer *Player
				if mode != "solo" {
					peer = criticalTinyTradePeer(t, w, p)
				}
				before := accountStateSnapshot(p.Account)
				trade := p.Trade
				var peerBefore *model.Account
				var peerTrade *TradeState
				if peer != nil {
					peerBefore, peerTrade = accountStateSnapshot(peer.Account), peer.Trade
				}
				backend.failure = criticalCertifiedRejection(t)
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				awaitCriticalWrite(t, backend.entered)
				// Repetition while held must not queue or publish a second operation.
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				var closed []*Player
				if mode == "closed-owner" || mode == "closed-both" {
					closed = append(closed, p)
				}
				if mode == "closed-peer" || mode == "closed-both" {
					closed = append(closed, peer)
				}
				for _, participant := range closed {
					w.handle(command{s: participant.Session})
					w.handle(command{s: participant.Session})
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 {
					t.Fatal("held rejection mutated, repeated or published")
				}
				release()
				result := awaitCriticalResult(t, queue)
				// Receiving the result synchronizes with the finished worker. Only
				// subsequent explicit/final writes now receive successful outcomes.
				backend.failure = nil
				discardCriticalDeferredDuplicates(w)
				w.applyCriticalCompletion(result)
				if w.shuttingDown || p.PersistencePoisoned || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade ||
					(peer != nil && (!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) || peer.Trade != peerTrade)) {
					t.Fatal("certified rejection altered retained state or entered maintenance")
				}
				if !p.Session.IsClosed() {
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 62, p.Char.Inv[62]))
					assertCriticalPickupPacket(t, p, wire.MessagePanel("Item use could not be saved. Please try again later."))
				}
				if len(closed) == 0 {
					if len(queue.pending) != 0 || len(backend.entered) != 0 || w.criticalPlayerReserved(p) || (peer != nil && w.criticalPlayerReserved(peer)) {
						t.Fatal("rejection retained a fence or automatically replayed a write")
					}
					w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
					awaitCriticalWrite(t, backend.entered)
					w.applyCriticalCompletion(awaitCriticalResult(t, queue))
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), criticalNativeExpected(w, before, p.CharSlot, tc)) || len(queue.pending) != 0 {
						t.Fatal("new intention after certified rejection did not commit exactly once")
					}
					if peer != nil {
						assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
						assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
					}
					assertCriticalNativePublication(t, p, tc)
				} else {
					seen := make(map[string]bool)
					for range closed {
						final := awaitCriticalWrite(t, backend.entered)
						var participant *Player
						for _, candidate := range closed {
							if final.accounts[0].Name == candidate.Account.Name {
								participant = candidate
							}
						}
						if participant == nil || seen[participant.Account.Name] || !reflect.DeepEqual(final.accounts[0], accountPersistenceSnapshot(participant.Account)) ||
							final.character == nil || !reflect.DeepEqual(final.character.state, buildCharState(participant, w.now())) {
							t.Fatal("final save after rejection lost retained state or repeated a participant")
						}
						seen[participant.Account.Name] = true
						if w.claimAccountSession(gameNet.NewTestSession(98, 8), participant.Account.Name) {
							t.Fatal("final save released identity before acknowledgement")
						}
					}
					for range closed {
						w.applyCriticalCompletion(awaitCriticalResult(t, queue))
					}
					for _, participant := range closed {
						if w.players[participant.Session] != nil || participant.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("final rejection cleanup retained identity or replied to closed transport")
						}
					}
					for _, participant := range []*Player{p, peer} {
						if !participant.Session.IsClosed() {
							assertCriticalPickupPacket(t, participant, wire.CloseTrade(participant.ID))
						}
					}
				}
				if w.shuttingDown || len(queue.pending) != 0 || len(backend.entered) != 0 || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || (peer != nil && peer.Session.QueuedPacketsForTest() != 0) {
					t.Fatal("rejection continuation leaked work, publication or fallback persistence")
				}
			})
		}
	}
}

func TestCriticalNativeConsumableRejectionRequiresRetainedIdentityAndState(t *testing.T) {
	for _, mode := range []string{"account", "peer-account", "trade", "lifecycle", "maintenance"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
			configureCriticalNative(w, p, criticalNativeCases()[0])
			peer := criticalTinyTradePeer(t, w, p)
			backend.failure = criticalCertifiedRejection(t)
			w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
			awaitCriticalWrite(t, backend.entered)
			switch mode {
			case "account":
				p.Account.CargoGold++
			case "peer-account":
				peer.Account.CargoGold++
			case "trade":
				peer.Trade.Gold++
			case "lifecycle":
				peer.criticalLifecycle = &criticalPlayerLifecycle{}
			case "maintenance":
				w.shutdownErr = errors.New("other operation has an unknown outcome")
			}
			before, peerBefore := accountStateSnapshot(p.Account), accountStateSnapshot(peer.Account)
			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, queue))
			if !w.shuttingDown || !p.PersistencePoisoned || !peer.PersistencePoisoned || len(queue.pending) != 1 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
				p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 || fallback.saves != 0 {
				t.Fatal("stale rejection released ownership or reconciled unverified state")
			}
		})
	}
}
