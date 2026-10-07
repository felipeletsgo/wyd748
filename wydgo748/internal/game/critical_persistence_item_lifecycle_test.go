package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

// Exercise producer-specific committed items and recalculated score against
// physical disconnect and terminal shutdown, not just coordinator save counts.
func testCriticalItemLifecycle(t *testing.T, prepare, submit func(*World, *Player)) {
	t.Helper()
	for _, mode := range []string{"disconnect", "shutdown"} {
		t.Run(mode, func(t *testing.T) {
			w, p, fallback := cargoGoldContractWorld(t)
			prepare(w, p)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("fixture account admission failed")
			}
			gate, release := criticalGate(t)
			finalGate, releaseFinal := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			backend := &criticalCargoDisconnectStore{criticalTestStore: base, finalRelease: finalGate}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			t.Cleanup(releaseFinal)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			terminal := &criticalPurchaseTerminalStore{craftStore: &fallback.craftStore}
			if mode == "shutdown" {
				w.store = terminal
			}
			before := accountStateSnapshot(p.Account)
			session, uid, id, key := p.Session, p.Char.UID, p.ID, accountSessionKey(p.Account.Name)
			reconnecting := gameNet.NewTestSession(3, 8)
			submit(w, p)
			proposal := awaitCriticalWrite(t, base.entered)
			expected := accountStateSnapshot(proposal.accounts[0])
			w.recalcPlayer(&expected.Chars[p.CharSlot])
			first, second := make(chan error, 1), make(chan error, 1)
			if mode == "shutdown" {
				w.runShutdown(first)
				w.runShutdown(second)
			}
			w.handle(command{s: session})
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.handle(command{s: session})
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if !session.IsClosed() || w.players[session] != p || w.playersByID[id] != p ||
					w.playersByCharacterUID[uid] != p || w.accountSessions[key] != session ||
					len(queue.pending) != 1 || len(base.entered) != 0 || terminal.account != nil ||
					terminal.flushes != 0 || w.shutdownFinished || w.claimAccountSession(reconnecting, p.Account.Name) {
					t.Fatalf("%s lost lifecycle ownership or persisted/finished before the item acknowledgement", phase)
				}
			}
			w.processCriticalCompletions(1)
			if !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), accountPersistenceSnapshot(expected)) ||
				session.QueuedPacketsForTest() != 0 {
				t.Fatal("closed-transport completion lost committed items/score or published effects")
			}
			if mode == "shutdown" {
				for _, done := range []chan error{first, second} {
					select {
					case err := <-done:
						if err != nil {
							t.Fatal(err)
						}
					default:
						t.Fatal("shutdown did not complete after the item acknowledgement")
					}
				}
				if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), accountPersistenceSnapshot(expected)) ||
					fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 {
					t.Fatal("shutdown terminal snapshot lost committed item/score or saved more than once")
				}
				third := make(chan error, 1)
				w.runShutdown(third)
				if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
					t.Fatal("repeated shutdown replayed the item terminal snapshot")
				}
				return
			}
			final := awaitCriticalWrite(t, base.entered)
			if !reflect.DeepEqual(final.accounts[0], accountPersistenceSnapshot(expected)) || final.character == nil ||
				final.character.uid != uid || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
				t.Fatal("atomic final account/sidecar omitted committed item/score")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					releaseFinal()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.handle(command{s: session})
				w.resumeCriticalDisconnects(8)
				if len(queue.pending) != 1 || len(base.entered) != 0 || fallback.saves != 0 ||
					w.players[session] != p || w.accountSessions[key] != session || session.QueuedPacketsForTest() != 0 ||
					w.claimAccountSession(reconnecting, p.Account.Name) {
					t.Fatalf("final %s replayed persistence or released readmission early", phase)
				}
			}
			w.processCriticalCompletions(1)
			if w.players[session] != nil || w.playersByID[id] != nil || w.playersByCharacterUID[uid] != nil ||
				w.accountSessions[key] != nil || len(queue.pending) != 0 || len(base.entered) != 0 || fallback.saves != 0 ||
				session.QueuedPacketsForTest() != 0 || !w.claimAccountSession(reconnecting, p.Account.Name) || w.shuttingDown {
				t.Fatal("final acknowledgement failed to release ownership once without closed-session effects")
			}
		})
	}
}

func TestCriticalEquipmentSwapCommittedLifecycleSnapshots(t *testing.T) {
	for _, mount := range []bool{false, true} {
		name := "fairy"
		if mount {
			name = "fresh-mount"
		}
		t.Run(name, func(t *testing.T) {
			testCriticalItemLifecycle(t, seedCriticalEquipment, func(w *World, p *Player) {
				sp, dp := byte(3), byte(fairySlot)
				if mount {
					sp, dp = 4, mountSlot
				}
				w.onSwapItem(p.Session, criticalCargoSwapRequest(p, placeInv, sp, placeEquip, dp))
			})
		})
	}
}

func TestCriticalPostAcknowledgementFailurePoisonsSessionsAndSuppressesTerminalOverwrite(t *testing.T) {
	for _, panicFailure := range []bool{false, true} {
		name := "error"
		if panicFailure {
			name = "panic"
		}
		t.Run(name, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			terminal := &criticalPurchaseTerminalStore{craftStore: &fallback.craftStore}
			w.store = terminal
			proposed := accountStateSnapshot(p.Account)
			proposed.Chars[p.CharSlot].Gold--
			applied, reconciled := 0, 0
			id, err := w.submitCriticalWritePhased(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p}, false,
				func(criticalPersistenceResult) error { applied++; p.Char.Gold--; return nil },
				func(criticalPersistenceResult) error {
					reconciled++
					if w.criticalPlayerReserved(p) {
						t.Fatal("post-ack retained its persistence fence")
					}
					if panicFailure {
						panic("post-ack failure")
					}
					return errors.New("post-ack failure")
				})
			if err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, backend.entered)
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if applied != 1 || reconciled != 1 || len(queue.pending) != 0 || !w.shuttingDown || !queue.stopping ||
				!p.PersistencePoisoned || !p.Session.IsClosed() || w.shutdownErr == nil {
				t.Fatal("post-ack failure was replayed or escaped global containment")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 {
				t.Fatal("failed post-ack phase overwrote its confirmed write with a terminal snapshot")
			}
			if err := w.criticalWrites.complete(criticalPersistenceResult{id: id}); err == nil || applied != 1 || reconciled != 1 {
				t.Fatal("duplicate failure replayed item phases")
			}
		})
	}
}
