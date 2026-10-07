package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func criticalRecoveryFixture(t *testing.T, failure error) (*World, *Player, *Player, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _, _ := killRewardTestWorld(t, nil)
	unrelated, _ := networkedTestPlayer(2, "Unrelated", 2110, 2100)
	w.players[unrelated.Session], w.playersByID[unrelated.ID] = unrelated, unrelated
	w.indexPlayerCharacter(unrelated)
	w.updatePlayerSpatial(unrelated)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate, failure: failure}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	p.Char.CP, p.NextCPRecovery = -2, w.now().Add(-time.Second)
	return w, p, unrelated, backend, queue, release
}

func TestCriticalChaosRecoveryProposesBeforePublishingAndAllowsUnrelatedTicks(t *testing.T) {
	w, p, unrelated, backend, queue, release := criticalRecoveryFixture(t, nil)
	before, deadline := cloneCharacterState(p.Char), p.NextCPRecovery
	now := w.now()
	w.tickChaosRecovery(now)
	write := awaitCriticalWrite(t, backend.entered)
	if len(write.accounts) != 1 || write.accounts[0].Chars[p.CharSlot].CP != before.CP+1 {
		t.Fatalf("worker did not receive the proposed CP snapshot: %+v", write)
	}
	// Worker-visible snapshots cannot alias actor-owned characters.
	if write.accounts[0] == p.Account || &write.accounts[0].Chars[0] == p.Char {
		t.Fatal("worker received live account state")
	}
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.tickChaosRecovery(now)
		assertCriticalCombatUnchanged(t, p, before, 0, time.Time{})
		if !p.NextCPRecovery.Equal(deadline) || !w.criticalPlayerReserved(p) || len(queue.pending) != 1 {
			t.Fatalf("%s consumed the deadline, released the fence or duplicated recovery", phase)
		}
		setPlayerCurHP(unrelated.Char, 500)
		unrelated.NextRegen = now.Add(-time.Second)
		w.tickPlayerRegen(now)
		if playerCurHP(unrelated.Char) <= 500 || !unrelated.NextRegen.After(now) {
			t.Fatalf("%s blocked an unrelated player's actual regeneration tick", phase)
		}
	}
	w.processCriticalCompletions(1)
	if p.Char.CP != before.CP+1 || !p.NextCPRecovery.Equal(now.Add(chaosRecoveryInterval)) ||
		w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("actor did not install and publish recovery before releasing its fence")
	}
	w.tickChaosRecovery(now)
	if len(queue.pending) != 0 {
		t.Fatal("acknowledged recovery was resubmitted before the 450-second deadline")
	}
}

func TestCriticalChaosRecoveryRejectionDoesNotMutateOrFallback(t *testing.T) {
	for _, scenario := range []string{"full", "stopping", "identity"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, _, backend, queue, _ := criticalRecoveryFixture(t, nil)
			switch scenario {
			case "full":
				for _, name := range []string{"occupied-a", "occupied-b"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			case "stopping":
				queue.closeAdmission()
			case "identity":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			}
			before, deadline, pending := cloneCharacterState(p.Char), p.NextCPRecovery, len(queue.pending)
			w.tickChaosRecovery(w.now())
			if !reflect.DeepEqual(*p.Char, before) || !p.NextCPRecovery.Equal(deadline) ||
				p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != pending || p.criticalLifecycle != nil {
				t.Fatal("admission rejection mutated, published or submitted a synchronous fallback")
			}
		})
	}
}

