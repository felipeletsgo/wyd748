package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCriticalGroundCertifiedRejectionResync(t *testing.T) {
	for _, kind := range []string{"pickup", "fallback", "currency", "Carry-drop", "Cargo-drop"} {
		for _, lifecycle := range []string{"open", "closed", "instance-release"} {
			t.Run(kind+"/"+lifecycle, func(t *testing.T) {
				var w *World
				var p *Player
				var g *GroundItem
				var fallback *craftStore
				var backend *criticalTestStore
				var queue *criticalPersistence
				var release func()
				place, slot := byte(placeInv), 7
				drop := kind == "Carry-drop" || kind == "Cargo-drop"
				if drop {
					place = placeInv
					if kind == "Cargo-drop" {
						place = placeStorage
					}
					w, p, fallback, backend, queue, release = criticalDropFixture(t, place, slot)
				} else {
					w, p, g, fallback, backend, queue, release = criticalPickupFixture(t)
				}
				w.criticalAccountContainment = true
				backend.failure = criticalCertifiedRejection(t)
				if kind == "fallback" {
					p.Char.Inv[slot] = model.Item{Index: 201, UID: "occupied-rejection-slot"}
					slot = 0
				} else if kind == "currency" {
					configureCriticalGroundCurrency(w, g, 400)
				}
				before := accountStateSnapshot(p.Account)
				if lifecycle == "instance-release" {
					w.itemInstances = map[string]*ItemInstance{"private-rejection": {RuntimeID: "private-rejection", MemberIDs: []uint16{p.ID}}}
					if g != nil {
						g.InstanceID = "private-rejection"
					}
				}
				var request []byte
				if drop {
					request = criticalDropRequest(place, slot)
				} else {
					request = criticalPickupRequest(g.ID, 7)
				}
				w.handle(command{s: p.Session, pkt: request})
				awaitCriticalWrite(t, backend.entered)
				if drop {
					g = pendingCriticalDrop(t, w)
				}
				observer := criticalDropObserver(t, w, p, g)
				if !drop {
					observer.show(g.ID)
				}
				id := queue.accounts[accountSessionKey(p.Account.Name)]
				if !w.criticalWrites.continuations[id].sharedWorld {
					t.Fatal("ground operation did not retain shared-world ownership")
				}
				// Exercise the reservation guard without enqueuing another intent.
				// World.handle deliberately defers fenced client commands for later.
				if drop {
					w.onDropItem(p.Session, request)
				} else {
					w.onGetItem(p.Session, request)
				}
				if lifecycle == "closed" {
					p.Session.Close()
				} else if lifecycle == "instance-release" {
					g.InstanceID = ""
				}
				if p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 ||
					len(queue.pending) != 1 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("pending rejection changed economy, published or repeated")
				}
				release()
				result := awaitCriticalResult(t, queue)
				if !g.criticalReserved() || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("worker released ownership or sent a packet before actor settlement")
				}
				w.applyCriticalCompletion(result)
				if w.shuttingDown || p.PersistencePoisoned || g.criticalReserved() || len(queue.pending) != 0 ||
					len(w.criticalWrites.continuations) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 {
					t.Fatal("certified rejection did not settle the unchanged preimage exactly once")
				}
				if drop && w.groundItems[g.ID] != nil || !drop && w.groundItems[g.ID] != g {
					t.Fatal("certified rejection removed pickup loot or retained an unpublished drop")
				}
				if observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejection published a spawn, removal or success to an observer")
				}
				if lifecycle == "closed" {
					if p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("rejection sent snapshots to a closed transport")
					}
				} else {
					if kind == "currency" {
						assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
					} else {
						assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, place, byte(slot), *playerSlotOf(p, place, byte(slot))))
					}
					message := "Save failed. The item was not picked up."
					if drop {
						message = "Save failed. The item was not dropped."
					}
					assertCriticalPickupPacket(t, p, wire.MessagePanel(message))
					if p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("rejection sent a success confirmation or extra packet")
					}
				}
				if err := w.criticalWrites.complete(result); err == nil || g.criticalReserved() {
					t.Fatal("duplicate rejection was accepted or re-reserved an object")
				}
				if lifecycle == "open" {
					// Retry is a new intention after a definite rejection, not replay
					// of an uncertain durable transaction.
					backend.failure = nil
					w.handle(command{s: p.Session, pkt: request})
					awaitCriticalWrite(t, backend.entered)
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
					if w.shuttingDown || len(queue.pending) != 0 || reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
						t.Fatal("new intention after certified rejection could not commit")
					}
				}
			})
		}
	}
}