func TestCriticalChaosRecoveryUnknownOutcomeDoesNotConfirmOrRetry(t *testing.T) {
	w, p, _, backend, queue, release := criticalRecoveryFixture(t, errors.New("commit acknowledgement lost"))
	before, deadline := cloneCharacterState(p.Char), p.NextCPRecovery
	w.tickChaosRecovery(w.now())
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !reflect.DeepEqual(*p.Char, before) || !p.NextCPRecovery.Equal(deadline) ||
		p.Session.QueuedPacketsForTest() != 0 || !w.criticalPlayerReserved(p) ||
		!p.PersistencePoisoned || !w.shuttingDown || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
		t.Fatal("unknown outcome confirmed recovery, released its fence or escaped maintenance")
	}
	w.tickChaosRecovery(w.now())
	if len(queue.pending) != 1 {
		t.Fatal("unknown recovery was retried")
	}
}

func TestCriticalChaosRecoveryDisconnectSettlesBeforeFinalSaveAndReadmission(t *testing.T) {
	w, p, _, backend, queue, release := criticalRecoveryFixture(t, nil)
	finalStore := w.store.(*killAuditStore)
	session, accountKey, uid, id := p.Session, accountSessionKey(p.Account.Name), p.Char.UID, p.ID
	reconnecting := gameNet.NewTestSession(3, 8)
	w.tickChaosRecovery(w.now())
	awaitCriticalWrite(t, backend.entered)
	w.handle(command{s: session})
	w.handle(command{s: session})
	w.handle(command{s: session, pkt: inboundPacket(wire.OpCharacterLogout, 12)})
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		if !session.IsClosed() || p.Char.CP != -2 || finalStore.saves != 0 || len(w.criticalDisconnects) != 1 ||
			w.players[session] != p || w.playersByID[id] != p || w.playersByCharacterUID[uid] != p ||
			w.accountSessions[accountKey] != session || w.claimAccountSession(reconnecting, p.Account.Name) {
			t.Fatalf("%s disconnected recovery saved stale CP or released live ownership", phase)
		}
	}
	w.processCriticalCompletions(1)
	finalWrite := awaitCriticalWrite(t, backend.entered)
	if p.Char.CP != -1 || finalWrite.accounts[0].Chars[p.CharSlot].CP != -1 || finalStore.saves != 0 ||
		w.players[session] != p || w.claimAccountSession(reconnecting, p.Account.Name) {
		t.Fatal("final asynchronous save omitted committed CP or released identity before acknowledgement")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if finalStore.saves != 0 || session.QueuedPacketsForTest() != 0 ||
		len(w.criticalDisconnects) != 0 || w.players[session] != nil || w.playersByID[id] != nil ||
		w.playersByCharacterUID[uid] != nil || w.accountSessions[accountKey] != nil || w.shuttingDown {
		t.Fatal("disconnect did not save committed CP once before cleanup, or published to closed transport")
	}
	w.processCriticalCompletions(1)
	w.handle(command{s: session})
	if finalStore.saves != 0 || len(backend.entered) != 0 || !w.claimAccountSession(reconnecting, p.Account.Name) {
		t.Fatal("completion replayed final persistence or account remained unavailable after cleanup")
	}
}

func TestCriticalChaosRecoveryUnknownDisconnectRetainsAccountWithoutFinalSave(t *testing.T) {
	w, p, _, backend, queue, release := criticalRecoveryFixture(t, errors.New("commit acknowledgement lost"))
	finalStore := w.store.(*killAuditStore)
	session := p.Session
	w.tickChaosRecovery(w.now())
	awaitCriticalWrite(t, backend.entered)
	w.handle(command{s: session})
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	w.handle(command{s: session})
	w.resumeCriticalDisconnects(8)
	if p.Char.CP != -2 || finalStore.saves != 0 || session.QueuedPacketsForTest() != 0 ||
		w.players[session] != p || len(w.criticalDisconnects) != 1 || !w.criticalPlayerReserved(p) ||
		w.accountSessions[accountSessionKey(p.Account.Name)] != session ||
		w.claimAccountSession(gameNet.NewTestSession(3, 8), p.Account.Name) || !w.shuttingDown {
		t.Fatal("unknown disconnected recovery was overwritten, confirmed, retried or readmitted")
	}
}