func TestCriticalGroundRejectionIdentityMismatchStaysGlobal(t *testing.T) {
	for _, operation := range []string{"pickup", "drop"} {
		for _, changed := range []string{"object", "reservation", "context", "source", "character", "lifecycle", "account-owner"} {
			t.Run(operation+"/"+changed, func(t *testing.T) {
				w, p, g, _, backend, queue, release := criticalPickupFixture(t)
				w.criticalAccountContainment = true
				backend.failure = criticalCertifiedRejection(t)
				if operation == "pickup" {
					w.onGetItem(p.Session, criticalPickupRequest(g.ID, 0))
				} else {
					p.Char.Inv[0] = g.Item
					w.unregisterGroundItem(g)
					w.items[g.Item.Index] = model.ItemDef{Index: g.Item.Index}
					w.onDropItem(p.Session, criticalDropRequest(placeInv, 0))
					g = pendingCriticalDrop(t, w)
				}
				awaitCriticalWrite(t, backend.entered)
				switch changed {
				case "object":
					replacement := *g
					w.groundItems[g.ID] = &replacement
				case "reservation":
					if operation == "pickup" {
						g.criticalPickup++
					} else {
						g.criticalDrop++
					}
				case "context":
					g.Item.Index++
				case "source":
					p.Char.Inv[0].Index++
				case "character":
					p.Char.UID += "-replacement"
				case "lifecycle":
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				case "account-owner":
					delete(w.accountSessions, accountSessionKey(p.Account.Name))
				}
				before := accountStateSnapshot(p.Account)
				objectBefore := *w.groundItems[g.ID]
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				w.tick()
				if !w.shuttingDown || w.shutdownErr == nil || !queue.stopping || len(queue.pending) != 1 ||
					len(w.criticalQuarantine) != 0 || !p.PersistencePoisoned || !p.Session.IsClosed() ||
					*w.groundItems[g.ID] != objectBefore || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("failed rejection check settled a replacement or released shared ownership")
				}
			})
		}
	}
}

func TestCriticalSharedUnknownPreservesFencesThroughShutdown(t *testing.T) {
	for _, operation := range []string{"pickup", "drop", "kill"} {
		for _, failure := range []string{"unknown", "indeterminate-rejection"} {
			t.Run(operation+"/"+failure, func(t *testing.T) {
				var w *World
				var p *Player
				var g *GroundItem
				var mob *Mob
				var backend *criticalTestStore
				var queue *criticalPersistence
				var release func()
				if operation == "kill" {
					w, p, mob, _ = killRewardTestWorld(t, nil)
					claimCriticalTestPlayers(t, w, p)
					gate, unblock := criticalGate(t)
					release = unblock
					backend = &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
					queue = newCriticalTestQueue(t, backend, 1, 2)
					t.Cleanup(release)
					w.criticalWrites = newCriticalPersistenceOwner(queue)
				} else {
					w, p, g, _, backend, queue, release = criticalPickupFixture(t)
					if operation == "drop" {
						p.Char.Inv[0] = g.Item
						w.unregisterGroundItem(g)
						w.items[g.Item.Index] = model.ItemDef{Index: g.Item.Index}
					}
				}
				w.criticalAccountContainment = true
				backend.failure = errors.New("shared write acknowledgement lost")
				if failure == "indeterminate-rejection" {
					backend.failure = errors.Join(criticalCertifiedRejection(t), errCriticalPersistenceIndeterminate)
				}
				observer := criticalPickupCompetitor(t, w, &GroundItem{X: p.X, Y: p.Y})
				before := accountStateSnapshot(p.Account)
				if operation == "kill" {
					w.killMobState(p, mob, 500, 37)
				} else if operation == "pickup" {
					w.onGetItem(p.Session, criticalPickupRequest(g.ID, 0))
				} else {
					w.onDropItem(p.Session, criticalDropRequest(placeInv, 0))
					g = pendingCriticalDrop(t, w)
				}
				awaitCriticalWrite(t, backend.entered)
				id := queue.accounts[accountSessionKey(p.Account.Name)]
				release()
				result := awaitCriticalResult(t, queue)
				w.applyCriticalCompletion(result)
				w.handle(command{s: p.Session})
				w.tick()
				retained := w.criticalWrites.continuations[id]
				if !w.shuttingDown || w.shutdownErr == nil || !queue.stopping || queue.pending[id] == nil ||
					!w.criticalAccountReserved(p.Account.Name) || len(w.criticalQuarantine) != 0 ||
					retained == nil || !retained.sharedWorld || !retained.delivered || retained.err == nil ||
					retained.apply != nil || retained.reject != nil || retained.afterAck != nil ||
					!observer.PersistencePoisoned || !observer.Session.IsClosed() || !p.PersistencePoisoned ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("shared uncertainty was treated as timed account-local containment")
				}
				if g != nil {
					w.expireGroundItems(w.now().Add(24 * time.Hour))
					if w.groundItems[g.ID] != g || !g.criticalReserved() {
						t.Fatal("unknown ground ownership was expired or cleared")
					}
				} else if w.mobsByID[mob.ID] != mob || !mob.Dead {
					t.Fatal("unknown kill was finalized or resurrected")
				}
				if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountStateSnapshot(observer.Account)}},
					[]*Player{observer}, func(criticalPersistenceResult) error { return nil }); !errors.Is(err, errCriticalPersistenceStopping) {
					t.Fatalf("maintenance admitted another durable mutation: %v", err)
				}
				if err := w.criticalWrites.complete(result); err == nil {
					t.Fatal("unknown completion was replayed")
				}
				done := make(chan error, 1)
				w.runShutdown(done)
				if err := <-done; err == nil || !w.shutdownFinished || queue.pending[id] == nil {
					t.Fatal("shutdown reported success or released uncertain ownership")
				}
				if len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("shared failure published success or saved a final RAM preimage")
				}
			})
		}
	}
}
